
#include "light.h"
#include "log.h"
#include "i2c.h"
#include "project.h"
#include "errno.h"
#include "socket.h"
#include "usrled.h"

int tsen_fd;
sem_t *sem;
mqd_t mq_sock_light,mq_heart, logger_q;
pthread_t heart_thread, sock_thread;
volatile sig_atomic_t kill_process=0, kill_hbthread=0;


temp_s write_cmd(int tfd,int val)
{	
	uint8_t reg_addr = val;
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;		
	return SUCCESS;	
}


temp_s read_cont(int tfd,uint8_t *val)
{
	uint8_t reg_addr = cmd_control;
	uint8_t *buf = malloc(sizeof(uint8_t)*1);
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	*val = buf[0];
	free(buf);
	return SUCCESS;		
}

temp_s write_cont(int tfd,uint8_t val)
{
	uint8_t reg_addr = cmd_control;
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_write_byte(tfd,val);
	if(stat == FAILURE)
		return FAILURE;
	return SUCCESS;
}

temp_s read_tim(int tfd,uint8_t *val)
{
	uint8_t reg_addr = cmd_timing;	
	uint8_t stat;	
	uint8_t *buf = malloc(sizeof(uint8_t)*1);

	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	*val = buf[0];
	free(buf);
	return SUCCESS;
}

temp_s write_tim(int tfd,int integ,int gain)
{
	uint8_t reg_addr = cmd_timing;
	uint8_t stat;	
	uint8_t reg_tim =0;

	stat=i2c_write_byte(tfd,reg_addr);
	if(gain == 1)
	reg_tim = reg_tim | 0b00010000;
	
	if(integ < 3)
	reg_tim = reg_tim + integ ;  
	
	else
	reg_tim = reg_tim +2;
	
	stat=i2c_write_byte(tfd,reg_tim);
	if(stat == FAILURE)
		return FAILURE;
	return SUCCESS;
}	

temp_s int_control(int tfd, int cont)
{
	uint8_t reg_addr = cmd_int_cont;
	uint8_t stat;
	uint8_t reg=0;
	stat =i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	if(cont>0)
		reg = reg | 0b00010000;

	stat =i2c_write_byte(tfd,reg);
	if(stat == FAILURE)
		return FAILURE;
	return SUCCESS;
}


temp_s identity(int tfd,uint8_t *val)
{
	uint8_t reg_addr = cmd_identity;
	uint8_t stat;
	uint8_t *buf = malloc(sizeof(uint8_t)*1);
	stat =i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;	
	stat =i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;	
	*val = buf[0];

	free(buf);
	return SUCCESS;
}

temp_s int_thres_read(int tfd, uint16_t * val_low,uint16_t *val_high)
{
	uint8_t buff[4];
	uint8_t stat;
	uint8_t *buf=malloc(sizeof(uint8_t)*1);
	uint8_t reg_addr = cmd_int_threslow1;
	stat =i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat =i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	buff[0]=buf[0];
//	printf("%x \n",buff[0]);
	reg_addr = cmd_int_threslow2;
	stat =i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat =i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	buff[1]=buf[0];
//	printf("%x \n",buff[1]);

	reg_addr = cmd_int_threshigh1;
	stat =i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat =i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	buff[2]=buf[0];
	printf("%x \n",buff[2]);

	reg_addr = cmd_int_threshigh2;
	i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat =i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	buff[3]=buf[0];
//	printf("%x \n ",buff[3]);

	*val_low = ((buff[1] << 8) | buff[0]);
	*val_high=((buff[3] << 8) | buff[2]);
	
	free(buf);
	return SUCCESS;

}

temp_s int_thres_write(int tfd, uint8_t vallow,uint8_t valhigh,int reg)
{
	uint8_t reg_addr = cmd_int_threslow1;
	uint8_t stat;
	if(reg>0)
		reg_addr = cmd_int_threshigh1;
	uint8_t *buf = malloc(sizeof(uint8_t)*3);
	buf[0]= reg_addr;
	buf[1]=vallow;
	buf[2]=valhigh;
	stat =i2c_write_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	free(buf);

	return SUCCESS;

}


