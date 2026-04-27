#include "global.h"
#include <stdio.h>
#include <string.h>

// =========================================================================
// 前向声明
// =========================================================================
extern int sailing_target_id;
extern int sailing_origin_id;
extern void spawn_monsters_for_loc(int loc_id);
extern const ZH_Location* get_current_loc();
extern void check_levelup();
extern void process_port_switch();
extern void refresh_skill_cfg_list();
extern void refresh_adjutant_cfg_list();
extern void open_forge_ui();

// =========================================================================
// 外部依赖声明
// =========================================================================
extern ZH_Player zh_player;
extern int current_bag_filter;
extern int selected_inventory_idx;

extern lv_obj_t * scr_zongheng;
extern lv_obj_t * lbl_zh_top_status;
extern lv_obj_t * lbl_zh_log;
extern lv_obj_t * lbl_zh_detail_status;
extern lv_obj_t * tab_zh_move;
extern lv_obj_t * tab_zh_action;
extern lv_obj_t * tab_zh_bag;
extern lv_obj_t * tab_zh_status;
extern lv_obj_t * tab_zh_quest;
extern lv_obj_t * list_zh_move;
extern lv_obj_t * list_zh_action;
extern lv_obj_t * list_zh_bag;
extern lv_obj_t * modal_zh_menu;
extern lv_obj_t * modal_zh_map;
extern lv_obj_t * list_map_locs;
extern lv_obj_t * modal_sailing;
extern lv_obj_t * lbl_sailing_info;
extern lv_obj_t * modal_npc;
extern lv_obj_t * lbl_npc_name;
extern lv_obj_t * lbl_npc_dialogue;
extern lv_obj_t * cont_npc_actions;
extern lv_obj_t * modal_item_detail;
extern lv_obj_t * lbl_item_detail_title;
extern lv_obj_t * lbl_item_detail_desc;
extern lv_obj_t * modal_zh_skill_cfg;
extern lv_obj_t * list_skill_slots;
extern lv_obj_t * modal_zh_skill_select;
extern lv_obj_t * list_learned_skills;
extern lv_obj_t * modal_adjutant;
extern lv_obj_t * list_adj_slots;
extern lv_obj_t * list_adj_roster;
extern lv_obj_t * modal_forge;
extern lv_obj_t * list_forge_recipes;

extern void zh_log(const char * msg);
extern void refresh_zongheng_ui();
extern void reset_zongheng_game();
extern void save_zongheng_game();
extern void reset_combat_ui_pointers();
extern void reset_market_ui_pointers();
extern void reset_quest_ui_pointers();
extern void reset_zongheng_ui_pointers();
extern void populate_map_list();
extern void refresh_skill_cfg_list();
extern void refresh_adjutant_cfg_list();
extern void build_market_and_shop_ui(lv_obj_t * parent);
extern void build_quest_ui(lv_obj_t * parent, lv_obj_t * tab);
extern lv_style_t style_cn;

