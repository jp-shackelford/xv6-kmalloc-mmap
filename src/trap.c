#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

// jps - pagefault_handler()
/*
 *  This function helps implement the lazy mmap allocation required for this project.
 *  When a pagefault occurs, this function will be called and the mmap linked list
 *  will be searched to see if the faulting address has been decleared as allocated.
 *  If it has been "allocated", the page containing the faulting address will acutally
 *  be allocated to the process.
 * 
 *  INPUT: struct trapframe *tf - the trapframe for a faulting address
 *  OUTPUT: void
 */
void
pagefault_handler(struct trapframe *tf)
{
  struct proc *curproc = myproc();
  uint fault_addr = rcr2(); //Control Register 2, holds the faulting page address.

  // Start -- Required debugging statement -----
  cprintf("============in pagefault_handler============\n");
  cprintf("pid %d %s: trap %d err %d on cpu %d "
  "eip 0x%x addr 0x%x\n",
  curproc->pid, curproc->name, tf->trapno,
  tf->err, cpuid(), tf->eip, fault_addr);
  // End -- Required debugging statement ----

  // Validate that the faulting address belongs to a valid mmap region
  // (check curproc linked list)
  int valid = 0;
  mmapped_region *p = curproc->region_head;

  //round the faulting address down to the page start.
  fault_addr = PGROUNDDOWN(fault_addr);

  while(p)
  {
    if (fault_addr == (uint)p->start_addr)
    {
      valid = 1;
      break; //leave the loop once we found a valid entry
    }
    else
    {
      p = p->next;
    }
  }

  // Map a single page around the faulting address.
  if (valid == 1)
  {
    if(mappages(curproc->pgdir, (void*)fault_addr, PGSIZE, V2P(p->start_addr), PTE_W|PTE_U) < 0)
    {
      cprintf("allocuvm out of memory (2)\n");
      deallocuvm(curproc->pgdir, curproc->sz, curproc->sz - PGSIZE);
      kfree(p->start_addr);
    }
  }
}

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  case T_PGFLT: //jps - added pagefault check in the trap switch statement
    pagefault_handler(tf);
    break;
  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
