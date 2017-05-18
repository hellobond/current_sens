//============================================================================
// Name        : current_sens.cpp
// Author      : Galen Church
// Version     :
// Copyright   : 
// Description : Current Sense Test with TLI4970
//============================================================================

#include <iostream>
#include <fstream>
#include <iomanip>

#include <unistd.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "tli4970.h"

#define STATUS_BIT	0x80
#define OCD_BIT		0x20

using namespace std;

//calculate the sleep time in uS for number of samples per cycle of f
int samplesToSleep(int samples, int f){
	float time = (float)1/f;
	return (time/samples) * 1000000;  //returns count in uS to be passed to usleep();
}


int main() {
	Packet read_packet;
	int size = 2;
	int output = 0;
	int n = 100;
	int t_sleep = samplesToSleep(40, 60);

	timespec curr, last;
	clock_gettime(CLOCK_REALTIME, &last);

	ofstream outfile("current_data.csv");

	setupSICI(10);
	SICIWord(0x83, 0x80);

	for(int i = 0; i < n; i++){
		clock_gettime(CLOCK_REALTIME, &curr);
		read_packet = readData(size);
		output = parseCurrentPacket(read_packet.data);
		cout << curr.tv_nsec << "," <<  curr.tv_nsec - last.tv_nsec <<" Packet Output: "<< output << " : Current (A): " <<  getCurrentFromValue(output) << endl;
		outfile << curr.tv_nsec << "," << curr.tv_nsec - last.tv_nsec << "," << output << "," <<  getCurrentFromValue(output) << endl;
		usleep(t_sleep);
		last = curr;
	}

	outfile.close();

	return 0;
}
