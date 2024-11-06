/* 作者:江秋荣 */
#include "includes.h"

/* 菜单相关 */
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

static menu_t menu_blue_tooth[];//蓝牙
static menu_t menu_blue_tooth_data[];//设置日期 DATE SET-2025-10-30-1#
static menu_t menu_blue_tooth_time[];//设置时间 TIME SET-19-36-30#
static menu_t menu_blue_tooth_alarm[];//设置闹钟 ALARM SET-10-20-30#

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

/*	一级菜单
	1.主页
	2.蓝牙模块
	3.运动模块
	4.心率血氧
	5.温湿度
*/
menu_t menu_main_1[]=
{
	/*  名字         图标,             函数  左   右           父  子      */
	{"Home"			,gImage_home_48x48,NULL,NULL,menu_main_2,NULL,menu_rtc_date},
	{"Blue tooth"	,gImage_ble_48x48,NULL,NULL,menu_main_2,NULL,menu_blue_tooth},	
	{"Sport"		,gImage_sport_48x48,NULL,NULL,menu_main_2,NULL,menu_mpu6050_date/* menu_touchid*/},
	// {"Face ID "		,gImage_faceid_48x48,NULL,NULL,menu_main_2,NULL,NULL/* menu_facid */},
	// {"Touch ID"		,gImage_touchid_48x48,NULL,NULL,menu_main_2,NULL,NULL/* menu_touchid*/},
	{"Pulse SO2"	,gImage_pulse_48x48,NULL,NULL,menu_main_2,NULL,menu_pulse_so2},	
	{"DHT11"		,gImage_temp_humi_48x48,NULL,NULL,menu_main_2,NULL,menu_temp_humi},		
	{NULL,NULL,NULL,NULL,NULL,NULL},
};
/*	一级菜单
	6.密码管理
	7.电量
	8.摄像头
	9.射频识别
	10.存储
*/
menu_t menu_main_2[]=
{
	/*  名字         图标  						函数   左         右   父  	子      */
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
	二级菜单
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
	二级菜单
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
	二级菜单
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
	二级菜单
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
	二级菜单
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
	二级菜单
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
	三级菜单
	add,delete等运行控制
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
	三级菜单
	auth,modify等运行控制
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
	三级菜单
	step、redate等运行控制
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
	三级菜单
	step、redate等运行控制
*/
static menu_t menu_blue_tooth_alarm[]=
{
	{"-",NULL,menu_blue_tooth_fun,NULL,NULL,menu_blue_tooth,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	三级菜单
	date、time等运行控制
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
	三级菜单
	pulse_so2运行控制
*/
static menu_t menu_pulse_so2[]=
{
	{"-",NULL, menu_pulse_so2_fun ,NULL,NULL,menu_main_1,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/*
	三级菜单
	date、time等运行控制
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
	三级菜单
	temp、humi等运行控制
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
 * @brief 进入rfid菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_rfid_add_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_rfid_add_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER )
	{
		g_pass_man_what = PASS_MAN_AUTH_RFID_ADD;
		/* 设置要显示的图片 */
		// LCD_SAFE
		// (					
		// 	lcd_sprintf_hzstr32s((LCD_WIDTH-(strlen("添加RFID")*16))/2,0,(uint8_t*)"添加RFID",BLACK,WHITE);
		
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

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER )
	{
		g_pass_man_what = PASS_MAN_AUTH_RFID_DEL;
		/* 设置要显示的图片 */
		// LCD_SAFE
		// (					
		// 	lcd_sprintf_hzstr32s((LCD_WIDTH-(strlen("删除RFID")*16))/2,0,(uint8_t*)"删除RFID",BLACK,WHITE);
		
		// );
	
	}	
	
	if(menu_ext->key_fn ==KEY_BACK)
	{
		g_pass_man_what = PASS_MAN_NONE;
	}
}


/**
 * @brief 密码管理界面
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details .识别到ENTER键
 * 			.识别到BACK键
 */
static void menu_pass_man_fun(void* pvParameters)
{
	BaseType_t 	xReturn;
	char 		key_val='N';
	uint8_t		chn_tbl[8]={0};
	uint32_t	i;

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_pass_man_fun] ...\r\n");

	/* 识别到ENTER按键编码 */
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
			/* 设置要显示的图片 */
			LCD_SAFE
			(					
				lcd_draw_picture(96,60,48,48,gImage_auto_48x48);
				/* 显示汉字-旧密码 */
				lcd_show_chn(30,160,34,BLACK,WHITE,32);
				lcd_show_chn(62,160,35,BLACK,WHITE,32);
				lcd_show_chn(94,160,36,BLACK,WHITE,32);
				/* 清空密码显示区 */
				// lcd_show_string(160,160,"------",BLACK,WHITE,32,0);
			);

			/* 通知矩阵键盘任务复位操作内容 */
			key_val='C';
			xReturn = xQueueSend(g_queue_kbd, 	/* 消息队列的句柄 */
								&key_val,		/* 发送的消息内容 */
								100);		   	/* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[menu_pass_man_fun] xQueueSend kbd error code is %d\r\n", xReturn);					
		}

		if(g_pass_man_what == FLAG_PASS_MAN_MODIFY)
		{

			/* 设置要显示的图片 */
			LCD_SAFE
			(					
				lcd_draw_picture(96,80,48,48,gImage_auto_48x48);
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


			/* 通知矩阵键盘任务复位操作内容 */
			key_val='C';
			xReturn = xQueueSend(g_queue_kbd, 	/* 消息队列的句柄 */
								&key_val,		/* 发送的消息内容 */
								100);		   	/* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[menu_pass_man_fun] xQueueSend kbd error code is %d\r\n", xReturn);
		}
		// vTaskResume(app_task_mpu6050_handle);
		/* 恢复矩阵键盘任务 */
		vTaskResume(app_task_kbd_handle);		
		// vTaskResume(app_task_mpu6050_handle);
	}

	/* 识别到BACK按键编码，则停止矩阵键盘任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		// g_pass_man_what = FLAG_PASS_MAN_NONE;
		/* 挂起矩阵键盘任务 */
		vTaskResume(app_task_mpu6050_handle);
	}	
}

/**
 * @brief mpu6050模块菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 当识别到计步,则g_mpu6050_get_what = FLAG_MPU6050_GET_STEP;
 * 			当识别到进入久坐提醒,则g_mpu6050_get_what = FLAG_MPU6050_GET_STOP;
 * 			当识别到KEY_BACK,则g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
	
 */
static void menu_mpu6050_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_sport_mpu6050] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		if(menu_ext->item_cursor == 0)
		{
			dgb_printf_safe("[menu_sport_mpu6050_step] \r\n");
			
			/* 进入记步 */
			g_mpu6050_get_what = FLAG_MPU6050_GET_STEP;
			
			/* 设置要显示的图片 */
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
			
			/* 进入久坐提醒 */
			g_mpu6050_get_what = FLAG_MPU6050_GET_STOP;
			
			/* 设置要显示的图片 */
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

	/* 识别到BACK按键编码,则停止mpu6050任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* 挂起max30102任务 */
		//vTaskSuspend(app_task_mpu6050_handle);
		
		g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
	}
}

