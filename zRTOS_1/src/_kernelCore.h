#ifndef _THREADSCORE_H
	#define _THREADSCORE_H

	#define SHPR3 *(uint32_t*)0xE000ED20  //macro for System Handler Priority Register 3's address
	#define ICSR *(uint32_t*)0xE000ED04  //macro for Interrupt Control and State Register's address
	
	void kernelInit(void); //Initializes the kernel (currently just sets PendSV's priority, will be developed further)
	
	void osSched(void); //called by the kernel to schedule threads 


#endif
