#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
  int upTime = uptime();
  printf("up %d clock ticks\n", upTime);
  exit(0);
}