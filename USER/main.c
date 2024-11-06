/****************************************************************
*名    称:FreeRTOS综合代码实例-LCD版本
*作    者:江秋荣
*创建日期:2024/10/30
*说  明:
	1.当前代码规范、结构清晰，尽可能实现了模块化编程，一个任务管理一个硬件。
	  该代码框架沿用《FreeRTOS综合代码实例-OLED版本》，重点将OLED显示修改为LCD显示
	2.菜单项对应硬件模块功能
		菜单第1页
			.Home 			[已实现]  显示当前时间、日期，显示蓝牙、闹钟图标
			.Blue tooth		[已实现]  时间、日期、闹钟设置
			.Sport			[已实现]  记步、久坐提醒
			.Pulse SO2		[已实现]  心率血氧
			.DHT11  		[已实现]  温湿度

		菜单第2页
			.Password 		[已实现]  密码管理
			
*操作说明:
	1.按键功能
		.按键1与按键2:菜单项的选择
		.按键3:进入菜单项
		.按键4:退出菜单项
	2.可调电阻
		.旋钮式选择对应的菜单，类似iPod
	3.蜂鸣器
		.执行对应的操作会嘀一声示意
	4.菜单项实现
		.多级菜单的访问
	5.自动熄屏
		.若无操作，过一会自动熄屏
		.按下任意按键唤醒屏幕
	6.手势识别
		.上滑与下滑:菜单项的选择
		.左滑:进入菜单项
		.右滑:退出菜单项
	7.语音识别
		.唤醒:小粤


*关键函数：
	1.app_task_menu，文件路径：main.c
		.负责对菜单项选择/进入/退出的管理
		.显示光标指示当前菜单项

	2.menu_show,文件路径：menu.c
		.显示菜单项图标
		.显示菜单项文字

*关键变量类型：
	1.menu_t，文件路径：menu.h
		.链式存储水平左/右菜单、父/子菜单

*关键宏：
	1.DEBUG_PRINTF_EN，文件路径：includes.h
		.使能则通过串口1输出调试信息

	2.LCD_SAFE，文件路径：includes.h
		.使用互斥锁保护LCD访问
*****************************************************************/
#include "includes.h"
#include "pictime.h"

TaskHandle_t app_task_init_handle = NULL;
TaskHandle_t app_task_key_handle = NULL;
TaskHandle_t app_task_menu_handle = NULL;
TaskHandle_t app_task_adc_handle = NULL;
TaskHandle_t app_task_beep_handle = NULL;
TaskHandle_t app_task_rtc_handle = NULL;
TaskHandle_t app_task_dht_handle = NULL;
TaskHandle_t app_task_max30102_handle = NULL;
TaskHandle_t app_task_usart_handle = NULL;
TaskHandle_t app_task_mpu6050_handle = NULL;
TaskHandle_t app_task_ble_handle = NULL;
TaskHandle_t app_task_kbd_handle = NULL;
TaskHandle_t app_task_pass_man_handle = NULL;
TaskHandle_t app_task_gesture_handle = NULL;

/* 软件定时器句柄 */
TimerHandle_t soft_timer_Handle = NULL;

GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
SPI_InitTypeDef SPI_InitStructure;
RTC_DateTypeDef RTC_DateStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;

volatile uint32_t g_dht_get_what = FLAG_DHT_GET_NONE;
volatile uint32_t g_rtc_get_what = FLAG_RTC_GET_NONE;
volatile uint32_t g_ble_status = FLAG_BLE_STATUS_NONE;
volatile uint32_t g_alarm_set = FLAG_ALARM_SET_NONE;
volatile uint32_t g_mpu6050_get_what=FLAG_MPU6050_GET_NONE;
volatile uint32_t g_pass_man_what = FLAG_PASS_MAN_NONE;
volatile uint32_t g_gesture_display_flag = FLAG_GESTURE_DISPLAY_OFF; // 记录手势识别模块是否显示手势图片
volatile uint32_t g_gesture_valid = 0;//手势识别模块是否有效								 // 记录手势识别模块是否连接成功
volatile uint32_t g_alarm_pic = 0;//闹钟图标
// volatile uint32_t ulCount = 0;
// volatile uint32_t step_count = 0;
volatile uint32_t g_system_no_opreation_cnt = 0;//统计系统无操作计数值
volatile uint32_t g_system_display_on = 1;//亮屏
volatile uint32_t ble_connect_get = 0;//蓝牙连接状态


// max30102任务的变量
volatile uint32_t aun_ir_buffer[500];  // IR LED sensor data
volatile int32_t n_ir_buffer_length;   // data length
volatile uint32_t aun_red_buffer[500]; // Red LED sensor data
volatile int32_t n_sp02;			   // SPO2 value
volatile int8_t ch_spo2_valid;		   // indicator to show if the SP02 calculation is valid
volatile int32_t n_heart_rate;		   // heart rate value
volatile int8_t ch_hr_valid;		   // indicator to show if the heart rate calculation is valid
volatile uint8_t uch_dummy;

/* 消息队列句柄 */
QueueHandle_t g_queue_led;
QueueHandle_t g_queue_beep;
QueueHandle_t g_queue_usart;
QueueHandle_t g_queue_kbd;

/* 事件标志组句柄 */
EventGroupHandle_t g_event_group;

/* 互斥型信号量句柄 */
SemaphoreHandle_t g_mutex_printf;
SemaphoreHandle_t g_mutex_lcd;
SemaphoreHandle_t g_sem_beep;

/* 任务*/
static void app_task_init			(void *pvParameters);
static void app_task_key			(void *pvParameters);
static void app_task_menu			(void *pvParameters);
static void app_task_adc			(void *pvParameters);
static void app_task_beep			(void *pvParameters);
static void app_task_rtc			(void *pvParameters);
static void app_task_dht			(void *pvParameters);
static void app_task_max30102		(void *pvParameters);
static void app_task_usart			(void *pvParameters);
static void app_task_mpu6050		(void* pvParameters);
// static void app_task_ble	(void* pvParameters);	
static void app_task_rfid			(void* pvParameters); 
static void app_task_rfid_add_del	(void* pvParameters); 
static void app_task_kbd			(void* pvParameters); 
static void app_task_pass_man		(void* pvParameters);
static void app_task_gesture		(void* pvParameters);

/* 软件定时器 */
static void soft_timer_callback(TimerHandle_t pxTimer);

void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_PRINTF_EN

	va_list args;
	va_start(args, format);

	/* 获取互斥信号量 */
	xSemaphoreTake(g_mutex_printf, portMAX_DELAY);

	vprintf(format, args);

	/* 释放互斥信号量 */
	xSemaphoreGive(g_mutex_printf);

	va_end(args);
#else
	(void)0;
#endif
}

/* 任务列表 */
static const task_t task_tbl[] = {
	{app_task_key, "app_task_key", 512, NULL, 5, &app_task_key_handle},
	{app_task_rtc, "app_task_rtc", 512, NULL, 5, &app_task_rtc_handle},
	{app_task_beep, "app_task_beep", 512, NULL, 5, &app_task_beep_handle},
	{app_task_adc, "app_task_adc", 512, NULL, 5, &app_task_adc_handle},
	{app_task_dht, "app_task_dht", 512, NULL, 5, &app_task_dht_handle},
	{app_task_menu, "app_task_menu", 512, &menu_main_tbl, 10, &app_task_menu_handle},
	{app_task_max30102, "app_task_max30102", 512, NULL, 5, &app_task_max30102_handle},
	{app_task_usart, "app_task_usart", 512, NULL, 5, &app_task_usart_handle},
	{app_task_mpu6050, "app_task_mpu6050", 1024, NULL, 5, &app_task_mpu6050_handle},
	{app_task_kbd, 	"app_task_kbd", 128, NULL, 5, &app_task_kbd_handle},
	{app_task_pass_man, "app_task_pass_man", 512, NULL, 5, &app_task_pass_man_handle},		
	{0, 0, 0, 0, 0, 0}};

int main(void)
{
	/* 设置系统中断优先级分组4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 系统定时器中断频率为configTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);

	/* 初始化串口1 */
	usart_init(115200);

	/* 创建app_task_init任务 */
	xTaskCreate((TaskFunction_t)app_task_init,			/* 任务入口函数 */
				(const char *)"app_task_init",			/* 任务名字 */
				(uint16_t)512,							/* 任务栈大小 */
				(void *)NULL,							/* 任务入口函数参数 */
				(UBaseType_t)5,							/* 任务的优先级 */
				(TaskHandle_t *)&app_task_init_handle); /* 任务控制块指针 */

	/* 开启任务调度 */
	vTaskStartScheduler();

	printf("none run here...\r\n");

	while (1)
		;
}

