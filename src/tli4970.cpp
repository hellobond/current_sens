/*
 * tli4970.cpp
 *
 *  Created on: May 3, 2017
 *      Author: Galen
 */

#include "tli4970.h"

#define SICI_PIN	21

static int myFd ;
int t1, t2, check, wait;

using namespace std;

void printBuf(unsigned char *ptr, int len ){

	int i = 0;
	printf("Data @ %#04x: ", ptr);
	for( i=0; i<len; i++){
	  printf("%#02x ", ptr[i]);
	}
	printf("\n");
}

void spiSetup(int speed){
  if ((myFd = wiringPiSPISetupMode (SPI_CHAN, speed, SPI_MODE)) < 0)
  {
    fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
    exit (EXIT_FAILURE) ;
  }
}

Packet readData(int size){
	Packet read;

	read.size = size;
	int speed = 1;
	unsigned char *data_frame;
	unsigned char to_write[size];

	for(int i = 0; i < size; i++){
		to_write[i] = 0x00;
	}


	if ((data_frame = new unsigned char[2]) == NULL){
	    fprintf (stderr, "Unable to allocate buffer: %s\n", strerror (errno)) ;
	    exit (EXIT_FAILURE) ;
	}

	if (wiringPiSetup () == -1)
		exit (1) ;

	spiSetup(speed * 1000000) ;
	//printf("assigning Data\n");

	memcpy(data_frame, to_write, 2);

	//printBuf(data_frame, size);

	if (wiringPiSPIDataRW (SPI_CHAN, data_frame, size) == -1){
		printf ("SPI failure: %s\n", strerror (errno)) ;
	}
	//printBuf(data_frame, size);

	read.data[0] = data_frame[0];
	read.data[1] = data_frame[1];

	close(myFd);
	free(data_frame);
	return read;
}


int parseCurrentPacket(unsigned char packet[2]){

	int value = 0;
	unsigned char currMSB = 0x1F;

	if(packet[0] && (1 << STATUS_BIT)){
		cout << "Status Packet (not current value): " << hex << packet[0] << endl;
		return -1;
	}
	else if(packet[0] && (1 <<  OCD_BIT)){
		cout << "OCD bit status asserted: " << hex << packet[0] << endl;
		return -1;
	}
	else{
		//clean and format current value
		currMSB &= packet[0];
		value = currMSB << 8;
		value |= packet[1];

		return value;
	}

	return -1;
}

float getCurrentFromValue(int value){
	return (float)(value -4096)/80;
}

void setupSICI(int pwm_small){

//	t1 = pwm_small;
//	t2 = pwm_small *4;
//	check = pwm_small*3-1;
//	check = t2-t1;
//	wait = pwm_small;
	t1 = 2;
	t2 = 6;
	check = 4;
	wait = 25;

	wiringPiSetup();
}

int SICIBit(bool bit){

	pinMode(SICI_PIN, OUTPUT);

	if(!bit){
		digitalWrite(SICI_PIN, LOW);
		delayMicroseconds(t1);
		digitalWrite(SICI_PIN, HIGH);
		delayMicroseconds(t2);
		digitalWrite(SICI_PIN, LOW);
	}
	else{
		digitalWrite(SICI_PIN, LOW);
		delayMicroseconds(t2);
		digitalWrite(SICI_PIN, HIGH);
		delayMicroseconds(t1);
		digitalWrite(SICI_PIN, LOW);
	}
	pinMode(SICI_PIN, INPUT);
	//pullUpDnControl(SICI_PIN, PUD_OFF);
	delayMicroseconds(check);
	return digitalRead(SICI_PIN);
}


int SICIWord(unsigned char reg, unsigned char val){
	unsigned short out_word;
	int current_bit;

	//concat reg and val into 16bit word bitwise
	out_word = reg << 8;
	out_word |= val;

	unsigned char sensor_check_reg = 0x00;
	for(int i = (sizeof(short)*8)-1;i>=0; i--){
		current_bit = (out_word >> i) & 1;
		if(i == sizeof(short)*8-1) //delay before first bit for clock stabilization?
			delayMicroseconds(50);
		sensor_check_reg |= SICIBit(current_bit) << i;
		//delayMicroseconds(wait);
	}
	//cout << "check: " << sensor_check_reg << endl;
	return sensor_check_reg;
}


