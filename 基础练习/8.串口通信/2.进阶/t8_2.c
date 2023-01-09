#include<stc15.h>
#include<intrins.h>

unsigned char r_sbuf;//���ջ���

void init_ls138(unsigned char value);//ѡ��138���
void cls();//�ر�LED���豸
void UartInit();//���ڳ�ʼ��
void service_uart();//�����жϷ�����
void sendchar(unsigned char Char);//�����ַ�
void sendstring(unsigned char *p);//�����ַ���
void check_r_sbuf();//���r_sbuf

//=======================================================//
void main()
{
	cls();
	UartInit();
	sendstring("Welcome to XiaoyunOS!");
	while(1)
	{
		if(RI == 1){check_r_sbuf();}
	}
}
//=======================================================//
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
	}
}
void cls()
{
	init_ls138(5);
	P0 = 0x00;
	init_ls138(4);
	P0 = 0xff;
}
void UartInit(void)		//9600bps@12.000MHz
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
	
	ES = 1;
	EA = 1;
	
	TR1 = 1;		//��ʱ��1��ʼ��ʱ
}
void service_uart() interrupt 4
{
	if(RI == 1)
	{
		RI = 0;
		r_sbuf = SBUF;
	}
}
void sendchar(unsigned char Char)
{
	SBUF = Char;
	while(!TI);
	TI=0;
}
void sendstring(unsigned char *p)
{
	while(*p)
	{
		sendchar(*p);
		p++;
	}
}
void check_r_sbuf()
{
	if((r_sbuf & 0xf0) == 0xa0){P0 = (P0 & 0xf0) | ((~r_sbuf) & 0x0f);}
	else if((r_sbuf & 0xf0) == 0xb0){P0 = (P0 & 0x0f) | (~r_sbuf << 4);}
	else if((r_sbuf & 0xf0) == 0xc0){sendstring("XiaoyunOS is running");}
}