/*
 * =====================================================================
 * NAME         : Main.c
 *
 * Descriptions : Main routine for S3C2450
 *
 * IDE          : GCC-4.1.0
 *
 * Modification
 *	  
 * =====================================================================
 */
#include "2450addr.h"
#include "my_lib.h"
#include "option.h"
#include <stdlib.h>
#include <time.h>

#include "title_.h"
#include "mole.h"
#include "bg.h"
#include "HIDE.H"
#include "SC.H"

/*#define BLACK	0x0000
#define WHITE	0xFFFF
#define BLUE	0x001F
#define GREEN	0x03E0
#define RED		0x7C00*/
#define  NonPal_Fb   ((volatile unsigned short(*)[480]) FRAME_BUFFER)

extern int Touch_Pressed;
extern int Touch_x, Touch_y;

void Show_Title(int *);

typedef struct _mole{
	int x;
	int y;
	int on;
}Mole;

void Main(void)
{	
	Mole moles[6] = { {133,66,0}, {349,17,0} , {368,90,0} 
	,{258,135,0}, {83,162,0}, {364,178,0}	};
	int score = 0;
	int i,j,stage=0;
	int cnt=0;
	int time = 60;
	int times=time*50;
	
	int lev = 150;// 줄을 수록 난이도 업 
	/*
	<<STAGE 2>> if score 200: time + 10, lev 200 
	<<STAGE 3>> if score 500: time + 10, lev 125
	<<FINAL STAGE>> if score 1000: time + 10, lev 50
	*/

	Buzzer_Init();
	Timer0_Init();
	MMU_Init();
	Uart_Init(115200);	
	Lcd_Port_Init();
	NonPal_Lcd_Init();
	Lcd_Select_Frame_Buffer(0);
	Touch_Init();	
	
	//Uart_Printf("*** Touch Test *** \n");
	
	// TO DO : 인터럽트 벡터에 Touch_ISR 함수 등록
	pISR_ADC = (unsigned int)Touch_ISR;
	
	// TO DO :  인터럽트 허용 on Touch 
	rINTSUBMSK &= ~(0x1<<9);
	rINTMSK1 &= ~(0x1<<31);	
	while(1){
		Touch_Pressed=0;
		times=time*50;
		stage = 0;
		score = 0;
		lev = 150;
		Show_Title(&Touch_Pressed);

		Lcd_Draw_BMP(0, 0, bg);
		Lcd_Printf(150,0,0xCCCC,0x0000,1,1,"Score: %d",score);
	
		while(1){
			times--;
			Lcd_Printf(150,0,0xCCCC,0x0000,1,1,"Score: %d",score);
			Lcd_Printf(50,0,0xCCCC,0x0000,1,1,"Stage: %d",stage+1);
			Lcd_Printf(250,0,0xCCCC,0x0000,1,1,"Time: %4d",times/time);
			if(times<=0) break;
			if(!(rand()%(lev/3))) if(cnt<3) {
				j=rand()%6;
				if(moles[j].on==0)
				{
					draw_mole(moles,j);
					cnt++;
				}
			}
			if(!(rand()%(lev/(stage+1)))){
				j=rand()%6;
				if(moles[j].on==1)
				{
					Lcd_Draw_BMP(moles[j].x,moles[j].y,hide);
					moles[j].on=0;
					cnt--;
				}
			}
			while(Touch_Pressed==1){
				
				for(i=0;i<6;i++)
					if(check_mole(moles,i)){
						Lcd_Printf(150,0,0xCCCC,0x0000,1,1,"Score: %d",score=score+10);
						Lcd_Draw_BMP(moles[i].x, moles[i].y, hide);
						moles[i].on=0;
						cnt--;
						Uart_Printf("HIT! SCORE : %d\n",score);
						Suc_Beep();
						times -= time / 5;
					}
				break;
			}
			/*
			<<STAGE 2>> if score 200: time + 10, lev 200 
			<<STAGE 3>> if score 500: time + 10, lev 125
			<<FINAL STAGE>> if score 1000: time + 10, lev 50
			*/
			if(score == 200 && stage == 0)	{
				Uart_Printf("==========STAGE 2==========\n");
				times += 10*time;
				
				lev = 50;
				for(j=0;j<3;j++)
				{
					Lcd_Printf(200,130,0xFFFF,0x0000,1,1,"Level UP");
					Timer0_Delay(100);
					EndBeep();
					Lcd_Printf(200,130,0x0000,0x0000,1,1,"Level UP");
					
				}
				Lcd_Draw_BMP(0, 0, bg);
				stage = 1;
				
			}
			if(score == 500 && stage == 1 ) {
				Uart_Printf("==========STAGE 3==========\n");
				times = times + 10*time; 
				lev = 40;
				for(j=0;j<3;j++)
				{
					Lcd_Printf(200,130,0xFFFF,0x0000,1,1,"Level UP");
					Timer0_Delay(100);
					EndBeep();
					Lcd_Printf(200,130,0x0000,0x0000,1,1,"Level UP");
					Lcd_Draw_BMP(0, 0, bg);
				}
				Lcd_Draw_BMP(0, 0, bg);
				stage = 2;
				
			}
			if(score == 1000 && stage == 2) 
			{
				Uart_Printf("==========FINAL STAGE==========\n");
				lev = 30;
				for(j=0;j<3;j++)
				{
					Lcd_Printf(200,130,0x7C00,0x0000,1,1,"FINAL STAGE");
					Timer0_Delay(100);
					EndBeep();
					Lcd_Printf(200,130,0x0000,0x0000,1,1,"FINAL STAGE");
					
				}
				Lcd_Draw_BMP(0, 0, bg);
				stage = 3;
				times += 10*time;
			}
		}
		Lcd_Draw_BMP(0, 0, sc);
		Lcd_Printf(0,20,0xfffe,0,2,2,"수고했다 이놈아");
		Lcd_Printf(0,100,0xfffe,0,2,2,"새참이나 먹어라");
		Lcd_Printf(40,170,0xfffe,0,2,2,"올해 농사는 풍년이여");
		Lcd_Printf(170,210,0xCCCC,0,2,2,"Score: %d",score);
		Timer0_Delay(100);
		Lcd_Printf(170,210,0,0,2,2,"Score: %d",score);
		Timer0_Delay(100);
		Lcd_Printf(170,210,0xCCCC,0,2,2,"Score: %d",score);
		Timer0_Delay(100);
		Lcd_Printf(170,210,0,0,2,2,"Score: %d",score);
		Timer0_Delay(100);
		Lcd_Printf(170,210,0xCCCC,0,2,2,"Score: %d",score);
		saeta();
		
		Timer0_Delay(1000);
	}
}		

void Show_Title(int *Touch_Pressed){
	int i;
	Lcd_Draw_BMP(0, 0, title_); 
	Led_Display(0);
	while(*Touch_Pressed==0)
		{

		Lcd_Printf(288,118,0xFFFF,0xCCCC,2,2,"Start!");
		
		for(i=0;i<50;i++)		{Timer0_Delay(10); 	if(*Touch_Pressed)	break;}
		if(*Touch_Pressed)	break;
		Lcd_Printf(288,118,0xCCCC,0xFFFF,2,2,"Start!");
		
		for(i=0;i<50;i++)		{Timer0_Delay(10); 	if(*Touch_Pressed)	break;}
		if(*Touch_Pressed)	break;
		}
	SuccessBeep();
}

void draw_mole(Mole*moles,int i){
	Lcd_Draw_BMP((moles+i)->x, (moles+i)->y, mole);
	(moles+i)->on=1;
}

int check_mole(Mole*moles,int i)
{
	if((moles+i)->on&&Touch_x>(moles+i)->x&&Touch_x<(moles+i)->x+48&&Touch_y>(moles+i)->y&& Touch_y<(moles+i)->y+48) return 1;
	else return 0;
}

int time_set(int sec, int degree){

}


