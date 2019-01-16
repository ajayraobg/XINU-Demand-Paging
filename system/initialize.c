/* initialize.c - nulluser, sysinit */

/* Handle system initialization and become the null process */

#include <xinu.h>
#include <string.h>

extern	void	start(void);	/* Start of Xinu code			*/
extern	void	*_end;		/* End of Xinu code			*/

/* Function prototypes */

extern	void main(void);	/* Main is the first process created	*/
static	void sysinit(); 	/* Internal system initialization	*/
extern	void meminit(void);	/* Initializes the free memory list	*/
local	process startup(void);	/* Process to finish startup tasks	*/
extern frame_map PDPT_region[NFRAMES];
unsigned long super_pdbr;
/* Declarations of major kernel variables */

struct	procent	proctab[NPROC];	/* Process table			*/
struct	sentry	semtab[NSEM];	/* Semaphore table			*/
struct	memblk	memlist;	/* List of free memory blocks		*/

/* Active system status */
//uint32	*pages_array;
int	prcount;		/* Total number of live processes	*/
pid32	currpid;		/* ID of currently executing process	*/
int32	global_heap;

//uint32	heap_access[10];

/* Control sequence to reset the console colors and cusor positiion	*/

#define	CONSOLE_RESET	" \033[0m\033[2J\033[;H"

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

