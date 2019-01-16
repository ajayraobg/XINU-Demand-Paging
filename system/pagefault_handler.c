#include<xinu.h>

int pagefault_handler(void){
	write_cr3(super_pdbr);

	//kprintf("Error code %d\n",errorcode);
	uint32 virtual_address;
	pd_t *pde;
	pt_t *frame;
	uint32 frame_pt;
	status_t status;
	virtual_address = (uint32)read_cr2();
	uint32 pdi,pti,vpn;
	uint32	frame_obtained;
	vpn = virtual_address >> 12;
	pdi = virtual_address >> 22;
	pde = (pd_t *)proctab[currpid].pdbr;
	//pti = vpn & 1023;
	pti = ((virtual_address<<10)>>22);
	//kprintf("vaddr %x PDI %x PTI %x\n",virtual_address,pdi,pti);
	if((virtual_address< (uint32) ((DISK_BEGIN+MAX_SWAP_SIZE+1)*PAGE_SIZE)) || (virtual_address> (uint32) ((DISK_BEGIN+MAX_SWAP_SIZE+1+MAX_HEAP_SIZE)*PAGE_SIZE))){
		write_cr3(proctab[currpid].pdbr);
		kprintf("Here 1\n");
		//kprintf("VPN = %d\n",vpn);
		kprintf("Illegal access !!!!!\n");
		kill(currpid);
		return SYSERR;
	}
	
	struct virtual_memblk *prev, *curr;
	prev = proctab[currpid].vmemlist;
	curr = prev->vnext;
	while(curr!=NULL){
		if(virtual_address>=(uint32)curr && virtual_address<=(uint32)curr+curr->vlength){
			write_cr3(proctab[currpid].pdbr);
			kprintf("Here 2\n");
			kprintf("Illegal access !!!!!\n");
			kill(currpid);
			return SYSERR;
		
		}
		else
			curr = curr->vnext;


	}
	
		
	
	
	/*Check for Page Directory Entry in the current process page directory*/
	if((pde + pdi)->pd_pres == 0){
		/*If no Page directory entry is present, get a new page table frame and add it to the page directory*/
		
		global_heap -= 1;
		//*(pages_array + currpid) += 1;
		if(global_heap < 0){
			kprintf("Out of Virtual heap\n");
			global_heap = 0;
			//*(pages_array + currpid) -= 1;
			write_cr3(proctab[currpid].pdbr);
			return SYSERR;
		}

		frame_pt = get_frame_PDPT_region(1,currpid);	
		frame = (pt_t*)(frame_pt * PAGE_SIZE);
		//kprintf("Frame: %d Frame addr: %x\n",frame_pt,frame);
		clear_page(frame);
		status = add_directory_entry(pde, frame_pt, pdi);
		//status = initialize_PT(frame,i);	
		if(status == SYSERR){
			kprintf("No way\n");
			write_cr3(proctab[currpid].pdbr);
			global_heap += 1;
			//*(pages_array + currpid) -= 1;
			return SYSERR;
		}
		/*Add FFS frame entry to the new page table page*/
		frame_obtained = getframe_from_FFS(vpn);
		
		if(frame_obtained == -1){
			kprintf("Out of Memory\n");
			global_heap += 1;
			//*(pages_array + currpid) -= 1;
			kill(currpid);
		}
		//update the page table entry with new physical frame address
		add_table_entry((frame+pti),frame_obtained);
		//add_table_entry((frame),frame_obtained);
	
		
	}

	else{
		int32 fr;
		pt_t *table_entry = (pt_t *)(((pde+pdi)->pd_base)*PAGE_SIZE);
		if((table_entry+pti)->pt_pres == 0){
			fr = find_in_disk(vpn,currpid); //find if the virtual address of this process is present in disk
			if(fr==-1){ //if not present get new FFS frame and add it to the page table entry
				//kprintf("Heap 2 %d\n",global_heap);
				global_heap -= 1;
				//*(pages_array + currpid) += 1;
				if(global_heap < 0){
					kprintf("Out of Virtual heap 2\n");
					global_heap = 0;
					//*(pages_array + currpid) -= 1;
					write_cr3(proctab[currpid].pdbr);
					kill(currpid);
					return SYSERR;
				}
				frame_obtained = getframe_from_FFS(vpn);
				if(frame_obtained==-1){
					kprintf("Out of Memory\n");
					global_heap += 1;
					//*(pages_array + currpid) -= 1;
					kill(currpid);
				}				
				add_table_entry((table_entry+pti),frame_obtained);
			}
			else{ //if present in disk, get new frame from FFS and copy contents from disk, add the new FFS entry in page table 
				frame_obtained = getframe_from_FFS(vpn);
				if(frame_obtained==-1){
					kprintf("Out of memory\n");
					kill(currpid);
				}
				bcopy((fr+DISK_BEGIN)*PAGE_SIZE,(frame_obtained)*PAGE_SIZE,PAGE_SIZE);
				add_table_entry((table_entry+pti),frame_obtained);

			}

		}




	}
	
	write_cr3(proctab[currpid].pdbr);
	return OK;


}
