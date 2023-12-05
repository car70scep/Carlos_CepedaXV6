#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "stat.h"

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

void kernelvec();

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}


// void
// usertrap(void)
// {
//   int which_dev = 0;

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

//     syscall();
//   } else if((which_dev = devintr()) != 0){
//     // ok

    
//   } else if(r_scause() == 13 || r_scause() == 15){
//       // Check mapped region protection permits operation
//       if(r_stval() >= p->sz){
//         for(int i=0; i<MAX_MMR; i++){
//           if(p->mmr[i].valid && p->mmr[i].addr < r_stval() && p->mmr[i].addr+p->mmr[i].length > r_stval()){
//             // Page fault load
//             if(r_scause() == 13){
//               // Read permission
//               if((p->mmr[i].prot & PROT_READ) == 0){
//                 p->killed = 1;
//                 exit(-1);
//               }
//             }
//             // Page fault store
//             if(r_scause() == 15){
//               // Write permission
//               if((p->mmr[i].prot & PROT_WRITE) == 0){
//                 p->killed = 1;
//                 exit(-1);
//               }
//             }
//           }
//         }
//       }
      
//       void *physical_frame = kalloc();
//       if(physical_frame){
       
//         if(mappages(p->pagetable, PGROUNDDOWN(r_stval()), PGSIZE, (uint64)physical_frame, (PTE_R | PTE_W | PTE_X | PTE_U)) < 0){
//           kfree(physical_frame);
//           p->killed = 1;
//         }
//       }
//   } else {
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

// void usertrap(void) {
//     int which_dev = 0;

//     if ((r_sstatus() & SSTATUS_SPP) != 0)
//         panic("usertrap: not from user mode");

//     w_stvec((uint64)kernelvec);

//     struct proc *p = myproc();

//     p->trapframe->epc = r_sepc();

//     if (r_scause() == 8) {
//         if (p->killed)
//             exit(-1);

//         p->trapframe->epc += 4;

//         intr_on();

//         syscall();
//     } else if ((which_dev = devintr()) != 0) {
//         // ok
//     } else if (r_scause() == 13 || r_scause() == 15) {
//         // Check mapped region protection permits operation
//         if (r_stval() >= p->sz) {
//             for (int i = 0; i < MAX_MMR; i++) {
//                 if (p->mmr[i].valid && p->mmr[i].addr < r_stval() && p->mmr[i].addr + p->mmr[i].length > r_stval()) {
//                     // Page fault load
//                     if (r_scause() == 13) {
//                         // Read permission
//                         if ((p->mmr[i].prot & PROT_READ) == 0) {
//                             p->killed = 1;
//                             exit(-1);
//                         }
//                     }
//                     // Page fault store
//                     if (r_scause() == 15) {
//                         // Write permission
//                         if ((p->mmr[i].prot & PROT_WRITE) == 0) {
//                             p->killed = 1;
//                             exit(-1);
//                         }
//                     }
//                 }
//             }
//         }

//         void *physical_frame = kalloc();
//         if (physical_frame) {
//             if (mappages(p->pagetable, PGROUNDDOWN(r_stval()), PGSIZE, (uint64)physical_frame, (PTE_R | PTE_W | PTE_X | PTE_U)) < 0) {
//                 kfree(physical_frame);
//                 p->killed = 1;
//             }
//         }
//     } else {
//         printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
//         printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
//         p->killed = 1;
//     }

//     if (p->killed)
//         exit(-1);

//     if (which_dev == 2)
//         yield();

//     usertrapret();
// }

void usertrap(void) {
    int which_dev = 0;

    if ((r_sstatus() & SSTATUS_SPP) != 0)
        panic("usertrap: not from user mode");

    w_stvec((uint64)kernelvec);

    struct proc *p = myproc();

    p->trapframe->epc = r_sepc();

    if (r_scause() == 8) {
        if (p->killed)
            exit(-1);

        p->trapframe->epc += 4;

        intr_on();

        syscall();
    } else if ((which_dev = devintr()) != 0) {
        // ok
    } else if (r_scause() == 13 || r_scause() == 15) {
        // Check mapped region protection permits operation
        if (r_stval() >= p->sz) {
            struct mmr_list *mmrlist = get_mmr_list(p->mmr[0].mmr_family.listid); // Assuming mmr_family is in the first entry of the array
            acquire(&mmrlist->lock);
            struct mmr_node *mmr_node = &p->mmr[0].mmr_family;
            while (mmr_node != 0) {
                struct mmr *mmr = container_of(mmr_node, struct mmr, mmr_family);
                if (mmr->valid && mmr->addr < r_stval() && mmr->addr + mmr->length > r_stval()) {
                    // Page fault load
                    if (r_scause() == 13) {
                        // Read permission
                        if ((mmr->prot & PROT_READ) == 0) {
                            release(&mmrlist->lock);
                            p->killed = 1;
                            exit(-1);
                        }
                    }
                    // Page fault store
                    if (r_scause() == 15) {
                        // Write permission
                        if ((mmr->prot & PROT_WRITE) == 0) {
                            release(&mmrlist->lock);
                            p->killed = 1;
                            exit(-1);
                        }
                    }
                }
                mmr_node = mmr_node->next;
            }
            release(&mmrlist->lock);
        }

        void *physical_frame = kalloc();
        if (physical_frame) {
            if (mappages(p->pagetable, PGROUNDDOWN(r_stval()), PGSIZE, (uint64)physical_frame, (PTE_R | PTE_W | PTE_X | PTE_U)) < 0) {
                kfree(physical_frame);
                p->killed = 1;
            }
        }
    } else {
        printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
        printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
        p->killed = 1;
    }

    if (p->killed)
        exit(-1);

    if (which_dev == 2)
        yield();

    usertrapret();
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

