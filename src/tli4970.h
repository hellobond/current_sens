/*
 * tli4970.h
 *
 *  Created on: May 3, 2017
 *      Author: Galen
 */

#ifndef TLI4970_H_
#define TLI4970_H_

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

#define STATUS_BIT	0x80
#define OCD_BIT		0x20

#define	TRUE	(1==1)
#define	FALSE	(!TRUE)

#define	SPI_CHAN	0
#define	MAX_SIZE	(1024*1024)
#define SPI_MODE	1

class Packet{
public:
	unsigned char data[];
	int size;
};
void printBuf(unsigned char *ptr, int len );

void spiSetup(int speed);

Packet readData(int len);
int parseCurrentPacket(unsigned char packet[2]);
float getCurrentFromValue(int value);


void setupSICI(int pwm_small);
int SICIBit(bool bit);
int SICIWord(unsigned char reg, unsigned char val);


#endif /* TLI4970_H_ */
