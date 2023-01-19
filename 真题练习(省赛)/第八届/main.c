#include<stc15.h>
#include<absacc.h>
#include<intrins.h>

#include<ds1302.h>
#include<onewire.h>
//========================================
unsigned char tab[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};//0~9��"-"Ϊ0xbf
unsigned char w_addr[] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};//ds1302д��ַ
unsigned char r_addr[] = {0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};//ds1302д��ַ
unsigned char time[] = {0x50,0x59,0x23,0x00,0x00,0x00,0x00};//��ʼʱ��23ʱ59��50��
unsigned char dspbuf[] = {255,255,255,255,255,255,255,255};//��ʾ����

sbit s7 = P3^0;
sbit s6 = P3^1;
sbit s5 = P3^2;
sbit s4 = P3^3;//�ĸ���������ӳ��

unsigned int temp = 0;//Ԥ���¶Ȳ���

unsigned char cs = 0x00;//��̬
unsigned char ns = 0x00;//��̬

unsigned int smg_flash = 0;//�������˸��ʾ
unsigned char led_flash = 0;//led��˸��ʾ

unsigned char led_flash_count = 0;//led��˸����

unsigned char set_time[] = {0, 0, 0};//ʱ�����ò���
unsigned char clk_time[] = {0, 0, 0};//�������ò���
unsigned char clk_time_buf[] = {0, 0, 0};//���Ӳ�����ʾ����

unsigned char clk_flag = 0;//������ʾ��־
unsigned char clk_set_flag = 0;//�������ñ�־

