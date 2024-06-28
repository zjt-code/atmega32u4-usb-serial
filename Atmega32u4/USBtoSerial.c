/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the USBtoSerial project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include <stdio.h>


#include "USBtoSerial.h"
#include "USARTtoPrint.h"
#include "timer.h"
#include "i2c.h"
#include "SHT2X.h"
#include "DAC7578.h"
#include "PCA9555.h"

/** Pulse generation counters to keep track of the number of milliseconds remaining for each pulse type */
#define TX_RX_LED_PULSE_MS 100

/** Circular buffer to hold data from the host before it is sent to the device via the serial port. */
static RingBuffer_t USBtoUSART_Buffer;

/** Underlying data buffer for \ref USBtoUSART_Buffer, where the stored bytes are located. */
static uint8_t      USBtoUSART_Buffer_Data[128];

/** Circular buffer to hold data from the serial port before it is sent to the host. */
static RingBuffer_t USARTtoUSB_Buffer;

/** Underlying data buffer for \ref USARTtoUSB_Buffer, where the stored bytes are located. */
static uint8_t      USARTtoUSB_Buffer_Data[128];

/** Pulse generation counters to keep track of the number of milliseconds remaining for each pulse type */
volatile struct
{
    uint8_t TxLEDPulse; /**< Milliseconds remaining for data Tx LED pulse */
    uint8_t RxLEDPulse; /**< Milliseconds remaining for data Rx LED pulse */
    uint8_t PingPongLEDPulse; /**< Milliseconds remaining for enumeration Tx/Rx ping-pong LED pulse */
} PulseMSRemaining;

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint                 =
					{
						.Address                = CDC_TX_EPADDR,
						.Size                   = CDC_TXRX_EPSIZE,
						.Banks                  = 1,
					},
				.DataOUTEndpoint                =
					{
						.Address                = CDC_RX_EPADDR,
						.Size                   = CDC_TXRX_EPSIZE,
						.Banks                  = 1,
					},
				.NotificationEndpoint           =
					{
						.Address                = CDC_NOTIFICATION_EPADDR,
						.Size                   = CDC_NOTIFICATION_EPSIZE,
						.Banks                  = 1,
					},
			},
	};

/*
void putchar(char c)
{
	Serial_SendByte(c);
}
*/

static uint8_t CMDToHost[128];
static uint8_t CMDToSlave[128];
static bool b_connect = false;

void Led_txrx_Init(void)
{
	PORTB&=~0X01; // Set PB0
	DDRB|=0x01; //PB0 output
	
	DDRD|=0X20; //PD5 output
	PORTD&=~0X20; // set pd5
	
	DDRD|=(1<<PORTD7)	; //PD7 output
	PORTD&=~(1<<PORTD7); // set pd7
	
}

void Led_rx_onff(bool onff)
{
	if(onff)
		PORTB&=~0X01;
	else
		PORTB|=0X01;
}

void Led_tx_onff(bool onff)
{
	if(onff)
		PORTD&=~0X20;
		else
		PORTD|=0X20;
}
/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	RingBuffer_InitBuffer(&USBtoUSART_Buffer, USBtoUSART_Buffer_Data, sizeof(USBtoUSART_Buffer_Data));
	RingBuffer_InitBuffer(&USARTtoUSB_Buffer, USARTtoUSB_Buffer_Data, sizeof(USARTtoUSB_Buffer_Data));

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);	
	Init_Usart_Print();
	Led_txrx_Init();
	Timer_Init();
	Init_I2c();
	//Init_SHT2X();
	
	DAC7578_Init();
	
	GlobalInterruptEnable();

	for (;;)
	{
		/* Only try to read in bytes from the CDC interface if the transmit buffer is not full */
		if (!(RingBuffer_IsFull(&USBtoUSART_Buffer)))
		{
			int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);

			/* Store received byte into the USART transmit buffer */
			if (!(ReceivedByte < 0))
			  RingBuffer_Insert(&USBtoUSART_Buffer, ReceivedByte);
		}

		uint16_t BufferCount = RingBuffer_GetCount(&USARTtoUSB_Buffer);
		if (BufferCount)
		{
			Endpoint_SelectEndpoint(VirtualSerial_CDC_Interface.Config.DataINEndpoint.Address);

			/* Check if a packet is already enqueued to the host - if so, we shouldn't try to send more data
			 * until it completes as there is a chance nothing is listening and a lengthy timeout could occur */
			if (Endpoint_IsINReady())
			{
			    //LEDs_TurnOnLEDs(LEDS_LED1);
				Led_tx_onff(true);
			    PulseMSRemaining.TxLEDPulse = TX_RX_LED_PULSE_MS;
			
				/* Never send more than one bank size less one byte to the host at a time, so that we don't block
				 * while a Zero Length Packet (ZLP) to terminate the transfer is sent if the host isn't listening */
				uint8_t BytesToSend = MIN(BufferCount, (CDC_TXRX_EPSIZE - 1));
				printf("send data to host\r\n");
				/* Read bytes from the USART receive buffer into the USB IN endpoint */
				while (BytesToSend--)
				{
					/* Try to send the next byte of data to the host, abort if there is an error without dequeuing */
					if (CDC_Device_SendByte(&VirtualSerial_CDC_Interface,
											RingBuffer_Peek(&USARTtoUSB_Buffer)) != ENDPOINT_READYWAIT_NoError)
					{
						break;
					}

					/* Dequeue the already sent byte from the buffer now we have confirmed that no transmission error occurred */
					RingBuffer_Remove(&USARTtoUSB_Buffer);
				}
			}
		}

		/* Load the next byte from the USART transmit buffer into the USART if transmit buffer space is available */
		if (Serial_IsSendReady() && !(RingBuffer_IsEmpty(&USBtoUSART_Buffer)))
        {
			  Serial_SendByte(RingBuffer_Remove(&USBtoUSART_Buffer));
              // add command hear , usart command interpter
			  
		  	//LEDs_TurnOnLEDs(LEDS_LED1);
			  Led_rx_onff(true);
			  
		  	  PulseMSRemaining.RxLEDPulse = TX_RX_LED_PULSE_MS;
			  
			  printf("send data to usart \r\n");
        }            

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
                
        /* Turn off TX LED(s) once the TX pulse period has elapsed */
        if (PulseMSRemaining.TxLEDPulse && !(--PulseMSRemaining.TxLEDPulse))
        //LEDs_TurnOffLEDs(LEDS_LED1);
		Led_tx_onff(false);

        /* Turn off RX LED(s) once the RX pulse period has elapsed */
        if (PulseMSRemaining.RxLEDPulse && !(--PulseMSRemaining.RxLEDPulse))
        //LEDs_TurnOffLEDs(LEDS_LED1);
		Led_rx_onff(false);
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();

    /* Start the flush timer so that overflows occur rapidly to push received bytes to the USB interface */
    TCCR0B = (1 << CS02);
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	//LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
	//b_connect =true;
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	//b_connect = false;
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** ISR to manage the reception of data from the serial port, placing received bytes into a circular buffer
 *  for later transmission to the host.
 */
