#include<xinu.h>
#include<stdlib.h>

frame_map PDPT_region[NFRAMES];
frame_map FFS_region[FFS_SIZE];
frame_map Disk_region[DISK_SIZE];

syscall Frames_init(){

	int i;

	while(i < DISK_SIZE){

		if(i<NFRAMES){

			PDPT_region[i].fr_status 	= FR_UNMAPPED;
			PDPT_region[i].fr_pid 	= -1;	
			PDPT_region[i].fr_vpno	= 0;
			PDPT_region[i].fr_type 	= 0;
			PDPT_region[i].fr_dirty	= 0;

			FFS_region[i].fr_status 	= FR_UNMAPPED;
			FFS_region[i].fr_pid 		= -1;	
			FFS_region[i].fr_vpno		= 0;
			FFS_region[i].fr_type 		= 0;
			FFS_region[i].fr_dirty		= 0;
			
			Disk_region[i].fr_status 	= FR_UNMAPPED;
			Disk_region[i].fr_pid 		= -1;	
			Disk_region[i].fr_vpno		= 0;
			Disk_region[i].fr_type 	= 0;
			Disk_region[i].fr_dirty	= 0;

		}	
		else if(i >= NFRAMES && i < FFS_SIZE){

			FFS_region[i].fr_status 	= FR_UNMAPPED;
			FFS_region[i].fr_pid 		= -1;	
			FFS_region[i].fr_vpno		= 0;
			FFS_region[i].fr_type 		= 0;
			FFS_region[i].fr_dirty		= 0;
			
			Disk_region[i].fr_status 	= FR_UNMAPPED;
			Disk_region[i].fr_pid 		= 0;	
			Disk_region[i].fr_vpno		= 0;
			Disk_region[i].fr_type 	= 0;
			Disk_region[i].fr_dirty	= 0;


		}

		else{

			Disk_region[i].fr_status 	= FR_UNMAPPED;
			Disk_region[i].fr_pid 		= -1;	
			Disk_region[i].fr_vpno		= 0;
			Disk_region[i].fr_type 	= 0;
			Disk_region[i].fr_dirty	= 0;

		}
		i++;

	}

	return OK;
}

int32 get_frame_PDPT_region(int type, pid32 pid){

	int frame_no = 0;

	char *ty;
	if(type)
		ty = "Table";
	else
		ty = "Directory";

	for(frame_no = 0;frame_no < NFRAMES;frame_no++){
		if(PDPT_region[frame_no].fr_status == FR_UNMAPPED){
			PDPT_region[frame_no].fr_status 	= FR_MAPPED;
			PDPT_region[frame_no].fr_pid		= pid;		 	
			PDPT_region[frame_no].fr_vpno		= 0;
			//PDPT_region[frame_no].fr_refcnt	= 1;
			PDPT_region[frame_no].fr_type 	= (!type)?PAGE_DIR_TYPE:PAGE_TBL_TYPE;
			PDPT_region[frame_no].fr_dirty	= 0;
			break;
		} 
	}
	if(frame_no < NFRAMES)
		return frame_no+FRAME_BEGIN;

	else
		return SYSERR;

	
}

syscall initialize_PD(pd_t *proc_pd){

	int i;
	//kprintf("%d\n",NUM_PAGE_ENTRIES);
	for( i =0 ; i < NUM_PAGE_ENTRIES; i++){	
		//kprintf("PD entry %d\n",proc_pd);	
  		proc_pd->pd_pres	= 0;
		//kprintf("After pres\n");
  		//proc_pd->pd_base	= 0;
		//kprintf("After base\n");
  		proc_pd->pd_write	= 1;
		//kprintf("After write\n");
		proc_pd->pd_mbz		= 0;
		//kprintf("After mbz\n");
		proc_pd->pd_avail 	= 0;
		//kprintf("After avail\n");
		proc_pd->pd_global	= 0;
		//kprintf("After global\n");
		proc_pd->pd_acc		= 0;
		//kprintf("After acc\n");
		proc_pd->pd_pwt		= 0;
		//kprintf("After pwt\n");
  		proc_pd->pd_pcd		= 0;
		//kprintf("After pcd\n");
  		proc_pd->pd_fmb		= 0;
		//kprintf("After fmb\n");
		proc_pd++;
		//kprintf("PID -- %d\n",currpid);
	}
	//kprintf("PID %d\n",currpid);
	return OK;
}

