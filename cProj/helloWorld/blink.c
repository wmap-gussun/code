// blink.c
//
// Example program for bcm2835 library
// Blinks a pin on an off every 0.5 secs
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o blink -l rt blink.c -l bcm2835
// sudo ./blink
//
// Or you can test it before installing with:
// gcc -o blink -l rt -I ../../src ../../src/bcm2835.c blink.c
// sudo ./blink
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2011 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $


#include <stdio.h>
#include <bcm2835.h>
#include <unistd.h>

// Blinks on RPi pin GPIO 11
#define PIN RPI_V2_GPIO_P1_22
#define PROTOCOL 17 //Protocol id
int SLEEP = 720; // (833 - some overhead)

void sendBit(int b, int s)
{
	// Send bit
	bcm2835_gpio_write(PIN, b);

	// Wait a bit
	usleep(s);
}
void sendByte(int data)
{
	// Loop all bits and send one by one
	int i;
	for(i = 0; i < 8; i++)
	{
		int b = data&1;
		data = data >> 1;
		sendBit(b, SLEEP);
	}
}

void send(int protocol, int id, int data)
{
	int i;
	for(i = 0; i < 6; i++)
	{
		// Send start
		sendBit(0, SLEEP);
		sendBit(1, SLEEP);
		// Send protocol
		sendByte(protocol);
		// Send id
		sendByte(id);
		// Send data
		sendByte(data);
		// Send parities
		sendBit(parity(protocol), SLEEP);
		sendBit(parity(id), SLEEP);
		sendBit(parity(data), SLEEP);
		// Send stop bit
		sendBit(1, SLEEP);
		// Set normal
		bcm2835_gpio_write(PIN, 1);
		// Wait and repeat
		delay(30);
	}
};

/**
 * Check parity, using voodo.
 */
int parity(unsigned int x) {
	unsigned int y;
	y = x ^ (x >> 1);
	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	return y & 1;
}

int main(int argc, char **argv)
{
	printf ("Raspberry Pi blink\n") ;
	// If you call this, it will not actually access the GPIO
	// Use for testing
	// bcm2835_set_debug(1);

	if(!bcm2835_init())
		return 1;

	// Set the pin to be an output
	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);

	// Set normal
	bcm2835_gpio_write(PIN, 1);

	// Blink
	unsigned int data;
	printf("Getting data \n");
	if(argc > 1)
	{
		data = atoi(argv[1]);
	}
	else
		return 0;
	printf("sending %d\n", data);
	int i = 0;
	while (i < 2000)
	{
		i++;
		send(PROTOCOL, 2, data);
		delay(1000);
		send(PROTOCOL, 2, data+10);
		delay(1000);
		//printf("on\n");
		//send(1, 2, 0);
		//delay(1000);
		//printf("of\n");
		//send(1, 2, 100);
		//delay(1000);
		//delay(500);
		//send(1, 2, 50);
		//delay(200);
		//send(1, 2, 25);
		//delay(200);
		//send(1, 2, 0);
		//delay(200);
		//send(1, 2, 25);
		//delay(200);
		//send(1, 2, 50);
		//delay(200);
		//send(1, 2, 75);
		//delay(200);
	}

	return 0;
}
