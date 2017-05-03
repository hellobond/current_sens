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


int main() {
	Packet read_packet;
	int size = 2;
	int output = 0;
	int n = 100;



	ofstream outfile("current_data.csv");

	for(int i = 0; i< n; i++){
		read_packet = readData(size);
		output = parseCurrentPacket(read_packet.data);
		cout << clock() <<" Packet Output: "<< output << " : Current (A): " <<  getCurrentFromValue(output) << endl;
		outfile << clock() << "," << output << "," <<  getCurrentFromValue(output) << endl;
		usleep(100000);
	}


	outfile.close();

	return 0;
}
