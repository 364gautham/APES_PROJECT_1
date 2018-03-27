#ifndef LOGGER_H_

#define LOGGER_H_

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <mqueue.h>

#define SEM_LOG "/sem_log"

#define LOG_QUEUE "/proc1"
#define QUEUE_NAME_TEMP "/TEMP_SENSOR"
#define QUEUE_NAME_LIGHT "/LIGHT_SENSOR"


#define MSG_SIZE 30

extern mqd_t logger;

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
	float *value;
}Logger_t;

void *logger_func(void *file);
void LOG(uint8_t loglevel, uint8_t lod_source, char *msg, float *value);


extern volatile sig_atomic_t log_cond_val;
extern pthread_cond_t logcond;
extern pthread_mutex_t log_lock ;


#endif
