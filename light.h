#ifndef light_h
#define light_h

#include "temp.h"

#include <math.h>

#define cmd_control 0x80
#define cmd_timing 0x81
#define cmd_int_cont 0x86
#define cmd_identity 0x8a
#define cmd_int_threslow1 0x82
#define cmd_int_threslow2 0x83
#define cmd_int_threshigh1 0x84
#define cmd_int_threshigh2 0x85
#define cmd_light_data 0xac
#define cmd_light_data1 0xae

#define timing_scale_0 0
#define timing_scale_1 1
#define timing_scale_2 2
#define int_enable 0x10
#define int_disable 0x00
#define int_thres_low 0
#define int_thres_high 1

#define LIGHT_PON 0x3
#define INTEG_SCALE1 0
#define INTEG_SCALE2 1
#define INTEG_SCALE3 2

#define GAIN_INC 1

#define INT_ENABLE 1
#define INT_DISABLE 0

#define LIGHT_SENSOR 2

int hold_light;

/********************************************************************************************************
*
* @name write cmd register
* @brief command reg is used to specify diff light sensor reg
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s write_cmd(int tfd,int val);
/********************************************************************************************************
*
* @name write cmd register
* @brief command reg is used to specify diff light sensor reg
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_cont(int tfd,uint8_t *val);


temp_s write_cont(int tfd,uint8_t val);
/********************************************************************************************************
*
* Control register is used to power up light sensor for use 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s read_tim(int tfd,uint8_t *val);




temp_s write_tim(int tfd,int integ , int gain);
/********************************************************************************************************
*
* wRITE VALUE to timing resister to set gain and integartion time for sensor
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s int_control(int tfd, int cont);




temp_s identity(int tfd,uint8_t *val);
/********************************************************************************************************
*
* @brief identityy is used to identiufy light sensoR DEVICE
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/


temp_s int_thres_read(int tfd, uint16_t * val_low,uint16_t *val_high);
/********************************************************************************************************
*
* @brief interrupt threshold register read
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s int_thres_write(int tfd, uint8_t vallow,uint8_t valhigh,int reg);
/********************************************************************************************************
*
* @brief interrupt threshold register is written approp value
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/

temp_s lum_data(int tfd, float * val);
/********************************************************************************************************
*
* @brief Get Luminosity value
*
* 
*
* @param file descrip , valu for register as address
* @Author Gautham and Kiran
* @return Returns enum Success ot Failure
*
********************************************************************************************************/


/* threads for Light Process task*/
//void *light_task(void *unused);
void *sock_func();
void *heartbeat_func();

#endif