syscall add_directory_entry(pd_t *proc_pd, uint32 frame_pt, int pos){

	int i;
  	(proc_pd+pos)->pd_pres		= 1;
	/*
  	//(proc_pd+pos)->pd_write 	= 1;
  	//(proc_pd+pos)->pd_user		= 0;
  	//(proc_pd+pos)->pd_pwt		= 0;
  	//(proc_pd+pos)->pd_pcd		= 0;
  	//(proc_pd+pos)->pd_acc		= 0;
  	//(proc_pd+pos)->pd_mbz		= 0;
  	//(proc_pd+pos)->pd_fmb		= 0;
  	//(proc_pd+pos)->pd_global	= 0;
  	//(proc_pd+pos)->pd_avail 	= 0;
	*/
  	(proc_pd+pos)->pd_base		= frame_pt;	 


	return OK; 
}

syscall initialize_PT(pt_t *proc_pt, int j){

	int i;	
	for(i=0 ; i< NUM_PAGE_ENTRIES ; i++){
		proc_pt->pt_pres	= 1;
		proc_pt->pt_write 	= 1;
		proc_pt->pt_user	= 0;
		proc_pt->pt_pwt		= 0;
		proc_pt->pt_pcd		= 0;
		proc_pt->pt_acc		= 0;
		proc_pt->pt_dirty 	= 0;
		proc_pt->pt_mbz		= 0;
		proc_pt->pt_global	= 0;
		proc_pt->pt_avail 	= 0;
		proc_pt->pt_base	= (j*NUM_PAGE_ENTRIES + i);	

		proc_pt++;
	}
	return OK;

}

syscall	add_table_entry(pt_t *proc_pt, uint32 frame){
	
	proc_pt->pt_pres	= 1;
	proc_pt->pt_write 	= 1;
	proc_pt->pt_user	= 0;
	proc_pt->pt_pwt		= 0;
	proc_pt->pt_pcd		= 0;
	proc_pt->pt_acc		= 0;
	proc_pt->pt_dirty 	= 0;
	proc_pt->pt_mbz		= 0;
	proc_pt->pt_global	= 0;
	proc_pt->pt_avail 	= 0;
	proc_pt->pt_base	= frame;	

	return OK;
}

void clear_page(pt_t *proc_pt){

	int i;
	for(i=0;i<1024;i++){

		proc_pt->pt_pres	= 0;
		proc_pt->pt_write 	= 0;
		proc_pt->pt_user	= 0;
		proc_pt->pt_pwt		= 0;
		proc_pt->pt_pcd		= 0;
		proc_pt->pt_acc		= 0;
		proc_pt->pt_dirty 	= 0;
		proc_pt->pt_mbz		= 0;
		proc_pt->pt_global	= 0;
		proc_pt->pt_avail 	= 0;
		proc_pt->pt_base	= 0;
		proc_pt++;
	}


}

int32 find_in_disk(uint32 vpn,pid32 pid){

	int i;	
	for(i=0;i<DISK_SIZE;i++){
		if((Disk_region[i].fr_pid == pid) && (Disk_region[i].fr_vpno == vpn))
			return i;
	}
	return -1;

}



int32	get_frame_from_disk(uint32 vpn,pid32 pid){


	int frame_no;

	for(frame_no = 0;frame_no < DISK_SIZE;frame_no++){
		if(Disk_region[frame_no].fr_status == FR_UNMAPPED){
			Disk_region[frame_no].fr_status 	= FR_MAPPED;
			Disk_region[frame_no].fr_pid		= pid;		 	
			Disk_region[frame_no].fr_vpno		= vpn;
			//PDPT_region[frame_no].fr_refcnt	= 1;
			Disk_region[frame_no].fr_type 	= PAGE_TYPE;
			Disk_region[frame_no].fr_dirty	= 0;
			break;
		} 
	}

	if(frame_no < DISK_SIZE)
		return(DISK_BEGIN+frame_no);

	else{ //if there are no free frames in disk space
		int frame;
		for(frame_no = 0;frame_no<FFS_SIZE; frame_no++){
			if(FFS_region[frame_no].fr_dirty == 0){ //i check if there are frames that are both in FFS and disk and arent dirty, so that i can remove them from disk
				frame = find_in_disk(FFS_region[frame_no].fr_vpno, FFS_region[frame_no].fr_pid);
				if(frame!=-1){
					Disk_region[frame].fr_status 	= FR_MAPPED;
					Disk_region[frame].fr_pid		= currpid;		 	
					Disk_region[frame].fr_vpno		= vpn;
					Disk_region[frame].fr_type 	= PAGE_TYPE;
					Disk_region[frame].fr_dirty	= 0;
					return(DISK_BEGIN+frame);					
				}

			}


		}		

	}
	return SYSERR;

}

