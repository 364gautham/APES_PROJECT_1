# Project's Readme
In this Project we use two sensors namely Temperature Sensor tmp102 and Light Sensor APDS-9301. 

Data from these sensors are retreived using beaglebone I2C driver application. 

IOCTL system call is used to get access to I2C bus. Two sensors are synchronised  using synchronization mechanishm, Semaphores. 

Four Different tasks are created : Temp sensor task , Light sensor task , Logger task  and socket task. These tasks are communicated using message queues. 

Socket tasks gets data using implemneted Message API'S using the TCP socket and displays it for user. ARCHITECTURE DIAGRAM is provided for more details.
