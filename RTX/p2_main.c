#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "GLCD.h"
#include <cmsis_os2.h>


void leds_on(uint32_t led){
	// function individually checks each bit of the uint32 led and turns on the corresponding LED if it is set, otherwise does nothing to that LED
	printf("run leds_on\n");
	if(led&1){
			LPC_GPIO1->FIOSET |= 1<<28;	
	}
	if(led&2){
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
	//function checks input of each joystick direction as well as if joystick or button is pushed and prints result
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
	// function checks if ADC is ready to give output, if so prints ADC output value to console
	if ( (LPC_ADC->ADGDR>>31) ){
		printf("%d\n",(LPC_ADC->ADGDR & 65520)>>4);
		LPC_ADC->ADCR |= (1<<24);
	}
}

// thread functions: 

void read_potentiometer(){
	// thread checks if ADC is ready to give output, if so prints ADC output bits to console
	while(1){
		if ( (LPC_ADC->ADGDR>>31) ){
			printf("%d\n",(LPC_ADC->ADGDR & 65520)>>4);
		  LPC_ADC->ADCR |= (1<<24);
		}
		// yield thread at end of each loop
		osThreadYield();
	}
}

void read_joystick_thread(){
	// thread reads joystick input and prints direction (or lack thereof)
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
		// reads if joystick has been pressed or not and prints results
		if(!((LPC_GPIO1->FIOPIN) & 1<<20)){
			printf("PRESSED\n");
		}
		else{
			printf("NOT PRESSED\n");
		}
		// yield thread at end of each loop
		osThreadYield();
	}
}

void pushbutton_LED(){
	// thread reads whether pushbutton is pressed, and if so toggles LED. 
	while(1){
		printf("in button function\n"); // printf for debugging
		if(!((LPC_GPIO2->FIOPIN) & 1<<10) && (LPC_GPIO1->FIOPIN & (1<<31)) ){
			if(LPC_GPIO1->FIOPIN & (1<<31)){
				LPC_GPIO1->FIOCLR |= 1<<31;
			}
			else{
				LPC_GPIO1->FIOSET |= (1<<31);
			}
		}
		else{
		}
		// yield thread at end of each loop
		osThreadYield();
	}
}


int main(void){
	// test value for bits to LEDs on 
	uint32_t leds_on_val = 123;
	// counter value to lower rate at which joystick is sampled
	uint32_t counter = 0;
	// hello world test string
	unsigned char hello_world[]="hello world";
	
	// printf at start to initialize printf
	printf("initializing printf");
	
	// initialize system and display screen
	SystemInit();
	GLCD_Init();
	
	
	// setting all GPIOs in port 1 to 0 (input)
	LPC_GPIO1->FIODIR |= 0;
	// setting LEDs to be outputs
	LPC_GPIO1->FIODIR |= (1<<28) + (1<<29) + (1<<31);
	// clearing all values (*don't think this does anything currently as ORing with 0 is the same value)
	LPC_GPIO1->FIOCLR |= 0;	
	
	// setting all GPIOs in port 2 to 0 (input)
	LPC_GPIO2->FIODIR |= 0;
	// setting LEDs to be outputs
	LPC_GPIO2->FIODIR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
	// clearing all values (*don't think this does anything currently as ORing with 0 is the same value)
	LPC_GPIO2->FIOCLR |= 0;	

	// enabling ADC
	LPC_PINCON->PINSEL1 &= ~((1<<18) + (1<<19));
	LPC_PINCON->PINSEL1 |= (1<<18);
	
	// powering on ADC
	LPC_SC->PCONP |= (1<<12);
	
	// setting ADC registers
	LPC_ADC->ADCR = (1<<2)|(4<<8)|(1<<21);
	
	// set ADC to read voltage as digital number
	LPC_ADC->ADCR |= (1<<24);

	
	// initialize kernel
	osKernelInitialize();
	
	// initialize the 3 threads (print outputs of osThreadNew as integers for debugging purposes)
	printf("push button: %d\n",(int)osThreadNew(pushbutton_LED,NULL,NULL));
	printf("potentiometer: %d\n",(int)osThreadNew(read_potentiometer,NULL,NULL));
	printf("joystick: %d\n",(int)osThreadNew(read_joystick_thread,NULL,NULL));
	
	// start kernel
	osKernelStart();
	
	// end in an infinite while loop
	while(1);
	
	
	// code for exercise 1:
	/*
	
	leds_on(leds_on_val);
	while(1)
	*/


	// code for exercise 2:
	/*
	while(1){
		if(!(counter++%1000000)){
			read_joystick();
		}
		
	}
	*/
	
	// code for exercise 3:
	/*
	while(1){
		adc_fun();
	}
	*/

	// code for exercise 4:
	/*
	
	GLCD_SetBackColor(0x001F);
	
	GLCD_DisplayString(1, 1, 1, hello_world);
	while(1);
	
	*/
	
}
