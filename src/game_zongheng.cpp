#include "global.h"
#include <LittleFS.h>
#include <vector>
#include "zongheng_types.h"

extern const ZH_Recipe zh_data_recipes[];
extern const int zh_data_recipes_count;
extern void reset_combat_ui_pointers();
extern void reset_market_ui_pointers();
extern void reset_quest_ui_pointers();
extern void build_market_and_shop_ui(lv_obj_t * parent);
extern void build_quest_ui(lv_obj_t * parent, lv_obj_t * tab);
extern void refresh_quest_ui();
extern void start_turn_based_combat(int monster_idx); 

const ZH_Adjutant zh_data_adjutants[] = {
    {1, "老练的大副", 1, 0, 1.0, "【航海长】航行免受风暴伤害"},
    {2, "传奇·哥伦布", 1, 1, 1.0, "【传奇航海长】免疫风暴，无视所有航海惩罚"},
    {3, "退役火枪手", 2, 0, 1.0, "【冲锋队长】战斗+15%暴击，+30%逃跑率"},
    {4, "传奇·黑帆剑豪", 2, 1, 1.5, "【传奇冲锋队】战斗+22%暴击，+45%逃跑率"},
    {5, "流浪游医", 3, 0, 1.0, "【船医】每次战斗后恢复20%生命"},
    {6, "传奇·皇家圣手", 3, 1, 2.0, "【传奇船医】每次战斗后恢复40%生命"},
    {7, "精明的犹太商人", 4, 0, 1.0, "【会计】买价降5%，卖价提5%"},
    {8, "传奇·美第奇总管", 4, 1, 2.0, "【传奇会计】买价降10%，卖价提10%"}
};
const int zh_data_adjutants_count = sizeof(zh_data_adjutants)/sizeof(ZH_Adjutant);

const ZH_Adjutant* get_adjutant_by_id(int id) {
    if(id <= 0) return NULL;
    for(int i = 0; i < zh_data_adjutants_count; i++) {
        if(zh_data_adjutants[i].id == id) return &zh_data_adjutants[i];
    }
    return NULL;
}

// -------------------------------------------------------------------------
// 全局 UI 对象指针
// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
// 工具与基础逻辑函数
// -------------------------------------------------------------------------
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

void open_skill_selector();
void refresh_zongheng_ui();

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

// -------------------------------------------------------------------------
// 锻造系统逻辑
// -------------------------------------------------------------------------
static bool craft_item(int recipe_idx) {
    const ZH_Recipe& r = zh_data_recipes[recipe_idx];
    
    if (zh_player.gold < r.cost_gold) { 
        zh_log("锻造失败：铜贝不足以支付手工费！"); 
        return false; 
    }
    
    int count1 = 0, count2 = 0;
    for(int i = 0; i < 50; i++) {
        if(zh_player.inventory[i] == r.mat1_id) count1++;
        if(r.mat2_id != -1 && zh_player.inventory[i] == r.mat2_id) count2++;
    }
    
    if (count1 < r.mat1_count || (r.mat2_id != -1 && count2 < r.mat2_count)) {
        zh_log("锻造失败：背包中缺乏足够的材料！"); return false;
    }
    
    int empty_slots = 0;
    for(int i = 0; i < 50; i++) if(zh_player.inventory[i] == -1) empty_slots++;
    if (empty_slots == 0 && r.mat1_count == 0 && r.mat2_count == 0) {
        zh_log("锻造失败：背包已满！"); return false;
    }

    zh_player.gold -= r.cost_gold;
    int c1 = 0, c2 = 0;
    for(int i = 0; i < 50; i++) {
        if (c1 < r.mat1_count && zh_player.inventory[i] == r.mat1_id) { 
            zh_player.inventory[i] = -1; c1++; continue; 
        }
        if (r.mat2_id != -1 && c2 < r.mat2_count && zh_player.inventory[i] == r.mat2_id) { 
            zh_player.inventory[i] = -1; c2++; 
        }
    }
    
    add_item_to_bag(r.result_item_id);
    char buf[128];
    const char* item_name = get_item_by_id(r.result_item_id).name;
    snprintf(buf, sizeof(buf), "【锻造成功】\n铁锤叮当声后，你获得了绝世神兵：\n[%s]！", item_name);
    zh_log(buf);
    return true;
}

static void open_forge_ui() {
    if (!modal_forge) {
        modal_forge = lv_obj_create(scr_zongheng);
        lv_obj_set_size(modal_forge, 230, 280);
        lv_obj_center(modal_forge);
        lv_obj_set_style_bg_color(modal_forge, lv_color_hex(0x2c3e50), 0);
        lv_obj_add_flag(modal_forge, LV_OBJ_FLAG_HIDDEN);
        
        lv_obj_t * lbl_title = lv_label_create(modal_forge);
        lv_obj_add_style(lbl_title, &style_cn, 0);
        lv_label_set_text(lbl_title, "宗师铁匠铺");
        lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, -10);
        
        list_forge_recipes = lv_list_create(modal_forge);
        lv_obj_set_size(list_forge_recipes, 210, 200);
        lv_obj_align(list_forge_recipes, LV_ALIGN_TOP_MID, 0, 20);
        lv_obj_set_style_bg_color(list_forge_recipes, lv_color_hex(0x1a252f), 0);
        lv_obj_set_style_border_width(list_forge_recipes, 0, 0);
        
        lv_obj_t * btn_close = lv_btn_create(modal_forge);
        lv_obj_set_size(btn_close, 100, 35);
        lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, 5);
        lv_obj_set_style_bg_color(btn_close, lv_color_hex(0xc0392b), 0);
        lv_obj_t * l_c = lv_label_create(btn_close);
        lv_obj_add_style(l_c, &style_cn, 0); 
        lv_label_set_text(l_c, "离开火炉"); 
        lv_obj_center(l_c);
        lv_obj_add_event_cb(btn_close, [](lv_event_t *e){ lv_obj_add_flag(modal_forge, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);
    }
    
    lv_obj_clean(list_forge_recipes);
    
    for (int i = 0; i < zh_data_recipes_count; i++) {
        const ZH_Recipe& r = zh_data_recipes[i];
        ZH_Item res = get_item_by_id(r.result_item_id);
        ZH_Item m1 = get_item_by_id(r.mat1_id);
        
        char buf[256];
        if (r.mat2_id != -1) {
            ZH_Item m2 = get_item_by_id(r.mat2_id);
            snprintf(buf, sizeof(buf), "造:%s\n需:%s(x%d), %s(x%d)\n费:%d铜", res.name, m1.name, r.mat1_count, m2.name, r.mat2_count, r.cost_gold);
        } else {
            snprintf(buf, sizeof(buf), "造:%s\n需:%s(x%d)\n费:%d铜", res.name, m1.name, r.mat1_count, r.cost_gold);
        }
        
        lv_obj_t * btn = lv_list_add_btn(list_forge_recipes, LV_SYMBOL_SETTINGS, buf);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xd35400), 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 1);
        if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
        
        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            int r_idx = (int)(intptr_t)lv_event_get_user_data(e);
            if(craft_item(r_idx)) {
                lv_obj_add_flag(modal_forge, LV_OBJ_FLAG_HIDDEN);
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            }
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
    lv_obj_clear_flag(modal_forge, LV_OBJ_FLAG_HIDDEN);
}

