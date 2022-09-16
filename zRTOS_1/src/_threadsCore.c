#include <stdint.h>
#include <LPC17xx.h>

uint32_t* getMSPInitialLocation(void){
	uint32_t* MSPloc = 0x0;
	
	return((uint32_t*)(*MSPloc));

}


uint32_t* getNewThreadStack(uint32_t offset){
	uint32_t newStackLocInt = (uint32_t)(getMSPInitialLocation());
	newStackLocInt -= offset;
	if(((uint32_t)(getMSPInitialLocation())-newStackLocInt) > 0x2000){ // not more than 0x2000 from MSP, need to check if this is what it should be still
		return 0;
	}
	if(newStackLocInt%8 != 0){
		newStackLocInt-=(8-newStackLocInt%8);
	}
		
		
	uint32_t* newStackLoc = (uint32_t*)(newStackLocInt);
	
	return(newStackLoc);
}

void setThreadingWithPSP(uint32_t* threadStack){
	__set_PSP((uint32_t)(threadStack));
	
	uint32_t ctrlReg = __get_CONTROL();
	
	ctrlReg |= 1<<1;
	
	__set_CONTROL(ctrlReg);
	
}
