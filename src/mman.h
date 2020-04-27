/*  jps - added mman.h header file
 *  File:           mman.h
 *  Description:    Header file containing flags and protection bits used
 *                  by the mmap() system call.
 */ 
// Protection bits for mmap
#define PROT_WRITE      1

// Flags for mmap
#define MAP_ANONYMOUS   0
#define MAP_FILE        1
