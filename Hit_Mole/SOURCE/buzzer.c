#include "2450addr.h"
#include "option.h"
//#include "macro.h"

// for GPH10
void Buzzer_Init(void);
void Buzzer_Beep(int tone, int duration);
/* Buzzer Beep with Timer4 */
void Beep(int tone, int duration);
void DelayForPlay2(unsigned short time);
void SuccessBeep(void);
void FailBeep(void);
void TestBeep(void);
void StartBeep(void);
void EndBeep(void);
void saeta(void);

#define BASE10	10000

#define TONE_BEEP		1000
#define DURATION_5SEC	5000
#define DURATION_3SEC	3000
#define DURATION_1SEC	1000
#define DURATION_05SEC	 500
#define DURATION_03SEC   300
#define DURATION_01SEC   100
#define DURATION_005SEC   50

#define metro			60000/100

#define NOTE_16			0.25*metro
#define NOTE_12			0.375*metro
#define NOTE_8_5		0.75*metro
#define NOTE_8			0.5*metro
#define NOTE_4_5		1.5*metro
#define NOTE_4			1*metro
#define NOTE_2			2*metro
#define NOTE_1			4*metro

#define Z       0      		// Mute

#define C1      523     // Do
#define CS1     554
#define D1      587     // Re
#define DS1     622
#define E1      659     // Mi
#define F1      699     // Pa
#define FS1     740
#define G1      784     // Sol
#define GS1     831
#define A1      880     // La
#define AS1     932
#define B1      988     // Si

#define C2      C1*2    // Do
#define CS2     CS1*2
#define D2      D1*2    // Re
#define DS2     DS1*2
#define E2      E1*2    // Mi
#define F2      F1*2    // Pa
#define FS2     FS1*2
#define G2      G1*2    // Sol
#define GS2     GS1*2
#define A2      A1*2    // La
#define AS2     AS1*2
#define B2      B1*2    // Si

void Buzzer_Init(void)
{
	// Buzzer = GPB1
	rGPBDAT |= (0x1<<1);
	rGPBCON &= ~(0x3 << 2);
	rGPBCON |= (0x1<<2);
}

void Buzzer_Beep(int tone, int duration)
{
	unsigned int temp;
	
	for( ;(unsigned)duration > 0; duration--)
	{
		rGPBDAT &= ~(0x1<<1);
		for(temp = 0 ; temp < (unsigned)tone; temp++);
		rGPBDAT |= (0x1<<1);
		for(temp = 0 ; temp < (unsigned)tone; temp++);
	}
}

void Beep(int tone, int duration)
{
	rTCFG0 = (0xff<<8)|(0); 
	rTCFG1 = (rTCFG1 &~ (0xf<<20))|(3<<16); 
	
	/* TCON설정 :Dead zone disable,  auto reload on, output inverter off
	 * manual update no operation, timer0 stop, TCNTB0=0, TCMPB0 =0
	 */
	rTCNTB4 = 16.113*duration;
	rTCON &=~  (1<<22);
	rTCON |=  (1<<21);
	rTCON &= ~(1<<21);
	rTCON |=  (1<<20);

	while(rTCNTO4 !=0) 
	{
		rGPBDAT &= ~(0x1<<1);
		DelayForPlay2(BASE10/tone);
		rGPBDAT |= (0x1<<1);
		DelayForPlay2(BASE10/tone);
	}
	rTCON &= ~(1<<20);
}

void DelayForPlay2(unsigned short time)	// resolution=0.1ms
{
	/* Prescaler value : 39  */
	/* Clock Select    : 128 */
	rWTCON=(37<<8)|(3<<3);			// resolution=0.1ms
	rWTDAT=time+10;					// Using WDT
	rWTCNT=time+10;
	rWTCON|=(1<<5);

	while(rWTCNT>10);
	rWTCON = 0;
}