// -------------------------------------------------------------------------
// 副官与技能系统 UI 逻辑
// -------------------------------------------------------------------------
void open_adjutant_roster() {
    lv_obj_clean(list_adj_roster);
    
    lv_obj_t * btn_uneq = lv_list_add_btn(list_adj_roster, LV_SYMBOL_CLOSE, "卸下当前槽位副官");
    lv_obj_set_style_bg_color(btn_uneq, lv_color_hex(0xc0392b), 0);
    lv_obj_t * lbl_u = lv_obj_get_child(btn_uneq, 1); 
    if(lbl_u) { lv_obj_add_style(lbl_u, &style_cn, 0); lv_obj_set_style_text_color(lbl_u, lv_color_hex(0xFFFFFF), 0); }
    lv_obj_add_event_cb(btn_uneq, [](lv_event_t *e){
        if(editing_adj_type == 1) zh_player.eq_adj_navigator = 0;
        else if(editing_adj_type == 2) zh_player.eq_adj_assault = 0;
        else if(editing_adj_type == 3) zh_player.eq_adj_doctor = 0;
        else if(editing_adj_type == 4) zh_player.eq_adj_accountant = 0;
        lv_obj_add_flag(list_adj_roster, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(list_adj_slots, LV_OBJ_FLAG_HIDDEN);
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_dismiss = lv_list_add_btn(list_adj_roster, LV_SYMBOL_TRASH, "解雇所有闲置的普通副官");
    lv_obj_set_style_bg_color(btn_dismiss, lv_color_hex(0x7f8c8d), 0);
    lv_obj_t * lbl_d = lv_obj_get_child(btn_dismiss, 1); 
    if(lbl_d) { lv_obj_add_style(lbl_d, &style_cn, 0); lv_obj_set_style_text_color(lbl_d, lv_color_hex(0xFFFFFF), 0); }
    lv_obj_add_event_cb(btn_dismiss, [](lv_event_t *e){
        for(int i = 0; i < 10; i++) {
            int aid = zh_player.adjutants_roster[i];
            if(aid > 0) {
                const ZH_Adjutant* adj = get_adjutant_by_id(aid);
                if(adj && adj->rarity == 0 && 
                   zh_player.eq_adj_navigator != aid && zh_player.eq_adj_assault != aid &&
                   zh_player.eq_adj_doctor != aid && zh_player.eq_adj_accountant != aid) {
                    zh_player.adjutants_roster[i] = 0;
                }
            }
        }
        zh_log("已清理麾下所有未上任的普通副官！");
        lv_obj_add_flag(list_adj_roster, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(list_adj_slots, LV_OBJ_FLAG_HIDDEN);
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
    }, LV_EVENT_CLICKED, NULL);

    bool has_adj = false;
    for(int i = 0; i < 10; i++) {
        int aid = zh_player.adjutants_roster[i];
        if(aid > 0) {
            const ZH_Adjutant* adj = get_adjutant_by_id(aid);
            if(adj && adj->type == editing_adj_type) {
                has_adj = true;
                char buf[128]; snprintf(buf, sizeof(buf), "%s %s", adj->rarity == 1 ? "[传奇]" : "", adj->name);
                lv_obj_t * btn = lv_list_add_btn(list_adj_roster, LV_SYMBOL_PLAY, buf);
                lv_obj_set_style_bg_color(btn, lv_color_hex(adj->rarity == 1 ? 0xd35400 : 0x2980b9), 0);
                lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
                if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
                
                lv_obj_add_event_cb(btn, [](lv_event_t *e){
                    int selected_id = (int)(intptr_t)lv_event_get_user_data(e);
                    if(editing_adj_type == 1) zh_player.eq_adj_navigator = selected_id;
                    else if(editing_adj_type == 2) zh_player.eq_adj_assault = selected_id;
                    else if(editing_adj_type == 3) zh_player.eq_adj_doctor = selected_id;
                    else if(editing_adj_type == 4) zh_player.eq_adj_accountant = selected_id;
                    
                    lv_obj_add_flag(list_adj_roster, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(list_adj_slots, LV_OBJ_FLAG_HIDDEN);
                    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
                }, LV_EVENT_CLICKED, (void*)(intptr_t)aid);
            }
        }
    }
    
    if(!has_adj) {
        lv_obj_t * lbl_none = lv_label_create(list_adj_roster); 
        lv_obj_add_style(lbl_none, &style_cn, 0);
        lv_obj_set_style_text_color(lbl_none, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(lbl_none, "\n您麾下没有该类型的副官。"); 
        lv_obj_center(lbl_none);
    }
}

void refresh_adjutant_cfg_list() {
    lv_obj_clean(list_adj_slots);
    const char* slot_names[] = {"航海长", "冲锋队长", "船医", "会计"};
    int eq_slots[] = {zh_player.eq_adj_navigator, zh_player.eq_adj_assault, zh_player.eq_adj_doctor, zh_player.eq_adj_accountant};
    
    for(int i = 0; i < 4; i++) {
        char buf[128];
        const ZH_Adjutant* adj = get_adjutant_by_id(eq_slots[i]);
        snprintf(buf, sizeof(buf), "%s: %s", slot_names[i], adj ? adj->name : "- 空缺 -");
        lv_obj_t * btn = lv_list_add_btn(list_adj_slots, LV_SYMBOL_SETTINGS, buf);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2c3e50), 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
        if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }

        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            editing_adj_type = (int)(intptr_t)lv_event_get_user_data(e);
            lv_obj_add_flag(list_adj_slots, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(list_adj_roster, LV_OBJ_FLAG_HIDDEN);
            open_adjutant_roster();
        }, LV_EVENT_CLICKED, (void*)(intptr_t)(i+1));
    }
}

void refresh_skill_cfg_list() {
    lv_obj_clean(list_skill_slots);
    for(int i = 0; i < 12; i++) {
        char buf[128];
        int sid = zh_player.eq_active_skills[i];
        const ZH_Skill* sk = get_skill_ptr(sid);
        snprintf(buf, sizeof(buf), "主动槽 %d: %s", i+1, sk ? sk->name : "- 未装配 -");
        lv_obj_t * btn = lv_list_add_btn(list_skill_slots, LV_SYMBOL_PLAY, buf);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2c3e50), 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
        if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }

        int ud = (1 << 16) | i;
        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            int v = (int)(intptr_t)lv_event_get_user_data(e);
            editing_skill_type = v >> 16; editing_skill_idx = v & 0xFFFF;
            open_skill_selector();
        }, LV_EVENT_CLICKED, (void*)(intptr_t)ud);
    }
    for(int i = 0; i < 4; i++) {
        char buf[128];
        int sid = zh_player.eq_passive_skills[i];
        const ZH_Skill* sk = get_skill_ptr(sid);
        snprintf(buf, sizeof(buf), "被动槽 %d: %s", i+1, sk ? sk->name : "- 未装配 -");
        lv_obj_t * btn = lv_list_add_btn(list_skill_slots, LV_SYMBOL_SETTINGS, buf);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1a252f), 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
        if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }

        int ud = (2 << 16) | i;
        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            int v = (int)(intptr_t)lv_event_get_user_data(e);
            editing_skill_type = v >> 16; editing_skill_idx = v & 0xFFFF;
            open_skill_selector();
        }, LV_EVENT_CLICKED, (void*)(intptr_t)ud);
    }
}

