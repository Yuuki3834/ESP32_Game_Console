#include "global.h"
#include <stdio.h>
#include <string.h>
#include <LittleFS.h>

// =========================================================================
// 外部依赖声明
// =========================================================================
extern ZH_Player zh_player;
extern void zh_log(const char * msg);
extern void refresh_zongheng_ui();
extern void spawn_monsters_for_loc(int loc_id);
extern void refresh_market_prices();
extern void refresh_quest_ui();
extern const ZH_Location* get_current_loc();

// =========================================================================
// 存档管理与重置
// =========================================================================
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