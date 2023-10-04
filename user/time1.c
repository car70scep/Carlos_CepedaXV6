#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#include "kernel/stat.h"
#include "kernel/pstat.h"

#define MAXARGS 16

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: time1 command [args...]\n");
        exit(1);
    }

    // Start timing
    int start_time = uptime();

    // Fork a child process to execute the command
    int pid = fork();
    if (pid < 0) {
        fprintf(2, "Fork failed\n");
        exit(1);
    } else if (pid == 0) {
        // This is the child process
        exec(argv[1], argv + 1);
        // If exec() fails, print an error message and exit
        fprintf(2, "exec failed\n");
        exit(1);
    } else {
        // This is the parent process
        int status;
        wait(&status);

        // Stop timing
        int end_time = uptime();

        // Calculate and print elapsed time
        int elapsed_time = end_time - start_time;
        printf("Time: %d ticks\n", elapsed_time);

        // Print elapsed time in a different format
        printf("elapsed time: %d ticks\n", elapsed_time);
    }
   exit(0);
}