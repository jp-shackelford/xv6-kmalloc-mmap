#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

/* Test modification to memory mapped by mmap.*/
int
memcmp(const void *v1, const void *v2, uint n)
{
  const uchar *s1, *s2;
  
  s1 = v1;
  s2 = v2;
  while(n-- > 0)
  {
    if(*s1 != *s2)
     return *s1 - *s2;

    s1++, s2++;
  }

  return 0;
}

void test() {
  int size =  10;  /* we need 10 bytes */
 
  char *addr = (char*)0x4000;
  char* str = mmap(addr, size,  0/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);

  if (str<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : mmap failed\n");
    return;
  }

  printf(1, "XV6_TEST_OUTPUT : mmap good\n");

  printf(1, "XV6_TEST_OUTPUT : Strlen Before modification: %d\n", strlen((char*)str));

  strcpy(str, "012345");

  printf(1, "XV6_TEST_OUTPUT : str = %s\n", (char*)str);
  printf(1, "XV6_TEST_OUTPUT : Strlen After modification: %d\n", strlen((char*)str));

  int rv = munmap(str, size);
  if (rv < 0) {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    return;
  }

  printf(1, "XV6_TEST_OUTPUT : munmap good\n");
  return;
}

int
main(int argc, char *argv[])
{
  test();
  exit();
}
