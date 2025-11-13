Implemntation of scheduler.c

Run the program by ./scheduler <num_tasks> <max_bits>, i.e ./scheduler 10 5.
Main works by parsing two command line arguments. It creates two pipes per core, main_to_core 
and core_to_main to support two-way communication. Additonally main registers unique signal handlers 
for each core using SIGRTMIN + ith core and sets them with sigaction. Each core is made using fork() and 
waits for a task message from main by calling read() in the child process. When a task message is received, 
the core_loop simulates processing using the no_interrupt_sleep() function and writes its result back to main 
through the pipe. The message sent to each core is correctly formatted as task_id_bits using snprintf() 
so that each core can process the correct task.

Task scheduling works by defining an array of ints to indicate which cores are idle and ready for new processes. 
If there are more tasks to be processed and a core is idle, a new task is assigned to that core by writing the message 
into the main_to_core pipe and setting its idle flag to 0. Results are handled using the sigaction structure to capture 
signals from finished processes. When a child finishes, it executes kill() to send its assigned signal (SIGRTMIN + core_id) 
to the main process. This triggers the sig_handler in util.c, which increments the count in the msg_count array that tracks 
how many results are ready from that core. Each ready result is then read using read() from the pipe and stored in a 2D results 
array where each row corresponds to a specific core. After all tasks are complete, the main process closes all pipes with 
close(main_to_core[i][1]) and close(core_to_main[i][0]), and calls wait(NULL) to kill all child processes. 
This ensures proper cleanup of all resources. Once finished, results are printed. Error handling is implemented after every 
system call; fork, pipe, read, write, and sigaction, using perror() followed by exit(1) on failure. This satisfies the process
requirements by correctly creating and terminating children, using pipes for two-way communication, correctly closing pipe ends, 
and formatting and sending messages. It satisfies the signal requirements by choosing appropriate real-time signals, using sigaction
for handler registration, implementing an atomic handler with volatile sig_atomic_t, and sending signals correctly when tasks complete. 
Command-line arguments are handled properly and outputs are correctly formatted.

Implemntation of util.c

All helper functions are defined here to keep scheduler.c organized. The core_loop function takes two integer arguments, core_id
and max_bits. It closes unused pipes to avoid garbage values, reads from the main_to_core pipe, simulates the assigned work 
with no_interrupt_sleep(), performs bit extraction with extract_bits(), then writes the result back to the parent through core_to_main.
After finishing, the core sends a signal using kill() to the main process to indicate task completion.
The sig_handler function determines which core sent the signal using sig - SIGRTMIN and increments the counter for that core in the
msg_count array. This ensures atomic updates since msg_count is declared as volatile sig_atomic_t. The no_interrupt_sleep() function
is taken from the provided assignment code and used to simulate processing time, while extract_bits() shifts the bits 
of num by n. This file satisfies the design and implementation requirements by separating functionality, 
using clear variable and function names, maintaining efficient control flow, and minimizing global variables.

The use of util.h
This header connects scheduler.c and util.c by declaring shared functions, constants, and global variables. 
It allows both files to work together while keeping the implementation modular and clean.