#ifndef _PROJECT_H

#define _PROJECT_H


#define LOG_QUEUE "/logqueue"
#define HBTEMP_QUEUE "/hbtempqueue"
#define HBLIGHT_QUEUE "/hblightqueue"
#define HBLOG_QUEUE "/hblogqueue"
#define HBSOCK_QUEUE "/hbsockqueue"
#define I2C_SEM "/isclock"
#define TEMP_SOCK_QUEUE "/tempsocketqueue"
#define LIGHT_SOCK_QUEUE "/lightsockqueue"

typedef enum process
{
	LOG_PROCESS,
	TEMP_PROCESS,
	LIGHT_PROCESS,
	SOCK_PROCESS,
}Process_t;

#endif
