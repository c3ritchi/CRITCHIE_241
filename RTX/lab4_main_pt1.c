#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmsis_os2.h>


uint8_t count = 0;

osMutexId_t countMutexId;
	
const osMutexAttr_t buttonCountMutexAttr = {
	"buttonCountMutex", // human readable mutex name
	0, // attr_bits
	NULL, // memory for control block
	0U // size for control block
};
		


void leds_set(){
	while(1){
		printf("count = %d\n",count);
	
	osMutexAcquire(countMutexId, osWaitForever); //requesting mutex access for count
	
	// clear LEDs
	LPC_GPIO1->FIOCLR |= (1<<28) + (1<<29) + (1<<31);
	LPC_GPIO2->FIOCLR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
		
	// turn on LEDs corresponding to count's bits
	if(count&1){
				LPC_GPIO1->FIOSET |= 1<<28;	
		}
	if(count&2){
				LPC_GPIO1->FIOSET |= 1<<29;	
		}
	if(count&4){
				LPC_GPIO1->FIOSET |= 1<<31;	
		}
	if(count&8){
				LPC_GPIO2->FIOSET |= 1<<2;	
		}
	if(count&16){
				LPC_GPIO2->FIOSET |= 1<<3;	
		}
	if(count&32){
				LPC_GPIO2->FIOSET |= 1<<4;	
		}
	if(count&64){
				LPC_GPIO2->FIOSET |= 1<<5;	
		}
	if(count&128){
				LPC_GPIO2->FIOSET |= 1<<6;	
		}
	osMutexRelease(countMutexId); // releasing mutex access for count
	osThreadYield(); // yielding thread
	}
	
}


void wait_for_button(){
	while(1){
	
	if( !((LPC_GPIO2->FIOPIN) & 1<<10 )){ // checking for button press
		while( !((LPC_GPIO2->FIOPIN) & 1<<10 ) ){ // waiting for button to be released
			osThreadYield(); // yield while waiting for button to be released to prevent blocking
		}
		osMutexAcquire(countMutexId, osWaitForever); //requesting mutex access for count
		count = (count + 1)%256; // when all LEDs on (8 bits) reset to 0
		osMutexRelease(countMutexId); //releasing mutex access for count
		osDelay(200); // noted issue with reading multiple inputs if button was pressed briefly, delay added for debouncing
	}
	
	}
	osThreadYield(); // yielding thread
}



int main(void){
	
	printf("initializing printf");
	
	SystemInit();
	
	// set LEDs to outputs and turn all LEDs off
	LPC_GPIO1->FIODIR |= 0;
	LPC_GPIO1->FIODIR |= (1<<28) + (1<<29) + (1<<31);
	LPC_GPIO1->FIOCLR |= 0;	
	
	LPC_GPIO2->FIODIR |= 0;
	LPC_GPIO2->FIODIR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
	LPC_GPIO2->FIOCLR |= 0;	
	
	
	// create mutex for count
	countMutexId = osMutexNew(&buttonCountMutexAttr);
	
	osKernelInitialize();
	
	// create threads
	osThreadNew(leds_set,NULL,NULL);
	osThreadNew(wait_for_button,NULL,NULL);
	
	osKernelStart();
	
	while(1);
}
