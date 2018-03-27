#ifndef _SOCKET_H_

#define _SOCKET_H_

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <mqueue.h>

#define PORT 5000
#define TEMP_SENSOR 1
#define LIGHT_SENSOR 2


mqd_t temp_q, light_q;
 
typedef enum format
{
	KELVIN,
	CELSIUS,
	FARANHEIT,
}Format_t;

typedef struct message
{
	uint16_t source;
	uint16_t format;
	float data;
	float data1;
	uint8_t a;
	uint8_t b;
	float sen[4];
}Msg_t;


typedef struct sensor_sock
{
	float data[4];
	uint8_t a,b;
	uint16_t val1, val2;
}data_t;



void *socket_func(void *args[]);

#endif




/*
typedef struct data
{
	float data[4];
	uint16_t val;
}data_t;

typedef struct dt
{
	uint16_t val;
	uint8_t a,b;
}Light_data;

typedef struct light
{
	float lum;
	uint16_t val1, val2;
}Light_send;
*/

