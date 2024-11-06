/****************************************************************
*��    ��:FreeRTOS�ۺϴ���ʵ��-LCD�汾
*��    ��:������
*��������:2024/10/30
*˵  ��:
	1.��ǰ����淶���ṹ������������ʵ����ģ�黯��̣�һ���������һ��Ӳ����
	  �ô��������á�FreeRTOS�ۺϴ���ʵ��-OLED�汾�����ص㽫OLED��ʾ�޸�ΪLCD��ʾ
	2.�˵����ӦӲ��ģ�鹦��
		�˵���1ҳ
			.Home 			[��ʵ��]  ��ʾ��ǰʱ�䡢���ڣ���ʾ����������ͼ��
			.Blue tooth		[��ʵ��]  ʱ�䡢���ڡ���������
			.Sport			[��ʵ��]  �ǲ�����������
			.Pulse SO2		[��ʵ��]  ����Ѫ��
			.DHT11  		[��ʵ��]  ��ʪ��

		�˵���2ҳ
			.Password 		[��ʵ��]  �������
			
*����˵��:
	1.��������
		.����1�밴��2:�˵����ѡ��
		.����3:����˵���
		.����4:�˳��˵���
	2.�ɵ�����
		.��ťʽѡ���Ӧ�Ĳ˵�������iPod
	3.������
		.ִ�ж�Ӧ�Ĳ�������һ��ʾ��
	4.�˵���ʵ��
		.�༶�˵��ķ���
	5.�Զ�Ϩ��
		.���޲�������һ���Զ�Ϩ��
		.�������ⰴ��������Ļ
	6.����ʶ��
		.�ϻ����»�:�˵����ѡ��
		.��:����˵���
		.�һ�:�˳��˵���
	7.����ʶ��
		.����:С��


*�ؼ�������
	1.app_task_menu���ļ�·����main.c
		.����Բ˵���ѡ��/����/�˳��Ĺ���
		.��ʾ���ָʾ��ǰ�˵���

	2.menu_show,�ļ�·����menu.c
		.��ʾ�˵���ͼ��
		.��ʾ�˵�������

*�ؼ��������ͣ�
	1.menu_t���ļ�·����menu.h
		.��ʽ�洢ˮƽ��/�Ҳ˵�����/�Ӳ˵�

*�ؼ��꣺
	1.DEBUG_PRINTF_EN���ļ�·����includes.h
		.ʹ����ͨ������1���������Ϣ

	2.LCD_SAFE���ļ�·����includes.h
		.ʹ�û���������LCD����
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

/* �����ʱ����� */
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
volatile uint32_t g_gesture_display_flag = FLAG_GESTURE_DISPLAY_OFF; // ��¼����ʶ��ģ���Ƿ���ʾ����ͼƬ
volatile uint32_t g_gesture_valid = 0;//����ʶ��ģ���Ƿ���Ч								 // ��¼����ʶ��ģ���Ƿ����ӳɹ�
volatile uint32_t g_alarm_pic = 0;//����ͼ��
// volatile uint32_t ulCount = 0;
// volatile uint32_t step_count = 0;
volatile uint32_t g_system_no_opreation_cnt = 0;//ͳ��ϵͳ�޲�������ֵ
volatile uint32_t g_system_display_on = 1;//����
volatile uint32_t ble_connect_get = 0;//��������״̬


// max30102����ı���
volatile uint32_t aun_ir_buffer[500];  // IR LED sensor data
volatile int32_t n_ir_buffer_length;   // data length
volatile uint32_t aun_red_buffer[500]; // Red LED sensor data
volatile int32_t n_sp02;			   // SPO2 value
volatile int8_t ch_spo2_valid;		   // indicator to show if the SP02 calculation is valid
volatile int32_t n_heart_rate;		   // heart rate value
volatile int8_t ch_hr_valid;		   // indicator to show if the heart rate calculation is valid
volatile uint8_t uch_dummy;

/* ��Ϣ���о�� */
QueueHandle_t g_queue_led;
QueueHandle_t g_queue_beep;
QueueHandle_t g_queue_usart;
QueueHandle_t g_queue_kbd;

/* �¼���־���� */
EventGroupHandle_t g_event_group;

/* �������ź������ */
SemaphoreHandle_t g_mutex_printf;
SemaphoreHandle_t g_mutex_lcd;
SemaphoreHandle_t g_sem_beep;

/* ����*/
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

/* �����ʱ�� */
static void soft_timer_callback(TimerHandle_t pxTimer);

void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_PRINTF_EN

	va_list args;
	va_start(args, format);

	/* ��ȡ�����ź��� */
	xSemaphoreTake(g_mutex_printf, portMAX_DELAY);

	vprintf(format, args);

	/* �ͷŻ����ź��� */
	xSemaphoreGive(g_mutex_printf);

	va_end(args);
#else
	(void)0;
#endif
}

/* �����б� */
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
	/* ����ϵͳ�ж����ȼ�����4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);

	/* ��ʼ������1 */
	usart_init(115200);

	/* ����app_task_init���� */
	xTaskCreate((TaskFunction_t)app_task_init,			/* ������ں��� */
				(const char *)"app_task_init",			/* �������� */
				(uint16_t)512,							/* ����ջ��С */
				(void *)NULL,							/* ������ں������� */
				(UBaseType_t)5,							/* ��������ȼ� */
				(TaskHandle_t *)&app_task_init_handle); /* ������ƿ�ָ�� */

	/* ����������� */
	vTaskStartScheduler();

	printf("none run here...\r\n");

	while (1)
		;
}

