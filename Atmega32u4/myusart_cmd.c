/*
 * myusart_cmd.c
 *
 * Created: 2024/6/28 16:01:22
 *  Author: SZZ0030
 */ 
#include <stdint-gcc.h>
#include "myusart_cmd.h"
#include "crc.h"
#include "USBtoSerial.h"
#include "DAC7578.h"
#include "PCA9555.h"

static uint8_t usart_cmd_buff[USART_CMD_BUFF_LEN_MAX];
static uint8_t usart_cmd_buff_cnt=0;

static uint8_t reponse_buff[USART_CMD_RESPONSE_BUFF_LEN_MAX];

static void myusart_rev_cmd(void);
static void myusart_clr_buff(void);

/*****************************************set usart recevt timeout******

	and reset receive procedure

*/

static bool b_rev_start=false;
static int32_t rev_time_count =0;


void myusart_rev_settimeout(void)
{
	b_rev_start=true;
	rev_time_count=0;	
}

void myusart_rev_cout_plus(void)
{
	if(b_rev_start)
	{	
		rev_time_count++;		
	    myusart_rev_check_timeout();		
	}
}

void myusart_rev_check_timeout(void)
{
	if(rev_time_count > USART_RECV_PROCESS_TIMEOUT_MAX)
	{
		b_rev_start = false;
		rev_time_count=0;
		myusart_clr_buff();
	}
}
static void myusart_clr_buff(void)
{
	usart_cmd_buff_cnt=0;
	memset(usart_cmd_buff,0,USART_CMD_BUFF_LEN_MAX);
	
}
/*****************************************/


void myusart_rev(uint8_t rec)
{
	myusart_rev_settimeout();
	
	if(usart_cmd_buff_cnt < USART_CMD_BUFF_LEN_MAX)
	{
		if(usart_cmd_buff_cnt < USART_CMD_LEN)
		{
			usart_cmd_buff[usart_cmd_buff_cnt++] = rec;
			
			if(usart_cmd_buff_cnt == USART_CMD_LEN)
			{
					//
					myusart_rev_cmd();
					usart_cmd_buff_cnt=0;
			}		
			
		}
	
	}else
	{
		usart_cmd_buff_cnt=0;
	}
	
	
}


static void myusart_rev_cmd(void)
{
		uint16_t crc_value=0;
		uint8_t response_len=0;
		memset(reponse_buff,0,USART_CMD_RESPONSE_BUFF_LEN_MAX);
		
#if(CRC_ENBLE)	
		crc_value = do_crc(usart_cmd_buff,USART_CMD_LEN);
#endif	

			reponse_buff[response_len++] = RESPOSE_CODE;
			reponse_buff[response_len++] = usart_cmd_buff[0];
			
		if(!crc_value)
			switch(usart_cmd_buff[0])
			{
				case SET_ADC:
				{
					uint8_t sigl_all=0;
					
					if((usart_cmd_buff[1] > 23) && usart_cmd_buff[1]!=0xff)  
					{
						reponse_buff[response_len++]=0x03;break; // out of channel range
					}else
					{
						sigl_all = (usart_cmd_buff[1]==0xff)?ALL_CHAN_SET:SIGLE_CHAN_SET;
					}
						
					uint16_t set_ma=(uint16_t)usart_cmd_buff[2];
					set_ma+= ((uint16_t)usart_cmd_buff[3]<<8);
					
					if(set_ma >200)
					{
						reponse_buff[response_len++]=0x03; break;  //out of  current range
					}
					
					
					if(DAC_Set_Output_Value(sigl_all,set_ma,usart_cmd_buff[1]))
					{
						reponse_buff[response_len++]=0x02; // hardware fault
					}
					else					
					{
						reponse_buff[response_len++]=0x00; //success
					}
					 
					 break;
				}
				break;
				case GET_STS:
				{
					if(usart_cmd_buff[1] !=0xff)
					{
						reponse_buff[response_len++]=0x03;break;
					}
					
					if(PCA9555_Read_All_Input(&reponse_buff[response_len+1]))
					{
						reponse_buff[response_len++] = 0x02; break;
					}else
					{
						reponse_buff[response_len++] = 0x00;
						response_len+=6; // add pca9555 reading len
					}
					
					
				}
				break;				
				default:
				reponse_buff[response_len++] = 0x04;
				break;
			}
		else
		{
		
			reponse_buff[response_len++] = 0x01;
			
		}
		
			crc_value = do_crc(reponse_buff,response_len);
			reponse_buff[response_len++] = (uint8_t)crc_value;
			reponse_buff[response_len++] = (uint8_t)(crc_value>>8);
			
			
			USBtoSerial_PutIntoUsbBuff(reponse_buff,response_len);
						
			//USBtoSerial_PutIntoUsbBuff(usart_cmd_buff,USART_CMD_LEN);
		
}