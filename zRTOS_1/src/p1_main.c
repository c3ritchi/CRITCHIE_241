//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include <stdio.h>

//this file sets up the UART
#include "uart.h"

//this file sets up sized integers
#include <stdint.h>

#include "_threadsCore.h"
#include "_kernelCore.h"


//This is C. The expected function heading is int main(void)
int main( void ) 
{
	//Sets up various periferals, the clock, etc
	SystemInit();

	//Printf though the UART
	printf("Hello, world!\r\n");
	
	//Retrieving MSP address and printing to UART
	printf("MSP address:%x\r\n",(uint32_t)(getMSPInitialLocation()));
	
	//probably should delete this line. i will go into physical lab to make sure it doesn't cause issues to do so before submitting
	printf("new stack address:%x\r\n",(uint32_t)(getNewThreadStack(512)));
	
	//getting new stack location for PSP
	uint32_t* newStackLoc = getNewThreadStack(512);
	// suggested addition: if(newStackLoc==0){printf("error: new thread stack location out of bounds\r\n");}
	
	//printing new stack location (can delete)
	printf("new stack address:%x\r\n",(uint32_t)newStackLoc);
	
	//setting the PSP location to the newly set stack address
	setThreadingWithPSP(newStackLoc);
	
	//initializing the kernel (sets PendSV priority to weakest) 
	kernelInit();
	
	//sets ICSR register to set PendSV to pending mode (will run when all other interrupts are done) and flushes pipeline
	osSched();
	
	//ending main with an infinite loop
	while(1){
	};
}
