#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "util.h"


void sig_handler(int sig) {
    int core = sig - SIGRTMIN;
    if(core >= 0 && core < NUM_CORES)
        msg_count[core]++;
}

void no_interrupt_sleep(int sec) {
    struct timespec req, rem;
    req.tv_sec = sec;
    req.tv_nsec = 0;
    while(nanosleep(&req, &rem) == -1 && errno == EINTR)
        req = rem;
}

int extract_bits(int num, int n) {
    return num & ((1 << n) - 1);
}

void core_loop(int core_id, int max_bits) {
    (void)max_bits; 
    close(main_to_core[core_id][1]);
    close(core_to_main[core_id][0]);
    char buf[MAX_TASK_LEN];

    while(1) {
        ssize_t n = read(main_to_core[core_id][0], buf, MAX_TASK_LEN);
        if(n <= 0) break;  
        buf[n] = '\0';

        int task_id, bits;
        sscanf(buf, "%d_%d", &task_id, &bits);

        no_interrupt_sleep(1);
        int res = extract_bits(task_id, bits);

        if(write(core_to_main[core_id][1], &res, sizeof(res)) != sizeof(res)) {
            perror("write");
            exit(1);
        }

        kill(getppid(), SIGRTMIN + core_id);
    }

    close(main_to_core[core_id][0]);
    close(core_to_main[core_id][1]);
    exit(0);
}
