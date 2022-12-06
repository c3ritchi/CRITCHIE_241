/*
	Lab 4 sample code. This code shows you the syntax for creating message queues and 
	receiving them, but it does not permit for an arbitrary number of threads of each type -
	that's your job to figure out. It also uses a fixed delay, rather than a random delay, and
	does not generate any statistics.
	
	Use this code as a starting point to learn how to use the RTX features. Feel free to modify
	it and use it as the start of your own main function. 
	
	Copyright Mike Cooper-Stachowsky, 2022, under the GNU General Public License v3.0, 
	available here: https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "uart.h" //for retargeting printf to UART
#include <cmsis_os2.h> //for RTX functions
#include <lpc17xx.h> //for system init and  MMIO
#include <stdio.h> //for printf - this sample code only initializes printf, but you may choose to use it later on, so it's included here
#include "random.h" //we don't need this here, but it's included so you remember to include it yourselves in your lab

//struct to store data such as message status, message counts, time spent for each server-client pair
typedef struct 
{
	osMessageQueueId_t queue_ID; // queue ID
	osStatus_t clientStatus; // status of client (success or failed)
	osStatus_t serverStatus; // status of server (success or failed)
	int clientNumMessage; // number of messages sent by client
	int clientNumMessagesDropped; // number of messages dropped by client
	int serverNumMessage; // number of messages received by server
	int serverNumMessagesDropped; // number of messages dropped by server
	int serverTime; // time elapsed in processing messages (server)
}loggingStruct;


//This will just set the LEDs to the binary representation of a given unsigned char.
//Quite useful for debugging.
void charToBinLED(unsigned char c)
{
		if(c&1)
			LPC_GPIO1->FIOSET |= 1<<28;
		else
			LPC_GPIO1->FIOCLR |= 1<<28;
		if(c&2)
			LPC_GPIO1->FIOSET |= 1<<29;
		else
			LPC_GPIO1->FIOCLR |= 1<<29;
		if(c&4)
			LPC_GPIO1->FIOSET |= 1U<<31;
		else
			LPC_GPIO1->FIOCLR |= 1U<<31;
		if(c&8)
			LPC_GPIO2->FIOSET |= 1<<2;
		else
			LPC_GPIO2->FIOCLR |= 1<<2;
		if(c&16)
			LPC_GPIO2->FIOSET |= 1<<3;
		else
			LPC_GPIO2->FIOCLR |= 1<<3;
		if(c&32)
			LPC_GPIO2->FIOSET |= 1<<4;
		else
			LPC_GPIO2->FIOCLR |= 1<<4;
		if(c&64)
			LPC_GPIO2->FIOSET |= 1<<5;
		else
			LPC_GPIO2->FIOCLR |= 1<<5;
		if(c&128)
			LPC_GPIO2->FIOSET |= 1<<6;
		else
			LPC_GPIO2->FIOCLR |= 1<<6;
}

//set the LED pins to be outputs
void initLEDPins()
{
	//set the LEDs to be outputs. You may or may not care about this
	LPC_GPIO1->FIODIR |= 1<<28; //LED on pin 28
	LPC_GPIO1->FIODIR |= 1<<29;
	LPC_GPIO1->FIODIR |= 1U<<31;
	LPC_GPIO2->FIODIR |= 1<<2;
	LPC_GPIO2->FIODIR |= 1<<3;
	LPC_GPIO2->FIODIR |= 1<<4;
	LPC_GPIO2->FIODIR |= 1<<5;
	LPC_GPIO2->FIODIR |= 1<<6;
}


int num_pairs = 2; //N
int server_capacity = 10; //K

void client(void* args)
{
	// initialize message variable to 0 
	int msg = 0;
	
	// initialize # of send messages to 0 
	((*((loggingStruct *)args)).clientNumMessage) = 0;

	while(1)
	{
		msg++;
		
		// get queue ID from pair's logging structure 
		osMessageQueueId_t* queue_id = (osMessageQueueId_t *)((*((loggingStruct *)args)).queue_ID);
		
		// put message on queue and set clientStatus to return of the Put function
		((*((loggingStruct *)args)).clientStatus) = osMessageQueuePut(queue_id,&msg,0,0);
		
		// if message failed, increment dropped message count
		if( (int)((*((loggingStruct *)args)).clientStatus) != 0){
			((*((loggingStruct *)args)).clientNumMessagesDropped)++;
		}
		
		// increment total message count
		((*((loggingStruct *)args)).clientNumMessage)++;
		
		//delay to simulate random request time
		osDelay(get_random_delay_seconds(9,osKernelGetTickFreq())); 

	}
}

void server(void* args)
{
	// initialize message variable to 0 
	int receivedMessage = 0;
	
	// initialize random delay time variable
	int rand_delay = 0;
	
	// initialize # of received messages, time elapsed, # of dropped messages to 0 
	((*((loggingStruct *)args)).serverNumMessage) = 0;
	((*((loggingStruct *)args)).serverTime) = 0;
	((*((loggingStruct *)args)).serverNumMessagesDropped) = 0;
	
	while(1)
	{
		// receive message from queue and set logging variable for status to output of get function
		((*((loggingStruct *)args)).serverStatus) = osMessageQueueGet((osMessageQueueId_t)((*((loggingStruct *)args)).queue_ID),&receivedMessage,NULL,osWaitForever);
		
		// if message received, increment count for received messages
		if( (int)((*((loggingStruct *)args)).serverStatus) == 0 ){
			((*((loggingStruct *)args)).serverNumMessage)++;
		}
		// if message failed to receive, increment count for dropped messages
		else{
			((*((loggingStruct *)args)).serverNumMessagesDropped)++;
		}
		
		// delay to simulate random processing time
		rand_delay = get_random_delay_seconds(9,osKernelGetTickFreq());
		osDelay(rand_delay);
		
		// add processing time to logged elapsed time
		((*((loggingStruct *)args)).serverTime) = (int)((*((loggingStruct *)args)).serverTime) + rand_delay;
		
		// print the message to the LEDs
		charToBinLED((unsigned char)(receivedMessage % 256));
		
		// yield
		osThreadYield();
	}
}

void monitor(void* args)
{
	// initialize variables used in logging to console
	int timeElapsed = 0;
	
	double messageLossRatio = 0;
	double messageArrivalRate = 0;
	double averageMessageRate = 0;	
	double averageServiceRate = 0;
	
	int q_id;
	osStatus_t client_stat;
	osStatus_t server_stat;
	
	int num_dropped = 0;
	
	while(1){
		for(int i = 0;i<num_pairs;i++){
			
			// number of dropped messages on both client and server ends
			num_dropped = (int)((*((loggingStruct *)args)).clientNumMessagesDropped) + (int)((*((loggingStruct *)args)).serverNumMessagesDropped);
			// ratio of dropped messages vs sent messages
			messageLossRatio =  (double)num_dropped / (double)((*((loggingStruct *)args)).clientNumMessage); 
			// number of sent messages over time elapsed
			messageArrivalRate = (double)((*((loggingStruct *)args)).clientNumMessage) / (double)timeElapsed;
			// number of received messages over server processing time
			averageServiceRate = (double)((*((loggingStruct *)args)).serverNumMessage) / (double)((*((loggingStruct *)args)).serverTime);
			
			// retrieve status and queue ID from logging structure
			q_id = (int)(((   ((loggingStruct *)args)[i]   )).queue_ID);
			client_stat = (((   ((loggingStruct *)args)[i]   )).clientStatus);
			server_stat = (((   ((loggingStruct *)args)[i]   )).serverStatus);
			
			// print results to serial port
			printf("%d,%d,%d,%f,",q_id,client_stat,server_stat,messageLossRatio);
			printf("%f,%f\n",messageArrivalRate,averageServiceRate);
		}
		// increment counter for how much time has elapsed, wait 1s to run again
		timeElapsed++;
		osDelay(osKernelGetTickFreq());
	}
}

int main(void)
{
	//always call this function first
	SystemInit();
	initLEDPins();
	
	//we need to initialize printf outside of any threads
	printf("Project 4 ready\n"); 
	
	//initialize the kernel
	osKernelInitialize();
	
	loggingStruct logging_array[num_pairs];
	
	//initialize our message queue: 10 messages that are integers with default parameters (the "NULL" part means "Let the OS figure out the configuration")
	
	
	int msg = 0;
	
	// set up N client threads and N server threads
	for(int i = 0;i<num_pairs;i++){
		osMessageQueueId_t q_id = osMessageQueueNew(server_capacity,sizeof(int),NULL);
		logging_array[i].queue_ID = q_id;
		
		//set up the threads
		osThreadNew(client,&logging_array[i],NULL);
		osThreadNew(server,&logging_array[i],NULL);
		osThreadNew(monitor,logging_array,NULL); 
	}
	
	
	osKernelStart();
	
	while(1);
}
