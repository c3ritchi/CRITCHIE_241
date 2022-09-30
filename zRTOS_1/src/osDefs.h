#ifndef OSDEFS_H
	#define OSDEFS_H
	
	#include <stdint.h>
	#include <LPC17xx.h>

	typedef struct{
		void (*thread_function)(void* input);
		uint32_t* TOS;
	
	}threadStruct;


#endif
