
#include <pthread.h>
#include "temp.h"
#include "log.h"


temp_s i2c_bus_open(int *file)
{	
	sem_wait(sem);
	char *filename= "/dev/i2c-2";
	int a;	
	a=open(filename,O_RDWR);
	printf("%d\n",a);
	*file = a;
	if((a)<0){
		perror("Error : Opening File-Creating FD\n");
		sem_post(sem);		
		return FAILURE;  
	}
	sem_post(sem);
	
	return SUCCESS;
}

temp_s i2c_bus_init(int file, int sensor)
{
	sem_wait(sem);
	int addr;
	if(sensor==1)
		addr  = 0x48;
	else if(sensor==2)
		addr = 0x39;
	else
		LOG(ERROR, LIGHT_TASK, "Invalid Slave Address", NULL);
	if(ioctl(file,I2C_SLAVE,addr)<0){
		printf("Error : IOCTL Call \n");
		sem_post(sem);
		return FAILURE;
	}
	sem_post(sem);
	return SUCCESS;
}


temp_s i2c_read_byte(int fd , uint8_t *byte)
{
	sem_wait(sem);
	if(read(fd,byte,1)!=1)
	{
		perror("Error : Reading I2C BYTE \n");
		sem_post(sem);
		return FAILURE;
	}
	sem_post(sem);
	return SUCCESS;
}
/* Two continuous bytes are read into buffer of 8 bit size data type*/

temp_s i2c_read_word(int fd , uint8_t *byte)
{
	sem_wait(sem);
	if(read(fd,byte,2)!=2)
	{
		perror("Error : Reading I2C word \n");
		sem_post(sem);
		return FAILURE;
	}
	sem_post(sem);
	return SUCCESS;
}


/* Single byte is written using  buffer of 8 bit */
temp_s i2c_write_byte(int fd , uint8_t byte)
{
	sem_wait(sem);
	if(write(fd,&byte,1)!=1)
	{
		perror("Error : Writing I2C BYTE \n");
		sem_post(sem);
		return FAILURE;
	}
	sem_post(sem);
	return SUCCESS;
}
/* three continuous bytes are written using  buffer of 8 bit inclues registre specific address of 8 bits wide */

temp_s i2c_write_word(int fd , uint8_t *byte)
{
	sem_wait(sem);
	if(read(fd,byte,3)!=3)
	{
		perror("Error : Reading I2C word \n");
		sem_post(sem);
		return FAILURE;
	}
	sem_post(sem);
	return SUCCESS;
}

/* Two continuous bytes are written using  buffer of 8 bit  : used for Light sensor Int threshold REG*/
temp_s i2c_write_word_light(int fd , uint8_t * byte)
{
	sem_wait(sem);
	if(write(fd,byte,2)!=2)
	{
		perror("Error : Reading I2C word \n");
		sem_post(sem);
		return FAILURE;
	}
	sem_post(sem);
	return SUCCESS;
}
