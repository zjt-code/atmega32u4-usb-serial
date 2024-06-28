/*
 * myusart_cmd.c
 *
 * Created: 2024/6/28 16:01:22
 *  Author: SZZ0030
 */ 
#include"myusart_cmd.h"
#include "stdint-gcc.h"

static uint8_t usart_cmd_buff[USART_CMD_BUFF_LEN_MAX];
static uint8_t usart_cmd_buff_cnt=0;

void myusart_rev(uint8_t rec)
{
	
}