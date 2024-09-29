/*
 * PCA9555.h
 *
 * Created: 2024/6/18 17:27:11
 *  Author: SZZ0030
 */ 


#ifndef PCA9555_H_
#define PCA9555_H_

#define PCA9555_MB_ADDR		0X42     // J13 VCC, J12->GND
#define PCA9555_SL0_ADDR    0X40    //  J13 GND, J12->GND
#define PCA9555_SL1_ADDR    0X44   //   J13 GND, J12->VCC


#define PCA9555_INPORT0  0
#define PCA9555_INPORT1  1



uint8_t PCA9555_All_Config(void);

uint8_t PCA9555_Read_All_Input(uint8_t *buff);

uint8_t PCA9555_Read_Data(uint8_t devaddr,uint8_t cmd, uint8_t *buff, uint8_t buf_len);
uint8_t PCA9555_Config(uint8_t addr);

#endif /* PCA9555_H_ */