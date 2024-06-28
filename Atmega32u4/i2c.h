/*
 * i2c.h
 *
 * Created: 2024/6/14 23:06:46
 *  Author: SZZ0030
 */ 


#ifndef I2C_H_
#define I2C_H_



#define TWPS_PRESCALER_1	0
#define TWPS_PRESCALER_4    1
#define TWPS_PRESCALER_16	2
#define TWPS_PRESCALER_64   3


#define		SCL				PIND0
#define		SDA				PIND1
#define		TWI_DDR			DDRD
#define		TWI_PORT		PORTD
#define		TWI_PIN			PIND


#define u8  uint8_t
#define u16 uint16_t



void Init_I2c(void);
u8 ii2c_readchar_8add(u8 devadd,u8 subadd_h);
u8 ii2c_writechar_8add(u8 devadd,u8 subadd_h,u8 data);
u16 ii2c_writechar_8add_16bit(u8 devadd,u8 subadd_h,u16 data);
u16 ii2c_readchar_8add_16bit(u8 devadd,u8 subadd_h);


u8 ii2c_writechar_cmd(u8 devadd,u8 subadd_h);


uint8_t ii2c_read_nosubaddr(uint8_t devadd,uint8_t *buff,uint8_t len);

uint8_t ii2c_write_nosubaddr(uint8_t deadd, uint8_t *buff, uint8_t len);


uint8_t ii2c_read_subaddr(uint8_t devadd,uint8_t subaddr, uint8_t *buff, uint8_t len);


uint8_t ii2c_scan_addr(uint8_t addr);


uint8_t Sda_Read(void);

#endif /* I2C_H_ */