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
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <signal.h>
#include "log.h"
#include "socket.h"
#include "temp.h"
#include "light.h"
#include <mqueue.h>
#include "project.h"
#include <errno.h>

/********************************************************************************************************
*
* Global Variables
*
********************************************************************************************************/
int server;
uint8_t hold_sock;
int sock;
sig_atomic_t kill_process=0, kill_thread=0;
pthread_t heartbeat;
mqd_t hbsock_q;
data_t temp_sock;
data_t light_sock;

/********************************************************************************************************
*
* @name handler_sock
* @brief signal handler
*
* This function handles the incoming SIGINT signal
*
* @param NOne
*
* @return None
*
********************************************************************************************************/
void handler_sock()
{	
	LOG(INFO, SOCK_TASK, "Closing server task", NULL);
	kill_thread = 1;
	kill_process =1;
	close(sock);
	close(server);
	mq_close(temp_q);
	mq_unlink(TEMP_SOCK_QUEUE);
	mq_close(light_q);
	mq_close(LIGHT_SOCK_QUEUE);
	pthread_cancel(heartbeat);
	pthread_join(heartbeat, NULL);
	exit(1);
}


/********************************************************************************************************
*
* @name heartbeat_sock
* @brief thread function
*
* This function is the thread function which sends heartbeat to main periodically
*
* @param NOne
*
* @return None
*
********************************************************************************************************/
void *heartbeat_sock(void *un)
{
	uint8_t val;

	struct mq_attr attr;
	attr.mq_maxmsg = 5;
        attr.mq_msgsize = sizeof(int);
	if((hbsock_q = mq_open(HBSOCK_QUEUE, O_RDWR, 0666, &attr))==-1)
        {
                perror("HB_TEMP QUEUE");
                exit(1);
        }

	while(!kill_thread)
	{
		if((mq_receive(hbsock_q, &val, sizeof(val), 0))==-1)
        	{
        		LOG(ERROR, MAIN_TASK, "No heartbeat for TEMP", NULL);
        	}
		val = val+1;
		if((mq_send(hbsock_q, &val, sizeof(val),0))==-1)
                {
                        printf("cant send message to process1 and returned %d\n", errno);
                }
		sleep(1);
	}
}

