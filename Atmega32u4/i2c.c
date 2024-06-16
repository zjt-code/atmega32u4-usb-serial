/*
 * i2c.c
 *
 * Created: 2024/6/14 23:06:29
 *  Author: SZZ0030
 */ 

#include <stdbool.h>
#include <avr/interrupt.h>
#include "i2c.h"




static void Sda_Set_Dir(bool b_input);
static void Sda_Set_value(bool b_input);
static void Scl_Set_Value(bool b_input);

#define SDA_OUT()	Sda_Set_Dir(false)
#define SDA_IN()	Sda_Set_Dir(true)

// SCL Frequency = CPU_CLOCK/(16 + 2*(TWBR)*4 **TWPS )
// current for scl frequency =100K

void Init_I2c(void)
{

	//TWBR = 18;  //TWBR should more than 10 in master mode ,
	//
	//TWSR = TWPS_PRESCALER_1;

	TWI_DDR |= (1<<SDA); // OUTPUT
	TWI_DDR |= (1<<SCL);
	TWI_PORT |=(1<<SDA);
	TWI_PORT |=(1<<SCL);
	
}
//
//// twi interrupt service
//ISR(TWI_vect)
//{
//
//}

uint8_t Sda_Read(void)
{
	// enable input
	//TWI_DDR &= ~(1<< SDA);
	
	//Sda_Set_Dir(true);
	// read input value
	return ( TWI_PIN & (1<<SDA)) ? 1 : 0;
}

static void Sda_Set_Dir(bool b_input)
{
	if(b_input)	{
		// enable input
		TWI_DDR &= ~(1<< SDA);
	}else
	{
		TWI_DDR |=  (1<<SDA); //Enable output	
	}
}


static void Sda_Set_value(bool b_input)
{
	if(b_input)
		TWI_PORT |=(1<<SDA);
	else
		TWI_PORT &= ~(1<<SDA);
}

static void Scl_Set_Value(bool b_input)
{
	if(b_input)
		TWI_PORT |=(1<<SCL);
	else
		TWI_PORT &=~(1<<SCL);
}

void IIC_delay1us(uint16_t delay)
{
	uint32_t i;
	i = delay << 3;  //  ?1us  ?8    ?delay * 8
	for(;i>0;i--) ;
}

static void delay_us(uint32_t us)
{
	uint32_t i;
	i = us << 3;  //  ?1us  ?8    ?delay * 8
	for(;i>0;i--) ;
}
   
