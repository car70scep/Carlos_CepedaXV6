#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"

struct semtab semtable;

void seminit(void){
  initlock(&semtable.lock,"semtable");
  for(int i = 0; i < NSEM; i++)
    initlock(&semtable.sem[i].lock,"sem");
}

int semalloc(void){
  acquire(&semtable.lock);
  for(int i = 0; i < NSEM; i++){
     if(semtable.sem[i].valid == 0){
        semtable.sem[i].valid = 1;
        release(&semtable.lock);
        return i;
    }
  }
    release(&semtable.lock);
    return -1;
}

void semdealloc(sem_t index){
   acquire(&semtable.lock);
   semtable.sem[index].valid = 0;
   release(&semtable.lock);
}