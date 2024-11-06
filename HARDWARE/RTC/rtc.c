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
	
	/* 允许RTC的A闹钟触发中断 */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	/* 清空标志位 */
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
	
	
	/* 关闭闹钟，若不关闭，配置闹钟触发的中断有BUG，无论怎么配置，只要到00秒，则触发中断*/
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_PM;
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x16;
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x39;
//	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x00;
//	
	//RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
	//RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	//RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;	//屏蔽日期和星期，就是闹钟每天都生效

	
	/* 配置RTC的A闹钟，注：RTC的闹钟有两个，分别为闹钟A与闹钟B */
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
	
	/* 让RTC的闹钟A工作*/
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	
	alarm_a_init();
}

void rtc_backup(void)
{
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x1111)//rtc初始化
	{
		rtc_init();
		/* 对备份寄存器0写入数据0x1111，用于标记已经初始化过 */
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x1111);
	}
	else
	{
		
		
		/* Enable the PWR clock ,使能电源时钟*/
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC ，允许访问RTC*/
		PWR_BackupAccessCmd(ENABLE);
		
		/* 使能LSI=32kmz*/
		RCC_LSICmd(ENABLE);
		
		/* 检查该LSI是否有效*/  
		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

		/* 选择LSI作为RTC的硬件时钟源 1hz=32000/128/250*/
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		
		/* Enable the RTC Clock ，使能RTC时钟*/
		RCC_RTCCLKCmd(ENABLE);

		/* 等待RTC相关寄存器就绪,关闭寄存器写保护*/
		RTC_WaitForSynchro();
		
		/*初始化rtc，配置RTC数据寄存器与RTC的分频值，时间格式*/
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//异步分频系数0-127
		RTC_InitStructure.RTC_SynchPrediv = 0xF9;				//同步分频系数0-249
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24小时格式
		RTC_Init(&RTC_InitStructure);
		
		
		
		//关闭唤醒功能
		RTC_WakeUpCmd(DISABLE);
		
		//为唤醒功能选择RTC配置好的时钟源
		RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
		
		//设置唤醒计数值为自动重载，写入值默认是0
		RTC_SetWakeUpCounter(1-1);
		
		//清除RTC唤醒中断标志
		RTC_ClearITPendingBit(RTC_IT_WUT);
		
		//使能RTC唤醒中断
		RTC_ITConfig(RTC_IT_WUT, ENABLE);

		//使能唤醒功能
		RTC_WakeUpCmd(ENABLE);
		
		/* Configure EXTI Line22，配置外部中断控制线22 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//当前使用外部中断控制线22
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//中断模式
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//上升沿触发中断 
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//使能外部中断控制线22
		EXTI_Init(&EXTI_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//允许RTC唤醒中断触发
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;//抢占优先级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//响应优先级为0x3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//使能
		NVIC_Init(&NVIC_InitStructure);
	}
}

void rtc_init(void)
{
	
	
	
	/* Enable the PWR clock ,使能电源时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC ，允许访问RTC*/
	PWR_BackupAccessCmd(ENABLE);
	
	/* 使能LSI=32kmz*/
	RCC_LSICmd(ENABLE);
	
	/* 检查该LSI是否有效*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

	/* 选择LSI作为RTC的硬件时钟源 1hz=32000/128/250*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* Enable the RTC Clock ，使能RTC时钟*/
	RCC_RTCCLKCmd(ENABLE);

	/* 等待RTC相关寄存器就绪,关闭寄存器写保护*/
	RTC_WaitForSynchro();
	
	/*初始化rtc，配置RTC数据寄存器与RTC的分频值，时间格式*/
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//异步分频系数0-127
	RTC_InitStructure.RTC_SynchPrediv = 0xF9;				//同步分频系数0-249
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24小时格式
	RTC_Init(&RTC_InitStructure);
	
	//BCD格式设置日期、时间
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
	
	//关闭唤醒功能
	RTC_WakeUpCmd(DISABLE);
	
	//为唤醒功能选择RTC配置好的时钟源
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//设置唤醒计数值为自动重载，写入值默认是0
	RTC_SetWakeUpCounter(1-1);
	
	//清除RTC唤醒中断标志
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//使能RTC唤醒中断
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	//使能唤醒功能
	RTC_WakeUpCmd(ENABLE);
	
	/* Configure EXTI Line22，配置外部中断控制线22 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//当前使用外部中断控制线22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//上升沿触发中断 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//使能外部中断控制线22
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//允许RTC唤醒中断触发
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;	//抢占优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//响应优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//使能
	NVIC_Init(&NVIC_InitStructure);
}

//rtc时钟中断函数
void RTC_WKUP_IRQHandler(void)
{
	uint32_t ulReturn;

	
	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	
	if(RTC_GetITStatus(RTC_IT_WUT) == SET)
	{
		//设置事件标志组
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_RTC_WAKEUP,NULL);	
		
		
		RTC_ClearITPendingBit(RTC_IT_WUT);
		EXTI_ClearITPendingBit(EXTI_Line22);
	} 
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );	
}

void RTC_Alarm_IRQHandler(void)
{
	uint32_t ulReturn;
	
	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	

	if(RTC_GetITStatus(RTC_IT_ALRA) == SET)
	{
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_RTC_ALARM,NULL);
		
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);
	}
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );	

}