void iicstart2(void) //?????,???????,??sck?0?1,???0(?????),??sda??????
{
    SDA_OUT();
    Sda_Set_value(1);//sda=1;
    delay_us(10);
    Scl_Set_Value(1);//scl=1;
    delay_us(10);
	Sda_Set_value(0);// sda=0;
    delay_us(10);
    Scl_Set_Value(0);//scl=0;
    delay_us(10);
}
/****************************????****************************
IIC????:when scl=0;make sda fr 0 to 1,finaly all high means free
??????
??:  ???????      1X(0)
??: ??,???????,????,???
        ??,???????,????,????
****************************************************************/
void iicstop2(void)
{
    
    SDA_OUT();//sda     
	//IIC_SCL=0;
	Scl_Set_Value(0);
	IIC_delay1us(10);
	Sda_Set_value(0);//IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	IIC_delay1us(10);
	//IIC_SCL=1; 
	Scl_Set_Value(1);
	IIC_delay1us(10);    
	Sda_Set_value(1);//IIC_SDA=1;//    I2C   ?    ? 
	IIC_delay1us(10);		//stop?    ?10ms	
}
/****************************????****************************
IIC??????:
include ack and noack,ackbit=1 means NOACK used in usual;   
read a byte then ack;read the end byte then noack;
****************************************************************/
void iicsendack2(u8 ackbit)
{
    //ackbit=1; 
    SDA_OUT();
    if(ackbit)
     Sda_Set_value(1);//sda=1;      //NO ACK    
    else
     Sda_Set_value(0);//sda=0;     //ACK
    IIC_delay1us(10);  
   // scl=1;
	Scl_Set_Value(1);
    IIC_delay1us(10);  
   // scl=0;
	Scl_Set_Value(0);
    IIC_delay1us(10);  
    Sda_Set_value(1);//sda=1;//??: ??ACK0??????;????sda
}
/****************************????****************************
IIC??????:
include ack an noack       1X-> 1X(0)
slave recv a byte then reply ack, otherwise reply noack;
****************************************************************/
u8 iicrecvack2(void)
{
    u8 ackbit=0;
    SDA_IN();
    Sda_Set_value(1);// sda=1;PULL UP
  //  scl=0;        //add
    IIC_delay1us(10);    //1/3T
    //scl=1;
	Scl_Set_Value(1);
    IIC_delay1us(10); 	//add	
    //_nop_();_nop_();    
    ackbit=100;      //1/3T,H????????0,????? (???,sda?????(9th)??????slave????)
    while(ackbit)
    {
     if(!Sda_Read()) break; 
     ackbit--;       
    }
	  IIC_delay1us(10); //??????????,????????????
		//scl=0;
	  Scl_Set_Value(0);
	  IIC_delay1us(10); //1/3T  add
    return(!ackbit);     //0????
}
/****************************????****************************
IIC??????:      10->1X
****************************************************************/
void iicsendbyte2(u8 senddata)
{
    u8 temp;
    u8 i;
    SDA_OUT();
    IIC_delay1us(3);   //1/3T
    temp=senddata;

    for(i=0;i<8;i++)
    {
        if(temp&0x80)
        {
            Sda_Set_value(1);//sda=1;
        }
        else
        {
           Sda_Set_value(0);// sda=0;
        }
        IIC_delay1us(10);  //1/3T
		
        //scl=1;
		Scl_Set_Value(1);
        IIC_delay1us(10);   //1/3T
				
		Scl_Set_Value(0);//scl=0;           
        IIC_delay1us(10);     //1/3T
        temp<<=1;
      
    }
    //ACK
   Scl_Set_Value(1);// sda=1;//??sda??      ?????slave????0?1

	
	
//	uchar t;       
//	scl=0;//??????????
//	NOP;NOP;NOP;
//	for(t=0;t<8;t++)
//	{              
//		sda=(senddata&0x80)>>7;
//		senddata<<=1; 	
//		scl=0;	
//		NOP;NOP;NOP;			
//			 //?TEA5767??????????
//		scl=1;
//		NOP;NOP;NOP;; 
//		scl=0;	
//		NOP;NOP;NOP;
////	}
}

//void iicsendbyte2(uchar senddata)
//{

//    uchar temp;
//    uchar i;
//    temp=senddata;

//    for(i=0;i<8;i++)
//    {
//        if(temp&0x80)
//        {
//            sda=1;
//        }
//        else
//        {
//            sda=0;
//        }
//        NOP;NOP;NOP;    //1/3T
//        scl=1;
//        NOP;NOP;NOP;    //1/3T
//        temp<<=1;
//        scl=0;
//        NOP;NOP;NOP;    //1/3T
//    }
//    //ACK
//    sda=1;//??sda??      ?????slave????0?1
//}

/****************************????****************************
IIC??????:
****************************************************************/
u8 iicrecvbyte2(void)
{   
    u8 i;
    u8 Read_byte = 0;
    SDA_IN();
    for(i=0;i<8;i++)
    {
        IIC_delay1us(10);    //1/3T
        Scl_Set_Value(1); //scl=1;
        IIC_delay1us(10);     //1/3T   ??????????
        Read_byte <<= 1;
        if(Sda_Read())
        {
            Read_byte |= 0x01;  
        }
        Scl_Set_Value(0);//scl=0;
        IIC_delay1us(10);    //1/3T
    }
    //I2C_Send_Nack();
    return( Read_byte);
}


