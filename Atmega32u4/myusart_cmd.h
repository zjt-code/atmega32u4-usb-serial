/*
 * myusart_cmd.h
 *
 * Created: 2024/6/28 16:02:02
 *  Author: SZZ0030
 */ 


#ifndef MYUSART_CMD_H_
#define MYUSART_CMD_H_

#include<stdint-gcc.h>


#define CRC_ENBLE		1

#define USART_CMD_BUFF_LEN_MAX				64
#define USART_CMD_LEN						6	
#define USART_CMD_RESPONSE_BUFF_LEN_MAX		32


// define command
#define  SET_ADC							0x1A
#define	 GET_STS							0x1B


#define  RESPOSE_CODE						0x2C


#define  USART_RECV_PROCESS_TIMEOUT_MAX		3

			


void myusart_rev(uint8_t rec);
void myusart_rev_settimeout(void);
void myusart_rev_cout_plus(void);
void myusart_rev_check_timeout(void);

#endif /* MYUSART_CMD_H_ */