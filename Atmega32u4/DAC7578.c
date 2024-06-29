/*
 * DAC7578.c
 *
 * Created: 2024/6/18 15:54:31
 *  Author: SZZ0030
 */

#include <stdint-gcc.h>
#include <stdio.h>
#include "appconfig.h"
#include "i2c.h"
#include "DAC7578.h"


static uint8_t DAC7578_Init(uint8_t addr, uint16_t vol);
static uint8_t DAC7578_Set_Output_V(uint8_t addr,uint8_t ch, uint16_t vol);

/***********************************************************************
 param:
       signle_all: 1-->all channel, 2-->signle channel
	   mA: rang 0-200
	   chn: 0-23
	                                                                
************************************************************************/

uint8_t DAC_Set_Output_Value(uint8_t signle_all,uint16_t mA,uint8_t chn)
{
	// mA is 0.1 precision, sample res = 250 ohm, so readback vol = mA * 25;
	// vol unit is mV
	uint16_t vol = mA * 25; 
	uint8_t res=0;
	  
	if(signle_all == ALL_CHAN_SET)
	{
		res = DAC_Init_Output(vol);
		
	}else if(signle_all == SIGLE_CHAN_SET)
	{
		uint8_t addr_arry[]={DAC7578_MB_ADDR,DAC7578_SB0_ADDR,DAC7578_SB1_ADDR};
			
		
		res = DAC7578_Set_Output_V(addr_arry[chn/8],chn%8,vol);
		
	}
	
	return res;
}

uint8_t DAC_Init_Output(uint16_t vol)
{
	
	uint8_t res=DAC7578_Init(DAC7578_MB_ADDR,vol);
	if(res) return 1;
	
	 res=DAC7578_Init(DAC7578_SB0_ADDR,vol);
	if(res) return 2;
	
	 res=DAC7578_Init(DAC7578_SB1_ADDR,vol);
	if(res) return 3;
	
	return 0;
}

static uint8_t DAC7578_Init(uint8_t addr, uint16_t vol)
{
	uint8_t res=0,x=0;
	
	for(int8_t x=DAC_CH_A; x < (DAC_CH_H +1); x++)
		{
		
			res = DAC7578_Set_Output_V(addr,x,vol);
			if(res)return res;
		}
			
	return res;
}


static uint8_t DAC7578_Set_Output_V(uint8_t addr,uint8_t ch, uint16_t vol)
{
	uint8_t dac_cmd_buf[4];
	
	uint16_t dac_value =(float)vol/(5000.0f/4096.0f);
	
	dac_cmd_buf[0] =addr;
	dac_cmd_buf[1]= DAC7578_CMD_W_UPD_CH|ch;
	dac_cmd_buf[2]= (uint8_t)(dac_value>>4);
	dac_cmd_buf[3]= (uint8_t )(dac_value<<4);
	 
	return ii2c_write_nosubaddr(addr,dac_cmd_buf,4);
}