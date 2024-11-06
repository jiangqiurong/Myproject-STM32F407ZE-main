#ifndef __MENU_H__
#define __MENU_H__

typedef struct __menu_t
{
    const char *item;		//当前项目要显示的字符
	const uint8_t *pic;			//当前项目要显示的图标
    void(*fun)(void *);			//选择某一菜单后执行的功能函数
    struct __menu_t *same_left;	//当前项目的同级左侧菜单
    struct __menu_t *same_right;//当前项目的同级右侧菜单	
    struct __menu_t *parent;	//当前项目的父菜单	
    struct __menu_t *child;		//当前项目的子菜单
}menu_t;


 typedef struct __menu_ext_t
{
	menu_t 	*menu;
	uint32_t item_cursor;
	uint32_t item_total;
	key_fn_t key_fn;	
}menu_ext_t;

extern menu_t  *menu_main_tbl[];
extern menu_t 	menu_main_1[];
extern menu_t 	menu_main_2[];


extern uint32_t menu_item_total(menu_t *menu);
extern void 	menu_show(menu_ext_t *menu_ext);

#endif