temp_s lum_data(int tfd, float * val)
{
	uint8_t reg_addr = cmd_light_data;
	uint8_t stat;
	uint8_t *buf= malloc(sizeof(uint8_t)*2);
	uint16_t ch0 , ch1;
	stat =i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
//	i2c_write_byte(tfd,reg_addr);
	
	stat =i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	ch0 = buf[1] << 8 | buf[0];
	printf("ch0: %d\n",ch0);
	reg_addr = cmd_light_data1;
	stat =i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat =i2c_read_byte(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	ch1 = buf[1] << 8 | buf[0];
//	printf("ch1 : %d",ch1);
	float temp,lux=0;	
	if(ch0!=0)
	{
	temp = (ch1/ch0);	
	
	if(0<temp<=0.5)
	{
		double x,y;
		x =(double)temp;
		y = pow(x,1.4);		
		
		lux = 	(0.0304 * ch0 )-(0.0602 *ch0*(y));
	}	
	
	else if(0.5 < temp <= 0.61)
		lux=((0.0224 * ch0)-(0.031 * ch1));

	else if(0.61 < temp <= 0.80) 
		lux = ((0.0128 * ch0)-(0.0153 * ch1));

	else if(0.80 < temp <= 1.30) 
		lux=((0.00146 * ch0)-(0.00112 * ch1));

	else if(temp>1.3)
		lux =0;
	}
	*val = lux;
	free(buf);
	return SUCCESS;

}



void signal_handler()
{
	LOG(INFO, LIGHT_TASK, "Light task is terminating", NULL);
	kill_hbthread =1;
	kill_process =1;
	mq_close(heart_thread);
	mq_unlink(HBLIGHT_QUEUE);
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	sem_close(sem);
	mq_close(mq_sock_light);
	mq_unlink(LIGHT_SOCK_QUEUE);
	sem_destroy(sem);
	pthread_cancel(heart_thread);
	pthread_cancel(sock_thread);
	pthread_join(heart_thread, NULL);
	pthread_join(sock_thread, NULL);
	exit(1);
}


int main()
{
	int result_thread;
	float k;

	/* signal handler to handle the file closings and approp exit for thread*/
	signal(SIGINT, signal_handler);
	
	sem = sem_open(I2C_SEM, O_CREAT | O_RDWR, 0666, 1);
	if(sem == SEM_FAILED)
	{
		perror("SEM FAILED");
	}
	struct mq_attr attr_log;
	attr_log.mq_maxmsg = 5;
	attr_log.mq_msgsize = sizeof(Logger_t);
	
	if((logger_q = mq_open(LOG_QUEUE, O_RDWR, 0666, &attr_log))==-1)
	{
		perror("LOGGER QUEUE");
		return -1;
	}
	int status;
	i2c_bus_open(&tsen_fd);
	status=i2c_bus_init(tsen_fd, LIGHT_SENSOR);
	if(status = SUCCESS)
		LOG(INFO,LIGHT_TASK, "STARTUP:LIGHT SENSOR WORKING BY OPENING I2C",NULL);
	/*writing to control register to see if sensor is active and powered up*/

	write_cont(tsen_fd,0x03);
	int v=0;
	read_cont(tsen_fd,&v);
	if(v = 0x03)
		LOG(INFO,LIGHT_TASK,"STARTUP: LIGHT SENSOR RESPONDED AND IS POWERED UP",&v);
	else if(v==0)
		identification_led();
	result_thread = pthread_create(&heart_thread, NULL, heartbeat_func, NULL);
        if(result_thread)
        {
                printf("Pthread Heartbeat Creation failed\n");
                exit(1);
        }
	
	result_thread = pthread_create(&sock_thread, NULL, sock_func, NULL);
        if(result_thread)
        {
                printf("Pthread Socket Creation failed\n");
                exit(1);
        }
	

	
	while(!kill_process)
	{	
		if((lum_data(tsen_fd,&k))==SUCCESS)
			LOG(INFO, LIGHT_TASK, "LUM is", &k);
		sleep(2);	 
	}
	mq_close(heart_thread);
	mq_unlink(HBLIGHT_QUEUE);
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	sem_close(sem);
	mq_close(mq_sock_light);
	mq_unlink(LIGHT_SOCK_QUEUE);
	sem_destroy(sem);
	pthread_cancel(heart_thread);
	pthread_cancel(sock_thread);
	pthread_join(heart_thread, NULL);
	pthread_join(sock_thread, NULL);
	exit(1);
}

/* heartbeat thread is used to signal main that it is alive and running*/

void * heartbeat_func(void *un)
{
	
	uint8_t val;
	struct mq_attr attr;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = sizeof(val);
	printf("light hb\n");
	if((mq_heart = mq_open(HBLIGHT_QUEUE, O_RDWR , 0666, &attr))==-1)
	{
		printf("Error Opening hb message queue : Temp Sensor ");
		exit(1);
	}
	

	while(!kill_hbthread)
	{

		if((mq_receive(mq_heart, &val, sizeof(val), 0))==-1)
			{
				printf("Din't receive message from process main and returned %d\n", errno);
			}
	
		val = val+1;

		if((mq_send(mq_heart, &val, sizeof(val), 0))==-1)
		{
		        printf("cant send message to process1 and returned %d\n", errno);
		}
		sleep(1);

	}

}

/* socket function for socket calls from other process to  light sensor*/

void *sock_func()
{
	int fd;	
	float data;
	struct mq_attr attr;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = sizeof(data_t);
	if((mq_sock_light = mq_open(LIGHT_SOCK_QUEUE, O_RDWR | O_CREAT, 0666, &attr))==-1)
	{
		printf("Error Opening sock message queue : Temp Sensor ");
		exit(1);
	}
	
	float lum;
	uint16_t val_low,val_high;
	data_t msg;

	while(1)
	{

		if((mq_receive(mq_sock_light, &msg, sizeof(data_t), 0))==-1)
			{
				printf("Din't receive message from process main and returned %d\n", errno);
			}
		

		if(msg.a == 1)
		{	
			//integration time scale 1
			write_tim(tsen_fd,1,2);

		}
		
		else if(msg.a ==2)
		{
			//integration time scale 2
			write_tim(tsen_fd,2,2);
		}
		
		else if(msg.a ==3)
		{
			//Enable gain
			write_tim(tsen_fd,4,1);
		}
		
		else if(msg.a ==4)
		{
			int_control(tsen_fd, 1);   //interrupt ENABLE
		}
		else if(msg.a ==5)
		{
			int_control(tsen_fd, 0);   //interrupt DISABLE
		}
		else if(msg.a ==6)
		{
			int_thres_read(tsen_fd, &val_low, &val_high) ; // intr reg read 
		}
		else if(msg.val1 ==8)
		{
			int_thres_write(tsen_fd, msg.a,msg.b,0);  // int low
		}
	
		else if(msg.val1 ==9)
		{
			int_thres_write(tsen_fd, msg.a,msg.b,1);  // int high
		}

		else if(msg.a ==7)
		{
			lum_data(tsen_fd, &lum);
		}
	
		msg.data[0] = lum;
		msg.val1 = val_low;
		msg.val2 = val_high;

		
		if((mq_send(mq_sock_light, &msg, sizeof(data_t), 0))==-1)
		{
		        printf("cant send message to Socket and returned %d\n", errno);
		}

	}

}


void LOG(uint8_t loglevel, uint8_t log_source, char *msg, float *value)
{
        Logger_t logging;
        logging.timestamp=time(NULL);
        logging.log_level = loglevel;
        logging.log_ID = log_source;
	if(value!=NULL)
		logging.value = *value;
	else
		logging.value = 0;
        memcpy(logging.message, msg, MSG_SIZE);
        if((mq_send(logger_q, &logging, sizeof(Logger_t),0))==-1)
        {
                printf("cant send message to process1 and returned %d\n", errno);
        }
}

