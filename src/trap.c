#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mman.h"
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
  mmapped_region *mmap_node = curproc->region_head;

  //round the faulting address down to the page start.
  fault_addr = PGROUNDDOWN(fault_addr);

  while(mmap_node)
  {
    //if (fault_addr == (uint)mmap_node->start_addr)
    if ((uint)(mmap_node->start_addr) <= fault_addr && (uint)(mmap_node->start_addr + mmap_node->length) > fault_addr)
    { 
      if ((mmap_node->prot & PROT_WRITE) || !(tf->err & T_ERR_PGFLT_W)) 
      {
        valid = 1;
      }
      break; //leave the loop once we found a valid entry
    }
    else
    {
      mmap_node = mmap_node->next;
    }
  }

  // Map a single page around the faulting address.
  // Allocation based off of allocuvm from vm.c
  if (valid == 1)
  {
    char *mem;
    mem = kalloc();

    if(mem == 0)
    {
      goto error;
    }
    memset(mem, 0, PGSIZE);

    // determine protection bits needed for mappages() call
    int perm;
    if (mmap_node->prot == PROT_WRITE)
    {
      perm = PTE_W|PTE_U; //give write permissions
    }
    else
    {
      perm = PTE_U; //do not give write permissions
    }

    if(mappages(curproc->pgdir, (char*)fault_addr, PGSIZE, V2P(mem), perm) < 0)
    {
      kfree(mem);
      goto error;
    }

    switchuvm(curproc);

    // If we are performing file-backed mmap, seek to where we need to in the
    // file and then read it into the memory location allocated above (mem)
    if (mmap_node->region_type == MAP_FILE)
    {
      if (curproc->ofile[mmap_node->fd])
      {
        fileseek(curproc->ofile[mmap_node->fd], mmap_node->offset);
        fileread(curproc->ofile[mmap_node->fd], mem, mmap_node->length);
        //Clear the dirty bit after read:
          pde_t* pde = &(myproc()->pgdir)[PDX(mmap_node->start_addr)];
          pte_t* pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
          pte_t* pte = &pgtab[PTX(mmap_node->start_addr)];

          *pte &= ~PTE_D;
      }
    }
  }
  else  //Page fault on a non-allocated address
  {
    error:
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