void lcd_startup_info(void)
{
	uint16_t x = 0;
	LCD_SAFE(

		/* 设置显示起始位置 */
		lcd_vs_set(0);

		/* 清屏 */
		lcd_clear(WHITE);
		lcd_fill(0, LCD_HEIGHT, LCD_WIDTH, 80, WHITE);

		/* 显示主界面 */
		lcd_draw_picture(90, 70, 60, 60, gImage_pic_red_flag);

		x = 50; lcd_show_chn(x, 170, 0, BLACK, WHITE, 32);
		x += 50; lcd_show_chn(x, 170, 1, BLACK, WHITE, 32);
		x += 50; lcd_show_chn(x, 170, 2, BLACK, WHITE, 32);
	
		x = 0;
		while (x < LCD_WIDTH) {
			/* 进度条 */
			lcd_fill(x, 230, 10, 50, BLUE);

			x += 10;

			vTaskDelay(50);
		}

	);
}

static void app_task_init(void *pvParameters)
{

	uint32_t i = 0;

	menu_ext_t m_ext;

	printf("[app_task_init] create success\r\n");

	/* lcd初始化 */
	lcd_init();

	/* beep初始化 */
	beep_init();

	/* 按键初始化 */
	key_init();

	/* adc初始化 */
	adc_init();

	/* rtc初始化 */
	rtc_init();

	/* 温湿度模块初始化 */
	dht11_init();
	
	
	ble_init(9600);


	/* MPU6050初始化 */	
	MPU_Init();

	/* 心率血氧模块初始化 */
	max30102_init();

	// 初始化MPU6050
	// MPU_Init();
	// while (mpu_dmp_init())
	// {
	// 	printf("[ERROR] MPU6050 ERROR \r\n");
	// 	delay_ms(500);
	// }

	/* eeprom初始化 */
	at24c02_init();

	/* 矩阵键盘初始化 */
	kbd_init();

	asr_init(9600);

	taskENTER_CRITICAL();

	/* 创建消息队列 */
	g_queue_usart = xQueueCreate(5, 128);
	g_queue_beep = xQueueCreate(QUEUE_BEEP_LEN, sizeof(beep_t));
	g_queue_kbd 	= xQueueCreate(5, sizeof(uint8_t));

	/* 创建互斥型信号量 */
	g_mutex_printf = xSemaphoreCreateMutex();
	g_mutex_lcd = xSemaphoreCreateMutex();
	g_sem_beep = xSemaphoreCreateBinary();

	/* 创建事件标志组 */
	g_event_group = xEventGroupCreate();

	/* 创建用到的任务 */
	i = 0;
	while (task_tbl[i].pxTaskCode)
	{
		xTaskCreate(task_tbl[i].pxTaskCode,		/* 任务入口函数 */
					task_tbl[i].pcName,			/* 任务名字 */
					task_tbl[i].usStackDepth,	/* 任务栈大小 */
					task_tbl[i].pvParameters,	/* 任务入口函数参数 */
					task_tbl[i].uxPriority,		/* 任务的优先级 */
					task_tbl[i].pxCreatedTask); /* 任务控制块指针 */
		i++;
	}

	taskEXIT_CRITICAL();

	/* 创建周期软件定时器 */
	soft_timer_Handle = xTimerCreate((const char *)"AutoReloadTimer",
									 (TickType_t)1000,	  /* 定时器周期 1000(tick) */
									 (UBaseType_t)pdTRUE, /* 周期模式 */
									 (void *)1,			  /* 为每个计时器分配一个索引的唯一ID */
									 (TimerCallbackFunction_t)soft_timer_callback);
	/* 开启周期软件定时器 */
	xTimerStart(soft_timer_Handle, 0);

	
	vTaskSuspend(app_task_kbd_handle);
	vTaskSuspend(app_task_max30102_handle);

	lcd_startup_info();

	/* PAJ7620U2手势识别传感器连接到开发板，并初始化成功后才创建手势识别任务 */
	if (paj7620u2_init())
	{
		lcd_clear(WHITE);
		lcd_fill(0, LCD_HEIGHT, LCD_WIDTH, 80, WHITE);

		dgb_printf_safe("[app_task_init] paj7620u2 init success\r\n");

		/* 记录手势识别模块有效性，1-工作有效 0-工作无效 */
		g_gesture_valid = 1;

		/* 显示手势识别模块图标 */
		// OLED_DrawBMP(0, 0, 64, 8, (uint8_t *)pic_gesture_icon);

		/* 显示"连接成功" */
		// OLED_ShowCHinese(64, 4, 22);
		// OLED_ShowCHinese(90, 4, 23);
		// OLED_ShowCHinese(64, 6, 30);
		// OLED_ShowCHinese(90, 6, 31);
		/* 设置要显示的图片 */
 							
		lcd_draw_picture(96,80,48,48,gImage_hand_48x48);
		/*手势连接成功 */
		lcd_show_chn(30,160,38,BLACK,WHITE,32);
		lcd_show_chn(62,160,39,BLACK,WHITE,32);
		lcd_show_chn(94,160,40,BLACK,WHITE,32);
		lcd_show_chn(126,160,41,BLACK,WHITE,32);
		lcd_show_chn(158,160,42,BLACK,WHITE,32);
		lcd_show_chn(190,160,43,BLACK,WHITE,32);
		

		/* 持续2秒 */
		vTaskDelay(2000);

		/* 创建app_task_gesture任务 */
		xTaskCreate((TaskFunction_t)app_task_gesture,		   /* 任务入口函数 */
					(const char *)"app_task_gesture",		   /* 任务名字 */
					(uint16_t)512,							   /* 任务栈大小 */
					(void *)NULL,							   /* 任务入口函数参数 */
					(UBaseType_t)5,							   /* 任务的优先级 */
					(TaskHandle_t *)&app_task_gesture_handle); /* 任务控制块指针 */
	}

	/* 显示主菜单 */
	m_ext.menu = menu_main_1;

	menu_show(&m_ext);

	vTaskDelete(NULL);
}

/**
 * @brief 手势识别任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 通过获取手势值，用作菜单的控制
 */
static void app_task_gesture(void *pvParameters)
{
	EventBits_t EventValue;
	BaseType_t xReturn;
	// oled_t oled;
	uint16_t gesture;
	uint32_t i = 0;

	// /* 手势图片列表，NULL表示不显示该手势 */
	// const uint8_t *pic_gesture_icon_tbl[] = {
	// 	pic_up_icon,
	// 	pic_down_icon,
	// 	pic_left_icon,
	// 	pic_right_icon,
	// 	NULL,
	// 	NULL,
	// 	pic_clockwise_icon,
	// 	pic_anticlockwise_icon,
	// 	NULL};

	// /* 提前设置图片显示的位置、大小 */
	// oled.x = 30;
	// oled.y = 0;
	// oled.pic_width = 64;
	// oled.pic_height = 8;

	dgb_printf_safe("[app_task_gesture] create success\r\n");
	// dgb_printf_safe("[app_task_gesture] pic_gesture_icon_tbl array size is %d \r\n", ARRAY_SIZE(pic_gesture_icon_tbl));

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_GESTURE,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		if (!(EventValue & EVENT_GROUP_GESTURE))
			continue;

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		/* 获取手势 */
		gesture = gesture_get();

		// /* 提取手势图片 */
		// for (i = 0; i < ARRAY_SIZE(pic_gesture_icon_tbl); i++)
		// {
		// 	if (gesture & (1 << i))
		// 		break;
		// }

		// /* 显示手势图片 */
		// if ((g_gesture_display_flag == FLAG_GESTURE_DISPLAY_ON) && pic_gesture_icon_tbl[i])
		// {
		// 	dgb_printf_safe("[app_task_gesture] gesture is %d \r\n", i);

		// 	oled.ctrl = OLED_CTRL_SHOW_PICTURE;

		// 	oled.pic = pic_gesture_icon_tbl[i];

		// 	xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
		// 						 &oled,		   /* 发送的消息内容 */
		// 						 100);		   /* 等待时间 100 Tick */
		// 	if (xReturn != pdPASS)
		// 		dgb_printf_safe("[app_task_gesture] xQueueSend oled picture error code is %d\r\n", xReturn);
		// }

		/* 根据手势，执行对应的按键功能 */
		switch (gesture)
		{
		case GES_UP:
		{
			dgb_printf_safe("[app_task_gesture] GES_UP trigger KEY_UP\r\n");

			if (g_gesture_display_flag == FLAG_GESTURE_DISPLAY_OFF)
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_UP);
		}
		break;

		case GES_DOWN:
		{
			dgb_printf_safe("[app_task_gesture] GES_DOWN trigger KEY_DOWN\r\n");

			if (g_gesture_display_flag == FLAG_GESTURE_DISPLAY_OFF)
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_DOWN);
		}
		break;

		case GES_RIGHT:
		{
			dgb_printf_safe("[app_task_gesture] GES_RIGHT trigger KEY_ENTER\r\n");

			if (g_gesture_display_flag == FLAG_GESTURE_DISPLAY_OFF)
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
		}
		break;

		case GES_CLOCKWISE:
		{
			dgb_printf_safe("[app_task_gesture] GES_CLOCKWISE trigger KEY_ENTER\r\n");

			if (g_gesture_display_flag == FLAG_GESTURE_DISPLAY_OFF)
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
		}
		break;

		case GES_LEFT:
		{
			dgb_printf_safe("[app_task_gesture] GES_LEFT trigger KEY_BACK\r\n");

			if (g_gesture_display_flag == FLAG_GESTURE_DISPLAY_OFF)
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);
		}
		break;

		case GES_COUNT_CLOCKWISE:
		{
			dgb_printf_safe("[app_task_gesture] GES_COUNT_CLOCKWISE trigger KEY_BACK\r\n");

			if (g_gesture_display_flag == FLAG_GESTURE_DISPLAY_ON)
			{
				/* 显示逆时针图标后，自动退出 */
				vTaskDelay(1000);
				g_gesture_display_flag = FLAG_GESTURE_DISPLAY_OFF;
			}

			/* 当在显示手势时，只有逆时针触发程序退出 */
			xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);
		}
		break;

		default:
			break;
		}
	}
}

