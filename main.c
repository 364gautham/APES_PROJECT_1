/********************************************************************************************************
*
* UNIVERSITY OF COLORADO BOULDER
*
* @file main.c
* @brief logger and interprocess communication
* 
* Several process talks in a synchronised way to achieve a common goal
*
* @author Kiran Hegde and Gautham K A
* @date  3/12/2018
* @tools vim editor
*
********************************************************************************************************/


/********************************************************************************************************
*
* Header Files
*
********************************************************************************************************/

#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include "usrled.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "log.h"
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "temp.h"
#include "light.h"
#include "socket.h"
#include <sys/types.h>
#include <fcntl.h>
#include "project.h"
#include "usrled.h"


/********************************************************************************************************
*
* Global Variables
*
********************************************************************************************************/
mqd_t hbtemp_q, hblight_q, hbsock_q, hblog_q;
pid_t main_pid, sock_pid, log_pid, temp_pid, light_pid;
pthread_t heartbeat_thread;
volatile sig_atomic_t kill_process=0;

/********************************************************************************************************
*
* @name cleanup_process
* @brief destroys all process
*
* This function safely closes all the created process
*
* @param None
*
* @return None
*
********************************************************************************************************/
void cleanup_process()
{
	if(sock_pid)
	{
		kill(sock_pid, SIGINT);
		waitpid(sock_pid, NULL, NULL);
		sock_pid = 0;
	}
	if(temp_pid)
	{
		kill(temp_pid, SIGINT);
		waitpid(temp_pid, NULL, NULL);
		temp_pid =0;
	}
	if(light_pid)
	{
		kill(light_pid, SIGINT);
		waitpid(light_pid, NULL, NULL);
		light_pid=0;	
	}
	usleep(5000);
	if(log_pid)
	{
		kill(log_pid, SIGINT);
		waitpid(log_pid, NULL, NULL);
		log_pid=0;
	}
}

/********************************************************************************************************
*
* @name create_process
* @brief creates all process
*
* This function safely creates process
*
* @param name and arguments
*
* @return pid_t
*
********************************************************************************************************/
pid_t create_process(uint8_t name, char *argv[])
{
	pid_t pid;
	pid = fork();
	if(pid < 0)
	{
		printf("Log Process Creation Failed\n");
		return -1;	
	}
	
	else if(pid == 0)
	{
		if(name == LOG_PROCESS)
		{
			char *args[]={"./log.out", argv[1], "&", NULL};
			execv(args[0], args);
		}
		else if(name==TEMP_PROCESS)
		{
			char *args[]={"./temp.out", "&", NULL};
			execv(args[0], args);
		}
		else if(name==LIGHT_PROCESS)
		{
			char *args[]={"./light.out", "&", NULL};
			execv(args[0], args);
		}
		else if(name==SOCK_PROCESS)
		{
			char *args[]={"./sock.out", "&", NULL};
			execv(args[0], args);
		}
	}
	
	else if(pid>0)
	{
		return pid;
	}
}

/********************************************************************************************************
*
* @name create queue
* @brief creates all queue
*
* This function creates queue and opens it
*
* @param None
*
* @return None
*
********************************************************************************************************/
uint8_t create_queue()
{

	struct mq_attr attr;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = sizeof(uint8_t);

	struct mq_attr attr_log;
	attr_log.mq_maxmsg = 5;
	attr_log.mq_msgsize = sizeof(Logger_t);
	mq_unlink(LOG_QUEUE);
	if((logger_q = mq_open(LOG_QUEUE, O_RDWR | O_CREAT, 0666, &attr_log))==-1)
	{
		perror("LOGGER QUEUE");
		return -1;
	}

	if((hbtemp_q = mq_open(HBTEMP_QUEUE, O_RDWR | O_CREAT, 0666, &attr))==-1)
	{
		perror("HB_TEMP QUEUE");
		return -1;
	}

	if((hblight_q = mq_open(HBLIGHT_QUEUE, O_RDWR | O_CREAT, 0666, &attr))==-1)
	{
		perror("HB_LIGHT QUEUE");
		return -1;
	}
	
	if((hblog_q = mq_open(HBLOG_QUEUE, O_RDWR | O_CREAT, 0666, &attr))==-1)
	{
		perror("HB_LOG QUEUE");
		return -1;
	}

	if((hbsock_q = mq_open(HBSOCK_QUEUE, O_RDWR | O_CREAT, 0666, &attr))==-1)
	{
		perror("HB_SOCK QUEUE");
		return -1;
	}
	return 0;
}


void cleanup_queue()
{
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	mq_close(hbtemp_q);
	mq_unlink(HBTEMP_QUEUE);
	mq_close(hblight_q);
	mq_unlink(HBLIGHT_QUEUE);
	mq_close(hblog_q);
	mq_unlink(HBLOG_QUEUE);
	mq_close(hbsock_q);
	mq_unlink(HBSOCK_QUEUE);
}

/********************************************************************************************************
*
* @name cleanup
* @brief destroys all process, threads, logger queue
*
* This function safely closes all the created process and other resources
*
* @param None
*
* @return None
*
********************************************************************************************************/
void cleanup()
{
	kill_process = 1;
	cleanup_process();
	cleanup_queue();
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	exit(1);
}