void lcd_startup_info(void)
{
	uint16_t x = 0;
	LCD_SAFE(

		/* ������ʾ��ʼλ�� */
		lcd_vs_set(0);

		/* ���� */
		lcd_clear(WHITE);
		lcd_fill(0, LCD_HEIGHT, LCD_WIDTH, 80, WHITE);

		/* ��ʾ������ */
		lcd_draw_picture(90, 70, 60, 60, gImage_pic_red_flag);

		x = 50; lcd_show_chn(x, 170, 0, BLACK, WHITE, 32);
		x += 50; lcd_show_chn(x, 170, 1, BLACK, WHITE, 32);
		x += 50; lcd_show_chn(x, 170, 2, BLACK, WHITE, 32);
	
		x = 0;
		while (x < LCD_WIDTH) {
			/* ������ */
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

	/* lcd��ʼ�� */
	lcd_init();

	/* beep��ʼ�� */
	beep_init();

	/* ������ʼ�� */
	key_init();

	/* adc��ʼ�� */
	adc_init();

	/* rtc��ʼ�� */
	rtc_init();

	/* ��ʪ��ģ���ʼ�� */
	dht11_init();
	
	
	ble_init(9600);


	/* MPU6050��ʼ�� */	
	MPU_Init();

	/* ����Ѫ��ģ���ʼ�� */
	max30102_init();

	// ��ʼ��MPU6050
	// MPU_Init();
	// while (mpu_dmp_init())
	// {
	// 	printf("[ERROR] MPU6050 ERROR \r\n");
	// 	delay_ms(500);
	// }

	/* eeprom��ʼ�� */
	at24c02_init();

	/* ������̳�ʼ�� */
	kbd_init();

	asr_init(9600);

	taskENTER_CRITICAL();

	/* ������Ϣ���� */
	g_queue_usart = xQueueCreate(5, 128);
	g_queue_beep = xQueueCreate(QUEUE_BEEP_LEN, sizeof(beep_t));
	g_queue_kbd 	= xQueueCreate(5, sizeof(uint8_t));

	/* �����������ź��� */
	g_mutex_printf = xSemaphoreCreateMutex();
	g_mutex_lcd = xSemaphoreCreateMutex();
	g_sem_beep = xSemaphoreCreateBinary();

	/* �����¼���־�� */
	g_event_group = xEventGroupCreate();

	/* �����õ������� */
	i = 0;
	while (task_tbl[i].pxTaskCode)
	{
		xTaskCreate(task_tbl[i].pxTaskCode,		/* ������ں��� */
					task_tbl[i].pcName,			/* �������� */
					task_tbl[i].usStackDepth,	/* ����ջ��С */
					task_tbl[i].pvParameters,	/* ������ں������� */
					task_tbl[i].uxPriority,		/* ��������ȼ� */
					task_tbl[i].pxCreatedTask); /* ������ƿ�ָ�� */
		i++;
	}

	taskEXIT_CRITICAL();

	/* �������������ʱ�� */
	soft_timer_Handle = xTimerCreate((const char *)"AutoReloadTimer",
									 (TickType_t)1000,	  /* ��ʱ������ 1000(tick) */
									 (UBaseType_t)pdTRUE, /* ����ģʽ */
									 (void *)1,			  /* Ϊÿ����ʱ������һ��������ΨһID */
									 (TimerCallbackFunction_t)soft_timer_callback);
	/* �������������ʱ�� */
	xTimerStart(soft_timer_Handle, 0);

	
	vTaskSuspend(app_task_kbd_handle);
	vTaskSuspend(app_task_max30102_handle);

	lcd_startup_info();

	/* PAJ7620U2����ʶ�𴫸������ӵ������壬����ʼ���ɹ���Ŵ�������ʶ������ */
	if (paj7620u2_init())
	{
		lcd_clear(WHITE);
		lcd_fill(0, LCD_HEIGHT, LCD_WIDTH, 80, WHITE);

		dgb_printf_safe("[app_task_init] paj7620u2 init success\r\n");

		/* ��¼����ʶ��ģ����Ч�ԣ�1-������Ч 0-������Ч */
		g_gesture_valid = 1;

		/* ��ʾ����ʶ��ģ��ͼ�� */
		// OLED_DrawBMP(0, 0, 64, 8, (uint8_t *)pic_gesture_icon);

		/* ��ʾ"���ӳɹ�" */
		// OLED_ShowCHinese(64, 4, 22);
		// OLED_ShowCHinese(90, 4, 23);
		// OLED_ShowCHinese(64, 6, 30);
		// OLED_ShowCHinese(90, 6, 31);
		/* ����Ҫ��ʾ��ͼƬ */
 							
		lcd_draw_picture(96,80,48,48,gImage_hand_48x48);
		/*�������ӳɹ� */
		lcd_show_chn(30,160,38,BLACK,WHITE,32);
		lcd_show_chn(62,160,39,BLACK,WHITE,32);
		lcd_show_chn(94,160,40,BLACK,WHITE,32);
		lcd_show_chn(126,160,41,BLACK,WHITE,32);
		lcd_show_chn(158,160,42,BLACK,WHITE,32);
		lcd_show_chn(190,160,43,BLACK,WHITE,32);
		

		/* ����2�� */
		vTaskDelay(2000);

		/* ����app_task_gesture���� */
		xTaskCreate((TaskFunction_t)app_task_gesture,		   /* ������ں��� */
					(const char *)"app_task_gesture",		   /* �������� */
					(uint16_t)512,							   /* ����ջ��С */
					(void *)NULL,							   /* ������ں������� */
					(UBaseType_t)5,							   /* ��������ȼ� */
					(TaskHandle_t *)&app_task_gesture_handle); /* ������ƿ�ָ�� */
	}

	/* ��ʾ���˵� */
	m_ext.menu = menu_main_1;

	menu_show(&m_ext);

	vTaskDelete(NULL);
}

/**
 * @brief ����ʶ������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ͨ����ȡ����ֵ�������˵��Ŀ���
 */
static void app_task_gesture(void *pvParameters)
{
	EventBits_t EventValue;
	BaseType_t xReturn;
	// oled_t oled;
	uint16_t gesture;
	uint32_t i = 0;

	// /* ����ͼƬ�б�NULL��ʾ����ʾ������ */
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

	// /* ��ǰ����ͼƬ��ʾ��λ�á���С */
	// oled.x = 30;
	// oled.y = 0;
	// oled.pic_width = 64;
	// oled.pic_height = 8;

	dgb_printf_safe("[app_task_gesture] create success\r\n");
	// dgb_printf_safe("[app_task_gesture] pic_gesture_icon_tbl array size is %d \r\n", ARRAY_SIZE(pic_gesture_icon_tbl));

	for (;;)
	{
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_GESTURE,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		if (!(EventValue & EVENT_GROUP_GESTURE))
			continue;

		/* ����ͳ��ϵͳ�޲�������ֵ */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		/* ��ȡ���� */
		gesture = gesture_get();

		// /* ��ȡ����ͼƬ */
		// for (i = 0; i < ARRAY_SIZE(pic_gesture_icon_tbl); i++)
		// {
		// 	if (gesture & (1 << i))
		// 		break;
		// }

		// /* ��ʾ����ͼƬ */
		// if ((g_gesture_display_flag == FLAG_GESTURE_DISPLAY_ON) && pic_gesture_icon_tbl[i])
		// {
		// 	dgb_printf_safe("[app_task_gesture] gesture is %d \r\n", i);

		// 	oled.ctrl = OLED_CTRL_SHOW_PICTURE;

		// 	oled.pic = pic_gesture_icon_tbl[i];

		// 	xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
		// 						 &oled,		   /* ���͵���Ϣ���� */
		// 						 100);		   /* �ȴ�ʱ�� 100 Tick */
		// 	if (xReturn != pdPASS)
		// 		dgb_printf_safe("[app_task_gesture] xQueueSend oled picture error code is %d\r\n", xReturn);
		// }

		/* �������ƣ�ִ�ж�Ӧ�İ������� */
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
				/* ��ʾ��ʱ��ͼ����Զ��˳� */
				vTaskDelay(1000);
				g_gesture_display_flag = FLAG_GESTURE_DISPLAY_OFF;
			}

			/* ������ʾ����ʱ��ֻ����ʱ�봥�������˳� */
			xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);
		}
		break;

		default:
			break;
		}
	}
}