/**
 * @brief 密码管理
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 密码验证、密码修改
 */
static void app_task_pass_man(void *pvParameters)
{
#define PASS1_ADDR_IN_EEPROM	0
#define PASS_LEN				6

#define PASS_MODIFY_STA_NONE	 0
#define PASS_MODIFY_STA_OLD_PASS 1
#define PASS_MODIFY_STA_NEW_PASS 2
	BaseType_t xReturn;
	int32_t rt;

	char 	 key_val=0;
	char 	 key_buf[PASS_LEN]={0};
	uint32_t key_cnt=0;

const 
	char 	 pass_auth_default[PASS_LEN]={'8','8','8','8','8','8'};
	char 	 pass_auth_eeprom[PASS_LEN]={0};

	char 	 pass_old[PASS_LEN]={0};
	char 	 pass_new[PASS_LEN]={0};
	
	uint8_t  chn_tbl[16]={0};
	uint32_t i=0;
	
	beep_t	 beep;

	const uint8_t x_start=140;	
	const uint8_t y_start=160;		
	uint8_t	 x=x_start,y=y_start;
	
	uint32_t pass_modify_sta = PASS_MODIFY_STA_OLD_PASS;

	dgb_printf_safe("[app_task_pass_man] create success\r\n");

	for (;;)
	{
		/* 等待矩阵键盘消息 */
		xReturn = xQueueReceive(g_queue_kbd,&key_val,portMAX_DELAY);

		if(xReturn!=pdPASS)
			continue;

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();			
		
		/* 复位密码操作内容 */
		if(key_val == 'C')
		{
			x=x_start;
			y=y_start;
			key_cnt=0;

			memset(key_buf, 0,sizeof key_buf);
			memset(pass_old,0,sizeof pass_old);	
			memset(pass_new,0,sizeof pass_new);	

			pass_modify_sta = PASS_MODIFY_STA_OLD_PASS;

			continue;
		}

		/* 密码认证测试 */
		if(g_pass_man_what == FLAG_PASS_MAN_AUTH)
		{
			/* 没有密码，无效删除操作 */
			if(key_val == '*' && key_cnt==0)
				continue;

			/* 删除上一格密码 */
			if(key_val == '*' && key_cnt)
			{
				key_cnt--;
				key_buf[key_cnt]=0;

				x-=16;
				lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
				continue;				
			}

			if(key_cnt<6)
			{
				/* 显示数字 */
				key_buf[key_cnt] = (uint8_t)key_val;
				lcd_show_char(x,y,key_buf[key_cnt],BLACK,WHITE,32,0);
				vTaskDelay(100);

				/* 显示* */
				lcd_show_char(x,y,'*',BLACK,WHITE,32,0);
				if(key_buf[0]==0) lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
				x+=16;
				key_cnt++;
			}
			else
			{
				if(key_val == '#')
				{
					dgb_printf_safe("[app_task_pass_man] auth key buf is %6s\r\n", key_buf);		
					
					/* 读取eeprom存储的密码 */
					at24c02_read(PASS1_ADDR_IN_EEPROM,(uint8_t *)pass_auth_eeprom,PASS_LEN);

					/* 匹配eeprom存储的密码 */
					rt = memcmp(pass_auth_eeprom,key_buf,PASS_LEN);
					if(rt)
					{
						dgb_printf_safe("[app_task_pass_man] password auth by eeprom fail\r\n");
						
						/* 匹配默认密码 */
						rt=memcmp(pass_auth_default,key_buf,PASS_LEN);
						if(rt)
							dgb_printf_safe("[app_task_pass_man] password auth by defalut fail too\r\n");
					}
					
					/* 密码匹配成功 */
					if(rt==0)
					{
						dgb_printf_safe("[app_task_pass_man] password auth success\r\n");
						
						lcd_clear(WHITE);
						lcd_fill(0,y,LCD_WIDTH,32,WHITE);
						/* 设置要显示的图片-表情（成功） */
						LCD_SAFE
						(					
							lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);
						);
						
						/* 嘀一声示意:第一次 */
						beep.sta = 1;
						beep.duration = 100;

						xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
											&beep,		   /* 发送的消息内容 */
											100);		   /* 等待时间 100 Tick */

						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

						/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);							

						vTaskDelay(100);
					}
					else
					{
						lcd_clear(WHITE);
						lcd_fill(0,y,LCD_WIDTH,32,WHITE);
						
						/* 设置要显示的图片-表情（失败） */
						LCD_SAFE
						(					
							lcd_draw_picture(70, 90, 100, 100, gImage_pic_fail_100x100);
						);
						/* 长鸣1秒示意 */
						beep.sta = 1;
						beep.duration = 100;

						xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
											&beep,		   /* 发送的消息内容 */
											100);		   /* 等待时间 100 Tick */				
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

						/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);	

					}

					/* 延时一会 */
					vTaskDelay(1000);

					/* 恢复初值 */
					x=x_start;
					key_cnt=0;
					memset(key_buf,0,sizeof key_buf);	

					// lcd_sprintf_hzstr32s((LCD_WIDTH-(strlen("密码认证失败")*16))/2,y,(uint8_t*)"密码认证失败",BLACK,WHITE);
					// vTaskDelay(1000);
					lcd_clear(WHITE);
					lcd_fill(0,y,LCD_WIDTH,32,WHITE);
					
					/* 设置要显示的图片 */
					LCD_SAFE
					(					
						lcd_draw_picture(96,80,48,48,gImage_auto_48x48);
						/* 显示汉字-旧密码 */
						lcd_show_chn(30,160,34,BLACK,WHITE,32);
						lcd_show_chn(62,160,35,BLACK,WHITE,32);
						lcd_show_chn(94,160,36,BLACK,WHITE,32);
						/* 清空密码显示区 */
						// lcd_show_string(160,160,"------",BLACK,WHITE,32,0);
					);
				}
			}
		}

		/* 密码修改测试 */
		if(g_pass_man_what == FLAG_PASS_MAN_MODIFY)
		{
			switch(pass_modify_sta)
			{
				case PASS_MODIFY_STA_OLD_PASS:
				{
					/* 没有密码，无效删除操作 */
					if(key_val == '*' && key_cnt==0)
						continue;

					/* 删除上一格密码 */
					if(key_val == '*' && key_cnt)
					{
						key_cnt--;
						key_buf[key_cnt]=0;

						// x-=8;
						// oled.ctrl = OLED_CTRL_SHOW_STRING;
						// oled.x = x;
						// oled.y = 2;
						// oled.font_size = 16;

						// oled.str ="-";
						// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
						// 					&oled,		   /* 发送的消息内容 */
						// 					100);		   /* 等待时间 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						x-=16;
						lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
						continue;				
					}

					if(key_cnt<6)
					{
						/* 显示数字 */
						// key_buf[key_cnt] = key_val;

						// oled.ctrl = OLED_CTRL_SHOW_STRING;
						// oled.x = x;
						// oled.y = 2;
						// oled.font_size = 16;

						// oled.str =(uint8_t *) &key_buf[key_cnt];
						// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
						// 					&oled,		   /* 发送的消息内容 */
						// 					100);		   /* 等待时间 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						key_buf[key_cnt] = (uint8_t)key_val;
						lcd_show_char(x,y,key_buf[key_cnt],BLACK,WHITE,32,0);

						vTaskDelay(100);

						// oled.str = "*";
						// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
						// 					&oled,		   /* 发送的消息内容 */
						// 					100);		   /* 等待时间 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

						// x+=8;
						// key_cnt++;
						/* 显示* */
						lcd_show_char(x,y,'*',BLACK,WHITE,32,0);
						if(key_buf[0]==0) lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
						x+=16;
						key_cnt++;
					} 
					else
					{
						if(key_val == '#')
						{
							pass_modify_sta=PASS_MODIFY_STA_NEW_PASS;
							key_cnt=0;
							memcpy(pass_old,key_buf,PASS_LEN);
							memset(key_buf,0,sizeof key_buf);
							x=x_start;
							y=y_start+40;
						}
					}	
				}break;

				case PASS_MODIFY_STA_NEW_PASS:
				{

					/* 没有密码，无效删除操作 */
					if(key_val == '*' && key_cnt==0)
						continue;

					/* 删除上一格密码 */
					if(key_val == '*' && key_cnt)
					{
						key_cnt--;
						key_buf[key_cnt]=0;

						// x-=8;
						// oled.ctrl = OLED_CTRL_SHOW_STRING;
						// oled.x = x;
						// oled.y = 6;
						// oled.font_size = 16;

						// oled.str ="-";
						// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
						// 					&oled,		   /* 发送的消息内容 */
						// 					100);		   /* 等待时间 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						x-=16;
						lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
						continue;				
					}


					if(key_cnt<6)
					{
						/* 显示'*' */
						// key_buf[key_cnt] = key_val;

						// oled.ctrl = OLED_CTRL_SHOW_STRING;
						// oled.x = x;
						// oled.y = 6;
						// oled.font_size = 16;

						// oled.str =(uint8_t *) &key_buf[key_cnt];
						// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
						// 					&oled,		   /* 发送的消息内容 */
						// 					100);		   /* 等待时间 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						key_buf[key_cnt] = (uint8_t)key_val;
						lcd_show_char(x,y,key_buf[key_cnt],BLACK,WHITE,32,0);

						vTaskDelay(100);

						// oled.str = "*";
						// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
						// 					&oled,		   /* 发送的消息内容 */
						// 					100);		   /* 等待时间 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

						// x+=8;
						// key_cnt++;
						lcd_show_char(x,y,'*',BLACK,WHITE,32,0);
						if(key_buf[0]==0) lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
						x+=16;
						key_cnt++;
					}
					else
					{
						if(key_val == '#')
						{
							/* 保存新密码 */
							memcpy(pass_new,key_buf,PASS_LEN);

							/* 读取eeprom存储的密码 */
							at24c02_read(PASS1_ADDR_IN_EEPROM,(uint8_t *)pass_auth_eeprom,PASS_LEN);
							dgb_printf_safe("[app_task_pass_man] password eeprom is %6s\r\n",pass_auth_eeprom);
							/* 匹配eeprom存储的密码 */
							rt = memcmp(pass_auth_eeprom,pass_old,PASS_LEN);
							if(rt)
							{
								dgb_printf_safe("[app_task_pass_man] password old is %6s\r\n",pass_old);
								dgb_printf_safe("[app_task_pass_man] password auth by eeprom fail\r\n");
								
								/* 匹配默认密码 */
								rt=memcmp(pass_auth_default,pass_old,PASS_LEN);
								dgb_printf_safe("[app_task_pass_man] password default is %6s\r\n",pass_auth_default);
								if(rt)
								{
									dgb_printf_safe("[app_task_pass_man] password auth by defalut fail too\r\n");
								}
									
							}
							
							/* 密码匹配成功 */
							if(rt==0)
							{
								
								dgb_printf_safe("[app_task_pass_man] password auth success\r\n");
								
								/* 进入临界区，可靠地向eeprom写入新密码 */
								taskENTER_CRITICAL();
								rt = at24c02_write(PASS1_ADDR_IN_EEPROM,(uint8_t *)pass_new,PASS_LEN);
								taskEXIT_CRITICAL();

								if(rt == 0)
									dgb_printf_safe("[app_task_pass_man] password new is %6s,write into eeprom success\r\n",pass_new);
								else
									dgb_printf_safe("[app_task_pass_man] password new is %6s,write into eeprom fail,error code is %d\r\n",pass_new,rt);

								/* 设置要显示的图片-表情（成功） */
								// oled.x=64;
								// oled.y=0;
								// oled.pic_width=64;
								// oled.pic_height=8;
								// oled.ctrl=OLED_CTRL_SHOW_PICTURE;
								// oled.pic=pic_face_success_icon;

								// xReturn = xQueueSend( 	g_queue_oled,	/* 消息队列的句柄 */
								// 						&oled,			/* 发送的消息内容 */
								// 						100);			/* 等待时间 100 Tick */
								// if(xReturn != pdPASS)
								// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n",xReturn);								
								lcd_clear(WHITE);
								lcd_fill(0,y,LCD_WIDTH,32,WHITE);
								/* 设置要显示的图片-表情（成功） */
								LCD_SAFE
								(					
									lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);
								);
								
								/* 嘀一声示意:第一次 */
								beep.sta = 1;
								beep.duration = 100;

								xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
													&beep,		   /* 发送的消息内容 */
													100);		   /* 等待时间 100 Tick */				
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

								/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
								xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);										

								vTaskDelay(100);

								}
							else
							{
								/* 设置要显示的图片-表情（失败） */
								// oled.x=64;
								// oled.y=0;
								// oled.pic_width=64;
								// oled.pic_height=8;
								// oled.ctrl=OLED_CTRL_SHOW_PICTURE;
								// oled.pic=pic_face_fail_icon;

								// xReturn = xQueueSend( 	g_queue_oled,	/* 消息队列的句柄 */
								// 						&oled,			/* 发送的消息内容 */
								// 						100);			/* 等待时间 100 Tick */
								// if(xReturn != pdPASS)
								lcd_clear(WHITE);
								lcd_fill(0,y,LCD_WIDTH,32,WHITE);
								
								/* 设置要显示的图片-表情（失败） */
								LCD_SAFE
								(					
									lcd_draw_picture(70, 90, 100, 100, gImage_pic_fail_100x100);
								);

								dgb_printf_safe("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n",xReturn);


								/* 长鸣1秒示意 */
								beep.sta = 1;
								beep.duration = 100;

								xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
													&beep,		   /* 发送的消息内容 */
													100);		   /* 等待时间 100 Tick */				
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

								/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
								xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);											


							}

							/* 延时一会 */
							vTaskDelay(1000);	


							/* 清空右半部分区域 */
							// oled.ctrl = OLED_CTRL_CLEAR_REGION;
							// oled.x = 64;
							// oled.y = 0;
							// oled.clr_region_width=64;
							// oled.clr_region_height=8;
							// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							// 					&oled,		   /* 发送的消息内容 */
							// 					100);		   /* 等待时间 100 Tick */
							// if (xReturn != pdPASS)
							// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
							lcd_clear(WHITE);
							lcd_fill(0,y,LCD_WIDTH,32,WHITE);
							

							// /* 显示汉字-旧密码 */
							// oled.ctrl = OLED_CTRL_SHOW_CHINESE;
							// oled.y = 0;
							// chn_tbl[0]=39;chn_tbl[1]=40;chn_tbl[2]=41;
						
							// for(i=0; i<3; i++)
							// {
							// 	oled.x = 66+i*16;
							// 	oled.chinese = chn_tbl[i];

							// 	xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							// 						&oled,		   /* 发送的消息内容 */
							// 						100);		   /* 等待时间 100 Tick */
							// 	if (xReturn != pdPASS)
							// 		dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);

							// }


							// /* 清空旧密码显示区 */
							// oled.ctrl = OLED_CTRL_SHOW_STRING;
							// oled.x = 66;
							// oled.y = 2;
							// oled.str = "------";
							// oled.font_size = 16;

							// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							// 					&oled,		   /* 发送的消息内容 */
							// 					1000);		   /* 等待时间 100 Tick */
							// if (xReturn != pdPASS)
							// 	dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);

							// /* 清空新密码显示区 */
							// oled.ctrl = OLED_CTRL_SHOW_STRING;
							// oled.x = 66;			
							// oled.y = 6;
							// oled.str = "------";
							// oled.font_size = 16;
							// xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							// 					&oled,		   /* 发送的消息内容 */
							// 					1000);		   /* 等待时间 100 Tick */
							// if (xReturn != pdPASS)
							// 	dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);


							// /* 显示汉字-新密码 */
							// oled.ctrl = OLED_CTRL_SHOW_CHINESE;
							// oled.x = 66;
							// oled.y = 4;
							// chn_tbl[0]=38;chn_tbl[1]=40;chn_tbl[2]=41;
						
							// for(i=0; i<3; i++)
							// {
							// 	oled.x = 66+i*16;
							// 	oled.chinese = chn_tbl[i];

							// 	xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							// 						&oled,		   /* 发送的消息内容 */
							// 						100);		   /* 等待时间 100 Tick */
							// 	if (xReturn != pdPASS)
							// 		dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);

							// }	

							/* 设置要显示的图片 */
							LCD_SAFE
							(					
								lcd_draw_picture(96,60,48,48,gImage_auto_48x48);
								/* 显示汉字-旧密码 */
								lcd_show_chn(30,160,34,BLACK,WHITE,32);
								lcd_show_chn(62,160,35,BLACK,WHITE,32);
								lcd_show_chn(94,160,36,BLACK,WHITE,32);
								// /* 清空密码显示区 */
								// lcd_show_string(160,160,"------",BLACK,WHITE,32,0);

								/* 显示汉字-新密码 */
								lcd_show_chn(30,200,37,BLACK,WHITE,32);
								lcd_show_chn(62,200,35,BLACK,WHITE,32);
								lcd_show_chn(94,200,36,BLACK,WHITE,32);

							);											

							/* 恢复初值 */
							pass_modify_sta=PASS_MODIFY_STA_OLD_PASS;
							key_cnt=0;
							memset(key_buf, 0,sizeof key_buf);
							memset(pass_old,0,sizeof pass_old);	
							memset(pass_new,0,sizeof pass_new);	

							x=x_start;
							y=y_start;
						}

					}

				}break;

				default:break;
			}
		}
	}
}


