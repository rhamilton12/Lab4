//   April 22nd  2017
//      
// Editeurs   Moulay_Elabidi & Ray Hamilton
//
//
//
//

//Hearder files
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

//Definitions
#define STROBE	(40) // IO8
#define GPIO_A0	(48) // A0
#define GPIO_A1	(50) // A1
#define GPIO_A2	(52) // A2
#define GPIO_A3	(54) // A3
#define GPIO_DIRECTION_IN      (1)  
#define GPIO_DIRECTION_OUT     (0)
#define ERROR                  (-1)
#define HIGH	0x31
#define LOW	0x30
#define NUM_THREADS     4
#define MSG_RESET 0x1 // Reset the sensor to initial state.
#define MSG_PING 0x2 // Check if the sensor is working properly.
#define GET_VOLT 0x3 // Obtain the most recent ADC result.
#define MSG_ACK 0xE // Acknowledgement to the commands.
#define MSG_NOTHING 0xF // Reserved
#define GET_TEMP	0x4
#define GET_PICT	0x5
#define SEND_DATA	0x6


	
//Global variables	
	int command = 0; // Stores user command
	int readData = 0;// Stores
	int results = 0;// Used
 	int d0 = 0; // GPIO_A0
	int d1 = 0; // GPIO_A1
	int d2 = 0; // GPIO_A2
	int d3 = 0; // GPIO_A3
	int dStrobe = 0;// GPIO STROBE
	float f,c;

void writeGPIO(int gpio, char val);
char readGPIO(int gpio);
void setGPIO(int gpio, int direction);
void enableGPIO(int gpio);
int storeData(int data0, int data1, int data2, int data3);
void signal_handler(int signum);
void HTTP_POST(const char* url, const char* image, int size);
const char current_time();
void *reset(void *arg1);
void *ping(void *arg2);
void *get(void *arg3);
void *temperature_reading(void* arg4);
void *image_capture( void* arg5);
void *client_server(void* arg6);



void *reset(void *arg1)
{		

			// Set strobe high to indicate to the pic that a command is incomming.
			
			writeGPIO(STROBE, HIGH);

			// Send ping command to the pic
			writeGPIO(GPIO_A0, LOW);
			writeGPIO(GPIO_A1, LOW);
			writeGPIO(GPIO_A2, LOW);
			writeGPIO(GPIO_A3, LOW);

			// Wait for 10 ms to ensure data is read and operated on by the pic.
			usleep(10000);

			// Set strobe low
			writeGPIO(STROBE, LOW);

			// Change direction of data lines to be able to read
			setGPIO(GPIO_A0, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A1, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A2, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A3, GPIO_DIRECTION_IN);
			usleep(10000);
			
			// Read data from pic
			d0 = readGPIO(GPIO_A0); // ASCII value of bit 0.
			d1 = readGPIO(GPIO_A1); // ASCII value of bit 1.
			d2 = readGPIO(GPIO_A2); // ASCII value of bit 2.
			d3 = readGPIO(GPIO_A3); // ASCII value of bit 3.
			
			// Store data from the pic into an integer value
			readData = storeData(d0, d1, d2, d3);
			
			// Change direction of data lines back to be able to write
			setGPIO(GPIO_A0, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A1, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A2, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A3, GPIO_DIRECTION_OUT);
						
			if (readData == MSG_ACK)
			{
				printf("Device reset.\n");
			}
			else
			{
				printf("Device did not reset.\n");
			}
}


void *ping(void *arg2)
{
	
			// Set strobe high to indicate to the pic that a command is incomming.
			writeGPIO(STROBE, HIGH);

			// Send ping command to the pic
			writeGPIO(GPIO_A0, HIGH);
			writeGPIO(GPIO_A1, LOW);
			writeGPIO(GPIO_A2, LOW);
			writeGPIO(GPIO_A3, LOW);

			// Wait for 10 ms to ensure data is read and operated on by the pic.
			usleep(10000);

			// Set strobe low
			writeGPIO(STROBE, LOW);

			// Change direction of data lines to be able to read
			setGPIO(GPIO_A0, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A1, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A2, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A3, GPIO_DIRECTION_IN);
			usleep(10000);
			
			// Read data from pic
			d0 = readGPIO(GPIO_A0); // ASCII value of bit 0.
			d1 = readGPIO(GPIO_A1); // ASCII value of bit 1.
			d2 = readGPIO(GPIO_A2); // ASCII value of bit 2.
			d3 = readGPIO(GPIO_A3); // ASCII value of bit 3.
			
			// Store data from the pic into an integer value
			readData = storeData(d0, d1, d2, d3);
			
			// Change direction of data lines back to be able to write
			setGPIO(GPIO_A0, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A1, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A2, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A3, GPIO_DIRECTION_OUT);
						
			if (readData == MSG_ACK)
			{
				printf("Device ready.\n");
			}
			else
			{
				printf(" Error Device no responding.\n");
			}
}


