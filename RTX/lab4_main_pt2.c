#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include "MPU9250.h"
#include "sensor_fusion.h"


uint8_t count = 0;

osMutexId_t input_mutex;
osMutexId_t output_mutex;
	
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
		
		osMutexAcquire(input_mutex, osWaitForever);
		
		MPU9250_read_gyro();
		MPU9250_read_acc();
		if(MPU9250_st_value | (1<<4)){
			MPU9250_read_mag();
		}
	osMutexRelease(input_mutex);
	osThreadYield();	
	}
}

void sensor_fusion_algo(){
	while(1){
		
		osMutexAcquire(input_mutex, osWaitForever);
		osMutexAcquire(output_mutex, osWaitForever);
		
		sensor_fusion_update(MPU9250_gyro_data[0],MPU9250_gyro_data[1],MPU9250_gyro_data[2],MPU9250_accel_data[0],MPU9250_accel_data[1],MPU9250_accel_data[2],MPU9250_mag_data[0],MPU9250_mag_data[1],MPU9250_mag_data[2]);
		
		osMutexRelease(input_mutex);
		osMutexRelease(output_mutex);
		
		osThreadYield();	
	}
}

void print_vals(){
	while(1){
		
		osMutexAcquire(output_mutex, osWaitForever);
		
		printf("%f,%f,%f\n",sensor_fusion_getPitch(),sensor_fusion_getYaw(),sensor_fusion_getRoll());
		
		osMutexRelease(output_mutex);
		
		osThreadYield();	
	}
}


int main(void){
	
	printf("initializing printf");
	
	SystemInit();
	MPU9250_init(1,1);
	sensor_fusion_init();
	
	LPC_GPIO1->FIODIR |= 0;
	LPC_GPIO1->FIODIR |= (1<<28) + (1<<29) + (1<<31);
	LPC_GPIO1->FIOCLR |= 0;	
	
	LPC_GPIO2->FIODIR |= 0;
	LPC_GPIO2->FIODIR |= (1<<2) + (1<<3) + (1<<4) + (1<<5) + (1<<6);
	LPC_GPIO2->FIOCLR |= 0;	
	
	
	input_mutex = osMutexNew(&input_mutex_attr);
	output_mutex = osMutexNew(&output_mutex_attr);
	
	
	osKernelInitialize();
	
	osThreadNew(read_IMU_vals,NULL,NULL);
	osThreadNew(sensor_fusion_algo,NULL,NULL);
	osThreadNew(print_vals,NULL,NULL);

	
	sensor_fusion_begin(512.0f);
	
	osKernelStart();
	
	while(1);
}
