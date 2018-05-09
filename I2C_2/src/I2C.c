







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
#include <I2C.h>

uint8_t TxBuffer[I2C_MAX_TX_BUFFER_SIZE];
uint16_t TxBufferSize = 0;
uint16_t TxBufferHead = 0;
uint16_t TxBufferTail = 0;

uint8_t RxBuffer[I2C_MAX_RX_BUFFER_SIZE];
uint16_t RxBufferSize = 0;
uint16_t RxBufferHead = 0;
uint16_t RxBufferTail = 0;

#define I2C_ADDRESS	0xE2
#define port gpioPortC //this is actually the port to be used for the efm32
#define sda_line 4 //this is the line that corresponds to the data out
#define scl_line 5 //^^^^ clock line
#define TRUE 1
#define FALSE 0
#define read 1 //might want these for later
#define write 0
 /*{clhr, enabled, frequency(0 to use defaults)
master mode, refrence frequency}*/

uint8_t I2C_TX = 0;

void master_init_i2c()
{
	I2C_Init_TypeDef init_I2c = I2C_INIT_DEFAULT;
	/*set the clock into start sequence for master mode*/

		//set up the clock and data lines
	GPIO_PinModeSet(port, sda_line, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(port, scl_line, gpioModeWiredAndPullUpFilter, 1);
	//enable pins
	I2C0->ROUTE = I2C_ROUTE_SDAPEN |
					I2C_ROUTE_SCLPEN |
					(1 << _I2C_ROUTE_LOCATION_SHIFT);
		I2C_Init(I2C0, &init_I2c);
}

void i2c_transmit(void) /*i need some input that tells an address, and some data for the buffer*/
{
	//I2C_TransferReturn_TypeDef done;

	I2C_TransferSeq_TypeDef Init_transfer;
	Init_transfer.addr = I2C_ADDRESS;
	Init_transfer.buf[0].data = TxBuffer; // put the buffer into the struct
	Init_transfer.buf[0].len = I2C_MAX_RX_BUFFER_SIZE; // this tells it how much memory to take up
	Init_transfer.flags = I2C_FLAG_WRITE;
	I2C_TransferInit(I2C0, &Init_transfer);


	while(I2C_Transfer(I2C0) == i2cTransferInProgress){
				;
	}
}






void i2c_recieve(void){


	//uint8_t myInfo = malloc(sizeof(uint*_t) * 4);
	I2C_TransferSeq_TypeDef Init_transfer;

    Init_transfer.addr = I2C_ADDRESS;
    Init_transfer.buf[0].data = RxBuffer; // put the buffer into the struct
    	Init_transfer.buf[0].len = I2C_MAX_RX_BUFFER_SIZE;
	Init_transfer.flags = I2C_FLAG_READ;
	I2C_TransferInit(I2C0, &Init_transfer);

	while(I2C_Transfer(I2C0) == i2cTransferInProgress){
					;
	}
}

void gpioCallback(uint8_t pin)
{
  if (pin == 9)
  {
    BSP_LedToggle(1);
    I2C_TX = 0;
  }
  else if (pin == 10)
  {
    BSP_LedToggle(0);
    I2C_TX = 1;
  }
  // Master Tx Data
  if (I2C_TX == 1) {
			  i2c_transmit();
		  }
		  // Master Rx Data
		  else {
			  i2c_recieve();
		  }
}



int main(void){

	CHIP_Init(); // Should be called at start
    CMU_ClockEnable(cmuClock_GPIO, true); // Enable GPIO peripheral clock
    /* Enable GPIO in CMU */

    /* Initialize GPIO interrupt dispatcher */
    GPIOINT_Init();

    /* Configure PB9 and PB10 as input */
    GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0);
    GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);

    /* Register callbacks before setting up and enabling pin interrupt. */
    GPIOINT_CallbackRegister(9, gpioCallback);
    GPIOINT_CallbackRegister(10, gpioCallback);

    /* Set falling edge interrupt for both ports */
    GPIO_IntConfig(gpioPortB, 9, false, true, true);
    GPIO_IntConfig(gpioPortB, 10, false, true, true);
	master_init_i2c();

}


