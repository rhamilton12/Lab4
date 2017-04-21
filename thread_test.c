#include <pthread.h>
#include <unistd.h> //  sleep() and usleep()
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>


void *thread(void *arg);
void signal_handler(int signum);



int fd;

void *thread(void *arg) 
{ // arguments not used in this case
 char *bus = "/dev/i2c-0";
  int addr = 0x48;          // The I2C address of TMP102 
  char buf[2] = {0};
  int temp;
  unsigned char MSB, LSB;

  double f,c;


  if ((fd = open(bus, O_RDWR)) < 0) {
    //ERROR HANDLING: you can check errno to see what went wrong
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

       temp = ((MSB << 8) | LSB) >> 4;

       c = temp*0.0625;
       f = (1.8 * c) + 32;

       printf("Temp Fahrenheit: %.3f Celsius: %.3f\n", f, c);
     }

     sleep(1);
   }

    return NULL;
}

int main() {
    
	pthread_t pt;
    int ret;
    ret = pthread_create(&pt, NULL, &thread, NULL);
	
    if(ret != 0) 
	{
        printf("Error: pthread_create() failed\n");
        exit(EXIT_FAILURE);
    }

        pthread_exit(NULL);
    
}


void signal_handler(int signum)
{

  assert(0 == close(fd));

  exit(signum);

}
