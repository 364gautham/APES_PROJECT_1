/********************************************************************************************************
*
* UNIVERSITY OF COLORADO BOULDER
*
* @file send_socket.c
* @brief socket communication
* 
* SHares data with other processes through socket
*
* @author Kiran Hegde and Gautham K A
* @date  3/13/2018
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
#include <string.h>
#include <unistd.h>
#include "socket.h"
#include <arpa/inet.h>
#include <stdint.h>
#include <netinet/in.h>


/********************************************************************************************************
*
* @name main
* @brief main function
*
* Opens a socket and binds it to port. connects to server and sends data.
* log the incoming data from server
*
* @param None
*
* @return zero on successful execution, otherwise error code
*
********************************************************************************************************/

int main()
{

        int client, sock, read_sock;
	int repeat=0;
        struct sockaddr_in address;
        int len = sizeof(address);
        Msg_t msg,msg_temp,msg_light;
	msg.source = TEMP_SENSOR;
	msg.format = 2;
	msg.data = 1;

        int option=1;
	/* open socket */
	while(!repeat)
	{	
        if((client = socket(AF_INET, SOCK_STREAM, 0))<0)
        {
                printf("server creation failed\n");
                exit(0);
        }
	
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	
	/* converts IP address to proper format */
	if((inet_pton(AF_INET, "127.0.0.1", &address.sin_addr))<=0)
	{
		printf("Addr error\n");
	}
	
	/* connect to server */
	if((connect(client, (struct sockaddr *)&address, sizeof(address)))<0)
		perror("connect:");
		uint8_t opt;
	
	/* User Interface */
		printf("Choose Any option\n");
		printf("1. TLOW Register Value\n");
		printf("2. THIGH Register Value\n");
		printf("3. Temperature Data\n");
		printf("4. Shutdown Mode Enable\n");
		printf("5. Shutdown Mode Disable\n");
		printf("6. Conversion rate present value\n");
		printf("7. To set Coversion Rate : 0.25HZ \n");
		printf("8. To Set Conversion Rate : 6 HZ\n");
		printf("9. Configuration Register Value\n");
		printf("10. To set Extended Mode Bit\n");
		printf("Light Sensor Options\n");
		printf("11. To set Integration Time to Scale 1\n");
		printf("12. To setIntegartion Tiem to Scale 2\n");
		printf("13. Enable GAIN\n");
		printf("14. Interrupt Enable \n");
		printf("15. Interrupt Disable\n");
		printf("16. Interrupt Register read\n");
		printf("17. Luminosity Value\n");
		printf("18. To Set Interrupt Low Threshold Value\n");
		printf("19. To set Interrupt High Threshold Value\n");
		printf("20. Check Night or Day using Light Sensor\n");
		scanf("%d", &opt);
		if(17<opt<20)
		{
			scanf("8 Bit value %x",&msg.a);
			scanf("8 bit Value %x",&msg.b);
		}
		if(opt ==20)
			opt=17;
		msg.source = TEMP_SENSOR;
		msg.format = opt;
		if(opt>10)
		{
			msg.source = LIGHT_SENSOR;
			msg.format = opt-10;
		}
		if(repeat!=1)
		{
			/* send data to other process */
			send(client, &msg, sizeof(Msg_t), 0);
			if(msg.source == TEMP_SENSOR)
			{
				read(client, &msg_temp, sizeof(Msg_t));
				if(opt==1)
					printf("TLOW Register Value %f\n",msg_temp.sen[0]);
				else if(opt ==2)
					printf("THIGH Register Value %f\n",msg_temp.sen[0]);
				else if(opt ==3){
					printf("Temperature Data in Celsius %f \n",msg_temp.sen[1]);
					printf("Temperature Data in Kelvin %f \n",msg_temp.sen[2]);
					printf("Temperature Data in Fahrenheit %f \n",msg_temp.sen[3]);
					}
				else if(opt ==4)
					printf("Shutdown Mode Enabled \n");
				else if(opt ==5)
					printf("Shutdown Mode Disabled \n");
				else if(opt ==6)
					printf("Conversion rate present value %f\n",msg_temp.sen[0]);
				else if(opt ==7)
					printf("Coversion Rate set to 0.25HZ \n");
				else if(opt ==8)
					printf("Coversion Rate set to 6 HZ \n");				
				else if(opt ==9)
					printf("Configuration Register Value %x",msg_temp.format);
				else if(opt ==10)
					printf("EM Bit Enabled \n");
			
			}
			
			if(msg.source == LIGHT_SENSOR)
			{
				/* read data from other process */
				read(client, &msg_light, sizeof(Msg_t));
				if(opt==11)
					printf("Integration Time Set to Scale 1\n");
				else if(opt ==12)
					printf("Integration Time Set to Scale 2\n");
				else if(opt ==13)
					printf("Gain Enabled for Light Sensor\n");
				else if(opt ==14)
					printf("Interrupt  Enabled \n");
				else if(opt ==15)
					printf("Interrupt  Disabled \n");
				else if(opt ==16)
					printf("Interrupt Low Register value %x : Interrupt High Regsiter Value %x :\n " ,msg_light.source,msg_light.format);
				else if(opt ==18)
					printf("Interrupt Low Threshold Regsiter Value Set \n");
				else if(opt ==19)
					printf("Interrupt High Threshold Regsiter Value Set \n");
				else if(opt ==17){

					printf("Luminosity Value %f\n",msg_light.sen[0]);
					if(msg_light.sen[0]>2)
						printf("It is Day \n");
					else
						printf("It is night or enough light is passed to Sensor\n");
				}
				
			}
		}
		/* close the client */
		close(client);
	}
	shutdown(client, 2);
	return 0;
}
