#include <stdint.h>
#include <LPC17xx.h>

uint32_t* getMSPInitialLocation(void){
	//sets MSPloc to a pointer with address 0 (part of vector table, contains location of MSP)
	uint32_t* MSPloc = 0x0;
	
	//returns contents of first vector table value/address 0 which is the location of MSP, cast to unsigned 32 bit integer
	return((uint32_t*)(*MSPloc));

}


uint32_t* getNewThreadStack(uint32_t offset){
	//sets variable that tracks unsigned integer to MSP's address initially
	uint32_t newStackLocInt = (uint32_t)(getMSPInitialLocation());
	
	//subtracts the offset for the new stack from the MSP's address
	newStackLocInt -= offset;
	
	//checks that stack location is within 0x2000 addresses of the MSP to ensure it is not out of range. If not, return false (0) to indicate error
	if(((uint32_t)(getMSPInitialLocation())-newStackLocInt) > 0x2000){ // check if this is right
		return 0;
	}
	//checks that stack location is on 8 byte boundary. If not, lower address further such that it is on an 8 byte boundary
	if(newStackLocInt%8 != 0){
		newStackLocInt-=(8-newStackLocInt%8);
	}
		
	//cast integer storing address to pointer
	uint32_t* newStackLoc = (uint32_t*)(newStackLocInt);
	
	//return the address
	return(newStackLoc);
}

void setThreadingWithPSP(uint32_t* threadStack){
	//sets the address of the PSP to the function input pointer
	__set_PSP((uint32_t)(threadStack));
	
	//retrieves the current state of the control register
	uint32_t ctrlReg = __get_CONTROL();
	
	//clears bit 1 of the control register
	ctrlReg |= 1<<1;
	
	//sets the control register to its original value but with bit 1 cleared
	__set_CONTROL(ctrlReg);
	
}
