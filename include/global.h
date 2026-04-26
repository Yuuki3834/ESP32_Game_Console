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
extern lv_obj_t *scr_music;
extern lv_obj_t *scr_reader;
extern lv_obj_t *scr_led;

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

// 音乐应用相关函数
void build_music_scene();

// 阅读器应用相关函数
void build_reader_scene();

// LED应用相关函数
void build_led_scene();

// Web LED控制相关函数
void initWebLEDControl();
void webLEDControlLoop();

// 任务系统相关函数
void init_zh_quests();
void build_quest_ui(lv_obj_t *screen, lv_obj_t *parent_tab);
void refresh_quest_ui();
void process_quest_kill(int monster_id);

// ==================== 网页游戏控制相关 ====================
// 游戏控制模式: 0=无, 1=LED控制, 2=魔塔
extern volatile uint8_t web_game_mode;
extern volatile bool web_control_active;

// 魔塔游戏控制函数
void tower_move(int dx, int dy);

#endif