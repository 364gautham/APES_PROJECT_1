/********************************************************************************************************
*
* UNIVERSITY OF COLORADO BOULDER
*
* @file logger.c
* @brief logger and interprocess
* 
* log the information to a file 
*
* @author Kiran Hegde and Gautham K A
* @date  3/14/2018
* @tools vim editor
*
********************************************************************************************************/


/********************************************************************************************************
*
* Header Files
*
********************************************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "log.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>
#include <signal.h>
#include "project.h"

/********************************************************************************************************
*
* Global Variables
*
********************************************************************************************************/
volatile sig_atomic_t kill_process=0;
volatile sig_atomic_t kill_thread=0;
FILE *fp_log;
Logger_t log;
uint8_t thread_status;
pthread_t heartbeat_thread;
mqd_t hblog_q;
sem_t *log_lock;

/********************************************************************************************************
*
* @name signal_handler
* @brief handles the signa;
*
* This function safely closes all applications when it receives a SIGINT signal
*
* @param None
*
* @return None
*
********************************************************************************************************/
void signal_handler()
{
	kill_thread =1;
	kill_process =1;
	if(!fp_log)
	{
		fflush(fp_log);
        	fclose(fp_log);
		fp_log = NULL;
	}
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	mq_close(hblog_q);
	mq_unlink(HBLOG_QUEUE);
	pthread_cancel(heartbeat_thread);
	pthread_join(heartbeat_thread, NULL);
	exit(1);
}

/********************************************************************************************************
*
* @name heartbeat_func
* @brief heartbeat thread functions
*
* This function send heartbeat to main periodically
*
* @param None
*
* @return None
*
********************************************************************************************************/
void *heartbeat_func(void *un)
{
	uint8_t val;
	while(!kill_thread)
	{
		if((mq_receive(hblog_q, &val, sizeof(val), 0))==ETIMEDOUT)
        	{
        		LOG(ERROR, MAIN_TASK, "No heartbeat for TEMP", NULL);
        	}
		val = val+1;
		if((mq_send(hblog_q, &val, sizeof(val),0))==-1)
                {
                        printf("cant send message to process1 and returned %d\n", errno);
                }
	}
}

/********************************************************************************************************
*
* @name main
* @brief logging
*
* This function logs the important informations to file
*
* @param args
*
* @return int
*
********************************************************************************************************/
int main(int argc, char *argv[])
{
	uint8_t result;
	signal(SIGINT, signal_handler);
	char *filename = argv[1];
	fp_log = fopen(filename, "wb");
        
	/* open heartbeat queue */
	struct mq_attr attr;
	attr.mq_maxmsg = 5;
        attr.mq_msgsize = sizeof(int);
	if((hblog_q = mq_open(HBLOG_QUEUE, O_RDWR, 0666, &attr))==-1)
        {
                perror("HB_TEMP QUEUE");
                exit(1);
        }
	/* open a thread for heartbeat */
	result = pthread_create(&heartbeat_thread, NULL, heartbeat_func, NULL);
        if(result)
        {
                perror("HEART_BEAT THREAD");
                sem_close(log_lock);
		mq_close(hblog_q);
                exit(1);
        }

        if(!fp_log)
	{
		printf("File can't be opened\n");
                exit(1);
	}        
	fprintf(fp_log, "Timestamp\t");
        fprintf(fp_log, "LOG_LEVEL\t");
        fprintf(fp_log, "LOG_ID\t");
        fprintf(fp_log, "Message\n");
        fflush(fp_log);
        fclose(fp_log);
        fp_log = NULL;

	/* logger message queue */
	struct mq_attr attr_log;
	attr_log.mq_maxmsg = 5;
	attr_log.mq_msgsize = sizeof(Logger_t);

	if((logger_q = mq_open(LOG_QUEUE, O_RDWR, 0666, &attr_log))==-1)
	{
		perror("LOGGER QUEUE");
		exit(1);
	}

	LOG(INIT, LOG_TASK, "Logger Initialised", NULL);

	while(!kill_process)
	{
		/* open a file */
		fp_log = fopen(filename, "a");
        	if(!fp_log)
                	exit(1);
		/* receive info*/
		if((mq_receive(logger_q, &log, sizeof(Logger_t), 0))==-1)
		{
			printf("Din't receive message from process 2 and returned %d\n", errno);
		}
        	fprintf(fp_log, "%lu\t",log.timestamp);
        	fprintf(fp_log, "%u\t\t", log.log_level);
        	fprintf(fp_log, "%u\t", log.log_ID);
		if((log.log_ID == TEMP_TASK) || (log.log_ID == LIGHT_TASK))
		{
			fprintf(fp_log, "%s\t", log.message);
			fprintf(fp_log, "%f\n", (log.value));
		}
		else
		{
			fprintf(fp_log, "%s\n", log.message);
		}
		fflush(fp_log);
		fclose(fp_log);
		fp_log = NULL;
		usleep(50);
	}
	if(!fp_log)
	{
		fflush(fp_log);
        	fclose(fp_log);
		fp_log = NULL;
	}
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	mq_close(hblog_q);
	mq_unlink(HBLOG_QUEUE);
	pthread_join(heartbeat_thread, NULL);
	exit(1);
}

/* Send the informations to LOG queue */
void LOG(uint8_t loglevel, uint8_t log_source, char *msg, float *value)
{
	Logger_t logging;
        logging.timestamp=time(NULL);
        logging.log_level = loglevel;
        logging.log_ID = log_source;
	if(value!=NULL)
	logging.value = *value;
        memcpy(logging.message, msg, MSG_SIZE);
        if((mq_send(logger_q, &logging, sizeof(Logger_t),0))==-1)
        {
                printf("cant send message to process1 and returned %d\n", errno);
        }
}

