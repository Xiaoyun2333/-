#include<stc15.h>
#include<absacc.h>
#include<intrins.h>

#include<onewire.h>
//---------------------------------------------------------------------------
unsigned char tab[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};//0~9��"-"Ϊ0xbf
unsigned char dspbuf[] = {255,255,255,255,255,255,255,255};//��ʾ����

sbit s7 = P3^0;
sbit s6 = P3^1;
sbit s5 = P3^2;
sbit s4 = P3^3;//����
sbit pwm = P3^4;//pwm���

bit pwm_s = 0;//���״̬��0ֹͣ���
unsigned char pwm_value = 2;//˯�߷�20����Ȼ��Ϊ30������Ϊ70

unsigned int temp = 25;//�¶Ȳ���

unsigned char s = 0;//����ʱ��
unsigned char count_pwm = 0;//pwm����
unsigned char count_timer0 = 0;//��ʱ������
unsigned char count_10ms = 0;//10�������

unsigned char cs = 0x00;
unsigned char ns = 0x00;
bit ts = 0;//�¶���ʾ״̬��0Ϊ�رգ�1Ϊ��ʾ
/*
0x_0˯�߷�״̬  0x_1��Ȼ��״̬  0x_2����״̬ 
0x0_ʣ��ʱ��Ϊ0 0x1_ʣ��ʱ��Ϊ1 0x2_ʣ��ʱ��Ϊ2
*/
//-----------------------------��ʼ��-----------------------------
void cls();//�رշ�������led���豸
//-----------------------------��ʱ��-----------------------------
void Timer0Init();//��ʱ��0 16λ�Զ����� 100us�ж�
void timer0_service();//��ʱ��0 �жϷ�����
//-----------------------------�����-----------------------------
void smg_show_bit(unsigned pos, unsigned dat);//�������ʾһλ
void smg_show_all(unsigned dat);//�����ȫ����ʾ
void smg_show();//����ܶ�̬��ʾ
void buf_set_time();//ʱ����ʾ����
void buf_set_mode(unsigned mode);//ģʽ��ʾ����
void buf_set_temp();//�¶���ʾ����
void Delay1ms();//��̬ɨ��1ms��ʱ
//-----------------------------�´���-----------------------------
void rd_temperature(void);//�����¶�
void delay(unsigned int i);//������ʱ
//-----------------------------״̬��-----------------------------
void statemachine();//״̬��
//----------------------------------------------------------------
void main()
{
	cls();//�رշ�������led���豸
	pwm = 0;
	rd_temperature();
	Timer0Init();
	XBYTE[0x8000] = 0xfe;
	while(1)
	{
		statemachine();//״̬��
	}
}
//-------------------------------��ʼ��----------------------------
void cls()
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
}
//-------------------------------��ʱ��-----------------------------
void Timer0Init()//��ʱ��0 16λ�Զ�����
{
	AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0xAE;		//���ö�ʱ��ʼֵ
	TH0 = 0xFB;		//���ö�ʱ��ʼֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
	
	ET0 = 1;
	EA = 1;
}	
void timer0_service() interrupt 1 //��ʱ��0 �жϷ�����
{
	count_timer0++;
	if(s == 0){pwm_s = 0; count_timer0 = 0;}
	else
	{
		pwm_s = 1;
		count_pwm++;
		if(count_timer0 == 100){count_10ms++; count_timer0 = 0;}
		if(count_10ms == 100){s--; count_10ms = 0;}
		if(count_pwm < pwm_value){pwm = 1;}
		else if(count_pwm < 10){pwm = 0;}
		else if(count_pwm == 10){count_pwm = 0;}
	}
	
}
//-------------------------------�����----------------------------
void smg_show_bit(unsigned pos, unsigned dat)//�������ʾһλ
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0x01 << pos;
	XBYTE[0xe000] = dat;
	Delay1ms();
}
void smg_show_all(unsigned dat)//�����ȫ����ʾ
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = dat;
}
void smg_show()//����ܶ�̬��ʾ
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		smg_show_bit(i, dspbuf[i]);
		Delay1ms();
	}
	smg_show_all(0xff);
}
void buf_set_time()//ʱ����ʾ����
{
	dspbuf[4] = 0xff;
	dspbuf[5] = tab[s/ 100];
	dspbuf[6] = tab[(s / 10) % 10];
	dspbuf[7] = tab[s % 10];
}
void buf_set_temp()//�¶���ʾ����
{
	dspbuf[4] = 0xff;
	dspbuf[5] = tab[temp / 10];
	dspbuf[6] = tab[temp % 10];
	dspbuf[7] = 0xc6;
}
void buf_set_mode(unsigned mode)//ģʽ��ʾ����
{
	dspbuf[0] = 0xbf;
	dspbuf[1] = tab[mode];
	dspbuf[2] = 0xbf;
	dspbuf[3] = 0xff;
}
void Delay1ms()//@11.0592MHz
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
//-------------------------------�´���-------------------------------
void rd_temperature(void)//�����¶�
{
	unsigned char lsb,msb;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	delay(10);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	lsb = Read_DS18B20();
	msb = Read_DS18B20();
	
	temp = msb;
	temp = (temp << 8) | lsb;
	temp >>= 4;
}
void delay(unsigned int i)//������ʱ
{
	while(--i)
	{
		smg_show();
	}
}
//-------------------------------״̬��-------------------------------
void statemachine()//״̬��
{
	cs = ns;
	switch((cs & 0x0f))
	{
		case 0x00:
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();};} ns = cs + 1; pwm_value = 3; XBYTE[0x8000] = 0xfd;}
			if(s7 == 0){smg_show();if(s7 == 0){while(s7 == 0){smg_show();};} ts = ~ts;}
		break;
		case 0x01:
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();};} ns = cs + 1; pwm_value = 7; XBYTE[0x8000] = 0xfb;}
			if(s7 == 0){smg_show();if(s7 == 0){while(s7 == 0){smg_show();};} ts = ~ts;} 
		break;
		case 0x02:
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();};} ns = cs - 2; pwm_value = 2; XBYTE[0x8000] = 0xfe;}
			if(s7 == 0){smg_show();if(s7 == 0){while(s7 == 0){smg_show();};} ts = ~ts;}
		break;	
	}
	switch((cs & 0xf0))
	{
		case 0x00:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();};} ns = cs + 16; s = 60;}
			if(s6 == 0){smg_show();if(s6 == 0){while(s6 == 0){smg_show();};} ns = cs & 0x0f; s = 0; pwm = 0;}
		break;
		case 0x10:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();};} ns = cs + 16; s = 120;}
			if(s6 == 0){smg_show();if(s6 == 0){while(s6 == 0){smg_show();};} ns = cs & 0x0f; s = 0; pwm = 0;}
		break;
		case 0x20:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();};} ns = cs - 32; s = 0; pwm = 0;}
			if(s6 == 0){smg_show();if(s6 == 0){while(s6 == 0){smg_show();};} ns = cs & 0x0f; s = 0; pwm = 0;}
		break;	
	}
	if(ts == 1){buf_set_mode(4);buf_set_temp();rd_temperature();}
	else if((cs & 0x0f) == 0x00){buf_set_mode(1);buf_set_time();}
	else if((cs & 0x0f) == 0x01){buf_set_mode(2);buf_set_time();}
	else if((cs & 0x0f) == 0x02){buf_set_mode(3);buf_set_time();}
	smg_show();;
}
//