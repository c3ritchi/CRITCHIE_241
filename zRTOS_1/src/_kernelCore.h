#ifndef _THREADSCORE_H
	#define _THREADSCORE_H

	#define SHPR3 *(uint32_t*)0xE000ED20
	#define ICSR *(uint32_t*)0xE000ED04
	
	void kernelInit(void);
	
	void osSched(void);



#endif
