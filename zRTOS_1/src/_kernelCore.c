
#include "_kernelCore.h"
#include "_threadsCore.h"
#include <stdint.h>

void kernelInit(void){
	//sets PendSV interrupt's priority to the weakest possible value (highest number, FF)
	SHPR3 |= 0xFF << 16;
	}
	
void osSched(void){
	//setting PendSV interrupt to only run when all other interrupts are completed (pending mode) by setting Interrupt Control and State Register's 28th bit to 1
	ICSR |= 1<<28;
	
	//clears the cortex's pipeline to ensure pipelined code doesn't run when it shouldn't
	__asm("isb");
}