void SuccessBeep(void)
{
   Beep(C2, DURATION_01SEC);
   Beep(Z, DURATION_005SEC);
   Beep(E2, DURATION_01SEC);
   Beep(Z, DURATION_005SEC);
   
   Beep(G2, DURATION_01SEC);    
   Beep(Z, DURATION_005SEC);   
}

void FailBeep(void)
{
/*
	Beep(CS1, DURATION_005SEC);
	Beep(Z, DURATION_005SEC);
	Beep(CS2, DURATION_005SEC);
*/
/*
	Beep(z1, DURATION_005SEC);
	Beep(Z, DURATION_005SEC);
	Beep(z2, DURATION_005SEC);
*/
	Beep(F1, DURATION_01SEC);
	Beep(Z, DURATION_005SEC);
	Beep(F1, DURATION_01SEC);
	Beep(Z, DURATION_005SEC);
	Beep(E1, DURATION_01SEC);
	Beep(Z, DURATION_005SEC);
}

void TestBeep(void)
{	Beep(Z, DURATION_005SEC);
}

void StartBeep(void)
{
	//슈퍼마리오 메인 테마
	Beep(Z, DURATION_01SEC);
	Beep(E2, DURATION_1SEC);
	Beep(Z, DURATION_01SEC);
	Beep(E2, DURATION_1SEC);
	Beep(E2, DURATION_1SEC);
	Beep(Z, DURATION_01SEC);
	Beep(E2, DURATION_1SEC);
	Beep(E2, DURATION_1SEC);
	Beep(Z, DURATION_01SEC);
	Beep(C2, DURATION_1SEC);
	Beep(Z, DURATION_005SEC);
	Beep(E2, DURATION_1SEC);
	Beep(E2, DURATION_1SEC);
	Beep(Z, DURATION_005SEC);
	Beep(G2, DURATION_1SEC);
	
	Beep(Z, DURATION_005SEC);
	Beep(G1, DURATION_005SEC);
	
}

void EndBeep(void)
{
	Beep(C2, DURATION_01SEC);
   Beep(Z, DURATION_005SEC);
   Beep(E2, DURATION_01SEC);
   Beep(Z, DURATION_005SEC);
   
   Beep(G2, DURATION_01SEC);    
   
	
}


void saeta(void)
{	//1마디
	Beep(Z, DURATION_1SEC);
	Beep(D1,NOTE_4_5);
	Beep(Z, DURATION_005SEC);
	Beep(G1,NOTE_4_5);
	Beep(Z, DURATION_005SEC);
	Beep(AS1,NOTE_16);
	Beep(A1,NOTE_16);
	Beep(G1,NOTE_8_5);
	Beep(D1,NOTE_16);
	Beep(Z, DURATION_005SEC);
	Beep(G1,NOTE_4);
	Beep(Z, NOTE_8);
	
	//2마디
	Beep(Z, NOTE_8);
	Beep(G1,NOTE_4);
	Beep(Z, DURATION_005SEC);
	Beep(AS1,NOTE_16);
	Beep(A1,NOTE_16);
	Beep(Z, DURATION_005SEC);
	Beep(D2, NOTE_8);
	Beep(DS1, NOTE_8_5);
	Beep(G1,NOTE_16);
	Beep(Z, DURATION_005SEC);
	Beep(AS1,NOTE_16);
	Beep(A1,NOTE_16);
	Beep(Z, DURATION_005SEC);
	Beep(G1,NOTE_4);
	Beep(Z, DURATION_005SEC);
	Beep(G1,NOTE_4);
	Beep(Z, DURATION_005SEC);
	Beep(Z, NOTE_8);
}
void Suc_Beep(void){
	Beep(C1, 	DURATION_005SEC);
	Beep(Z, 	DURATION_005SEC);
	Beep(E1,	DURATION_005SEC);
	Beep(Z,	 	DURATION_005SEC);
	Beep(G1,	DURATION_005SEC);

}
