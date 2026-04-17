#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <lvgl.h>

extern lv_obj_t * scr_menu;
extern lv_style_t style_cn;

LV_FONT_DECLARE(my_font_cn_16);

extern lv_obj_t * scr_tower;
extern bool is_tower_started;
void build_main_menu();
void build_tower_scene();
void reset_tower_game();
void refresh_tower_ui();
void save_tower_game();
bool load_tower_game();
bool has_tower_save();

extern lv_obj_t * scr_beijing;
void build_beijing_scene();
void reset_beijing_game();
void refresh_beijing_ui();
void save_beijing_game();
bool load_beijing_game();
bool has_beijing_save();

extern lv_obj_t * scr_music;
void build_music_scene();

extern lv_obj_t * scr_reader;
void build_reader_scene();

extern lv_obj_t * scr_led;
void build_led_scene();

extern lv_obj_t * scr_zongheng;
void build_zongheng_scene();
void reset_zongheng_game();
void refresh_zongheng_ui();
void save_zongheng_game();
bool load_zongheng_game();
bool has_zongheng_save();

// --- 纵横四海：任务系统接口 ---
void init_zh_quests();
void build_quest_ui(lv_obj_t* screen, lv_obj_t* parent_tab);
void refresh_quest_ui();
void process_quest_kill(int monster_id);

#endif