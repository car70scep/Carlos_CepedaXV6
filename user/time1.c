#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"
#define MAXARGS 16

int
main(int argc, char *argv[]){
  int i, start, elapsed, rc;
  char *newargv[MAXARGS];
  struct rusage ru;

  if(argc < 2){
      printf("Usage: time <comm> [args...]");
      exit(-1);
   }
   for(i = 1; i < argc; i++){
       newargv[i-1] = argv[i];
    }
    newargv[argc-1] = NULL;
    start = uptime();
    rc = fork();

    if(rc == 0){
       exec(newargv[0], newargv);
       perror("execvp");
      exit(-1);
    }else if (rc >0) > {
     waitpid(rc,NULL,0);
     elapsed = uptime() - start;
      getrusage(RUSAGE_CHILDREN, &ru);
     printf("Elapsed Time: %d Ticks, CPU Time: %d Ticks, CPU Percentage: %d% \n", elapsed, ru.cputime, ru.cputime*100/elapsed);
     exit(0);
}else {
perror("fork");
exit(-1);
}
}
