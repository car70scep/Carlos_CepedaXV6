#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64 kfreepagecount(void);

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

// uint64
// sys_sbrk(void)
// {
//   int addr;
//   int n;

//   if(argint(0, &n) < 0)
//     return -1;
//   addr = myproc()->sz;
//   if(growproc(n) < 0)
//     return -1;
//   return addr;
// }


// uint64 sys_sbrk(void)
// {
//   int n;

//   if (argint(0, &n) < 0)
//     return -1;

//   struct proc *p = myproc();
  
//   if(n < 0 && p->sz + n < p->sz)
//     return -1;

//   p->sz += n;
//   return p->sz - n;
// }

uint64 sys_sbrk(void)
{
  int n;

  if (argint(0, &n) < 0)
    return -1;

  struct proc *p = myproc();
  
  uint64 newsz = p->sz + n;
  if(newsz < p->sz || newsz >= TRAPFRAME){
    return -1;
  }
  p->sz = newsz;
  return p->sz - n;
}



uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// return the number of active processes in the system
// fill in user-provided data structure with pid,state,sz,ppid,name
uint64
sys_getprocs(void)
{
  uint64 addr;  // user pointer to struct pstat

  if (argaddr(0, &addr) < 0)
    return -1;
  return(procinfo(addr));
}


uint64
sys_freepmem(void){
  int count = kfreepagecount();
  return count;
}

uint64
sys_memuser(void){
  int count = kfreepagecount();
  return count;
}

// uint64
// sys_sem_init(void){
//   uint64 sem_addr;
//   int init_val, pshared;

//   if(argaddr(0,&sem_addr)<0){
//     return -1;
//   }
//   if(argint(1,&pshared)<0||argint(2,&init_val)<0){
//     return -1;
//   }
//   if(pshared == 0){
//     return -1;
//   }
//   int index = semalloc();
//   semtable.sem[index].count = init_val;

//   if(copyout(myproc()->pagetable,sem_addr,(char*)&index,sizeof(int)<0)){
//     semdealloc(index);
//     return -1;
//   }
//  return 0;
// }

// uint64
// sys_sem_destroy(void){
//   uint64 sem_addr;

//   if(argaddr(0,&sem_addr)<0){
//     return -1;
//   }
//   int sem_index;
//   acquire(&semtable.lock);

//   if(copyin(myproc()->pagetable,(char*)&sem_index,sem_addr,sizeof(int)<0 )){
//     release(&semtable.lock);
//     return -1;
//   }
//   semdealloc(sem_index);
//   release(&semtable.lock);
//   return 0;
// }

// uint64
// sys_sem_wait(void){
//   uint64 sem_addr;

//   if(argaddr(0,&sem_addr)<0){
//     return -1;
//   }
//   int sem_index;

//   copyin(myproc()->pagetable,(char*)&sem_index,sem_addr,sizeof(int));

//   acquire(&semtable.sem[sem_index].lock);

// 	if(semtable.sem[sem_index].count > 0){
// 		semtable.sem[sem_index].count--;	
// 		release(&semtable.sem[sem_index].lock);
// 		return 0;
// 	}else{
// 		while(semtable.sem[sem_index].count == 0){
// 			sleep((void*)&semtable.sem[sem_index], &semtable.sem[sem_index].lock);
// 			//release(&semtable.sem[addr].lock);
// 		}
// 		semtable.sem[sem_index].count -=1;
// 		release(&semtable.sem[sem_index].lock);
// 	}
// 	return 0;
// }

// uint64
// sys_sem_post(void){
//   uint64 sem_addr;

//   if(argaddr(0,&sem_addr)<0){
//     return -1;
//   }
//   int sem_index;

//   copyin(myproc()->pagetable,(char*)&sem_index,sem_addr,sizeof(int));

//   acquire(&semtable.sem[sem_index].lock);
//   semtable.sem[sem_index].count +=1;
//   wakeup((void*)&semtable.sem[sem_index]);
//   release(&semtable.sem[sem_index].lock);
//   return 0;
// }

int
sys_sem_init(void){
	uint64 s;
	int index;
	int value;
	int pshared;
	//struct semtab semtable;
	
	//semaphore failed
	if(argaddr(0,&s) < 0){
		return -1;
	}
	//pshared failed
	if(argint(1,&pshared) < 0){
		return -1;
	}
	//value failed
	if(argint(2,&value) < 0){
		return -1;
	}
	//making sure pshared is not equal to zero
	if(pshared == 0){
		return -1;
	}
	//initialization
	index = semalloc();
	semtable.sem[index].count = value;
	//copyout 
	if(copyout(myproc()->pagetable, s, (char*)&index, sizeof(index)) <0){
		return -1;
	}
	
	return 0;
}
int
sys_sem_destroy(void){
	uint64 s;
	int addr;
	//semaphore failed
	if(argaddr(0, &s) < 0){
		return -1;
	}
	
	//destroy
	acquire(&semtable.lock);
	
	if(copyin(myproc()->pagetable, (char*)&addr, s, sizeof(int))<0){
		release(&semtable.lock);
		return -1;
	}
	sedealloc(addr);
	release(&semtable.lock);
	return 0;
}
int
sys_sem_wait(void){
	uint64 s;
	int addr;
	//semaphore failed
	if(argaddr(0, &s) < 0){
		return -1;
	}
	//get address
	copyin(myproc()->pagetable, (char*)&addr, s, sizeof(int));
	
	acquire(&semtable.sem[addr].lock);
	//decrement
	if(semtable.sem[addr].count > 0){
		semtable.sem[addr].count--;	
		release(&semtable.sem[addr].lock);
		return 0;
	}else{
		while(semtable.sem[addr].count == 0){
			sleep((void*)&semtable.sem[addr], &semtable.sem[addr].lock);
			//release(&semtable.sem[addr].lock);
		}
		semtable.sem[addr].count--;
		release(&semtable.sem[addr].lock);
	}
	
	return 0;
}
int
sys_sem_post(void){
	uint64 s;
	int addr;
	//semaphore failed
	if(argaddr(0, &s) < 0){
		return -1;
	}
	//get address
	copyin(myproc()->pagetable, (char*)&addr, s, sizeof(int));
	
	acquire(&semtable.sem[addr].lock);
	//increment
	semtable.sem[addr].count++;
	wakeup((void*)&semtable.sem[addr]);
	
	release(&semtable.sem[addr].lock);
	
	return 0;
}