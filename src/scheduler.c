#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include <time.h>


int main_to_core[NUM_CORES][2];
int core_to_main[NUM_CORES][2];
pid_t pids[NUM_CORES];
volatile sig_atomic_t msg_count[NUM_CORES] = {0};


int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr,"Usage: %s <num_tasks> <max_bits>\n",argv[0]);
        return 1;
    }

    int num_tasks = atoi(argv[1]);
    int max_bits = atoi(argv[2]);
    srand(time(NULL));

   
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sa.sa_flags = SA_RESTART; 
    sigemptyset(&sa.sa_mask);

    for(int i = 0; i < NUM_CORES; i++) {
        if(sigaction(SIGRTMIN + i, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }
    }


    for(int i = 0; i < NUM_CORES; i++) {
        if(pipe(main_to_core[i]) < 0 || pipe(core_to_main[i]) < 0) {
            perror("pipe");
            exit(1);
        }

        pids[i] = fork();
        if(pids[i] < 0) { perror("fork"); exit(1); }
        if(pids[i] == 0) {
            core_loop(i, max_bits); // child process
        }

        // Parent closes unused ends
        close(main_to_core[i][0]);
        close(core_to_main[i][1]);
    }


    // Task scheduling
    int tasks_done = 0, tasks_assigned = 0;
    int idle[NUM_CORES] = {1,1,1};
    int results[NUM_CORES][num_tasks];
    int res_count[NUM_CORES] = {0};

    while(tasks_done < num_tasks) {
        // Assign tasks to idle cores
        for(int i = 0; i < NUM_CORES && tasks_assigned < num_tasks; i++) {
            if(idle[i]) {
                int bits = rand() % max_bits + 1;
                char buf[MAX_TASK_LEN];
                snprintf(buf, MAX_TASK_LEN, "%d_%d", tasks_assigned, bits);
                if(write(main_to_core[i][1], buf, strlen(buf)) != (ssize_t)strlen(buf)) {
                    perror("write");
                    exit(1);
                }
                idle[i] = 0;
                tasks_assigned++;
            }
        }

        // Read singals
        for(int i = 0; i < NUM_CORES; i++) {
            while(msg_count[i] > 0) {
                int res;
                if(read(core_to_main[i][0], &res, sizeof(res)) != sizeof(res)) {
                    perror("read");
                    exit(1);
                }
                results[i][res_count[i]++] = res;
                idle[i] = 1;
                tasks_done++;
                msg_count[i]--;
            }
        }
    }


    //post processing /results
    for(int i = 0; i < NUM_CORES; i++) {
        close(main_to_core[i][1]);
        close(core_to_main[i][0]);
    }

    for(int i = 0; i < NUM_CORES; i++)
        wait(NULL);

    //print results
    for(int i = 0; i < NUM_CORES; i++) {
        printf("Core %d: ", i+1);
        for(int j = 0; j < res_count[i]; j++) {
            printf("%d", results[i][j]);
            if(j < res_count[i]-1) printf(", ");
        }
        printf("\n");
    }

    return 0;
}
