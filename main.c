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

typedef struct ROSCASDataFromRASPI_
{
	int8_t v_linear;
	int8_t v_angular;
	uint8_t cmd;
}ROSCASDataFromRASPI;

typedef struct ROSCASDataToRASPI_
{
	int32_t left_encoder_count;
	int32_t right_encoder_count;
	uint8_t battery_voltage;
	uint8_t battery_current;
	uint8_t cmd_back;
}ROSCASDataToRASPI;

typedef enum
{
	IDLE_STATE = 0, RECEIVING_STATE, SENDING_AFTER_RECEIVING, CONTINUE_RECEIVING_THEN_SENDING, IS_SENDING_RECEIVING_FINISHED
}SSI_Interrupt_State;


static unsigned long ulClockMS = 0;

void SSIIntHandler(void);

ROSCASDataFromRASPI struct_to_receive;
ROSCASDataToRASPI struct_to_send;

int main(void)
{

	//	unsigned long ulDataRx;
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
	//IntMasterEnable();
	UARTprintf("Init\n");
	UARTprintf("status ssi %x \n",SSIIntStatus(SSI1_BASE,0x0F));

	SSIIntRegister(SSI1_BASE, SSIIntHandler);
	SSIIntClear(SSI1_BASE, SSI_RXTO /*| SSI_RXFF*/);
	SSIIntEnable(SSI1_BASE, SSI_RXTO /*| SSI_RXFF*/);
	IntEnable(INT_SSI1);
	UARTprintf("status ssi %x \n",SSIIntStatus(SSI1_BASE,0x0F));



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
	SSIIntClear(SSI1_BASE, SSI_RXTO);

	static int8_t bytes_left_to_send = 0;
	static int8_t n_bytes_received = 0;
	static SSI_Interrupt_State state_interrupt = RECEIVING_STATE;

	uint8_t buff[12];
	static unsigned long received_byte = 0, buffer_index =0;

	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,GPIO_PIN_7);

	uint8_t *pointer_received = (uint8_t *)&struct_to_receive;
	uint8_t *pointer_send = (uint8_t *)&struct_to_send;

	switch(state_interrupt)
	{
	case RECEIVING_STATE:

		// Receives up to 4 bytes (minimum bytes to trigger the interrupt
		if(SSIDataGetNonBlocking(SSI1_BASE, &received_byte))
		{

			*(pointer_received + n_bytes_received) = received_byte;
			buffer_index++;
			n_bytes_received++;
			//UARTprintf("rec %x\n", received_byte);


		}

		if(n_bytes_received >= 3)
		{

			struct_to_send.left_encoder_count = struct_to_receive.v_linear;
			struct_to_send.right_encoder_count = -struct_to_receive.v_linear;
			struct_to_send.battery_voltage  = 20;
			struct_to_send.battery_current = 30;
			struct_to_send.cmd_back = struct_to_receive.cmd;

			state_interrupt = SENDING_AFTER_RECEIVING;
			bytes_left_to_send = sizeof( ROSCASDataToRASPI);
			SSIDataPutNonBlocking(SSI1_BASE, pointer_send[sizeof( ROSCASDataToRASPI) - bytes_left_to_send]);
			bytes_left_to_send--;
		}



		break;

	case SENDING_AFTER_RECEIVING:

		SSIDataGetNonBlocking(SSI1_BASE, &received_byte);

		if(bytes_left_to_send > 0)
		{
			SSIDataPutNonBlocking(SSI1_BASE, pointer_send[sizeof( ROSCASDataToRASPI) - bytes_left_to_send]);
			bytes_left_to_send--;
		}
		else
		{
			state_interrupt = RECEIVING_STATE;
			n_bytes_received = 0;
			buffer_index = 0;
		}


	break;
	}



	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,0);

}
