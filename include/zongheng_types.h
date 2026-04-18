#pragma once

#include <stdint.h>
#include <cstddef>
#include <lvgl.h>

// 宏定义
#define ZH_NUM_GOODS 245
#define MAX_STORY_QUESTS 120

// 枚举定义
enum SkillType 
{ 
    SKILL_ACTIVE_PHYSICAL = 1, 
    SKILL_ACTIVE_MAGIC = 2, 
    SKILL_ACTIVE_HEAL = 3, 
    SKILL_ACTIVE_BUFF = 4, 
    SKILL_PASSIVE_STAT = 5, 
    SKILL_PASSIVE_TRIG = 6 
};

enum AdjutantType 
{ 
    ADJ_NAVIGATOR = 1, 
    ADJ_ASSAULT = 2, 
    ADJ_DOCTOR = 3, 
    ADJ_ACCOUNTANT = 4 
};

// 结构体定义
struct ZH_Location 
{ 
    int id; 
    const char* name; 
    const char* desc; 
    int links[4]; 
    int action_type; 
    int param1; 
    int param2; 
};

struct ZH_NPC 
{ 
    int id; 
    const char* name; 
    const char* dialogs[5]; 
    int bind_type; 
    int bind_value; 
    int region_id; 
    int shop_type; 
    int special_func; 
    int gift_item_id; 
    int steal_gold; 
    int steal_diff; 
};

struct ZH_Monster 
{ 
    int id; 
    const char* name; 
    int level; 
    int hp; 
    int atk; 
    int def; 
    int exp_drop; 
    int gold_drop; 
    bool is_aggressive; 
    int region_id; 
};

struct ZH_Item 
{ 
    int id; 
    const char* name; 
    int type; 
    int value; 
    int price; 
};

struct ZH_Skill 
{ 
    int id; 
    const char* name; 
    int type; 
    int region; 
    int mp_cost; 
    float power_mult; 
    int effect_value; 
    const char* desc; 
};

struct ZH_Recipe 
{
    int result_item_id;
    int mat1_id;
    int mat1_count;
    int mat2_id;
    int mat2_count;
    int cost_gold;
};

struct ZH_Adjutant 
{ 
    int id; 
    const char* name; 
    int type;
    int rarity;
    float power_mult;
    const char* desc; 
};

struct ZH_Player 
{
    char name[32];
    int level; 
    int exp; 
    int hp; 
    int max_hp; 
    int mp; 
    int max_mp;        
    int atk; 
    int def; 
    int base_crit; 
    int base_dodge;            
    long gold; 
    long silver; 
    long bank_gold; 
    long debt; 
    
    int crime_value; 
    int reputation;

    int location_id;
    int eq_weapon; 
    int eq_head; 
    int eq_chest; 
    int eq_legs; 
    int eq_shoes;    
    
    int welfare_flag;
    int quest_id; 
    int quest_progress; 
    int quest_target;
    
    uint8_t story_status[MAX_STORY_QUESTS];
    uint8_t story_progress[MAX_STORY_QUESTS];
    uint16_t story_counter[MAX_STORY_QUESTS];
    
    int active_bounty_id;

    int inventory[50]; 
    int goods_inventory[ZH_NUM_GOODS]; 
    int goods_buy_price[ZH_NUM_GOODS]; 
    int current_monsters[5];    
    int current_monsters_hp[5]; 
    uint32_t npc_gift_bits[4];  
    uint8_t npc_status[128];    
    
    int learned_skills[160];   
    int eq_active_skills[12];  
    int eq_passive_skills[4];  

    int adjutants_roster[10];
    int eq_adj_navigator;
    int eq_adj_assault;
    int eq_adj_doctor;
    int eq_adj_accountant;
};

// 外部变量声明
extern const ZH_Recipe zh_data_recipes[];
extern const int zh_data_recipes_count;
extern const ZH_Location zh_data_locations[];
extern const int zh_data_locations_count;
extern const ZH_NPC zh_data_npcs[];
extern const int zh_data_npcs_count;
extern const ZH_Monster zh_data_monsters[];
extern const int zh_data_monsters_count;
extern const char* zh_goods_names[ZH_NUM_GOODS];
extern const ZH_Item zh_data_fixed_items[];
extern const int zh_data_fixed_items_count;
extern const ZH_Skill zh_data_skills[];
extern const int zh_data_skills_count;
extern const ZH_Adjutant zh_data_adjutants[];
extern const int zh_data_adjutants_count;
extern ZH_Player zh_player;
extern int current_goods_prices[ZH_NUM_GOODS];
extern int zh_market_state;
extern int current_npc_idx;
extern lv_obj_t * modal_npc;

// 函数声明
const ZH_Adjutant* get_adjutant_by_id(int id);
extern void zh_log(const char * msg);
extern void refresh_zongheng_ui();
extern void player_die();
extern void start_turn_based_combat(int monster_idx);
ZH_Item get_item_by_id(int id);
int get_random_drop_id();
bool has_item_in_bag(int item_id);
bool add_item_to_bag(int item_id);
bool has_sailor_set();
bool has_steel_set();
int get_total_atk();
int get_total_def();
void trigger_random_land_event(char* current_log_buf, size_t buf_size);
void trigger_sailing_event(char* msg, size_t buf_size);
void refresh_market_prices();
void process_port_switch();
void build_market_and_shop_ui(lv_obj_t * parent_scr);
void refresh_market_action_list(lv_obj_t * list_zh_action, const ZH_Location* loc);
void open_npc_shop_ui(int npc_idx);
void open_npc_skill_shop_ui(int npc_idx);
void reset_quest_ui_pointers();
void build_quest_ui(lv_obj_t* screen, lv_obj_t* parent_tab);
void refresh_quest_ui();
void process_quest_kill(int monster_id);
int check_bounty_spawn(int loc_id);
void open_tavern_bounty_board();