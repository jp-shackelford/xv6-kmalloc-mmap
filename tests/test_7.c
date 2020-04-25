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


/* File Backed mmap test : testing file backed mmap with offset */
int
main(int argc, char *argv[])
{  
    /*
   * Prepare a file which is filled with raw integer values. These
   * integer values are their offsets in the file.
   */
  int rc;
  char tmp_filename[26];
  strcpy(tmp_filename, "test_file.txt");


  // Create file
  int fd = open(tmp_filename, O_WRONLY | O_CREATE);
  if(fd<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : file creation failed %d\n", fd);
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file creation suceeded\n");



  // Write raw integer to the file
  const int map_size = 0x10;
  for (int i = 0; i < map_size * 2; i += sizeof(i))
  {
    uint written = write(fd, &i, sizeof(i));
    printf(1, "XV6_TEST_OUTPUT : writing %d to file\n", i);

    if(written != sizeof(i))
    {
      printf(1, "XV6_TEST_OUTPUT : file write failed\n");
      exit();
    }
  }
  printf(1, "XV6_TEST_OUTPUT : file write suceeded\n");



  // close file
  rc = close(fd);
  if(rc != 0)
  {
    printf(1, "XV6_TEST_OUTPUT : file close failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file close suceeded\n");



  // Open file again
  fd = open(tmp_filename, O_RDWR);
  if(fd<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : file open failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : file open suceeded\n");



  // mmap the file with a offset
  /* A valid mmap call with an offset specified. */
  int file_offset = 0x10;
  char *addr = (char *) mmap(0, map_size, PROT_WRITE, MAP_FILE, fd, file_offset);

  if (addr<=0)
  {
    printf(1, "XV6_TEST_OUTPUT : mmap failed\n");
    exit();
  }
  printf(1, "XV6_TEST_OUTPUT : mmap suceeded\n");



  //Validate the contents of the file from the specified offset.
  for (int i = 0; i < map_size; i += sizeof(i))
  {
    int expected = i + file_offset;
    int actual = *(int *) (addr + i);

    printf(1, "XV6_TEST_OUTPUT : Expected val : %d Actual val : %d\n",expected, actual);

    if(actual != expected)
    {
      printf(1, "XV6_TEST_OUTPUT : file is Incorrectly mapped\n");
      exit();
    }
  }
  printf(1, "XV6_TEST_OUTPUT : file is correctly mapped\n");



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
