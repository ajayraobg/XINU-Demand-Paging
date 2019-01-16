#include<xinu.h>



syscall	vfree(
	  char		*ptr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	virtual_memblk	*next, *prev, *block;
	uint32	top;

	mask = disable();
	if ((nbytes == 0) || ((uint32) ptr < (uint32) ((DISK_BEGIN+MAX_SWAP_SIZE+1)*PAGE_SIZE))
			  || ((uint32) ptr > (uint32) ((DISK_BEGIN+MAX_SWAP_SIZE+1+MAX_HEAP_SIZE)*PAGE_SIZE))) {
		restore(mask);
		return SYSERR;
	}

	write_cr3(super_pdbr);

	nbytes = (uint32) roundpage(nbytes);	/* Use virtual_memblk multiples	*/
	block = (struct virtual_memblk *)ptr;

	prev = proctab[currpid].vmemlist;			/* Walk along free list	*/
	next = proctab[currpid].vmemlist->vnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->vnext;
	}

	if (prev == proctab[currpid].vmemlist) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->vlength;
	}

	/* Ensure new block does not overlap previous or next blocks	*/

	if (((prev != proctab[currpid].vmemlist) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		
		write_cr3(proctab[currpid].pdbr);
		restore(mask);
		return SYSERR;
	}

	proctab[currpid].vmemlist->vlength += nbytes;

	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* Coalesce with previous block	*/
		prev->vlength += nbytes;
		block = prev;
	} else {			/* Link into list as new node	*/
		block->vnext = next;
		block->vlength = nbytes;
		prev->vnext = block;
	}

	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->vlength) == (uint32) next) {
		block->vlength += next->vlength;
		block->vnext = next->vnext;
	}

	int i = 0,j=0,k;
	uint32 pages = nbytes/PAGE_SIZE;
	uint32 flag=0;
	pd_t *pde;
	pt_t *pti;

	while(j<pages){
		i=0;
		flag = 0;
		while(i < DISK_SIZE){

		
			if(i < FFS_SIZE){

				if(FFS_region[i].fr_vpno == ((uint32)ptr>>12) && (FFS_region[i].fr_pid == currpid)){			
					FFS_region[i].fr_status 	= FR_UNMAPPED;
					FFS_region[i].fr_pid 		= -1;	
					FFS_region[i].fr_vpno		= 0;
					FFS_region[i].fr_type 		= 0;
					FFS_region[i].fr_dirty		= 0;
					pde = (struct pd_t *)(proctab[currpid].pdbr + ((uint32)ptr>>22));
					pti = (struct pt_t *)(pde->pd_base + (uint32)(((uint32)ptr<<10)>>22));
					pti->pt_pres = 0;
					if(flag == 0)
						flag = 1;
					else if(flag == 1)
						flag = 2;
			
				}
			
				if(Disk_region[i].fr_vpno == ((uint32)ptr>>12) && (Disk_region[i].fr_pid == currpid)){
			
					Disk_region[i].fr_status 	= FR_UNMAPPED;
					Disk_region[i].fr_pid 		= -1;	
					Disk_region[i].fr_vpno		= 0;
					Disk_region[i].fr_type 	= 0;
					Disk_region[i].fr_dirty	= 0;
					pde = (struct pd_t *)(proctab[currpid].pdbr + ((uint32)ptr>>22));
					pti = (struct pt_t *)(pde->pd_base + (uint32)(((uint32)ptr<<10)>>22));
					pti->pt_pres = 0;
					if(flag == 0)
						flag = 1;
					else if(flag == 1)
						flag = 2;
		
				}


			}

			else{

				if(Disk_region[i].fr_vpno == ((uint32)ptr>>12) && (Disk_region[i].fr_pid == currpid)){
			
					Disk_region[i].fr_status 	= FR_UNMAPPED;
					Disk_region[i].fr_pid 		= -1;	
					Disk_region[i].fr_vpno		= 0;
					Disk_region[i].fr_type 	= 0;
					Disk_region[i].fr_dirty	= 0;
					pde = (struct pd_t *)(proctab[currpid].pdbr + ((uint32)ptr>>22));
					pti = (struct pt_t *)(pde->pd_base + (uint32)(((uint32)ptr<<10)>>22));
					pti->pt_pres = 0;
					if(flag == 0)
						flag = 1;
					else if(flag == 1)
						flag = 2;
		
				}

			}

		
			i++;

		}

		ptr+=PAGE_SIZE;
		j++;
		if(flag==1 || flag==2){
			global_heap += 1;
			//*(pages_array + currpid) -= 1;
		}
	}
	//kprintf("Global heap after free %d : Pages freed %d\n",global_heap,pages);
	write_cr3(proctab[currpid].pdbr);
	restore(mask);
	return OK;
}
