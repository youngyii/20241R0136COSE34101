#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX 10       // process 최대 개수
#define SKD 6        // scheduling 알고리즘 개수
#define ITER 100     // best scheduling을 찾을 때 실험 횟수

typedef struct{
    int PID;         // process ID
    int arrival;     // Ready Queue에 도착하는 시간
    int CPU_burst;   // 총 CPU 수행 시간
    int priority;    // 우선순위 값
} process;

int process_num;     // process 개수
process info_p[MAX]; // process 정보 저장(원본)
process p[MAX];      // 각 알고리즘을 실행할 때 info_p로부터 복사해서 사용
char *skd_alg[SKD+1] = {"FCFS", "SJF", "Priority(larger number, larger priority)", "RR", "preemptive SJF", "preemptive Priority", "Exit"};
int waiting_t[SKD], turnaround_t[SKD]; // 대기 시간, 반환 시간
int no_print = 0;    // Gantt chart no print(evaluation의 경우 1)

void create_process(void); // process 생성
void schedule(void);       // 어떤 scheduling을 사용할지 선택
void copy_process(void);   // process 정보 복사(ori -> p)
void gantt(int start, int end, int idx); // start부터 end까지 idx번째 process가 CPU 사용

void FCFS(void);
void SJF(void);
void Priority(void);
void RR(void);

void preemptive_SJF(void);
int find_shortest(int time, int cur);

void preemptive_Priority(void);
int find_highest(int time, int cur);

void Evaluation(void); // 각 알고리즘 성능 평가
void Find_Best_Scheduling(void); // process 랜덤생성 & evaluation 반복하여 best scheduling 찾기

int main(){
    int choice;

    printf("1. Test scheduling\n2. Find best scheduling\nEnter the number: "); 
    scanf("%d", &choice);

    if(choice == 1){
        create_process();
        schedule();
        Evaluation();
    }
    else if(choice == 2) 
        Find_Best_Scheduling();
    else 
        printf("Wrong input! Please restart\n");

    return 0;
}

void create_process(void){
    char choice;

    printf("Enter the number of processes(1~%d): ", MAX);
    scanf("%d", &process_num);

    for(int i = 0; i < process_num; i++){
        printf("Enter ID of %dth process: ", i + 1);
        scanf("%d", &info_p[i].PID);
    }

    printf("Random creation? Y/N: "); // Random 입력 여부
    do { scanf("%c", &choice);
    } while(choice == '\n');

    if(choice == 'Y' || choice == 'y'){
        srand(time(NULL));

        for (int i = 0; i < process_num; i++) {
            info_p[i].arrival = (rand() % 20); // 0~19
            info_p[i].CPU_burst = (rand() % 20) + 1; // 1~20
            info_p[i].priority = (rand() % process_num) + 1; // 1~process개수
        }

        printf("\nEach process's (arrival time / CPU burst time / priority)\n");
        for(int i = 0; i < process_num; i++)
            printf("[process %d]: (%d / %d / %d)\n", info_p[i].PID, info_p[i].arrival, info_p[i].CPU_burst, info_p[i].priority);
    }
    else if(choice == 'N' || choice == 'n'){
        for(int i = 0; i < process_num; i++){
            printf("[process %d]: Enter arrival time, CPU burst time, priority: ", info_p[i].PID);
            scanf("%d %d %d", &info_p[i].arrival, &info_p[i].CPU_burst, &info_p[i].priority);
        }
    }
    else {
        printf("Wrong input! Please restart\n");
        return 0;
    }
}

void schedule(void){
    int choice;

    while(1){
        printf("\nwhich scheduling?\n");
        for(int i = 0; i < SKD + 1; i++)
            printf("%d. %s / ", i + 1, skd_alg[i]);
        scanf("%d", &choice);

        if(choice == SKD + 1) break; // Exit

        copy_process();
        printf("\n---%s start---\n", skd_alg[choice - 1]);

        switch (choice) {
        case 1:
            FCFS();
            break;
        case 2:
            SJF();
            break;
        case 3:
            Priority();
            break;
        case 4:
            RR();
            break;
        case 5:
            preemptive_SJF();
            break;
        case 6:
            preemptive_Priority();
            break;
        default:
            break;
        }

        printf("---%s end---\n", skd_alg[choice-1]);
    }
}

void copy_process(void){
    for(int i = 0; i < process_num; i++)
        p[i] = info_p[i];
}

