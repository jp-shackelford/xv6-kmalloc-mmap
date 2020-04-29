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
#include "mman.h"

 /* Testing Anonymous mmap that spans across multiple pages*/
int
main(int argc, char *argv[])
{
  int size =  3*PGSIZE;

  // mmap
  char *addr = (char*)0x4000;
  char* r = mmap(addr, size,  PROT_WRITE/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);

  if (r<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : mmap failed\n");
    exit();
  }
  
  printf(1, "XV6_TEST_OUTPUT : mmap good\n");


  // memset and validate
  memset(r, 'a', size - 1);
  printf(1, "XV6_TEST_OUTPUT : strlen(r) = %d\n", strlen(r));

  char* aChars = malloc(size);
  memset(aChars, 'a', size-1);

  int ret = strcmp(r, aChars);
  printf(1, "XV6_TEST_OUTPUT : Return val of strcmp %d\n", ret);

  free(aChars);
  if (ret != 0) {
    printf(1, "XV6_TEST_OUTPUT : value at the newly mapped anonymous memory region is wrong.!!\n");
    exit();
  }
  else
  {
    printf(1, "XV6_TEST_OUTPUT : value at the newly mapped anonymous memory region is correct.!!\n");
  }


  // munmap
  int rv = munmap(r, size);
  if (rv < 0) 
  {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : munmap good\n");

  
  exit();
}
