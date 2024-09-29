/*
 * DAC7578.h
 *
 * Created: 2024/6/18 15:54:54
 *  Author: SZZ0030
 */ 

#include <stdint-gcc.h>
#include <stdbool.h>


#ifndef DAC7578_H_
#define DAC7578_H_


#define DAC7578_MB_ADDR			 0x98  // addr0 Float
#define DAC7578_SB0_ADDR		 0x90 // ADDR0 Conneted to GND
#define DAC7578_SB1_ADDR		 0x94 // addr0 connected to VCC


#define DAC7578_CMD_W_DAC_CH		0X00
#define DAC7578_CMD_UPD_CH_REG		0X10
#define DAC7578_CMD_W_UPD_ALL		0X20
#define DAC7578_CMD_W_UPD_CH		0X30


#define DAC7578_DAC_PWR_DOWN		0X40


#define POWER_MODE_ON					(0X00<<5)
#define POWER_MODE_1K					(0X01<<5)
#define POWER_MODE_100K					(0X02<<5)
#define POWER_MODE_H_Z					(0X03<<5)


#define DAC7578_DAC_RESET			0x70

#define DAC7578_RSTMODE_POW			0X00
#define DAC7578_RSTMODE_TO_HS		0X40
#define DAC7578_RSTMODE_M_HS		0X80


#define DAC_CH_A					0
#define DAC_CH_B					1
#define DAC_CH_C					2
#define DAC_CH_D					3

#define DAC_CH_E					4
#define DAC_CH_F					5
#define DAC_CH_G					6
#define DAC_CH_H					7


#define ALL_CHAN_SET				1
#define SIGLE_CHAN_SET				2



#define DAC_SET_MA_MODE				0
	


uint8_t DAC_Init(bool b_on);
uint8_t DAC_All_Chn_Output(uint16_t vol);
uint8_t DAC_Set_Output_Value(uint8_t signle_all,uint16_t mA,uint8_t chn);

void DAC_Set_Pwr_State(bool state);
bool DAC_Get_Pwr_State(void);

void DAC_Pwr_Ch_Mode_Set_(uint8_t mode, uint8_t channel);
void DAC_RESET(uint8_t rst_mode);



#endif /* DAC7578_H_ */