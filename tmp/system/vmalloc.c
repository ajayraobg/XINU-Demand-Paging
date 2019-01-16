#include<xinu.h>


char *vmalloc(uint32 nbytes){


	intmask mask;
	struct	virtual_memblk	*prev, *curr, *leftover;

	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundpage(nbytes);

	if(nbytes > proctab[currpid].hsize*PAGE_SIZE){
		restore(mask);
		return (char *)SYSERR;

	}
	write_cr3(super_pdbr);	

	//nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/

	


	prev = proctab[currpid].vmemlist;
	curr = prev->vnext;
	while (curr != NULL) {			/* Search free list	*/

		if (curr->vlength == nbytes) {	/* Block is exact match	*/
			prev->vnext = curr->vnext;
			proctab[currpid].vmemlist->vlength -= nbytes;
			restore(mask);
			write_cr3(proctab[currpid].pdbr);				
			return (char *)(curr);

		} else if (curr->vlength > nbytes) { /* Split big block	*/
			leftover = (struct virtual_memblk *)((uint32) curr +
					nbytes);
			prev->vnext = leftover;
			leftover->vnext = curr->vnext;
			leftover->vlength = curr->vlength - nbytes;
			proctab[currpid].vmemlist->vlength -= nbytes;
			restore(mask);
			write_cr3(proctab[currpid].pdbr);
			return (char *)(curr);
		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->vnext;
		}
	}

	write_cr3(proctab[currpid].pdbr);	
	restore(mask);
	return (char *)SYSERR;


}
