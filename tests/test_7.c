#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "mmu.h"


/*Stress Testing allocating large sized memory using anonymous mmap spanning across multiple pages..*/
int
main(int argc, char *argv[])
{
  
  int size =  2*PGSIZE;

  char *addr = (char*)0x4000;
  char* r = mmap(addr, size,  0/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);

  if (r<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : mmap failed\n");
    exit();
  }
  
  printf(1, "XV6_TEST_OUTPUT : mmap good\n");

  memset(r, 'a', size - 1);
  printf(1, "XV6_TEST_OUTPUT : strlen(r) = %d\n", strlen(r));

  int rv = munmap(r, size);
  if (rv < 0) 
  {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    exit();
  }

  printf(1, "XV6_TEST_OUTPUT : munmap good\n");
  
  exit();
}