void gantt(int start, int end, int idx){
    if(no_print) return; // no_print가 1이면 Find_Best_Scheduling() 실행 중. 이때는 성능 분석만 하므로 간트 차트를 출력하지 않는다.

    printf("%d~%d: ", start, end);

    if(idx == -1) printf("IDLE\n"); // CPU가 유휴 상태
    else printf("[process %d] used CPU\n", info_p[idx].PID); // 해당 index의 process가 CPU 점유
}

void FCFS(void){
    int time = 0;     // 현재 시간
    int finished = 0; // CPU 작업이 끝난 process 개수

    waiting_t[0] = 0;
    turnaround_t[0] = 0;

    while(finished < process_num){
        int idx = -1; 
        
        for(int i = 0; i < process_num; i++){
            if(!p[i].CPU_burst) continue;
            
            if(idx == -1 || p[i].arrival < p[idx].arrival) // arrival time이 가장 빠른 process
                idx = i;
        }

        if(time < p[idx].arrival){ // process가 아직 도착 안 했을 경우
            gantt(time, p[idx].arrival, -1); // CPU 유휴
            time = p[idx].arrival;
        }

        gantt(time, time + p[idx].CPU_burst, idx); // CPU 점유

        waiting_t[0] += (time - p[idx].arrival);
        turnaround_t[0] += (time - p[idx].arrival + p[idx].CPU_burst);

        time += p[idx].CPU_burst;
        p[idx].CPU_burst = 0;
        finished++;
    }
}

void SJF(void){
    int time = 0;     // 현재 시간
    int finished = 0; // CPU 작업이 끝난 process 개수

    waiting_t[1] = 0;
    turnaround_t[1] = 0;

    while(finished < process_num){
        int idx = -1;

        for(int i = 0; i < process_num; i++){
            if(!p[i].CPU_burst) continue;

            if(p[i].arrival <= time) // time 시간에 도착해있는 프로세스 중 CPU burst가 가장 짧은 process, 같으면 먼저 도착한 process
                if(idx == -1 || p[i].CPU_burst < p[idx].CPU_burst || (p[i].CPU_burst == p[idx].CPU_burst && p[i].arrival < p[idx].arrival))
                    idx = i;
        }
        
        if(idx == -1){ // 도착해있는 process X -> 가장 빨리 도착할 process
            for(int i = 0; i < process_num; i++){ 
                if(!p[i].CPU_burst) continue;
                
                // arrival time 같으면 CPU burst가 가장 짧은 process
                if(idx == -1 || p[i].arrival < p[idx].arrival || (p[i].arrival == p[idx].arrival && p[i].CPU_burst < p[idx].CPU_burst))
                    idx = i;
            }

            gantt(time, p[idx].arrival, -1); // process 도착 전까지 CPU 유휴
            time = p[idx].arrival;
        }

        gantt(time, time + p[idx].CPU_burst, idx); // CPU 점유

        waiting_t[1] += (time - p[idx].arrival);
        turnaround_t[1] += (time - p[idx].arrival + p[idx].CPU_burst);

        time += p[idx].CPU_burst;
        p[idx].CPU_burst = 0;
        finished++;
    }
}

void Priority(void){  // larger number, larger priority
    int time = 0;     // 현재 시간
    int finished = 0; // CPU 작업이 끝난 process 개수

    waiting_t[2] = 0;
    turnaround_t[2] = 0;

    while(finished < process_num){
        int idx = -1;

        for(int i = 0; i < process_num; i++){
            if(!p[i].CPU_burst) continue;

            if(p[i].arrival <= time) // time 시간에 도착해있는 process 중 priority가 가장 높은 process, 같으면 먼저 도착한 process
                if(idx == -1 || p[i].priority > p[idx].priority || (p[i].priority == p[idx].priority && p[i].arrival < p[idx].arrival))
                    idx = i;
        }
        
        if(idx == -1){ // 도착해있는 process X -> 가장 빨리 도착할 process
            for(int i = 0; i < process_num; i++){ // arrival time 같으면 priority가 가장 높은 process
                if(!p[i].CPU_burst) continue;

                if(idx == -1 || p[i].arrival < p[idx].arrival || (p[i].arrival == p[idx].arrival && p[i].priority > p[idx].priority))
                    idx = i;
            }

            gantt(time, p[idx].arrival, -1); // process 도착 전까지 CPU 유휴
            time = p[idx].arrival;
        }

        gantt(time, time + p[idx].CPU_burst, idx); // CPU 점유

        waiting_t[2] += (time - p[idx].arrival);
        turnaround_t[2] += (time - p[idx].arrival + p[idx].CPU_burst);

        time += p[idx].CPU_burst;
        p[idx].CPU_burst = 0;
        finished++;
    }
}