void *get(void *arg3)
{

			// Set strobe high to indicate to the pic that a command is incomming.
			writeGPIO(STROBE, HIGH);

			// Send ping command to the pic
			writeGPIO(GPIO_A0, LOW);
			writeGPIO(GPIO_A1, HIGH);
			writeGPIO(GPIO_A2, LOW);
			writeGPIO(GPIO_A3, LOW);

			// Wait for 10 ms to ensure data is read and operated on by the pic.
			usleep(10000);

			// Set strobe low
			writeGPIO(STROBE, LOW);

			// Change direction of data lines to be able to read
			setGPIO(GPIO_A0, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A1, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A2, GPIO_DIRECTION_IN);
			setGPIO(GPIO_A3, GPIO_DIRECTION_IN);
			usleep(10000);
			
			results = 0;
			readData = 0;
			// First set of data
			d0 = readGPIO(GPIO_A0); // ASCII value of bit 0.
			d1 = readGPIO(GPIO_A1); // ASCII value of bit 1.
			d2 = readGPIO(GPIO_A2); // ASCII value of bit 2.
			d3 = readGPIO(GPIO_A3); // ASCII value of bit 3.
			
			// Store data from the pic into an integer value
			results = storeData(d0, d1, d2, d3);
			writeGPIO(STROBE, HIGH);
			usleep(10000);
			writeGPIO(STROBE, LOW);
			usleep(10000);
			
			// Second set of data
			d0 = readGPIO(GPIO_A0); // ASCII value of bit 0.
			d1 = readGPIO(GPIO_A1); // ASCII value of bit 1.
			d2 = readGPIO(GPIO_A2); // ASCII value of bit 2.
			d3 = readGPIO(GPIO_A3); // ASCII value of bit 3.
			
			// Store data from the pic into an integer value
			readData = storeData(d0, d1, d2, d3);
			readData <<= 4;
			results = results | readData;
			writeGPIO(STROBE, HIGH);
			usleep(10000);
			writeGPIO(STROBE, LOW);
			usleep(10000);
			
			// Third set of data
			d0 = readGPIO(GPIO_A0); // ASCII value of bit 0.
			d1 = readGPIO(GPIO_A1); // ASCII value of bit 1.
			d2 = readGPIO(GPIO_A2); // ASCII value of bit 2.
			d3 = readGPIO(GPIO_A3); // ASCII value of bit 3.
			
			// Store data from the pic into an integer value
			readData = storeData(d0, d1, d2, d3);
			readData <<= 8;
			results = results | readData;
			
			// Change direction of data lines back to be able to write
			setGPIO(GPIO_A0, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A1, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A2, GPIO_DIRECTION_OUT);
			setGPIO(GPIO_A3, GPIO_DIRECTION_OUT);

			printf("ADC reults: %d\n", results);
}
	



// Enables a gpio for use.
void enableGPIO(int gpio)
{
	//pthread_mutex_lock( &mutex1 );
	char buffer[256];
	int fileHandle;

	//Export GPIO
	fileHandle = open("/sys/class/gpio/export", O_WRONLY);
	if(ERROR == fileHandle)
	{
		puts("Error: Unable to opening /sys/class/gpio/export");
		exit(1);
	}
	sprintf(buffer, "%d", gpio);
	write(fileHandle, buffer, strlen(buffer));
	close(fileHandle);
}



// Set the GPIO to either read or write
void setGPIO(int gpio, int direction)
{

	char buffer[256];
        int fileHandle;
        int fileMode;
	
   	//Direction GPIO
        sprintf(buffer, "/sys/class/gpio/gpio%d/direction", gpio);
        fileHandle = open(buffer, O_WRONLY);
        if(ERROR == fileHandle)
        {
               puts("Unable to open file:");
               puts(buffer);
               exit(1);
        }
        if (direction == GPIO_DIRECTION_OUT)
        {
               // Set out direction
               write(fileHandle, "out", 3);
               fileMode = O_WRONLY;
        }
        else
        {
               // Set in direction
               write(fileHandle, "in", 2);
               fileMode = O_RDONLY;
        }
        close(fileHandle);
}


