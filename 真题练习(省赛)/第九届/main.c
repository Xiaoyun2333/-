#include<stc15.h>
#include<intrins.h>
#include<absacc.h>

#include<iic.h>
//****************************************************
unsigned char tab[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};//0~9��"-"Ϊ0xbf
unsigned char dspbuf[] = {255,255,255,255,255,255,255,255};//��ʾ����

sbit s7 = P3^0;
sbit s6 = P3^1;
sbit s5 = P3^2;
sbit s4 = P3^3;//�ĸ���������ӳ��

unsigned char ad = 0;//adת����ֵ
unsigned char light = 1;//����

bit led_value = 0;//led������־
unsigned char led_t = 0;//led��������
unsigned char led_mode = 1;//led����ģʽģʽ
bit smg_value = 0;//�����������־
unsigned char time_mode = 4;//����ܹ���ʱ����ʾģʽ

unsigned char cs = 0x00;//��̬
unsigned char ns = 0x00;//��̬

unsigned int timer0_count = 0;//��ʱ��0 10ms����
unsigned int flash_count = 0;//��˸����
unsigned char pwm_count = 0;

unsigned char pwm_value = 25;//pwm�ȼ� 50 75 100 
unsigned char pwm = 0xff;
//****************************************************
void cls();//�ر��޹��豸
void statemachine();//״̬��
void smg_show_bit(unsigned pos, unsigned char dat);//�������ʾһλ
void smg_show_all(unsigned char dat);//�����ȫ��ʾ
void smg_show();//����ܶ�̬��ʾ
void buf_work_mode();//����ܹ���ģʽ��ʾ����
void buf_time_mode();//�����ʱ��ģʽ��ʾ����
void buf_light_mode();//�����������ʾ����
void Delay1ms();//@11.0592MHz,����ܶ�̬ɨ����ʱ
void key_s7();//s7led���ؼ��
void led(char mode);//led��ʾ
void timer0_init();//@11.0592MHz 10ms��ʱ
void timer0_service();//��ʱ��0�жϷ���
void led_light();//led���ȿ���
void ad_write(unsigned addr, unsigned dat);//iicд��
unsigned char ad_read(unsigned char addr);//iic��ȡ
//****************************************************
void main()
{
	cls();
	timer0_init();
	while(1)
	{
		statemachine();
	}
}
//****************************************************
void cls()
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
}

void statemachine()
{
	cs = ns;

	switch (cs)
	{
	case 0x00:
		if(s6 == 0){smg_show();if(s6 == 0){while(s6 == 0){smg_show();}ns = 0x01; smg_value = 1;}}
		if(s4 == 0){buf_light_mode();smg_value = 1;while(s4 == 0){smg_show();led_light();buf_light_mode();led(led_mode);}smg_value = 0;}
		break;
	case 0x01:
		if(s6 == 0){smg_show();if(s6 == 0){while(s6 == 0){smg_show();}ns = 0x02; flash_count = 0;}}
		if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){buf_work_mode();smg_show();led(led_mode);}if(++led_mode == 5){led_mode = 1;}led_t = 0;}}
		if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){buf_work_mode();smg_show();led(led_mode);}if(--led_mode == 0){led_mode = 4;}led_t = 0;}}
		break;
	case 0x02:
		if(s6 == 0){smg_show();if(s6 == 0){while(s6 == 0){smg_show();}ns = 0x00; smg_value = 0;}}	
		if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){buf_time_mode();smg_show();led(led_mode);}if(++time_mode >= 13){time_mode = 4;}}}
		if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){buf_time_mode();smg_show();led(led_mode);}if(--time_mode <= 3){time_mode = 12;}}}
		break;
	}
	led_light();
	buf_work_mode();
	buf_time_mode();
	smg_show();
	led(led_mode);
	key_s7();
}

void smg_show_bit(unsigned pos, unsigned char dat)
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0x01 << pos;
	XBYTE[0xe000] = dat;
	Delay1ms();
}

void smg_show_all(unsigned char dat)
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = dat;
}

void smg_show()
{
	unsigned char i;
	if(smg_value == 1)
	{
		for(i = 0; i < 8; i++)
		{
			smg_show_bit(i, dspbuf[i]);
			led(led_mode);
			Delay1ms();		
		}
		smg_show_all(0xff);
	}
	else{return;}
}