void set_dirtybit(void){

	int frame_no,i;
	for(frame_no = 0;frame_no<NFRAMES;frame_no++){
		if(PDPT_region[frame_no].fr_type == PAGE_TBL_TYPE){
			pt_t *pt = (pt_t *)((frame_no+FRAME_BEGIN)*PAGE_SIZE);
			for(i=0;i<NUM_PAGE_ENTRIES;i++){
				if((pt+i)->pt_dirty == 1 && ((pt+i)->pt_base>=FFS_BEGIN)){
					FFS_region[((pt+i)->pt_base - FFS_BEGIN)].fr_dirty = 1;
					break;
				}
				else if((pt+i)->pt_dirty == 0 && ((pt+i)->pt_base>=FFS_BEGIN)){
					FFS_region[((pt+i)->pt_base - FFS_BEGIN)].fr_dirty = 0;
				}
			}

		}



	}

}



int32	getframe_from_FFS(uint32 vpn){


	int frame_no;

	for(frame_no = 0;frame_no < FFS_SIZE;frame_no++){
		if(FFS_region[frame_no].fr_status == FR_UNMAPPED){
			FFS_region[frame_no].fr_status 	= FR_MAPPED;
			FFS_region[frame_no].fr_pid		= currpid;		 	
			FFS_region[frame_no].fr_vpno		= vpn;
			//PDPT_region[frame_no].fr_refcnt	= 1;
			FFS_region[frame_no].fr_type 	= PAGE_TYPE;
			FFS_region[frame_no].fr_dirty	= 0;
			break;
		} 
	}


	if(frame_no < FFS_SIZE)
		return(FFS_BEGIN+frame_no);
	/*else
		return SYSERR;*/
	/*There are no free FFS frames, so need to evict something*/
	else{
		int32 dframe;		
		uint32 evict_frame = rand()%FFS_SIZE; //generating a random page to be evicted
		//kprintf("Evict frame %d\n",evict_frame);
		//evict_frame += FFS_BEGIN;
		set_dirtybit();	 //i am updating my frame data structure to indicate which pages of page table entries are dirty
		dframe = find_in_disk(FFS_region[evict_frame].fr_vpno, FFS_region[evict_frame].fr_pid); //i am checking if the evicted frame is already in disk
		if(dframe != -1){ //if it is in disk and FFS copy is dirty then i copy the FFScontent to the disk page already present
			//kprintf("Copy \n");
			if(FFS_region[evict_frame].fr_dirty == 1){
				bcopy((evict_frame+FFS_BEGIN)*PAGE_SIZE,(dframe+DISK_BEGIN)*PAGE_SIZE,PAGE_SIZE); 				
			}
		
		}	
		else{ //evicted frame is not in disk, so i find a free frame in disk for the evicted frame
			frame_no = get_frame_from_disk(FFS_region[evict_frame].fr_vpno,FFS_region[evict_frame].fr_pid);
			//kprintf("Disk frame %d\n",frame_no);
			if(frame_no == -1)
				return SYSERR;
		
			bcopy((evict_frame+FFS_BEGIN)*PAGE_SIZE,(frame_no)*PAGE_SIZE,PAGE_SIZE); //copy evicted frame contents to disk frame that is found
		}
		int flag = 0,i;		
		for(frame_no = 0;frame_no<NFRAMES;frame_no++){ //make present bit 0 for the evicted frame
			if((PDPT_region[frame_no].fr_type == PAGE_TBL_TYPE) && (PDPT_region[frame_no].fr_pid == FFS_region[evict_frame].fr_pid)){
				pt_t *pt = (pt_t *)((frame_no+FRAME_BEGIN)*PAGE_SIZE);
				for(i=0;i<NUM_PAGE_ENTRIES;i++){
					if(((pt+i)->pt_base==(evict_frame+FFS_BEGIN)) && ((pt+i)->pt_pres==1)){
						(pt+i)->pt_pres = 0;
						flag = 1;
						break;
					}

				}

			}
			if(flag == 1)
				break;
			
		}	
	
		FFS_region[evict_frame].fr_status 	= FR_MAPPED;
		FFS_region[evict_frame].fr_pid		= currpid;		 	
		FFS_region[evict_frame].fr_vpno		= vpn;
		FFS_region[evict_frame].fr_type 	= PAGE_TYPE;
		FFS_region[evict_frame].fr_dirty	= 0;
		return (FFS_BEGIN+evict_frame);

	}


}






