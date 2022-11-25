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

typedef struct 
{
	osMessageQueueId_t queue_ID;
	osStatus_t clientStatus;
	osStatus_t serverStatus;
	int clientNumMessage;
	int clientNumMessagesDropped;
	int serverNumMessage;
	int serverNumMessagesDropped;
	int serverTime;
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


/*
	
	Our client will send a message once every second

*/

int num_pairs = 2; //N
int server_capacity = 10; //K

void client(void* args)
{
	int msg = 0;
	
	((*((loggingStruct *)args)).clientNumMessage) = 0;

	while(1)
	{
		msg++;
		
		/*
			Put a message into the queue. We give it the ID of the queue we are posting to,
			the address of the message, 0 for priority (there is only one message, priority is irrelevant), and
			0 for timeout, meaning "either do this immediately or fail"
		
			The put function does return a osStatus_t variable that tells us if it succeeded, which you should use in your 
			lab to determine if the put was successful (meaning the queue had space for the new message...).   In this example
			we will not do anything with this information.
		*/
		
		osMessageQueueId_t* queue_id = (osMessageQueueId_t *)((*((loggingStruct *)args)).queue_ID);
		((*((loggingStruct *)args)).clientStatus) = osMessageQueuePut(queue_id,&msg,0,0);
		
		if( (int)((*((loggingStruct *)args)).clientStatus) != 0){
			((*((loggingStruct *)args)).clientNumMessagesDropped)++;
		}
		
		((*((loggingStruct *)args)).clientNumMessage)++;
		osDelay(get_random_delay_seconds(9,osKernelGetTickFreq())); //delay for tick frequency number of ticks. This means 1 second. We do not need to yield because
		//of this delay

	}
}

void server(void* args)
{
	int receivedMessage = 0;
	int rand_delay = 0;
	((*((loggingStruct *)args)).serverNumMessage) = 0;
	((*((loggingStruct *)args)).serverTime) = 0;
  ((*((loggingStruct *)args)).serverNumMessagesDropped) = 0;
	
	while(1)
	{
		/*
			Get a message from the queue and store it in receivedMessage. 
			The third parameter is for recording the message priority, which we are ignoring.
			Finally, we tell this thread to waitForever, since its only purpose is to receive and handle
			messages - it won't do anything if there are no messages available, so it might as well wait!
			
			This thread does not use any status information
		*/
		((*((loggingStruct *)args)).serverStatus) = osMessageQueueGet((osMessageQueueId_t)((*((loggingStruct *)args)).queue_ID),&receivedMessage,NULL,osWaitForever);
		if( (int)((*((loggingStruct *)args)).serverStatus) == 0 ){
			((*((loggingStruct *)args)).serverNumMessage)++;
		}
		else{
			((*((loggingStruct *)args)).serverNumMessagesDropped)++;
		}
		rand_delay = get_random_delay_seconds(9,osKernelGetTickFreq());
		osDelay(rand_delay);
		((*((loggingStruct *)args)).serverTime) = (int)((*((loggingStruct *)args)).serverTime) + rand_delay;
		
		//we're just going to print the message to the LEDs, mod 256:
		charToBinLED((unsigned char)(receivedMessage % 256));
		
		//We need to yield because it is possible that this thread wakes and sees a message right away and we aren't
		//using priority in this course
		osThreadYield();
	}
}

void monitor(void* args)
{
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
			
			// num dropped messaged
			num_dropped = (int)((*((loggingStruct *)args)).clientNumMessagesDropped) + (int)((*((loggingStruct *)args)).serverNumMessagesDropped);
			//dropped over sent
			messageLossRatio =  (double)num_dropped / (double)((*((loggingStruct *)args)).clientNumMessage); 
			// sent over time elapsed
			messageArrivalRate = (double)((*((loggingStruct *)args)).clientNumMessage) / (double)timeElapsed;
			// received / serverSleepTime
			averageServiceRate = (double)((*((loggingStruct *)args)).serverNumMessage) / (double)((*((loggingStruct *)args)).serverTime);
			
			q_id = (int)(((   ((loggingStruct *)args)[i]   )).queue_ID);
			client_stat = (((   ((loggingStruct *)args)[i]   )).clientStatus);
			server_stat = (((   ((loggingStruct *)args)[i]   )).serverStatus);
			
			printf("%d,%d,%d,%f,",q_id,client_stat,server_stat,messageLossRatio);
			printf("%f,%f\n",messageArrivalRate,averageServiceRate);
			//printf("%d,%d,%d,%f,%f,%f \n\n",q_id,client_stat,server_stat,messageLossRatio,messageArrivalRate,averageServiceRate); 
		}
		
		//messageLossRatio = droppedMessages / sentMessages;
		//messageArrivalRate = sentMessages / timeElapsed;
		//averageServiceRate = successfulMessages / serverSleepTime;
		
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
