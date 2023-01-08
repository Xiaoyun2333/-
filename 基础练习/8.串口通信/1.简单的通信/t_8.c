#include<stc15.h>
#include<intrins.h>

unsigned char r_sbuf;

void UartInit();//���ڳ�ʼ��
void service_uart();//�����жϷ���
void send(unsigned char t_sbuf);//���ڷ���

void main()
{
	UartInit();
	
	send(0x5a);
	send(0xa5);
	while(1)
	{
		
	}
}
void UartInit()	//9600bps@11.0592MHz
{
	PCON &= 0x7F;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR &= 0xBF;		//��ʱ��ʱ��12Tģʽ
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//���ö�ʱ��ģʽ
	TMOD |= 0x20;		//���ö�ʱ��ģʽ
	TL1 = 0xFD;		//���ö�ʱ��ʼֵ
	TH1 = 0xFD;		//���ö�ʱ����ֵ
	ET1 = 0;		//��ֹ��ʱ��%d�ж�
	TR1 = 1;		//��ʱ��1��ʼ��ʱ
	
	ES = 1;
	EA = 1;
}
void service_uart() interrupt 4
{
	if(RI == 1)
	{
		RI = 0;
		r_sbuf = SBUF;
		send(r_sbuf + 1);
	}
}
void send(unsigned char t_sbuf)
{
	SBUF = t_sbuf;
	while(TI == 0);
	TI = 0;
}