void open_skill_selector() {
    lv_obj_clean(list_learned_skills);
    
    lv_obj_t * btn_uneq = lv_list_add_btn(list_learned_skills, LV_SYMBOL_CLOSE, "卸下当前槽位技能");
    lv_obj_set_style_bg_color(btn_uneq, lv_color_hex(0xc0392b), 0);
    lv_obj_t * lbl_u = lv_obj_get_child(btn_uneq, 1); 
    if(lbl_u) { lv_obj_add_style(lbl_u, &style_cn, 0); lv_obj_set_style_text_color(lbl_u, lv_color_hex(0xFFFFFF), 0); }
    lv_obj_add_event_cb(btn_uneq, [](lv_event_t *e){
        if(editing_skill_type == 1) zh_player.eq_active_skills[editing_skill_idx] = 0;
        else zh_player.eq_passive_skills[editing_skill_idx] = 0;
        lv_obj_add_flag(modal_zh_skill_select, LV_OBJ_FLAG_HIDDEN);
        refresh_skill_cfg_list(); 
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
    }, LV_EVENT_CLICKED, NULL);

    bool has_skill = false;
    for(int i = 0; i < 160; i++) {
        int sid = zh_player.learned_skills[i];
        if(sid > 0) {
            const ZH_Skill* sk = get_skill_ptr(sid); if(!sk) continue;
            bool is_active = (sk->type >= 1 && sk->type <= 4);
            bool is_passive = (sk->type == 5 || sk->type == 6);
            if((editing_skill_type == 1 && is_active) || (editing_skill_type == 2 && is_passive)) {
                has_skill = true;
                lv_obj_t * btn = lv_list_add_btn(list_learned_skills, LV_SYMBOL_FILE, sk->name);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x34495e), 0);
                lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
                if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
                
                lv_obj_add_event_cb(btn, [](lv_event_t *e){
                    int selected_id = (int)(intptr_t)lv_event_get_user_data(e);
                    if(editing_skill_type == 1) {
                        for(int j=0; j<12; j++) if(zh_player.eq_active_skills[j] == selected_id) zh_player.eq_active_skills[j] = 0;
                        zh_player.eq_active_skills[editing_skill_idx] = selected_id;
                    } else {
                        for(int j=0; j<4; j++) if(zh_player.eq_passive_skills[j] == selected_id) zh_player.eq_passive_skills[j] = 0;
                        zh_player.eq_passive_skills[editing_skill_idx] = selected_id;
                    }
                    lv_obj_add_flag(modal_zh_skill_select, LV_OBJ_FLAG_HIDDEN);
                    refresh_skill_cfg_list(); 
                    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
                }, LV_EVENT_CLICKED, (void*)(intptr_t)sid);
            }
        }
    }
    if(!has_skill) {
        lv_obj_t * lbl_none = lv_label_create(list_learned_skills); 
        lv_obj_add_style(lbl_none, &style_cn, 0);
        lv_obj_set_style_text_color(lbl_none, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(lbl_none, "\n您还未学会此类技能"); 
        lv_obj_center(lbl_none);
    }
    lv_obj_clear_flag(modal_zh_skill_select, LV_OBJ_FLAG_HIDDEN);
}

// -------------------------------------------------------------------------
// 探索与地图逻辑
// -------------------------------------------------------------------------
void spawn_monsters_for_loc(int loc_id);

void player_die() {
    zh_player.gold = 0; 
    zh_player.hp = zh_player.max_hp / 2; 
    zh_player.mp = zh_player.max_mp / 2;
    if (zh_player.crime_value > 0) {
        zh_player.crime_value = 0; 
        zh_log("被打败，铜贝掉落！\n幸被送往教堂/医馆，罪恶清零。");
    } else {
        zh_log("被打败！铜贝被洗劫一空！\n你被抬到了最近的教堂/医馆。");
    }
    int base_id = (zh_player.location_id / 40) * 40; 
    int target_church = base_id;
    for(int i = 0; i < zh_data_locations_count; i++) {
        if(zh_data_locations[i].id >= base_id && zh_data_locations[i].id < base_id + 40 && zh_data_locations[i].action_type == 6) { 
            target_church = zh_data_locations[i].id; 
            break; 
        }
    }
    zh_player.location_id = target_church; 
    spawn_monsters_for_loc(target_church); 
    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
}

void spawn_monsters_for_loc(int loc_id) {
    for(int i = 0; i < 5; i++) zh_player.current_monsters[i] = -1;
    const ZH_Location* loc = get_current_loc();
    if(loc->action_type != 1 && loc->action_type != 13) return;

    int hub_id = 0;
    if (loc_id >= 240 && loc_id < 480) hub_id = 240;
    else if (loc_id >= 480 && loc_id < 720) hub_id = 480;
    else if (loc_id >= 720 && loc_id < 960) hub_id = 720;
    else if (loc_id >= 960 && loc_id < 1200) hub_id = 960;
    else if (loc_id >= 1200 && loc_id < 1320) hub_id = 1200;
    else if (loc_id >= 1320) hub_id = 1320;

    int bounty_boss = check_bounty_spawn(loc_id);
    if (bounty_boss != -1 && bounty_boss >= 0 && bounty_boss < zh_data_monsters_count) {
        zh_player.current_monsters[0] = bounty_boss; 
        zh_player.current_monsters_hp[0] = zh_data_monsters[bounty_boss].hp;
        zh_log("【危险警告】\n通缉令上的终极目标就在眼前！准备殊死搏斗吧！");
    } else if (bounty_boss != -1) {
        Serial.printf("WARNING: Invalid bounty_boss ID: %d > %d\n", bounty_boss, zh_data_monsters_count);
    }

    int start_i = (bounty_boss != -1) ? 1 : 0;
    int num_spawns = rand() % 3 + 1; 
    if (loc->param2 - loc->param1 > 20) num_spawns = rand() % 3 + 3; 

    for(int i = start_i; i < num_spawns; i++) {
        int r = rand() % 100;
        int target_region = 0; 
        if (loc->param2 > 40) {
            if (r < 25) target_region = 0;      
            else if (r < 55) target_region = 1; 
            else target_region = hub_id;        
        } else if (loc->param2 > 15) {
            if (r < 40) target_region = 0; else target_region = 1;
        }

        std::vector<int> pool;
        for(int m = 0; m < zh_data_monsters_count; m++) {
            if (zh_data_monsters[m].region_id == target_region || zh_data_monsters[m].region_id == 0) {
                if (zh_data_monsters[m].level <= loc->param2 + 10) pool.push_back(m);
            }
        }
        if(pool.empty()) pool.push_back(0); 
        int chosen_m = pool[rand() % pool.size()];
        zh_player.current_monsters[i] = chosen_m;
        zh_player.current_monsters_hp[i] = zh_data_monsters[chosen_m].hp;
    }

    for(int i = 0; i < 5; i++) {
        int mid = zh_player.current_monsters[i];
        if(mid != -1 && zh_data_monsters[mid].is_aggressive && rand() % 100 < 30) {
            char buf[128]; snprintf(buf, sizeof(buf), "【被偷袭！】\n刚踏入此地，%s 发动了突袭！", zh_data_monsters[mid].name);
            zh_log(buf); 
            start_turn_based_combat(i); 
            break; 
        }
    }
}

void execute_combat(int monster_idx) { 
    if (zh_player.current_monsters[monster_idx] == -1) return; 
    start_turn_based_combat(monster_idx); 
}

void populate_map_list() {
    lv_obj_clean(list_map_locs);
    int base = (zh_player.location_id / 40) * 40;
    for(int i = 0; i < zh_data_locations_count; i++) {
        if(zh_data_locations[i].id >= base && zh_data_locations[i].id < base + 40 && 
           zh_data_locations[i].action_type != 1 && zh_data_locations[i].name[0] != '\0') {
            lv_obj_t * btn = lv_list_add_btn(list_map_locs, LV_SYMBOL_RIGHT, zh_data_locations[i].name);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x34495e), 0);
            lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
            if(lbl){ lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
            lv_obj_add_event_cb(btn, [](lv_event_t *e){
                zh_player.location_id = (int)(intptr_t)lv_event_get_user_data(e);
                lv_obj_add_flag(modal_zh_map, LV_OBJ_FLAG_HIDDEN); 
                zh_market_state = 0; 
                spawn_monsters_for_loc(zh_player.location_id); 
                zh_log("你快速抵达了目的地。"); 
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            }, LV_EVENT_CLICKED, (void*)(intptr_t)zh_data_locations[i].id);
        }
    }
}

void start_sailing(int origin_id, int target_id) {
    sailing_origin_id = origin_id; sailing_target_id = target_id;
    char msg[256] = ""; 
    trigger_sailing_event(msg, sizeof(msg));
    lv_label_set_text(lbl_sailing_info, msg); 
    lv_obj_clear_flag(modal_sailing, LV_OBJ_FLAG_HIDDEN);
}

