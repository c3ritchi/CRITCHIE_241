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
	
	osMutexAcquire(countMutexId, osWaitForever);
	
		LPC_GPIO1->FIOCLR |= (1<<28) + (1<<29) + (1<<31);
		LPC_GPIO2->FIOCLR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
		
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
	osMutexRelease(countMutexId);
	osThreadYield();	
	}
	
}


void wait_for_button(){
	while(1){
	
	if( !((LPC_GPIO2->FIOPIN) & 1<<10 )){
		while( !((LPC_GPIO2->FIOPIN) & 1<<10 ) ){
			osThreadYield();
		}
		osMutexAcquire(countMutexId, osWaitForever);
		count = (count + 1)%256;
		osMutexRelease(countMutexId);
		osDelay(200);
	}
	
	}
	osThreadYield();
}



int main(void){
	
	printf("initializing printf");
	
	SystemInit();
	
	LPC_GPIO1->FIODIR |= 0;
	LPC_GPIO1->FIODIR |= (1<<28) + (1<<29) + (1<<31);
	LPC_GPIO1->FIOCLR |= 0;	
	
	LPC_GPIO2->FIODIR |= 0;
	LPC_GPIO2->FIODIR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
	LPC_GPIO2->FIOCLR |= 0;	
	
	
	countMutexId = osMutexNew(&buttonCountMutexAttr);
	
	
	osKernelInitialize();
	
	osThreadNew(leds_set,NULL,NULL);
	osThreadNew(wait_for_button,NULL,NULL);
	
	osKernelStart();
	
	while(1);
}