/**
 * @brief �������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ������֤�������޸�
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
		/* �ȴ����������Ϣ */
		xReturn = xQueueReceive(g_queue_kbd,&key_val,portMAX_DELAY);

		if(xReturn!=pdPASS)
			continue;

		/* ����ͳ��ϵͳ�޲�������ֵ */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();			
		
		/* ��λ����������� */
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

		/* ������֤���� */
		if(g_pass_man_what == FLAG_PASS_MAN_AUTH)
		{
			/* û�����룬��Чɾ������ */
			if(key_val == '*' && key_cnt==0)
				continue;

			/* ɾ����һ������ */
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
				/* ��ʾ���� */
				key_buf[key_cnt] = (uint8_t)key_val;
				lcd_show_char(x,y,key_buf[key_cnt],BLACK,WHITE,32,0);
				vTaskDelay(100);

				/* ��ʾ* */
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
					
					/* ��ȡeeprom�洢������ */
					at24c02_read(PASS1_ADDR_IN_EEPROM,(uint8_t *)pass_auth_eeprom,PASS_LEN);

					/* ƥ��eeprom�洢������ */
					rt = memcmp(pass_auth_eeprom,key_buf,PASS_LEN);
					if(rt)
					{
						dgb_printf_safe("[app_task_pass_man] password auth by eeprom fail\r\n");
						
						/* ƥ��Ĭ������ */
						rt=memcmp(pass_auth_default,key_buf,PASS_LEN);
						if(rt)
							dgb_printf_safe("[app_task_pass_man] password auth by defalut fail too\r\n");
					}
					
					/* ����ƥ��ɹ� */
					if(rt==0)
					{
						dgb_printf_safe("[app_task_pass_man] password auth success\r\n");
						
						lcd_clear(WHITE);
						lcd_fill(0,y,LCD_WIDTH,32,WHITE);
						/* ����Ҫ��ʾ��ͼƬ-���飨�ɹ��� */
						LCD_SAFE
						(					
							lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);
						);
						
						/* ��һ��ʾ��:��һ�� */
						beep.sta = 1;
						beep.duration = 100;

						xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
											&beep,		   /* ���͵���Ϣ���� */
											100);		   /* �ȴ�ʱ�� 100 Tick */

						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

						/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);							

						vTaskDelay(100);
					}
					else
					{
						lcd_clear(WHITE);
						lcd_fill(0,y,LCD_WIDTH,32,WHITE);
						
						/* ����Ҫ��ʾ��ͼƬ-���飨ʧ�ܣ� */
						LCD_SAFE
						(					
							lcd_draw_picture(70, 90, 100, 100, gImage_pic_fail_100x100);
						);
						/* ����1��ʾ�� */
						beep.sta = 1;
						beep.duration = 100;

						xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
											&beep,		   /* ���͵���Ϣ���� */
											100);		   /* �ȴ�ʱ�� 100 Tick */				
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

						/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);	

					}

					/* ��ʱһ�� */
					vTaskDelay(1000);

					/* �ָ���ֵ */
					x=x_start;
					key_cnt=0;
					memset(key_buf,0,sizeof key_buf);	

					// lcd_sprintf_hzstr32s((LCD_WIDTH-(strlen("������֤ʧ��")*16))/2,y,(uint8_t*)"������֤ʧ��",BLACK,WHITE);
					// vTaskDelay(1000);
					lcd_clear(WHITE);
					lcd_fill(0,y,LCD_WIDTH,32,WHITE);
					
					/* ����Ҫ��ʾ��ͼƬ */
					LCD_SAFE
					(					
						lcd_draw_picture(96,80,48,48,gImage_auto_48x48);
						/* ��ʾ����-������ */
						lcd_show_chn(30,160,34,BLACK,WHITE,32);
						lcd_show_chn(62,160,35,BLACK,WHITE,32);
						lcd_show_chn(94,160,36,BLACK,WHITE,32);
						/* ���������ʾ�� */
						// lcd_show_string(160,160,"------",BLACK,WHITE,32,0);
					);
				}
			}
		}

		/* �����޸Ĳ��� */
		if(g_pass_man_what == FLAG_PASS_MAN_MODIFY)
		{
			switch(pass_modify_sta)
			{
				case PASS_MODIFY_STA_OLD_PASS:
				{
					/* û�����룬��Чɾ������ */
					if(key_val == '*' && key_cnt==0)
						continue;

					/* ɾ����һ������ */
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
						// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
						// 					&oled,		   /* ���͵���Ϣ���� */
						// 					100);		   /* �ȴ�ʱ�� 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						x-=16;
						lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
						continue;				
					}

					if(key_cnt<6)
					{
						/* ��ʾ���� */
						// key_buf[key_cnt] = key_val;

						// oled.ctrl = OLED_CTRL_SHOW_STRING;
						// oled.x = x;
						// oled.y = 2;
						// oled.font_size = 16;

						// oled.str =(uint8_t *) &key_buf[key_cnt];
						// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
						// 					&oled,		   /* ���͵���Ϣ���� */
						// 					100);		   /* �ȴ�ʱ�� 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						key_buf[key_cnt] = (uint8_t)key_val;
						lcd_show_char(x,y,key_buf[key_cnt],BLACK,WHITE,32,0);

						vTaskDelay(100);

						// oled.str = "*";
						// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
						// 					&oled,		   /* ���͵���Ϣ���� */
						// 					100);		   /* �ȴ�ʱ�� 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

						// x+=8;
						// key_cnt++;
						/* ��ʾ* */
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

					/* û�����룬��Чɾ������ */
					if(key_val == '*' && key_cnt==0)
						continue;

					/* ɾ����һ������ */
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
						// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
						// 					&oled,		   /* ���͵���Ϣ���� */
						// 					100);		   /* �ȴ�ʱ�� 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						x-=16;
						lcd_show_char(x,y,' ',BLACK,WHITE,32,0);
						continue;				
					}


					if(key_cnt<6)
					{
						/* ��ʾ'*' */
						// key_buf[key_cnt] = key_val;

						// oled.ctrl = OLED_CTRL_SHOW_STRING;
						// oled.x = x;
						// oled.y = 6;
						// oled.font_size = 16;

						// oled.str =(uint8_t *) &key_buf[key_cnt];
						// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
						// 					&oled,		   /* ���͵���Ϣ���� */
						// 					100);		   /* �ȴ�ʱ�� 100 Tick */
						// if (xReturn != pdPASS)
						// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
						key_buf[key_cnt] = (uint8_t)key_val;
						lcd_show_char(x,y,key_buf[key_cnt],BLACK,WHITE,32,0);

						vTaskDelay(100);

						// oled.str = "*";
						// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
						// 					&oled,		   /* ���͵���Ϣ���� */
						// 					100);		   /* �ȴ�ʱ�� 100 Tick */
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
							/* ���������� */
							memcpy(pass_new,key_buf,PASS_LEN);

							/* ��ȡeeprom�洢������ */
							at24c02_read(PASS1_ADDR_IN_EEPROM,(uint8_t *)pass_auth_eeprom,PASS_LEN);
							dgb_printf_safe("[app_task_pass_man] password eeprom is %6s\r\n",pass_auth_eeprom);
							/* ƥ��eeprom�洢������ */
							rt = memcmp(pass_auth_eeprom,pass_old,PASS_LEN);
							if(rt)
							{
								dgb_printf_safe("[app_task_pass_man] password old is %6s\r\n",pass_old);
								dgb_printf_safe("[app_task_pass_man] password auth by eeprom fail\r\n");
								
								/* ƥ��Ĭ������ */
								rt=memcmp(pass_auth_default,pass_old,PASS_LEN);
								dgb_printf_safe("[app_task_pass_man] password default is %6s\r\n",pass_auth_default);
								if(rt)
								{
									dgb_printf_safe("[app_task_pass_man] password auth by defalut fail too\r\n");
								}
									
							}
							
							/* ����ƥ��ɹ� */
							if(rt==0)
							{
								
								dgb_printf_safe("[app_task_pass_man] password auth success\r\n");
								
								/* �����ٽ������ɿ�����eepromд�������� */
								taskENTER_CRITICAL();
								rt = at24c02_write(PASS1_ADDR_IN_EEPROM,(uint8_t *)pass_new,PASS_LEN);
								taskEXIT_CRITICAL();

								if(rt == 0)
									dgb_printf_safe("[app_task_pass_man] password new is %6s,write into eeprom success\r\n",pass_new);
								else
									dgb_printf_safe("[app_task_pass_man] password new is %6s,write into eeprom fail,error code is %d\r\n",pass_new,rt);

								/* ����Ҫ��ʾ��ͼƬ-���飨�ɹ��� */
								// oled.x=64;
								// oled.y=0;
								// oled.pic_width=64;
								// oled.pic_height=8;
								// oled.ctrl=OLED_CTRL_SHOW_PICTURE;
								// oled.pic=pic_face_success_icon;

								// xReturn = xQueueSend( 	g_queue_oled,	/* ��Ϣ���еľ�� */
								// 						&oled,			/* ���͵���Ϣ���� */
								// 						100);			/* �ȴ�ʱ�� 100 Tick */
								// if(xReturn != pdPASS)
								// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n",xReturn);								
								lcd_clear(WHITE);
								lcd_fill(0,y,LCD_WIDTH,32,WHITE);
								/* ����Ҫ��ʾ��ͼƬ-���飨�ɹ��� */
								LCD_SAFE
								(					
									lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);
								);
								
								/* ��һ��ʾ��:��һ�� */
								beep.sta = 1;
								beep.duration = 100;

								xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
													&beep,		   /* ���͵���Ϣ���� */
													100);		   /* �ȴ�ʱ�� 100 Tick */				
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

								/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
								xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);										

								vTaskDelay(100);

								}
							else
							{
								/* ����Ҫ��ʾ��ͼƬ-���飨ʧ�ܣ� */
								// oled.x=64;
								// oled.y=0;
								// oled.pic_width=64;
								// oled.pic_height=8;
								// oled.ctrl=OLED_CTRL_SHOW_PICTURE;
								// oled.pic=pic_face_fail_icon;

								// xReturn = xQueueSend( 	g_queue_oled,	/* ��Ϣ���еľ�� */
								// 						&oled,			/* ���͵���Ϣ���� */
								// 						100);			/* �ȴ�ʱ�� 100 Tick */
								// if(xReturn != pdPASS)
								lcd_clear(WHITE);
								lcd_fill(0,y,LCD_WIDTH,32,WHITE);
								
								/* ����Ҫ��ʾ��ͼƬ-���飨ʧ�ܣ� */
								LCD_SAFE
								(					
									lcd_draw_picture(70, 90, 100, 100, gImage_pic_fail_100x100);
								);

								dgb_printf_safe("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n",xReturn);


								/* ����1��ʾ�� */
								beep.sta = 1;
								beep.duration = 100;

								xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
													&beep,		   /* ���͵���Ϣ���� */
													100);		   /* �ȴ�ʱ�� 100 Tick */				
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);	

								/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
								xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
								if (xReturn != pdPASS)
									dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);											


							}

							/* ��ʱһ�� */
							vTaskDelay(1000);	


							/* ����Ұ벿������ */
							// oled.ctrl = OLED_CTRL_CLEAR_REGION;
							// oled.x = 64;
							// oled.y = 0;
							// oled.clr_region_width=64;
							// oled.clr_region_height=8;
							// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
							// 					&oled,		   /* ���͵���Ϣ���� */
							// 					100);		   /* �ȴ�ʱ�� 100 Tick */
							// if (xReturn != pdPASS)
							// 	dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
							lcd_clear(WHITE);
							lcd_fill(0,y,LCD_WIDTH,32,WHITE);
							

							// /* ��ʾ����-������ */
							// oled.ctrl = OLED_CTRL_SHOW_CHINESE;
							// oled.y = 0;
							// chn_tbl[0]=39;chn_tbl[1]=40;chn_tbl[2]=41;
						
							// for(i=0; i<3; i++)
							// {
							// 	oled.x = 66+i*16;
							// 	oled.chinese = chn_tbl[i];

							// 	xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
							// 						&oled,		   /* ���͵���Ϣ���� */
							// 						100);		   /* �ȴ�ʱ�� 100 Tick */
							// 	if (xReturn != pdPASS)
							// 		dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);

							// }


							// /* ��վ�������ʾ�� */
							// oled.ctrl = OLED_CTRL_SHOW_STRING;
							// oled.x = 66;
							// oled.y = 2;
							// oled.str = "------";
							// oled.font_size = 16;

							// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
							// 					&oled,		   /* ���͵���Ϣ���� */
							// 					1000);		   /* �ȴ�ʱ�� 100 Tick */
							// if (xReturn != pdPASS)
							// 	dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);

							// /* �����������ʾ�� */
							// oled.ctrl = OLED_CTRL_SHOW_STRING;
							// oled.x = 66;			
							// oled.y = 6;
							// oled.str = "------";
							// oled.font_size = 16;
							// xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
							// 					&oled,		   /* ���͵���Ϣ���� */
							// 					1000);		   /* �ȴ�ʱ�� 100 Tick */
							// if (xReturn != pdPASS)
							// 	dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);


							// /* ��ʾ����-������ */
							// oled.ctrl = OLED_CTRL_SHOW_CHINESE;
							// oled.x = 66;
							// oled.y = 4;
							// chn_tbl[0]=38;chn_tbl[1]=40;chn_tbl[2]=41;
						
							// for(i=0; i<3; i++)
							// {
							// 	oled.x = 66+i*16;
							// 	oled.chinese = chn_tbl[i];

							// 	xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
							// 						&oled,		   /* ���͵���Ϣ���� */
							// 						100);		   /* �ȴ�ʱ�� 100 Tick */
							// 	if (xReturn != pdPASS)
							// 		dgb_printf_safe("[menu_pass_man_fun] xQueueSend oled string error code is %d\r\n", xReturn);

							// }	

							/* ����Ҫ��ʾ��ͼƬ */
							LCD_SAFE
							(					
								lcd_draw_picture(96,60,48,48,gImage_auto_48x48);
								/* ��ʾ����-������ */
								lcd_show_chn(30,160,34,BLACK,WHITE,32);
								lcd_show_chn(62,160,35,BLACK,WHITE,32);
								lcd_show_chn(94,160,36,BLACK,WHITE,32);
								// /* ���������ʾ�� */
								// lcd_show_string(160,160,"------",BLACK,WHITE,32,0);

								/* ��ʾ����-������ */
								lcd_show_chn(30,200,37,BLACK,WHITE,32);
								lcd_show_chn(62,200,35,BLACK,WHITE,32);
								lcd_show_chn(94,200,36,BLACK,WHITE,32);

							);											

							/* �ָ���ֵ */
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
 * @brief �����������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 */
