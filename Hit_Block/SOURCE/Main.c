#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "2450addr.h"
#include "my_lib.h"
#include "option.h"
#include "images.h"

//함수
void HW_Init(void);

//변수
extern int Touch_pressed;
int militime;

//하드웨어 초기화
void HW_Init(void)
{
	MMU_Init(); //MMU init
	Init_Led(); // LED init
	Init_Key(); // button init
 	Uart_Init(115200); 
	Lcd_Port_Init();
	NonPal_Lcd_Init();
	Buzzer_Init(); //buzzer init by LHM	
	Touch_Isr_Init(Touch_ISR); // touch interuct init
	Touch_ISR_Enable(1);	//touch interuct enable
	Lcd_Select_Frame_Buffer(0);
}


void Main(void)
{
	HW_Init();					//사용하는 하드웨어 초기화
	
	int i,j;					//반복문에서 사용함
	int score;					//점수
	int combo; 					//콤보 표시, 게임 점수에 영향
	int life; 					//생명, 생명 갯수가 0이 되면 게임 오버 
	int total=10;				//블록 갯수 설정 
	int highscore=0;			//전원이 켜진 후부터의 최고 점수 기록
	int list[total];			//블록의 모양을 저장
	militime=0;					//게임 제한 시간, 터치 입력전에는 난수 발생을 위한 변수로 사용
	
	while(1)
	{
		//////////////////////////// 게임 시작 대기화면 ////////////////////////////////////////////
		
		Lcd_Draw_BMP(0,0,start); //대기화면 이미지 출력
		Lcd_Printf(330,20,0xfffe,0,1,1,"high score : %d",highscore);
		while(Touch_pressed==0)			//터치가 안되고 있을 때 반복
		{			
			Lcd_Printf(240,100,0xf800,0,2,2,"Touch Screen"); //화면에 문자를 찍기 위해
			Lcd_Printf(270,130,0xf800,0,2,2,"to start");
			for(i=0;i<10;i++) //글자를 일정시간만큼 보이기 위해 딜레이
			{
				Timer_Delay(50); 
				militime++; //rand 함수 사용을 위해 증가
				if (Touch_pressed) break; //터치체크 1이면 while을 빠져 나간다
			}
			if (Touch_pressed) break; //터치체크 1이면 while을 빠져 나간다
			Lcd_Printf(240,100,0,0,2,2,"Touch Screen"); //화면에 있는 문자를 지우기 위해
			Lcd_Printf(270,130,0,0,2,2,"to start");
			for(i=0;i<10;i++) //글자가 일정시간만큼 없음을 보이기 위해 딜레이
			{
				Timer_Delay(50);
				militime++; //rand 함수 사용을 위해 증가
				if (Touch_pressed) break; //터치체크 1이면 while을 빠져 나간다
			}
			if (Touch_pressed) break; //터치체크 1이면 while을 빠져 나간다
		}
		//////////////////////////// 게임 시작 대기화면 끝 //////////////////////////////////////////
		
		/////////////////////////////////// 게임시작 ////////////////////////////////////////////////
		StartBeep(); //시작 음악 
		srand(militime); // 난수 발생
		
		///////////////////////////// 게임 변수 초기화 시작 /////////////////////////////////////////
		score=0;
		combo=0;
		life=5;
		militime=6000;
		/////////////////////////// 게임 변수 초기화 완료 ///////////////////////////////////////////
		
		////////////////////////////////// 그래픽 출력 //////////////////////////////////////////////
		Lcd_Draw_BMP(0,0,ingame); 						//바탕화면 검은색
		for(i=0;i<5;i++) Lcd_Draw_BMP(i*52,0,heart); 	//생명 
		for(i=0;i<total;i++) list[i]=1+rand()%3; 		//list에 블록 세팅
			
		for(j=0;j<total;j++) 	//블록 갯수만큼 반복
		{
			if (life==0) break; //생명 체크
			for(i=1;i<=6;i++) 	//화면에 블록 6개를 출력
			{
				if (i+j>total) Lcd_Draw_BMP(380,270-i*50,black); //블록이 6개 미만이면  빈공간에 검은색 출력
				else if(list[j+i-1]==1) Lcd_Draw_BMP(380,270-i*50,red); 	//블록 출력 red
				else if(list[j+i-1]==2) Lcd_Draw_BMP(380,270-i*50,blue);	//블록 출력 blue
				else if(list[j+i-1]==3) Lcd_Draw_BMP(380,270-i*50,green);	//블록 출력 green
			}
			militime-=50;													//시간 감소
		//////////////////////////////// 그래픽 출력 끝 //////////////////////////////////////////////
			
		////////////////////////// 블록과 버튼 체크, 스코어&하트 갱신 ////////////////////////////////
			while(1)
			{
				//score 출력
				Lcd_Printf(20,50,0xffc0,0,1,1,"score : %d",score);
				
				//현재 스코어가 하이스코어보다 클 경우 현재 스코어 출력
				if(score>highscore) Lcd_Printf(220,50,0xffc0,0,1,1,"high score : %d",score);
				else Lcd_Printf(220,50,0xffc0,0,1,1,"high score : %d",highscore); //highscore 출력
				//correct 지움
				Lcd_Printf(50,70,0xf800,0,1,1,"         ");

				i=Wait_KeyPressed()-1; //보드의 버튼 값
				
				if(i==0) //리셋버튼 SW15 체크
				{					
					life=0; //리셋일때 블록과 버튼체크 루틴을 벗어나 그래픽 출력 루프로 돌아감.
					break;  //그곳에서 Life 체크를 통해 게임 전체 루프가 리셋됨.
				}

				if (i==list[j]) //버튼이 블록하고 일치
				{
					//해당 버튼의 LED를 켰다 끈다.
					Led_On(i);
					Timer_Delay(50);
					Led_Off(i);
					
					//성공 부저 울림
					SuccessBeep();					
					
					Lcd_Printf(50,70,0xf800,0,1,1,"correct");
					combo++;
					if(combo>=2) Lcd_Printf(220,120,0xf800,0,2,2,"%d combo",combo);
					score+=combo; //스코어에 콤보수 더함
					
					break;
				}
				else //버튼이 블록하고 불일치
				{
					// LED 4개 전부를 켰다 끈다
					Led_On(999);
					Timer_Delay(50);
					Led_Off(999);
					//실패 부저 울림
					FailBeep();	
					
					//wrong 출력
					Lcd_Printf(50,70,0xf800,0,1,1,"wrong  ");
					//combo 지움
					Lcd_Printf(220,120,0xf800,0,2,2,"         ");
					//생명 깍임
					life--;
					Lcd_Draw_BMP(life*52,0,noheart);
					//생명 체크 0일때 게임오버
					if (life==0) break;
					combo=0;
				}			
			}
		///////////////////////// 블록과 버튼 체크, 스코어&하트 갱신 끝 ///////////////////////////////
		}
		///////////////////////////////////// 게임 오버 ///////////////////////////////////////////////
		Lcd_Draw_BMP(0,0,over); //게임오버 화면 출력
		score=score+(militime/200)+life*10; //최종스코어=콤보점수+남은시간+남은생명*10
		Lcd_Printf(200,230,0xffc0,0,1,1,"score : %d",score);
		
		//스코어가 하이스코어보다 클때 
		if (score>highscore)
		{
			Lcd_Printf(300,230,0xf800,0,1,1,"new record!"); 
			highscore=score; //하이스코어 갱신
		}
		EndBeep();
		Timer_Delay(3000);  //새게임 전 대기
		/////////////////////////////////// 게임 오버 끝 //////////////////////////////////////////////
	}
		/////////////////////////////////// 게임 시작 끝///////////////////////////////////////////////
	
}
