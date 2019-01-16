/* vcreate.c - vcreate */

#include <xinu.h>



/*------------------------------------------------------------------------
 *  create  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32	vcreate(
	  void		*funcaddr,	/* Address of the function	*/
	  uint32	ssize,		/* Stack size in bytes		*/
	  uint32	hsize,		/* Heap size in pages*/
	  pri16		priority,	/* Process priority > 0		*/
	  char		*name,		/* Name (for debugging)		*/
	  uint32	nargs,		/* Number of args that follow	*/
	  ...
	)
{
	


	


	uint32		savsp, *pushsp;
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	int32		i;
	uint32		*a;		/* Points to list of args	*/
	uint32		*saddr;		/* Stack address		*/

	mask = disable();




	
	
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( (priority < 1) || ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR)|| (hsize > MAX_HEAP_SIZE)) {
		restore(mask);
		write_cr3(proctab[currpid].pdbr);
		return SYSERR;
	}


	prcount++;
	prptr = &proctab[pid];
	//kprintf("Creating %d\n",pid);
	

	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	prptr->prprio = priority;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;
	prptr->hsize = hsize;
	//prptr->pages = 0;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	/***************Added by Ajay*************************/
	//kprintf("Super %d\n",super_pdbr);
	write_cr3(super_pdbr);	
	uint32 frame_pd, frame_pt;
	pt_t *nullproc_pt;
	status_t status;
	//kprintf("Yello\n");
	//PDPT_region_init();

	frame_pd = get_frame_PDPT_region(0,pid);
	//kprintf("PD frame for vcreate %d\n",frame_pd);
	//kprintf("Yello1, %d\n",frame_pd);
	pd_t *nullproc_pd = (pd_t*)(frame_pd * PAGE_SIZE);	
	//kprintf("Yello100 %d\n",nullproc_pd);
	status = initialize_PD(nullproc_pd);
	//kprintf("Yello2\n");
	if(status == SYSERR){
		write_cr3(proctab[currpid].pdbr);
		return SYSERR;
	}

	/*for(i = 0;i<10;i++){
		kprintf("%d,%d\n",i,PDPT_region[i].fr_status);


	}*/


	for(i=0; i<8; i++){
		
		frame_pt = get_frame_PDPT_region(1,pid);
		//kprintf("PT frame for vcreate %d\n",frame_pt);
		//kprintf("PROC %d, name %s,Frame %d\n",pid,prptr->prname,frame_pt);	
		nullproc_pt = (pt_t*)(frame_pt * PAGE_SIZE);
		clear_page(nullproc_pt);	
		status = add_directory_entry(nullproc_pd, frame_pt, i);
		status = initialize_PT(nullproc_pt,i);
		//kprintf("PROC %d,Page %d\n",pid,nullproc_pt->pt_base);	
		if(status == SYSERR){
			write_cr3(proctab[currpid].pdbr);
			return SYSERR;
		}
	}
	/*for(i = 0;i<10;i++){
		kprintf("%d,%d\n",i,PDPT_region[i].fr_status);


	}*/

	//kprintf("PROC %d,Page %d\n",pid,nullproc_pd->pd_base);

	unsigned long cr3_word = frame_pd* PAGE_SIZE;
	//cr3_word = (cr3_word >> 12) << 12;	
	//write_cr3(cr3_word);

	proctab[pid].pdbr = cr3_word; 
	

	struct virtual_memblk blk;
	//struct virtual_memblk nxt;
	proctab[pid].vmemlist = getmem(sizeof(struct virtual_memblk));
	proctab[pid].vmemlist->vlength = PAGE_SIZE*hsize;
	proctab[pid].vmemlist->vnext = (struct virtual_memblk *)roundpage(PAGE_SIZE*(DISK_BEGIN+MAX_SWAP_SIZE+1));
	proctab[pid].vmemlist->vnext->vlength = PAGE_SIZE*hsize;
	proctab[pid].vmemlist->vnext->vnext = NULL;

	write_cr3(proctab[currpid].pdbr);

	/*****************************************************/

	/* Initialize stack as if the process was called		*/

	*saddr = STACKMAGIC;
	savsp = (uint32)saddr;

	/* Push arguments */
	a = (uint32 *)(&nargs + 1);	/* Start of args		*/
	a += nargs -1;			/* Last argument		*/
	for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
		*--saddr = *a--;	/* onto created process's stack	*/
	*--saddr = (long)INITRET;	/* Push on return address	*/

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registers, and an old SP	*/

	*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	*--saddr = savsp;		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

	*--saddr = 0;			/* %eax */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);

		
	restore(mask);

	//kprintf("%d \n",end-start);
	return pid;
}
