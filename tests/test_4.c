#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

/*Stress test : Testing modification to anonymous memory mapped by mmap in a loop.*/
void test() {
  int size =  10;  /* we need 10 bytes */
 
  char *addr = (char*)0x4000;
  char* str = mmap(addr, size,  0/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);

  if (str<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : mmap failed\n");
    return;
  }

  strcpy(str, "012345");

  printf(1, "XV6_TEST_OUTPUT : str = %s\n", (char*)str);

  int rv = munmap(str, size);
  if (rv < 0) {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    return;
  }

  return;
}

int
main(int argc, char *argv[])
{
  int i;

  for(i=1;i<=100;i++)
  {
    test();
  }
  
  exit();
}