/**
 * @brief 矩阵键盘任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 */
void app_task_kbd(void* pvParameters)
{
	char key_val='N';
	beep_t beep;
	BaseType_t xReturn;
	dgb_printf_safe("[app_task_kbd] create success\r\n");
	
	while(1)
	{
		/* 读取矩阵键盘按键值 */
		key_val=kbd_read();

		if(key_val != 'N')
		{
			dgb_printf_safe("[app_task_kbd] kbd press %c \r\n",key_val);

			xReturn = xQueueSend(g_queue_kbd, 	/* 消息队列的句柄 */
								&key_val,		/* 发送的消息内容 */
								100);		   	/* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_kbd] xQueueSend kbd error code is %d\r\n", xReturn);	

			/* 嘀一声示意 */
			beep.sta = 1;
			beep.duration = 20;

			xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
								&beep,		   /* 发送的消息内容 */
								100);		   /* 等待时间 100 Tick */				
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_kbd] xQueueSend beep error code is %d\r\n", xReturn);

			/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_system_reset] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);					

		}	
	}
}

/**
 * @brief usart任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 通过串口接收数据,来实现对时间日期的修改
 */
static void app_task_usart(void *pvParameters)
{

	char buf[128] = {0};
	char *p = NULL;
	uint32_t year = 0;
	uint32_t month = 0;
	uint32_t day = 0;
	uint32_t week_day = 0;
	uint32_t hours = 0;
	uint32_t minutes = 0;
	uint32_t seconds = 0;

	uint8_t dht11_data[5]={0};
	
	uint32_t value;

	for (;;)
	{

		// 串口接收数据
		xQueueReceive(g_queue_usart, buf, portMAX_DELAY);

		printf("%s\r\n", buf);

		// 设置时间
		if (g_rtc_get_what == FLAG_RTC_GET_TIME || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{
			// “TIME SET-10-20-30#”
			if (strstr((const char *)buf, "TIME SET"))
			{

				p = strtok((char *)buf, "-");
				// 小时10
				p = strtok(NULL, "-");
				hours = atoi(p);
				// 分钟20
				p = strtok(NULL, "-");
				minutes = atoi(p);
				// 秒30
				p = strtok(NULL, "-");
				seconds = atoi(p);

				RTC_TimeStructure.RTC_Hours = hours;
				RTC_TimeStructure.RTC_Minutes = minutes;
				RTC_TimeStructure.RTC_Seconds = seconds;
				RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

				printf("set time ok!\r\n");

				if (g_rtc_get_what == FLAG_RTC_GET_TIME)
					lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);
			}
		}

		// 设置日期
		if (g_rtc_get_what == FLAG_RTC_GET_DATE || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{

			// “DATE SET-2023-5-25-4#”
			if (strstr((const char *)buf, "DATE SET"))
			{
				p = strtok((char *)buf, "-");
				p = strtok(NULL, "-"); // 2023
				year = atoi(p);
				// 提取月份
				p = strtok(NULL, "-");
				month = atoi(p);

				// 提取天数
				p = strtok(NULL, "-");
				day = atoi(p);
				// printf("p=%s",p);

				// 提取星期几
				p = strtok(NULL, "-");
				week_day = atoi(p);
				// printf("p=%s",p);

				RTC_DateStructure.RTC_Year = year - 2000;
				RTC_DateStructure.RTC_Month = month;
				RTC_DateStructure.RTC_Date = day;
				RTC_DateStructure.RTC_WeekDay = week_day;

				RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);

				printf("set date ok!\r\n");

				if (g_rtc_get_what == FLAG_RTC_GET_DATE)
					lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);
			}
		}

		// 设置闹钟
		if (g_alarm_set == FLAG_ALARM_SET_START || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{
			// “ALARM SET-10-20-30#”
			if (strstr((const char *)buf, "ALARM SET"))
			{

				// 关闭闹钟
				RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

				p = strtok((char *)buf, "-");
				// 小时10
				p = strtok(NULL, "-");
				hours = atoi(p);
				// 分钟20
				p = strtok(NULL, "-");
				minutes = atoi(p);

				// 秒30
				p = strtok(NULL, "-");
				seconds = atoi(p);

				if (hours > 12)
				{
					RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;
				}
				else
				{
					RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
				}

				RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = hours;
				RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = minutes;
				RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = seconds;

				//				RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
				//				RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
				RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay; // 屏蔽日期和星期，就是闹钟每天都生效

				/* 配置RTC的A闹钟，注：RTC的闹钟有两个，分别为闹钟A与闹钟B */
				RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

				/* 让RTC的闹钟A工作*/
				RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

				printf("alarm set ok!\r\n");

				if (g_alarm_set == FLAG_ALARM_SET_START)
					lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);

				// 点亮闹钟图标
				g_alarm_pic = 1;
			}
			if (strstr((const char *)buf, "ALARM OFF"))
			{
				printf("闹钟关闭!\r\n");
				// 关闭闹钟
				RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

				if (g_alarm_set == FLAG_ALARM_SET_START)
					lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);

				// 熄灭闹钟图标
				g_alarm_pic = 0;
			}
		}

		
			if(strstr((const char *)buf,"LED ON"))
			{
				PFout(9)=0;
				asr_send_str("1#");
			}
			
			if(strstr((const char *)buf,"LED OFF"))
			{	
				PFout(9)=1;				
				asr_send_str("1#");
			}

			if(strstr((const char *)buf,"TEMP"))
			{
				dht11_read_data(dht11_data);
				sprintf(buf,"%d#",dht11_data[2]);
				asr_send_str(buf);
				printf("%s\r\n",buf);
			}			
			
			if(strstr((const char *)buf,"HUMI"))
			{
				dht11_read_data(dht11_data);
				sprintf(buf,"%d#",dht11_data[0]);
				
				asr_send_str(buf);	
				printf("%s\r\n",buf);
			}	

			if(strstr((const char *)buf,"TIME"))
			{
				RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);	

				value=(RTC_TimeStructure.RTC_Hours<<16) \
					|(RTC_TimeStructure.RTC_Minutes<<8) \
					|(RTC_TimeStructure.RTC_Seconds);
				
				sprintf(buf,"%d#",value);
				
				asr_send_str(buf);
				
				printf("%02d:%02d:%02d\r\n", RTC_TimeStructure.RTC_Hours,
						RTC_TimeStructure.RTC_Minutes,
						RTC_TimeStructure.RTC_Seconds);				
			}
			
			if(strstr((const char *)buf,"DATE"))
			{
				RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);	
				/*  bit[0 - 3]: RTC_WeekDay 1~7
					bit[4 - 8]: RTC_Date 1~31
					bit[9 - 12]: RTC_Month 1~12
					bit[13 - 23]: RTC_Year 0~99
					bit[24 - 31]: reserve
				*/
				value=(RTC_DateStructure.RTC_Year<<13) \
					|(RTC_DateStructure.RTC_Month<<9) \
					|(RTC_DateStructure.RTC_Date<<4)\
					|(RTC_DateStructure.RTC_WeekDay);
				
				sprintf(buf,"%d#",value);
				
				asr_send_str(buf);
			
			}			
			
			g_usart2_rx_end=0;
			g_usart2_rx_cnt=0;
			
			memset((void *)buf,0,sizeof buf);
		
	}
}

