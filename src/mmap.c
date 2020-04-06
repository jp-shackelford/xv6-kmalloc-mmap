#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

//jps - mmap.c
//  File containing implementation of mmap and munmap
//  for the xv6 kernel memory allocator.

/* mmap creats a new mapping for the calling proc's address space
 * this function will round up to a page aligned address if needed
 * 
 * Inputs:  addr -  suggestion for starting address, mmap will
 *                  round up to page aligned addr if needed
 *          length- length of region to allocate
 *          prot -  protection level on mapped region
 *          flags - info flags for mapped region
 *          fd -    file descriptors
 *          offset- offset for a file-backed allocation     
 * Returns: starting address (page aligned) of mapped region
 */ 
void *mmap(void *addr, uint length, int prot, int flags, 
            int fd, int offset)
{
  // grab the process's mapped regions linked-list
  struct mapped_region *map_ll = myproc()->mapped_regions;
  // create the ll node to hold the new region
  struct mapped_region *new_region = (struct mapped_region*)kmalloc(sizeof(struct mapped_region));

  // length must be greater than 0
  if (length < 1)
  {
    return (void*) -1;  //TODO: free/deallocate?
  }
  void *addr_tip;
  if(addr == 0)
  {
    addr_tip = map_ll->start_addr; //this will be changed in the loop below
  }
  else
  {
    addr_tip = (void*)PGROUNDUP((int)addr);
  }

  if (new_region == 0)
  {
    return (void*) -1;  //TODO: free/deallocate?
  }

  uint oldsz = myproc()->sz;
  uint newsz = PGROUNDUP(myproc()->sz + length);

  newsz = allocuvm(myproc()->pgdir, oldsz, newsz); //Does this zero out?
  if (newsz == 0)
  {
    return (void*) -1; //TODO: free/deallocate?
  }
  myproc()->sz = newsz;

  //walk mapped regions to find location for new region
  if (map_ll != (void*)0)
  {
    while (map_ll->next != (void*)0)
    {
      //check if the address we want is available
      if (map_ll->start_addr == addr_tip)
      {
        addr_tip = (void*)PGROUNDUP((int)addr_tip + map_ll->length);
      }

      map_ll = map_ll->next;
    }
    //end of loop: we know that addr_tip is not currently being used
    //and can use it safely for our new region
    //we also know that map_ll points to the end of our regions 

    new_region->start_addr = addr_tip;
    new_region->length = length;
    new_region->next = (void*)0;

    map_ll->next = new_region;
  }
  else
  {
    new_region->start_addr = addr_tip;
    new_region->length = length;
    new_region->next = (void*)0;

    map_ll = new_region;
  }

  cprintf("%x\n", (int)addr_tip);
  return new_region->start_addr;
}

int munmap(void *addr, uint length)
{
  return 0;
}