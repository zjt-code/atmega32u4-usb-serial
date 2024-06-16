/*
 * USARTtoPrint.c
 *
 * Created: 2024/5/24 23:17:22
 *  Author: SZZ0030
 */ 
#include <stdio.h>
#include "USBtoSerial.h"
#include "USARTtoPrint.h"

static int uart_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,  _FDEV_SETUP_WRITE);

static int  uart_putchar(char c, FILE *stream)
{

	if (c == '\n')
	uart_putchar('\r', stream);
	while (!(Serial_IsSendReady()));
	//loop_until_bit_is_set(UCSR1A, UDRE1);
	UDR1 = c;
	return 0;
}


void Init_Usart_Print( void)
{
		stdout = &mystdout;		
}
	
/*
   int    main(void)
    {
	    init_uart();
	    stdout = &mystdout;
	    printf("Hello, world!\n");

	    return 0;
    }
	*/

