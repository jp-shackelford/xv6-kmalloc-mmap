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


/*Testing whether address returned by anonymous mmap is page aligned.*/
int
main(int argc, char *argv[])
{
  int size = 200;
  char *r1 = mmap(0, size, 0/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);

  char *r2 = mmap(0, size, 0/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);

  char *r3 = mmap(0, size, 0/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);

  int rem1 = ((int)r1 % PGSIZE);
  int rem2 = ((int)r2 % PGSIZE);
  int rem3 = ((int)r3 % PGSIZE);

  printf(1, "XV6_TEST_OUTPUT : rem1 = %d rem2 = %d rem3 = %d\n",rem1,rem2,rem3);

  if(rem1 != 0 || rem2 != 0 || rem3 != 0)
  {
    printf(1, "XV6_TEST_OUTPUT : Address returned by mmap should be page aligned\n");
    exit();
  }

  printf(1, "XV6_TEST_OUTPUT : mmap good --> address returned is page aligned\n");

  int rv1 = munmap(r1, size);
  if (rv1 < 0) {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    exit();
  }

  int rv2 = munmap(r2, size);
  if (rv2 < 0) {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    exit();
  }


  int rv3 = munmap(r3, size);
  if (rv3 < 0) {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    exit();
  }
  
  printf(1, "XV6_TEST_OUTPUT : munmap good\n");

  exit();
}
