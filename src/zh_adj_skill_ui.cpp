#include "global.h"
#include <stdio.h>
#include <string.h>

// ==========================================
// 副官数据表 (Adjutant Data)
// ==========================================
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

// =========================================================================
// 前向声明
// =========================================================================
void open_skill_selector();

// =========================================================================
// 外部依赖声明
// =========================================================================
extern ZH_Player zh_player;
extern lv_obj_t * list_adj_slots;
extern lv_obj_t * list_adj_roster;
extern int editing_adj_type;
extern lv_obj_t * list_skill_slots;
extern lv_obj_t * list_learned_skills;
extern lv_obj_t * modal_zh_skill_select;
extern int editing_skill_type;
extern int editing_skill_idx;
extern void zh_log(const char * msg);
extern void refresh_zongheng_ui();
extern const ZH_Skill* get_skill_ptr(int id);
extern lv_style_t style_cn;

// =========================================================================
// 副官系统 UI 逻辑
// =========================================================================
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

// =========================================================================
// 技能系统 UI 逻辑
// =========================================================================
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