// Write a value to a GPIO
void writeGPIO(int gpio, char val)
{
	
	char buffer[256];
        int fileHandle;
	
	//Setup GPIO to write
        sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio);
        fileHandle = open(buffer, O_WRONLY);
        if(ERROR == fileHandle)
        {
		puts("Unable to open file:");
               	puts(buffer);
               	exit(1);
        }
	write(fileHandle, &val, 1);
        close(fileHandle);  //This file handle will be used in write and close operations.	
}



// Read a value to a GPIO
char readGPIO(int gpio)
{
	char buffer[256];
	char data;
        int fileHandle;
	
	//Setup GPIO to read
        sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio);
        fileHandle = open(buffer, O_RDONLY);
        if(ERROR == fileHandle)
        {
		puts("Unable to open file:");
               	puts(buffer);
               	exit(1);
        }
	read(fileHandle, &data, 1);// Read 1 byte of data from fileHandle and place it in data
        return(data);  //This file handle will be used in read and close operations.	
}



// Convert the bit values of 4 gpios into an integer value.
int storeData(int data0, int data1, int data2, int data3)
{
	int data;
	
	// Convert data from its ASCII value to 0 or 1
	data0 &= 0x01;
	data1 &= 0x01;
	data2 &= 0x01;
	data3 &= 0x01;
	
	// Store bit values data0 - data3 into data
	data = 0;
	data = data0;
	data |= (data1 << 1);
	data |= (data2 << 2);
	data |= (data3 << 3);
	
	return data;
}


void signal_handler(int signum)
{
	int fd;
  assert(0 == close(fd));

  exit(signum);

}



void *temperature_reading(void* arg4)
{
  const char *bus = "/dev/i2c-0"; 
  int addr = 0x48;          // The I2C address of TMP102 
  char buf[2] = {0};
  int temp, fd;
  unsigned char MSB, LSB;

  if ((fd = open(bus, O_RDWR)) < 0) {
    // ERROR HANDLING: you can check errno to see what went wrong 
    perror("Failed to open the i2c bus");
    exit(1);
  }

 if (ioctl(fd, I2C_SLAVE, addr) < 0) {
   perror("Failed to acquire bus access and/or talk to slave.\n");
   // ERROR HANDLING; you can check errno to see what went wrong 
   exit(1);
 }

 // Register the signal handler 
 signal(SIGINT, signal_handler);


 while(1)
   {
     // Using I2C Read
     if (read(fd,buf,2) != 2) {
       // ERROR HANDLING: i2c transaction failed 
       perror("Failed to read from the i2c bus.\n");

   } else {

       MSB = buf[0];
       LSB = buf[1];

       // Convert 12bit int using two's compliment 
       temp = ((MSB << 8) | LSB) >> 4;

       c = temp*0.0625;
       f = (1.8 * c) + 32;

     }
	 printf("Temp Fahrenheit: %.3f\n", f);
	 printf("Temp Celsius: %.3f\n", c);
     sleep(1);	//wait for the thread to be executed
   }

}


 
void *image_capture( void* arg5)
{

	using namespace cv;
	using namespace std;
	VideoCapture stream1(0);  		 
 
	if (!stream1.isOpened())  		
	{
		cout << "cannot open camera"<<endl;
	}
 

	while (true) {
			Mat cameraFrame;
			stream1.read(cameraFrame);
			imwrite("/home/root/G8_image.jpg", cameraFrame);
			//imshow("cam", cameraFrame);
			if (waitKey(30) >= 0)
			break;
		     }
	 
}


void *client_server(void* arg6)
{
	const char* timestamp;	
	const char* hostname="192.168.43.242";
	const int   port=8000;
	const int   id=1;
	const char* password="password";
	const char* name="group8";
	const int   adcval=254;
	const char* status="Very+good+thank+you";
	const char* filename="G8_image.jpg";
	
   time_t rawtime;
   struct tm *info;
   char buff[80];
   time( &rawtime );
   info = localtime( &rawtime );
   //printf("Current local time and date: %s", asctime(info));
	
	char buf[1024];
	snprintf(buf, 1024, "http://%s:%d/update?id=%d&password=%s&name=%s&data=%d&status=%s&asctime(info)=%s&filename=%s",
			hostname,
			port,
			id,
			password,
			name,
			adcval,
			status,
            timestamp,
			filename);
	FILE *fp;
	
	struct stat num;
	stat(filename, &num);
	int size = num.st_size;
	printf("Image size: %dB\n", size);	

	char *buffer = (char*)malloc(size);

	fp = fopen(filename,"rb");
	int n = fread(buffer, 1, size, fp);

	HTTP_POST(buf, buffer, size);
	fclose(fp);
}