/**
 * @brief mpu6050任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 通过dmp_get_pedometer_step_count获取步数,通过mpu_dmp_get_data来获取欧拉角实现抬手亮屏
 */
static void app_task_mpu6050(void* pvParameters)
{
	char buf[16] = {0};
	unsigned long  step_count_last=0;
	unsigned long  step_count=0;
	uint32_t sedentary_event=0;
	uint32_t t=0;
	float pitch, roll, yaw; /* 欧拉角 */

	dgb_printf_safe("[app_task_mpu6050] create success and suspend self \r\n");

	//vTaskSuspend(NULL);

	dgb_printf_safe("mpu6050持续工作中......r\n");

	while(mpu_dmp_init())
	{
		delay_ms(100);
	}
	
	/* 设置步数初值为0*/
	while(dmp_set_pedometer_step_count(0))
	{
		delay_ms(100);
	}

	for (;;)
	{

		/* 获取步数 */
		dmp_get_pedometer_step_count(&step_count);
		
		//dgb_printf_safe("mpu6050== %dr\n",step_count);
		//dgb_printf_safe("[INFO] 当前步数:%ld 以前步数:%ld\r\n",step_count,step_count_last);
		
		step_count_last=step_count;
		
		sprintf((char *)buf, ":%d",step_count);
		// dgb_printf_safe("%d\r\n",g_mpu6050_get_what);
		if (g_mpu6050_get_what == FLAG_MPU6050_GET_STEP)
		{
		
			/* 当前步数 */
			lcd_show_chn(30,160,22,BLACK,WHITE,32);
			lcd_show_chn(62,160,23,BLACK,WHITE,32);
			
			lcd_show_chn(94,160,24,BLACK,WHITE,32);
			lcd_show_chn(126,160,25,BLACK,WHITE,32);
				
			lcd_show_string(160,160,buf,BLACK,WHITE,32,0);
			
			delay_ms(200);			
			
		}
				

		if(g_mpu6050_get_what == FLAG_MPU6050_GET_STOP)
		{
			t++;
		
			/* 若指定秒数内没有行走，触发久坐事件 */
			if(t>=100)
			{
				/* 检查步数的变化 */
				if((step_count - step_count_last) < 5)
				{
					/* 步数变化不大，则设置久坐事件标志位为1 */
					sedentary_event=1;
				}
				
				dgb_printf_safe("[INFO] 当前步数:%ld 以前步数:%ld\r\n",step_count,step_count_last);
				
				step_count_last=step_count;
				
				t=0;
			}
			
			if(sedentary_event)
			{
				sedentary_event=0;
				
				dgb_printf_safe("[INFO] 坐立过久，请站起来走走...\r\n");
				
				/* 显示久坐提醒 */
				lcd_show_chn(30,160,26,BLACK,WHITE,32);
				lcd_show_chn(62,160,27,BLACK,WHITE,32);
				lcd_show_chn(94,160,28,BLACK,WHITE,32);
				lcd_show_chn(126,160,29,BLACK,WHITE,32);
				lcd_show_chn(158,160,30,BLACK,WHITE,32);
				lcd_show_chn(190,160,31,BLACK,WHITE,32);
				// lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);
				// lcd_draw_picture(90,80,48,48,gImage_pulse_48x48);

				PFout(8)=1;PFout(9)=0;
				delay_ms(100);
				PFout(8)=0;PFout(9)=1;	
			}
		
		}
		memset(buf, 0, sizeof buf);
		
		/* 抬手亮屏 */
		if(0 == mpu_dmp_get_data(&pitch, &roll, &yaw))
		{
			/* 延时一会 */
			delay_ms(200);
			
			if(roll > 15 && !g_system_display_on)
			{
				/* 进入临界区 */
				taskENTER_CRITICAL();
				
				g_system_display_on=1;

				/* 清零统计系统无操作计数值 */
				g_system_no_opreation_cnt=0;

				/* 退出临界区 */
				taskEXIT_CRITICAL();

				dgb_printf_safe("抬手亮屏\r\n");

				/* 点亮OLED屏 */
				lcd_display_on(1);				
			}
		}
				
	}
}


