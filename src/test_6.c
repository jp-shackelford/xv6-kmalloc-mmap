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

/* File Backed mmap test : testing msync */

int PrintFileContents(char* fileName)
{
  int fd = open(fileName, O_RDONLY);
  if(fd<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : file open failed %d inside PrintFileContents function.\n", fd);
    return 0;
  }


  char buff[256];
  int sz = read(fd, buff, 50);
  buff[sz] = '\0';
  
  printf(1, "XV6_TEST_OUTPUT : file content now : < %s >\n", (char*)buff);

  // close file
  int rc = close(fd);
  if(rc != 0)
  {
    printf(1, "XV6_TEST_OUTPUT : file close failed inside PrintFileContents function.\n");
    return 0;
  }

  return 1;
}

int
main(int argc, char *argv[])
{  
  /*
  Print the contents of file
  Open an existing file.
  mmap it.
  write to the memory mapped region.
  msync.
  munmap.
  Check file for newly written content.
  close the file.
  */
  int rc;
  char buff[256];
  char fileName[50]="sample.txt";


  // Print the file contents
  if(!PrintFileContents(fileName))
  {
    printf(1, "XV6_TEST_OUTPUT : Printing file content failed.\n");
    exit();
  }


  // Open file in Read-Write mode
  int fd = open(fileName, O_RDWR);
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



  // Print the mmap-ed region.
  strcpy(buff, addr);
  printf(1, "XV6_TEST_OUTPUT : Before mysnc, content in mmap-ed region: %s\n", buff);



  // write to the file-backed mmap memory region.
  strcpy((char*)addr, "This is overwritten content.!");



  // call msync
  printf(1, "XV6_TEST_OUTPUT : msync return val : %d\n", msync(addr, map_size));



  // Print the mmap-ed region.
  strcpy(buff, addr);
  printf(1, "XV6_TEST_OUTPUT : After mysnc, content in the mmap-ed region : %s\n", buff);



  // Print the file contents
  if(!PrintFileContents(fileName))
  {
    printf(1, "XV6_TEST_OUTPUT : Printing file content failed.\n");
    exit();
  }



  //munmap
  rc = munmap(addr, map_size);
  if (rc < 0) 
  {
    printf(1, "XV6_TEST_OUTPUT : munmap failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : munmap suceeded\n");



  // close file
  rc = close(fd);
  if(rc != 0)
  {
    printf(1, "XV6_TEST_OUTPUT : file close failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file close suceeded\n");



  exit();
}
