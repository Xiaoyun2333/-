#include<stc15.h>
#include<intrins.h>
//================================================
sbit s5 = P3^2;
sbit s4 = P3^3;
//================================================
code unsigned char tab[] = {0xc0,0xf9,0xa4,0xb0,
														0x99,0x92,0x82,0xf8,
														0x80,0x90,0x88,0x83,
														0xc6,0xa1,0x86,0x8e};//�����0~F��"-"Ϊ0xbf
unsigned char dspbuf[] = {255,255,255,255,255,255,255,255};//�������ʾ����

unsigned char hour = 0;
unsigned char min = 0;
unsigned char sec = 0;
unsigned char count = 0;
unsigned char led = 0xff;
unsigned char r_sbuf;
//================================================
void init_ls138(unsigned char value);//ѡ��P0ͨ��
void cls();//�ر�led�ͷ�����
void led_check();//LED�����Լ�
void smg_check();//����ܿ����Լ�
void Timer0Init();//50����@11.0592MHz
void service_t0();//t0�жϷ�����
void smg_show();//�������ʾ
void key_alone();//��������
void UartInit();//9600bps@11.0592MHz
void service_uart();//�����жϷ���
void sendchar(unsigned char Char);//�����ֽ�
void command();//ִ����λ������
//================================================
void Delay100ms();//�Լ캯����ʱ
void Delay1ms();//����ܶ�̬ɨ����ʱ
//================================================
void main()
{
	cls();
	led_check();
	smg_check();
	Timer0Init();
	UartInit();
	EA = 1;
	while(1)
	{
		smg_show();
		key_alone();
	}
}
//================================================
void init_ls138(unsigned char value)
{
	switch(value)
	{
		case 4:
			P2 = P2 & 0x1f | 0x80;
		break;
		case 5:
			P2 = P2 & 0x1f | 0xa0;
		break;
		case 6:
			P2 = P2 & 0x1f | 0xc0;
		break;
		case 7:
			P2 = P2 & 0x1f | 0xe0;
		break;
		default:
			P2 = P2 & 0x1f;
		break;
	}
}
void cls()
{
	init_ls138(5);
	P0 = 0x00;
	init_ls138(4);
	P0 = 0xff;
	init_ls138(0);
}
void led_check()
{
	unsigned char i;
	init_ls138(4);
	for(i = 0; i < 8; i++)
	{
		P0 = 0xfe << i;
		Delay100ms();
	}
	for(i = 0; i < 8; i++)
	{
		P0 = ~(~0x01 << i);
		Delay100ms();
	}
	init_ls138(0);
}
void smg_check()
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		init_ls138(6);
		P0 = ~(~0x01 << i);
		init_ls138(7);
		P0 = 0x00;
		Delay100ms();
	}
	for(i = 0; i < 8; i++)
	{
		init_ls138(6);
		P0 = 0xfe << i;
		init_ls138(7);
		P0 = 0x00;
		Delay100ms();
	}
	init_ls138(0);
}
void Timer0Init(void)		//50����@11.0592MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0x00;		//���ö�ʱ��ʼֵ
	TH0 = 0x4C;		//���ö�ʱ��ʼֵ
	TF0 = 0;		//���TF0��־
	ET0 =1;
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
}
void UartInit(void)		//9600bps@11.0592MHz
{
	PCON &= 0x7F;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��ʱ��1Tģʽ
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//���ö�ʱ��ģʽ
	TMOD |= 0x20;		//���ö�ʱ��ģʽ
	TL1 = 0xDC;		//���ö�ʱ��ʼֵ
	TH1 = 0xDC;		//���ö�ʱ����ֵ
	ET1 = 0;		//��ֹ��ʱ��%d�ж�
	ES =1;
	TR1 = 1;		//��ʱ��1��ʼ��ʱ
}
void service_t0() interrupt 1
{
	count++;
	if(count == 20)
	{
		sec++;
		count = 0;
		if(sec == 60)
		{
			min++;
			sec = 0;
			if(min == 60)
			{
				hour++;
				min = 0;
				if(hour == 100)
				{
					hour = 0;
				}
			}
		}
	}
}
void smg_show()
{
	unsigned char i;
	dspbuf[0] = tab[hour / 10];
	dspbuf[1] = tab[hour % 10];
	dspbuf[2] = 0xbf;
	dspbuf[3] = tab[min / 10];
	dspbuf[4] = tab[min % 10];
	dspbuf[5] = 0xbf;
	dspbuf[6] = tab[sec / 10];
	dspbuf[7] = tab[sec % 10];
	
	for(i = 0; i < 8; i++)
	{
		init_ls138(6);
		P0 = 0x01 << i;
		init_ls138(7);
		P0 = dspbuf[i];
		Delay1ms();
	}
	init_ls138(0);
}
void key_alone()
{
	if(s5 == 0)
	{
		smg_show();
		if(s5 == 0)
		{
			while(s5 == 0)
			{
				smg_show();
			}
			init_ls138(4);
			led = (led | 0x80) & (~led | 0x7f);
			P0 = led;
			init_ls138(0);
		}
	}
	if(s4 == 0)
	{
		smg_show();
		if(s4 == 0)
		{
			while(s4 == 0)
			{
				smg_show();
			}
			init_ls138(4);
			led = (led | 0x40) & (~led | 0xbf);
			P0 = led;
			init_ls138(0);
		}
	}	
}
void service_uart() interrupt 4
{
	if(RI == 1)
	{
		RI = 0;
		r_sbuf = SBUF;
		command();
	}
}
void sendchar(unsigned char Char)
{
	SBUF = Char;
	while(TI == 0);
	TI = 0;
}
void command()
{
	if((r_sbuf & 0xf0) == 0xa0)
	{
		init_ls138(4);
		led = (led & 0xf0) | ~(r_sbuf | 0xf0);
		P0 = led;
		init_ls138(0);
	}
	else if(r_sbuf == 0xb0)
	{		
		sendchar(((hour / 10) << 4) + (hour % 10));
		sendchar(((min / 10) << 4) + (min % 10));
		sendchar(((sec / 10) << 4) + (sec % 10));
	}
}
//================================================
void Delay100ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 5;
	j = 52;
	k = 195;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
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


	