/**
 * @brief max30102任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 通过可调电阻获取的电压值，用作转盘菜单的控制
 */
static void app_task_max30102(void* pvParameters)
{
	char buff[32] = {0};
	char bufff[32] = {0};
	uint32_t un_min, un_max, un_prev_data;  
	int32_t i;
	int32_t n_brightness;
	float f_temp;
	
	uint8_t temp[6];
	
	un_min=0x3FFFF;
	un_max=0;
	
	n_ir_buffer_length=500; //buffer length of 100 stores 5 seconds of samples running at 100sps
	//read the first 500 samples, and determine the signal range
	
	//vTaskSuspend(NULL);
    for(i=0;i<n_ir_buffer_length;i++)
    {
        while(MAX30102_INT==1);   //wait until the interrupt pin asserts
        
		max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
		aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
            
        if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];    //update signal min
        if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];    //update signal max
    }
	un_prev_data=aun_red_buffer[i];
	//calculate heart rate and SpO2 after first 500 samples (first 5 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
	
	//BaseType_t xReturn = pdFALSE;

	dgb_printf_safe("[app_task_max30] create success and suspend self\r\n");

	vTaskSuspend(NULL);

	dgb_printf_safe("[app_task_max30] stay\r\n");
	
	for(;;)
	{
		i=0;
        un_min=0x3FFFF;
        un_max=0;
		n_ir_buffer_length=500;
		
		/* dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
		
		   将前100组样本转储到存储器中，并将最后400组样本移到顶部
		*/
		
        for(i=100;i<500;i++)
        {
            aun_red_buffer[i-100]=aun_red_buffer[i];
            aun_ir_buffer[i-100]=aun_ir_buffer[i];
            
            /* update the signal min and max 
			   更新信号最小值和最大值
			*/
			
            if(un_min>aun_red_buffer[i])
				un_min=aun_red_buffer[i];
			
            if(un_max<aun_red_buffer[i])
				un_max=aun_red_buffer[i];
        }
		
		/* take 100 sets of samples before calculating the heart rate 
		
		   在计算心率之前采集100组样本
		*/
		
        for(i=400;i<500;i++)
        {
            un_prev_data=aun_red_buffer[i-1];
			
            while(MAX30102_INT==1);
			
            max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
			
			/* 组合值以获得实际数字 */
			aun_red_buffer[i] =  ((temp[0]&0x03)<<16) |(temp[1]<<8) | temp[2];   
			aun_ir_buffer[i] =   ((temp[3]&0x03)<<16) |(temp[4]<<8) | temp[5];   
        
            if(aun_red_buffer[i]>un_prev_data)
            {
                f_temp=aun_red_buffer[i]-un_prev_data;
				
                f_temp/=(un_max-un_min);
				
                f_temp*=MAX_BRIGHTNESS;
				
                n_brightness-=(int32_t)f_temp;
				
                if(n_brightness<0)
                    n_brightness=0;
            }
            else
            {
                f_temp=un_prev_data-aun_red_buffer[i];
				
                f_temp/=(un_max-un_min);
				
                f_temp*=MAX_BRIGHTNESS;
				
                n_brightness+=(int32_t)f_temp;
				
                if(n_brightness>MAX_BRIGHTNESS)
                    n_brightness=MAX_BRIGHTNESS;
            }
		}
		
		/* 计算心率和血氧饱和度 */
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
		
		/* 通过UART将样本和计算结果发送到终端程序 */
		if((ch_hr_valid == 1)&& (n_heart_rate>=60) && (n_heart_rate<100))
		{

			dgb_printf_safe("心率=%d\r\n", n_heart_rate);
			
			sprintf(buff, ":%d", n_heart_rate);
			
			
			lcd_show_chn(60,160,13,BLACK,WHITE,32);
			lcd_show_chn(92,160,14,BLACK,WHITE,32);
			
			/* 显示心率 */
			lcd_show_string(130,160,buff,BLACK,WHITE,32,0);
			
			memset(buff, 0, sizeof buff);
		}

		
		if((ch_spo2_valid ==1)&& (n_sp02>=95) && (n_sp02<100))
		{
			dgb_printf_safe("血氧浓度=%d\r\n", n_sp02);

			sprintf(bufff, ":%d", n_sp02);
			
			lcd_show_chn(30,200,15,BLACK,WHITE,32);
			lcd_show_chn(62,200,16,BLACK,WHITE,32);
			
			lcd_show_chn(94,200,21,BLACK,WHITE,32);
			lcd_show_chn(126,200,18,BLACK,WHITE,32);
			
			/* 显示血氧浓度 */
			lcd_show_string(160,200,bufff,BLACK,WHITE,32,0);
			
			memset(bufff, 0, sizeof bufff);	
		}			
		
		delay_ms(100);
		
		
	}
}


/**
 * @brief 按键任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 */
