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

	unsigned int check_wait = 0;
	timespec start_x, t1_x, t2_x, t_x;

	pinMode(SICI_PIN, OUTPUT);


	if(!bit){
		digitalWrite(SICI_PIN, LOW);
		clock_gettime(CLOCK_REALTIME, &start_x);
		delayMicroseconds(t1);
		digitalWrite(SICI_PIN, HIGH);
		clock_gettime(CLOCK_REALTIME, &t1_x);
		delayMicroseconds(t2);
		digitalWrite(SICI_PIN, LOW);
		clock_gettime(CLOCK_REALTIME, &t_x);
		//cout << "start: " <<start_x.tv_nsec << " t: " <<t_x.tv_nsec << " t1: " << t1_x.tv_nsec << endl;
		//cout <<"t: " <<t_x.tv_nsec - start_x.tv_nsec<<endl;
		//cout <<"t1: " <<t1_x.tv_nsec - start_x.tv_nsec <<endl;
		check_wait = (unsigned int)((t_x.tv_nsec - start_x.tv_nsec) - 2*(t1_x.tv_nsec - start_x.tv_nsec))/1000;
	}
	else{
		digitalWrite(SICI_PIN, LOW);
		clock_gettime(CLOCK_REALTIME, &start_x);
		delayMicroseconds(t2);
		digitalWrite(SICI_PIN, HIGH);
		clock_gettime(CLOCK_REALTIME, &t2_x);
		delayMicroseconds(t1);
		digitalWrite(SICI_PIN, LOW);
		clock_gettime(CLOCK_REALTIME, &t_x);
		//cout <<"t: " <<t_x.tv_nsec - start_x.tv_nsec<<endl;
		//cout <<"t2: " <<t2_x.tv_nsec - start_x.tv_nsec <<endl;
		//check_wait = (unsigned int)(2*(t2_x.tv_nsec - start_x.tv_nsec) - (t_x.tv_nsec - start_x.tv_nsec))/1000;
	}
	pinMode(SICI_PIN, INPUT);
	//pullUpDnControl(SICI_PIN, PUD_OFF);
	//cout << "check_wait: "   << check_wait << endl;
	delayMicroseconds(3);
	return digitalRead(SICI_PIN);
}

unsigned char SICIFrame(unsigned char val[2]){

	unsigned char *out_frame;
	int current_bit;
	int width = 2;

	if ((out_frame = new unsigned char[2]) == NULL){
	    fprintf (stderr, "Unable to allocate buffer: %s\n", strerror (errno)) ;
	    exit (EXIT_FAILURE);
	}

	memcpy(out_frame, val, width);
	printBuf(out_frame, width);
	unsigned char sensor_check_reg[width];

	for(int i = 0; i < width; i++){
		sensor_check_reg[i] = 0x00;
	}

	for(int j = 0; j<width;j++){
		for(int i = 7;i>=0; i--){
				current_bit = (out_frame[j] >> i) & 1;
				if(j == 0 && i == 7) {//delay before first bit for clock stabilization?
					delayMicroseconds(50);
					//cout <<"initial delay"<<endl;
				}
				//cout << "bit " << i << ": " << current_bit <<endl;
				sensor_check_reg[j] |= SICIBit(current_bit) << i;
				//delayMicroseconds(wait);
			}
		//cout << "check: " << j << "-"<< hex << (int) sensor_check_reg[j] << dec << endl;
	}

	cout << "check: " << hex<< (int)sensor_check_reg[0] << " - "<< (int)sensor_check_reg[1] <<dec<< endl;
	free(out_frame);
	return sensor_check_reg[1];
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