// =========================================================================
// 场景与界面构建逻辑
// =========================================================================
void build_zongheng_scene() {
    if (scr_zongheng) {
        lv_obj_del_async(scr_zongheng);
        scr_zongheng = NULL;
    }
    
    modal_npc = NULL;
    modal_item_detail = NULL;
    modal_zh_skill_cfg = NULL;
    modal_zh_skill_select = NULL;
    modal_adjutant = NULL;
    modal_forge = NULL;
    list_forge_recipes = NULL;
    list_adj_slots = NULL;
    list_adj_roster = NULL;

    reset_combat_ui_pointers();
    reset_market_ui_pointers();
    reset_quest_ui_pointers();
    
    scr_zongheng = lv_obj_create(NULL); 
    if (!scr_zongheng) { zh_log("错误：无法创建主场景对象！"); return; }
    lv_obj_set_style_bg_color(scr_zongheng, lv_color_hex(0x1a252f), 0);
    
    lbl_zh_top_status = lv_label_create(scr_zongheng); 
    if (!lbl_zh_top_status) { lv_obj_del_async(scr_zongheng); scr_zongheng = NULL; return; }
    lv_obj_add_style(lbl_zh_top_status, &style_cn, 0); 
    lv_obj_set_width(lbl_zh_top_status, 150); 
    lv_obj_set_style_text_color(lbl_zh_top_status, lv_color_hex(0xF1C40F), 0); 
    lv_obj_align(lbl_zh_top_status, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t * btn_menu = lv_btn_create(scr_zongheng); 
    lv_obj_set_size(btn_menu, 50, 30); 
    lv_obj_align(btn_menu, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_bg_color(btn_menu, lv_color_hex(0x34495e), 0); 
    lv_obj_t * lbl_menu = lv_label_create(btn_menu);
    lv_obj_add_style(lbl_menu, &style_cn, 0); 
    lv_label_set_text(lbl_menu, "菜单"); 
    lv_obj_center(lbl_menu);
    lv_obj_add_event_cb(btn_menu, [](lv_event_t *e){ 
        lv_obj_clear_flag(modal_zh_menu, LV_OBJ_FLAG_HIDDEN); 
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_map = lv_btn_create(scr_zongheng); 
    lv_obj_set_size(btn_map, 50, 30); 
    lv_obj_align(btn_map, LV_ALIGN_TOP_RIGHT, -60, 5);
    lv_obj_set_style_bg_color(btn_map, lv_color_hex(0x2980b9), 0); 
    lv_obj_t * lbl_map = lv_label_create(btn_map);
    lv_obj_add_style(lbl_map, &style_cn, 0); 
    lv_label_set_text(lbl_map, "地图"); 
    lv_obj_center(lbl_map);
    lv_obj_add_event_cb(btn_map, [](lv_event_t *e){ 
        populate_map_list(); 
        lv_obj_clear_flag(modal_zh_map, LV_OBJ_FLAG_HIDDEN); 
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * log_cont = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(log_cont, 230, 110); 
    lv_obj_align(log_cont, LV_ALIGN_TOP_MID, 0, 45); 
    lv_obj_set_style_bg_color(log_cont, lv_color_hex(0x2c3e50), 0); 
    lv_obj_set_style_border_width(log_cont, 0, 0);
    lbl_zh_log = lv_label_create(log_cont); 
    lv_obj_set_width(lbl_zh_log, 210); 
    lv_obj_add_style(lbl_zh_log, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_zh_log, lv_color_hex(0xecf0f1), 0); 
    lv_label_set_text(lbl_zh_log, "欢迎来到大航海时代！");

    lv_obj_t * tv = lv_tabview_create(scr_zongheng, LV_DIR_TOP, 35); 
    lv_obj_set_size(tv, 240, 165); 
    lv_obj_align(tv, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_t * tv_cont = lv_tabview_get_content(tv); 
    lv_obj_set_style_bg_color(tv_cont, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_bg_opa(tv_cont, LV_OPA_COVER, 0);
    lv_obj_t * tv_btns = lv_tabview_get_tab_btns(tv); 
    lv_obj_add_style(tv_btns, &style_cn, 0); 
    lv_obj_set_style_bg_color(tv_btns, lv_color_hex(0x222222), 0); 
    lv_obj_set_style_text_color(tv_btns, lv_color_hex(0xCCCCCC), 0); 
    lv_obj_set_style_pad_left(tv_btns, 0, 0); 
    lv_obj_set_style_pad_right(tv_btns, 0, 0);

    tab_zh_move = lv_tabview_add_tab(tv, "移动"); 
    tab_zh_action = lv_tabview_add_tab(tv, "行动"); 
    tab_zh_status = lv_tabview_add_tab(tv, "状态"); 
    tab_zh_bag = lv_tabview_add_tab(tv, "背包");
    tab_zh_quest = lv_tabview_add_tab(tv, "任务");

    list_zh_move = lv_list_create(tab_zh_move); 
    lv_obj_set_size(list_zh_move, 240, 130); 
    lv_obj_align(list_zh_move, LV_ALIGN_TOP_LEFT, -15, -15); 
    lv_obj_set_style_bg_color(list_zh_move, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_border_width(list_zh_move, 0, 0); 
    
    list_zh_action = lv_list_create(tab_zh_action); 
    lv_obj_set_size(list_zh_action, 240, 130); 
    lv_obj_align(list_zh_action, LV_ALIGN_TOP_LEFT, -15, -15); 
    lv_obj_set_style_bg_color(list_zh_action, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_border_width(list_zh_action, 0, 0); 
    
    list_zh_bag = lv_list_create(tab_zh_bag); 
    lv_obj_set_size(list_zh_bag, 240, 130); 
    lv_obj_align(list_zh_bag, LV_ALIGN_TOP_LEFT, -15, -15); 
    lv_obj_set_style_bg_color(list_zh_bag, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_border_width(list_zh_bag, 0, 0); 

    lv_obj_set_style_bg_color(tab_zh_status, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_border_width(tab_zh_status, 0, 0); 
    lv_obj_set_style_pad_all(tab_zh_status, 10, 0);
    lbl_zh_detail_status = lv_label_create(tab_zh_status); 
    lv_obj_add_style(lbl_zh_detail_status, &style_cn, 0); 
    lv_obj_set_style_text_color(lbl_zh_detail_status, lv_color_hex(0xecf0f1), 0);

    // 主菜单
    modal_zh_menu = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_zh_menu, 180, 270); 
    lv_obj_center(modal_zh_menu);
    lv_obj_set_style_bg_color(modal_zh_menu, lv_color_hex(0x111111), 0); 
    lv_obj_add_flag(modal_zh_menu, LV_OBJ_FLAG_HIDDEN);
    
    const char* m_opts[] = {"继续游戏", "配置技能", "副官任命", "保存进度", "重新开始", "退出游戏"};
    for(int i = 0; i < 6; i++) {
        lv_obj_t * b = lv_btn_create(modal_zh_menu); 
        lv_obj_set_size(b, 150, 36); 
        lv_obj_align(b, LV_ALIGN_TOP_MID, 0, i * 40); 
        lv_obj_set_style_bg_color(b, lv_color_hex(0x34495e), 0);
        lv_obj_t * l = lv_label_create(b); 
        lv_obj_add_style(l, &style_cn, 0); 
        lv_label_set_text(l, m_opts[i]); 
        lv_obj_center(l);
        lv_obj_add_event_cb(b, [](lv_event_t *e){ 
            int id = (int)(intptr_t)lv_event_get_user_data(e); 
            lv_obj_add_flag(modal_zh_menu, LV_OBJ_FLAG_HIDDEN); 
            if(id == 1) { refresh_skill_cfg_list(); lv_obj_clear_flag(modal_zh_skill_cfg, LV_OBJ_FLAG_HIDDEN); }
            if(id == 2) { refresh_adjutant_cfg_list(); lv_obj_clear_flag(modal_adjutant, LV_OBJ_FLAG_HIDDEN); }
            if(id == 3) save_zongheng_game(); 
            if(id == 4) reset_zongheng_game(); 
            if(id == 5) {
                save_zongheng_game();
                lv_scr_load(scr_menu);
                lv_obj_del_async(scr_zongheng);
                reset_combat_ui_pointers();
                reset_market_ui_pointers();
                reset_quest_ui_pointers();
                reset_zongheng_ui_pointers();
            }
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    // 技能配置 UI
    modal_zh_skill_cfg = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_zh_skill_cfg, 220, 280); 
    lv_obj_center(modal_zh_skill_cfg);
    lv_obj_add_flag(modal_zh_skill_cfg, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_zh_skill_cfg, lv_color_hex(0x2c3e50), 0);
    lv_obj_t * lbl_sc_title = lv_label_create(modal_zh_skill_cfg); 
    lv_obj_add_style(lbl_sc_title, &style_cn, 0); 
    lv_label_set_text(lbl_sc_title, "自定义技能装配"); 
    lv_obj_align(lbl_sc_title, LV_ALIGN_TOP_MID, 0, 0);
    list_skill_slots = lv_list_create(modal_zh_skill_cfg); 
    lv_obj_set_size(list_skill_slots, 200, 200); 
    lv_obj_align(list_skill_slots, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_set_style_bg_color(list_skill_slots, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_border_width(list_skill_slots, 0, 0);
    lv_obj_t * btn_sc_close = lv_btn_create(modal_zh_skill_cfg); 
    lv_obj_set_size(btn_sc_close, 100, 35); 
    lv_obj_align(btn_sc_close, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_sc_close, lv_color_hex(0xc0392b), 0); 
    lv_obj_t * lbl_sc_close = lv_label_create(btn_sc_close); 
    lv_obj_add_style(lbl_sc_close, &style_cn, 0); 
    lv_label_set_text(lbl_sc_close, "完成装配"); 
    lv_obj_center(lbl_sc_close);
    lv_obj_add_event_cb(btn_sc_close, [](lv_event_t *e){ lv_obj_add_flag(modal_zh_skill_cfg, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    modal_zh_skill_select = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_zh_skill_select, 200, 260); 
    lv_obj_center(modal_zh_skill_select);
    lv_obj_add_flag(modal_zh_skill_select, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_zh_skill_select, lv_color_hex(0x1a252f), 0);
    lv_obj_t * lbl_ss_title = lv_label_create(modal_zh_skill_select); 
    lv_obj_add_style(lbl_ss_title, &style_cn, 0); 
    lv_label_set_text(lbl_ss_title, "选择要装配的技能"); 
    lv_obj_align(lbl_ss_title, LV_ALIGN_TOP_MID, 0, 0);
    list_learned_skills = lv_list_create(modal_zh_skill_select); 
    lv_obj_set_size(list_learned_skills, 180, 190); 
    lv_obj_align(list_learned_skills, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_set_style_bg_color(list_learned_skills, lv_color_hex(0x2c3e50), 0); 
    lv_obj_set_style_border_width(list_learned_skills, 0, 0);
    lv_obj_t * btn_ss_close = lv_btn_create(modal_zh_skill_select); 
    lv_obj_set_size(btn_ss_close, 80, 30); 
    lv_obj_align(btn_ss_close, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_ss_close, lv_color_hex(0x555555), 0); 
    lv_obj_t * lbl_ss_close = lv_label_create(btn_ss_close); 
    lv_obj_add_style(lbl_ss_close, &style_cn, 0); 
    lv_label_set_text(lbl_ss_close, "返回"); 
    lv_obj_center(lbl_ss_close);
    lv_obj_add_event_cb(btn_ss_close, [](lv_event_t *e){ lv_obj_add_flag(modal_zh_skill_select, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    // 地图传送 UI
    modal_zh_map = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_zh_map, 220, 280); 
    lv_obj_center(modal_zh_map);
    lv_obj_add_flag(modal_zh_map, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_zh_map, lv_color_hex(0x2c3e50), 0);
    lv_obj_t * map_title = lv_label_create(modal_zh_map); 
    lv_obj_add_style(map_title, &style_cn, 0); 
    lv_label_set_text(map_title, "本港快速传送"); 
    lv_obj_align(map_title, LV_ALIGN_TOP_MID, 0, 0);
    list_map_locs = lv_list_create(modal_zh_map); 
    lv_obj_set_size(list_map_locs, 200, 190); 
    lv_obj_align(list_map_locs, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_set_style_bg_color(list_map_locs, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_border_width(list_map_locs, 0, 0);
    lv_obj_t * btn_map_close = lv_btn_create(modal_zh_map); 
    lv_obj_set_size(btn_map_close, 100, 35); 
    lv_obj_align(btn_map_close, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_map_close, lv_color_hex(0x552222), 0); 
    lv_obj_t * lbl_map_close = lv_label_create(btn_map_close); 
    lv_obj_add_style(lbl_map_close, &style_cn, 0); 
    lv_label_set_text(lbl_map_close, "关闭"); 
    lv_obj_center(lbl_map_close); 
    lv_obj_add_event_cb(btn_map_close, [](lv_event_t *e){ lv_obj_add_flag(modal_zh_map, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    // 航海 UI
    modal_sailing = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_sailing, 220, 240); 
    lv_obj_center(modal_sailing);
    lv_obj_add_flag(modal_sailing, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_sailing, lv_color_hex(0x2c3e50), 0);
    lv_obj_t * sail_title = lv_label_create(modal_sailing); 
    lv_obj_add_style(sail_title, &style_cn, 0); 
    lv_obj_set_style_text_color(sail_title, lv_color_hex(0xFFD700), 0); 
    lv_label_set_text(sail_title, "漫长的航行"); 
    lv_obj_align(sail_title, LV_ALIGN_TOP_MID, 0, 0);
    lbl_sailing_info = lv_label_create(modal_sailing); 
    lv_obj_add_style(lbl_sailing_info, &style_cn, 0); 
    lv_obj_set_width(lbl_sailing_info, 190); 
    lv_obj_set_style_text_color(lbl_sailing_info, lv_color_hex(0xFFFFFF), 0); 
    lv_obj_align(lbl_sailing_info, LV_ALIGN_TOP_MID, 0, 35);
    
    lv_obj_t * btn_sail_cont = lv_btn_create(modal_sailing); 
    lv_obj_set_size(btn_sail_cont, 180, 40); 
    lv_obj_align(btn_sail_cont, LV_ALIGN_BOTTOM_MID, 0, -50); 
    lv_obj_set_style_bg_color(btn_sail_cont, lv_color_hex(0x225522), 0); 
    lv_obj_t * lbl_sail_cont = lv_label_create(btn_sail_cont); 
    lv_obj_add_style(lbl_sail_cont, &style_cn, 0); 
    lv_label_set_text(lbl_sail_cont, "继续前往"); 
    lv_obj_center(lbl_sail_cont); 
    lv_obj_add_event_cb(btn_sail_cont, [](lv_event_t *e){ 
        lv_obj_add_flag(modal_sailing, LV_OBJ_FLAG_HIDDEN); 
        process_port_switch(); 
        zh_player.location_id = sailing_target_id; 
        spawn_monsters_for_loc(sailing_target_id); 
        const ZH_Location* nloc = get_current_loc(); 
        char buf[256]; snprintf(buf, sizeof(buf), "【%s】\n你抵达了新港口。\n%s", nloc->name, nloc->desc); 
        zh_log(buf); 
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * btn_sail_cancel = lv_btn_create(modal_sailing); 
    lv_obj_set_size(btn_sail_cancel, 180, 40); 
    lv_obj_align(btn_sail_cancel, LV_ALIGN_BOTTOM_MID, 0, -5); 
    lv_obj_set_style_bg_color(btn_sail_cancel, lv_color_hex(0x552222), 0); 
    lv_obj_t * lbl_sail_cancel = lv_label_create(btn_sail_cancel); 
    lv_obj_add_style(lbl_sail_cancel, &style_cn, 0); 
    lv_label_set_text(lbl_sail_cancel, "紧急折返"); 
    lv_obj_center(lbl_sail_cancel);
    lv_obj_add_event_cb(btn_sail_cancel, [](lv_event_t *e){ 
        lv_obj_add_flag(modal_sailing, LV_OBJ_FLAG_HIDDEN); 
        process_port_switch(); 
        zh_player.location_id = sailing_origin_id; 
        spawn_monsters_for_loc(sailing_origin_id); 
        zh_log("航线取消，回到原港口。"); 
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
    }, LV_EVENT_CLICKED, NULL);

    // 物品详情 UI
    modal_item_detail = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_item_detail, 200, 220); 
    lv_obj_center(modal_item_detail); 
    lv_obj_add_flag(modal_item_detail, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_item_detail, lv_color_hex(0x2c3e50), 0);
    lbl_item_detail_title = lv_label_create(modal_item_detail); 
    lv_obj_add_style(lbl_item_detail_title, &style_cn, 0); 
    lv_obj_set_style_text_color(lbl_item_detail_title, lv_color_hex(0xF1C40F), 0); 
    lv_obj_align(lbl_item_detail_title, LV_ALIGN_TOP_MID, 0, -10);
    lbl_item_detail_desc = lv_label_create(modal_item_detail); 
    lv_obj_add_style(lbl_item_detail_desc, &style_cn, 0); 
    lv_obj_set_style_text_color(lbl_item_detail_desc, lv_color_hex(0xecf0f1), 0); 
    lv_obj_align(lbl_item_detail_desc, LV_ALIGN_TOP_LEFT, 0, 25); 
    lv_obj_set_width(lbl_item_detail_desc, 180);
    
    lv_obj_t * btn_item_use = lv_btn_create(modal_item_detail); 
    lv_obj_set_size(btn_item_use, 180, 35); 
    lv_obj_align(btn_item_use, LV_ALIGN_BOTTOM_MID, 0, -45); 
    lv_obj_set_style_bg_color(btn_item_use, lv_color_hex(0x27ae60), 0);
    lv_obj_t * lbl_iuse = lv_label_create(btn_item_use); 
    lv_obj_add_style(lbl_iuse, &style_cn, 0); 
    lv_label_set_text(lbl_iuse, "使用 / 装备"); 
    lv_obj_center(lbl_iuse);
    lv_obj_add_event_cb(btn_item_use, [](lv_event_t *e){
        lv_obj_add_flag(modal_item_detail, LV_OBJ_FLAG_HIDDEN); 
        if(selected_inventory_idx == -1) return;
        ZH_Item i_ptr = get_item_by_id(zh_player.inventory[selected_inventory_idx]);
        char buf[128]; 
        int* target_slot = nullptr; 
        const char* slot_n = "";
        
        if (i_ptr.type == 1) { target_slot = &zh_player.eq_weapon; slot_n = "武器"; } 
        else if (i_ptr.type == 2) { target_slot = &zh_player.eq_head; slot_n = "头盔"; } 
        else if (i_ptr.type == 3) { target_slot = &zh_player.eq_chest; slot_n = "胸甲"; } 
        else if (i_ptr.type == 4) { target_slot = &zh_player.eq_legs; slot_n = "护腿"; } 
        else if (i_ptr.type == 5) { target_slot = &zh_player.eq_shoes; slot_n = "鞋子"; }
        
        if (target_slot) { 
            int old_eq = *target_slot; *target_slot = i_ptr.id; zh_player.inventory[selected_inventory_idx] = old_eq; 
            snprintf(buf, sizeof(buf), "装备了新%s：【%s】", slot_n, i_ptr.name); 
        } 
        else if (i_ptr.type == 6 || i_ptr.type == 7) { 
            if(i_ptr.id == 109 || i_ptr.id == 413) {
                zh_player.mp += i_ptr.value; if(zh_player.mp > zh_player.max_mp) zh_player.mp = zh_player.max_mp;
            } else {
                zh_player.hp += i_ptr.value; if(zh_player.hp > zh_player.max_hp) zh_player.hp = zh_player.max_hp;
            }
            if(i_ptr.id == 107 || i_ptr.id == 417) zh_player.crime_value = 0; 
            if(i_ptr.id == 414) { zh_player.atk += 10; zh_player.def += 10; }
            zh_player.inventory[selected_inventory_idx] = -1; snprintf(buf, sizeof(buf), "使用了【%s】。", i_ptr.name); 
        } 
        else if (i_ptr.type == 9) { snprintf(buf, sizeof(buf), "【%s】极度危险，只能在【战斗中】点击道具按钮使用！", i_ptr.name); }
        else if (i_ptr.type == 8) {
            bool is_consumed = true; 
            switch(i_ptr.id) {
                case 201: zh_player.max_hp += 50; zh_player.hp = zh_player.max_hp; snprintf(buf, sizeof(buf), "最大生命永久+50，状态全满！"); break;
                case 206: zh_player.gold += 50000; zh_player.crime_value += 999; snprintf(buf, sizeof(buf), "获得五万铜贝！罪恶滔天！"); break;
                case 216: zh_player.atk += 10; zh_player.def += 10; snprintf(buf, sizeof(buf), "永久增加10点攻防！"); break;
                case 217: zh_player.level++; zh_player.max_hp += 100; zh_player.hp = zh_player.max_hp; snprintf(buf, sizeof(buf), "提升1个等级！"); break;
                case 237: zh_player.debt = 0; snprintf(buf, sizeof(buf), "银行的欠款被抹除！"); break;
                case 241: zh_player.level += 3; zh_player.atk += 30; zh_player.def += 30; zh_player.hp = zh_player.max_hp; snprintf(buf, sizeof(buf), "连升3级，全属性暴涨！"); break;
                case 249: zh_player.exp += 5000; check_levelup(); snprintf(buf, sizeof(buf), "获得海量经验！"); break;
                case 250: zh_player.silver += 100; zh_player.gold += 100000; snprintf(buf, sizeof(buf), "获得100银贝与十万铜贝！"); break;
                default: is_consumed = false; snprintf(buf, sizeof(buf), "【%s】散发光芒，矿石或异宝，需被动庇护、做任务或锻造用。", i_ptr.name); break;
            }
            if (is_consumed) zh_player.inventory[selected_inventory_idx] = -1;
        } 
        zh_log(buf); 
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * btn_item_drop = lv_btn_create(modal_item_detail); 
    lv_obj_set_size(btn_item_drop, 85, 35); 
    lv_obj_align(btn_item_drop, LV_ALIGN_BOTTOM_LEFT, 0, 0); 
    lv_obj_set_style_bg_color(btn_item_drop, lv_color_hex(0xc0392b), 0);
    lv_obj_t * lbl_idrop = lv_label_create(btn_item_drop); 
    lv_obj_add_style(lbl_idrop, &style_cn, 0); 
    lv_label_set_text(lbl_idrop, "丢弃"); 
    lv_obj_center(lbl_idrop);
    lv_obj_add_event_cb(btn_item_drop, [](lv_event_t *e){ 
        lv_obj_add_flag(modal_item_detail, LV_OBJ_FLAG_HIDDEN); 
        zh_player.inventory[selected_inventory_idx] = -1; 
        zh_log("物品已被丢弃进大海。"); 
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * btn_item_canc = lv_btn_create(modal_item_detail); 
    lv_obj_set_size(btn_item_canc, 85, 35); 
    lv_obj_align(btn_item_canc, LV_ALIGN_BOTTOM_RIGHT, 0, 0); 
    lv_obj_set_style_bg_color(btn_item_canc, lv_color_hex(0x7f8c8d), 0);
    lv_obj_t * lbl_icanc = lv_label_create(btn_item_canc); 
    lv_obj_add_style(lbl_icanc, &style_cn, 0); 
    lv_label_set_text(lbl_icanc, "取消"); 
    lv_obj_center(lbl_icanc);
    lv_obj_add_event_cb(btn_item_canc, [](lv_event_t *e){ lv_obj_add_flag(modal_item_detail, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    // NPC UI
    modal_npc = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_npc, 200, 260); 
    lv_obj_center(modal_npc);
    lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_npc, lv_color_hex(0x2c3e50), 0);
    lbl_npc_name = lv_label_create(modal_npc); 
    lv_obj_add_style(lbl_npc_name, &style_cn, 0); 
    lv_obj_set_style_text_color(lbl_npc_name, lv_color_hex(0xF1C40F), 0); 
    lv_obj_align(lbl_npc_name, LV_ALIGN_TOP_MID, 0, -10);
    lbl_npc_dialogue = lv_label_create(modal_npc); 
    lv_obj_add_style(lbl_npc_dialogue, &style_cn, 0); 
    lv_obj_set_style_text_color(lbl_npc_dialogue, lv_color_hex(0xecf0f1), 0); 
    lv_obj_set_width(lbl_npc_dialogue, 180); 
    lv_obj_align(lbl_npc_dialogue, LV_ALIGN_TOP_LEFT, 0, 15);
    
    cont_npc_actions = lv_obj_create(modal_npc); 
    lv_obj_set_size(cont_npc_actions, 190, 150); 
    lv_obj_align(cont_npc_actions, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_opa(cont_npc_actions, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(cont_npc_actions, 0, 0); 
    lv_obj_set_style_pad_all(cont_npc_actions, 0, 0);
    lv_obj_set_flex_flow(cont_npc_actions, LV_FLEX_FLOW_COLUMN); 
    lv_obj_set_flex_align(cont_npc_actions, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); 
    lv_obj_set_style_pad_row(cont_npc_actions, 5, 0);

    // 副官管理 UI
    modal_adjutant = lv_obj_create(scr_zongheng); 
    lv_obj_set_size(modal_adjutant, 220, 280); 
    lv_obj_center(modal_adjutant);
    lv_obj_add_flag(modal_adjutant, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_adjutant, lv_color_hex(0x2c3e50), 0);
    lv_obj_t * lbl_adj_title = lv_label_create(modal_adjutant); 
    lv_obj_add_style(lbl_adj_title, &style_cn, 0); 
    lv_label_set_text(lbl_adj_title, "舰队副官管理"); 
    lv_obj_align(lbl_adj_title, LV_ALIGN_TOP_MID, 0, 0);
    
    list_adj_slots = lv_list_create(modal_adjutant); 
    lv_obj_set_size(list_adj_slots, 200, 200); 
    lv_obj_align(list_adj_slots, LV_ALIGN_TOP_MID, 0, 25); 
    lv_obj_set_style_bg_color(list_adj_slots, lv_color_hex(0x1a252f), 0);
    
    list_adj_roster = lv_list_create(modal_adjutant); 
    lv_obj_set_size(list_adj_roster, 200, 200); 
    lv_obj_align(list_adj_roster, LV_ALIGN_TOP_MID, 0, 25); 
    lv_obj_set_style_bg_color(list_adj_roster, lv_color_hex(0x1a252f), 0); 
    lv_obj_add_flag(list_adj_roster, LV_OBJ_FLAG_HIDDEN); 
    
    lv_obj_t * btn_adj_close = lv_btn_create(modal_adjutant); 
    lv_obj_set_size(btn_adj_close, 100, 35); 
    lv_obj_align(btn_adj_close, LV_ALIGN_BOTTOM_MID, 0, 0); 
    lv_obj_set_style_bg_color(btn_adj_close, lv_color_hex(0xc0392b), 0); 
    lv_obj_t * lbl_adj_close = lv_label_create(btn_adj_close); 
    lv_obj_add_style(lbl_adj_close, &style_cn, 0); 
    lv_label_set_text(lbl_adj_close, "关闭"); 
    lv_obj_center(lbl_adj_close);
    lv_obj_add_event_cb(btn_adj_close, [](lv_event_t *e){ 
        lv_obj_add_flag(modal_adjutant, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(list_adj_roster, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_clear_flag(list_adj_slots, LV_OBJ_FLAG_HIDDEN); 
    }, LV_EVENT_CLICKED, NULL);

    build_market_and_shop_ui(scr_zongheng);
    build_quest_ui(scr_zongheng, tab_zh_quest);
}