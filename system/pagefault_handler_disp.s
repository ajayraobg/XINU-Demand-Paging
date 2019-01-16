#include <icu.s>

			.text
errorcode:		.long 0
			.globl pagefault_handler_disp,errorcode

pagefault_handler_disp:
			popl	errorcode
			pushfl	
			cli
			pushal
			call pagefault_handler
			popal
			popfl
			iret		
