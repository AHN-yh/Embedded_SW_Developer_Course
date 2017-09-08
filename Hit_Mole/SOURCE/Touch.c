#include "2450addr.h"

#define LCD_XSIZE       (480)   
#define LCD_YSIZE       (272)

void Touch_Init(void);

void Touch_ISR(void) __attribute__ ((interrupt ("IRQ")));

volatile  int ADC_x, ADC_y;

volatile 	int ADC_x=0, ADC_y=0;
volatile  int Touch_Pressed=0;


// Calibration 
volatile int Cal_x1=848;
volatile int Cal_y1=656;
volatile int Cal_x2=186;
volatile int Cal_y2=349; 

volatile int Touch_x, Touch_y;

volatile unsigned int Touch_config=1; 

void Touch_ISR()
{
   /* TO DO: Pendng Clear on Touch */   
   rSUBSRCPND = (0x1<<9);
   rSRCPND1 = (0x1<<31);
   rINTPND1 = (0x1<<31);
   
   // Touch UP
   if(rADCTSC&0x100)
   {
      rADCTSC&=0xff;
      Touch_Pressed = 0;
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
      Touch_Pressed = 1; 
	  //Uart_Printf("%d %d\n",Touch_x,Touch_y);
   }
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

