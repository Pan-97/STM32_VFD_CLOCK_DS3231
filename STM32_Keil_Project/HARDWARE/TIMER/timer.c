/*************************************************************************************************************
�����ۺϵ��ӹ�����(GIE������)        ��Ʒ
Gearing Interated Electronics Studio

ʮ��IN14������Թ�����վ+������

����Уʱ�Ͳ�����������жϵ���غ��������Ի�������ԭ��


��ע����ID��tnt34 ������¹����ɹ�
��עBվUP����GIE������ ��ø�����Ƶ��Դ

2018-7-31��һ��
��Ȩ���� ��ֹ�����κ���ҵ��;��
ע����������DS3231��EPS8266��DS18B20��������ļ����Ի��������������Ұ�Ȩ������glow_tube_display.c & glow_tube_display.h
***********************************************************************************************************/
#include <sys.h>
#include "timer.h"
#include "esp8266.h"

extern NTP NetTime;
extern void clock_irq(void);
clock localTime;
/**
  *@brief timer2 used for local time counting
  *      TIM2 is 72MHz
  */
void timer2ForClock(void)
{
  RCC->APB1ENR |= 0x00000001;
  TIM2->PSC = 7199;
  TIM2->ARR = 9999;
  TIM2->EGR  |= 0x0001;
  TIM2->DIER |= 0x0001;
  TIM2->CR1  |= 0x0001;
  NVIC->ISER[0] |= 1<<28;
  NVIC->IP[28] = 2<<4;
}


int localmonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 oneHourFlag=0;
void TIM2_IRQHandler(void)
{
  if(TIM2->SR&1)
  {
    localTime.sec += 1;
    if(60 == localTime.sec)
    {
      localTime.sec = 0;
      localTime.min += 1;
      if(60 == localTime.min)
      {
        localTime.min = 0;
        localTime.hour += 1;
        if(24 == localTime.hour)
        {
          oneHourFlag = 1;
          localTime.hour = 0;
          localTime.day += 1;
          localTime.dateTemp +=1;
          localTime.date = localTime.dateTemp%7;
          if(localTime.day > NetTime.daysInMonth )
          {
            localTime.day = 1;
            localTime.month += 1;
            NetTime.daysInMonth = localmonth[localTime.month-1];
            if(localTime.month > 11)
            {
              localTime.month = 0;
              localTime.year += 1;
            }
          }
        }
      }
    }
    
  }
  
  TIM2->SR = 0;
}

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx					 
}
//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
		{
		//if(Radio->Process() != RF_IDLE)
			clock_irq();
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
		}
}

