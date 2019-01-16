# XINU-Demand-Paging
The goal of this project is to implement virtual memory management and demand paging in Xinu.

The project implements the following:
1. pid32 vcreate (void *funcaddr, uint32 ssize, uint32 hsize, pri16 priority, char *name, int nargs, …)
This system call creates a process with a specified heap size. The process’s heap must be private and exist in
the process’s own virtual memory space. Parameter hsize indicates the heap size (in number of pages). Use
a 4KB page size. The total number of pages available for heap is determined by macro MAX_HEAP_SIZE
defined in paging.h. 

2. char* vmalloc (uint32 nbytes)
This function allocates the desired amount of memory (in bytes) off a process’s virtual heap space, and
returns SYSERR if the allocation fails.

3. syscall vfree (char* ptr, uint32 nbytes)
This function frees heap space (previously allocated with vmalloc). vfree returns OK in case of success,
and SYSERR in case of failure. In case of failure, none of the pages involved should be freed.

Additional requirements:
Free Frame Space (FFS)
The FFS is the physical memory space where processes map their virtual heap space. FFS must be released
when no longer required (in other words, heap frames should be released upon heap deallocation). The total
amount of FSS frames available is determined by macro MAX_FSS_SIZE defined in paging.h.
MAX_FSS_SIZE is not a per-process limitation – it indicates the total amount of FSS available to all
processes. When a user process maps a FFS frame to a virtual page, that FFS frame should not be visible to
other user processes (i.e., user processes can map to their virtual address space only the FFS frames they use). 

Disk Space Simulation 
Virtual memory typically uses disk space to extend the physical memory of the machine. However, the
Virtual Box version of Xinu that you are using does not have a file system. Thus, you need to simulate the
disk space using memory. Reserve a total of MAX_SWAP_SIZE frames to emulate the disk space. Macro
MAX_SWAP_SIZE is defined in paging.h. This disk space must not be mapped onto the virtual memory
space of the user processes.

Page directory and Page Tables
Page directories and page tables must be always resident in physical memory (i.e., they should never be
swapped to disk). Physical memory used by page directories and page tables must be released when no longer
required. A total of MAX_PT_SIZE frames can be reserved for page directory and page tables.

Heap allocation
Project uses a lazy allocation policy for heap allocation. That is, the physical space should be reserved not
at allocation time, but when the virtual page is first accessed. Accesses to pages that have not been previously
allocated must cause a SEGMENTATION_FAULT.

Page replacement and swapping
Project uses the random replacement policy. In addition, you must have global replacement (that is, a
process might cause the eviction of pages belonging to other processes). On page eviction, only dirty pages
must be copied to disk. 
