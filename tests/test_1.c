#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"


int
main(int argc, char *argv[])
{
  int i;
  for(i=1;i<=500;i++)
  {
    void* addr = kmalloc(2000);

    if(addr == 0)
    {
      printf(1, "XV6_TEST_OUTPUT : kmalloc failed to allocate memory\n");
      exit();
    }

    kmfree(addr);
  }

  printf(1, "XV6_TEST_OUTPUT : kmalloc  and kmfree good.\n");

  exit();
}