/**
 * @brief 进入蓝牙连接模块菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 当识别到时间设置,则g_rtc_get_what = FLAG_RTC_GET_TIME;
 * 			当识别到日期设置,则g_rtc_get_what = FLAG_RTC_GET_DATE;
 * 			当识别到KEY_BACK,则g_rtc_get_what = FLAG_RTC_GET_NONE;
 */
static void menu_blue_tooth_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
		
	//清屏
	LCD_SAFE(
		/* 清屏 */
		lcd_clear(WHITE);
		lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
		
	);
	
	if(menu_ext->key_fn==KEY_ENTER)
	{
		//时间设置
		if(menu_ext->item_cursor == 0)
		{
			g_rtc_get_what = FLAG_RTC_GET_TIME;
			
			lcd_draw_picture(90,90,48,48,gImage_time_48x48);	
	
			
			
		}
		
		//日期设置
		if(menu_ext->item_cursor == 1)
		{
			g_rtc_get_what = FLAG_RTC_GET_DATE;
			
			lcd_draw_picture(90,90,48,48,gImage_date_48x48);	
	
		}

		//闹钟设置
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
 * @brief 进入温湿度菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details .识别到ENTER键,恢复温湿度任务运行；
 * 			.识别到BACK键,挂起温湿度任务
 */
static void menu_dht_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	
	dgb_printf_safe("[menu_dht_fun] ...\r\n");
	
	dgb_printf_safe("[menu_dht_fun] key_fn=%d item_cursor=%d\r\n",menu_ext->key_fn,menu_ext->item_cursor );

	/* 识别到ENTER按键编码,则恢复DHT任务运行并获取温度/湿度 */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		if(menu_ext->item_cursor == 0)
		{
			dgb_printf_safe("[menu_dht_fun] FLAG_DHT_GET_TEMP \r\n");
			g_dht_get_what = FLAG_DHT_GET_TEMP;
			
			/* 设置要显示的图片 */
			LCD_SAFE
			(					
				lcd_draw_picture(96,80,48,48,gImage_temperature_48x48);
			
			);
		}
			

		if(menu_ext->item_cursor == 1)
		{
			g_dht_get_what = FLAG_DHT_GET_HUMI;
			
			dgb_printf_safe("[menu_dht_fun] FLAG_DHT_GET_HUMI \r\n");
			
			/* 设置要显示的图片 */
			LCD_SAFE
			(
				lcd_draw_picture(96,80,48,48,gImage_humidity_48x48);
			);
		}
			

		if(g_dht_get_what!=FLAG_DHT_GET_TEMP && g_dht_get_what!=FLAG_DHT_GET_HUMI)
			return;			

		/* 恢复DHT任务运行 */ 
		vTaskResume(app_task_dht_handle);
	}

	/* 识别到BACK按键编码,则停止DHT任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* 挂起DHT任务 */
		vTaskSuspend(app_task_dht_handle);
	}	
}
/**
 * @brief 进入灯光设置菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_light_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_light_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */

		
	}

}
/**
 * @brief 进入rfid菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_rfid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_rfid_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */
	
	
	
	}	
}
/**
 * @brief 进入camera菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无	
 */
