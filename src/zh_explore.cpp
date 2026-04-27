#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// =========================================================================
// 外部依赖声明
// =========================================================================
extern ZH_Player zh_player;
extern lv_obj_t * scr_zongheng;
extern lv_obj_t * modal_zh_map;
extern lv_obj_t * list_map_locs;
extern lv_obj_t * modal_sailing;
extern lv_obj_t * lbl_sailing_info;
extern int sailing_target_id;
extern int sailing_origin_id;
extern int zh_market_state;
extern void zh_log(const char * msg);
extern void refresh_zongheng_ui();
extern void start_turn_based_combat(int monster_idx);
extern const ZH_Location* get_current_loc();
extern int get_current_region();
extern void trigger_sailing_event(char* msg, size_t buf_size);
extern void process_port_switch();
extern int check_bounty_spawn(int loc_id);
extern lv_style_t style_cn;

// =========================================================================
// 探索与地图逻辑
// =========================================================================
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
