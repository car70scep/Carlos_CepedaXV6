#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
// void
// usertrap(void)
// {
//   int which_dev = 0;

//   if((r_sstatus() & SSTATUS_SPP) != 0)
//     panic("usertrap: not from user mode");

//   // send interrupts and exceptions to kerneltrap(),
//   // since we're now in the kernel.
//   w_stvec((uint64)kernelvec);

//   struct proc *p = myproc();
  
//   // save user program counter.
//   p->trapframe->epc = r_sepc();
  
//   if(r_scause() == 8){
//     // system call

//     if(p->killed)
//       exit(-1);

//     // sepc points to the ecall instruction,
//     // but we want to return to the next instruction.
//     p->trapframe->epc += 4;

//     // an interrupt will change sstatus &c registers,
//     // so don't enable until done with those registers.
//     intr_on();

//     syscall();
//   } else if((which_dev = devintr()) != 0){
//     // ok
//   } else {
//     printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
//     printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
//     p->killed = 1;
//   }

//   if(p->killed)
//     exit(-1);

//   // give up the CPU if this is a timer interrupt.
//   if(which_dev == 2)
//     yield();

//   usertrapret();

// }

// void
// usertrap(void)
// {
//   int which_dev = 0;

//   if ((r_sstatus() & SSTATUS_SPP) != 0)
//     panic("usertrap: not from user mode");

//   // send interrupts and exceptions to kerneltrap(),
//   // since we're now in the kernel.
//   w_stvec((uint64)kernelvec);

//   struct proc *p = myproc();
  
//   // save user program counter.
//   p->trapframe->epc = r_sepc();
  
//   if (r_scause() == 8) {
//     // system call

//     if (p->killed)
//       exit(-1);

//     // sepc points to the ecall instruction,
//     // but we want to return to the next instruction.
//     p->trapframe->epc += 4;

//     // an interrupt will change sstatus &c registers,
//     // so don't enable until done with those registers.
//     intr_on();

//     syscall();
//   } else if ((which_dev = devintr()) != 0) {
//     // ok
//   } else if (r_scause() == 13 || r_scause() == 15) {
//     // Load or store fault
//     uint64 faulting_addr = r_stval();

//     // Check if the faulting address is within the process's allocated virtual memory
//     if (faulting_addr >= p->sz && faulting_addr < TRAPFRAME) {
//       // Allocate physical memory frame
//       char *mem = kalloc();
//       if (mem == 0) {
//         p->killed = 1;
//       }

//       // Install page table mapping
//       if (mappages(p->pagetable, PGROUNDDOWN(faulting_addr), PGSIZE, (uint64)mem, PTE_W | PTE_X | PTE_R | PTE_U) != 0) {
//         kfree(mem);  // Free allocated memory on failure
//         p->killed = 1;
//       }

//       // Continue the user program
//       return;
//     }
//   } else {
//     printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
//     printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
//     p->killed = 1;
//   }

//   if (p->killed)
//     exit(-1);

//   // give up the CPU if this is a timer interrupt.
//   if (which_dev == 2)
//     yield();

//   usertrapret();
// }

// void
// usertrap(void)
// {
//   int which_dev = 0;
//   uint64 addr;
//   if((r_sstatus() & SSTATUS_SPP) != 0)
//     panic("usertrap: not from user mode");
//   w_stvec((uint64)kernelvec);
//   struct proc *p = myproc();
//   p->trapframe->epc = r_sepc();
//   if(r_scause() == 8){
//     if(p->killed)
//       exit(-1);
//     p->trapframe->epc += 4;
//     intr_on();
//  syscall();
//   } else if((which_dev = devintr()) != 0){
//   } else if(r_scause() == 0xf || r_scause() == 13){
//     addr = r_stval();
//     if(addr < p -> sz){
//       char *mem = kalloc();
//       if (mem == 0) {
//         printf("Out of memory\n");
//         p->killed = 1;
//       } else {
//         memset(mem, 0, PGSIZE);
//         if (mappages(p->pagetable, PGROUNDDOWN(addr), PGSIZE, (uint64)mem, PTE_W | PTE_X | PTE_R | PTE_U) != 0) {
//           printf("Failed to map memory\n");
//           kfree(mem); 
//           p->killed = 1;
//         }
//       }
//     }else{
//       printf("Invalid memory access at address %p\n", addr);
//       p->killed = 1;
//     }
//   }else{
//     printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
//     printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
//     p->killed = 1;
//   }
//   if(p->killed)
//     exit(-1);
//   if(which_dev == 2)
//     yield();

//   usertrapret();
// }


void
usertrap(void)
{
  int which_dev = 0;

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();
  
  // save user program counter.
  p->trapframe->epc = r_sepc();
  
  if(r_scause() == 8){
    // system call

    if(p->killed)
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sstatus &c registers,
    // so don't enable until done with those registers.
    intr_on();

    syscall();
    //Here I start doing Lab 3 Task 2 Code

  } else if(r_scause()==13||r_scause()==15){
     uint64 fault_addr = r_stval();
    for(int i = 0; i < MAX_MMR; i++){
    if(p->mmr[i].valid == 1){
      if((fault_addr >= p->mmr[i].addr) && (fault_addr < p->mmr[i].addr + p->mmr[i].length)){
        if(r_scause() == 13 && (p->mmr[i].prot & PTE_R)){ 
          //Load Faults - Read
          memset(kalloc(), 0, PGSIZE);
          if(mappages(p->pagetable, PGROUNDDOWN(fault_addr), PGSIZE, (uint64)kalloc(), p->mmr[i].prot | PTE_U) < 0){
            p->killed = 1;
            exit(-1);
          }
        }
        if(r_scause() == 15 && (p->mmr[i].prot & PTE_W)){ 
          //Store Faults - Write
          memset(kalloc(), 0, PGSIZE);
          if(mappages(p->pagetable, PGROUNDDOWN(fault_addr), PGSIZE, (uint64)kalloc(), p->mmr[i].prot | PTE_U) < 0){
            p->killed = 1;
            exit(-1);
                                        }
          }
        }
      }
    }
    //Here it finishes

  } else if((which_dev = devintr()) != 0){
    // ok
  } else {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    p->killed = 1;
  }

  if(p->killed)
    exit(-1);

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2){
    p->cputime++; //Christian Gomez: Increment CPU time
    p->tsticks++; //Christian Gomez Task 4
   // yield();

    //Christian Gomez Task 4 -> usertrap() method
   if(p->tsticks >= timeslice(p->priority)){
        if(p->priority == HIGH){
           p->priority = MEDIUM;
           yield();
         }else if(p->priority == MEDIUM){
           p->priority = LOW;
           yield();
         }else{
           p->priority = LOW;
           yield();
         }
    }//if
  }

  usertrapret();
}
//
// return to user space
//
void
usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to trampoline.S
  w_stvec(TRAMPOLINE + (uservec - trampoline));

  // set up trapframe values that uservec will need when
  // the process next re-enters the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.
  
  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 fn = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64,uint64))fn)(TRAPFRAME, satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
clockintr()
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if((scause & 0x8000000000000000L) &&
     (scause & 0xff) == 9){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    } else if(irq){
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq);

    return 1;
  } else if(scause == 0x8000000000000001L){
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if(cpuid() == 0){
      clockintr();
    }
    
    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  } else {
    return 0;
  }
}

