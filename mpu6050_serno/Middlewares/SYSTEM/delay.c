#include "delay.h"
#include "sys.h"
 
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)


#if no_RTOS
//roughly delay
void delay_us(u32 nus)
{		
	u32 i;
	u16 temp;
	temp = nus*SYS_CLK/15;
	for(i=0;i<temp;i++);
}	 		      

//��ʱnms
//nms:Ҫ��ʱ��ms��
void delay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}
#endif

void delay_init(void)
{
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	/* Configure the SysTick to have interrupt in 1ms time basis*/
	HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
}

#if OS_SUPPORT 						    								   
void delay_us(u32 nus)
{		
	u32 ticks;
       u32 told,tnow,reload,tcnt=0;
       if((0x0001&(SysTick->CTRL)) ==0)    //��ʱ��δ����
              vPortSetupTimerInterrupt();  //��ʼ����ʱ��
 
       reload = SysTick->LOAD;                     //��ȡ��װ�ؼĴ���ֵ
       ticks = nus * (SystemCoreClock / 1000000);  //����ʱ��ֵ
       
       vTaskSuspendAll();//��ֹOS���ȣ���ֹ���us��ʱ
       told=SysTick->VAL;  //��ȡ��ǰ��ֵ�Ĵ���ֵ����ʼʱ��ֵ��
       while(1)
       {
              tnow=SysTick->VAL; //��ȡ��ǰ��ֵ�Ĵ���ֵ
              if(tnow!=told)  //��ǰֵ�����ڿ�ʼֵ˵�����ڼ���
              {         
                     if(tnow<told)  //��ǰֵС�ڿ�ʼ��ֵ��˵��δ�Ƶ�0
                          tcnt+=told-tnow; //����ֵ=��ʼֵ-��ǰֵ
 
                     else     //��ǰֵ���ڿ�ʼ��ֵ��˵���ѼƵ�0�����¼���
                            tcnt+=reload-tnow+told;   //����ֵ=��װ��ֵ-��ǰֵ+��ʼֵ  ��
                                                      //�Ѵӿ�ʼֵ�Ƶ�0�� 
 
                     told=tnow;   //���¿�ʼֵ
                     if(tcnt>=ticks)break;  //ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
              } 
       }  
       xTaskResumeAll();	//�ָ�OS����		   
}  

void delay_ms(u16 nms)
{	
//	if(delay_osrunning&&delay_osintnesting==0)    
//	{		 
//		if(nms>=fac_ms)						
//		{ 
//   			delay_ostimedly(nms/fac_ms);	
//		}
//		nms%=fac_ms;						 
//	}
	delay_us((u32)(nms*1000));			
}
#else  
# if freeRTOS
void delay_us(u32 nus)
{	/*	
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				    	 
	ticks=nus*SYS_CLK; 						
	told=SysTick->VAL;        				
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	}*/
	uint8_t i;
    for (i = 0; i < 10; i++);
}

void delay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}
#endif
#endif