/********************************************************************************************************
*
* @name main
* @brief main function
*
* This function creates all other process and receives heartbeat from every other processes
*
* @param None
*
* @return None
*
********************************************************************************************************/
int main(int argc, char *argv[])
{
	uint8_t result, val_send, val_rec, val;
	struct timespec tm;

	main_pid=0; 
	sock_pid=0; 
	log_pid=0; 
	temp_pid=0;
	light_pid=0;

	if(!argv[1])
	{
		printf("No Filename entered \n");
		return -1;
	}	

	if((create_queue())==-1)	return -1;
	
	/* create temp task */
	temp_pid = create_process(TEMP_PROCESS, argv);
	if(temp_pid < 0)
	{
		cleanup_queue();
		cleanup_process();
		identification_led();
		return -1;
	}
	usleep(500);

	/* create light task */
	light_pid = create_process(LIGHT_PROCESS, argv);
	if(light_pid < 0)
	{
		cleanup_queue();
		cleanup_process();
		return -1;
	}

	
	usleep(500);
	/* create log task*/
	log_pid = create_process(LOG_PROCESS, argv);
	if(log_pid < 0)
	{
		cleanup_queue();
		return -1;
	}
	
	usleep(500);
	/*create socket task */
	sock_pid = create_process(SOCK_PROCESS, argv);
	if(sock_pid < 0)
	{
		cleanup_queue();
		cleanup_process();
		return -1;
	}

	LOG(INIT, MAIN_TASK, "Main task Initialised", NULL);
	uint8_t hblog_count=0, hbsock_count=0, hbtemp_count=0, hblight_count=0;
	signal(SIGINT, cleanup);
	sleep(3);	

	/* Till end */
	while(!kill_process)
	{
		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3;
		tm.tv_nsec =0;
		/* request for heartbeat */
		if((mq_timedsend(hbtemp_q, &val_send, sizeof(val_send),0, &tm))==-1)
        	{	
			if((hbtemp_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat from TEMP", NULL);
        	}
		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3;
		tm.tv_nsec = 0; 
		/* wait for heartbeat */
		if((mq_timedreceive(hbtemp_q, &val_rec, sizeof(val_rec), 0, &tm))==-1)
		{
			if((hbtemp_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat from TEMP", NULL);
		}
		else if(val_rec == (val_send+1))
		{
			LOG(HEARTBEAT, TEMP_TASK, "ALIVE: HEARTBEAT FROM TEMP", NULL);
		}
		val_send=0, val_rec=0;	
		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3; 
		tm.tv_nsec = 0;
		if((mq_timedsend(hblight_q, &val_send, sizeof(val_send),0, &tm))==-1)
        	{
			if((hblight_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat for LIGHT LOL", NULL);
        	}

		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3;
		tm.tv_nsec = 0; 
		if((mq_timedreceive(hblight_q, &val_rec, sizeof(val_rec), 0, &tm))==-1)
		{
			if((hblight_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat for LIGHT HEHE", NULL);
		}
		/* check whether the recived value is correct or not */
		else if(val_rec == (val_send+1))
		{
			LOG(HEARTBEAT, LIGHT_TASK, "ALIVE: HEARTBEAT FROM LIGHT", NULL);
		}

		val_send=0, val_rec=0;		
		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3;
		tm.tv_nsec =0;
		if((mq_timedsend(hbsock_q, &val_send, sizeof(val_send),0, &tm))==-1)
        	{
			if((hbsock_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat for SOCK", NULL);
        	}

		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3;
		tm.tv_nsec =0;
		if((mq_receive(hbsock_q, &val_rec, sizeof(val_rec), 0))==-1)
		{
			printf("NOT\n");
			if((hbsock_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat for SOCK", NULL);
		}
		else if(val_rec == (val_send+1))
		{
			printf("HERE\n");
			LOG(HEARTBEAT, SOCK_TASK, "ALIVE: HEARTBEAT FROM SOCKET", NULL);
		}
		
		val_send=0, val_rec=0;		
		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3;
	        tm.tv_nsec = 0;	
		if((mq_timedsend(hblog_q, &val_send, sizeof(val_send),0, &tm))==-1)
        	{
			if((hblog_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat for LOG", NULL);
        	}
		
		clock_gettime(CLOCK_MONOTONIC, &tm);
		tm.tv_sec += 3;
		tm.tv_nsec = 0; 
		if((mq_receive(hblog_q, &val_rec, sizeof(val_rec), 0))==-1)
		{
			if((hblog_count++) > 10)
				LOG(ERROR, MAIN_TASK, "No heartbeat for LOG", NULL);
		}
		else if(val_rec == (val_send+1))
		{
			LOG(HEARTBEAT, LOG_TASK, "ALIVE: HEARTBEAT FROM LOG TASK", NULL);
		}
		val_rec=0;
		val_send=0;
		sleep(1);
	}
	//cleanup_process();
	//cleanup_queue();
	exit(1);	
}

/* Send the informations to LOG queue */
void LOG(uint8_t loglevel, uint8_t log_source, char *msg, float *value)
{
	Logger_t logging;
        logging.timestamp=time(NULL);
        logging.log_level = loglevel;
        logging.log_ID = log_source;
	/* check for null */
	if(value!=NULL)
	logging.value = *value;
        memcpy(logging.message, msg, MSG_SIZE);
        if((mq_send(logger_q, &logging, sizeof(Logger_t),0))==-1)
        {
                printf("cant send message to process1 and returned %d\n", errno);
        }
}