void RR(void){
    int quantum = 3;
    int time = 0;     // 현재 시간
    int finished = 0; // CPU 작업이 끝난 process 개수

    waiting_t[3] = 0;
    turnaround_t[3] = 0;

    if(no_print == 0){ // best scheduling 찾는 것이 아닐 때
        printf("time quantum: ");
        scanf("%d", &quantum);
    }

    while(finished < process_num){
        int idx = -1;
        int end; // 이번 작업이 끝나는 시간. time + min(quantum, p[idx].CPU_burst)
        
        for(int i = 0; i < process_num; i++){
            if(!p[i].CPU_burst) continue;
            
            if(idx == -1 || p[i].arrival < p[idx].arrival) // 도착 시간이 가장 빠른 process
                idx = i;
        }

        if(time < p[idx].arrival){
            gantt(time, p[idx].arrival, -1); // process 도착 전까지 CPU 유휴
            time = p[idx].arrival;
        }

        end = time;

        waiting_t[3] += (time - p[idx].arrival);

        if(p[idx].CPU_burst <= quantum){ // arrival time이 가장 빠른 process의 CPU burst가 quantum 이하 -> 종료
            turnaround_t[3] += (time - p[idx].arrival + p[idx].CPU_burst);

            end += p[idx].CPU_burst;
            p[idx].CPU_burst = 0;
            finished++;
        }
        else{ // 다시 ready queue로
            turnaround_t[3] += (time - p[idx].arrival + quantum);

            end += quantum;
            p[idx].CPU_burst -= quantum;
            p[idx].arrival = end;
        }

        gantt(time, end, idx); // CPU 점유

        time = end;
    }
}

void preemptive_SJF(void){
    int time = 0;        // 현재 시간
    int finished = 0;    // CPU 작업이 끝난 process 개수
    int cur = -1;        // 현재 CPU를 점유하고 있는 프로세스의 인덱스 -1:유휴 상태 
    int cur_started = 0; // 현재 CPU를 점유하는 프로세스의 점유 시작 시간

    waiting_t[4] = 0;
    turnaround_t[4] = 0;

    while(finished < process_num){
        int next = find_shortest(time, cur); // time 시간에 도착한 프로세스들 중 cur보다 CPU burst가 짧은 프로세스의 인덱스를 리턴

        if(next != -1){
            if(cur == -1){
                if(cur_started != time)
                    gantt(cur_started, time, -1); // 유휴 상태
            }

            else{
                gantt(cur_started, time, cur); // CPU 점유

                p[cur].arrival = time; // 점유하던 프로세스는 다시 ready queue로
            }

            waiting_t[4] += (time - p[next].arrival);
            turnaround_t[4] += (time - p[next].arrival);

            cur = next;
            cur_started = time;
        }

        time++;

        if(cur != -1){ // CPU를 점유하는 프로세스가 있을 경우
            p[cur].CPU_burst--;

            turnaround_t[4]++;

            if(p[cur].CPU_burst == 0){ // 만약 작업이 끝난 경우
                gantt(cur_started, time, cur);
                finished++;

                cur = -1;
                cur_started = time;
            }
        }
    }
}

int find_shortest(int time, int cur){ // time 시간에 도착해있는 프로세스 중 cur보다 CPU burst가 짧은 프로세스의 인덱스 리턴
    int idx = -1;
    
    for(int i = 0; i < process_num; i++){
        if(i == cur || p[i].CPU_burst == 0 || p[i].arrival > time) continue;

        if(idx != -1){
            if(p[i].CPU_burst < p[idx].CPU_burst || (p[i].CPU_burst == p[idx].CPU_burst && p[i].arrival < p[idx].arrival))
                idx = i;
        }

        else if(cur != -1){ // idx가 -1인데 cur가 -1이 아닐 경우 cur와 비교
            if(p[i].CPU_burst < p[cur].CPU_burst)
                idx = i;
        }

        else{ // 둘 다 -1일 경우
            idx = i;
        }
    }

    return idx;
}

