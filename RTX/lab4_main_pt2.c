#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include "MPU9250.h"
#include "sensor_fusion.h"


uint8_t count = 0; // probably should remove

osMutexId_t input_mutex; // mutex for IMU inputs
osMutexId_t output_mutex; // mutex for sensor_fusion outputs
	
const osMutexAttr_t input_mutex_attr = {
	"inputMutex", // human readable mutex name
	0, // attr_bits
	NULL, // memory for control block
	0U // size for control block
};

const osMutexAttr_t output_mutex_attr = {
	"outputMutex", // human readable mutex name
	0, // attr_bits
	NULL, // memory for control block
	0U // size for control block
};
		

void read_IMU_vals(){
	while(1){
		
		osMutexAcquire(input_mutex, osWaitForever); //requesting mutex access for IMU inputs
		
		MPU9250_read_gyro(); // read gyro IMU inputs
		MPU9250_read_acc(); // read accelerometer IMU inputs
		if(MPU9250_st_value | (1<<4)){ // check if magnometer inputs are valid
			MPU9250_read_mag(); // read magnometer IMU inputs
		}
	osMutexRelease(input_mutex); //releasing mutex access for IMU inputs
	osThreadYield(); // yield thread
	}
}

void sensor_fusion_algo(){
	while(1){
		
		osMutexAcquire(input_mutex, osWaitForever); //requesting mutex access for IMU inputs
		osMutexAcquire(output_mutex, osWaitForever); //requesting mutex access for sensorfusion outputs
		
		sensor_fusion_update(MPU9250_gyro_data[0],MPU9250_gyro_data[1],MPU9250_gyro_data[2],MPU9250_accel_data[0],MPU9250_accel_data[1],MPU9250_accel_data[2],MPU9250_mag_data[0],MPU9250_mag_data[1],MPU9250_mag_data[2]); // run sensor fusion algorithm
		
		osMutexRelease(input_mutex); //releasing mutex access for IMU inputs
		osMutexRelease(output_mutex); //releasing mutex access for sensorfusion outputs
		
		osThreadYield(); // yield thread
	}
}

void print_vals(){
	while(1){
		
		osMutexAcquire(output_mutex, osWaitForever); //requesting mutex access for sensorfusion outputs
		
		printf("%f,%f,%f\n",sensor_fusion_getPitch(),sensor_fusion_getYaw(),sensor_fusion_getRoll()); // print results via UART to Brent
		
		osMutexRelease(output_mutex); //releasing mutex access for sensorfusion outputs
		
		osThreadYield(); // yield thread
	}
}


int main(void){
	
	printf("initializing printf");
	
	SystemInit();
	MPU9250_init(1,1);
	sensor_fusion_init();
	
	// set LEDs to outputs and turn all LEDs off
	LPC_GPIO1->FIODIR |= 0;
	LPC_GPIO1->FIODIR |= (1<<28) + (1<<29) + (1<<31);
	LPC_GPIO1->FIOCLR |= 0;	
	
	LPC_GPIO2->FIODIR |= 0;
	LPC_GPIO2->FIODIR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
	LPC_GPIO2->FIOCLR |= 0;	
	
	// create IMU input and sensor_fusion output mutexes
	input_mutex = osMutexNew(&input_mutex_attr);
	output_mutex = osMutexNew(&output_mutex_attr);
	
	osKernelInitialize();
	
	// create threads
	osThreadNew(read_IMU_vals,NULL,NULL);
	osThreadNew(sensor_fusion_algo,NULL,NULL);
	osThreadNew(print_vals,NULL,NULL);

	// start sensor_fusion algo frequency
	sensor_fusion_begin(512.0f);
	
	osKernelStart();
	
	while(1);
}
