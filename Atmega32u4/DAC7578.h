/*
 * DAC7578.h
 *
 * Created: 2024/6/18 15:54:54
 *  Author: SZZ0030
 */ 

#include <stdint-gcc.h>


#ifndef DAC7578_H_
#define DAC7578_H_


#define DAC7578_MB_ADDR			 0x98
#define DAC7578_SB0_ADDR		 0x90
#define DAC7578_SB1_ADDR		 0x94


#define DAC7578_CMD_W_DAC_CH		0X00
#define DAC7578_CMD_UPD_CH_REG		0X10
#define DAC7578_CMD_W_UPD_ALL		0X20
#define DAC7578_CMD_W_UPD_CH		0X30

#define DAC_CH_A					0
#define DAC_CH_B					1
#define DAC_CH_C					2
#define DAC_CH_D					3

#define DAC_CH_E					4
#define DAC_CH_F					5
#define DAC_CH_G					6
#define DAC_CH_H					7




uint8_t DAC7578_Init(void);
uint8_t DAC7578_Set_Output_V(uint8_t addr,uint8_t ch, uint16_t vol);

#endif /* DAC7578_H_ */