ISR(USART1_RX_vect, ISR_BLOCK)
{
	uint8_t ReceivedByte = UDR1;

	if ((USB_DeviceState == DEVICE_STATE_Configured) && !(RingBuffer_IsFull(&USARTtoUSB_Buffer)))
	  RingBuffer_Insert(&USARTtoUSB_Buffer, ReceivedByte);
}

/** Event handler for the CDC Class driver Line Encoding Changed event.
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
	uint8_t ConfigMask = 0;

	switch (CDCInterfaceInfo->State.LineEncoding.ParityType)
	{
		case CDC_PARITY_Odd:
			ConfigMask = ((1 << UPM11) | (1 << UPM10));
			break;
		case CDC_PARITY_Even:
			ConfigMask = (1 << UPM11);
			break;
	}

	if (CDCInterfaceInfo->State.LineEncoding.CharFormat == CDC_LINEENCODING_TwoStopBits)
	  ConfigMask |= (1 << USBS1);

	switch (CDCInterfaceInfo->State.LineEncoding.DataBits)
	{
		case 6:
			ConfigMask |= (1 << UCSZ10);
			break;
		case 7:
			ConfigMask |= (1 << UCSZ11);
			break;
		case 8:
			ConfigMask |= ((1 << UCSZ11) | (1 << UCSZ10));
			break;
	}

	/* Keep the TX line held high (idle) while the USART is reconfigured */
	PORTD |= (1 << 3);

	/* Must turn off USART before reconfiguring it, otherwise incorrect operation may occur */
	UCSR1B = 0;
	UCSR1A = 0;
	UCSR1C = 0;

	/* Set the new baud rate before configuring the USART */
	UBRR1  = SERIAL_2X_UBBRVAL(CDCInterfaceInfo->State.LineEncoding.BaudRateBPS);

	/* Reconfigure the USART in double speed mode for a wider baud rate range at the expense of accuracy */
	UCSR1C = ConfigMask;
	UCSR1A = (1 << U2X1);
	UCSR1B = ((1 << RXCIE1) | (1 << TXEN1) | (1 << RXEN1));

	/* Release the TX line after the USART has been reconfigured */
	PORTD &= ~(1 << 3);
}

/** Event handler for control line state change on a CDC interface.
 *
 * \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced 
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
    bool CurrentDTRState = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR);
}

static uint16_t cnt=0;
static uint8_t addr =1;
static uint8_t input_buf[2];

void Timer1_Handler()
{
	
	    uint8_t res1, res2;
		
		//res = DAC7578_Init();
		
		
		
		
		cnt++;
		
		if(cnt > 30)
		{
			    res1 = DAC7578_Init();
			   // res = ii2c_scan_addr(addr);
			    res2 = PCA9555_Read_Data(PCA9555_SL0_ADDR,0,input_buf,2);
			 
				//res = PCA9555_Config(PCA9555_SL0_ADDR);
				
				memset(CMDToHost,0,sizeof(CMDToHost));
				
				sprintf(CMDToHost,"timer1 interrupt =%d,res1 = %d, res2=%d,input[0]=%d,input[1]=%d, addr=%d\r\n",cnt,res1,res2,
				input_buf[0],input_buf[1],addr);
				
				int len = strlen(CMDToHost) + 2; // add 2 bytes \r\n
				for(int x=0; x< len; x++)
				RingBuffer_Insert(&USARTtoUSB_Buffer,CMDToHost[x]);
				
			   addr+=2;
		}
		
		

//
		//uint8_t res = ii2c_writechar_cmd(0x80, 0xfe); //reset
//
		//printf("ii2c_write reset = %u\r\d", res);
		
	//	SHT2X_Read_T();
	
	if(PIND &(1<<PIND7))
	{
		PORTD&=~(1<<PORTD7);
			//TWI_PORT |=(1<<SCL);
			//TWI_PORT |=(1<<SCL);
	}else
	{
		PORTD|=(1<<PORTD7);
			//TWI_PORT &=~(1<<SCL);
			//TWI_PORT |=(1<<SCL);
	}
	
}

