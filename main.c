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

#define BLINKY_BIT          0x80
#define ASK_DATA_BIT        0x40
#define HBRIDGEMODE_BIT     0x20
#define ASK_FIRMWARE_BIT    0x10
#define STARTSTOP_BIT       0x08

struct ROSCASDataFromRASPI
{
	int8_t v_linear;
	int8_t v_angular;
	uint8_t cmd;
};

typedef struct ROSCASDataToRASPI_
{
	int32_t left_encoder_count;
	int32_t right_encoder_count;
	uint8_t battery_voltage;
	uint8_t battery_current;

	uint8_t cenas;
	uint8_t cmd_back;
}ROSCASDataToRASPI;


static unsigned long ulClockMS = 0;

void SSIIntHandler(void);

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

	//IntMasterEnable();

	SSIIntRegister(SSI1_BASE, SSIIntHandler);
	SSIIntClear(SSI1_BASE, SSI_RXFF /*| SSI_RXTO*/);
	SSIIntEnable(SSI1_BASE, SSI_RXFF /*| SSI_RXTO*/);
	IntEnable(INT_SSI1);

	//Uart
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioInit(0);

	UARTprintf("Init\n");

	/*for (int i=8;i;i--)
	      SSIDataPut(SSI1_BASE, 0xCC+i);        // put data in the transmit FIFO*/

	struct ROSCASDataFromRASPI cmd;
	ROSCASDataToRASPI send_data;


	uint8_t received_data = 0;
	uint8_t send_next_data = 0;

	uint8_t rec_buffer[20];
	memset(rec_buffer,0,20);

	UARTprintf("size %d \n",sizeof( ROSCASDataToRASPI));

	//
	// Loop forever.
	//
	while(1)
	{

	}
}

void SSIIntHandler(void)
{
	SSIIntClear(SSI1_BASE, SSI_RXFF );

	static uint8_t bytes_left_to_receive = 0;
	static uint8_t last_send_byte_index = 0;

	uint8_t buff[12];
	unsigned long received_byte = 0, buffer_index =0;

	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,GPIO_PIN_7);

	while(SSIDataGetNonBlocking(SSI1_BASE, &received_byte))
	{
		if(bytes_left_to_receive)	// bytes_left_to_receive > 0
		{
			bytes_left_to_receive--;
		}
		else
		{
			//UARTprintf("rec %x \n", received_byte);
			buff[buffer_index] = received_byte;
			buff[buffer_index + 4] = received_byte+1;
			buff[buffer_index + 8] = received_byte+2;
			//UARTprintf("rec1 %x %d\n", buff[buffer_index], buffer_index);
			buffer_index++;
		}
	}


	//	UARTprintf("buf %x\n", buff[0]);
	//	UARTprintf("buf %x\n", buff[1]);
	//	UARTprintf("buf %x\n", buff[2]);
	//	UARTprintf("buf %x\n", buff[3]);

	//	for (int i = 0; i < 4; i++)
	//	{
	//		UARTprintf("buff %x\n", buff[i]);
	//	}
	//UARTprintf("rec %x %x %x %x\n", buff[0], buff[1], buff[2], buff[3]);
	//UARTprintf("rec %x \n", c1);

	for (int f = last_send_byte_index; f < 12; f++)
	{

		SSIDataPutNonBlocking(SSI1_BASE, buff[f]);

		bytes_left_to_receive += 1;

		unsigned long ulStatus = SSIIntStatus(SSI1_BASE,false);
		UARTprintf("%d - %x\n", f, ulStatus);
		if(/*(ulStatus & SSI_TXFF) ||*/(ulStatus & SSI_RXTO) || (ulStatus & SSI_RXFF))
		{
			last_send_byte_index = f+1;
			break;
		}
	}
	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,0);
}
