/* ����:������ */
#include "includes.h"

/* �˵���� */
static menu_t menu_date_time[];
static menu_t menu_rtc_date[];
static menu_t menu_rtc_time[];


static menu_t menu_facid[];
static menu_t menu_touchid[];
static menu_t menu_pulse_so2[];

static menu_t menu_temp_humi[];
static menu_t menu_dht_temp[];
static menu_t menu_dht_humi[];


static menu_t menu_light[];
static menu_t menu_battery[];
static menu_t menu_camera[];
static menu_t menu_rfid[];
static menu_t menu_storage[];

static menu_t menu_blue_tooth[];//����
static menu_t menu_blue_tooth_data[];//�������� DATE SET-2025-10-30-1#
static menu_t menu_blue_tooth_time[];//����ʱ�� TIME SET-19-36-30#
static menu_t menu_blue_tooth_alarm[];//�������� ALARM SET-10-20-30#

static menu_t menu_mpu6050_date[];
static menu_t menu_mpu6050_step[];
static menu_t menu_mpu6050_redate[];

static menu_t menu_pass_man[];
static menu_t menu_pass_auth[];
static menu_t menu_pass_modify[];

static menu_t menu_rfid[];
static menu_t menu_rfid_add[];
static menu_t menu_rfid_delete[];

static void menu_dht_fun(void* pvParameters);
static void menu_rtc_fun(void* pvParameters);
static void menu_light_fun(void* pvParameters);
static void menu_battery_fun(void* pvParameters);
static void menu_pulse_so2_fun(void* pvParameters);
static void menu_facid_fun(void* pvParameters);
static void menu_touchid_fun(void* pvParameters);
static void menu_rfid_fun(void* pvParameters);
static void menu_camera_fun(void* pvParameters);
static void menu_storage_fun(void* pvParameters);
static void menu_mpu6050_fun(void* pvParameters);

static void menu_blue_tooth_fun(void* pvParameters);

static void menu_pass_man_fun(void* pvParameters);


static void menu_rfid_add_fun(void* pvParameters);
static void menu_rfid_delete_fun(void* pvParameters);

/*	һ���˵�
	1.��ҳ
	2.����ģ��
	3.�˶�ģ��
	4.����Ѫ��
	5.��ʪ��
*/
menu_t menu_main_1[]=
{
	/*  ����         ͼ��,             ����  ��   ��           ��  ��      */
	{"Home"			,gImage_home_48x48,NULL,NULL,menu_main_2,NULL,menu_rtc_date},
	{"Blue tooth"	,gImage_ble_48x48,NULL,NULL,menu_main_2,NULL,menu_blue_tooth},	
	{"Sport"		,gImage_sport_48x48,NULL,NULL,menu_main_2,NULL,menu_mpu6050_date/* menu_touchid*/},
	// {"Face ID "		,gImage_faceid_48x48,NULL,NULL,menu_main_2,NULL,NULL/* menu_facid */},
	// {"Touch ID"		,gImage_touchid_48x48,NULL,NULL,menu_main_2,NULL,NULL/* menu_touchid*/},
	{"Pulse SO2"	,gImage_pulse_48x48,NULL,NULL,menu_main_2,NULL,menu_pulse_so2},	
	{"DHT11"		,gImage_temp_humi_48x48,NULL,NULL,menu_main_2,NULL,menu_temp_humi},		
	{NULL,NULL,NULL,NULL,NULL,NULL},
};
/*	һ���˵�
	6.�������
	7.����
	8.����ͷ
	9.��Ƶʶ��
	10.�洢
*/
menu_t menu_main_2[]=
{
	/*  ����         ͼ��  						����   ��         ��   ��  	��      */
	// {"Light"		,gImage_light_48x48,		NULL,menu_main_1,NULL,NULL,NULL/*menu_light*/},	
	{"Password"		,gImage_password_48x48,		NULL,menu_main_1,NULL,NULL,menu_pass_man},
	{"Battery"		,gImage_battery_48x48,		NULL,menu_main_1,NULL,NULL,NULL/*menu_battery*/},
	{"Camera"		,gImage_camera_48x48,		NULL,menu_main_1,NULL,NULL,NULL/*menu_camera*/},
	// {"RFID"			,gImage_rfid_48x48,			NULL,NULL,menu_main_1,NULL,menu_rfid},	
	{"Storage"		,gImage_pic_storage_48x48,	NULL,menu_main_1,NULL,NULL,NULL/*menu_storage*/},	
	{NULL,NULL,NULL,NULL,NULL,NULL},
};


