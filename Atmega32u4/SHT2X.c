/*
 * SHT2X.c
 *
 * Created: 2024/6/15 16:12:16
 *  Author: SZZ0030
 */ 

#include <stdint-gcc.h>
#include <stdbool.h>
#include <stdio.h>

#include "USARTtoPrint.h"
#include "i2c.h"
#include "SHT2X.h"

void Init_SHT2X(void)
{
	
	uint8_t res = ii2c_writechar_cmd(0x80, 0xfe); //reset			
//printf("ii2c_write reset = %u\r\d", res);
	
}

static bool t_start=false;
static uint8_t meat_data[3];
static float sensor_temp;


uint16_t decode16(uint8_t *buff)
{
	return((uint16_t)*buff) << 8 | ((uint16_t)*(buff+1));
}

void SHT2X_Read_T(void)
{
	if(!t_start)
	{
		// START T MEASUREMENT
		ii2c_writechar_cmd(SHT2X_ADDR, Trig_T_Meas_NoHold);
		t_start=true;
	}
	else
	{
		//READ DATA
		uint8_t res = ii2c_read_nosubaddr(SHT2X_ADDR,meat_data,3);
		printf("read measuret res=%u,0x%02x,0x%02x,0x%02x,\r\n",res,meat_data[0],meat_data[1],meat_data[2]);
		if(!res){
			
			 sensor_temp = (float)(decode16(meat_data)& 0x7fff) * 175.72f/65536.0f -46.85;
			 printf("temp = %d\r\n",(int32_t)(sensor_temp*100));
			
		}
		else{
			printf("Read temp fail res = %d\r\n",res);
		}
		t_start = false;
	}
}

