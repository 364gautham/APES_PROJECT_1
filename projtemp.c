
#include "temp.h"
#include "log.h"
#include "socket.h"
#include "i2c.h"
#include "project.h"
#include "errno.h"
#include "usrled.h"

pthread_t heart_thread,sock_thread;
int tsen_fd;
sem_t *sem;
mqd_t mq_sock_temp, mq_heart;
mqd_t logger;
volatile sig_atomic_t kill_process=0, kill_hbthread=0;

temp_s write_ptrreg(int tfd , int regval)
{
	
	uint8_t reg_addr = regval;
	uint8_t stat;
	stat = i2c_write_byte(tfd,reg_addr);

	return stat;
}

temp_s read_tlowreg(int tfd, float* byte)
{
	uint8_t *buf = malloc(sizeof(uint8_t)*2);		
	uint8_t stat;
	uint8_t reg_addr = tlow_reg;
	unsigned char MSB,LSB;
	float temp,f,c,k;
	int val;
	

	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;		
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;

	MSB = buf[0];
	LSB = buf[1];

	/* READ EM_bit of Config register*/
	read_config_em(tfd,&val);
	if(val ==1)
	{
		temp  = (( MSB << 8 )| LSB ) >> 3;
	
		c = temp * 0.0625;	//celsius
		f = (1.8*c)+32;		// fahrenheit
		k = c +273;        	// kelvin
		//printf();

		*byte = c;       //Sending Celsius data 
	}
	else{

		temp  = (( MSB << 8 )| LSB ) >> 4;
	
		c = temp * 0.0625;	//celsius
		f = (1.8*c)+32;		// fahrenheit
		k = c +273;        	// kelvin
		//printf();

		*byte = c;    //Sending Celsius data  	
	
	}

	free(buf);
	return SUCCESS;

}


temp_s read_thighreg(int tfd,float* byte)
{
	uint8_t reg_addr = thigh_reg;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);	
	unsigned char MSB,LSB;
	uint8_t stat;
	float temp,f,c,k;
	int val;

	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;	
	MSB =buf[0];
	LSB = buf[1];

	/* READ EM_bit of Config register*/
	read_config_em(tfd,&val);

	if(val == 1)
	{
		temp  = (( MSB << 8 )| LSB ) >> 3;
	
		c = temp * 0.0625;	//celsius
		f = (1.8*c)+32;		// fahrenheit
		k = c +273;        	// kelvin
		//printf();

		*byte = c  ;     //Sending Celsius data 
	}
	else{

		temp  = (( MSB << 8 )| LSB ) >> 4;
	
		c = temp * 0.0625;	//celsius
		f = (1.8*c)+32;		// fahrenheit
		k = c +273;        	// kelvin
		//printf();
		printf("th %f\n",c);
		printf("th f %f \n",f);
		*byte = c   ;    //Sending Celsius data  	
	
	}

	free(buf);
	return SUCCESS;

}