/********************************************************************************************************
*
* @name Main
* @brief Main function
*
* This function is for socket communication to other process
*
* @param NOne
*
* @return None
*
********************************************************************************************************/
void main(void)
{
	sleep(1);
	int read_sock;
	hold_sock = 1;
	struct sockaddr_in address;
	int len = sizeof(address);
	Msg_t msg;
	int option=1;
	signal(SIGINT, handler_sock);
	
	/* attribute for queue and logger_q open */
	struct mq_attr attr_log;
	attr_log.mq_maxmsg = 5;
	attr_log.mq_msgsize = sizeof(Logger_t);

	if((logger_q = mq_open(LOG_QUEUE, O_RDWR, 0666, &attr_log))==-1)
	{
		perror("LOGQUEUE in SOCK");
		return -1;
	}
	
	/* open a server */
	if((server = socket(AF_INET, SOCK_STREAM, 0))==0)
	{
		printf("server creation failed");
		exit(NULL);
	} 

	/* set socket options */
	if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
	{
		printf("Can't set socket");
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	/* bind the socket to port mentioned */
	if((bind(server, (struct sockaddr *)&address, sizeof(address)))<0)
	{
		printf("Port binding failed");
		exit(NULL);
	}
	
	/* queue to talk to temp task */
	struct mq_attr attr_sen;
        attr_sen.mq_maxmsg = 5;
        attr_sen.mq_msgsize = sizeof(data_t);

	if((temp_q = mq_open(TEMP_SOCK_QUEUE, O_RDWR | O_CREAT, 0666, &attr_sen))==-1)
        {
                LOG(ERROR, MAIN_TASK, "Error Opening sock message queue", NULL);
                close(server);
                pthread_exit(NULL);
        }

	/* queue to task to light task*/
	if((light_q = mq_open(LIGHT_SOCK_QUEUE, O_RDWR | O_CREAT, 0666, &attr_sen))==-1)
        {
                LOG(ERROR, MAIN_TASK, "Error Opening sock message queue", NULL);
                close(server);
                pthread_exit(NULL);
        }

	/* create thread for heartbeat */
	int result = pthread_create(&heartbeat, NULL, heartbeat_sock, NULL);
        if(result)
	{
                perror("HEART_BEAT THREAD");
                mq_close(light_q);
		mq_close(temp_q);
		mq_unlink(TEMP_SOCK_QUEUE);
		mq_unlink(LIGHT_SOCK_QUEUE);
		close(server);
                exit(1);
        }
	LOG(INIT, SOCK_TASK, "Sock Task Initialised", NULL);
		
	/* listen for connection */
	if(listen(server, 3) < 0)
	{
		LOG(ERROR, SOCK_TASK, "Can't listen", NULL);
	}

		uint8_t data_send;
	while(!kill_thread)
	{
			/* accept the connection from client */
		sock = accept(server, (struct sockaddr *)&address, (socklen_t *)&len);
		if(sock < 0)
		{
			LOG(ERROR, SOCK_TASK, "Can't accept connection", NULL);
		}
		
		read_sock = read(sock, &msg, sizeof(Msg_t));
		
		/* check message for different information*/
		if(msg.source == TEMP_SENSOR)
		{
			if((msg.format==1))	temp_sock.a=1;
			else if((msg.format==2))	temp_sock.a=2;
			else if((msg.format==3))	temp_sock.a=3;
			else if((msg.format==4))	temp_sock.a=4;	
			else if((msg.format==5))	temp_sock.a=5;
			else if((msg.format==6))	temp_sock.a=6;
			else if((msg.format==7))	temp_sock.a=7;
			else if((msg.format==8))	temp_sock.a=8;
			else if((msg.format==9))	temp_sock.a=9;
			else if((msg.format==10))	temp_sock.a=10;


			/* send it to temp queue */
			if((mq_send(temp_q, &temp_sock, sizeof(data_t),0))==-1)
		        {
                		LOG(ERROR, SOCK_TASK, "temp task comm. failed", NULL);
        		}

			if((mq_receive(temp_q, &temp_sock, sizeof(data_t), 0))==-1)
                	{
				LOG(ERROR, SOCK_TASK, "Error receiving data from temp", NULL);
                	}
			
			msg.sen[0]=temp_sock.data[0];
			msg.sen[1]=temp_sock.data[1];
			msg.sen[2]=temp_sock.data[2];
			msg.sen[3]=temp_sock.data[3];
			msg.format=temp_sock.val1;
			send(sock, &msg, sizeof(Msg_t),0);
		}
		
		if (msg.source == LIGHT_SENSOR)
		{
			if(msg.format==1)	light_sock.a = 1;
			if(msg.format==2)	light_sock.a=2;
			if(msg.format==3)	light_sock.a=3;
			if(msg.format==4)	light_sock.a=4;
			if(msg.format==5)	light_sock.a=5;
			if(msg.format==6)	light_sock.a=6;
			if(msg.format==8)	
			{
				light_sock.val1=8;
				light_sock.a = msg.a;
				light_sock.b = msg.b;
			}
			if(msg.format==9)
			{
				light_sock.val1=9;
				light_sock.a = msg.a;
				light_sock.b = msg.b;
			}
			if(msg.format==7)
			{
				light_sock.a=7;
			}

			/* send it to light queue */
			if((mq_send(light_q, &light_sock, sizeof(data_t),0))==-1)
        		{
				LOG(ERROR, SOCK_TASK, "Light task comm. failed", NULL);
        		}

			if((mq_receive(light_q, &light_sock, sizeof(data_t), 0))==-1)
                	{
				LOG(ERROR, SOCK_TASK, "Error receiving data from light", NULL);
                	}
			
			msg.sen[0]=light_sock.data[0];
			msg.source = light_sock.val1;
			msg.format = light_sock.val2;

			/* send it back to requested process */
			send(sock, &msg, sizeof(Msg_t), 0);
		}
		close(sock);
	}
	/* close connection and server */
	close(sock);
	close(server);
}


/********************************************************************************************************
*
* @name LOG
* @brief logs data
*
* This function logs important data to a file
*
* @param log_level, source, message and any value
*
* @return None
*
********************************************************************************************************/
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


