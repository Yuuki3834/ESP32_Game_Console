#include "global.h"
#include <stdlib.h>
#include <LittleFS.h>

lv_obj_t * scr_beijing = NULL;

#define MAX_DAYS 40
#define NUM_ITEMS 30
#define NUM_LOCATIONS 10

struct InventoryItem {
    int count;
    int avg_buy_price;
};

struct BJ_Item {
    const char* name;
    int base_price;
    int volatility;
    int current_price;
    int source_loc;
};

struct BJ_Player {
    int day;
    long cash;
    long bank;
    long debt;
    float debt_rate;
    int health;
    int storage;
    int max_storage;
    int location;
    InventoryItem inventory[NUM_ITEMS];
    char rumor[128];
};

BJ_Player bj;
BJ_Item items[NUM_ITEMS] = {
    {"进口香烟", 150, 50, 0, -1},
    {"盗版VCD", 30, 15, 0, -1},
    {"水货手机", 1200, 400, 0, -1},
    {"假冒化妆品", 400, 150, 0, -1},
    {"走私汽车", 15000, 5000, 0, -1},
    {"伪劣玩具", 80, 30, 0, -1},
    {"进口奶粉", 250, 80, 0, -1},
    {"三株口服液", 500, 250, 0, -1},
    {"貂皮大衣", 4000, 1500, 0, -1},
    {"走私大彩电", 3000, 1000, 0, -1},
    {"认购证", 5000, 2000, 0, -1},
    {"猴年邮票", 8000, 3000, 0, -1},
    {"二手BP机", 300, 100, 0, -1},
    {"山寨名表", 2000, 600, 0, -1},
    {"高档假酒", 1000, 400, 0, -1},
    {"小霸王学习机", 150, 50, 0, -1},
    {"港台明星海报", 10, 5, 0, -1},
    {"健力宝饮料", 20, 8, 0, -1},
    {"俄罗斯套娃", 100, 40, 0, -1},
    {"奥运纪念章", 800, 300, 0, -1},
    {"王府井丝绸", 500, 200, 0, 0},
    {"中关村盗版盘", 50, 20, 0, 1},
    {"西直门水管", 120, 40, 0, 2},
    {"秀水街A货包", 600, 250, 0, 3},
    {"复兴门打口磁带", 15, 5, 0, 4},
    {"北京站黄牛票", 200, 150, 0, 5},
    {"苹果园首钢废钢", 400, 100, 0, 6},
    {"公主坟手机壳", 40, 15, 0, 7},
    {"积水潭旧医书", 80, 30, 0, 8},
    {"崇文门旧家具", 350, 150, 0, 9}
};

const char* locations[NUM_LOCATIONS] = {
    "王府井", "中关村", "西直门", "建国门", "复兴门",
    "北京站", "苹果园", "公主坟", "积水潭", "崇文门"
};