temp_s read_tempdata(int tfd, float *c ,float *k ,float *f)
{
	
	uint8_t data[10];
	unsigned char MSB,LSB;	
	uint8_t reg_addr = temp_reg;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	uint8_t stat;
	float temp;
	unsigned char nMSB ;
	int val;

	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	MSB = buf[0];
	LSB = buf[1];
	printf("%x...%x",MSB,LSB);

	
	read_config_em(tfd,&val);

	printf("Val %d",val);
	

	if(val ==1)
	{
		nMSB = MSB;
		nMSB = (nMSB & 0b10000000);
	
		if(nMSB == 0x80)
		{
			//negative temp
			MSB = ~MSB;
			LSB = ~LSB;
		
			temp  = (( ( MSB << 8 )| LSB ) >> 3) + 1;
			*c = temp * 0.0625;	//celsius
			*f = (1.8*(*c))+32;	// fahrenheit
			*k = (*c) +273;        	// kelvin
			
		}	
		else
		{
			temp  = ( ( MSB << 8 )| LSB ) >> 3;
			*c = temp * 0.0625;	//celsius
			*f = (1.8*(*c))+32;	// fahrenheit
			*k = (*c) +273;        	// kelvin
		}
	


	}
	
	else{
		nMSB = MSB;
		nMSB = (nMSB & 0b10000000);
	
		if(nMSB == 0x80)
		{
			//negative temp
			MSB = ~MSB;
			LSB = ~LSB;
		
			temp  = (( ( MSB << 8 )| LSB ) >> 4) + 1;
			*c = temp * 0.0625;	//celsius
			*f = (1.8*(*c))+32;		// fahrenheit
			*k = (*c) +273;        	// kelvin
				
		}	
		else
		{
			temp  = ( ( MSB << 8 )| LSB ) >> 4;
	
			*c = temp * 0.0625;	//celsius
			*f = (1.8*(*c))+32;		// fahrenheit
			*k =(*c) +273;        	// kelvin

			//printf("Temp: Fahrenheit: %f Celsius : %f\n",f,c); 
		}
	}
	
	free(buf);
	return SUCCESS;
}



temp_s write_config_sh(int tfd,int val)
{
	uint8_t reg_addr = config_reg_val;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	uint8_t *buf1 = malloc(sizeof(uint8_t)*3);	
	unsigned char MSB,LSB;
	uint8_t stat;	
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;	
	/* read present config reg value */
	
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;	
	MSB = buf[0];
	if(val == 0)
		MSB = MSB | 0b00000000;	
	else{
	MSB = MSB | 0b00000001;
	}	

	buf1[0]=reg_addr;
	buf1[1]=MSB;
	buf1[2]=buf[1];

	i2c_write_word(tfd,buf1);
	
	free(buf);
	free(buf1);
	return SUCCESS;	

}


temp_s read_config_fault(int tfd,uint8_t *fval)
{
	uint8_t reg_addr = config_reg_val;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);	
	uint8_t MSB,LSB;
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	MSB = buf[0];
	
	MSB = MSB & 0b00011000;
	
	*fval = MSB;

	/*
	if(MSB == 0x0)
		printf("Fault Queue is set to 1 Fault Queue\n");

	if(MSB == 0x08)
		printf("Fault Queue is set to 2 Fault Queue\n");

	if(MSB == 0x10)
		printf("Fault Queue is set to 4 Fault Queue\n");

	if(MSB == 0x18)
		printf("Fault Queue is set to 6 Fault Queue\n");
	*/
	free(buf);
	return SUCCESS;


}


temp_s read_config_em(int tfd , int * val)
{
	uint8_t reg_addr = config_reg_val;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);	
	unsigned char MSB,LSB;
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	LSB = buf[1];	
	
	LSB = LSB & (0b00010000);
	
	if(LSB == 0x10){
		//printf("Extended mode is enabled : 13 bit register value \n");
		*val =1;
	}
	else
	{

		//printf("Extended mode is disabled : 12 bit register value \n");
		*val = 0;
	}

	free(buf);
	return SUCCESS;

}

temp_s set_config_em(int tfd,int em_bit)
{

	uint8_t reg_addr = config_reg_val;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	uint8_t *buf1=malloc(sizeof(uint8_t)*3);
	unsigned char MSB,LSB;
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;	
	/* read present config reg value */
	
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	MSB= buf[0];
	LSB= buf[1];
	
	if(em_bit == 1)
	{
		LSB = LSB | 0b00010000;
	}
	else
		LSB= LSB | 0b00000000;	

	buf1[0]=reg_addr;
	buf1[1]=MSB;
	buf1[2]=LSB;

	i2c_write_word(tfd,buf1);
	
	free(buf);
	free(buf1);
	return SUCCESS;	


}