void buf_work_mode()
{
	if(ns == 0x01)
	{
		if(flash_count < 400)
		{
			dspbuf[0] = 0xff;
			dspbuf[1] = 0xff;
			dspbuf[2] = 0xff;
		}
		else
		{
			dspbuf[0] = 0xbf;
			dspbuf[1] = tab[led_mode];
			dspbuf[2] = 0xbf;
		}
	}
	else
	{
		dspbuf[0] = 0xbf;
		dspbuf[1] = tab[led_mode];
		dspbuf[2] = 0xbf;
	}
}
void buf_time_mode()
{
	if(ns == 0x02)
	{
		if(flash_count < 400)
		{
			dspbuf[4] = 0xff;
			dspbuf[5] = 0xff;
			dspbuf[6] = 0xff;
			dspbuf[7] = 0xff;
		}
		else
		{
			if((time_mode / 10) == 0){dspbuf[4] = 255;}
			else{dspbuf[4] = tab[time_mode / 10];}
			dspbuf[5] = tab[time_mode % 10];
			dspbuf[6] = tab[0];
			dspbuf[7] = tab[0];
		}
	}
	else
	{
		if((time_mode / 10) == 0){dspbuf[4] = 255;}
		else{dspbuf[4] = tab[time_mode / 10];}
		dspbuf[5] = tab[time_mode % 10];
		dspbuf[6] = tab[0];
		dspbuf[7] = tab[0];
	}
}
void buf_light_mode()
{
	dspbuf[0] = 0xff;
	dspbuf[1] = 0xff;
	dspbuf[2] = 0xff;
	dspbuf[3] = 0xff;
	dspbuf[4] = 0xff;
	dspbuf[5] = 0xff;
    dspbuf[6] = 0xbf;
    dspbuf[7] = tab[light];
}
void Delay1ms()
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

void key_s7()
{
	if(s7 == 0)
	{
		smg_show();
		if(s7 == 0)
		{
			while(s7 == 0)
			{
				smg_show();
			}
			led_value = ~led_value;
		}
	}
}

void led(char mode)
{
	if(led_value == 0){XBYTE[0x8000] = 0xff; led_t = 0;}
	else
	{
		switch (mode)
		{
		case 1:
			XBYTE[0x8000] = ~((0x01 & pwm) << led_t);
			break;
		case 2:
			XBYTE[0x8000] = ~((0x80 & pwm) >> led_t);
			break;
		case 3:
			if(led_t < 4){XBYTE[0x8000] = ~(((0x01 & pwm)<< led_t) | ((0x80 & pwm)>> led_t));}
			else{led_t -= 4;}
			break;
		case 4:
			if(led_t < 4){XBYTE[0x8000] = ~(((0x08 & pwm)>> led_t) | ((0x10 & pwm)<< led_t));}
			else{led_t -= 4;}
			break;
		}
	}
}

void timer0_init()
{
	AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0xCD;		//���ö�ʱ��ʼֵ
	TH0 = 0xD4;		//���ö�ʱ��ʼֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ

	ET0 = 1;
	EA = 1;
}
void timer0_service() interrupt 1
{
	if(led_value == 1)
	{
		timer0_count++;
		if(timer0_count == time_mode * 100)
		{
			led_t++;
			timer0_count = 0;
			if(led_t == 8){led_t = 0;}
		}
	}
	else{timer0_count = 0;}
	if(smg_value == 1)
	{
		flash_count++;
		if(flash_count == 800)
		{
			flash_count = 0;
		}
	}	
	else{flash_count = 0;}
	pwm_count++;
	if(pwm_count < pwm_value){pwm = 0xff;}
	else if(pwm_count < 10){pwm = 0x00;}
	else if(pwm_count == 10){pwm = 0xff;pwm_count = 0;}
}

unsigned char ad_read(unsigned char addr) 
{
	unsigned char temp;
    IIC_Start();
    IIC_SendByte(0x90);
    IIC_WaitAck();
    IIC_SendByte(addr);
    IIC_WaitAck();

    IIC_Start();
    IIC_SendByte(0x91);
    IIC_WaitAck();
    temp = IIC_RecByte();
    IIC_SendAck(1);
    IIC_Stop();
    return temp;
}
void ad_write(unsigned addr, unsigned dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}
void led_light()
{
	ad = ad_read(0x43);
	if((0 <= ad ) && (ad < 64)){light = 1;pwm_value = 1;}
	else if((64 <= ad ) && (ad < 128)){light = 2;pwm_value = 4;}
	else if((128 <= ad ) && (ad < 192)){light = 3;pwm_value = 7;}
	else if((192 <= ad ) && (ad < 256)){light = 4;pwm_value = 10;}
}
//****************************************************