lv_obj_t *lbl_bj_status, *tab_market, *tab_bag, *tab_move, *tab_place;
lv_obj_t *list_market, *list_bag, *modal_msg, *lbl_msg_content, *menu_overlay;
lv_obj_t *modal_trade, *lbl_trade_title, *lbl_trade_info, *lbl_trade_qty;
static int trade_idx = -1;
static bool is_buying_mode = true;
static int trade_qty = 0;
static int max_qty = 0;
lv_obj_t *modal_sub, *list_sub, *lbl_sub_title;
void show_bj_msg(const char* msg) {
    lv_label_set_text(lbl_msg_content, msg);
    lv_obj_clear_flag(modal_msg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(modal_msg);
}

void randomize_prices() {
    strcpy(bj.rumor, "今天风平浪静，没有特别的消息。");
    for(int i = 0; i < NUM_ITEMS; i++) {
        int var = 0;
        if (items[i].volatility > 0) {
            var = (rand() % (items[i].volatility * 2)) - items[i].volatility;
        }
        items[i].current_price = items[i].base_price + var;
        if(items[i].current_price < 5) items[i].current_price = 5; 
        if(items[i].source_loc != -1 && items[i].source_loc != bj.location) {
            items[i].current_price *= 2; 
        }
    }

    int r = rand() % 100;
    if (r < 8) { 
        items[0].current_price *= (3 + rand()%3); items[4].current_price *= (4 + rand()%5);
        strcpy(bj.rumor, "海关严防死守！香烟和走私汽车黑市价格狂飙！");
    } else if (r < 16) { 
        items[3].current_price /= 4; items[7].current_price /= 4;
        strcpy(bj.rumor, "315晚会重拳出击，假冒化妆品和三株口服液无人问津！");
    } else if (r < 22) { 
        items[1].current_price *= (3 + rand()%3); items[21].current_price *= (2 + rand()%3);
        strcpy(bj.rumor, "严打盗版！市面上的盗版VCD和光盘被扫荡一空！");
    } else if (r < 28) { 
        items[14].current_price /= 5; items[8].current_price /= 3;
        strcpy(bj.rumor, "曝光！高档假酒喝出人命，假貂皮掉毛，价格跌落谷底！");
    } else if (r < 34) { 
        items[10].current_price *= (2 + rand()%3); items[11].current_price *= (2 + rand()%3);
        strcpy(bj.rumor, "炒作热潮来袭！认购证和猴票遭大妈们疯抢！");
    } else if (r < 40) {
        items[15].current_price *= (3 + rand()%2); items[12].current_price *= (2 + rand()%2);
        strcpy(bj.rumor, "新一波科技热！小霸王学习机和二手BP机供不应求！");
    } else if (r < 45) {
        items[25].current_price /= 4; 
        strcpy(bj.rumor, "火车站派出所严打票贩子！黄牛票全砸手里了！");
    } else if (r < 55) { 
        int idx; do { idx = rand() % NUM_ITEMS; } while (items[idx].source_loc != -1);
        items[idx].current_price = items[idx].base_price / (3 + rand()%3);
        snprintf(bj.rumor, 128, "大甩卖：%s 价格跳水，市场恐慌大抛售！", items[idx].name);
    } else if (r < 65) { 
        int idx; do { idx = rand() % NUM_ITEMS; } while (items[idx].source_loc != -1);
        items[idx].current_price = items[idx].base_price * (2 + rand()%4);
        snprintf(bj.rumor, 128, "黑市急需！%s 有价无市，有货的都发大财了！", items[idx].name);
    }
}

void refresh_beijing_ui();

void check_end_game() {
    if (bj.health <= 0) {
        show_bj_msg("你积劳成疾，倒在了北京的寒风中...\n游戏提前结束！");
        bj.day = 999;
    } else if (bj.day > MAX_DAYS) {
        long total = bj.cash + bj.bank - bj.debt;
        const char* title;
        if (total < 0) title = "负债累累：被遣送回乡，打工还债。";
        else if (total < 50000) title = "平平淡淡：勉强在北京吃饱了饭。";
        else if (total < 500000) title = "小康生活：在五环外按揭了一套小房。";
        else if (total < 5000000) title = "商业奇才：成立了自己的贸易公司！";
        else title = "一代传奇：你成了京城首屈一指的新巨头！";
        
        static char buf[512];
        snprintf(buf, sizeof(buf), "40天期满！\n最终净资产: %ld 块\n结局: %s", total, title);
        show_bj_msg(buf);
        bj.day = 999;
    }
}

void next_day(int new_loc) {
    if (bj.day > MAX_DAYS) return;
    bj.day++;
    bj.location = new_loc;
    
    bj.bank += (long)((long long)bj.bank * 5LL / 100LL);
    bj.debt += (long)((long long)bj.debt * (long long)(bj.debt_rate * 1000) / 1000LL);
    
    int pr = rand() % 100;
    static char event_msg[256];
    event_msg[0] = '\0'; // 每次进入函数时清空首字符
    
    if (pr < 5) {
        bj.cash += 3000; 
        snprintf(event_msg, sizeof(event_msg), "偶遇曾经的发小！\n现在人家发财了，硬塞给你 3000 块茶水钱！");
    } else if (pr < 10) {
        int lost = bj.cash * 0.15 + 100; if (bj.cash < lost) lost = bj.cash;
        bj.cash -= lost; bj.health -= 15;
        snprintf(event_msg, sizeof(event_msg), "糟糕！在天桥遇到地痞流氓打劫，\n损失了 %d 块钱，还挨了一顿胖揍！", lost);
    } else if (pr < 15) {
        int found = 500 + rand() % 1000; bj.cash += found;
        snprintf(event_msg, sizeof(event_msg), "走狗屎运了！\n在路边捡到一个鼓鼓的钱包，白赚 %d 块！", found);
    } else if (pr < 20) {
        int lost = 800; if (bj.cash < lost) lost = bj.cash; bj.cash -= lost;
        snprintf(event_msg, sizeof(event_msg), "被仙人跳了！\n在潘家园贪便宜买假古董，被坑了 %d 块钱！", lost);
    } else if (pr < 25) {
        bj.health -= 25; 
        snprintf(event_msg, sizeof(event_msg), "吃了路边不干净的卤煮火烧，\n上吐下泻一整天，健康 -25！");
    } else if (pr < 30) {
        if(bj.health < 100) { bj.health = 100; snprintf(event_msg, sizeof(event_msg), "遇到了胡同里的老中医，\n几副中药下去，你的健康恢复满了！"); }
    } else if (pr < 35) {
        bj.max_storage += 10;
        snprintf(event_msg, sizeof(event_msg), "好消息！\n房东大妈看你可怜，给你换了个大点的柜子，\n存储空间永久 +10！");
    } else if (pr < 40) {
        if(bj.cash >= 500) {
            bj.cash -= 500;
            snprintf(event_msg, sizeof(event_msg), "你在西直门立交桥彻底迷路了！\n被黑车司机疯狂绕路，打车费被坑了 500 块！");
        }
    } else if (pr < 43) {
        if(bj.debt > 0) {
            long forgive = bj.debt * 0.2;
            bj.debt -= forgive;
            snprintf(event_msg, sizeof(event_msg), "黑市老大今天心情好！\n大手一挥，免了你 %ld 块钱的高利贷！", forgive);
        }
    }

    randomize_prices(); refresh_beijing_ui();
    if (event_msg[0] != '\0') show_bj_msg(event_msg); 
    check_end_game();
}

bool has_beijing_save() {
    return LittleFS.exists("/beijing.sav");
}

void save_beijing_game() {
    // 先写入临时文件
    File file = LittleFS.open("/beijing_temp.sav", FILE_WRITE);
    if (!file) { show_bj_msg("保存失败！"); return; }
    size_t written = file.write((uint8_t*)&bj, sizeof(bj));
    file.close();
    
    // 验证写入完整性
    if (written == sizeof(bj)) {
        // 删除旧存档（如果存在）
        if (LittleFS.exists("/beijing.sav")) {
            LittleFS.remove("/beijing.sav");
        }
        // 原子性重命名临时文件为正式文件
        if (LittleFS.rename("/beijing_temp.sav", "/beijing.sav")) {
            show_bj_msg("游戏进度已成功保存至 LittleFS！");
        } else {
            show_bj_msg("重命名失败，存档异常！");
        }
    } else {
        show_bj_msg("写入异常，存档终止！");
        // 清理临时文件
        LittleFS.remove("/beijing_temp.sav");
    }
}

bool load_beijing_game() {
    if (!LittleFS.exists("/beijing.sav")) return false;
    File file = LittleFS.open("/beijing.sav", FILE_READ);
    if (!file) return false;
    if (file.size() != sizeof(bj)) { file.close(); return false; }
    file.read((uint8_t*)&bj, sizeof(bj));
    file.close();
    refresh_beijing_ui();
    return true;
}

void populate_sub_menu(int type) {
    lv_obj_clean(list_sub);
    if(type == 0) {
        lv_label_set_text(lbl_sub_title, "银行营业厅");
        lv_obj_t *b1 = lv_list_add_btn(list_sub, NULL, "存入所有现金");
        lv_obj_add_event_cb(b1, [](lv_event_t *e){
            if(bj.cash>0){ bj.bank+=bj.cash; bj.cash=0; show_bj_msg("现金已存入，享受每日5%利息"); }
            else show_bj_msg("你身上连个钢镚都没有！");
            refresh_beijing_ui(); lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, NULL);

        lv_obj_t *b2 = lv_list_add_btn(list_sub, NULL, "取出所有存款");
        lv_obj_add_event_cb(b2, [](lv_event_t *e){
            if(bj.bank>0){ bj.cash+=bj.bank; bj.bank=0; show_bj_msg("存款已全部取出，拿稳了！"); }
            else show_bj_msg("账单余额为0！");
            refresh_beijing_ui(); lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, NULL);

    } else if(type == 1) {
        lv_label_set_text(lbl_sub_title, "黑市胡同");
        lv_obj_t *b1 = lv_list_add_btn(list_sub, NULL, "借款 5000块");
        lv_obj_add_event_cb(b1, [](lv_event_t *e){
            if(bj.debt > 0) {
                bj.debt_rate = 0.10f; 
                show_bj_msg("黑市：你小子还欠着钱呢！\n这笔算你 10% 的高利贷！");
            } else {
                bj.debt_rate = 0.075f;
                show_bj_msg("拿钱爽快，每日 7.5% 利息！");
            }
            bj.debt += 5000; bj.cash += 5000;
            refresh_beijing_ui(); lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, NULL);

        lv_obj_t *b2 = lv_list_add_btn(list_sub, NULL, "偿还欠款");
        lv_obj_add_event_cb(b2, [](lv_event_t *e){
            if(bj.debt==0) { show_bj_msg("你又不欠钱，还什么？"); }
            else if(bj.cash==0) { show_bj_msg("没钱还债，小心被砍手！"); }
            else {
                long pay = bj.cash > bj.debt ? bj.debt : bj.cash;
                bj.cash -= pay; bj.debt -= pay;
                if(bj.debt==0) {
                    bj.debt_rate = 0.075f; 
                    show_bj_msg("无债一身轻！欠款已结清！");
                }
                else show_bj_msg("钱不够，只还了一部分，剩下的继续利滚利。");
            }
            refresh_beijing_ui(); lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, NULL);

    } else if(type == 2) {
        lv_label_set_text(lbl_sub_title, "房屋中介");
        lv_obj_t *b1 = lv_list_add_btn(list_sub, NULL, "+20空间 (2500块)");
        lv_obj_add_event_cb(b1, [](lv_event_t *e){
            if(bj.cash>=2500){ bj.cash-=2500; bj.max_storage+=20; show_bj_msg("租了个小单间，空间 +20"); }
            else show_bj_msg("没钱免谈！");
            refresh_beijing_ui(); lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, NULL);

        lv_obj_t *b2 = lv_list_add_btn(list_sub, NULL, "+50空间 (5000块)");
        lv_obj_add_event_cb(b2, [](lv_event_t *e){
            if(bj.cash>=5000){ bj.cash-=5000; bj.max_storage+=50; show_bj_msg("租了个地下室，空间 +50"); }
            else show_bj_msg("没钱免谈！");
            refresh_beijing_ui(); lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, NULL);

        lv_obj_t *b3 = lv_list_add_btn(list_sub, NULL, "+100空间 (8000块)");
        lv_obj_add_event_cb(b3, [](lv_event_t *e){
            if(bj.cash>=8000){ bj.cash-=8000; bj.max_storage+=100; show_bj_msg("租了大平层，空间+100，划算！"); }
            else show_bj_msg("没钱免谈！");
            refresh_beijing_ui(); lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, NULL);
    }
    
    for(uint32_t i=0; i<lv_obj_get_child_cnt(list_sub); i++) {
        lv_obj_t * child = lv_obj_get_child(list_sub, i);
        lv_obj_set_style_bg_color(child, lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_color(child, lv_color_hex(0x555555), 0);
        lv_obj_t * lbl = lv_obj_get_child(child, 0); 
        if (lbl) {
            lv_obj_add_style(lbl, &style_cn, 0);
            lv_obj_set_style_text_color(lbl, lv_color_hex(0xE0E0E0), 0);
        }
    }
    lv_obj_clear_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(modal_sub);
}

void refresh_trade_qty() {
    if (trade_qty < 0) trade_qty = 0;
    if (trade_qty > max_qty) trade_qty = max_qty;
    
    char buf[64];
    snprintf(buf, sizeof(buf), "数 量 :  %d", trade_qty);
    lv_label_set_text(lbl_trade_qty, buf);
    
    if (is_buying_mode) {
        long cost = (long)trade_qty * items[trade_idx].current_price;
        snprintf(buf, sizeof(buf), "单价: %d 块\n总价: %ld 块\n空间余量: %d", 
                items[trade_idx].current_price, cost, bj.max_storage - bj.storage);
    } else {
        long earn = (long)trade_qty * items[trade_idx].current_price;
        snprintf(buf, sizeof(buf), "现价: %d 块\n总计: %ld 块\n买入均价: %d", 
                items[trade_idx].current_price, earn, bj.inventory[trade_idx].avg_buy_price);
    }
    lv_label_set_text(lbl_trade_info, buf);
}

void open_trade_modal(int idx, bool is_buy) {
    trade_idx = idx;
    is_buying_mode = is_buy;
    
    if (is_buy) {
        int max_by_cash = bj.cash / items[idx].current_price;
        int max_by_storage = bj.max_storage - bj.storage;
        max_qty = max_by_cash < max_by_storage ? max_by_cash : max_by_storage;
        char buf[64]; snprintf(buf, sizeof(buf), "买入 - %s", items[idx].name);
        lv_label_set_text(lbl_trade_title, buf);
        lv_obj_set_style_text_color(lbl_trade_title, lv_color_hex(0xFFDD44), 0);
    } else {
        max_qty = bj.inventory[idx].count;
        char buf[64]; snprintf(buf, sizeof(buf), "卖出 - %s", items[idx].name);
        lv_label_set_text(lbl_trade_title, buf);
        lv_obj_set_style_text_color(lbl_trade_title, lv_color_hex(0x44DDFF), 0);
    }
    
    trade_qty = max_qty > 0 ? 1 : 0;
    refresh_trade_qty();
    
    lv_obj_clear_flag(modal_trade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(modal_trade);
}

void execute_trade() {
    if (trade_idx < 0 || trade_idx >= NUM_ITEMS) return;
    if (trade_qty > 0) {
        if (is_buying_mode) {
            long total_cost = ((long)bj.inventory[trade_idx].count * bj.inventory[trade_idx].avg_buy_price) + ((long)trade_qty * items[trade_idx].current_price);
            bj.cash -= (long)trade_qty * items[trade_idx].current_price;
            bj.inventory[trade_idx].count += trade_qty;
            bj.inventory[trade_idx].avg_buy_price = total_cost / bj.inventory[trade_idx].count;
            bj.storage += trade_qty;
        } else {
            bj.cash += (long)trade_qty * items[trade_idx].current_price;
            bj.inventory[trade_idx].count -= trade_qty;
            bj.storage -= trade_qty;
            if (bj.inventory[trade_idx].count == 0) bj.inventory[trade_idx].avg_buy_price = 0;
        }
        refresh_beijing_ui();
    }
    lv_obj_add_flag(modal_trade, LV_OBJ_FLAG_HIDDEN);
}

void refresh_beijing_ui() {
    if (!scr_beijing) return;

    static char buf[256];
    snprintf(buf, sizeof(buf), "第%d天 [%s] 血:%d 仓:%d/%d\n%s\n现:%ld 存:%ld 欠:%ld", 
            bj.day > MAX_DAYS ? MAX_DAYS : bj.day, locations[bj.location], 
            bj.health, bj.storage, bj.max_storage, bj.rumor,
            bj.cash, bj.bank, bj.debt);
    lv_label_set_text(lbl_bj_status, buf);

    if (bj.day >= 999) return;

    lv_obj_clean(list_market);
    for(int i=0; i<NUM_ITEMS; i++) {
        if(items[i].current_price <= 0) continue;
        if(items[i].source_loc != -1 && items[i].source_loc != bj.location) continue;

        static char item_buf[64];
        snprintf(item_buf, sizeof(item_buf), "%s: %d 块", items[i].name, items[i].current_price);
        lv_obj_t * btn = lv_list_add_btn(list_market, NULL, item_buf);
        
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x444444), 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 0);
        lv_obj_add_style(lbl, &style_cn, 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xE0E0E0), 0);
        lv_obj_add_event_cb(btn, [](lv_event_t *e){ open_trade_modal((int)(intptr_t)lv_event_get_user_data(e), true); }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    lv_obj_clean(list_bag);
    for(int i=0; i<NUM_ITEMS; i++) {
        if(bj.inventory[i].count <= 0) continue;
        char item_buf[64];
        snprintf(item_buf, sizeof(item_buf), "%s  x%d", items[i].name, bj.inventory[i].count);
        lv_obj_t * btn = lv_list_add_btn(list_bag, NULL, item_buf);
        
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x444444), 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 0);
        lv_obj_add_style(lbl, &style_cn, 0);
        
        if(items[i].current_price >= bj.inventory[i].avg_buy_price) {
            lv_obj_set_style_text_color(lbl, lv_color_hex(0x44FF44), 0); 
        } else {
            lv_obj_set_style_text_color(lbl, lv_color_hex(0xFF5555), 0); 
        }
        lv_obj_add_event_cb(btn, [](lv_event_t *e){ open_trade_modal((int)(intptr_t)lv_event_get_user_data(e), false); }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
}

void build_beijing_scene() {
    if (scr_beijing != NULL) {
        lv_obj_del_async(scr_beijing);
        scr_beijing = NULL;
    }
    scr_beijing = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_beijing, lv_color_hex(0x111111), 0);

    lbl_bj_status = lv_label_create(scr_beijing);
    lv_obj_set_size(lbl_bj_status, 230, 80);
    lv_obj_add_style(lbl_bj_status, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_bj_status, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_bj_status, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t * tv = lv_tabview_create(scr_beijing, LV_DIR_TOP, 35);
    lv_obj_set_size(tv, 240, 235);
    lv_obj_align(tv, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t * tv_btns = lv_tabview_get_tab_btns(tv);
    lv_obj_add_style(tv_btns, &style_cn, 0);
    lv_obj_set_style_bg_color(tv_btns, lv_color_hex(0x222222), 0);
    lv_obj_set_style_text_color(tv_btns, lv_color_hex(0xCCCCCC), 0);
    
    lv_obj_set_style_pad_right(tv_btns, 45, 0); 

    lv_obj_t * btn_menu = lv_btn_create(scr_beijing);
    lv_obj_set_size(btn_menu, 45, 35);
    lv_obj_set_style_bg_color(btn_menu, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(btn_menu, 0, 0); 
    lv_obj_align_to(btn_menu, tv, LV_ALIGN_TOP_RIGHT, 0, 0); 
    
    lv_obj_t * lbl_m = lv_label_create(btn_menu);
    lv_obj_add_style(lbl_m, &style_cn, 0);
    lv_label_set_text(lbl_m, "菜单");
    lv_obj_set_style_text_color(lbl_m, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(lbl_m);
    lv_obj_add_event_cb(btn_menu, [](lv_event_t *e){ lv_obj_clear_flag(menu_overlay, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    tab_market = lv_tabview_add_tab(tv, "市场");
    tab_bag    = lv_tabview_add_tab(tv, "行囊");
    tab_move   = lv_tabview_add_tab(tv, "移动");
    tab_place  = lv_tabview_add_tab(tv, "场所");

    list_market = lv_list_create(tab_market); lv_obj_set_size(list_market, 230, 190);
    lv_obj_set_style_bg_color(list_market, lv_color_hex(0x1A1A1A), 0); lv_obj_set_style_border_width(list_market, 0, 0);
    lv_obj_align(list_market, LV_ALIGN_TOP_MID, 0, 0);
    
    list_bag = lv_list_create(tab_bag); lv_obj_set_size(list_bag, 230, 190);
    lv_obj_set_style_bg_color(list_bag, lv_color_hex(0x1A1A1A), 0); lv_obj_set_style_border_width(list_bag, 0, 0);
    lv_obj_align(list_bag, LV_ALIGN_TOP_MID, 0, 0);

    for(int i=0; i<NUM_LOCATIONS; i++) {
        lv_obj_t * b = lv_btn_create(tab_move);
        lv_obj_set_size(b, 100, 35);
        lv_obj_align(b, LV_ALIGN_TOP_LEFT, 10 + (i%2)*115, (i/2)*40);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x333333), 0); 
        lv_obj_t * l = lv_label_create(b); lv_obj_add_style(l, &style_cn, 0);
        lv_label_set_text(l, locations[i]); lv_obj_center(l);
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0); 
        lv_obj_add_event_cb(b, [](lv_event_t *e){ next_day((int)(intptr_t)lv_event_get_user_data(e)); }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    const char* place_names[] = {
        "银行 (存款/取款)", "黑市 (借款/还款)", "中介 (扩充空间)", 
        "神棍 (重新算卦)", "医院 (治病满血)"
    };
    for(int i=0; i<5; i++) {
        lv_obj_t * b = lv_btn_create(tab_place);
        lv_obj_set_size(b, 210, 35);
        lv_obj_align(b, LV_ALIGN_TOP_MID, 0, i*40);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x333333), 0);
        lv_obj_t * l = lv_label_create(b); lv_obj_add_style(l, &style_cn, 0);
        lv_label_set_text(l, place_names[i]); lv_obj_center(l);
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
        lv_obj_add_event_cb(b, [](lv_event_t *e){
            int id = (int)(intptr_t)lv_event_get_user_data(e);
            if(id == 0 || id == 1 || id == 2) { 
                populate_sub_menu(id); 
            }
            if(id==3) { if(bj.cash>=100){ bj.cash-=100; randomize_prices(); show_bj_msg("算命先生拨弄时间线，物价刷新！"); }else show_bj_msg("没钱求什么卦？去去去！"); }
            if(id==4) { if(bj.health<100){ if(bj.cash>=500){ bj.health=100; bj.cash-=500; show_bj_msg("出院时身体倍儿棒！"); }else show_bj_msg("挂号费 500 块。");} else show_bj_msg("没病装什么病？");}
            if(id == 3 || id == 4) refresh_beijing_ui();
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    modal_trade = lv_obj_create(scr_beijing);
    lv_obj_set_size(modal_trade, 220, 240); lv_obj_center(modal_trade);
    lv_obj_set_style_bg_color(modal_trade, lv_color_hex(0x1F1F1F), 0);
    lv_obj_set_style_border_color(modal_trade, lv_color_hex(0x555555), 0);
    lv_obj_set_style_border_width(modal_trade, 2, 0);
    lv_obj_add_flag(modal_trade, LV_OBJ_FLAG_HIDDEN);
    
    lbl_trade_title = lv_label_create(modal_trade);
    lv_obj_add_style(lbl_trade_title, &style_cn, 0);
    lv_obj_align(lbl_trade_title, LV_ALIGN_TOP_MID, 0, 0);
    
    lbl_trade_info = lv_label_create(modal_trade);
    lv_obj_add_style(lbl_trade_info, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_trade_info, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(lbl_trade_info, LV_ALIGN_TOP_MID, 0, 30);
    
    lbl_trade_qty = lv_label_create(modal_trade);
    lv_obj_add_style(lbl_trade_qty, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_trade_qty, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_trade_qty, LV_ALIGN_TOP_MID, 0, 90);
    
    const char* qty_btns_txt[] = {"-10", "-1", "+1", "+10"};
    int qty_btns_val[] = {-10, -1, 1, 10};
    for(int i=0; i<4; i++) {
        lv_obj_t * b = lv_btn_create(modal_trade);
        lv_obj_set_size(b, 40, 35);
        lv_obj_align(b, LV_ALIGN_TOP_LEFT, 10 + i*45, 125);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x333333), 0);
        lv_obj_t * l = lv_label_create(b); lv_obj_add_style(l, &style_cn, 0);
        lv_label_set_text(l, qty_btns_txt[i]); lv_obj_center(l);
        lv_obj_add_event_cb(b, [](lv_event_t *e){
            trade_qty += (int)(intptr_t)lv_event_get_user_data(e);
            refresh_trade_qty();
        }, LV_EVENT_CLICKED, (void*)(intptr_t)qty_btns_val[i]);
    }
    
    lv_obj_t *btn_cancel = lv_btn_create(modal_trade); lv_obj_set_size(btn_cancel, 55, 35);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 5, 0);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x552222), 0);
    lv_obj_t *l_c = lv_label_create(btn_cancel); lv_obj_add_style(l_c, &style_cn, 0); lv_label_set_text(l_c, "取消"); lv_obj_center(l_c);
    lv_obj_add_event_cb(btn_cancel, [](lv_event_t *e){ lv_obj_add_flag(modal_trade, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_max = lv_btn_create(modal_trade); lv_obj_set_size(btn_max, 55, 35);
    lv_obj_align(btn_max, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_max, lv_color_hex(0x333333), 0);
    lv_obj_t *l_m = lv_label_create(btn_max); lv_obj_add_style(l_m, &style_cn, 0); lv_label_set_text(l_m, "最大"); lv_obj_center(l_m);
    lv_obj_add_event_cb(btn_max, [](lv_event_t *e){ trade_qty = max_qty; refresh_trade_qty(); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_ok = lv_btn_create(modal_trade); lv_obj_set_size(btn_ok, 55, 35);
    lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_RIGHT, -5, 0);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0x225522), 0);
    lv_obj_t *l_o = lv_label_create(btn_ok); lv_obj_add_style(l_o, &style_cn, 0); lv_label_set_text(l_o, "确认"); lv_obj_center(l_o);
    lv_obj_add_event_cb(btn_ok, [](lv_event_t *e){ execute_trade(); }, LV_EVENT_CLICKED, NULL);

    modal_sub = lv_obj_create(scr_beijing);
    lv_obj_set_size(modal_sub, 200, 210); lv_obj_center(modal_sub);
    lv_obj_set_style_bg_color(modal_sub, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_color(modal_sub, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(modal_sub, 2, 0);
    lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN);

    lbl_sub_title = lv_label_create(modal_sub);
    lv_obj_add_style(lbl_sub_title, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_sub_title, lv_color_hex(0xFFDD44), 0);
    lv_obj_align(lbl_sub_title, LV_ALIGN_TOP_MID, 0, -5);

    list_sub = lv_list_create(modal_sub);
    lv_obj_set_size(list_sub, 180, 130);
    lv_obj_align(list_sub, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_bg_color(list_sub, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(list_sub, 0, 0);

    lv_obj_t * btn_sub_close = lv_btn_create(modal_sub);
    lv_obj_set_size(btn_sub_close, 80, 30);
    lv_obj_align(btn_sub_close, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(btn_sub_close, lv_color_hex(0x552222), 0);
    lv_obj_t * lbl_sub_close = lv_label_create(btn_sub_close);
    lv_obj_add_style(lbl_sub_close, &style_cn, 0);
    lv_label_set_text(lbl_sub_close, "返回");
    lv_obj_center(lbl_sub_close);
    lv_obj_add_event_cb(btn_sub_close, [](lv_event_t *e){ lv_obj_add_flag(modal_sub, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    menu_overlay = lv_obj_create(scr_beijing);
    lv_obj_set_size(menu_overlay, 240, 320);
    lv_obj_set_style_bg_color(menu_overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(menu_overlay, 220, 0);
    lv_obj_add_flag(menu_overlay, LV_OBJ_FLAG_HIDDEN);
    
    const char* menu_opts[] = {"继续游戏", "保存游戏", "重新开始", "退出游戏"};
    for(int i=0; i<4; i++) {
        lv_obj_t * b = lv_btn_create(menu_overlay);
        lv_obj_set_size(b, 160, 45);
        lv_obj_align(b, LV_ALIGN_CENTER, 0, -80 + i*55);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x333333), 0); 
        lv_obj_t * l = lv_label_create(b); lv_obj_add_style(l, &style_cn, 0);
        lv_label_set_text(l, menu_opts[i]); lv_obj_center(l);
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
        lv_obj_add_event_cb(b, [](lv_event_t *e){
            int id = (int)(intptr_t)lv_event_get_user_data(e);
            if(id==0) lv_obj_add_flag(menu_overlay, LV_OBJ_FLAG_HIDDEN);
            if(id==1) { save_beijing_game(); lv_obj_add_flag(menu_overlay, LV_OBJ_FLAG_HIDDEN); }
            if(id==2) { reset_beijing_game(); lv_obj_add_flag(menu_overlay, LV_OBJ_FLAG_HIDDEN); }
            if(id==3) {
                save_beijing_game();
                lv_obj_add_flag(menu_overlay, LV_OBJ_FLAG_HIDDEN);
                
                // 【修复】：先暂存指针，或者先调用 del_async 再置空
                lv_obj_t * temp_scr = scr_beijing;
                
                // 清空所有全局/静态 lv_obj_t* 指针
                scr_beijing = NULL;
                lbl_bj_status = NULL;
                tab_market = NULL;
                tab_bag = NULL;
                tab_move = NULL;
                tab_place = NULL;
                list_market = NULL;
                list_bag = NULL;
                modal_msg = NULL;
                lbl_msg_content = NULL;
                menu_overlay = NULL;
                modal_trade = NULL;
                lbl_trade_title = NULL;
                lbl_trade_info = NULL;
                lbl_trade_qty = NULL;
                modal_sub = NULL;
                list_sub = NULL;
                lbl_sub_title = NULL;
                
                lv_scr_load(scr_menu);
                lv_obj_del_async(temp_scr); // 【修复】：传递有效指针给LVGL
            }
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    modal_msg = lv_obj_create(scr_beijing);
    lv_obj_set_size(modal_msg, 200, 160); lv_obj_center(modal_msg);
    lv_obj_set_style_bg_color(modal_msg, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_color(modal_msg, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(modal_msg, 2, 0);
    lv_obj_add_flag(modal_msg, LV_OBJ_FLAG_HIDDEN);
    
    lbl_msg_content = lv_label_create(modal_msg);
    lv_obj_add_style(lbl_msg_content, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_msg_content, lv_color_hex(0xFFFFFF), 0); 
    lv_obj_set_width(lbl_msg_content, 180); lv_obj_align(lbl_msg_content, LV_ALIGN_TOP_MID, 0, 0);
    
    lv_obj_t * btn_ok_msg = lv_btn_create(modal_msg);
    lv_obj_set_size(btn_ok_msg, 80, 35); lv_obj_align(btn_ok_msg, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_ok_msg, lv_color_hex(0x444444), 0);
    lv_obj_t * lok = lv_label_create(btn_ok_msg); lv_obj_add_style(lok, &style_cn, 0); 
    lv_label_set_text(lok, "知道了"); lv_obj_center(lok);
    lv_obj_add_event_cb(btn_ok_msg, [](lv_event_t *e){ lv_obj_add_flag(modal_msg, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);
}

void reset_beijing_game() {
    bj = {1, 2000, 0, 5000, 0.075f, 100, 0, 100, 0};
    for(int i=0; i<NUM_ITEMS; i++) bj.inventory[i] = {0, 0};
    randomize_prices();
    refresh_beijing_ui();
}

