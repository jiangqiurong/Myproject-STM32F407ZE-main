#include "includes.h"

static RTC_InitTypeDef  RTC_InitStructure;
static RTC_DateTypeDef  RTC_DateStructure;
static RTC_TimeTypeDef  RTC_TimeStructure;
static EXTI_InitTypeDef EXTI_InitStructure;
static NVIC_InitTypeDef NVIC_InitStructure;
static RTC_AlarmTypeDef  RTC_AlarmStructure;


void rtc_init(void);

void alarm_a_init(void)
{
	
	/* ����RTC��A���Ӵ����ж� */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	/* ��ձ�־λ */
	RTC_ClearFlag(RTC_FLAG_ALRAF);

	
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable the RTC Alarm Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}


void alarm_set(void)
{
	
	
	/* �ر����ӣ������رգ��������Ӵ������ж���BUG��������ô���ã�ֻҪ��00�룬�򴥷��ж�*/
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_PM;
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x16;
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x39;
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x00;
//	
	//RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
	//RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	//RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;	//�������ں����ڣ���������ÿ�춼��Ч

	
	/* ����RTC��A���ӣ�ע��RTC���������������ֱ�Ϊ����A������B */
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
	
	/* ��RTC������A����*/
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	
	alarm_a_init();
}

void rtc_backup(void)
{
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x1111)//rtc��ʼ��
	{
		rtc_init();
		/* �Ա��ݼĴ���0д������0x1111�����ڱ���Ѿ���ʼ���� */
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x1111);
	}
	else
	{
		
		
		/* Enable the PWR clock ,ʹ�ܵ�Դʱ��*/
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC ���������RTC*/
		PWR_BackupAccessCmd(ENABLE);
		
		/* ʹ��LSI=32kmz*/
		RCC_LSICmd(ENABLE);
		
		/* ����LSI�Ƿ���Ч*/  
		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

		/* ѡ��LSI��ΪRTC��Ӳ��ʱ��Դ 1hz=32000/128/250*/
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		
		/* Enable the RTC Clock ��ʹ��RTCʱ��*/
		RCC_RTCCLKCmd(ENABLE);

		/* �ȴ�RTC��ؼĴ�������,�رռĴ���д����*/
		RTC_WaitForSynchro();
		
		/*��ʼ��rtc������RTC���ݼĴ�����RTC�ķ�Ƶֵ��ʱ���ʽ*/
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//�첽��Ƶϵ��0-127
		RTC_InitStructure.RTC_SynchPrediv = 0xF9;				//ͬ����Ƶϵ��0-249
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24Сʱ��ʽ
		RTC_Init(&RTC_InitStructure);
		
		
		
		//�رջ��ѹ���
		RTC_WakeUpCmd(DISABLE);
		
		//Ϊ���ѹ���ѡ��RTC���úõ�ʱ��Դ
		RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
		
		//���û��Ѽ���ֵΪ�Զ����أ�д��ֵĬ����0
		RTC_SetWakeUpCounter(1-1);
		
		//���RTC�����жϱ�־
		RTC_ClearITPendingBit(RTC_IT_WUT);
		
		//ʹ��RTC�����ж�
		RTC_ITConfig(RTC_IT_WUT, ENABLE);

		//ʹ�ܻ��ѹ���
		RTC_WakeUpCmd(ENABLE);
		
		/* Configure EXTI Line22�������ⲿ�жϿ�����22 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//��ǰʹ���ⲿ�жϿ�����22
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//�ж�ģʽ
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//�����ش����ж� 
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//ʹ���ⲿ�жϿ�����22
		EXTI_Init(&EXTI_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//����RTC�����жϴ���
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;//��ռ���ȼ�
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//��Ӧ���ȼ�Ϊ0x3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//ʹ��
		NVIC_Init(&NVIC_InitStructure);
	}
}

void rtc_init(void)
{
	
	
	
	/* Enable the PWR clock ,ʹ�ܵ�Դʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC ���������RTC*/
	PWR_BackupAccessCmd(ENABLE);
	
	/* ʹ��LSI=32kmz*/
	RCC_LSICmd(ENABLE);
	
	/* ����LSI�Ƿ���Ч*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

	/* ѡ��LSI��ΪRTC��Ӳ��ʱ��Դ 1hz=32000/128/250*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* Enable the RTC Clock ��ʹ��RTCʱ��*/
	RCC_RTCCLKCmd(ENABLE);

	/* �ȴ�RTC��ؼĴ�������,�رռĴ���д����*/
	RTC_WaitForSynchro();
	
	/*��ʼ��rtc������RTC���ݼĴ�����RTC�ķ�Ƶֵ��ʱ���ʽ*/
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//�첽��Ƶϵ��0-127
	RTC_InitStructure.RTC_SynchPrediv = 0xF9;				//ͬ����Ƶϵ��0-249
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24Сʱ��ʽ
	RTC_Init(&RTC_InitStructure);
	
	//BCD��ʽ�������ڡ�ʱ��
	/* Set the date: Wednesday 2024/10/30 */
	RTC_DateStructure.RTC_Year = 0x24;
	RTC_DateStructure.RTC_Month = RTC_Month_October;
	RTC_DateStructure.RTC_Date = 0x30;
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Wednesday;
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

	/* Set the time to 14h 56mn 00s PM  */
	RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	RTC_TimeStructure.RTC_Hours   = 0x16;
	RTC_TimeStructure.RTC_Minutes = 0x20;
	RTC_TimeStructure.RTC_Seconds = 0x00; 
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
	
	//�رջ��ѹ���
	RTC_WakeUpCmd(DISABLE);
	
	//Ϊ���ѹ���ѡ��RTC���úõ�ʱ��Դ
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//���û��Ѽ���ֵΪ�Զ����أ�д��ֵĬ����0
	RTC_SetWakeUpCounter(1-1);
	
	//���RTC�����жϱ�־
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//ʹ��RTC�����ж�
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	//ʹ�ܻ��ѹ���
	RTC_WakeUpCmd(ENABLE);
	
	/* Configure EXTI Line22�������ⲿ�жϿ�����22 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//��ǰʹ���ⲿ�жϿ�����22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//�����ش����ж� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//ʹ���ⲿ�жϿ�����22
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//����RTC�����жϴ���
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;	//��ռ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//��Ӧ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//ʹ��
	NVIC_Init(&NVIC_InitStructure);
}

//rtcʱ���жϺ���
void RTC_WKUP_IRQHandler(void)
{
	uint32_t ulReturn;

	
	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	
	if(RTC_GetITStatus(RTC_IT_WUT) == SET)
	{
		//�����¼���־��
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_RTC_WAKEUP,NULL);	
		
		
		RTC_ClearITPendingBit(RTC_IT_WUT);
		EXTI_ClearITPendingBit(EXTI_Line22);
	} 
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );	
}

void RTC_Alarm_IRQHandler(void)
{
	uint32_t ulReturn;
	
	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	

	if(RTC_GetITStatus(RTC_IT_ALRA) == SET)
	{
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_RTC_ALARM,NULL);
		
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);
	}
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );	

}
