//============================================================================
// Name        : current_sens.cpp
// Author      : Galen Church
// Version     :
// Copyright   : 
// Description : Current Sense Test with TLI4970
//============================================================================

#include <iostream>
#include <iomanip>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#define	TRUE	(1==1)
#define	FALSE	(!TRUE)

#define	SPI_CHAN	0
#define	MAX_SIZE	(1024*1024)
#define SPI_MODE	1

#define STATUS_BIT	0x80
#define OCD_BIT		0x20


static int myFd ;

using namespace std;

void spiSetup (int speed)
{
  if ((myFd = wiringPiSPISetupMode (SPI_CHAN, speed, SPI_MODE)) < 0)
  {
    fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
    exit (EXIT_FAILURE) ;
  }
}

void printBuf(unsigned char *ptr, int len ){

	int i = 0;
	printf("Data @ %#04x: ", ptr);
	for( i=0; i<len; i++){
	  printf("%#02x ", ptr[i]);
	}
	printf("\n");
}

int main() {
	unsigned char *data_frame;
	unsigned char currMSB = 0x1F;
	unsigned char MSB = 0x00;
	int speed = 1;
	int size = 4;
	int output = 0;
	float curr = 0.0f;

	unsigned const char to_write[] = {0x00, 0x00, 0x00, 0x00};

	if ((data_frame = new unsigned char[4]) == NULL){
	    fprintf (stderr, "Unable to allocate buffer: %s\n", strerror (errno)) ;
	    exit (EXIT_FAILURE) ;
	}

	if (wiringPiSetup () == -1)
		exit (1) ;

	spiSetup (speed * 1000000) ;
	printf("assigning Data\n");

	memcpy(data_frame, to_write, 4);

	printBuf(data_frame, size);

	if (wiringPiSPIDataRW (SPI_CHAN, data_frame, size) == -1){
		printf ("SPI failure: %s\n", strerror (errno)) ;
	}
	printBuf(data_frame, size);

	close (myFd);

	MSB = data_frame[0];

	if(MSB && (1 << STATUS_BIT)){
		cout << "Status Packet (not current value): " << hex << MSB << endl;
	}
	else if(MSB && (1 <<  OCD_BIT)){
		cout << "OCD bit status asserted: " << hex << MSB << endl;
	}
	else{
		//clean and format current value
		currMSB &= MSB;
		output = currMSB << 8;
		output |= data_frame[1];

		//calculate current value
		curr = (float)(output - 4096)/80;

		cout << "Output Value: " << output <<endl; //Prints value of Output [12:0]
		cout << "Current Value (A): " << setw(3) << curr << endl;
	}
	free(data_frame);
	return 0;
}
