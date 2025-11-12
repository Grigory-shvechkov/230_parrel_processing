#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <signal.h>

#define NUM_CORES 3
#define MAX_TASK_LEN 32

extern int main_to_core[NUM_CORES][2];
extern int core_to_main[NUM_CORES][2];
extern pid_t pids[NUM_CORES];
extern volatile sig_atomic_t msg_count[NUM_CORES];

void no_interrupt_sleep(int sec);
int extract_bits(int num, int n);
void core_loop(int core_id, int max_bits);
void sig_handler(int sig);

#endif // UTILS_H