temp_s read_config_conv(int tfd, float *val)
{

	uint8_t reg_addr = config_reg_val;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);	
	unsigned char MSB,LSB;
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	LSB = buf[1];	
	
	LSB = LSB & (0b11000000);
	
	if(LSB == 0x0){
		//printf("Conversion rate is 0.25 Hz\n");
		*val = 	0.25;
	}
	if(LSB == 0x40){
		//printf("Conversion rate is 1 Hz \n");
		*val = 1;
	}

	if(LSB == 0x80){
		//printf("Conversion rate is 4 Hz (DEFAULT) \n");
		*val = 4;
	}

	if(LSB == 0xC0){
		printf("Conversion rate is 6 Hz \n");
		*val =6;
	}

	free(buf);
	return SUCCESS;

}

temp_s set_config_conv(int tfd , int conv_val)
{
	uint8_t reg_addr = config_reg_val;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	uint8_t *buf1 = malloc(sizeof(uint8_t)*3);	
	unsigned char MSB,LSB;
	uint8_t stat;

	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;	
	/* read present config reg value */
	
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;
	MSB= buf[0];
	LSB= buf[1];

	
	buf1[0] = reg_addr;
	buf1[1]= MSB;

	// read register value and then update accordingly ?

	if(conv_val == 0)
	{
		LSB = LSB & 0b00111111;
	}

	else if(conv_val == 1)
	{
		LSB = LSB & 0b01111111;
		LSB = LSB | 0b01000000;
	}

	else if(conv_val == 2)
	{
		LSB = LSB & 0b10111111;
		LSB = LSB | 0b10000000;
	}	

	else if(conv_val == 3)
	{
		LSB = LSB | 0b11000000;
	}
	
	buf1[2]=LSB;
	i2c_write_word(tfd,buf1);
	
	free(buf);
	free(buf1);
	return SUCCESS;
}
temp_s read_config(int tfd,uint16_t *val)
{
	uint8_t reg_addr = config_reg_val;
	uint8_t *buf = malloc(sizeof(uint8_t)*2);
	uint8_t MSB,LSB;
	uint16_t data;
	uint8_t stat;
	stat=i2c_write_byte(tfd,reg_addr);
	if(stat == FAILURE)
		return FAILURE;
	stat=i2c_read_word(tfd,buf);
	if(stat == FAILURE)
		return FAILURE;	
	data = (buf[0] <<8 | buf[1])>>4;

	*val = data;

	return SUCCESS;
}

void signal_handler()
{
	LOG(INFO, TEMP_TASK, "Temp process terminating", NULL);
	kill_process = 1;
	kill_hbthread = 1;
	sem_close(sem);
	sem_destroy(sem);
	mq_close(mq_heart);

	mq_unlink(HBTEMP_QUEUE);
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	mq_close(mq_sock_temp);
	mq_unlink(TEMP_SOCK_QUEUE);
	pthread_cancel(heart_thread);
	pthread_cancel(sock_thread);
	pthread_join(heart_thread, NULL);
	pthread_join(sock_thread, NULL);
	exit(1);
	
}


void startup_test()
{
	
}

int main()
{
	int result_thread;
	float c,k,f;
	signal(SIGINT, signal_handler);
	
	sem = sem_open(I2C_SEM, O_CREAT | O_RDWR, 0666,1);
	if(sem == SEM_FAILED)
	{
		perror("Semaphore failed \n");
		//exit(0);
	}
	
	struct mq_attr attr_log;
	attr_log.mq_maxmsg = 5;
	attr_log.mq_msgsize = sizeof(Logger_t);

	if((logger_q = mq_open(LOG_QUEUE, O_RDWR | O_CREAT, 0666, &attr_log))==-1)
	{
		perror("LOGGER QUEUE");
		return -1;
	}
	int st;

	i2c_bus_open(&tsen_fd);
	st=i2c_bus_init(tsen_fd,TEMP_SENSOR);
	if(st == FAILURE)
		identification_led();

	if((read_tempdata(tsen_fd, &c, &k, &f))==SUCCESS)
		LOG(INFO, TEMP_TASK, "STARTUP: TEMPERATURE SENSOR IS WORKING", NULL);
	else 
		LOG(INFO, TEMP_TASK, "STARTUP: TEMPERATURE SENSOR NOT CONNECTED", NULL);
		
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
		if((read_tempdata(tsen_fd,&c,&k,&f))==SUCCESS)
	         	LOG(INFO,TEMP_TASK,"TEMP IN CELSIUS IS",&c);
		sleep(2);	 
	}
	sem_close(sem);
	sem_destroy(sem);
	mq_close(mq_heart);
	mq_unlink(HBTEMP_QUEUE);
	mq_close(logger_q);
	mq_unlink(LOG_QUEUE);
	mq_close(mq_sock_temp);
	mq_unlink(TEMP_SOCK_QUEUE);
	pthread_cancel(heart_thread);
	pthread_cancel(sock_thread);
	pthread_join(heart_thread, NULL);
	pthread_join(sock_thread, NULL);
	exit(1);
}



