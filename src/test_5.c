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


/* File Backed mmap test : Accessing mmap-ed memory region after file close.
The contents should persist.*/

int
main(int argc, char *argv[])
{
  int rc;
  char fileName[50];
  strcpy(fileName, "sample.txt");


  // open existing file
  int fd = open(fileName, O_RDWR);
  if(fd<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : file open failed %d\n", fd);
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file open suceeded\n");



  // Read the file content into buff1
  char buff1[256];
  int sz = read(fd, buff1, 50);
  buff1[sz] = '\0';
  printf(1, "XV6_TEST_OUTPUT : First Read returned : %d\n", sz);
  printf(1, "XV6_TEST_OUTPUT : file content : %s\n", (char*)buff1);



  // close file
  rc = close(fd);
  if(rc != 0)
  {
    printf(1, "XV6_TEST_OUTPUT : file close failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file close suceeded\n");




  // open the file again
  fd = open(fileName, O_RDWR);
  if(fd<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : file open failed %d\n", fd);
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file open suceeded\n");



  // mmap the file
  int file_offset = 0;
  int map_size = 50;
  char *addr = (char *) mmap(0, map_size, PROT_WRITE, MAP_FILE, fd, file_offset);

  if (addr<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : mmap failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : mmap suceeded\n");



  // close file
  rc = close(fd);
  if(rc != 0)
  {
    printf(1, "XV6_TEST_OUTPUT : file close failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file close suceeded\n");



  // Read the content from the mmap-ed memory region
  char buff2[256];
  strcpy(buff2, addr);
  printf(1, "XV6_TEST_OUTPUT : File content from mmap: %s\n", (char*)buff2);

  int ret = strcmp(buff1, buff2);
  printf(1, "XV6_TEST_OUTPUT : Return val of strcmp %d\n", ret);

  if (ret == 0)
  {
    printf(1, "XV6_TEST_OUTPUT : File content and memory mapped content are Same.\n");
  }
  else
  {
    printf(1, "XV6_TEST_OUTPUT : File content and memory mapped content are different.\n");
    exit();
  }



  // munmap
  rc = munmap(addr, map_size);
  if (rc < 0) 
  {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : munmap suceeded\n");


  exit();
}
