#ifndef GLOBAL_H
#define GLOBAL_H

// 系统头文件
#include <Arduino.h>
#include <lvgl.h>
#include <FreeRTOS.h>
#include <semphr.h>

// 全局变量声明
extern lv_obj_t *scr_menu;
extern lv_style_t style_cn;

// 字体声明
LV_FONT_DECLARE(my_font_cn_16);

// 屏幕对象声明
extern lv_obj_t *scr_tower;
extern lv_obj_t *scr_beijing;
extern lv_obj_t *scr_music;
extern lv_obj_t *scr_reader;
extern lv_obj_t *scr_led;
extern lv_obj_t *scr_zongheng;

// 游戏状态变量
extern bool is_tower_started;

// SD 卡互斥锁
extern SemaphoreHandle_t sd_mutex;

// 主菜单函数
void build_main_menu();

// 塔楼游戏相关函数
void build_tower_scene();
void reset_tower_game();
void refresh_tower_ui();
void save_tower_game();
bool load_tower_game();
bool has_tower_save();

// 北京游戏相关函数
void build_beijing_scene();
void reset_beijing_game();
void refresh_beijing_ui();
void save_beijing_game();
bool load_beijing_game();
bool has_beijing_save();

// 音乐应用相关函数
void build_music_scene();

// 阅读器应用相关函数
void build_reader_scene();

// LED应用相关函数
void build_led_scene();

// 纵横游戏相关函数
void build_zongheng_scene();
void reset_zongheng_game();
void refresh_zongheng_ui();
void save_zongheng_game();
bool load_zongheng_game();
bool has_zongheng_save();

// 任务系统相关函数
void init_zh_quests();
void build_quest_ui(lv_obj_t *screen, lv_obj_t *parent_tab);
void refresh_quest_ui();
void process_quest_kill(int monster_id);

#endif