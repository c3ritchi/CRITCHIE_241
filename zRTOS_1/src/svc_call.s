	AREA handle_pend,CODE,READONLY  ;ensures following code is in a continuous read only block of memory
	
	GLOBAL PendSV_Handler  ;PendSV_Handler function prototype (PendSV interrupt handler). must be global as it can be used in other files
	
	PRESERVE8  ;states that stack always lays with 8 byte boundaries
	
PendSV_Handler  ;PendSV interrupt handler function body

	MOV LR,#0xFFFFFFFD  ;sets LR to 0xFFFFFFFD, a constant specific to the microcontroller. this switches the microcontroller to thread mode from handler mode, switches to the PSP from the MSP, and returns the program from the interrupt service routine.
	
	BX LR  ;returns to part of code that was interrupted, the address of which was stored in LR.
	
	END  ;end of file