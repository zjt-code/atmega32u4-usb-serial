/*
 * PCA9555.c
 *
 * Created: 2024/6/18 17:26:47
 *  Author: SZZ0030
 */ 

#include <stdint-gcc.h>
#include <stdio.h>
#include "appconfig.h"
#include "i2c.h"
#include "PCA9555.h"


uint8_t PCA9555_All_Config(void)
{
		uint8_t res = PCA9555_Config(PCA9555_MB_ADDR);
		if(res) return 1;
		
		res = PCA9555_Config(PCA9555_SL0_ADDR);
		if(res) return 2;
		
		res = PCA9555_Config(PCA9555_SL1_ADDR);
		if(res) return 3;
		
		
		return 0;
}


uint8_t PCA9555_Read_All_Input(uint8_t *buff)
{
	uint8_t res = PCA9555_Read_Data(PCA9555_MB_ADDR,PCA9555_INPORT0,buff,2);
	if(res)return 1;
	
	res = PCA9555_Read_Data(PCA9555_SL0_ADDR,PCA9555_INPORT0,buff+2,2);
	if(res)return 2;
	
	res = PCA9555_Read_Data(PCA9555_SL1_ADDR,PCA9555_INPORT0,buff+4,2);
	if(res)return 3;
	
	return 0;
	
}
uint8_t PCA9555_Read_Data(uint8_t devaddr,uint8_t cmd, uint8_t *buff, uint8_t buf_len)
{
	
	return ii2c_read_subaddr(devaddr,cmd,buff,buf_len);
	

}


// config all ports as input

uint8_t PCA9555_Config(uint8_t addr)
{
	uint8_t buff[]={addr,0x06,0xff,0xff};
	
	return ii2c_write_nosubaddr(addr,buff,sizeof(buff));

}