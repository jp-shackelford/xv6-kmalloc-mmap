#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

/*Test simple mmap and munmap.
Test mmap has zeroed the newly mapped anonymous region.*/
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
  int size =  10;
  void *zeroes;

  void* res=0;
  res = mmap(res, size, 0/*prot*/, 0/*flags*/, -1/*fd*/, 0/*offset*/);
  if (res<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : mmap failed\n");
    return;
  }

  printf(1, "XV6_TEST_OUTPUT : mmap good\n");
  zeroes = malloc(size);
  memset(zeroes, 0, size);

  int ret = memcmp(res, zeroes, size);
  printf(1, "XV6_TEST_OUTPUT : Ret of memcmp %d\n", ret);

  if (ret != 0) {
    printf(1, "XV6_TEST_OUTPUT : mmap() should zero out the  newly mapped anonymous memory region.!!\n");
    return;
  }

  printf(1, "XV6_TEST_OUTPUT : mmap() has correctly cleared the newly mapped anonymous memory region\n");

  int rv = munmap(res, size);
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
