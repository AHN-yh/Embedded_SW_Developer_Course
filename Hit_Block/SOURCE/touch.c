#include "2450addr.h"

//터치패드
#define LCD_XSIZE 		(480)	
#define LCD_YSIZE 		(272)

volatile int Touch_pressed = 0;
volatile int ADC_x=0, ADC_y=0;

volatile int Cal_x1=848;
volatile int Cal_y1=656;
volatile int Cal_x2=186;
volatile int Cal_y2=349; 


volatile int Touch_x, Touch_y;
volatile unsigned int Touch_config=1;

void Touch_ISR(void) __attribute__ ((interrupt ("IRQ")));
void Touch_ISR()
{
	rINTSUBMSK |= (0x1<<9);
	rINTMSK1 |= (0x1<<31);	
	
	/* TO DO: Pendng Clear on Touch */	
	rSUBSRCPND |= (0x1<<9);
	rSRCPND1 |= (0x1<<31);
	rINTPND1 |= (0x1<<31);
	
	// Touch UP
	if(rADCTSC&0x100)
	{
		rADCTSC&=0xff;
		Touch_pressed = 0;
	}
	// Touch Down
	else 
	{
		rADCTSC=(0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);
		// SADC_ylus Down,Don't care,Don't care,Don't care,Don't care,XP pullup Dis,Auto,No operation
		rADCCON|=0x1;
		while(rADCCON & 0x1);
		while(!(0x8000&rADCCON));
		ADC_x=(int)(0x3ff&rADCDAT0);
		ADC_y=(int)(0x3ff&rADCDAT1);
		// Touch calibration complete
		if(Touch_config)
		{
			Touch_y=(ADC_y-Cal_y1)*(LCD_YSIZE-10)/(Cal_y2-Cal_y1)+5;
			Touch_x=(ADC_x-Cal_x2)*(LCD_XSIZE-10)/(Cal_x1-Cal_x2)+5;
			Touch_x=LCD_XSIZE-Touch_x;
			if(Touch_x<0) Touch_x=0;
			if(Touch_x>=LCD_XSIZE) Touch_x=LCD_XSIZE-1;
			if(Touch_y<0) Touch_y=0;
			if(Touch_y>=LCD_YSIZE) Touch_y=LCD_YSIZE-1;
		}
		// before calibration		
		else
		{
			Touch_x = ADC_x;
			Touch_y = ADC_y;
		}

		rADCTSC=(1<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
		// SADC_ylus Up,Don't care,Don't care,Don't care,Don't care,XP pullup En,Normal,Waiting mode
		Touch_pressed = 1; 
	}

	rINTSUBMSK &= ~(0x1<<9);
	rINTMSK1 &= ~(0x1<<31);
}


void Touch_Init(void)
{
	rADCDLY = (50000); 
     /* TO DO : prescaler enable, prescaler value=39, Analog no input, 
      * 		Normal operation mode, Disable start, No operation */
	rADCCON =(1<<14)|(39<<6)|(0<<3)|(0<<2)|(0<<1)|(0);  
	 
	 /* TO DO :  For Waiting Interrupt Mode rADCTSC=0xd3 */
	rADCTSC =(0<<8)|(1<<7)|(1<<6)|(1<<4)|(0<<3)|(0<<2)|(3);	
}


void Touch_Isr_Init(void (*fp)(void))
{
	rADCDLY=(50000);	
	// Enable Prescaler,Prescaler,AIN5/7 fix (MUX don't care),Normal,Disable read start,No operation
	rADCCON = (1<<14)+(39<<6)+(0<<3)+(0<<2)+(0<<1)+(0);	
	// Wait Down,YM:GND,YP:Hi-z,XM:Hi-z,XP:DIS,XP pullup En,Normal,Waiting for interrupt mode     
	rADCTSC=(0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);

	pISR_ADC = (unsigned int)fp;
}

void Touch_ISR_Enable(int enable)
{	
	enable? (rINTMSK1 &= (unsigned)(~(1<<31))) : (rINTMSK1 |= (unsigned)(1<<31));
	enable? (rINTSUBMSK &= (unsigned)(~(1<<9))) : (rINTSUBMSK |= (unsigned)(1<<9));	
}
