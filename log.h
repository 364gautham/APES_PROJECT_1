#ifndef _LOG_H

#define _LOG_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <mqueue.h>


#define MSG_SIZE 60

#define LOG_LOCK "/locklog"


mqd_t logger_q;

typedef enum log_id
{
	TEMP_TASK, 
	LIGHT_TASK,
	LOG_TASK,
	SOCK_TASK,
	MAIN_TASK,
}log_ID;

typedef enum log_lev
{
	INIT,
	HEARTBEAT,
	INFO, 
	ERROR,
	WARNING,
	ALERT,
}log_level;

typedef struct log
{
	time_t timestamp;
	uint8_t log_level;
	uint8_t log_ID;
	char message[MSG_SIZE];
	float value;
}Logger_t;

void *logger_func(void *file);
void LOG(uint8_t loglevel, uint8_t lod_source, char *msg, float *value);

#endif
