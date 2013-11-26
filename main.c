/*
 * main-old.c
 *
 *  Created on: Sep 27, 2013
 *      Author: bgouveia
 */




#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/debug.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/gpio.h>
#include <driverlib/ssi.h>
#include <driverlib/uart.h>
#include <stdint.h>
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include <string.h>

struct ROSCASDataFromRASPI
{
	int8_t v_linear;
	int8_t v_angular;
	uint8_t cmd;
};

struct ROSCASDataToRASPI
{
	uint8_t var1;
	uint8_t var2;
};

static unsigned long ulClockMS = 0;

int main(void)
{

	unsigned long ulDataRx;

	//MAP_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_12MHZ);
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |SYSCTL_XTAL_12MHZ);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);


	ulClockMS = SysCtlClockGet() / (3 * 1000);

	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_7); //PB7 output led
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE,GPIO_PIN_5); //PE5 output

	GPIOPinTypeSSI(GPIO_PORTE_BASE,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); //SPI1 output
	SSIDisable(SSI1_BASE);
	SSIConfigSetExpClk(SSI1_BASE,SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,SSI_MODE_SLAVE, 5000,8);
	SSIEnable(SSI1_BASE);

	//Uart
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioInit(0);

	UARTprintf("Init\n");

	/*for (int i=8;i;i--)
	      SSIDataPut(SSI1_BASE, 0xCC+i);        // put data in the transmit FIFO*/

	struct ROSCASDataFromRASPI cmd;
	struct ROSCASDataToRASPI send_data;

	send_data.var1 = 20;
	send_data.var2 = 20;

	uint8_t received_data = 0;
	uint8_t send_next_data = 0;

	uint8_t rec_buffer[20];
	memset(rec_buffer,0,20);

	//
	// Loop forever.
	//
	while(1)
	{
		/*MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,GPIO_PIN_7);
		MAP_SysCtlDelay(ulClockMS*500);
		MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,0);
		MAP_SysCtlDelay(ulClockMS*500);*/

		if(SSIDataGetNonBlocking(SSI1_BASE, &ulDataRx) == 0)  // if no more data
		{
			if(received_data == sizeof(struct ROSCASDataFromRASPI))
			{

				memcpy(&cmd, rec_buffer, received_data);
				received_data = 0;

				UARTprintf("Struct L = %d, A = %d, C = %d\r", cmd.v_linear, cmd.v_angular, cmd.cmd);
				send_next_data = cmd.cmd;

				send_data.var1 = cmd.v_linear;
				send_data.var2 = cmd.v_angular;
			}
		}
		else
		{

			//SSIDataPut(SSI1_BASE, 0xBB);
			//        SSIDataGet(SSI1_BASE, &ulDataRx);
			ulDataRx &= 0x00FF;
			rec_buffer[received_data] = ulDataRx;
			//SSIDataPut(SSI1_BASE, ulDataRx);
			UARTprintf("SSI byte %x\r", ulDataRx);

			received_data++;
		}

		if(send_next_data == 1)
		{
			int i;
			for(i = 0; i < (int)sizeof(struct ROSCASDataToRASPI); i++)
			{
				uint8_t *p_send_dada = (uint8_t *)&send_data;

//				SSIDataPutNonBlocking(SSI1_BASE, p_send_dada[i]);
//				UARTprintf("SSI wrote byte %x\r", p_send_dada[i]);

				SSIDataPutNonBlocking(SSI1_BASE, p_send_dada[i]);


			}
			send_next_data = 0;

			for(i = 0; i < (int)sizeof(struct ROSCASDataToRASPI); i++)
			{
				while(!SSIDataGetNonBlocking(SSI1_BASE, NULL));
			}
		}

	}
}
