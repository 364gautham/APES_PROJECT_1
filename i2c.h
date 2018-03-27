#ifndef i2c_h
#define i2c_h


#include<linux/i2c-dev.h>
#include<sys/ioctl.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<stdint.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/syscall.h>

extern sem_t *sem;

/* I2C Functions */

/********************************************************************************************************
*
* @name I2C_bus_open
* @brief I2C_bus_open function
*
* Opens a bus for I2C protocol Communication
* 
*
* @param file descrip
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/
temp_s i2c_bus_open(int *file);

/********************************************************************************************************
*
* @name I2C_bus_INIT function
* @brief :Initialises ioctl i2c communication interface for hardware resource
*
* Opens a bus for I2C protocol Communication
* 
*
* @param file descrip
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/
temp_s i2c_bus_init(int file, int sensor);



/* Read a single byte from Specified address of Sensor */

temp_s i2c_read_byte(int fd , uint8_t *byte);

/* Read a word from Specified address of Sensor */
temp_s i2c_read_word(int fd , uint8_t *byte);

/* Write a single byte from Specified address of Sensor */
temp_s i2c_write_byte(int fd , uint8_t byte);

/* Write a single word from Specified address of Sensor */
temp_s i2c_write_word(int fd , uint8_t *byte);

/* Write a word light from Specified address of Sensor */
temp_s i2c_write_word_light(int fd ,uint8_t * byte );


#endif
