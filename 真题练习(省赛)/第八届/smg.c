#include<stc15.h>
#include<absacc.h>
#include<intrins.h>
#include<smg.h>


void smg_show_bit(unsigned pos, unsigned char dat)//�������ʾһλ
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xa000] = 0x01 << pos;
	XBYTE[0xe000] = dat;
	Delay1ms();
}
void smg_show_all(unsigned char dat)//�����ȫ��ʾ
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xa000] = 0xff;
	XBYTE[0xe000] = dat;
}
void smg_show()//����ܶ�̬��ʾ
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		smg_show_bit(i, dspbuf[i]);
	}
	smg_show_all(0xff);
}
void Delay1ms()//@11.0592MHz,����ܶ�̬ɨ����ʱ
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