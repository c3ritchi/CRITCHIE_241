#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "GLCD.h"
#include <cmsis_os2.h>


void leds_on(uint32_t led){
	
	printf("run leds_on\n");
	if(led&1){
			LPC_GPIO1->FIOSET |= 1<<28;	
	}
	if(led&2){
			printf("in if statement\n");
			LPC_GPIO1->FIOSET |= 1<<29;	
	}
	if(led&4){
			LPC_GPIO1->FIOSET |= 1<<31;	
	}
	if(led&8){
			LPC_GPIO2->FIOSET |= 1<<2;	
	}
	if(led&16){
			LPC_GPIO2->FIOSET |= 1<<3;	
	}
	if(led&32){
			LPC_GPIO2->FIOSET |= 1<<4;	
	}
	if(led&64){
			LPC_GPIO2->FIOSET |= 1<<5;	
	}
	if(led&128){
			LPC_GPIO2->FIOSET |= 1<<6;	
	}
	
}


uint8_t read_joystick(){
	if(!((LPC_GPIO1->FIOPIN) & 1<<20)){
		printf("joystick button pressed\n");
	}
	if(!((LPC_GPIO1->FIOPIN) & 1<<23)){
		printf("joystick direction left pressed\n");
	}
	if(!((LPC_GPIO1->FIOPIN) & 1<<24)){
		printf("joystick direction up pressed\n");
	}
	if(!((LPC_GPIO1->FIOPIN) & 1<<25)){
		printf("joystick direction right pressed\n");
	}
	if(!((LPC_GPIO1->FIOPIN) & 1<<26)){
		printf("joystick direction down pressed\n");
	}
	if(!((LPC_GPIO2->FIOPIN) & 1<<10)){
		printf("push button pressed\n");
	}
	return 0;
}

void adc_fun(){
	
	 if ( (LPC_ADC->ADGDR>>31) ){
			printf("%d\n",(LPC_ADC->ADGDR & 65520)>>4);
		  LPC_ADC->ADCR |= (1<<24);
		}
}


void read_potentiometer(){
	while(1){
		if ( (LPC_ADC->ADGDR>>31) ){
			printf("%d\n",(LPC_ADC->ADGDR & 65520)>>4);
		  LPC_ADC->ADCR |= (1<<24);
		}
		osThreadYield();
	}
}

void read_joystick_thread(){
	while(1){
		if(!((LPC_GPIO1->FIOPIN) & 1<<23)){
			printf("LEFT\n");
		}
			else if(!((LPC_GPIO1->FIOPIN) & 1<<24)){
			printf("UP\n");
		}
		else if(!((LPC_GPIO1->FIOPIN) & 1<<25)){
			printf("RIGHT\n");
		}
		else if(!((LPC_GPIO1->FIOPIN) & 1<<26)){
			printf("DOWN\n");
		}
		else{
			printf("NO DIR\n");
		}
		
		if(!((LPC_GPIO1->FIOPIN) & 1<<20)){
			printf("PRESSED\n");
		}
		else{
			printf("NOT PRESSED\n");
		}
		osThreadYield();
	}
}

void pushbutton_LED(){
	while(1){
		printf("in button function\n");
		if(!((LPC_GPIO2->FIOPIN) & 1<<10) && (LPC_GPIO1->FIOPIN & (1<<31)) ){
			printf("LED should be toggled\n");
			if(LPC_GPIO1->FIOPIN & (1<<31)){
				LPC_GPIO1->FIOCLR |= 1<<31;
			}
			else{
				LPC_GPIO1->FIOSET |= (1<<31);
			}
		}
		else{
		}
		osThreadYield();
	}
}


int main(void){
	uint32_t leds_on_val = 123;
	uint32_t counter = 0;
	unsigned char hello_world[]="hello world";
	
	printf("initializing printf");
	
	SystemInit();
	GLCD_Init();
	
	
	LPC_GPIO1->FIODIR |= 0;
	LPC_GPIO1->FIODIR |= (1<<28) + (1<<29) + (1<<31);
	LPC_GPIO1->FIOCLR |= 0;	
	
	LPC_GPIO2->FIODIR |= 0;
	LPC_GPIO2->FIODIR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
	LPC_GPIO2->FIOCLR |= 0;	

	LPC_PINCON->PINSEL1 &= ~((1<<18) + (1<<19));
	LPC_PINCON->PINSEL1 |= (1<<18);
	
	LPC_SC->PCONP |= (1<<12);
	
	LPC_ADC->ADCR = (1<<2)|(4<<8)|(1<<21);
	
	LPC_ADC->ADCR |= (1<<24);

	
	
	osKernelInitialize();
	
	printf("push button: %d\n",(int)osThreadNew(pushbutton_LED,NULL,NULL));
	printf("potentiometer: %d\n",(int)osThreadNew(read_potentiometer,NULL,NULL));
	printf("joystick: %d\n",(int)osThreadNew(read_joystick_thread,NULL,NULL));
	
	
	osKernelStart();
	
	while(1);
	
	/*
	
	leds_on(leds_on_val);
	
	
	
	
	GLCD_SetBackColor(0x001F);
	
	
	GLCD_DisplayString(1, 1, 1, hello_world);
	
	
	while(1){
		if(!(counter++%1000000)){
			read_joystick();
		}
		
		adc_fun();
	}
	*/
}