void app_task_kbd(void* pvParameters)
{
	char key_val='N';
	beep_t beep;
	BaseType_t xReturn;
	dgb_printf_safe("[app_task_kbd] create success\r\n");
	
	while(1)
	{
		/* ��ȡ������̰���ֵ */
		key_val=kbd_read();

		if(key_val != 'N')
		{
			dgb_printf_safe("[app_task_kbd] kbd press %c \r\n",key_val);

			xReturn = xQueueSend(g_queue_kbd, 	/* ��Ϣ���еľ�� */
								&key_val,		/* ���͵���Ϣ���� */
								100);		   	/* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_kbd] xQueueSend kbd error code is %d\r\n", xReturn);	

			/* ��һ��ʾ�� */
			beep.sta = 1;
			beep.duration = 20;

			xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
								&beep,		   /* ���͵���Ϣ���� */
								100);		   /* �ȴ�ʱ�� 100 Tick */				
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_kbd] xQueueSend beep error code is %d\r\n", xReturn);

			/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_system_reset] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);					

		}	
	}
}

/**
 * @brief usart����
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ͨ�����ڽ�������,��ʵ�ֶ�ʱ�����ڵ��޸�
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

		// ���ڽ�������
		xQueueReceive(g_queue_usart, buf, portMAX_DELAY);

		printf("%s\r\n", buf);

		// ����ʱ��
		if (g_rtc_get_what == FLAG_RTC_GET_TIME || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{
			// ��TIME SET-10-20-30#��
			if (strstr((const char *)buf, "TIME SET"))
			{

				p = strtok((char *)buf, "-");
				// Сʱ10
				p = strtok(NULL, "-");
				hours = atoi(p);
				// ����20
				p = strtok(NULL, "-");
				minutes = atoi(p);
				// ��30
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

		// ��������
		if (g_rtc_get_what == FLAG_RTC_GET_DATE || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{

			// ��DATE SET-2023-5-25-4#��
			if (strstr((const char *)buf, "DATE SET"))
			{
				p = strtok((char *)buf, "-");
				p = strtok(NULL, "-"); // 2023
				year = atoi(p);
				// ��ȡ�·�
				p = strtok(NULL, "-");
				month = atoi(p);

				// ��ȡ����
				p = strtok(NULL, "-");
				day = atoi(p);
				// printf("p=%s",p);

				// ��ȡ���ڼ�
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

		// ��������
		if (g_alarm_set == FLAG_ALARM_SET_START || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{
			// ��ALARM SET-10-20-30#��
			if (strstr((const char *)buf, "ALARM SET"))
			{

				// �ر�����
				RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

				p = strtok((char *)buf, "-");
				// Сʱ10
				p = strtok(NULL, "-");
				hours = atoi(p);
				// ����20
				p = strtok(NULL, "-");
				minutes = atoi(p);

				// ��30
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
				RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay; // �������ں����ڣ���������ÿ�춼��Ч

				/* ����RTC��A���ӣ�ע��RTC���������������ֱ�Ϊ����A������B */
				RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

				/* ��RTC������A����*/
				RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

				printf("alarm set ok!\r\n");

				if (g_alarm_set == FLAG_ALARM_SET_START)
					lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);

				// ��������ͼ��
				g_alarm_pic = 1;
			}
			if (strstr((const char *)buf, "ALARM OFF"))
			{
				printf("���ӹر�!\r\n");
				// �ر�����
				RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

				if (g_alarm_set == FLAG_ALARM_SET_START)
					lcd_draw_picture(70, 90, 100, 100, gImage_success_48x48);

				// Ϩ������ͼ��
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
 * @brief mpu6050����
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ͨ��dmp_get_pedometer_step_count��ȡ����,ͨ��mpu_dmp_get_data����ȡŷ����ʵ��̧������
 */
static void app_task_mpu6050(void* pvParameters)
{
	char buf[16] = {0};
	unsigned long  step_count_last=0;
	unsigned long  step_count=0;
	uint32_t sedentary_event=0;
	uint32_t t=0;
	float pitch, roll, yaw; /* ŷ���� */

	dgb_printf_safe("[app_task_mpu6050] create success and suspend self \r\n");

	//vTaskSuspend(NULL);

	dgb_printf_safe("mpu6050����������......r\n");

	while(mpu_dmp_init())
	{
		delay_ms(100);
	}
	
	/* ���ò�����ֵΪ0*/
	while(dmp_set_pedometer_step_count(0))
	{
		delay_ms(100);
	}

	for (;;)
	{

		/* ��ȡ���� */
		dmp_get_pedometer_step_count(&step_count);
		
		//dgb_printf_safe("mpu6050== %dr\n",step_count);
		//dgb_printf_safe("[INFO] ��ǰ����:%ld ��ǰ����:%ld\r\n",step_count,step_count_last);
		
		step_count_last=step_count;
		
		sprintf((char *)buf, ":%d",step_count);
		// dgb_printf_safe("%d\r\n",g_mpu6050_get_what);
		if (g_mpu6050_get_what == FLAG_MPU6050_GET_STEP)
		{
		
			/* ��ǰ���� */
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
		
			/* ��ָ��������û�����ߣ����������¼� */
			if(t>=100)
			{
				/* ��鲽���ı仯 */
				if((step_count - step_count_last) < 5)
				{
					/* �����仯���������þ����¼���־λΪ1 */
					sedentary_event=1;
				}
				
				dgb_printf_safe("[INFO] ��ǰ����:%ld ��ǰ����:%ld\r\n",step_count,step_count_last);
				
				step_count_last=step_count;
				
				t=0;
			}
			
			if(sedentary_event)
			{
				sedentary_event=0;
				
				dgb_printf_safe("[INFO] �������ã���վ��������...\r\n");
				
				/* ��ʾ�������� */
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
		
		/* ̧������ */
		if(0 == mpu_dmp_get_data(&pitch, &roll, &yaw))
		{
			/* ��ʱһ�� */
			delay_ms(200);
			
			if(roll > 15 && !g_system_display_on)
			{
				/* �����ٽ��� */
				taskENTER_CRITICAL();
				
				g_system_display_on=1;

				/* ����ͳ��ϵͳ�޲�������ֵ */
				g_system_no_opreation_cnt=0;

				/* �˳��ٽ��� */
				taskEXIT_CRITICAL();

				dgb_printf_safe("̧������\r\n");

				/* ����OLED�� */
				lcd_display_on(1);				
			}
		}
				
	}
}


/**
 * @brief max30102����
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ͨ���ɵ������ȡ�ĵ�ѹֵ������ת�̲˵��Ŀ���
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
		
		   ��ǰ100������ת�����洢���У��������400�������Ƶ�����
		*/
		
        for(i=100;i<500;i++)
        {
            aun_red_buffer[i-100]=aun_red_buffer[i];
            aun_ir_buffer[i-100]=aun_ir_buffer[i];
            
            /* update the signal min and max 
			   �����ź���Сֵ�����ֵ
			*/
			
            if(un_min>aun_red_buffer[i])
				un_min=aun_red_buffer[i];
			
            if(un_max<aun_red_buffer[i])
				un_max=aun_red_buffer[i];
        }
		
		/* take 100 sets of samples before calculating the heart rate 
		
		   �ڼ�������֮ǰ�ɼ�100������
		*/
		
        for(i=400;i<500;i++)
        {
            un_prev_data=aun_red_buffer[i-1];
			
            while(MAX30102_INT==1);
			
            max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
			
			/* ���ֵ�Ի��ʵ������ */
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
		
		/* �������ʺ�Ѫ�����Ͷ� */
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
		
		/* ͨ��UART�������ͼ��������͵��ն˳��� */
		if((ch_hr_valid == 1)&& (n_heart_rate>=60) && (n_heart_rate<100))
		{

			dgb_printf_safe("����=%d\r\n", n_heart_rate);
			
			sprintf(buff, ":%d", n_heart_rate);
			
			
			lcd_show_chn(60,160,13,BLACK,WHITE,32);
			lcd_show_chn(92,160,14,BLACK,WHITE,32);
			
			/* ��ʾ���� */
			lcd_show_string(130,160,buff,BLACK,WHITE,32,0);
			
			memset(buff, 0, sizeof buff);
		}

		
		if((ch_spo2_valid ==1)&& (n_sp02>=95) && (n_sp02<100))
		{
			dgb_printf_safe("Ѫ��Ũ��=%d\r\n", n_sp02);

			sprintf(bufff, ":%d", n_sp02);
			
			lcd_show_chn(30,200,15,BLACK,WHITE,32);
			lcd_show_chn(62,200,16,BLACK,WHITE,32);
			
			lcd_show_chn(94,200,21,BLACK,WHITE,32);
			lcd_show_chn(126,200,18,BLACK,WHITE,32);
			
			/* ��ʾѪ��Ũ�� */
			lcd_show_string(160,200,bufff,BLACK,WHITE,32,0);
			
			memset(bufff, 0, sizeof bufff);	
		}			
		
		delay_ms(100);
		
		
	}
}


/**
 * @brief ��������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 */
static void app_task_key(void *pvParameters)
{
	EventBits_t EventValue;

	dgb_printf_safe("[app_task_key] create success\r\n");

	for (;;)
	{
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_KEYALL_DOWN,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* ��ʱ���� */
		vTaskDelay(20);

		if (g_system_display_on == 0)
		{
			lcd_display_on(1);
			g_system_display_on = 1;
			continue;
		}

		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;

		if (EventValue & EVENT_GROUP_KEY1_DOWN)
		{
			/* ��ֹEXTI0�����ж� */
			NVIC_DisableIRQ(EXTI0_IRQn);

			/* ȷ���ǰ��� */
			if (PAin(0) == 0)
			{

				dgb_printf_safe("[app_task_key] S1 Press\r\n");

				/* ����KEY_UP�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_DOWN);
			}

			while (PAin(0) == 0)
				vTaskDelay(10);

			/* ����EXTI0�����ж� */
			NVIC_EnableIRQ(EXTI0_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY2_DOWN)
		{
			/* ��ֹEXTI2�����ж� */
			NVIC_DisableIRQ(EXTI2_IRQn);

			if (PEin(2) == 0)
			{
				dgb_printf_safe("[app_task_key] S2 Press\r\n");

				/* ����KEY_DOWN�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_UP);
			}

			while (PEin(2) == 0)
				vTaskDelay(10);

			/* ����EXTI2�����ж� */
			NVIC_EnableIRQ(EXTI2_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY3_DOWN)
		{
			/* ��ֹEXTI3�����ж� */
			NVIC_DisableIRQ(EXTI3_IRQn);

			if (PEin(3) == 0)
			{
				dgb_printf_safe("[app_task_key] S3 Press\r\n");

				/* ����KEY_ENTER�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
			}

			while (PEin(3) == 0)
				vTaskDelay(10);

			/* ����EXTI3�����ж� */
			NVIC_EnableIRQ(EXTI3_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY4_DOWN)
		{
			/* ��ֹEXTI4�����ж� */
			NVIC_DisableIRQ(EXTI4_IRQn);

			if (PEin(4) == 0)
			{

				dgb_printf_safe("[app_task_key] S4 Press\r\n");

				/* ����KEY_BACK�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);
			}

			while (PEin(4) == 0)
				vTaskDelay(10);

			/* ����EXTI4�����ж� */
			NVIC_EnableIRQ(EXTI4_IRQn);
		}
	}
}

/**
 * @brief adc����
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ͨ���ɵ������ȡ�ĵ�ѹֵ������ת�̲˵��Ŀ���
 */
static void app_task_adc(void *pvParameters)
{
	int32_t adc_vol_last = 0;
	int32_t adc_vol = 0;
	int32_t result;

	/* ��ȡ��ǰ��ѹ��ֵ */
	adc_vol = adc_voltage_get();
	adc_vol_last = adc_vol;

	for (;;)
	{
		/* ��ȡ��ѹֵ */
		adc_vol = adc_voltage_get();

		result = adc_vol - adc_vol_last;

		if (result > 200 || result < -200)
		{

			/* ����KEY_DOWN/KEY_UP�����¼� */
			xEventGroupSetBits(g_event_group, result > 0 ? EVENT_GROUP_FN_KEY_UP : EVENT_GROUP_FN_KEY_DOWN);

			dgb_printf_safe("[app_task_adc] adc_vol_last=%d adc_vol=%d\r\n", adc_vol_last, adc_vol);

			adc_vol_last = adc_vol;
		}

		vTaskDelay(200);
	}
}

/**
 * @brief ����������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ������Ϣ���п��Ʒ�������
 */
static void app_task_beep(void *pvParameters)
{
	beep_t beep;
	BaseType_t xReturn = pdFALSE;

	dgb_printf_safe("[app_task_beep] create success\r\n");

	for (;;)
	{
		xReturn = xQueueReceive(g_queue_beep,	/* ��Ϣ���еľ�� */
								&beep,			/* �õ�����Ϣ���� */
								portMAX_DELAY); /* �ȴ�ʱ��һֱ�� */
		if (xReturn != pdPASS)
			continue;

		dgb_printf_safe("[app_task_beep] beep.sta=%d beep.duration=%d\r\n", beep.sta, beep.duration);

		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;

		/* ���������Ƿ���Ҫ�������� */
		if (beep.duration)
		{
			BEEP(beep.sta);

			while (beep.duration--)
				vTaskDelay(10);

			/* ������״̬��ת */
			beep.sta ^= 1;
		}

		BEEP(beep.sta);

		/* �ͷ��ź��������߶Է�����ǰbeep���������Ѿ���� */
		xSemaphoreGive(g_sem_beep);
	}
}

/**
 * @brief ʵʱʱ������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ������Ϣ���п��Ʒ�������
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
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_RTC_WAKEUP | EVENT_GROUP_RTC_ALARM,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);		
		
		if (EventValue & EVENT_GROUP_RTC_WAKEUP)
		{

				/* RTC_GetTime����ȡʱ�� */
				RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

				memset(buf, 0, sizeof buf);

				/* ��ʽ���ַ��� */
				sprintf((char *)buf, "%02d:%02d:%02d", RTC_TimeStructure.RTC_Hours,
						RTC_TimeStructure.RTC_Minutes,
						RTC_TimeStructure.RTC_Seconds);
				
				lcd_draw_hour(RTC_TimeStructure.RTC_Hours);

				lcd_draw_seconds(RTC_TimeStructure.RTC_Seconds, ((LCD_WIDTH-strlen(buf)*32/2)/2)+65, 160);
				// lcd_draw_picture(105,65,30,60,gImage_maohao);
				lcd_draw_minutes(RTC_TimeStructure.RTC_Minutes);

				//dgb_printf_safe("[app_task_rtc����time] %s\r\n", buf);
				
				/* RTC_GetTime����ȡ���� */
				RTC_GetDate(RTC_Format_BCD, &RTC_DateStructure);

				memset(buf, 0, sizeof buf);
				
				memset(buffweekday, 0, sizeof buffweekday);

				/* ��ʽ����������ȡ���� */
				sprintf((char *)buf, "20%02x-%02x-%02x", RTC_DateStructure.RTC_Year,
						RTC_DateStructure.RTC_Month,
						RTC_DateStructure.RTC_Date);

				sprintf((char *)buffweekday, "%d", 
						RTC_DateStructure.RTC_WeekDay);
				
				
				/* ��ʾ���� */	
				x=(LCD_WIDTH-strlen(buf)*32/2)/2;
				
				LCD_SAFE(
				
				/* ��ʾ������ */
				lcd_show_string(x,125,buf,BLACK,WHITE,32,0);
				
				/* ��ʾ���� */
				lcd_show_chn(x,160,32,BLACK,WHITE,32);
				lcd_show_chn(x+30,160,33,BLACK,WHITE,32);
				lcd_show_string(x+60,160,buffweekday,BLACK,WHITE,32,0);
				);
				
				//dgb_printf_safe("[app_task_rtc����date] %s\r\n", buf);
				// ����ͼ��
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

				// ����ͼ��
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

				/* ��ʾ��������״̬ */
				// ble_connect_get = ble_connect_sta_get();
				// if(ble_connect_get == BLE_CONNECT)
				// {
				// 	//dgb_printf_safe("����������");
					
				// 	LCD_SAFE(
				// 	lcd_draw_picture(200,20,30,30,gImage_ble3_30x30);
				// 	);
				// }
		
				//dgb_printf_safe("ble_connect_get == %d\r\n", ble_connect_get);

				
		}

		if (EventValue & EVENT_GROUP_RTC_ALARM)
				{
					printf("�������ˣ�\n");

					for (i = 0; i < 5; i++)
					{
						/* ��һ��ʾ�� */
						beep.sta = 1;
						beep.duration = 10;

						xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
								&beep,		 /* ���͵���Ϣ���� */
								100);		 /* �ȴ�ʱ�� 100 Tick */
						delay_ms(500);
					}
				}
	}
}

/**
 * @brief ��ʪ������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ������Ϣ���п��Ʒ�������
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

		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;

		if (rt == 0)
		{
			memset(buf, 0, sizeof buf);

			if (g_dht_get_what == FLAG_DHT_GET_TEMP)
				sprintf((char *)buf, "Temp:%02d.%d", dht11_data[2], dht11_data[3]);

			if (g_dht_get_what == FLAG_DHT_GET_HUMI)
				sprintf((char *)buf, "Humi:%02d.%d", dht11_data[0], dht11_data[1]);

			/* ��ʾ�¶�/ʪ�� */
			x = (LCD_WIDTH - strlen(buf) * 32 / 2) / 2;
			lcd_show_string(x, 160, buf, BLACK, WHITE, 32, 0);

			dgb_printf_safe("[app_task_dht] %s\r\n", buf);
		}

		vTaskDelay(6000);
	}
}

/**
 * @brief �˵�����
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ��
 */
static void app_task_menu(void *pvParameters)
{
	EventBits_t EventValue;

	beep_t beep;

	uint32_t item_total;	  // ��¼��ǰ�˵��ж��ٸ���Ŀ
	uint32_t item_cursor = 0; // ��¼��ǰ�˵�ָ���ĸ���Ŀ
	uint32_t fun_run = 0;	  // ��¼�Ƿ�����Ŀ���ܺ�����ִ��

	menu_ext_t m_ext;

	menu_t *m;

	/* ͨ���������ݻ�ȡ���˵�ҳָ�� */
	menu_t **m_main = (menu_t **)pvParameters;

	uint16_t vs = 0;

	/* ����m_extָ��ĵ�ǰ�˵�����ز��� */
	m_ext.menu = *m_main;
	m_ext.key_fn = KEY_NONE;
	m_ext.item_cursor = item_cursor;
	m_ext.item_total = menu_item_total(m_ext.menu);

	/* ���ü�¼�˵�ָ��m */
	m = m_ext.menu;

	dgb_printf_safe("[app_task_menu] create success\r\n");

	for (;;)
	{
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_FN_KEY_UP | EVENT_GROUP_FN_KEY_DOWN | EVENT_GROUP_FN_KEY_ENTER | EVENT_GROUP_FN_KEY_BACK,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;

		/* ��һ��ʾ�� */
		beep.sta = 1;
		beep.duration = 1;

		xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
				   &beep,		 /* ���͵���Ϣ���� */
				   100);		 /* �ȴ�ʱ�� 100 Tick */

		if (EventValue & EVENT_GROUP_FN_KEY_UP)
		{

			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
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

				/* ��Ŀ�˵�ָ����һ���˵� */
				m--;

				/* ����RAM�Ĵ�ֱ������ʼ��ַ */
				if (item_cursor == 0)
				{
					while (vs)
					{
						vs--;
						LCD_SAFE(lcd_vs_set(vs));
						vTaskDelay(5);
					}
				}

				/* ��ʾ��� */
				LCD_SAFE(
					lcd_fill(230, (item_cursor + 1) * 60, 10, 50, WHITE);
					lcd_fill(230, item_cursor * 60, 10, 50, GREY););
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor3=%d\r\n", item_cursor);

				/* ��main�˵���ͬ�������˵� */
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

					/* ��ʾ�˵����� */
					menu_show(&m_ext);
				}
			}

			/* ���浱ǰ�������� */
			m_ext.key_fn = KEY_UP;

			/* ���浱ǰ��Ŀ���λ��ֵ */
			m_ext.item_cursor = item_cursor;
		}

		if (EventValue & EVENT_GROUP_FN_KEY_DOWN)
		{
			dgb_printf_safe("[app_task_menu] KEY_DOWN\r\n");

			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}

			/* ��ȡ��ǰ�˵��ж��ٸ���Ŀ */
			item_total = menu_item_total(m_ext.menu);

			/* ���浱ǰ�˵��ж��ٸ���Ŀ */
			m_ext.item_total = item_total;

			dgb_printf_safe("[app_task_menu] item_total=%d\r\n", item_total);

			if (item_cursor < (item_total - 1))
			{
				dgb_printf_safe("[app_task_menu] item_cursor4=%d\r\n", item_cursor);

				item_cursor++;

				/* ����RAM�Ĵ�ֱ������ʼ��ַ */
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

				/* ��ʾ��� */
				LCD_SAFE(
					lcd_fill(230, (item_cursor - 1) * 60, 10, 50, WHITE);
					lcd_fill(230, item_cursor * 60, 10, 50, GREY););

				/* ��Ŀ�˵�ָ����һ���˵� */
				m++;
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor6=%d\r\n", item_cursor);

				/* ��main�˵���ͬ�����Ҳ�˵� */

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

					/* ��ʾ�˵����� */
					menu_show(&m_ext);
				}
			}

			/* ���浱ǰ�������� */
			m_ext.key_fn = KEY_DOWN;

			/* ���浱ǰ��Ŀ���λ��ֵ */
			m_ext.item_cursor = item_cursor;
		}

		if (EventValue & EVENT_GROUP_FN_KEY_ENTER)
		{
			m_ext.key_fn = KEY_ENTER;

			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}

			dgb_printf_safe("[app_task_menu] KEY_ENTER item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_ENTER item name %s\r\n", m->item ? m->item : "NULL");

			m_ext.item_cursor = item_cursor;

			/* �Ӳ˵���Ч */
			if (m->child)
			{
				/* ָ���Ӳ˵� */
				m = m->child;

				/* ���浱ǰ�˵� */
				m_ext.menu = m;

				/* ��ʾ�˵����� */
				menu_show(&m_ext);

				/* ��λ���ֵ */
				item_cursor = 0;
			}

			/* ��û���Ӳ˵�,��ֱ��ִ�й��ܺ��� */
			if (!m->child && m->fun)
			{
				/* �������Ŀ���ܺ��������� */
				fun_run = 1;

				m->fun(&m_ext);
			}
		}

		if (EventValue & EVENT_GROUP_FN_KEY_BACK)
		{
			m_ext.key_fn = KEY_BACK;

			dgb_printf_safe("[app_task_menu] KEY_BACK item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_BACK item name %s\r\n", m->item ? m->item : "NULL");

			/* ���Ӳ˵����ܺ�����Ч����ִ�У���Ҫ�ǹ����Ӧ���� */
			if (m->fun)
			{
				/* �������Ŀ���ܺ��������� */
				fun_run = 1;

				m->fun(&m_ext);
			}

			/* ���˵���Ч */
			if (m->parent)
			{
				/* ָ�򸸲˵� */
				m = m->parent;

				/* ���浱ǰ�˵� */
				m_ext.menu = m;

				/* ��λ���ֵ */
				item_cursor = 0;
				m_ext.item_cursor = 0;

				dgb_printf_safe("[app_task_menu] m->parent item cursor=%d\r\n", item_cursor);
				dgb_printf_safe("[app_task_menu] m->parent item name %s\r\n", m->item ? m->item : "NULL");

				fun_run = 0;

				/* ��ʾ��ǰ�˵� */
				menu_show(&m_ext);
			}
		}
	}
}

/**
 * @brief �����ʱ������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ���������������Ź�ι��
 */
static void soft_timer_callback(TimerHandle_t pxTimer)
{

	/* ͳ��ϵͳ�޲�������ֵ�Լ�1 */
	g_system_no_opreation_cnt++;

	if (g_system_no_opreation_cnt >= 20)
	{
		/* Ϩ�� */
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