menu_t *menu_main_tbl[]={
	menu_main_1,
	menu_main_2,	
	NULL,
};

/*
	�����˵�
	[1] Auth
	[2] Modify
*/
static menu_t menu_pass_man[]=
{
	{"Auth"  ,gImage_auto_48x48,NULL,NULL,NULL,menu_main_2,menu_pass_auth},
	{"Modify",gImage_modify_48x48,NULL,NULL,NULL,menu_main_2,menu_pass_modify},
	{NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	�����˵�
	[1] add
	[2] delete
*/
static menu_t menu_rfid[]=
{
	{"add",NULL,	NULL,NULL,NULL,menu_main_2,menu_rfid_add},
	{"delete",NULL,NULL,NULL,NULL,menu_main_2,menu_rfid_delete},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	�����˵�
	[1] step
	[2] alert
*/
static menu_t menu_mpu6050_date[]=
{
	{"step",gImage_step_48x48,NULL,NULL,NULL,menu_main_1,menu_mpu6050_step},
	{"alert",gImage_revdate_48x48,NULL,NULL,NULL,menu_main_1,menu_mpu6050_redate},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};


/*
	�����˵�
	[1] Set Time
	[2] Set Date
	[3] Set Alarm
*/
static menu_t menu_blue_tooth[]=
{
	{"Set Time",gImage_time_48x48,NULL,NULL,NULL,menu_main_1,menu_blue_tooth_data},
	{"Set Date",gImage_date_48x48,NULL,NULL,NULL,menu_main_1,menu_blue_tooth_time},
	{"Set Alarm",gImage_alarm_48x48,NULL,NULL,NULL,menu_main_1,menu_blue_tooth_alarm},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};


/*
	�����˵�
	[1] Temp
	[2] Humi
*/
static menu_t menu_temp_humi[]=
{
	{"Temp",gImage_temperature_48x48,NULL,NULL,NULL,menu_main_1,menu_dht_temp},
	{"Humi",gImage_humidity_48x48,NULL,NULL,NULL,menu_main_1,menu_dht_humi},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};
/*
	�����˵�
	[1] Date
	[2] Time
*/
static menu_t menu_date_time[]=
{
	{"Date",gImage_date_48x48,NULL,NULL,NULL,menu_main_1,menu_rtc_date},
	{"Time",gImage_time_48x48,NULL,NULL,NULL,menu_main_1,menu_rtc_time},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};



/*
	�����˵�
	add,delete�����п���
*/
static menu_t menu_rfid_add[]=
{
	{"-",NULL,menu_rfid_add_fun,NULL,NULL,menu_rfid,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},
};

static menu_t menu_rfid_delete[]=
{
	{"-",NULL,menu_rfid_delete_fun,NULL,NULL,menu_rfid,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},
};

/*
	�����˵�
	auth,modify�����п���
*/

static menu_t menu_pass_auth[]=
{
	{"-",NULL,menu_pass_man_fun,NULL,NULL,menu_pass_man,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_pass_modify[]=
{
	{"-",NULL,menu_pass_man_fun,NULL,NULL,menu_pass_man,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};


/*
	�����˵�
	step��redate�����п���
*/

static menu_t menu_mpu6050_step[]=
{
	{"-",NULL,menu_mpu6050_fun,NULL,NULL,menu_mpu6050_date,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_mpu6050_redate[]=
{
	{"-",NULL,menu_mpu6050_fun,NULL,NULL,menu_mpu6050_date,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	�����˵�
	step��redate�����п���
*/
static menu_t menu_blue_tooth_alarm[]=
{
	{"-",NULL,menu_blue_tooth_fun,NULL,NULL,menu_blue_tooth,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	�����˵�
	date��time�����п���
*/
static menu_t menu_blue_tooth_data[]=
{
	{"-",NULL,menu_blue_tooth_fun,NULL,NULL,menu_blue_tooth,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_blue_tooth_time[]=
{
	{"-",NULL,menu_blue_tooth_fun,NULL,NULL,menu_blue_tooth,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	�����˵�
	pulse_so2���п���
*/
static menu_t menu_pulse_so2[]=
{
	{"-",NULL, menu_pulse_so2_fun ,NULL,NULL,menu_main_1,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	�����˵�
	date��time�����п���
*/
static menu_t menu_rtc_date[]=
{
	{"-",NULL,menu_rtc_fun,NULL,NULL,menu_main_1,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_rtc_time[]=
{
	{"-",NULL,menu_rtc_fun,NULL,NULL,menu_date_time,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	�����˵�
	temp��humi�����п���
*/
static menu_t menu_dht_temp[]=
{
	{"-",NULL,menu_dht_fun,NULL,NULL,menu_temp_humi,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_dht_humi[]=
{
	{"-",NULL,menu_dht_fun,NULL,NULL,menu_temp_humi,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};


static menu_t menu_light[]=
{
	{"-",NULL,menu_light_fun,NULL,NULL,menu_main_2,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};


static menu_t menu_battery[]=
{
	{"-",NULL,menu_light_fun,NULL,NULL,menu_main_2,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};





static menu_t menu_camera[]=
{
	{"-",NULL,menu_camera_fun,NULL,NULL,menu_main_2,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_storage[]=
{
	{"-",NULL,menu_storage_fun,NULL,NULL,menu_main_2,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/**
 * @brief ����rfid�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_rfid_add_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_rfid_add_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER )
	{
		g_pass_man_what = PASS_MAN_AUTH_RFID_ADD;
		/* ����Ҫ��ʾ��ͼƬ */
		// LCD_SAFE
		// (					
		// 	lcd_sprintf_hzstr32s((LCD_WIDTH-(strlen("���RFID")*16))/2,0,(uint8_t*)"���RFID",BLACK,WHITE);
		
		// );
	
	}	
	
	if(menu_ext->key_fn ==KEY_BACK)
	{
		g_pass_man_what = PASS_MAN_NONE;
	}
}

static void menu_rfid_delete_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_rfid_delete_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER )
	{
		g_pass_man_what = PASS_MAN_AUTH_RFID_DEL;
		/* ����Ҫ��ʾ��ͼƬ */
		// LCD_SAFE
		// (					
		// 	lcd_sprintf_hzstr32s((LCD_WIDTH-(strlen("ɾ��RFID")*16))/2,0,(uint8_t*)"ɾ��RFID",BLACK,WHITE);
		
		// );
	
	}	
	
	if(menu_ext->key_fn ==KEY_BACK)
	{
		g_pass_man_what = PASS_MAN_NONE;
	}
}


/**
 * @brief ����������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details .ʶ��ENTER��
 * 			.ʶ��BACK��
 */
static void menu_pass_man_fun(void* pvParameters)
{
	BaseType_t 	xReturn;
	char 		key_val='N';
	uint8_t		chn_tbl[8]={0};
	uint32_t	i;

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_pass_man_fun] ...\r\n");

	/* ʶ��ENTER�������� */
	if(menu_ext->key_fn == KEY_ENTER)
	{
		if(menu_ext->item_cursor == 0)
			g_pass_man_what = FLAG_PASS_MAN_AUTH;

		if(menu_ext->item_cursor == 1)
			g_pass_man_what = FLAG_PASS_MAN_MODIFY;

		if(g_pass_man_what!=FLAG_PASS_MAN_AUTH && g_pass_man_what!=FLAG_PASS_MAN_MODIFY)
			return;	

		if(g_pass_man_what == FLAG_PASS_MAN_AUTH)
		{
			/* ����Ҫ��ʾ��ͼƬ */
			LCD_SAFE
			(					
				lcd_draw_picture(96,60,48,48,gImage_auto_48x48);
				/* ��ʾ����-������ */
				lcd_show_chn(30,160,34,BLACK,WHITE,32);
				lcd_show_chn(62,160,35,BLACK,WHITE,32);
				lcd_show_chn(94,160,36,BLACK,WHITE,32);
				/* ���������ʾ�� */
				// lcd_show_string(160,160,"------",BLACK,WHITE,32,0);
			);

			/* ֪ͨ�����������λ�������� */
			key_val='C';
			xReturn = xQueueSend(g_queue_kbd, 	/* ��Ϣ���еľ�� */
								&key_val,		/* ���͵���Ϣ���� */
								100);		   	/* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[menu_pass_man_fun] xQueueSend kbd error code is %d\r\n", xReturn);					
		}

		if(g_pass_man_what == FLAG_PASS_MAN_MODIFY)
		{

			/* ����Ҫ��ʾ��ͼƬ */
			LCD_SAFE
			(					
				lcd_draw_picture(96,80,48,48,gImage_auto_48x48);
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


			/* ֪ͨ�����������λ�������� */
			key_val='C';
			xReturn = xQueueSend(g_queue_kbd, 	/* ��Ϣ���еľ�� */
								&key_val,		/* ���͵���Ϣ���� */
								100);		   	/* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[menu_pass_man_fun] xQueueSend kbd error code is %d\r\n", xReturn);
		}
		// vTaskResume(app_task_mpu6050_handle);
		/* �ָ������������ */
		vTaskResume(app_task_kbd_handle);		
		// vTaskResume(app_task_mpu6050_handle);
	}

	/* ʶ��BACK�������룬��ֹͣ��������������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		// g_pass_man_what = FLAG_PASS_MAN_NONE;
		/* �������������� */
		vTaskResume(app_task_mpu6050_handle);
	}	
}

/**
 * @brief mpu6050ģ��˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ��ʶ�𵽼Ʋ�,��g_mpu6050_get_what = FLAG_MPU6050_GET_STEP;
 * 			��ʶ�𵽽����������,��g_mpu6050_get_what = FLAG_MPU6050_GET_STOP;
 * 			��ʶ��KEY_BACK,��g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
	
 */
static void menu_mpu6050_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_sport_mpu6050] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		if(menu_ext->item_cursor == 0)
		{
			dgb_printf_safe("[menu_sport_mpu6050_step] \r\n");
			
			/* ����ǲ� */
			g_mpu6050_get_what = FLAG_MPU6050_GET_STEP;
			
			/* ����Ҫ��ʾ��ͼƬ */
			// LCD_SAFE
			// (	
			// 	// lcd_show_chn(60,0,45,WHITE,BLACK,32);
			// 	// lcd_show_chn(100,0,56,WHITE,BLACK,32);
			// 	// lcd_show_chn(140,0,43,WHITE,BLACK,32);
				
			
			// 	lcd_draw_picture(60,30,120,120,gImage_sport_48x48);
			
			// );
			lcd_draw_picture(90,80,48,48,gImage_step_48x48);
		}
			

		if(menu_ext->item_cursor == 1)
		{
			
			dgb_printf_safe("[menu_sport_mpu6050_date] \r\n");
			
			/* ����������� */
			g_mpu6050_get_what = FLAG_MPU6050_GET_STOP;
			
			/* ����Ҫ��ʾ��ͼƬ */
			// LCD_SAFE
			// (
			// 	// lcd_show_chn(60,0,41,WHITE,BLACK,32);
			// 	// lcd_show_chn(100,0,42,WHITE,BLACK,32);
			// 	// lcd_show_chn(140,0,43,WHITE,BLACK,32);
			
			// 	lcd_draw_picture(60,30,120,120,gImage_sport_48x48);
			// );
			lcd_draw_picture(90,80,48,48,gImage_revdate_48x48);
		}	
		vTaskResume(app_task_mpu6050_handle);
	}

	/* ʶ��BACK��������,��ֹͣmpu6050�������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* ����max30102���� */
		//vTaskSuspend(app_task_mpu6050_handle);
		
		g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
	}
}

/**
 * @brief ������������ģ��˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ��ʶ��ʱ������,��g_rtc_get_what = FLAG_RTC_GET_TIME;
 * 			��ʶ����������,��g_rtc_get_what = FLAG_RTC_GET_DATE;
 * 			��ʶ��KEY_BACK,��g_rtc_get_what = FLAG_RTC_GET_NONE;
 */
static void menu_blue_tooth_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
		
	//����
	LCD_SAFE(
		/* ���� */
		lcd_clear(WHITE);
		lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
		
	);
	
	if(menu_ext->key_fn==KEY_ENTER)
	{
		//ʱ������
		if(menu_ext->item_cursor == 0)
		{
			g_rtc_get_what = FLAG_RTC_GET_TIME;
			
			lcd_draw_picture(90,90,48,48,gImage_time_48x48);	
	
			
			
		}
		
		//��������
		if(menu_ext->item_cursor == 1)
		{
			g_rtc_get_what = FLAG_RTC_GET_DATE;
			
			lcd_draw_picture(90,90,48,48,gImage_date_48x48);	
	
		}

		//��������
		if(menu_ext->item_cursor == 2)
		{
			g_alarm_set=FLAG_ALARM_SET_START;
			
			lcd_draw_picture(90,90,48,48,gImage_alarm_48x48);	
	
		}
		
		
		if(g_rtc_get_what!=FLAG_RTC_GET_DATE && g_rtc_get_what!=FLAG_RTC_GET_TIME&&g_alarm_set!=FLAG_ALARM_SET_START)
			return;	

	}
	
	
	if(menu_ext->key_fn == KEY_BACK)
	{
		g_rtc_get_what=FLAG_RTC_GET_NONE;
		g_alarm_set=FLAG_ALARM_SET_NONE;
	}	
	
}


/**
 * @brief ������ʪ�Ȳ˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details .ʶ��ENTER��,�ָ���ʪ���������У�
 * 			.ʶ��BACK��,������ʪ������
 */
static void menu_dht_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	
	dgb_printf_safe("[menu_dht_fun] ...\r\n");
	
	dgb_printf_safe("[menu_dht_fun] key_fn=%d item_cursor=%d\r\n",menu_ext->key_fn,menu_ext->item_cursor );

	/* ʶ��ENTER��������,��ָ�DHT�������в���ȡ�¶�/ʪ�� */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		if(menu_ext->item_cursor == 0)
		{
			dgb_printf_safe("[menu_dht_fun] FLAG_DHT_GET_TEMP \r\n");
			g_dht_get_what = FLAG_DHT_GET_TEMP;
			
			/* ����Ҫ��ʾ��ͼƬ */
			LCD_SAFE
			(					
				lcd_draw_picture(96,80,48,48,gImage_temperature_48x48);
			
			);
		}
			

		if(menu_ext->item_cursor == 1)
		{
			g_dht_get_what = FLAG_DHT_GET_HUMI;
			
			dgb_printf_safe("[menu_dht_fun] FLAG_DHT_GET_HUMI \r\n");
			
			/* ����Ҫ��ʾ��ͼƬ */
			LCD_SAFE
			(
				lcd_draw_picture(96,80,48,48,gImage_humidity_48x48);
			);
		}
			

		if(g_dht_get_what!=FLAG_DHT_GET_TEMP && g_dht_get_what!=FLAG_DHT_GET_HUMI)
			return;			

		/* �ָ�DHT�������� */ 
		vTaskResume(app_task_dht_handle);
	}

	/* ʶ��BACK��������,��ֹͣDHT�������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* ����DHT���� */
		vTaskSuspend(app_task_dht_handle);
	}	
}
/**
 * @brief ����ƹ����ò˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_light_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_light_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */

		
	}

}
/**
 * @brief ����rfid�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_rfid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_rfid_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */
	
	
	
	}	
}
/**
 * @brief ����camera�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����	
 */
static void menu_camera_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_camera_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
	
	
	
	}	
}


/**
 * @brief ����rtc�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details .ʶ��ENTER��,����ݴ������Ŀ���λ��ָ����ȡ����ʱ�仹������,
 * 			 ���ָ�rtc��������У�
 * 			.ʶ��BACK��,�����rtc����
 */
static void menu_rtc_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_rtc_fun] ... \r\n");

	/* ʶ��ENTER��������,��ָ�RTC�������в���ȡ����/ʱ�� */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		// if(menu_ext->item_cursor == 0)
		// 	g_rtc_get_what = FLAG_RTC_GET_DATE;

		// if(menu_ext->item_cursor == 1)
		// 	g_rtc_get_what = FLAG_RTC_GET_TIME;

		// if(g_rtc_get_what!=FLAG_RTC_GET_DATE && g_rtc_get_what!=FLAG_RTC_GET_TIME)
		// 	return;

		// if(g_rtc_get_what == FLAG_RTC_GET_DATE)
		// {
		// 	/* ����Ҫ��ʾ��ͼƬ */	
		// 	LCD_SAFE
		// 	(
		// 		lcd_draw_picture(96,80,48,48,gImage_date_48x48);
			
		// 	);
		// }
		
		// if(g_rtc_get_what == FLAG_RTC_GET_TIME)
		// {
		// 	/* ����Ҫ��ʾ��ͼƬ */
		// 	LCD_SAFE
		// 	(			
		// 		lcd_draw_picture(96,80,48,48,gImage_time_48x48);				
		// 	);
		// }
		/* �ָ�RTC�������� */
		vTaskResume(app_task_rtc_handle);
	}

	/* ʶ��BACK��������,��ֹͣRTC�������� */	
	if(menu_ext->key_fn == KEY_BACK)
		vTaskSuspend(app_task_rtc_handle);
}


/**
 * @brief ����洢����˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����	
 */
static void menu_storage_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_storage_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */
	
	}
}

/**
 * @brief ��������ʶ��˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����	
 */
static void menu_facid_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;


	dgb_printf_safe("[menu_facid] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */


	}
}

/**
 * @brief ����ָ��ʶ��˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����	
 */
static void menu_touchid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_touchid] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */


	}
}
/**
 * @brief ��������Ѫ���˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ʶ��ENTER��,�ָ�����Ѫ����������,����mpu6050����
 * 			.ʶ��BACK��,��������Ѫ������,�ָ�mpu6050����	
 */
static void menu_pulse_so2_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	
	dgb_printf_safe("[menu_pulse_so2] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
			dgb_printf_safe("[menu_pluse_fun] \r\n");
			
			// /* ����Ҫ��ʾ��ͼƬ������ */
			// LCD_SAFE
			// (	
			// 	lcd_show_chn(60,0,41,WHITE,BLACK,32);
			// 	lcd_show_chn(100,0,42,WHITE,BLACK,32);
			// 	lcd_show_chn(140,0,43,WHITE,BLACK,32);
			
			//����Ѫ��ͼ��
			lcd_draw_picture(90,80,48,48,gImage_pulse_48x48);
			
			// 	lcd_draw_picture(60,30,120,120,gImage_pulse_120x120);
			
			// );	
		/* �ָ�max30102�������� */ 
		vTaskResume(app_task_max30102_handle);
		vTaskSuspend(app_task_mpu6050_handle);
	}

	/* ʶ��BACK��������,��ֹͣmax30102�������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* ����max30102���� */
		vTaskSuspend(app_task_max30102_handle);
		vTaskResume(app_task_mpu6050_handle);
	}	
		
}

/**
 * @brief ��ȡ��ǰ�˵�����Ŀ����
 * @param .menu ָ��ǰ�˵�
 * @retval ��ǰ�˵���Ŀ����
 * @details ��
 */
uint32_t menu_item_total(menu_t *menu)
{
	menu_t *m = menu;

	uint32_t item_count=0;

	while(m->item)
	{
		/* ָ����һ���˵� */
		m++;

		/* ͳ����Ŀ���� */
		item_count++;
	}
		
	return item_count;
}

/**
 * @brief ��ʾ�˵���Ŀ
 * @param .menu_ext ָ��ǰ�˵�
 * @retval ��ǰ�˵���Ŀ����
 * @details ��
 */
void menu_show(menu_ext_t *menu_ext)
{
	uint8_t  y=0;

	menu_ext_t *m_ext = menu_ext;
	menu_t *m = m_ext->menu;
		
	/* ��ʾ��ǰ�˵�������Ŀ���� */
	lcd_vs_set(0);	
	
	LCD_SAFE
	(
		lcd_clear(WHITE);
		lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
	);

	while(1)
	{
		/* ���˵���Ŀ������Ч�� */
		if(m->item == NULL || m->item[0]=='-' )
			break;		

		dgb_printf_safe("[menu_show] m->item is %s \r\n",m->item);

		LCD_SAFE
		(
			lcd_draw_picture(0,y,48,48,m->pic);
			lcd_show_string (60,y+6,m->item,BLACK,WHITE,32,0);	
		);
		
		
		/* ָ����һ���˵� */
		m++;

		/* y������һ�� */
		y+=60;
	}
	
	/* ӵ���Ӳ˵�,��ʾ��� */
	if(m->child)
	{
		m_ext->item_cursor = 0;		
	}	
	
	if(m->item[0]!='-')
	{
		LCD_SAFE
		(
			lcd_fill(230,0,10,50,GREY);	
		);	
	}


	dgb_printf_safe("[menu_show] end\r\n");
}


