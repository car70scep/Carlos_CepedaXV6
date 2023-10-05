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
    int start_time = uptime();
    int pid = fork();
    if (pid < 0) {
      fprintf(2, "Fork failed\n");
      exit(1);
    } else if (pid == 0) {
      exec(argv[1], argv + 1);
      fprintf(2, "exec failed\n");
      exit(1);
    } else {
      int status;
      wait(&status);
      int end_time = uptime();
      int elapsed_time = end_time - start_time;
      printf("Time: %d ticks\n", elapsed_time);
      printf("elapsed time: %d ticks\n", elapsed_time);
    }
   exit(0);
}