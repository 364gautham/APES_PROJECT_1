#include "usrled.h"

int identification_led()
{

	FILE * led =NULL;
	const char* LEDBrightness = "/sys/class/leds/beaglebone:green:usr0/brightness";

	int i=0;
	for(i=0;i<2;i++)
	{
		if((led=fopen(LEDBrightness,"r+"))!=NULL)
		{
			fwrite("1",sizeof(char),1,led);
			fclose(led);

		}

		usleep(50);

		if((led=fopen(LEDBrightness,"r+"))!=NULL)
		{
			fwrite("0",sizeof(char),1,led);
			fclose(led);

		}

	}
	return 0;
}
