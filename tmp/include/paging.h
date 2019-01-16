/* paging.h */
#ifndef _PAGING_H_
#define _PAGING_H_

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  unsigned int fm_offset : 12;		/* frame offset			*/
  unsigned int fm_num : 20;		/* frame number			*/
} phy_addr_t;

/* Macros */

#define PAGE_SIZE       4096    /* number of bytes per page		 		 */
#define MAX_HEAP_SIZE   4096    /* max number of frames for virtual heap		 */
#define MAX_SWAP_SIZE   4096    /* size of swap space (in frames) 			 */
#define MAX_FSS_SIZE    2048    /* size of FSS space  (in frames)			 */
#define MAX_PT_SIZE	256	/* size of space used for page tables (in frames)	 */


/**************Added by Ajay********************/

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  //int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;
}frame_map;

#define NFRAMES MAX_PT_SIZE
#define FFS_SIZE MAX_FSS_SIZE
#define DISK_SIZE MAX_SWAP_SIZE
#define FRAME_BEGIN 8192
#define NUM_PAGE_ENTRIES 1024
#define	FFS_BEGIN	(FRAME_BEGIN+MAX_PT_SIZE+1)
#define	DISK_BEGIN	(FFS_BEGIN+MAX_FSS_SIZE+1)

extern int32 get_frame_PDPT_region(int,pid32);
extern syscall initialize_PD(pd_t *);
extern syscall initialize_PT(pt_t *, int);
extern syscall add_directory_entry(pd_t*, uint32, int);
extern syscall add_table_entry(pt_t*, uint32);
extern syscall Frames_init();
extern int32	getframe_from_FFS(uint32);
extern int32	find_in_disk(uint32,pid32);
extern int32	get_frame_from_disk(uint32,pid32);
extern void set_dirtybit(void);
extern void	clear_page(pt_t*);

typedef uint16 status_t;

#define FR_UNMAPPED 0
#define FR_MAPPED 1

#define PAGE_DIR_TYPE 1
#define PAGE_TBL_TYPE 2
#define PAGE_TYPE 3


extern frame_map PDPT_region[NFRAMES];
extern frame_map FFS_region[FFS_SIZE];
extern frame_map Disk_region[DISK_SIZE];
void pfintr();

/***********************************************/

#endif