// -------------------------------------------------------------------------
// 核心刷新逻辑
// -------------------------------------------------------------------------
void refresh_zongheng_ui() {
    if (!scr_zongheng) return;
    
    char top_stat[128]; 
    snprintf(top_stat, sizeof(top_stat), "LV:%d HP:%d/%d\n铜:%ld 银:%ld", 
            zh_player.level, zh_player.hp, zh_player.max_hp, zh_player.gold, zh_player.silver);
    lv_label_set_text(lbl_zh_top_status, top_stat);

    auto get_eq_name = [](int id) -> const char* { 
        if(id == -1) return "无"; 
        return get_item_by_id(id).name; 
    };

    const ZH_Adjutant* a1 = get_adjutant_by_id(zh_player.eq_adj_navigator);
    const ZH_Adjutant* a2 = get_adjutant_by_id(zh_player.eq_adj_assault);
    const ZH_Adjutant* a3 = get_adjutant_by_id(zh_player.eq_adj_doctor);
    const ZH_Adjutant* a4 = get_adjutant_by_id(zh_player.eq_adj_accountant);

    int t_atk = get_total_atk();
    int t_def = get_total_def();
    int level_exp = zh_player.level * 100;
    const char* sailor_set = has_sailor_set() ? "[水手]" : "";
    const char* steel_set = has_steel_set() ? "[精钢]" : "";
    
    static char detail_buf[1024];
    snprintf(detail_buf, sizeof(detail_buf),
        "【详细属性】\n等 级: %d (EXP: %d/%d)\n生 命: %d / %d | 魔 法: %d / %d\n"
        "攻 击: %d | 防 御: %d\n暴 击: %d%% | 闪 避: %d%%\n套 装: %s%s\n"
        "存款: %ld | 负债: %ld\n声望(名誉): %d | 罪恶(通缉): %d\n\n【装备】\n"
        "头: %s | 胸: %s\n腿: %s | 鞋: %s\n武: %s\n\n【副官】\n航海: %s | 冲锋: %s\n船医: %s | 会计: %s",
        zh_player.level, zh_player.exp, level_exp, zh_player.hp, zh_player.max_hp, zh_player.mp, zh_player.max_mp,
        t_atk, t_def, zh_player.base_crit, zh_player.base_dodge, sailor_set, steel_set,
        zh_player.bank_gold, zh_player.debt, zh_player.reputation, zh_player.crime_value,
        get_eq_name(zh_player.eq_head), get_eq_name(zh_player.eq_chest), get_eq_name(zh_player.eq_legs), 
        get_eq_name(zh_player.eq_shoes), get_eq_name(zh_player.eq_weapon),
        a1 ? a1->name : "空", a2 ? a2->name : "空", a3 ? a3->name : "空", a4 ? a4->name : "空"
    );
    lv_label_set_text(lbl_zh_detail_status, detail_buf);

    const ZH_Location* loc = get_current_loc();
    lv_obj_clean(list_zh_move);
    for (int i = 0; i < 4; i++) {
        if (loc->links[i] != -1) {
            int tid = loc->links[i]; 
            const char* tname = "未知";
            for(int j = 0; j < zh_data_locations_count; j++) {
                if(zh_data_locations[j].id == tid) tname = zh_data_locations[j].name;
            }
            lv_obj_t * btn = lv_list_add_btn(list_zh_move, LV_SYMBOL_RIGHT, tname);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2c3e50), 0); 
            lv_obj_set_style_border_width(btn, 0, 0); 
            lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
            if(lbl){ lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
            
            lv_obj_add_event_cb(btn, [](lv_event_t *e){
                int tid = (int)(intptr_t)lv_event_get_user_data(e);
                bool is_cross_port = (zh_player.location_id / 40) != (tid / 40);
                zh_market_state = 0; 
                if (is_cross_port && zh_player.crime_value >= 100) { 
                    zh_log("【警告】通缉犯禁止出港！去警局处理！"); return; 
                }
                if (is_cross_port) { 
                    start_sailing(zh_player.location_id, tid); return; 
                } 
                
                if (zh_player.crime_value >= 50 && rand() % 100 < 5) { 
                    zh_log("街上被巡警盘问罚款 50 铜贝！"); 
                    zh_player.gold -= 50; 
                    if(zh_player.gold < 0) zh_player.gold = 0; 
                }
                zh_player.location_id = tid; 
                spawn_monsters_for_loc(tid); 
                const ZH_Location* nloc = get_current_loc(); 
                static char buf[512];
                snprintf(buf, sizeof(buf), "【%s】\n%s", nloc->name, nloc->desc); 
                trigger_random_land_event(buf, sizeof(buf)); 
                zh_log(buf); 
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            }, LV_EVENT_CLICKED, (void*)(intptr_t)tid);
        }
    }

    lv_obj_clean(list_zh_action);
    auto add_act_btn = [](const char* txt, lv_event_cb_t cb, uint32_t color = 0x2c3e50, void* user_data = NULL) {
        lv_obj_t * btn = lv_list_add_btn(list_zh_action, LV_SYMBOL_PLAY, txt); 
        lv_obj_set_style_bg_color(btn, lv_color_hex(color), 0); 
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
        if(lbl){ lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    };

    if (loc->action_type == 1 || loc->action_type == 13) { 
        bool has_monster = false;
        for(int i = 0; i < 5; i++) {
            int mid = zh_player.current_monsters[i];
            if (mid != -1) {
                has_monster = true; 
                const ZH_Monster* m = &zh_data_monsters[mid]; 
                int hp = zh_player.current_monsters_hp[i];
                char buf[64]; 
                snprintf(buf, sizeof(buf), "%s %s (Lv.%d | HP:%d)", m->is_aggressive ? "[主动]" : "攻击", m->name, m->level, hp);
                uint32_t color = m->is_aggressive ? 0x8B0000 : 0x8B4500;
                add_act_btn(buf, [](lv_event_t *e){ execute_combat((int)(intptr_t)lv_event_get_user_data(e)); }, color, (void*)(intptr_t)i);
            }
        }
        if (!has_monster && loc->action_type == 1) add_act_btn("此地已肃清，非常安全。", [](lv_event_t *e){}, 0x555555);
        
        if (loc->action_type == 1) {
            add_act_btn("探索搜集 (材料/矿石/草药)", [](lv_event_t *e){
                int r = rand() % 100;
                int loc_lvl = get_current_loc()->param2;
                int loot_id = -1;

                if(r < 25) { 
                    int herbs[] = {108, 109, 412, 413, 414, 415, 416, 417};
                    loot_id = herbs[rand() % 8];
                } else if (r < 55) { 
                    if (loc_lvl > 70) loot_id = 405 + rand()%6; 
                    else if (loc_lvl > 40) loot_id = 403 + rand()%4; 
                    else loot_id = 401 + rand()%3; 
                } else if (r < 80) { 
                    loot_id = 421 + rand() % 15;
                } else if (r < 90) {
                    zh_log("【搜集】\n你找了半天，只摸到一手灰。");
                } else {
                    zh_log("【警觉】\n搜集动静太大，惊动了附近的怪物！");
                    for(int i = 0; i < 5; i++) if(zh_player.current_monsters[i] != -1) { start_turn_based_combat(i); return; }
                }

                if (loot_id != -1) {
                    if(add_item_to_bag(loot_id)) {
                        char buf[128];
                        const char* item_name = get_item_by_id(loot_id).name;
                        snprintf(buf, sizeof(buf), "【搜集】\n你仔细摸索，获得了 【%s】！", item_name);
                        zh_log(buf);
                    } else {
                        zh_log("背包已满，无法拾取材料。");
                    }
                }
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            }, 0x27ae60);
        }

        if (loc->action_type == 13) {
            add_act_btn("打劫路人 (高风险)", [](lv_event_t *e){ 
                int gain = rand() % 500 + 100; 
                zh_player.gold += gain; zh_player.crime_value += 20; 
                char buf[128]; snprintf(buf, sizeof(buf), "抢到 %d 铜贝！罪恶+20！", gain); 
                zh_log(buf); 
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            }, 0x8B0000);
        }
    } else if (loc->action_type == 5) { 
        add_act_btn("存入所有铜贝", [](lv_event_t *e){ 
            zh_player.bank_gold += zh_player.gold; zh_player.gold = 0; zh_log("已存入。换港口结5%利息。"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        });
        add_act_btn("取出所有存款", [](lv_event_t *e){ 
            zh_player.gold += zh_player.bank_gold; zh_player.bank_gold = 0; zh_log("存款已取出。"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        });
        add_act_btn("贷款 1000 铜贝", [](lv_event_t *e){ 
            zh_player.debt += 1000; zh_player.gold += 1000; zh_log("放款成功。换港口扣7.5%利息。"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        });
        add_act_btn("偿还 1000 欠款", [](lv_event_t *e){
            if(zh_player.debt <= 0) zh_log("没有欠款。"); 
            else if(zh_player.gold >= 1000) { 
                zh_player.gold -= 1000; zh_player.debt -= 1000; 
                if(zh_player.debt < 0) zh_player.debt = 0; 
                zh_log("还款成功！"); 
            } else zh_log("铜贝不足！"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
        });
        add_act_btn("1200铜贝 换 1银贝", [](lv_event_t *e){ 
            if(zh_player.gold >= 1200) { zh_player.gold -= 1200; zh_player.silver += 1; zh_log("兑换成功。"); } 
            else zh_log("铜贝不足！"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        }, 0x555500);
        add_act_btn("1银贝 换 1000铜贝", [](lv_event_t *e){ 
            if(zh_player.silver >= 1) { zh_player.silver -= 1; zh_player.gold += 1000; zh_log("兑换成功！"); } 
            else zh_log("没有银贝！"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        }, 0x555500);
    } else if (loc->action_type == 14) { 
        add_act_btn("查看悬赏榜(世界Boss)", [](lv_event_t *e){ 
            open_tavern_bounty_board(); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        }, 0x8B0000);
    } else if (loc->action_type == 7) { 
        add_act_btn("领救济", [](lv_event_t *e){ 
            if(zh_player.gold < 100) { zh_player.gold += 50; zh_log("愿主保佑。"); } 
            else zh_log("留给需要的人吧。"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        });
    } else if (loc->action_type == 8) { 
        add_act_btn("赌大小 (下注100铜)", [](lv_event_t *e){ 
            if(zh_player.gold >= 100) { 
                zh_player.gold -= 100; 
                if(rand()%2 == 0) { zh_player.gold += 200; zh_log("赢了！"); } 
                else zh_log("通吃！"); 
            } else zh_log("滚出去！"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        }, 0x8B0000);
    } else if (loc->action_type == 10) { 
        refresh_market_action_list(list_zh_action, loc);
    } else if (loc->action_type == 9) { 
        add_act_btn("上床睡觉 (全恢复)", [](lv_event_t *e){ 
            zh_player.hp = zh_player.max_hp; zh_player.mp = zh_player.max_mp; 
            zh_log("睡了一觉，精神百倍，伤势与法力全部恢复！"); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        }, 0x884400);
    }

    if (loc->action_type == 14 || loc->action_type == 0 || loc->action_type == 13) {
        add_act_btn("招募水手/副官 (花费 1 银贝)", [](lv_event_t *e){ 
            if(zh_player.silver >= 1) {
                int empty_slot = -1;
                for(int i = 0; i < 10; i++) { 
                    if(zh_player.adjutants_roster[i] == 0) { empty_slot = i; break; } 
                }
                if(empty_slot == -1) { zh_log("你的麾下已满编 (10人上限)，请先解雇部分人员！"); return; }
                
                zh_player.silver -= 1;
                int pool_start = (rand() % 100 < 5) ? 1 : 0; 
                int r_type = (rand() % 4) * 2 + pool_start;
                int new_adj_id = zh_data_adjutants[r_type].id;
                bool already_has = false;
                for(int k = 0; k < 10; k++) {
                    if(zh_player.adjutants_roster[k] == new_adj_id) { already_has = true; break; }
                }
                if(already_has) {
                    zh_log("【招募失败】\n该类型的副官你已拥有，招募者退回了你的银贝。");
                    zh_player.silver += 1; 
                    return;
                }
                
                zh_player.adjutants_roster[empty_slot] = new_adj_id;
                char buf[128];
                snprintf(buf, sizeof(buf), "【招募成功】\n花费 1 枚银贝，你成功招募了: %s！\n请在菜单-副官中任命。", zh_data_adjutants[r_type].name);
                zh_log(buf); 
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            } else {
                zh_log("银贝不足！副官招募需要硬通货！");
            }
        }, 0x16a085);
    }

    auto open_npc_cb = [](lv_event_t *e) {
        current_npc_idx = (int)(intptr_t)lv_event_get_user_data(e);
        if (current_npc_idx < 0 || current_npc_idx >= zh_data_npcs_count) {
            zh_log("错误：无效的NPC索引！"); return;
        }
        const ZH_NPC* npc = &zh_data_npcs[current_npc_idx];
        
        char final_name[64];
        if (npc->id >= 100) {
            const char* prefix[] = {"神秘的", "愤怒的", "鬼祟的", "热情的", "伤心的", "无聊的", "醉酒的", "落魄的"};
            snprintf(final_name, sizeof(final_name), "%s%s", prefix[rand() % 8], npc->name);
        } else {
            strcpy(final_name, npc->name);
        }
        lv_label_set_text(lbl_npc_name, final_name);
        
        bool is_injured = (zh_player.npc_status[npc->id] == 1);
        if (is_injured) {
            lv_label_set_text(lbl_npc_dialogue, "别打我了，我好痛！你需要什么我都给你...");
        } else {
            int d_idx;
            int safe_cnt = 0;
            do {
                d_idx = rand() % 5;
                safe_cnt++;
            } while ((npc->dialogs[d_idx] == NULL || strlen(npc->dialogs[d_idx]) == 0) && safe_cnt < 10);

            if (npc->dialogs[d_idx] == NULL || strlen(npc->dialogs[d_idx]) == 0) {
                lv_label_set_text(lbl_npc_dialogue, "（这个人沉默不语...）");
            } else {
                lv_label_set_text(lbl_npc_dialogue, npc->dialogs[d_idx]);
            }
        }

        lv_obj_clean(cont_npc_actions);
        auto add_n_btn = [](const char* txt, lv_event_cb_t cb, uint32_t color) {
            lv_obj_t * b = lv_btn_create(cont_npc_actions); 
            lv_obj_set_size(b, 170, 35); 
            lv_obj_set_style_bg_color(b, lv_color_hex(color), 0);
            lv_obj_t * l = lv_label_create(b); 
            lv_obj_add_style(l, &style_cn, 0); 
            lv_label_set_text(l, txt); 
            lv_obj_center(l);
            lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
        };

        add_n_btn("继续对话", [](lv_event_t *e){
            const ZH_NPC* npc = &zh_data_npcs[current_npc_idx];
            bool is_injured = (zh_player.npc_status[npc->id] == 1);
            if (is_injured) zh_log("对方捂着伤口，恐惧地看着你不敢说话。");
            else {
                int word_idx = npc->id / 32, bit_idx = npc->id % 32;
                bool has_gift = (zh_player.npc_gift_bits[word_idx] & (1 << bit_idx));
                if(npc->gift_item_id != -1 && !has_gift) {
                    bool added = false;
                    for(int k=0; k<50; k++) { 
                        if(zh_player.inventory[k] == -1) { zh_player.inventory[k] = npc->gift_item_id; added = true; break; } 
                    }
                    if(added) { 
                        zh_player.npc_gift_bits[word_idx] |= (1 << bit_idx); 
                        char buf[128]; snprintf(buf, sizeof(buf), "【意外之喜】\n%s 塞给你一件物品！", npc->name); 
                        zh_log(buf); 
                    } else zh_log("背包已满，无法接收对方的馈赠！");
                } else {
                    int d_idx;
                    do { d_idx = rand() % 5; } while (npc->dialogs[d_idx] == NULL || strlen(npc->dialogs[d_idx]) == 0);
                    zh_log(npc->dialogs[d_idx]);
                }
            }
            lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
        }, 0x2980b9);

        if(npc->shop_type == 0 && npc->special_func == 0 && !is_injured) {
            if(zh_player.quest_id == 0) {
                add_n_btn("询问跑腿委托", [](lv_event_t *e){
                    zh_player.quest_id = (rand() % 3) + 1; 
                    if(zh_player.quest_id == 1) { 
                        int cur_region = get_current_region();
                        std::vector<int> pool;
                        for(int m=0; m<zh_data_monsters_count; m++) {
                            if (zh_data_monsters[m].region_id == cur_region && zh_data_monsters[m].level <= zh_player.level + 10) pool.push_back(m);
                        }
                        if(pool.empty()) pool.push_back(0); 
                        zh_player.quest_target = pool[rand() % pool.size()]; 
                        zh_player.quest_progress = (0 << 16) | 3; 
                        char buf[128]; snprintf(buf, sizeof(buf), "【接受委托 - 讨伐】\n城镇周边不太平，请去野外帮我清理 3 只【%s】吧！", zh_data_monsters[zh_player.quest_target].name);
                        zh_log(buf);
                    } else if(zh_player.quest_id == 2) { 
                        zh_player.quest_target = (rand()%2==0)? 108 : 401; // 止血草 或 劣质铜矿
                        zh_player.quest_progress = (0 << 16) | 3; 
                        char buf[128];
                        const char* item_name = get_item_by_id(zh_player.quest_target).name;
                        snprintf(buf, sizeof(buf), "【接受委托 - 搜集】\n请去野外搜集 3 份【%s】，完成后带回来交给我或任何人。", item_name);
                        zh_log(buf);
                    } else { 
                        zh_player.quest_target = (zh_player.location_id / 40) * 40 + (rand()%6); 
                        zh_player.quest_progress = (0 << 16) | 1;
                        const char* t_name = "未知区域";
                        for(int j=0; j<zh_data_locations_count; j++) if(zh_data_locations[j].id == zh_player.quest_target) t_name = zh_data_locations[j].name;
                        char buf[128]; snprintf(buf, sizeof(buf), "【接受委托 - 送货】\n帮我把这个加急包裹送到【%s】去。", t_name);
                        zh_log(buf);
                    }
                    lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
                    lv_async_call([](void*){ refresh_quest_ui(); }, NULL); 
                    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
                }, 0x16a085);
            }
            else if (zh_player.quest_id == 2) {
                add_n_btn("上交搜集物资", [](lv_event_t *e){
                    int req_cnt = zh_player.quest_progress & 0xFFFF;
                    int found = 0;
                    for(int i=0; i<50; i++) if(zh_player.inventory[i] == zh_player.quest_target) found++;
                    if(found >= req_cnt) {
                        int removed = 0;
                        for(int i=0; i<50 && removed < req_cnt; i++) {
                            if(zh_player.inventory[i] == zh_player.quest_target) { zh_player.inventory[i] = -1; removed++; }
                        }
                        zh_player.gold += 800; zh_player.reputation += 10;
                        zh_player.quest_id = 0; zh_player.quest_target = 0; zh_player.quest_progress = 0;
                        zh_log("【委托完成】你上交了物资！获得 800 铜贝与声望！");
                    } else zh_log("包里物资数量不够！再去野外找找！");
                    lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
                    lv_async_call([](void*){ refresh_quest_ui(); }, NULL); 
                    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
                }, 0x27ae60);
            }
            else if (zh_player.quest_id == 1) {
                int req_cnt = zh_player.quest_progress & 0xFFFF;
                int cur_cnt = (zh_player.quest_progress >> 16) & 0xFFFF;
                if(cur_cnt >= req_cnt) {
                    add_n_btn("汇报击杀(交付)", [](lv_event_t *e){
                        zh_player.gold += 1000; zh_player.reputation += 15;
                        zh_player.quest_id = 0; zh_player.quest_target = 0; zh_player.quest_progress = 0;
                        zh_log("【委托完成】感谢你保护了大家！获得 1000 铜贝与大量声望！");
                        lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
                        lv_async_call([](void*){ refresh_quest_ui(); }, NULL); 
                        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
                    }, 0x27ae60);
                }
            }
        }
        
        if (zh_player.quest_id == 3 && zh_player.location_id == zh_player.quest_target) {
            add_n_btn("递交包裹(完成送货)", [](lv_event_t *e){
                zh_player.gold += 500; zh_player.reputation += 8;
                zh_player.quest_id = 0; zh_player.quest_target = 0; zh_player.quest_progress = 0;
                zh_log("【委托完成】包裹已送达！收件人支付了 500 铜贝辛苦费。");
                lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
                lv_async_call([](void*){ refresh_quest_ui(); }, NULL); 
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            }, 0x27ae60);
        }

        if(npc->shop_type > 0 && npc->shop_type <= 3) {
            add_n_btn("浏览商品", [](lv_event_t *e){ open_npc_shop_ui(current_npc_idx); }, 0x27ae60);
            
            if (npc->shop_type == 1) {
                add_n_btn("请求锻造(消耗材料)", [](lv_event_t *e){
                    lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN);
                    open_forge_ui();
                }, 0xd35400); 
            }
        } else if (npc->shop_type == 4) { 
            add_n_btn("传授技能", [](lv_event_t *e){ open_npc_skill_shop_ui(current_npc_idx); }, 0x8e44ad);
        }
        
        if(npc->special_func > 0) {
            if(npc->special_func == 1) add_n_btn("祈祷疗伤 (20铜)", [](lv_event_t *e){ 
                if(zh_player.gold >= 20){ zh_player.gold -= 20; zh_player.hp = zh_player.max_hp; zh_player.mp = zh_player.max_mp; zh_player.crime_value = 0; zh_log("神父为你洗去了伤痛，并赦免了你的罪恶。"); } 
                else zh_log("钱不够。"); 
                lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
            }, 0x8e44ad);
            else if(npc->special_func == 2) add_n_btn("花钱消灾 (500铜)", [](lv_event_t *e){ 
                if(zh_player.crime_value <= 0) zh_log("你是个良民。"); 
                else if(zh_player.gold >= 500){ zh_player.gold -= 500; zh_player.crime_value = 0; zh_log("治安官收下了钱。"); } 
                else zh_log("没钱滚蛋！"); 
                lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
            }, 0x8e44ad);
            else if(npc->special_func == 3) add_n_btn("预测物价 (50铜)", [](lv_event_t *e){ 
                if(zh_player.gold >= 50){ zh_player.gold -= 50; zh_log("占星师：我看某些货品即将大涨..."); } 
                else zh_log("穷鬼走开。"); 
                lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
            }, 0x8e44ad);
        }

        add_n_btn("尝试盗窃", [](lv_event_t *e){
            const ZH_NPC*npc = &zh_data_npcs[current_npc_idx];
            if(rand() % 100 >= npc->steal_diff) {
                zh_player.gold += npc->steal_gold; zh_player.crime_value += 5;
                char buf[128]; snprintf(buf, sizeof(buf), "妙手空空！偷到了 %d 铜贝！", npc->steal_gold); zh_log(buf);
            } else {
                zh_player.hp -= 20; zh_player.crime_value += 30; zh_log("被当场抓获！挨了一顿打，罪恶飙升！");
                if(zh_player.hp <= 0) player_die();
            }
            lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
        }, 0xd35400);

        add_n_btn("发起攻击 (高危)", [](lv_event_t *e){
            const ZH_NPC*npc = &zh_data_npcs[current_npc_idx];
            if (zh_player.npc_status[npc->id] == 1) zh_log("你狠狠踢了一脚倒在地上的他，他痛苦地蜷缩起来。别打死人了！");
            else { 
                zh_player.npc_status[npc->id] = 1; zh_player.crime_value += 50; 
                char buf[256]; snprintf(buf, sizeof(buf), "你一拳打翻了 %s！对方倒地！\n你引发了严重的骚乱，罪恶值飙升！", npc->name); 
                zh_log(buf); 
            }
            lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
        }, 0xc0392b);

        add_n_btn("离开", [](lv_event_t *e){ lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); }, 0x7f8c8d);
        lv_obj_clear_flag(modal_npc, LV_OBJ_FLAG_HIDDEN);
    };

    int cur_region = get_current_region();
    int generic_npc_count = 0; 
    
    for(int i=0; i<zh_data_npcs_count; i++) {
        if (zh_data_npcs[i].region_id == 0 || zh_data_npcs[i].region_id == cur_region) {
            if (zh_data_npcs[i].bind_type == 1 && zh_data_npcs[i].bind_value == loc->id) {
                char buf[64]; snprintf(buf, sizeof(buf), "【人物】 %s", zh_data_npcs[i].name);
                add_act_btn(buf, open_npc_cb, 0x8e44ad, (void*)(intptr_t)i);
            } 
            else if (zh_data_npcs[i].bind_type == 2 && zh_data_npcs[i].bind_value == loc->action_type) {
                if (zh_data_npcs[i].id >= 100) {
                    if ((loc->id + zh_data_npcs[i].id) % 4 == 0) { 
                        if (generic_npc_count < 2) {
                            char buf[64]; snprintf(buf, sizeof(buf), "【路人】 %s", zh_data_npcs[i].name);
                            add_act_btn(buf, open_npc_cb, 0x9b59b6, (void*)(intptr_t)i);
                            generic_npc_count++;
                        }
                    }
                } else {
                    char buf[64]; snprintf(buf, sizeof(buf), "【人物】 %s", zh_data_npcs[i].name);
                    add_act_btn(buf, open_npc_cb, 0x8e44ad, (void*)(intptr_t)i);
                }
            }
        }
    }

    if (lv_obj_get_child_cnt(list_zh_action) == 0) {
        lv_obj_t * l = lv_label_create(list_zh_action); 
        lv_obj_add_style(l, &style_cn, 0); 
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0); 
        lv_label_set_text(l, "这里没什么可操作的..."); 
        lv_obj_center(l);
    }

    lv_obj_clean(list_zh_bag);
    lv_obj_t * filter_cont = lv_obj_create(list_zh_bag); 
    lv_obj_set_size(filter_cont, 220, 40); 
    lv_obj_set_style_bg_color(filter_cont, lv_color_hex(0x1a252f), 0); 
    lv_obj_set_style_border_width(filter_cont, 0, 0); 
    lv_obj_set_style_pad_all(filter_cont, 0, 0); 
    lv_obj_clear_flag(filter_cont, LV_OBJ_FLAG_SCROLLABLE);

    const char* filter_names[] = {"全部", "装备", "消耗", "特殊"};
    for(int i=0; i<4; i++) {
        lv_obj_t * btn_f = lv_btn_create(filter_cont); 
        lv_obj_set_size(btn_f, 50, 30); 
        lv_obj_align(btn_f, LV_ALIGN_LEFT_MID, i * 55, 0); 
        lv_obj_set_style_bg_color(btn_f, i == current_bag_filter ? lv_color_hex(0x2980b9) : lv_color_hex(0x34495e), 0);
        lv_obj_t * lbl_f = lv_label_create(btn_f); 
        lv_obj_add_style(lbl_f, &style_cn, 0); 
        lv_label_set_text(lbl_f, filter_names[i]); 
        lv_obj_center(lbl_f);
        lv_obj_add_event_cb(btn_f, [](lv_event_t *e){ 
            current_bag_filter = (int)(intptr_t)lv_event_get_user_data(e); 
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    if (current_bag_filter == 0 || current_bag_filter == 1) {
        auto handle_unequip = [&](int* eq_slot_ptr, const char* slot_name) {
            if(*eq_slot_ptr != -1) {
                ZH_Item item = get_item_by_id(*eq_slot_ptr);
                char buf[64]; snprintf(buf, sizeof(buf), "[穿戴中] %s: %s", slot_name, item.name);
                lv_obj_t * btn = lv_list_add_btn(list_zh_bag, LV_SYMBOL_MINUS, buf);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x552222), 0);
                lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
                if(lbl){ lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFD700), 0); }
                lv_obj_add_event_cb(btn, [](lv_event_t *e){
                    int* slot_ptr = (int*)lv_event_get_user_data(e);
                    for(int i=0; i<50; i++) {
                        if(zh_player.inventory[i] == -1) { 
                            zh_player.inventory[i] = *slot_ptr; 
                            *slot_ptr = -1; 
                            zh_log("装备已卸下放入背包。"); 
                            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
                            return; 
                        }
                    }
                    zh_log("背包已满，无法卸下！");
                }, LV_EVENT_CLICKED, (void*)eq_slot_ptr);
            }
        };
        handle_unequip(&zh_player.eq_head, "头盔"); 
        handle_unequip(&zh_player.eq_chest, "胸甲"); 
        handle_unequip(&zh_player.eq_legs, "护腿"); 
        handle_unequip(&zh_player.eq_shoes, "鞋子"); 
        handle_unequip(&zh_player.eq_weapon, "武器");
    }

    bool has_item = false;
    for (int i = 0; i < 50; i++) {
        if (zh_player.inventory[i] != -1) {
            ZH_Item item = get_item_by_id(zh_player.inventory[i]);
            bool show = false;
            if (current_bag_filter == 0) show = true;
            else if (current_bag_filter == 1 && item.type >= 1 && item.type <= 5) show = true;
            else if (current_bag_filter == 2 && (item.type == 6 || item.type == 7 || item.type == 9)) show = true;
            else if (current_bag_filter == 3 && item.type == 8) show = true;

            if (show) {
                has_item = true; 
                const char* icon = LV_SYMBOL_DUMMY; uint32_t color = 0x2c3e50; 
                if(item.type >= 1 && item.type <= 5) { icon = LV_SYMBOL_SETTINGS; color = 0x2980b9; } 
                else if(item.type == 6 || item.type == 7 || item.type == 9) { icon = LV_SYMBOL_CHARGE; color = 0x27ae60; } 
                else if(item.type == 8) { icon = LV_SYMBOL_WARNING; color = 0x8e44ad; }

                lv_obj_t * btn = lv_list_add_btn(list_zh_bag, icon, item.name); 
                lv_obj_set_style_bg_color(btn, lv_color_hex(color), 0);
                lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
                if(lbl){ lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
                
                lv_obj_add_event_cb(btn, [](lv_event_t *e){
                    selected_inventory_idx = (int)(intptr_t)lv_event_get_user_data(e);
                    ZH_Item i_ptr = get_item_by_id(zh_player.inventory[selected_inventory_idx]);
                    char title_buf[128]; snprintf(title_buf, sizeof(title_buf), "【%s】", i_ptr.name); 
                    lv_label_set_text(lbl_item_detail_title, title_buf);
                    
                    static char desc_buf[512];
                    if (i_ptr.type >= 1 && i_ptr.type <= 5) { 
                        const char* slots[] = {"武器", "头盔", "胸甲", "护腿", "鞋子"}; 
                        snprintf(desc_buf, sizeof(desc_buf), "部位: %s\n属性值: +%d\n售价: %d 铜贝\n\n点击装备替换当前部位。", slots[i_ptr.type-1], i_ptr.value, i_ptr.price); 
                    } 
                    else if (i_ptr.type == 6 || i_ptr.type == 7) {
                        snprintf(desc_buf, sizeof(desc_buf), "神奇药水 (部分草药可直接使用)\n效果: 提升/恢复 %d 点数值\n售价: %d\n\n咕噜咕噜...", i_ptr.value, i_ptr.price);
                    }
                    else if (i_ptr.type == 9) {
                        snprintf(desc_buf, sizeof(desc_buf), "【战斗秘宝】\n售价: %d 铜贝\n\n注意：此物品极度危险/珍贵，只能在【战斗界面】中点击使用！", i_ptr.price);
                    }
                    else if (i_ptr.type == 8) {
                        if (i_ptr.id == 201 || i_ptr.id == 206 || i_ptr.id == 216 || i_ptr.id == 217 || i_ptr.id == 237 || i_ptr.id == 241 || i_ptr.id == 249 || i_ptr.id == 250) { 
                            snprintf(desc_buf, sizeof(desc_buf), "【消耗型异宝】\n售价: %d 铜贝\n\n点击[使用]释放力量，改变命运！", i_ptr.price); 
                        } 
                        else if (i_ptr.id >= 401 && i_ptr.id <= 410) {
                            snprintf(desc_buf, sizeof(desc_buf), "【原矿石】\n售价: %d 铜贝\n\n可用于铁匠铺锻造绝世神兵。", i_ptr.price);
                        }
                        else if (i_ptr.id >= 421 && i_ptr.id <= 445) {
                            snprintf(desc_buf, sizeof(desc_buf), "【锻造材料】\n售价: %d 铜贝\n\n珍稀的野兽素材，锻造神装必备。", i_ptr.price);
                        }
                        else {
                            snprintf(desc_buf, sizeof(desc_buf), "【被动/任务异宝】\n售价: %d 铜贝\n\n带在包里有惊人的被动加成或用于交任务。", i_ptr.price);
                        }
                    } else {
                        snprintf(desc_buf, sizeof(desc_buf), "特殊道具\n价值连城。");
                    }
                    
                    lv_label_set_text(lbl_item_detail_desc, desc_buf); 
                    lv_obj_clear_flag(modal_item_detail, LV_OBJ_FLAG_HIDDEN);
                }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
            }
        }
    }
    if (!has_item) { 
        lv_obj_t * l = lv_label_create(list_zh_bag); 
        lv_obj_add_style(l, &style_cn, 0); 
        lv_obj_set_style_text_color(l, lv_color_hex(0x95a5a6), 0); 
        lv_label_set_text(l, "\n没有符合条件的物品"); 
        lv_obj_center(l); 
    }
}

// -------------------------------------------------------------------------
// 场景与界面构建逻辑
// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
// 存档管理与重置
// -------------------------------------------------------------------------
void reset_zongheng_game() {
    strcpy(zh_player.name, "水手"); 
    zh_player.level = 1; zh_player.exp = 0; zh_player.max_hp = 100; zh_player.hp = 100; zh_player.atk = 15; zh_player.def = 5;
    zh_player.max_mp = 100; zh_player.mp = 100; zh_player.base_crit = 5; zh_player.base_dodge = 5;
    for(int i = 0; i < 160; i++) zh_player.learned_skills[i] = 0;
    for(int i = 0; i < 12; i++) zh_player.eq_active_skills[i] = 0;
    for(int i = 0; i < 4; i++) zh_player.eq_passive_skills[i] = 0;

    for(int i = 0; i < 10; i++) zh_player.adjutants_roster[i] = 0;
    zh_player.eq_adj_navigator = 0; zh_player.eq_adj_assault = 0; zh_player.eq_adj_doctor = 0; zh_player.eq_adj_accountant = 0;

    zh_player.gold = 500; zh_player.silver = 0; zh_player.bank_gold = 0; zh_player.debt = 0;
    zh_player.crime_value = 0; zh_player.reputation = 0;
    zh_player.welfare_flag = 0;
    zh_player.location_id = 0;
    
    zh_player.active_bounty_id = -1; 
    zh_player.quest_id = 0; zh_player.quest_target = 0; zh_player.quest_progress = 0; 
    
    for(int i = 0; i < MAX_STORY_QUESTS; i++) { zh_player.story_status[i] = 0; zh_player.story_progress[i] = 0; zh_player.story_counter[i] = 0; }

    zh_player.eq_weapon = -1; zh_player.eq_head = -1; zh_player.eq_chest = -1; zh_player.eq_legs = -1; zh_player.eq_shoes = -1;
    for(int i = 0; i < 50; i++) zh_player.inventory[i] = -1;
    for(int i = 0; i < ZH_NUM_GOODS; i++) { zh_player.goods_inventory[i] = 0; zh_player.goods_buy_price[i] = 0; }
    for(int i = 0; i < 5; i++) { zh_player.current_monsters[i] = -1; zh_player.current_monsters_hp[i] = 0; }
    for(int i = 0; i < 4; i++) zh_player.npc_gift_bits[i] = 0;
    for(int i = 0; i < 128; i++) zh_player.npc_status[i] = 0;

    refresh_market_prices(); 
    spawn_monsters_for_loc(zh_player.location_id);
    refresh_quest_ui(); 
    zh_log("【新的旅程】\n目标是星辰大海！"); 
    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
}

bool has_zongheng_save() { return LittleFS.exists("/zh_save.dat"); }

void save_zongheng_game() {
    File file = LittleFS.open("/zh_save_temp.dat", FILE_WRITE);
    if (!file) { zh_log("系统：存档失败！"); return; }
    size_t written = file.write((uint8_t*)&zh_player, sizeof(zh_player));
    file.close();
    
    if (written == sizeof(zh_player)) {
        if (LittleFS.rename("/zh_save_temp.dat", "/zh_save.dat")) {
            zh_log("系统：游戏进度已保存。");
        } else {
            zh_log("系统：重命名失败，存档异常！");
            LittleFS.remove("/zh_save_temp.dat");
        }
    } else {
        zh_log("系统：写入异常，存档终止！");
        LittleFS.remove("/zh_save_temp.dat");
    }
}

bool load_zongheng_game() { 
    if (!LittleFS.exists("/zh_save.dat")) return false; 
    File file = LittleFS.open("/zh_save.dat", FILE_READ); 
    if (!file) return false; 
    
    for(int i = 0; i < 4; i++) zh_player.npc_gift_bits[i] = 0; 
    for(int i = 0; i < 128; i++) zh_player.npc_status[i] = 0;
    
    memset(zh_player.story_status, 0, sizeof(zh_player.story_status));
    memset(zh_player.story_progress, 0, sizeof(zh_player.story_progress));
    memset(zh_player.story_counter, 0, sizeof(zh_player.story_counter));

    if (file.size() < sizeof(zh_player)) {
        for(int i = 0; i < 10; i++) zh_player.adjutants_roster[i] = 0;
        zh_player.eq_adj_navigator = 0; zh_player.eq_adj_assault = 0; zh_player.eq_adj_doctor = 0; zh_player.eq_adj_accountant = 0;
    }

    file.read((uint8_t*)&zh_player, file.size() < sizeof(zh_player) ? file.size() : sizeof(zh_player)); 
    file.close(); 
    
    if(zh_player.max_mp <= 0 || zh_player.max_mp > 1000000 || zh_player.level <= 0 || zh_player.level > 999) {
        zh_player.max_mp = 100 + zh_player.level * 10; zh_player.mp = zh_player.max_mp;
        zh_player.base_crit = 5; zh_player.base_dodge = 5;
        for(int i = 0; i < 160; i++) zh_player.learned_skills[i] = 0;
        for(int i = 0; i < 12; i++) zh_player.eq_active_skills[i] = 0;
        for(int i = 0; i < 4; i++) zh_player.eq_passive_skills[i] = 0;
    }
    
    bool needs_monster_reset = false;
    for(int i = 0; i < 5; i++) { 
        if(zh_player.current_monsters[i] < -1 || zh_player.current_monsters[i] >= zh_data_monsters_count) { 
            needs_monster_reset = true; break; 
        } 
    }
    if(needs_monster_reset) spawn_monsters_for_loc(zh_player.location_id);
    
    refresh_market_prices(); 
    const ZH_Location* loc = get_current_loc(); 
    refresh_quest_ui();
    
    char buf[128]; 
    snprintf(buf, sizeof(buf), "【读取存档】\n你回到了 %s。", loc->name); 
    zh_log(buf); 
    
    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); 
    return true;
}