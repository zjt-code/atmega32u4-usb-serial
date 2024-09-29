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
#include <stdbool.h>


static bool dac_pow_on = false;
static uint8_t chn_on_off_flag_buff[3]={0}; // bit 0 means is pow off, else bit 1 means power on;
	

static uint8_t DAC7578_Init(uint8_t addr, uint16_t vol);
static uint8_t DAC7578_Set_Output_V(uint8_t addr,uint8_t ch, uint16_t vol);
static void	   DAC7578_Ch_PWR_Ctrl(uint8_t addr, uint8_t mode,uint8_t channel);
static void	   DAC7578_Soft_Reset(uint8_t addr, uint8_t reset_mode);


/***********************************************************************
 param:
       signle_all: 1-->all channel, 2-->signle channel
	   mA: rang 0-200
	   chn: 0-23
	                                                                
************************************************************************/


bool DAC_Get_Pwr_State(void)
{
	return dac_pow_on;
}


void DAC_Set_Pwr_State(bool state)
{
	
	dac_pow_on = state;
	
}
uint8_t DAC_Set_Output_Value(uint8_t signle_all,uint16_t mA,uint8_t chn)
{
	// mA is 0.1 precision, sample res = 250 ohm, so readback vol = mA * 25;
	// vol unit is mV
	
#if DAC_SET_MA_MODE	
	uint16_t vol = mA * 25; 
#else
	uint16_t vol = mA;
#endif
	
	uint8_t res=0;
	  
	if(signle_all == ALL_CHAN_SET)
	{
		//if(vol == 0) // power down all channel
		//{
				//res = DAC_All_Chn_Output(0);
				//DAC_Init(false);
		//}
		//else
		//{
			 //power on all channnel
				//DAC_Init(true);
				//res = DAC_All_Chn_Output(vol);
		//}
		
		DAC_All_Chn_Output(vol);
		DAC_Init((vol > 0)?1:0);			
	}
	else 
	if(signle_all == SIGLE_CHAN_SET)
	{
		
		uint8_t addr_arry[]={DAC7578_MB_ADDR,DAC7578_SB0_ADDR,DAC7578_SB1_ADDR};	
		
		res = DAC7578_Set_Output_V(addr_arry[chn/8],chn%8,vol);
		
		if(vol==0)
		{
				//power off  the "chn"
				
				chn_on_off_flag_buff[chn/8]&= ~(1<<(chn%8));
				DAC7578_Ch_PWR_Ctrl(addr_arry[chn/8],POWER_MODE_1K, (~chn_on_off_flag_buff[chn/8]));
				
				
		}
		else
		{
			 // power on the "chn"
				chn_on_off_flag_buff[chn/8]|=(1<<(chn%8));
				DAC7578_Ch_PWR_Ctrl(addr_arry[chn/8],POWER_MODE_ON,chn_on_off_flag_buff[chn/8]);
		}
		
		
	}
	
	return res;
}

uint8_t DAC_Init(bool b_on)
{

	if(!b_on)
	{
		DAC_Pwr_Ch_Mode_Set_(POWER_MODE_1K,0xff); // all channel  power down
		
		for(uint8_t x=0; x<3; x++)
		chn_on_off_flag_buff[x] = 0;
	}else
	{
		DAC_Pwr_Ch_Mode_Set_(POWER_MODE_ON,0xff); // all channel  power on
		
		for(uint8_t x=0; x<3; x++)
		chn_on_off_flag_buff[x] = 0xff;
	}
	
}


uint8_t DAC_All_Chn_Output(uint16_t vol)
{
		
	uint8_t res=DAC7578_Init(DAC7578_MB_ADDR,vol);
	if(res) return 1;
	
	res=DAC7578_Init(DAC7578_SB0_ADDR,vol);
	if(res) return 2;
	
	res=DAC7578_Init(DAC7578_SB1_ADDR,vol);
	if(res) return 3;	
	return 0;
}


void DAC_Pwr_Ch_Mode_Set_(uint8_t mode, uint8_t channel)
{
	DAC7578_Ch_PWR_Ctrl(DAC7578_MB_ADDR,mode,channel);
	DAC7578_Ch_PWR_Ctrl(DAC7578_SB0_ADDR,mode,channel);
	DAC7578_Ch_PWR_Ctrl(DAC7578_SB1_ADDR,mode,channel);
}


void DAC_RESET(uint8_t rst_mode)
{
	DAC7578_Soft_Reset(DAC7578_MB_ADDR,rst_mode);
	DAC7578_Soft_Reset(DAC7578_SB0_ADDR,rst_mode);
	DAC7578_Soft_Reset(DAC7578_SB1_ADDR,rst_mode);
	
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
	
#if DAC_SET_MA_MODE	

	uint16_t dac_value =(float)vol/(5000.0f/4096.0f);
#else

	uint16_t dac_value = vol;

#endif
	
	
	//printf("dac_value =%d\r\n", dac_value);
	
	dac_cmd_buf[0] =addr;
	dac_cmd_buf[1]= DAC7578_CMD_W_UPD_CH|ch;
	dac_cmd_buf[2]= (uint8_t)(dac_value>>4);
	dac_cmd_buf[3]= (uint8_t )(dac_value<<4);
	 
	return ii2c_write_nosubaddr(addr,dac_cmd_buf,4);
}


static void DAC7578_Ch_PWR_Ctrl(uint8_t addr, uint8_t mode, uint8_t channel)
{
	uint8_t dac_cmd_buf[4];
	
	dac_cmd_buf[0] =addr;
	dac_cmd_buf[1]= DAC7578_DAC_PWR_DOWN;
	dac_cmd_buf[2]= mode|(channel>>3);
	dac_cmd_buf[3]= (channel<<5);
	
    return ii2c_write_nosubaddr(addr,dac_cmd_buf,4);
	
}

static void DAC7578_Soft_Reset(uint8_t addr, uint8_t reset_mode)
{
	
		uint8_t dac_cmd_buf[4];
		
		dac_cmd_buf[0] =addr;
		dac_cmd_buf[1]= DAC7578_DAC_RESET;
		dac_cmd_buf[2]= reset_mode;
		dac_cmd_buf[3]= 0X00;
		
		return ii2c_write_nosubaddr(addr,dac_cmd_buf,4);
	
}