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


uint8_t DAC7578_Init(void)
{
	uint8_t res=0,x=0;
	
	for(int8_t x=DAC_CH_A; x < (DAC_CH_H +1); x++)
	res = DAC7578_Set_Output_V(DAC7578_MB_ADDR,x,1000);
			
	return res;
}


uint8_t DAC7578_Set_Output_V(uint8_t addr,uint8_t ch, uint16_t vol)
{
	uint8_t dac_cmd_buf[4];
	
	uint16_t dac_value =(float)vol/(5000.0f/4096.0f);
	
	dac_cmd_buf[0] =addr;
	dac_cmd_buf[1]= DAC7578_CMD_W_UPD_CH|ch;
	dac_cmd_buf[2]= (uint8_t)(dac_value>>4);
	dac_cmd_buf[3]= (uint8_t )(dac_value<<4);
	
	uint8_t ret = ii2c_write_nosubaddr(addr,dac_cmd_buf,4);

   /*if(DEBUG_USART_ENABLE)
	
	printf("Set ch[%d],vol =%d mV, RES = %d\r\n",ch,vol,ret);
			
	#endif
	*/
	return ret;
}