unsigned char t0_count = 0;
unsigned char t0_h = 0;
unsigned char t0_m = 0;
unsigned char t0_s = 0;//T0����
/*״̬��
0x00 ʱ��״̬
0x10 ʱ������_ʱ	0x11 ʱ������_�� 0x12 ʱ������_��
0x20 ��������_ʱ	0x21 ��������_�� 0x22 ��������_��
0x30 ������ʾ״̬
�¶���ʾ״̬Ϊ����s4����������
*/
//==============��ʼ��======================
void cls();//�رշ�������led���豸
//==============��ʱ��0======================
void Timer0Init();//T0��ʼ��50����@11.0592MHz
void timer0_service();//T0�жϷ�����
//==============�����======================
void smg_show_bit(unsigned pos, unsigned char dat);//�������ʾһλ
void smg_show_all(unsigned char dat);//�����ȫ��ʾ
void smg_show();//����ܶ�̬��ʾ
void buf_set_time();//ʱ����ʾ����
void buf_set_temp();//�¶���ʾ����
void buf_set_clk();//������ʾ����
void Delay1ms();//@11.0592MHz,����ܶ�̬ɨ����ʱ
void smg_show_flash(unsigned char pos);//�������˸
//==============������======================
void ds1302_init();//ds1302��ʼ��
void read_time();//��ȡʱ��
void time_set(unsigned char pos);//ʱ������
void clock_set(unsigned char pos);//��������
//==============�´���======================
void rd_temperature();//�¶�ת��
void Delay(unsigned int t);//�¶�ת���ӳ�
//==============led��ʾ=====================
void clk_check();//�������
void led_flashing();//led��˸
void led_stop();//ledֹͣ��˸
//==============״̬��======================
void statemachine();//״̬��
//==============������======================
void main()
{
	cls();//�رշ�������led���豸
	rd_temperature();//�¶�ת��
	ds1302_init();//ds1302��ʼ��
	buf_set_time();//ʱ����ʾ����
	while(1)
	{
		statemachine();//״̬��
	}
}
//==============��ʼ��======================
void cls()//�رշ�������led���豸
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
}
//==============��ʱ��0======================
void Timer0Init()//T0��ʼ��50����@11.0592MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0x00;		//���ö�ʱ��ʼֵ
	TH0 = 0x4C;		//���ö�ʱ��ʼֵ
	TF0 = 0;		//���TF0��־
	ET0 = 1;
	EA = 1;
	t0_h = clk_time[0];
	t0_m = clk_time[1];
	t0_s = clk_time[2];
	if(((t0_h == 0) & (t0_m == 0) & (t0_s == 0)) == 1)
	{
		clk_set_flag = 0;
		TR0 = 0;
	}
	else
	{
		clk_set_flag = 1;
		TR0 = 1;
	}
}
void timer0_service() interrupt 1//T0�жϷ�����
{
	t0_count++;
	if(t0_count == 20)
	{
		t0_s--;
		t0_count = 0;
		if(t0_s == 255)
		{
			t0_m--;
			t0_s = 59;
			if(t0_m == 255)
			{
				t0_h--;
				t0_m = 59;
				if(t0_h == 255)
				{
					t0_h = 0;
					TR0 = 0;
				}
			}
		}
	}
}
//==============�����======================
void smg_show_bit(unsigned pos, unsigned char dat)//�������ʾһλ
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0x01 << pos;
	XBYTE[0xe000] = dat;
	Delay1ms();
}
void smg_show_all(unsigned char dat)//�����ȫ��ʾ
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
	dspbuf[0] = tab[time[2] / 16];
	dspbuf[1] = tab[time[2] % 16];
	dspbuf[2] = 0xbf;
	dspbuf[3] = tab[time[1] / 16];
	dspbuf[4] = tab[time[1] % 16];
	dspbuf[5] = 0xbf;
	dspbuf[6] = tab[time[0] / 16];
	dspbuf[7] = tab[time[0] % 16];
}
void buf_set_temp()//�¶���ʾ����
{
	dspbuf[0] = 255;
	dspbuf[1] = 255;
	dspbuf[2] = 255;
	dspbuf[3] = 255;
	dspbuf[4] = 255;
	dspbuf[5] = tab[temp / 10];
	dspbuf[6] = tab[temp % 10];
	dspbuf[7] = 0xc6;
}
void buf_set_clk()//������ʾ����
{
	dspbuf[0] = tab[clk_time_buf[2] / 16];
	dspbuf[1] = tab[clk_time_buf[2] % 16];
	dspbuf[2] = 0xbf;
	dspbuf[3] = tab[clk_time_buf[1] / 16];
	dspbuf[4] = tab[clk_time_buf[1] % 16];
	dspbuf[5] = 0xbf;
	dspbuf[6] = tab[clk_time_buf[0] / 16];
	dspbuf[7] = tab[clk_time_buf[0] % 16];
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
void smg_show_flash(unsigned char pos)
{
	switch(pos)
	{
		case 0:
			if(smg_flash < 100)
			{
				smg_show_bit(0, 0xff);
				smg_show_bit(1, 0xff);
				smg_flash++;
			}
			else if(smg_flash < 200)
			{
				smg_show_bit(0, dspbuf[0]);
				smg_show_bit(1, dspbuf[1]);
				smg_flash++;
				if(smg_flash == 200){smg_flash = 0;}
			}
			smg_show_bit(2, dspbuf[2]);
			smg_show_bit(3, dspbuf[3]);
			smg_show_bit(4, dspbuf[4]);
			smg_show_bit(5, dspbuf[5]);
			smg_show_bit(6, dspbuf[6]);
			smg_show_bit(7, dspbuf[7]);
		break;
		case 1:
			smg_show_bit(0, dspbuf[0]);
			smg_show_bit(1, dspbuf[1]);
			smg_show_bit(2, dspbuf[2]);
			if(smg_flash < 100)
			{
				smg_show_bit(3, 0xff);
				smg_show_bit(4, 0xff);
				smg_flash++;
			}
			else if(smg_flash < 200)
			{
				smg_show_bit(3, dspbuf[3]);
				smg_show_bit(4, dspbuf[4]);
				smg_flash++;
				if(smg_flash == 200){smg_flash = 0;}
			}
			smg_show_bit(5, dspbuf[5]);
			smg_show_bit(6, dspbuf[6]);
			smg_show_bit(7, dspbuf[7]);
		break;
		case 2:
			smg_show_bit(0, dspbuf[0]);
			smg_show_bit(1, dspbuf[1]);
			smg_show_bit(2, dspbuf[2]);
			smg_show_bit(3, dspbuf[3]);
			smg_show_bit(4, dspbuf[4]);
			smg_show_bit(5, dspbuf[5]);
			if(smg_flash < 100)
			{
				smg_show_bit(6, 0xff);
				smg_show_bit(7, 0xff);
				smg_flash++;
			}
			else if(smg_flash < 200)
			{
				smg_show_bit(6, dspbuf[6]);
				smg_show_bit(7, dspbuf[7]);
				smg_flash++;
				if(smg_flash == 200){smg_flash = 0;}
			}			
		break;
	}
}
//==============������======================
void ds1302_init()//ʱ�ӳ�ʼ��
{
	char i;
	Write_Ds1302_Byte(0x8e,0x00);
	for(i = 0; i < 7; i++)
	{
		Write_Ds1302_Byte(w_addr[i],time[i]);     
	}
	Write_Ds1302_Byte(0x8e,0x80);
}
void read_time()//ʱ�Ӷ�ȡ
{
	char i;
	for(i = 0; i < 7; i++)
	{
		time[i] =  Read_Ds1302_Byte(r_addr[i]);
	}
}
void time_set(unsigned char pos)//ʱ������
{
	set_time[0] = time[2] /16 * 10 + time[2] % 16;
	set_time[1] = time[1] /16 * 10 + time[1] % 16;
	set_time[2] = time[0] /16 * 10 + time[0] % 16;
	switch(pos)
	{
		case 0:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();}if(set_time[0] < 23){set_time[0]++;}else{set_time[0] = 0;}}}
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();}if(set_time[0] > 0){set_time[0]--;}else{set_time[0] = 23;}}}
		break;
		case 1:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();}if(set_time[1] < 59){set_time[1]++;}else{set_time[1] = 0;}}}
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();}if(set_time[1] > 0){set_time[1]--;}else{set_time[1] = 59;}}}
		break;
		case 2:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();}if(set_time[2] < 59){set_time[2]++;}else{set_time[2] = 0;}}}
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();}if(set_time[2] > 0){set_time[2]--;}else{set_time[2] = 59;}}}
		break;
	}
	time[2] = set_time[0] / 10 *16 + set_time[0] % 10;
	time[1] = set_time[1] / 10 *16 + set_time[1] % 10;
	time[0] = set_time[2] / 10 *16 + set_time[2] % 10;
}
void clock_set(unsigned char pos)//��������
{
	switch(pos)
	{
		case 0:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();}if(clk_time[0] < 23){clk_time[0]++;}else{clk_time[0] = 0;}}}
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();}if(clk_time[0] > 0){clk_time[0]--;}else{clk_time[0] = 23;}}}
		break;
		case 1:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();}if(clk_time[1] < 59){clk_time[1]++;}else{clk_time[1] = 0;}}}
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();}if(clk_time[1] > 0){clk_time[1]--;}else{clk_time[1] = 59;}}}
		break;
		case 2:
			if(s5 == 0){smg_show();if(s5 == 0){while(s5 == 0){smg_show();}if(clk_time[2] < 59){clk_time[2]++;}else{clk_time[2] = 0;}}}
			if(s4 == 0){smg_show();if(s4 == 0){while(s4 == 0){smg_show();}if(clk_time[2] > 0){clk_time[2]--;}else{clk_time[2] = 59;}}}
		break;
	}
	clk_time_buf[2] = clk_time[0] / 10 *16 + clk_time[0] % 10;
	clk_time_buf[1] = clk_time[1] / 10 *16 + clk_time[1] % 10;
	clk_time_buf[0] = clk_time[2] / 10 *16 + clk_time[2] % 10;
}
//==============�´���======================
void rd_temperature()//�¶�ת��
{
	unsigned char LSB,MSB;
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	Delay(100);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB = Read_DS18B20();
	MSB = Read_DS18B20();
	
	temp = MSB;
	temp = (temp << 8) | LSB;
	temp >>= 4;
}
void Delay(unsigned int t)//�¶�ת���ӳ�
{
	while(--t)
	{
		smg_show();
	}
}
//==============led��ʾ=====================
void clk_check()//�������
{
	if(((time[0] == 0) & (time[1] == 0) & (time[2] == 0)) == 1){clk_flag = 1;}
	if(((clk_set_flag == 1) & (t0_h == 0) & (t0_m == 0) & (t0_s == 0)) == 1){clk_flag = 1;}
}
void led_flashing()//led��˸
{
	if(clk_flag == 1)
	{
		if(led_flash < 7)
		{
			XBYTE[0x8000] = 0xfe;
			led_flash++;
		}
		else if(led_flash < 14)
		{
			XBYTE[0x8000] = 0xff;
			led_flash++;
			if(led_flash == 14)
			{
				led_flash = 0;
				led_flash_count++;
			}
		}
		if(led_flash_count == 25)
		{
			clk_flag = 0;
			led_flash_count = 0;
			led_flash = 0;
			clk_set_flag = 0;
			ns = 0x00;
		}
	}
}
void led_stop()//ledֹͣ��˸
{
	if((s4 & s5 & s6 & s7) == 0)
	{
		if((s4 & s5 & s6 & s7) == 0)
		{
			while((s4 & s5 & s6 & s7) == 0)
			{
				smg_show();
			}
			clk_flag = 0; 
			clk_set_flag = 0;
			led_flash = 0; 
			XBYTE[0x8000] = 0xff;
			ns = 0x00;
		}
	}
}
//========================================
void statemachine()//״̬��
{
	cs = ns;
	switch(cs)
	{
		case 0x00:
			if(s7 == 0){smg_show(); if(s7 == 0){while(s7 == 0){smg_show();}buf_set_time(); ns = 0x10;}}
			if(s6 == 0){smg_show(); if(s6 == 0){while(s6 == 0){smg_show();}buf_set_time(); ns = 0x20;}}
			if(s4 == 0){smg_show(); if(s4 == 0){while(s4 == 0){buf_set_temp();smg_show();rd_temperature();}buf_set_time(); ns = 0x00;}}
			if(clk_flag == 1){ns = 0x30;}
			else{buf_set_time();}
		break;
		case 0x10:
			if(s7 == 0){smg_show(); if(s7 == 0){while(s7 == 0){smg_show();} ns = 0x11;}}
			if(clk_flag == 1){ns = 0x30;}
			else{buf_set_time();}
		break;
		case 0x11:
			if(s7 == 0){smg_show(); if(s7 == 0){while(s7 == 0){smg_show();} ns = 0x12;}}
			if(clk_flag == 1){ns = 0x30;}
			else{buf_set_time();}
		break;
		case 0x12:
			if(s7 == 0){smg_show(); if(s7 == 0){while(s7 == 0){smg_show();} ns = 0x00;}}
			if(clk_flag == 1){ns = 0x30;}
			else{buf_set_time();ds1302_init();}
		break;
		case 0x20:
			if(s6 == 0){smg_show(); if(s6 == 0){while(s6 == 0){smg_show();} ns = 0x21;}}
			if(clk_flag == 1){ns = 0x30;}
			else{TR0 = 0;clk_set_flag = 1;buf_set_time();}
		break;
		case 0x21:
			if(s6 == 0){smg_show(); if(s6 == 0){while(s6 == 0){smg_show();} ns = 0x22;}}
			if(clk_flag == 1){ns = 0x30;}
			else{buf_set_time();}
		break;
		case 0x22:
			if(s6 == 0){smg_show(); if(s6 == 0){while(s6 == 0){smg_show();} ns = 0x00;}}
			if(clk_flag == 1){ns = 0x30;}
			else{buf_set_time();Timer0Init();}
		break;
		case 0x30:
			if((s4 & s5 & s6 & s7) == 0){smg_show(); if((s4 & s5 & s6 & s7) == 0){while((s4 & s5 & s6 & s7) == 0){smg_show();} ns = 0x00;}}
			if(clk_flag == 1){ns = 0x30;}
			else{buf_set_time();}
		break;
	}
	if(cs == 0x00){clk_check();buf_set_time();smg_show();read_time();}
	else if(cs == 0x10){smg_show_flash(0);time_set(0);}
	else if(cs == 0x11){smg_show_flash(1);time_set(1);}
	else if(cs == 0x12){smg_show_flash(2);time_set(2);}
	else if(cs == 0x20){buf_set_clk();smg_show_flash(0);clock_set(0);read_time();}
	else if(cs == 0x21){buf_set_clk();smg_show_flash(1);clock_set(1);read_time();}
	else if(cs == 0x22){buf_set_clk();smg_show_flash(2);clock_set(2);read_time();}
	else if(cs == 0x30){buf_set_time();smg_show();read_time();led_flashing();led_stop();read_time();}
}