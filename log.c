
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "log.h"
#include <errno.h>


void LOG(uint8_t loglevel, uint8_t log_source, char *msg, float *value)
{
	sem_wait(log_lock);
        Logger_t logging;
        logging.timestamp=time(NULL);
        logging.log_level = loglevel;
        logging.log_ID = log_source;
	logging.value = value;
        memcpy(logging.message, msg, MSG_SIZE);
        if((mq_send(logger_q, &logging, sizeof(Logger_t),0))==-1)
        {
                printf("cant send message to process1 and returned %d\n", errno);
        }
	sem_post(log_lock);
}

