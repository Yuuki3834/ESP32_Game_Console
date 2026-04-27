#include "global.h"
#include <stdio.h>
#include <string.h>

// =========================================================================
// 外部依赖声明
// =========================================================================
extern ZH_Player zh_player;
extern lv_obj_t * scr_zongheng;
extern lv_obj_t * modal_forge;
extern lv_obj_t * list_forge_recipes;
extern void zh_log(const char * msg);
extern void refresh_zongheng_ui();
extern ZH_Item get_item_by_id(int id);
extern bool add_item_to_bag(int item_id);
extern const ZH_Recipe zh_data_recipes[];
extern const int zh_data_recipes_count;
extern lv_style_t style_cn;

// =========================================================================
// 锻造系统逻辑
// =========================================================================
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

void open_forge_ui() {
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