static void app_task_key(void *pvParameters)
{
	EventBits_t EventValue;

	dgb_printf_safe("[app_task_key] create success\r\n");

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_KEYALL_DOWN,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* 延时消抖 */
		vTaskDelay(20);

		if (g_system_display_on == 0)
		{
			lcd_display_on(1);
			g_system_display_on = 1;
			continue;
		}

		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;

		if (EventValue & EVENT_GROUP_KEY1_DOWN)
		{
			/* 禁止EXTI0触发中断 */
			NVIC_DisableIRQ(EXTI0_IRQn);

			/* 确认是按下 */
			if (PAin(0) == 0)
			{

				dgb_printf_safe("[app_task_key] S1 Press\r\n");

				/* 发送KEY_UP按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_DOWN);
			}

			while (PAin(0) == 0)
				vTaskDelay(10);

			/* 允许EXTI0触发中断 */
			NVIC_EnableIRQ(EXTI0_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY2_DOWN)
		{
			/* 禁止EXTI2触发中断 */
			NVIC_DisableIRQ(EXTI2_IRQn);

			if (PEin(2) == 0)
			{
				dgb_printf_safe("[app_task_key] S2 Press\r\n");

				/* 发送KEY_DOWN按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_UP);
			}

			while (PEin(2) == 0)
				vTaskDelay(10);

			/* 允许EXTI2触发中断 */
			NVIC_EnableIRQ(EXTI2_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY3_DOWN)
		{
			/* 禁止EXTI3触发中断 */
			NVIC_DisableIRQ(EXTI3_IRQn);

			if (PEin(3) == 0)
			{
				dgb_printf_safe("[app_task_key] S3 Press\r\n");

				/* 发送KEY_ENTER按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
			}

			while (PEin(3) == 0)
				vTaskDelay(10);

			/* 允许EXTI3触发中断 */
			NVIC_EnableIRQ(EXTI3_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY4_DOWN)
		{
			/* 禁止EXTI4触发中断 */
			NVIC_DisableIRQ(EXTI4_IRQn);

			if (PEin(4) == 0)
			{

				dgb_printf_safe("[app_task_key] S4 Press\r\n");

				/* 发送KEY_BACK按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);
			}

			while (PEin(4) == 0)
				vTaskDelay(10);

			/* 允许EXTI4触发中断 */
			NVIC_EnableIRQ(EXTI4_IRQn);
		}
	}
}

/**
 * @brief adc任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 通过可调电阻获取的电压值，用作转盘菜单的控制
 */
static void app_task_adc(void *pvParameters)
{
	int32_t adc_vol_last = 0;
	int32_t adc_vol = 0;
	int32_t result;

	/* 获取当前电压初值 */
	adc_vol = adc_voltage_get();
	adc_vol_last = adc_vol;

	for (;;)
	{
		/* 获取电压值 */
		adc_vol = adc_voltage_get();

		result = adc_vol - adc_vol_last;

		if (result > 200 || result < -200)
		{

			/* 发送KEY_DOWN/KEY_UP按键事件 */
			xEventGroupSetBits(g_event_group, result > 0 ? EVENT_GROUP_FN_KEY_UP : EVENT_GROUP_FN_KEY_DOWN);

			dgb_printf_safe("[app_task_adc] adc_vol_last=%d adc_vol=%d\r\n", adc_vol_last, adc_vol);

			adc_vol_last = adc_vol;
		}

		vTaskDelay(200);
	}
}

/**
 * @brief 蜂鸣器任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 接收消息队列控制蜂鸣器。
 */
static void app_task_beep(void *pvParameters)
{
	beep_t beep;
	BaseType_t xReturn = pdFALSE;

	dgb_printf_safe("[app_task_beep] create success\r\n");

	for (;;)
	{
		xReturn = xQueueReceive(g_queue_beep,	/* 消息队列的句柄 */
								&beep,			/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		if (xReturn != pdPASS)
			continue;

		dgb_printf_safe("[app_task_beep] beep.sta=%d beep.duration=%d\r\n", beep.sta, beep.duration);

		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;

		/* 检查蜂鸣器是否需要持续工作 */
		if (beep.duration)
		{
			BEEP(beep.sta);

			while (beep.duration--)
				vTaskDelay(10);

			/* 蜂鸣器状态翻转 */
			beep.sta ^= 1;
		}

		BEEP(beep.sta);

		/* 释放信号量，告诉对方，当前beep控制任务已经完成 */
		xSemaphoreGive(g_sem_beep);
	}
}

/**
 * @brief 实时时间任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 接收消息队列控制蜂鸣器。
 */
static void app_task_rtc(void *pvParameters)
{

	beep_t beep;

	uint32_t i = 0;

	char buf[16] = {0};
	char buffweekday[32] = {0};
	
	uint16_t x;
	EventBits_t EventValue;	
	
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	dgb_printf_safe("[app_task_rtc] create success and suspend self\r\n");

	vTaskSuspend(NULL);
	
	for(;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_RTC_WAKEUP | EVENT_GROUP_RTC_ALARM,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);		
		
		if (EventValue & EVENT_GROUP_RTC_WAKEUP)
		{

				/* RTC_GetTime，获取时间 */
				RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

				memset(buf, 0, sizeof buf);

				/* 格式化字符串 */
				sprintf((char *)buf, "%02d:%02d:%02d", RTC_TimeStructure.RTC_Hours,
						RTC_TimeStructure.RTC_Minutes,
						RTC_TimeStructure.RTC_Seconds);
				
				lcd_draw_hour(RTC_TimeStructure.RTC_Hours);

				lcd_draw_seconds(RTC_TimeStructure.RTC_Seconds, ((LCD_WIDTH-strlen(buf)*32/2)/2)+65, 160);
				// lcd_draw_picture(105,65,30,60,gImage_maohao);
				lcd_draw_minutes(RTC_TimeStructure.RTC_Minutes);

				//dgb_printf_safe("[app_task_rtc——time] %s\r\n", buf);
				
				/* RTC_GetTime，获取日期 */
				RTC_GetDate(RTC_Format_BCD, &RTC_DateStructure);

				memset(buf, 0, sizeof buf);
				
				memset(buffweekday, 0, sizeof buffweekday);

				/* 格式化将日期提取出来 */
				sprintf((char *)buf, "20%02x-%02x-%02x", RTC_DateStructure.RTC_Year,
						RTC_DateStructure.RTC_Month,
						RTC_DateStructure.RTC_Date);

				sprintf((char *)buffweekday, "%d", 
						RTC_DateStructure.RTC_WeekDay);
				
				
				/* 显示日期 */	
				x=(LCD_WIDTH-strlen(buf)*32/2)/2;
				
				LCD_SAFE(
				
				/* 显示年月日 */
				lcd_show_string(x,125,buf,BLACK,WHITE,32,0);
				
				/* 显示星期 */
				lcd_show_chn(x,160,32,BLACK,WHITE,32);
				lcd_show_chn(x+30,160,33,BLACK,WHITE,32);
				lcd_show_string(x+60,160,buffweekday,BLACK,WHITE,32,0);
				);
				
				//dgb_printf_safe("[app_task_rtc——date] %s\r\n", buf);
				// 蓝牙图标
				g_ble_status = ble_connect_sta_get();
				if (g_ble_status == FLAG_BLE_STATUS_CONNECT)
				{
					LCD_SAFE(
						lcd_draw_picture(0, 0, 32, 32, gImage_ble_32x32););
				}
				if (g_ble_status == FLAG_BLE_STATUS_NONE)
				{
					LCD_SAFE(
						lcd_fill(0, 0, 32, 32, WHITE););
				}

				// 闹钟图标
				if (g_alarm_pic)
				{
					LCD_SAFE(
						lcd_draw_picture(40, 0, 32, 32, gImage_alarm_32x32););
				}
				else
				{
					LCD_SAFE(
						lcd_fill(40, 0, 32, 32, WHITE););
				}

				/* 显示蓝牙连接状态 */
				// ble_connect_get = ble_connect_sta_get();
				// if(ble_connect_get == BLE_CONNECT)
				// {
				// 	//dgb_printf_safe("蓝牙已连接");
					
				// 	LCD_SAFE(
				// 	lcd_draw_picture(200,20,30,30,gImage_ble3_30x30);
				// 	);
				// }
		
				//dgb_printf_safe("ble_connect_get == %d\r\n", ble_connect_get);

				
		}

		if (EventValue & EVENT_GROUP_RTC_ALARM)
				{
					printf("闹钟响了！\n");

					for (i = 0; i < 5; i++)
					{
						/* 嘀一声示意 */
						beep.sta = 1;
						beep.duration = 10;

						xQueueSend(g_queue_beep, /* 消息队列的句柄 */
								&beep,		 /* 发送的消息内容 */
								100);		 /* 等待时间 100 Tick */
						delay_ms(500);
					}
				}
	}
}

/**
 * @brief 温湿度任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 接收消息队列控制蜂鸣器。
 */
static void app_task_dht(void *pvParameters)
{
	uint8_t dht11_data[5] = {0};
	char buf[16] = {0};

	int32_t rt = -1;
	uint16_t x;

	dgb_printf_safe("[app_task_dht] create success and suspend self \r\n");

	vTaskSuspend(NULL);

	dgb_printf_safe("[app_task_dht] resume success\r\n");

	for (;;)
	{
		rt = dht11_read_data(dht11_data);

		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;

		if (rt == 0)
		{
			memset(buf, 0, sizeof buf);

			if (g_dht_get_what == FLAG_DHT_GET_TEMP)
				sprintf((char *)buf, "Temp:%02d.%d", dht11_data[2], dht11_data[3]);

			if (g_dht_get_what == FLAG_DHT_GET_HUMI)
				sprintf((char *)buf, "Humi:%02d.%d", dht11_data[0], dht11_data[1]);

			/* 显示温度/湿度 */
			x = (LCD_WIDTH - strlen(buf) * 32 / 2) / 2;
			lcd_show_string(x, 160, buf, BLACK, WHITE, 32, 0);

			dgb_printf_safe("[app_task_dht] %s\r\n", buf);
		}

		vTaskDelay(6000);
	}
}

/**
 * @brief 菜单任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 无
 */
static void app_task_menu(void *pvParameters)
{
	EventBits_t EventValue;

	beep_t beep;

	uint32_t item_total;	  // 记录当前菜单有多少个项目
	uint32_t item_cursor = 0; // 记录当前菜单指向哪个项目
	uint32_t fun_run = 0;	  // 记录是否有项目功能函数在执行

	menu_ext_t m_ext;

	menu_t *m;

	/* 通过参数传递获取主菜单页指针 */
	menu_t **m_main = (menu_t **)pvParameters;

	uint16_t vs = 0;

	/* 配置m_ext指向的当前菜单及相关参数 */
	m_ext.menu = *m_main;
	m_ext.key_fn = KEY_NONE;
	m_ext.item_cursor = item_cursor;
	m_ext.item_total = menu_item_total(m_ext.menu);

	/* 配置记录菜单指针m */
	m = m_ext.menu;

	dgb_printf_safe("[app_task_menu] create success\r\n");

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_FN_KEY_UP | EVENT_GROUP_FN_KEY_DOWN | EVENT_GROUP_FN_KEY_ENTER | EVENT_GROUP_FN_KEY_BACK,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;

		/* 嘀一声示意 */
		beep.sta = 1;
		beep.duration = 1;

		xQueueSend(g_queue_beep, /* 消息队列的句柄 */
				   &beep,		 /* 发送的消息内容 */
				   100);		 /* 等待时间 100 Tick */

		if (EventValue & EVENT_GROUP_FN_KEY_UP)
		{

			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}

			dgb_printf_safe("[app_task_menu] KEY_UP\r\n");

			dgb_printf_safe("[app_task_menu] item_total=%d\r\n", menu_item_total(m));

			if (item_cursor)
			{

				dgb_printf_safe("[app_task_menu] item_cursor1=%d\r\n", item_cursor);

				item_cursor--;

				dgb_printf_safe("[app_task_menu] item_cursor2=%d\r\n", item_cursor);

				/* 项目菜单指向上一个菜单 */
				m--;

				/* 设置RAM的垂直滚动起始地址 */
				if (item_cursor == 0)
				{
					while (vs)
					{
						vs--;
						LCD_SAFE(lcd_vs_set(vs));
						vTaskDelay(5);
					}
				}

				/* 显示光标 */
				LCD_SAFE(
					lcd_fill(230, (item_cursor + 1) * 60, 10, 50, WHITE);
					lcd_fill(230, item_cursor * 60, 10, 50, GREY););
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor3=%d\r\n", item_cursor);

				/* 若main菜单有同级的左侧菜单 */
				if (m->same_left)
				{
					dgb_printf_safe("[app_task_menu] menu main switch to the left menu\r\n");

					item_cursor = 0;

					m_main--;

					m_ext.menu = *m_main;
					m_ext.key_fn = KEY_NONE;
					m_ext.item_cursor = 0;
					m_ext.item_total = menu_item_total(m_ext.menu);

					m = m_ext.menu;

					/* 显示菜单内容 */
					menu_show(&m_ext);
				}
			}

			/* 保存当前按键编码 */
			m_ext.key_fn = KEY_UP;

			/* 保存当前项目光标位置值 */
			m_ext.item_cursor = item_cursor;
		}

		if (EventValue & EVENT_GROUP_FN_KEY_DOWN)
		{
			dgb_printf_safe("[app_task_menu] KEY_DOWN\r\n");

			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}

			/* 获取当前菜单有多少个项目 */
			item_total = menu_item_total(m_ext.menu);

			/* 保存当前菜单有多少个项目 */
			m_ext.item_total = item_total;

			dgb_printf_safe("[app_task_menu] item_total=%d\r\n", item_total);

			if (item_cursor < (item_total - 1))
			{
				dgb_printf_safe("[app_task_menu] item_cursor4=%d\r\n", item_cursor);

				item_cursor++;

				/* 设置RAM的垂直滚动起始地址 */
				if (item_cursor == 4)
				{
					while (vs < 60)
					{
						vs++;
						LCD_SAFE(lcd_vs_set(vs));
						vTaskDelay(5);
					}
				}

				dgb_printf_safe("[app_task_menu] item_cursor5=%d\r\n", item_cursor);

				/* 显示光标 */
				LCD_SAFE(
					lcd_fill(230, (item_cursor - 1) * 60, 10, 50, WHITE);
					lcd_fill(230, item_cursor * 60, 10, 50, GREY););

				/* 项目菜单指向下一个菜单 */
				m++;
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor6=%d\r\n", item_cursor);

				/* 若main菜单有同级的右侧菜单 */

				if (m->same_right)
				{
					dgb_printf_safe("[app_task_menu] menu main switch to the right menu\r\n");
					item_cursor = 0;
					m_main++;

					m_ext.menu = *m_main;
					m_ext.key_fn = KEY_NONE;
					m_ext.item_cursor = 0;
					m_ext.item_total = menu_item_total(m_ext.menu);

					m = m_ext.menu;

					vs = 0;

					/* 显示菜单内容 */
					menu_show(&m_ext);
				}
			}

			/* 保存当前按键编码 */
			m_ext.key_fn = KEY_DOWN;

			/* 保存当前项目光标位置值 */
			m_ext.item_cursor = item_cursor;
		}

		if (EventValue & EVENT_GROUP_FN_KEY_ENTER)
		{
			m_ext.key_fn = KEY_ENTER;

			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}

			dgb_printf_safe("[app_task_menu] KEY_ENTER item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_ENTER item name %s\r\n", m->item ? m->item : "NULL");

			m_ext.item_cursor = item_cursor;

			/* 子菜单有效 */
			if (m->child)
			{
				/* 指向子菜单 */
				m = m->child;

				/* 保存当前菜单 */
				m_ext.menu = m;

				/* 显示菜单内容 */
				menu_show(&m_ext);

				/* 复位光标值 */
				item_cursor = 0;
			}

			/* 若没有子菜单,则直接执行功能函数 */
			if (!m->child && m->fun)
			{
				/* 标记有项目功能函数在运行 */
				fun_run = 1;

				m->fun(&m_ext);
			}
		}

		if (EventValue & EVENT_GROUP_FN_KEY_BACK)
		{
			m_ext.key_fn = KEY_BACK;

			dgb_printf_safe("[app_task_menu] KEY_BACK item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_BACK item name %s\r\n", m->item ? m->item : "NULL");

			/* 若子菜单功能函数有效，先执行，主要是挂起对应任务 */
			if (m->fun)
			{
				/* 标记有项目功能函数在运行 */
				fun_run = 1;

				m->fun(&m_ext);
			}

			/* 父菜单有效 */
			if (m->parent)
			{
				/* 指向父菜单 */
				m = m->parent;

				/* 保存当前菜单 */
				m_ext.menu = m;

				/* 复位光标值 */
				item_cursor = 0;
				m_ext.item_cursor = 0;

				dgb_printf_safe("[app_task_menu] m->parent item cursor=%d\r\n", item_cursor);
				dgb_printf_safe("[app_task_menu] m->parent item name %s\r\n", m->item ? m->item : "NULL");

				fun_run = 0;

				/* 显示当前菜单 */
				menu_show(&m_ext);
			}
		}
	}
}

/**
 * @brief 软件定时器任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 倒数管理、独立看门狗喂狗
 */
static void soft_timer_callback(TimerHandle_t pxTimer)
{

	/* 统计系统无操作计数值自加1 */
	g_system_no_opreation_cnt++;

	if (g_system_no_opreation_cnt >= 20)
	{
		/* 熄屏 */
		LCD_SAFE(
			lcd_display_on(0););

		g_system_display_on = 0;
	}
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

void vApplicationTickHook(void)
{
}
