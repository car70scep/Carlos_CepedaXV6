#include "user/user.h"
#include "kernel/types.h"
#include "kernel/stat.h"

int main(){
  int upTime = uptime();
  printf("up %d clock ticks\n", upTime);
  exit(0);
}