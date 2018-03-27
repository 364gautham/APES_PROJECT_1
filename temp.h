#ifndef temp_h
#define temp_h


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
#include <semaphore.h>

#define temp_reg 0x00
#define config_reg_val 0x01
#define dev_addr 0x48
#define tlow_reg 0x02
#define thigh_reg 0x03

#define conf_sd_on 0x61A0
#define conf_sd_off 0x60A0

#define SH_ON 1
#define SH_OFF 0

#define EM_SET 1
#define EM_CLEAR 0

#define CONV_025HZ 0
#define CONV_1HZ 1
#define CONV_4HZ 2
#define CONV_6HZ 3

#define TEMP_SENSOR 1



int hold;
typedef enum {
	SUCCESS,
	FAILURE,
}temp_s;



/* Temp Sensor Read and Write Functions */

/********************************************************************************************************
*
* @name pointer reg writing
* @brief it is used to write to pointer register which specifies approp register for sensor use
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s write_ptrreg(int tfd , int regval);


/********************************************************************************************************
*
* @name TLOW reg writing
* @brief Used to read tlow register of temp sensor
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_tlowreg(int tfd, float* byte);

/********************************************************************************************************
*
* @name THIGH reg writing
* @brief Used to read thigh register of temp sensor
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_thighreg(int tfd,float* byte);

/********************************************************************************************************
*
* @name Temperature data reg
* @brief Used to read temperature value in celsius through 12 bits data
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_tempdata(int tfd, float *c ,float *k ,float *f);


/********************************************************************************************************
*
* @name CONFIG register
* @brief Used to read Config reg
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/
temp_s read_config(int tfd, uint16_t *val);


/********************************************************************************************************
*
* @name CONFIG register
* @brief Usd for shutdown modeof sensor
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/


temp_s write_config_sh(int tfd,int val);

/********************************************************************************************************
*
* @name CONFIG register
* @brief Usd for Fault bits of config reg
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_config_fault(int tfd,uint8_t* fval);


/********************************************************************************************************
*
* @name CONFIG register
* @brief Usd for Extended mode of config reg
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_config_em(int tfd , int *val);




temp_s set_config_em(int tfd,int em_bit);

/********************************************************************************************************
*
* @name CONFIG register
* @brief read config register value :16 bits
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_config_conv(int tfd, float *val);

/********************************************************************************************************
*
* @name CONFIG register
* @brief Set Config reg value
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s set_config_conv(int tfd , int conv_val);

//void* tempsen_func();


/* threads used for Temperature sensor Task Process */

void *sock_func();
void *heartbeat_func();

sem_t *sem;


#endif