void HTTP_POST(const char* url, const char* image, int size)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl){
		curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,(long) size);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, image);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
      			fprintf(stderr, "curl_easy_perform() failed: %s\n",
              			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
	}
}



int main(void)
{
		
	 while(1)
      {
	
		// Initilize GPIO pins
		enableGPIO(GPIO_A0);
		enableGPIO(GPIO_A1);
		enableGPIO(GPIO_A2);
		enableGPIO(GPIO_A3);
		enableGPIO(STROBE);

		setGPIO(GPIO_A0, GPIO_DIRECTION_OUT);
		setGPIO(GPIO_A1, GPIO_DIRECTION_OUT);
		setGPIO(GPIO_A2, GPIO_DIRECTION_OUT);
		setGPIO(GPIO_A3, GPIO_DIRECTION_OUT);
		setGPIO(STROBE, GPIO_DIRECTION_OUT);

   
		// Prompt user for a command.
		printf("\nPlease enter your command :\n");
		printf("	  	1 -> Reset the ADC \n");
		printf("      		2 -> Ping the PIC \n");
		printf("      		3 -> Get the voltage value \n");
		printf("      		4 -> Get the temperature value \n");		
		printf("      		5 -> Take the picture \n");		
		printf("      		6 -> Send data to the server \n");
		scanf("%d", &command);
		
	
		if (command == MSG_RESET)
		{
			pthread_t pt1;
            int ret1;
			ret1 = pthread_create(&pt1, NULL, &reset, NULL);  //thread1 creation
			if(ret1 != 0) 
				{
					printf("Error: pthread_create() failed\n");
					exit(EXIT_FAILURE);
				}
			sleep(1);	//wait for the thread to be executed	
			ret1 = pthread_cancel(pt1);	
		}
		else if (command == MSG_PING)
		{
			pthread_t pt2;
            int ret2;		
			ret2 = pthread_create(&pt2, NULL, &ping, NULL); //thread2 creation
			if(ret2 != 0) 
				{
					printf("Error: pthread_create() failed\n");
					exit(EXIT_FAILURE);
				}
			sleep(1);	//wait for the thread to be executed	
			ret2 = pthread_cancel(pt2);	
		}
		else if (command == GET_VOLT)
		{
			pthread_t pt3;
            int ret3;		
			ret3 = pthread_create(&pt3, NULL, &get, NULL); //thread3 creation
			if(ret3 != 0) 
				{
					printf("Error: pthread_create() failed\n");
					exit(EXIT_FAILURE);
				}
			sleep(1);	//wait for the thread to be executed	
			ret3 = pthread_cancel(pt3);	
		}
		if (command == GET_TEMP)
		{
			pthread_t pt4;
            int ret4;			
			ret4 = pthread_create(&pt4, NULL, &temperature_reading, NULL); //thread4 creation
			if(ret4 != 0) 
				{
					printf("Error: pthread_create() failed\n");
					exit(EXIT_FAILURE);
				}
			sleep(1);	//wait for the thread to be executed	
			ret4 = pthread_cancel(pt4);	
		}
		else if (command == GET_PICT)
		{
			pthread_t pt5;
            int ret5;	
			ret5 = pthread_create(&pt5, NULL, &image_capture, NULL); //thread5 creation
			if(ret5 != 0) 
				{
					printf("Error: pthread_create() failed\n");
					exit(EXIT_FAILURE);
				}
			sleep(1);	//wait for the thread to be executed	
			ret5 = pthread_cancel(pt5);
		}
		else if (command == SEND_DATA)
		{
			pthread_t pt6;
            int ret6;		
			ret6 = pthread_create(&pt6, NULL, &client_server, NULL); //thread6 creation
			if(ret6 != 0) 
				{
					printf("Error: pthread_create() failed\n");
					exit(EXIT_FAILURE);
				}
			sleep(1);	//wait for the thread to be executed	
			ret6 = pthread_cancel(pt6);	
		}		

	  }

		pthread_exit(NULL);

}
//compiling command
//root@galileo:~# g++ -lpthread -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_core -lcurl lab4_prog1_2_3.c -o lab4_2