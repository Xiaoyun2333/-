#include<stc15.h>
#include<ds1302.h>
#include<absacc.h>
#include<intrins.h>
//================================================
unsigned char wirte_da1302_add[] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};//ds1302����ַ
unsigned char read_da1302_add[] = {0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};//ds1302д��ַ
unsigned char da1302_time[7] = {0x58,0x59,0x23,0x31,0x12,0x06,0x22};//��ʼʱ��22��12��31�գ�������23ʱ59��58��
unsigned char tab[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};//�����0~F��"-"Ϊ0xbf
//================================================
void cls();//�ر�led�ͷ�����
//================================================
void smg_show_bit(unsigned char pos, unsigned char dat);//�����һλ��ʾ
void smg_show();//����ܶ�̬��ʾ
void smg_show_all(unsigned char dat);//�����ͳһ��ʾ
//================================================
void ds1302_config();//ds1302��ʼ��
void read_da1302_time();//��ȡʱ��
//================================================
void Delay1ms();
//================================================
void main()
{
	cls();
	ds1302_config();
	while(1)
	{
		smg_show();
		read_da1302_time();
	}
}
//================================================
void cls()
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
}
void ds1302_config()
{
	char i;
	Write_Ds1302_Byte(0x8e,0x00);
	for(i = 0; i < 7; i++)
	{
		Write_Ds1302_Byte(wirte_da1302_add[i],da1302_time[i]);     
	}
	Write_Ds1302_Byte(0x8e,0x80);
}
void read_da1302_time()
{
	char i;
	for(i = 0; i < 7; i++)
	{
		da1302_time[i] =  Read_Ds1302_Byte(read_da1302_add[i]);
	}
}
//================================================
void smg_show_bit(unsigned char pos, unsigned char dat)
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0x01 << pos;
	XBYTE[0xe000] = dat;
}
void smg_show()
{
	smg_show_bit(0, tab[da1302_time[2] / 16]);
	Delay1ms();
	smg_show_bit(1, tab[da1302_time[2] % 16]);
	Delay1ms();
	smg_show_bit(2, 0xbf);
	Delay1ms();
	smg_show_bit(3, tab[da1302_time[1] / 16]);
	Delay1ms();
	smg_show_bit(4, tab[da1302_time[1] % 16]);
	Delay1ms();
	smg_show_bit(5, 0xbf);
	Delay1ms();
	smg_show_bit(6, tab[da1302_time[0] / 16]);
	Delay1ms();
	smg_show_bit(7, tab[da1302_time[0] % 16]);
	Delay1ms();
	
	smg_show_all(0xff);
}
void smg_show_all(unsigned char dat)
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = dat;
}
//================================================
void Delay1ms()		//@11.0592MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
}
