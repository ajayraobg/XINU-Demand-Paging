/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	
	
	
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	//kprintf("here to kill %d\n",pid);
	
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}



	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);
	
	write_cr3(super_pdbr);	
	i=0;
	int j=0;
	uint32 count = 0;
	uint32 arr[MAX_HEAP_SIZE];
	while(i < DISK_SIZE){

		if(i<NFRAMES){

			if(PDPT_region[i].fr_pid == pid){
			//kprintf("PDPT frame removed %d\n",i);			
			PDPT_region[i].fr_status 	= FR_UNMAPPED;
			PDPT_region[i].fr_pid 	= -1;	
			PDPT_region[i].fr_vpno	= 0;
			PDPT_region[i].fr_type 	= 0;
			PDPT_region[i].fr_dirty	= 0;
			}

			if(FFS_region[i].fr_pid == pid){
			for(j=0;j<count;j++){
				if(arr[j]==FFS_region[i].fr_vpno)
					break;
			}
			if(j==count){
				arr[j] = FFS_region[i].fr_vpno;
				count++;
			}			
			FFS_region[i].fr_status 	= FR_UNMAPPED;
			FFS_region[i].fr_pid 		= -1;	
			FFS_region[i].fr_vpno		= 0;
			FFS_region[i].fr_type 		= 0;
			FFS_region[i].fr_dirty		= 0;
			}
			
			if(Disk_region[i].fr_pid == pid){
			for(j=0;j<count;j++){
				if(arr[j]==FFS_region[i].fr_vpno)
					break;
			}
			if(j==count){
				arr[j] = FFS_region[i].fr_vpno;
				count++;
			}
			
			Disk_region[i].fr_status 	= FR_UNMAPPED;
			Disk_region[i].fr_pid 		= -1;	
			Disk_region[i].fr_vpno		= 0;
			Disk_region[i].fr_type 	= 0;
			Disk_region[i].fr_dirty	= 0;
			}

		}	
		else if(i >= NFRAMES && i < FFS_SIZE){

			if(FFS_region[i].fr_pid == pid){
			for(j=0;j<count;j++){
				if(arr[j]==FFS_region[i].fr_vpno)
					break;
			}
			if(j==count){
				arr[j] = FFS_region[i].fr_vpno;
				count++;
			}			
			FFS_region[i].fr_status 	= FR_UNMAPPED;
			FFS_region[i].fr_pid 		= -1;	
			FFS_region[i].fr_vpno		= 0;
			FFS_region[i].fr_type 		= 0;
			FFS_region[i].fr_dirty		= 0;
			}
			
			if(Disk_region[i].fr_pid == pid){
			for(j=0;j<count;j++){
				if(arr[j]==FFS_region[i].fr_vpno)
					break;
			}
			if(j==count){
				arr[j] = FFS_region[i].fr_vpno;
				count++;
			}
			
			Disk_region[i].fr_status 	= FR_UNMAPPED;
			Disk_region[i].fr_pid 		= -1;	
			Disk_region[i].fr_vpno		= 0;
			Disk_region[i].fr_type 	= 0;
			Disk_region[i].fr_dirty	= 0;
			}


		}

		else{

			if(Disk_region[i].fr_pid == pid){
			for(j=0;j<count;j++){
				if(arr[j]==FFS_region[i].fr_vpno)
					break;
			}
			if(j==count){
				arr[j] = FFS_region[i].fr_vpno;
				count++;
			}
			
			Disk_region[i].fr_status 	= FR_UNMAPPED;
			Disk_region[i].fr_pid 		= -1;	
			Disk_region[i].fr_vpno		= 0;
			Disk_region[i].fr_type 	= 0;
			Disk_region[i].fr_dirty	= 0;
			}

		}
		i++;

	}
	global_heap += count;

	write_cr3(proctab[currpid].pdbr);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	

	restore(mask);

	return OK;
}
