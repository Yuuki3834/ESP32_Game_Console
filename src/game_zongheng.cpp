#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// =========================================================================
// 外部依赖声明 
// =========================================================================
extern ZH_Player zh_player;
extern int current_npc_idx;
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
extern int zh_market_state;

extern void zh_log(const char * msg);
extern const ZH_Location* get_current_loc();
extern int get_current_region();
extern void spawn_monsters_for_loc(int loc_id);
extern void execute_combat(int monster_idx);
extern void start_sailing(int origin_id, int target_id);
extern void start_turn_based_combat(int monster_idx);
extern void player_die();
extern void open_forge_ui();
extern void open_npc_shop_ui(int npc_idx);
extern void open_npc_skill_shop_ui(int npc_idx);
extern void open_tavern_bounty_board();
extern void refresh_market_action_list(lv_obj_t * list_zh_action, const ZH_Location* loc);
extern void refresh_quest_ui();
extern void trigger_random_land_event(char* current_log_buf, size_t buf_size);
extern bool add_item_to_bag(int item_id);
extern ZH_Item get_item_by_id(int id);
extern int get_total_atk();
extern int get_total_def();
extern bool has_sailor_set();
extern bool has_steel_set();
extern const ZH_Adjutant* get_adjutant_by_id(int id);
extern lv_style_t style_cn;

// =========================================================================
// 核心刷新逻辑
// =========================================================================
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
                        zh_player.quest_target = (rand()%2==0)? 108 : 401;
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