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

uint8_t PCA9555_Read_Data(uint8_t devaddr,uint8_t cmd, uint8_t *buff, uint8_t buf_len)
{
	
	return ii2c_read_subaddr(devaddr,cmd,buff,buf_len);
	

}

uint8_t PCA9555_Config(uint8_t addr)
{
	uint8_t buff[]={addr,0x06,0xff,0xff};
	
	return ii2c_write_nosubaddr(addr,buff,sizeof(buff));

}