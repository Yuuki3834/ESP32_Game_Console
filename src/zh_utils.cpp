#include "global.h"

// =========================================================================
// 全局 UI 对象指针
// =========================================================================
lv_obj_t * scr_zongheng = NULL;
lv_obj_t * lbl_zh_top_status;    
lv_obj_t * lbl_zh_log;           
lv_obj_t * lbl_zh_detail_status; 

lv_obj_t * tab_zh_move;
lv_obj_t * tab_zh_action;
lv_obj_t * tab_zh_bag;
lv_obj_t * tab_zh_status;        
lv_obj_t * tab_zh_quest; 

lv_obj_t * list_zh_move;
lv_obj_t * list_zh_action;
lv_obj_t * list_zh_bag;
lv_obj_t * modal_zh_menu;
lv_obj_t * modal_zh_map; 
lv_obj_t * list_map_locs;
lv_obj_t * modal_sailing;
lv_obj_t * lbl_sailing_info;

int sailing_target_id = -1;
int sailing_origin_id = -1;

ZH_Player zh_player;
int current_npc_idx = -1;
int current_bag_filter = 0; 
int selected_inventory_idx = -1;

lv_obj_t * modal_npc = NULL;
lv_obj_t * lbl_npc_name;
lv_obj_t * lbl_npc_dialogue;
lv_obj_t * cont_npc_actions;

lv_obj_t * modal_item_detail;
lv_obj_t * lbl_item_detail_title;
lv_obj_t * lbl_item_detail_desc;

lv_obj_t * modal_zh_skill_cfg = NULL;
lv_obj_t * list_skill_slots = NULL;
lv_obj_t * modal_zh_skill_select = NULL;
lv_obj_t * list_learned_skills = NULL;
int editing_skill_type = 0;
int editing_skill_idx = 0;

lv_obj_t * modal_adjutant = NULL;
lv_obj_t * list_adj_slots = NULL;
lv_obj_t * list_adj_roster = NULL;
int editing_adj_type = 0;

lv_obj_t * modal_forge = NULL;
lv_obj_t * list_forge_recipes = NULL;

// =========================================================================
// 工具与基础逻辑函数
// =========================================================================
void reset_zongheng_ui_pointers() {
    scr_zongheng = NULL;
    lbl_zh_top_status = NULL;
    lbl_zh_log = NULL;
    lbl_zh_detail_status = NULL;
    tab_zh_move = NULL;
    tab_zh_action = NULL;
    tab_zh_bag = NULL;
    tab_zh_status = NULL;
    tab_zh_quest = NULL;
    list_zh_move = NULL;
    list_zh_action = NULL;
    list_zh_bag = NULL;
    modal_zh_menu = NULL;
    modal_zh_map = NULL;
    list_map_locs = NULL;
    modal_sailing = NULL;
    lbl_sailing_info = NULL;
    sailing_target_id = -1;
    sailing_origin_id = -1;
    modal_npc = NULL;
    lbl_npc_name = NULL;
    lbl_npc_dialogue = NULL;
    cont_npc_actions = NULL;
    modal_item_detail = NULL;
    lbl_item_detail_title = NULL;
    lbl_item_detail_desc = NULL;
    selected_inventory_idx = -1;
    modal_zh_skill_cfg = NULL;
    list_skill_slots = NULL;
    modal_zh_skill_select = NULL;
    list_learned_skills = NULL;
    modal_adjutant = NULL;
    list_adj_slots = NULL;
    list_adj_roster = NULL;
    modal_forge = NULL;
    list_forge_recipes = NULL;
}

const ZH_Skill* get_skill_ptr(int id) {
    if(id <= 0) return NULL;
    for(int i = 0; i < zh_data_skills_count; i++) {
        if(zh_data_skills[i].id == id) return &zh_data_skills[i];
    }
    return NULL;
}

void zh_log(const char * msg) { 
    if (lbl_zh_log != NULL) {
        lv_label_set_text(lbl_zh_log, msg);
    }
}

const ZH_Location* get_current_loc() {
    for (int i = 0; i < zh_data_locations_count; i++) {
        if (zh_data_locations[i].id == zh_player.location_id) return &zh_data_locations[i];
    }
    return &zh_data_locations[0];
}

int get_current_region() {
    int id = zh_player.location_id;
    if(id < 240) return 1; if(id < 480) return 2; if(id < 720) return 3; 
    if(id < 960) return 4; if(id < 1200) return 5; if(id < 1320) return 6; return 7; 
}

void check_levelup() {
    int need_exp = zh_player.level * 100;
    while (zh_player.exp >= need_exp) {
        zh_player.level++; zh_player.exp -= need_exp;
        zh_player.max_hp += 30; zh_player.hp = zh_player.max_hp;
        zh_player.max_mp += 10; zh_player.mp = zh_player.max_mp;
        zh_player.atk += 6; zh_player.def += 4; zh_player.base_crit += 1; 
        
        char buf[128]; 
        snprintf(buf, sizeof(buf), "升级了！当前等级: %d\n生命、魔法、攻击、防御全面提升！", zh_player.level);
        zh_log(buf); 
        need_exp = zh_player.level * 100; 
    }
}