u16 ii2c_readchar_8add_16bit(u8 devadd,u8 subadd_h)
{
//	u8 subadd_high,subadd_low;
	u16 data1,data2;
	//devadd = devadd<<1;
 
	//IIC_Start(); 
    iicstart2();
//	delay_us(10);//5
	iicsendbyte2(devadd);
	if(iicrecvack2())
	return 0xffff; 
	iicsendbyte2(subadd_h);
	if(iicrecvack2())
	return 0xffff;
 //  	IIC_Stop(); 

    
	iicstart2(); 
//	delay_us(10);//5 
	iicsendbyte2(devadd | 0x01);
	if(iicrecvack2())
	return 0xffff;
	data1 = iicrecvbyte2();
	 iicsendack2(0);  //ack
	data2 = iicrecvbyte2();
	 iicsendack2(1);//no ack
	iicstop2(); 
	data2 = ((data2<<8)&0xFF00)+(data1&0x00FF);
	
	return data2;
}

uint8_t ii2c_read_nosubaddr(uint8_t devadd,uint8_t *buff,uint8_t len)
{
	//	u8 subadd_high,subadd_low;
	//u16 data1,data2;
	uint8_t i_index=0;
	
	//devadd = devadd<<1;
	
	//IIC_Start();
	//iicstart2();
	////	delay_us(10);//5
	//iicsendbyte2(devadd);
	//if(iicrecvack2())
	//return 0xff;
	//iicsendbyte2(subadd_h);
	//if(iicrecvack2())
	//return 0xffff;
	//  	IIC_Stop();

	
	iicstart2();
	//	delay_us(10);//5
	iicsendbyte2(devadd | 0x01);
	if(iicrecvack2())
	return 0xff;
	for(i_index=0; i_index<(len-1);i_index++)
	{
			buff[i_index] = iicrecvbyte2();
			iicsendack2(0);  //ack
	}
	buff[i_index] = iicrecvbyte2();
	iicsendack2(1);//no ack
	iicstop2();
	//data2 = ((data2<<8)&0xFF00)+(data1&0x00FF);
	
	return 0x00;
}

u16 ii2c_writechar_8add_16bit(u8 devadd,u8 subadd_h,u16 data)
{
//	u8 subadd_high,subadd_low;
	//devadd = devadd<<1;
 
	iicstart2();
	//delay_us(2);
	iicsendbyte2(devadd);
	if(iicrecvack2())
	return 0xffff; 
	iicsendbyte2(subadd_h);
	if(iicrecvack2())
	return 0xffff;
    
	iicsendbyte2((u8)(data>>8));
	if(iicrecvack2())
	return 0xffff;  
	iicsendbyte2((u8)data);
	if(iicrecvack2())
	return 0xffff;
	iicstop2();
   return 0;
}


u8 ii2c_writechar_8add(u8 devadd,u8 subadd_h,u8 data)
{
	   
  iicstart2();	
  iicsendbyte2(devadd);	
	if(iicrecvack2())
	return 0xff; 
	iicsendbyte2(subadd_h);
  if(iicrecvack2())
	return 0xff;     
	iicsendbyte2(data);
	if(iicrecvack2())
	return 0xff;
	iicstop2();
  return 0;
}


u8 ii2c_readchar_8add(u8 devadd,u8 subadd_h)
{

	u8 data;	
 
	iicstart2();
	iicsendbyte2(devadd);
	if(iicrecvack2())
	return 0xff; 
	iicsendbyte2(subadd_h);
	if(iicrecvack2())
	return 0xff;
    
	iicstart2(); 	
	iicsendbyte2(devadd | 0x01);
	if(iicrecvack2())
	return 0xff;
	data = iicrecvbyte2();
	iicsendack2(1);//no ack
	iicstop2();
	return data;
}

u8 ii2c_writechar_cmd(u8 devadd,u8 subadd_h)
{
	   
  iicstart2();	
  iicsendbyte2(devadd);	
  if(iicrecvack2())
	return 0xff; 
	iicsendbyte2(subadd_h);
  if(iicrecvack2())
	return 0xff;  	
	iicstop2();
  return 0;
}
