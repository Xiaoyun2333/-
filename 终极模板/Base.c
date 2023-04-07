#include "stc15f2k60s2.h"
#include "Base.h"
#include "intrins.h"

code unsigned char Seg_Table[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0xc1};
static unsigned char temp1 = 0x00;
static unsigned char temp1_old = 0xff;
sbit Tx = P1^0;
sbit Rx = P1^1;

//��ֵ��ȡ
//unsigned char Key_Read()
//{
//	unsigned char temp = 0x00;
//	if(P33 == 0)temp = 4;
//	if(P32 == 0)temp = 5;
//	if(P31 == 0)temp = 6;
//	if(P30 == 0)temp = 7;
//	return temp;
//}
//����ܰ�λ��ʾ
void Smg_Bit(unsigned char pos,dat,dot)
{
	P0 = 0xff;
	P2 = P2 & 0x1f | 0xe0;
	P2 &= 0x1f;
	
	P0 = 0x01 << pos;
	P2 = P2 & 0x1f | 0xc0;
	P2 &= 0x1f;
	
	P0 = Seg_Table[dat];
	if(dot)P0 &= 0x7f;
	P2 = P2 & 0x1f | 0xe0;
	P2 &= 0x1f;
}
//led��λ��ʾ
void Led_Bit(unsigned char pos,flag)
{
	static unsigned char temp = 0x00;
	static unsigned char temp_old = 0xff;
	
	if(flag)temp |= 0x01 << pos;
	else temp &= ~(0x01 << pos);
	if(temp != temp_old)
	{
		P0 = ~temp;
		P2 = P2 & 0x1f | 0x80;
		P2 &= 0x1f;
		temp_old = temp;
	}
}
//�̵���
void Relay(unsigned char flag)
{
	if(flag)temp1 |= 0x10;
	else temp1 &= ~(0x10);
	if(temp1 != temp1_old)
	{
		P0 = temp1;
		P2 = P2 & 0x1f | 0xa0;
		P2 &= 0x1f;
		temp1_old = temp1;
	}
}
//������
void Beep(unsigned char flag)
{
	if(flag)temp1 |= 0x40;
	else temp1 &= ~(0x40);
	if(temp1 != temp1_old)
	{
		P0 = temp1;
		P2 = P2 & 0x1f | 0xa0;
		P2 &= 0x1f;
		temp1_old = temp1;
	}
}
//����������40khz��ʱ
void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}
//����8���ź�
void Wave_Init()
{
	unsigned char i;
	for(i = 0;i < 8;i++)
	{
		Tx = 1;
		Delay12us();
		Tx = 0;
		Delay12us();
	}
}
//��������ȡ���루PCAʵ�֣�
unsigned char Dis_Read()
{
	unsigned int time = 0;
	CMOD &= 0x00;
	CH = CL = 0;
	CF = 0;
	CR = 0;
	Wave_Init();
	CR = 1;
	while(Rx == 1&&CF == 0);
	CR = 0;
	if(CF == 0)
	{
		time = CH << 8 | CL;
		Rx = 1;
		return time * 0.017;
	}
	else
	{
		CF = 0;
		return 255;
	}
}

//�ö�ʱ��ʵ�ֳ�����
//unsigned char Dis_Read()
//{
//	unsigned int time = 0;
//	AUXR &= 0xBF;			//��ʱ��ʱ��12Tģʽ
//	TMOD &= 0x0F;			//���ö�ʱ��ģʽ
//	TL1 = TH1 = 0x00;				
//	TF1 = 0;			
//	TR1 = 0;
//	
//	Wave_Init();
//	TR1 = 1;
//	while(Rx == 1&&TF1 == 0);
//	TR1 = 0;
//	if(TF1 == 0)
//	{
//		time = TH1 << 8 | TL1;
//		Rx = 1;
//		return time * 0.017;
//	}
//	else 
//	{
//		TF1 = 0;
//		return 255;
//	}
//}
//������̶�ȡ��ֵ
unsigned char Key_Read()
{
	unsigned char temp = 0x00;
	AUXR &= 0xef;
	P44 = 0,P42 = 1,P35 = 1,P34 = 1;
	if(P33 == 0)temp = 4;
	if(P32 == 0)temp = 5;
	if(P31 == 0)temp = 6;
	if(P30 == 0)temp = 7;
	P44 = 1,P42 = 0,P35 = 1,P34 = 1;
	if(P33 == 0)temp = 8;
	if(P32 == 0)temp = 9;
	if(P31 == 0)temp = 10;
	if(P30 == 0)temp = 11;
	P44 = 1,P42 = 1,P35 = 0,P34 = 1;
	if(P33 == 0)temp = 12;
	if(P32 == 0)temp = 13;
	if(P31 == 0)temp = 14;
	if(P30 == 0)temp = 15;
//	P44 = 1,P42 = 1,P35 = 1,P34 = 0;
//	if(P33 == 0)temp = 16;
//	if(P32 == 0)temp = 17;
//	if(P31 == 0)temp = 18;
//	if(P30 == 0)temp = 19;
	P33 = 0xff;
	AUXR |= 0x10;
	return temp;
}