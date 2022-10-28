#include <stdint.h>
#include <LPC17xx.h>


//might want to put this in kernelCore.c, if so, redefine here using 'extern' keyword
//Global threads
threadStruct* store_threads[10];
uint32_t num_threads = 0;

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
	if(((uint32_t)(getMSPInitialLocation())-newStackLocInt) > 0x2000){ 
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

//

void createNewThread(void (*input_func)(void * input)){ //accepts function pointer as argument
	num_threads++;
	store_threads[num_threads-1]->TOS = getNewThreadStack(512*num_threads); //what is a reasonable offset, 512?
	store_threads[num_threads-1]->thread_function = input_func; //function is placed in thread
	
	*--(store_threads[num_threads-1]->TOS) = 1<<24; //offset to ensure 8 byte alignment, xPSR
	*--(store_threads[num_threads-1]->TOS) = (uint32_t)(store_threads[num_threads-1]->thread_function); //store PC
	
	
	//the registers are set to values easily identifiable in memory
	
	//LR, R12, R3, R2, R1, R0
	*--(store_threads[num_threads-1]->TOS) = 0x0A;
	*--(store_threads[num_threads-1]->TOS) = 0x0B;
	*--(store_threads[num_threads-1]->TOS) = 0x0C;
	*--(store_threads[num_threads-1]->TOS) = 0x0D;
	*--(store_threads[num_threads-1]->TOS) = 0x0E;
	*--(store_threads[num_threads-1]->TOS) = 0x0F;
	
	//R11, R10, R9, R8, R7, R6, R5, R4
	*--(store_threads[num_threads-1]->TOS) = 0x01;
	*--(store_threads[num_threads-1]->TOS) = 0x02;
	*--(store_threads[num_threads-1]->TOS) = 0x03;
	*--(store_threads[num_threads-1]->TOS) = 0x04;
	*--(store_threads[num_threads-1]->TOS) = 0x05;
	*--(store_threads[num_threads-1]->TOS) = 0x06;
	*--(store_threads[num_threads-1]->TOS) = 0x07;
	*--(store_threads[num_threads-1]->TOS) = 0x08;
		
		
	
}