static void menu_camera_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_camera_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
	
	
	
	}	
}


/**
 * @brief 进入rtc菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details .识别到ENTER键,则根据传入的项目光标位置指定获取的是时间还是日期,
 * 			 并恢复rtc任务的运行；
 * 			.识别到BACK键,则挂起rtc任务。
 */
static void menu_rtc_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_rtc_fun] ... \r\n");

	/* 识别到ENTER按键编码,则恢复RTC任务运行并获取日期/时间 */	
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
		// 	/* 设置要显示的图片 */	
		// 	LCD_SAFE
		// 	(
		// 		lcd_draw_picture(96,80,48,48,gImage_date_48x48);
			
		// 	);
		// }
		
		// if(g_rtc_get_what == FLAG_RTC_GET_TIME)
		// {
		// 	/* 设置要显示的图片 */
		// 	LCD_SAFE
		// 	(			
		// 		lcd_draw_picture(96,80,48,48,gImage_time_48x48);				
		// 	);
		// }
		/* 恢复RTC任务运行 */
		vTaskResume(app_task_rtc_handle);
	}

	/* 识别到BACK按键编码,则停止RTC任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
		vTaskSuspend(app_task_rtc_handle);
}


/**
 * @brief 进入存储管理菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无	
 */
static void menu_storage_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_storage_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */
	
	}
}

/**
 * @brief 进入人脸识别菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无	
 */
static void menu_facid_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;


	dgb_printf_safe("[menu_facid] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */


	}
}

/**
 * @brief 进入指纹识别菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无	
 */
static void menu_touchid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_touchid] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */


	}
}
/**
 * @brief 进入心率血氧菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 识别到ENTER键,恢复心率血氧任务运行,挂起mpu6050任务；
 * 			.识别到BACK键,挂起心率血氧任务,恢复mpu6050任务。	
 */
static void menu_pulse_so2_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	
	dgb_printf_safe("[menu_pulse_so2] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
			dgb_printf_safe("[menu_pluse_fun] \r\n");
			
			// /* 设置要显示的图片与文字 */
			// LCD_SAFE
			// (	
			// 	lcd_show_chn(60,0,41,WHITE,BLACK,32);
			// 	lcd_show_chn(100,0,42,WHITE,BLACK,32);
			// 	lcd_show_chn(140,0,43,WHITE,BLACK,32);
			
			//心率血氧图标
			lcd_draw_picture(90,80,48,48,gImage_pulse_48x48);
			
			// 	lcd_draw_picture(60,30,120,120,gImage_pulse_120x120);
			
			// );	
		/* 恢复max30102任务运行 */ 
		vTaskResume(app_task_max30102_handle);
		vTaskSuspend(app_task_mpu6050_handle);
	}

	/* 识别到BACK按键编码,则停止max30102任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* 挂起max30102任务 */
		vTaskSuspend(app_task_max30102_handle);
		vTaskResume(app_task_mpu6050_handle);
	}	
		
}

/**
 * @brief 获取当前菜单的项目总数
 * @param .menu 指向当前菜单
 * @retval 当前菜单项目总数
 * @details 无
 */
uint32_t menu_item_total(menu_t *menu)
{
	menu_t *m = menu;

	uint32_t item_count=0;

	while(m->item)
	{
		/* 指向下一个菜单 */
		m++;

		/* 统计项目数量 */
		item_count++;
	}
		
	return item_count;
}

/**
 * @brief 显示菜单项目
 * @param .menu_ext 指向当前菜单
 * @retval 当前菜单项目总数
 * @details 无
 */
void menu_show(menu_ext_t *menu_ext)
{
	uint8_t  y=0;

	menu_ext_t *m_ext = menu_ext;
	menu_t *m = m_ext->menu;
		
	/* 显示当前菜单所有项目名称 */
	lcd_vs_set(0);	
	
	LCD_SAFE
	(
		lcd_clear(WHITE);
		lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
	);

	while(1)
	{
		/* 检查菜单项目标题有效性 */
		if(m->item == NULL || m->item[0]=='-' )
			break;		

		dgb_printf_safe("[menu_show] m->item is %s \r\n",m->item);

		LCD_SAFE
		(
			lcd_draw_picture(0,y,48,48,m->pic);
			lcd_show_string (60,y+6,m->item,BLACK,WHITE,32,0);	
		);
		
		
		/* 指向下一个菜单 */
		m++;

		/* y坐标下一行 */
		y+=60;
	}
	
	/* 拥有子菜单,显示光标 */
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


