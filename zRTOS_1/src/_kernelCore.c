
#include "_kernelCore.h"
#include <stdint.h>

void kernelInit(void){
	SHPR3 |= 0xFF << 16;
	}
	
void osSched(void){
	ICSR |= 1<<28;
	__asm("isb");
}