void *heartbeat_func()
{
	
	uint8_t val;
	struct mq_attr attr;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = sizeof(val);
	if((mq_heart = mq_open(HBTEMP_QUEUE, O_RDWR, 0666, &attr))==-1)
	{
		printf("Error Opening hb message queue : Temp Sensor ");
		exit(1);
	}
	
	printf("hb tempe\n");
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



void *sock_func()
{	
	
	int data;
	struct mq_attr attr;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = sizeof(data_t);
	if((mq_sock_temp = mq_open(TEMP_SOCK_QUEUE, O_RDWR | O_CREAT, 0666, &attr))==-1)
	{
		printf("Error Opening sock message queue : Temp Sensor ");
		exit(1);
	}

	float c,k,f,byte;
	uint16_t val;
	data_t msg;
	
	while(1)
	{

		if((mq_receive(mq_sock_temp, &msg, sizeof(data_t), 0))==-1)
			{
				printf("Din't receive message from process main and returned %d\n", errno);
			}
		
		if(msg.a ==1)
		{
			read_tlowreg(tsen_fd,&byte);

		}
		
		else if(msg.a  ==2)
		{
			read_thighreg(tsen_fd,&byte);
		}
		
		else if(msg.a  ==3)
		{
			read_tempdata(tsen_fd,&c,&k,&f);
		}
		
		else if(msg.a  ==4)
		{
			write_config_sh(tsen_fd, 1);   //SHUTDOWN ENABLE
		}
		else if(msg.a  ==5)
		{
			write_config_sh(tsen_fd, 0);   //SHUTDOWN DISABLE
		}
		else if(msg.a  ==6)
		{
			read_config_conv(tsen_fd, &byte);
		}
		else if(msg.a  ==7)
		{
			set_config_conv(tsen_fd, 0);  //0.25 Hz CONFIGURATION
		}
	
		else if(msg.a  ==8)
		{
			set_config_conv(tsen_fd, 3);   //6 Hz CONFIGURATION
		}

		else if(msg.a  ==9)
		{
			read_config(tsen_fd,&val);   //CONFIGURATION REG VALUE
		}

		else if(msg.a ==10)
		{
			set_config_em(tsen_fd, 1);
		}
	
		msg.data[0]= byte;
		msg.data[1]= c;
		msg.data[2]= k;
		msg.data[3]= f;
		msg.val1 = val;

		if((mq_send(mq_sock_temp, &msg, sizeof(data_t), 0))==-1)
		{
		        printf("cant send message to Socket and returned %d\n", errno);
		}

	}

}

void LOG(uint8_t loglevel, uint8_t log_source, char *msg, float *value)
{
	int k=20;
	Logger_t logging;
        logging.timestamp=time(NULL);
        logging.log_level = loglevel;
        logging.log_ID = log_source;
	if(value!=NULL)
	(logging.value) = *value;
        memcpy(logging.message, msg, MSG_SIZE);
        if((mq_send(logger_q, &logging, sizeof(Logger_t),0))==-1)
        {
                printf("cant send message to process1 and returned %d\n", errno);
        }
}


