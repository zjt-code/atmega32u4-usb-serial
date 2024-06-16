/*
 * SHT2X.h
 *
 * Created: 2024/6/15 16:17:50
 *  Author: SZZ0030
 */ 


#ifndef SHT2X_H_
#define SHT2X_H_

//Command

#define		Trig_T_Meas			0xe3
#define		Trig_RH_Meas		0xe5

#define		Trig_T_Meas_NoHold	0xf3
#define		Trig_RH_Meas_NoHold	0xf5

#define		SoftReset			0xfe


#define  SHT2X_ADDR				0X80

void Init_SHT2X(void);
void SHT2X_Read_T(void);


#endif /* SHT2X_H_ */