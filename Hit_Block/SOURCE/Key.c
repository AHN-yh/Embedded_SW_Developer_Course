#include "2450addr.h"

extern int militime;

int Wait_KeyPressed(void)
{		
	int j;
	while(rGPFDAT==0x7f)
	{
		militime--;
		Lcd_Printf(290,10,0xfffe,0,2,2,"%2d",militime/200);
		if (militime<=0) return 1;
	}
	for(j=0;j<7;j++) if((rGPFDAT^0x7f)>>j==1) break;
	return j;
}
void Init_Key(void)
{
    //07.10 버튼 배열 변경 by LHM //
	rGPFCON &= ~(0xffff);  // 0000000000000000 GPF를 0으로 클리어 //
    //07.10 버튼 4~8번 사용이 필요 없어 설정 막음 by LHM
	//rGPFCON = (rGPFCON & ~(0x3 << 14)) | 0x1 << 14; //0100000000000000 -> GPF7[15:14] 01=Output mode set //
	//rGPFDAT &= ~(0x1 << 7);
	rGPFCON |= 0x1; //GPF[1:0] 버튼 15번 사용을 위해 outputmode 설정 by LHM
	rGPGCON |= 0x1;	//GPG[1:0] 01=Output mode set // 버튼 8~11번까지 사용
}
void Init_Led(void)
{
	rGPGDAT|=0xf<<4;
	rGPGCON=(rGPGCON&~(0xff<<8))|(0x55<<8);
}
void Led_On(int num)
{
	//07.10 버튼 9,10,11번이 아니면 모든 LED on by LHM //
	if(num<4)
	{
		rGPGDAT&=~(0x1<<(num+3));		
	}
	else
	{
		rGPGDAT&=~(0xf<<4);		
	}
}
void Led_Off(int num)
{
	if(num<4)
	{
		rGPGDAT|=1<<(num+3);
	}
	else
	{
		rGPGDAT|=(0xf<<4);
	}
}
void Timer_Delay(int msec)
{
	int i, rep = msec * 0x1ff;
	for(i=0; i<rep; i++);
}