void	nulluser()
{	
	struct	memblk	*memptr;	/* Ptr to memory block		*/
	uint32	free_mem;		/* Total amount of free memory	*/
	
	/* Initialize the system */

	sysinit();

	/* Output Xinu memory layout */
	free_mem = 0;
	for (memptr = memlist.mnext; memptr != NULL;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
	for (memptr=memlist.mnext; memptr!=NULL;memptr = memptr->mnext) {
	    kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
	}

	kprintf("%10d bytes of Xinu code.\n",
		(uint32)&etext - (uint32)&text);
	kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Enable interrupts */

	enable();

	/* Initialize the network stack and start processes */

	net_init();

	/******Paging related - Added by Ajay********/




	/********************************************/

	/* Create a process to finish startup and start main */

	resume(create((void *)startup, INITSTK, INITPRIO,
					"Startup process", 0, NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/*  something to run when no other process is ready to execute)	*/

	while (TRUE) {
		//kprintf("null\n");		/* Do nothing */
	}

}


/*------------------------------------------------------------------------
 *
 * startup  -  Finish startup takss that cannot be run from the Null
 *		  process and then create and resume the main process
 *
 *------------------------------------------------------------------------
 */
local process	startup(void)
{
	uint32	ipaddr;			/* Computer's IP address	*/
	char	str[128];		/* String used to format output	*/


	/* Use DHCP to obtain an IP address and format it */

	ipaddr = getlocalip();
	if ((int32)ipaddr == SYSERR) {
		kprintf("Cannot obtain an IP address\n");
	} else {
		/* Print the IP in dotted decimal and hex */
		ipaddr = NetData.ipucast;
		sprintf(str, "%d.%d.%d.%d",
			(ipaddr>>24)&0xff, (ipaddr>>16)&0xff,
			(ipaddr>>8)&0xff,        ipaddr&0xff);
	
		kprintf("Obtained IP address  %s   (0x%08x)\n", str,
								ipaddr);
	}

	/* Create a process to execute function main() */

	resume(create((void *)main, INITSTK, INITPRIO,
					"Main process", 0, NULL));

	/* Startup process exits at this point */

	return OK;
}


/*------------------------------------------------------------------------
 *
 * sysinit  -  Initialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */
static	void	sysinit()
{
	int32	i;
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct	sentry	*semptr;	/* Ptr to semaphore table entry	*/

	/* Reset the console */

	kprintf(CONSOLE_RESET);
	kprintf("\n%s\n\n", VERSION);

	/* Initialize the interrupt vectors */

	initevec();
	
	/* Initialize free memory list */
	
	meminit();

	/* Initialize system variables */

	/* Count the Null process as the first process in the system */

	prcount = 1;

	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	/* Initialize process table entries free */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
		//prptr->accessed_heap_pages = 0;

	}

	//pages_array = (uint32 *)getmem(NPROC*sizeof(uint32));
	//for(i=0;i<NPROC;i++)
		//*(pages_array+i) = 0;
	/* Initialize the Null process entry */	

	prptr = &proctab[NULLPROC];
	prptr->prstate = PR_CURR;
	prptr->prprio = 0;
	strncpy(prptr->prname, "prnull", 7);
	prptr->prstkbase = getstk(NULLSTK);
	prptr->prstklen = NULLSTK;
	prptr->prstkptr = 0;
	currpid = NULLPROC;

	
	/* Initialize semaphores */

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	/* Initialize buffer pools */

	bufinit();

	/* Create a ready list for processes */

	readylist = newqueue();
	Frames_init();

	uint32 super_frame_pd, super_frame_pt;
	pt_t *super_pt;
	status_t status1;

	super_frame_pd = get_frame_PDPT_region(0,-1);

	pd_t *super_pd = (pd_t*)(super_frame_pd * PAGE_SIZE);	

	status1 = initialize_PD(super_pd);
	if(status1 == SYSERR)
		return SYSERR;


	for(i=0; i<20; i++){
		
		super_frame_pt = get_frame_PDPT_region(1,-1);
		//kprintf("PROC %d, name %s,Frame %d\n",currpid,prptr->prname,frame_pt);		
		super_pt = (pt_t*)(super_frame_pt * PAGE_SIZE);
		status1 = add_directory_entry(super_pd, super_frame_pt, i);
		status1 = initialize_PT(super_pt,i);
		//kprintf("PROC %d,Page %d\n",currpid,nullproc_pt->pt_base);
		if(status1 == SYSERR)
			return SYSERR;

	}

	super_pdbr = super_frame_pd* PAGE_SIZE;
	
 	//super_frame_pd = (super_frame_pd >> 12) << 12;
	//write_cr3(super_pdbr);
	//enable_paging();

	/* initialize the PCI bus */

	pci_init();

	/* Initialize the real time clock */

	clkinit();

	for (i = 0; i < NDEVS; i++) {
		init(i);
	}

	//int i;
	uint32 frame_pd, frame_pt;
	pt_t *nullproc_pt;
	status_t status;

	//PDPT_region_init();

	/*for(i = 0;i<15;i++){
		kprintf("%d,%d\n",i,PDPT_region[i].fr_status);


	}*/

	frame_pd = get_frame_PDPT_region(0,NULLPROC);

	pd_t *nullproc_pd = (pd_t*)(frame_pd * PAGE_SIZE);	

	status = initialize_PD(nullproc_pd);
	if(status == SYSERR)
		return SYSERR;


	for(i=0; i<8; i++){
		
		frame_pt = get_frame_PDPT_region(1,NULLPROC);
		//kprintf("PROC %d, name %s,Frame %d\n",currpid,prptr->prname,frame_pt);		
		nullproc_pt = (pt_t*)(frame_pt * PAGE_SIZE);
		status = add_directory_entry(nullproc_pd, frame_pt, i);
		status = initialize_PT(nullproc_pt,i);
		//kprintf("PROC %d,Page %d\n",currpid,nullproc_pt->pt_base);
		if(status == SYSERR)
			return SYSERR;

	}
	/*for(i = 0;i<15;i++){
		kprintf("%d,%d\n",i,PDPT_region[i].fr_status);


	}*/

	unsigned long cr3_word = frame_pd* PAGE_SIZE;
	cr3_word = (cr3_word >> 12) << 12;
	//kprintf("cr3 = %d\n",cr3_word);
	//kprintf("NULL PROC %d\n",nullproc_pd->pd_base);
	
		
	write_cr3(cr3_word);
	global_heap = MAX_HEAP_SIZE;
	prptr->pdbr = cr3_word; 

	set_evec(14, (uint32)pagefault_handler_disp);

	enable_paging();
	return;
}

int32	stop(char *s)
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* Empty */;
}

int32	delay(int n)
{
	DELAY(n);
	return OK;
}
