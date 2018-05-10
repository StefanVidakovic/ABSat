/*
 * try.c
 *
 *  Created on: May 10, 2018
 *      Author: andre
 */

#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include "bsp.h"
#include "bsp_trace.h"
#include <em_i2c.h>
#include <stdlib.h>
#include <stdio.h>
#include "gpiointerrupt.h"
#include "segmentlcd.h"
#include "I2C.h"


void send(void * ptr)
{
	uint8_t state = I2C0->STATE;
	uint8_t flag = I2C0->IF;
	I2C0->CTRL = I2C0->CTRL & I2C_CTRL_AUTOACK;//autoack set

	*ptr = I2C0->TXDATA;


}


void write()//probably get some buffer data - checked and then an address
{
	//I2C_IntClear(I2C0, I2C_IFC_ADDR);
	//if (I2C->RXDATA )
	uint8_t state = I2C0->STATUS;
	//writing!!!
	if (state==0x57)//if there is a start signal
	{
		I2C0->TXDATA = addr+w; //put the addr and rw bits into txdata
		//send();
	}
	else if (state==0x97) // addr trasnmitted and ack recieved I2C_IF_ACK

	{
	I2C0->TXDATA = datatosend;
			//send();
	//I2C0->CMD = I2C0->CMD & I2C_CMD_START;

	}
	else if (state==0x9F)//address transmitted, Nack recieved
			{
		I2C0->CMD = I2C_CMD_CONT;//continue
		I2C0->TXDATA = addr+w;
			}
	else if (state==0xD7)//data sent ack recieved
	{
		I2C0->CMD = I2C_IF_STOP;
	}
	else if (state&0xDF)//data sendt, got NACK
	{
		I2C0->CMD = I2C_CMD_CONT;//continue
		I2C0->TXDATA = datatosend;
	}
}

void read()
{
	int status = I2C0->IF;// tells us what the bus is tying to do annd we can create the rest of the funciton accordingly

	if (status & I2C_IF_START){//start flag has been set, SENDING an address and read flag
		I2C0->TXDATA = addr+R;
	}
	else if (status & I2C_IF_ACK){
		I2C0->RXDATA = datacomingin;
		}
	else if (status & I2C_IF_NACK){
		I2C0->CMD = I2C_CMD_CONT;
		I2C0->RXDATA = datacomingin;
	}
	else if (status & I2C_IF_RXDATAV){
		I2C0->RXDATA = moredatacomingin;

	}
}