void preemptive_Priority(void){
    int time = 0;        // 현재 시간
    int finished = 0;    // CPU 작업이 끝난 process 개수
    int cur = -1;        // 현재 CPU를 점유하고 있는 프로세스의 인덱스 -1:유휴 상태
    int cur_started = 0; // 현재 CPU를 점유하는 프로세스의 점유 시작 시간

    waiting_t[5] = 0;
    turnaround_t[5] = 0;

    while(finished < process_num){
        int next = find_highest(time, cur); // time 시간에 도착한 프로세스들 중 cur보다 Priority가 높은 프로세스의 인덱스를 리턴

        if(next != -1){
            if(cur == -1){
                if(cur_started != time)
                    gantt(cur_started, time, -1); // 유휴 상태
            }

            else{
                gantt(cur_started, time, cur); // CPU 점유

                p[cur].arrival = time; // 점유하던 프로세스는 다시 ready queue로
            }

            waiting_t[5] += (time - p[next].arrival);
            turnaround_t[5] += (time - p[next].arrival);

            cur = next;
            cur_started = time;
        }

        time++;

        if(cur != -1){ // CPU를 점유하는 프로세스가 있을 경우
            p[cur].CPU_burst--;

            turnaround_t[5] += 1;

            if(p[cur].CPU_burst == 0){ // 만약 작업이 끝난 경우
                gantt(cur_started, time, cur);
                finished++;

                cur = -1;
                cur_started = time;
            }
        }
    }
}

int find_highest(int time, int cur){
    int idx = -1; // time 시간에 도착해있는 프로세스 중 cur보다 Priority가 높은 프로세스의 인덱스 리턴

    for(int i=0; i<process_num; i++){
        if(i == cur || p[i].CPU_burst == 0 || p[i].arrival > time) continue;

        if(idx != -1){
            if(p[i].priority > p[idx].priority || (p[i].priority == p[idx].priority && p[i].arrival < p[idx].arrival))
                idx = i;
        }

        else if(cur != -1){ // idx가 -1인데 cur가 -1이 아닐 경우 cur와 비교
            if(p[i].priority > p[cur].priority)
                idx = i;
        }

        else{ // 둘 다 -1일 경우
            idx = i;
        }
    }

    return idx;
}

void Evaluation(void){
    int min_wait_idx = -1, min_turn_idx = -1; // waiting time과 turnaround time이 최소인 scheduling의 인덱스

    printf("\n---Average waiting time and turnaround time in each algorithm---\n");
    for(int i = 0; i < SKD; i++){
        float avg_wait, avg_turn;

        if(turnaround_t[i] == 0) continue;

        avg_wait = (float)waiting_t[i]/process_num;
        avg_turn = (float)turnaround_t[i]/process_num;

        printf("%d. %s: %.2f, %.2f\n", i+1, skd_alg[i], avg_wait, avg_turn);
        
        if(min_wait_idx == -1 || waiting_t[i] < waiting_t[min_wait_idx]) min_wait_idx = i;
        if(min_turn_idx == -1 || turnaround_t[i] < turnaround_t[min_turn_idx]) min_turn_idx = i;
    }

    if(min_wait_idx != -1){
        printf("\nAlgorithm with minimum waiting time is %s\n", skd_alg[min_wait_idx]);
        printf("Algorithm with minimum turnaround time is %s\n", skd_alg[min_turn_idx]);
    }
    else printf("No data\n");
}

void Find_Best_Scheduling(void){
    int best = 0;         // best scheduling의 인덱스
    int voted[SKD] = {0}; // 각 scheduling이 best scheduling으로 선정된 횟수(총 ITER(100)회 test)

    no_print = 1;
    process_num = MAX;

    for(int i = 0; i < ITER; i++){
        int min_wait_idx = 0;

        srand(time(NULL) + i);

        for(int i = 0; i < process_num; i++){
            info_p[i].arrival = (rand()%20);
            info_p[i].CPU_burst = (rand()%20)+1;
            info_p[i].priority = (rand()%process_num)+1;
        }

        copy_process();
        FCFS();

        copy_process();
        SJF();
        
        copy_process();
        Priority();
        
        copy_process();
        RR();
        
        copy_process();
        preemptive_SJF();
        
        copy_process();
        preemptive_Priority();

        /*printf("\nTry %d!\n", i+1);
        for(int j=0; j<SKD; j++)
            printf("%d %d\n", waiting_t[j], turnaround_t[j]);*/

        for(int j = 0; j < SKD; j++){
            if(waiting_t[j] < waiting_t[min_wait_idx]) min_wait_idx = j; // waiting time이 최소인 스케줄링 찾기
        }
        
        for(int j = 0; j < SKD; j++){
            //if(j == 1 || j == 4) continue;
            if(waiting_t[j] == waiting_t[min_wait_idx]) voted[j]++; // 해당 스케줄링에 투표(공동 1등 가능)
        }
    }

    for(int i = 0; i < SKD; i++)
        if(voted[i] > voted[best]) best = i;

    printf("\n---The number of votes for each algorithm(allowing plural voting)---\n");
    for(int i=0; i<SKD; i++){
        //if(i==1 || i==4) continue;
        printf("%d. %s: %d votes\n", i+1, skd_alg[i], voted[i]);
    }

    printf("\n%s is the best scheduling\n", skd_alg[best]);
}