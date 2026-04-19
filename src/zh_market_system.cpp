#include "global.h"
#include "zongheng_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 声明外部副官获取函数
extern const ZH_Adjutant* get_adjutant_by_id(int id);

// ==================== 贸易与市场商品数据 (共 245 种) ====================
const char* zh_goods_names[ZH_NUM_GOODS] = {
    // --- 原有 105 种商品 ---
    "玻璃球", "威尼斯蕾丝", "天鹅绒", "大理石雕塑", "水晶镜", "细剑", "艺术品",
    "岩盐", "葡萄酒", "橄榄油", "杏仁", "火枪", "软木", "罗盘",
    "羊毛", "锡矿", "呢绒", "烈酒", "煤炭", "舰炮", "怀表",
    "钻石", "象牙", "鸵鸟毛", "斑马皮", "犀牛角", "狮牙", "木雕",
    "胡椒", "肉桂", "丁香", "豆蔻", "蓝宝石", "印度棉布", "紫檀木",
    "丝绸", "景德瓷器", "茶叶", "麝香", "漆器", "宣纸", "和田玉",
    "棉布", "没药", "乳香", "莎草纸", "木乃伊", "椰枣", "骆驼毛",
    "地毯", "大马士革钢", "香水", "鱼子酱", "蜂蜡", "玫瑰水", "绿松石",
    "香槟", "挂毯", "香皂", "油画", "鸢尾花水", "蓝纹奶酪", "骑士甲",
    "古董石雕", "羊奶酪", "神像", "陈酿红酒", "紫水晶", "海绵", "希腊火",
    "贝壳", "高粱", "粗布", "海盗弯刀", "硝石", "沙漠香料", "红珊瑚",
    "郁金香", "鲱鱼", "亚麻布", "杜松子酒", "望远镜", "航海图", "玻璃器皿",
    "蔗糖", "朗姆酒", "雪茄", "加勒比咖啡", "香草", "菠萝", "海盗宝藏",
    "白银", "武士刀", "日式屏风", "莳绘", "粉珍珠", "浮世绘", "清酒",
    "祖母绿", "巴西木", "烟草", "玛瑙", "羊驼毛", "可可豆", "热带鹦鹉",

    // --- 新增 140 种商品 (索引 105 ~ 244) ---
    // [欧洲补充]
    "威尼斯面具", "镶嵌画", "米兰锻钢", "波西米亚玻璃", "安达卢西亚马", "地中海海盐", "大西洋鳕鱼",
    "卡斯蒂利亚银器", "苏格兰方格裙", "波罗的海琥珀", "日耳曼木刻", "法兰西刺绣", "英格兰锡器", "北海鲸油",
    
    // [非洲补充]
    "黑木雕刻", "孔雀石", "金红石", "非洲木琴", "狮皮", "豹皮", "鳄鱼皮",
    "犀牛皮盾", "非洲黑檀木", "马达加斯加贝", "祖鲁掷矛", "非洲金砂", "热带香蕉", "芒果干",
    
    // [中东/印度洋补充]
    "阿拉伯弯刀", "红海黑珊瑚", "阿曼树脂", "锡兰红茶", "印度神油", "克什米尔蓝宝石", "孟加拉黄麻",
    "马尔代夫黑珍珠", "果阿腰果", "卡利卡特印花布", "波斯铜器", "大马士革玫瑰", "印度孔雀羽", "中东游牧帐篷",
    
    // [东亚/东南亚补充]
    "大明火铳", "蜀锦", "苏绣", "西湖龙井", "武夷岩茶", "宣德炉", "高丽青瓷",
    "高丽人参", "日本折扇", "长崎漆盒", "马六甲锡锭", "爪哇沉香", "暹罗燕窝", "安南红宝石",
    
    // [美洲补充]
    "印加黄金面具", "阿兹特克黑曜石", "羊驼毛毯", "加勒比海螺", "哈瓦那原糖", "新英格兰木材", "弗吉尼亚烟丝",
    "墨西哥银元", "哥伦比亚绿柱石", "秘鲁银矿", "亚马逊橡胶", "南美原木", "印第安战斧", "美洲豹皮",
    
    // [澳洲补充]
    "澳洲细羊毛", "袋鼠皮", "回旋镖", "大堡礁彩贝", "塔斯马尼亚鲍鱼", "尤加利精油", "澳洲坚果",
    "黑天鹅羽毛", "原住民图腾", "红土矿石", "澳洲野狗牙", "鸸鹋蛋雕", "内陆金块", "蓝山石楠木",
    
    // [太平洋补充]
    "波利尼西亚木雕", "夏威夷檀香木", "太平洋椰油", "鲨鱼齿剑", "海龟壳盾", "大洋洲金砂", "面包果",
    "大溪地黑珍珠", "毛利人玉石", "火山黑曜岩", "巨型砗磲", "珊瑚礁标本", "深海海人草", "龙涎香块",
    
    // [稀世奇珍与黑市特供]
    "远古化石", "美人鱼鳞片", "失落航海日志", "海怪之眼", "幽灵船舵", "皇家私掠许可证", "炼金秘药",
    "神秘星图", "精灵之泪", "恶魔的犄角", "冰霜结晶", "烈焰红莲", "深渊魔晶", "天使之羽",
    "吸血鬼披风", "巨龙逆鳞", "贤者之石碎片", "时之沙漏", "海神三叉戟残片", "冥界摆渡金币", "世界树嫩枝",
    "星空陨石", "幻影披风", "雷鸣战鼓", "大地之核", "风暴之眼结晶", "生命泉水", "厄运诅咒木偶",
    "王者之剑剑格", "古代机械齿轮", "魔法禁书", "海神号角", "破晓晨星之辉", "暗夜咏叹乐谱", "精灵王冠",
    "深海巨兽之心", "烈阳神鸟之卵", "星空罗盘", "异界召唤卷轴", "命运纺线", "永恒冰壁之角", "灰烬火种"
};
// ==================== 全局变量与辅助函数 ====================
int current_goods_prices[ZH_NUM_GOODS];
int zh_market_state = 0; 

static lv_obj_t * modal_market = NULL;
static lv_obj_t * lbl_market_info = NULL;
static lv_obj_t * slider_market = NULL;
static lv_obj_t * lbl_market_qty = NULL;

static int market_item_id = -1;
static int market_mode = 0;

static lv_obj_t * modal_npc_shop = NULL;
static lv_obj_t * list_npc_shop = NULL;

// 计算市场商品购买动态价格（包含声望打折、罪恶惩罚、武力屈服与【副官会计加成】）
static int get_market_buy_price(int gid) {
    int price = current_goods_prices[gid];
    if (zh_player.welfare_flag == 1) {
        price = price * 30 / 100;
        if (price < 1) price = 1;
        return price;
    }
    if (zh_player.reputation >= 500) price = price * 90 / 100;
    if (zh_player.crime_value >= 200) price = price * 150 / 100;

    // --- 副官：会计买入降价 ---
    const ZH_Adjutant* adj_acc = get_adjutant_by_id(zh_player.eq_adj_accountant);
    if (adj_acc) {
        price = price * (100 - (int)(5 * adj_acc->power_mult)) / 100;
    }

    if (price < 1) price = 1;
    return price;
}

static const ZH_Location* get_current_location_ptr() {
    for(int i = 0; i < zh_data_locations_count; i++) {
        if(zh_data_locations[i].id == zh_player.location_id) return &zh_data_locations[i];
    }
    return &zh_data_locations[0];
}

// ==================== 1. 市场经济波动与跨港口逻辑 ====================

void refresh_market_prices() {
    for(int i = 0; i < ZH_NUM_GOODS; i++) {
        int base_price = 10 + (rand() % 30); 
        if(i >= 0 && i <= 20) base_price += 15;        
        else if(i >= 21 && i <= 48) base_price += 120; 
        else if(i >= 84 && i <= 104) base_price += 80; 
        
        int fluctuation = (rand() % 101) - 50; 
        current_goods_prices[i] = base_price + (base_price * fluctuation / 100);
        if(current_goods_prices[i] < 5) current_goods_prices[i] = 5;
    }
}

void process_port_switch() {
    // 修复：使用 long long 运算防止几十亿的本金与利息相乘导致整型溢出变成负数
    if (zh_player.bank_gold > 0 && zh_player.bank_gold < 1000000000) { 
        long long interest = (long long)zh_player.bank_gold * 5LL / 100LL;
        zh_player.bank_gold += (long)interest;
    }
    if (zh_player.debt > 0) {
        long long debt_interest = (long long)zh_player.debt * 75LL / 1000LL;
        zh_player.debt += (long)debt_interest;
    }
    zh_player.welfare_flag = 0; // 重置港口商会的屈服状态
    refresh_market_prices();
}

// ==================== 2. 市场/贸易UI界面构建 ====================

void build_market_and_shop_ui(lv_obj_t * parent_scr) {
    modal_market = lv_obj_create(parent_scr); 
    lv_obj_set_size(modal_market, 220, 250); 
    lv_obj_center(modal_market); 
    lv_obj_add_flag(modal_market, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_market, lv_color_hex(0x2c3e50), 0);
    
    lbl_market_info = lv_label_create(modal_market); 
    lv_obj_add_style(lbl_market_info, &style_cn, 0); 
    lv_obj_set_style_text_color(lbl_market_info, lv_color_hex(0xFFFFFF), 0); 
    lv_obj_align(lbl_market_info, LV_ALIGN_TOP_MID, 0, -10);
    
    slider_market = lv_slider_create(modal_market); 
    lv_obj_set_size(slider_market, 160, 15); 
    lv_obj_align(slider_market, LV_ALIGN_TOP_MID, 0, 90);
    
    lbl_market_qty = lv_label_create(modal_market); 
    lv_obj_add_style(lbl_market_qty, &style_cn, 0); 
    lv_obj_set_style_text_color(lbl_market_qty, lv_color_hex(0xFFD700), 0); 
    lv_obj_align(lbl_market_qty, LV_ALIGN_TOP_MID, 0, 115);
    
    lv_obj_add_event_cb(slider_market, [](lv_event_t *e){ 
        int v = lv_slider_get_value(lv_event_get_target(e)); 
        char buf[32]; snprintf(buf, sizeof(buf), "滑动选择数量: %d", v); 
        lv_label_set_text(lbl_market_qty, buf); 
    }, LV_EVENT_VALUE_CHANGED, NULL);
    
    lv_obj_t * btn_market_max = lv_btn_create(modal_market); 
    lv_obj_set_size(btn_market_max, 80, 30); 
    lv_obj_align(btn_market_max, LV_ALIGN_TOP_MID, 0, 140); 
    lv_obj_set_style_bg_color(btn_market_max, lv_color_hex(0x2980b9), 0);
    lv_obj_t * lbl_mmax = lv_label_create(btn_market_max); 
    lv_obj_add_style(lbl_mmax, &style_cn, 0); 
    lv_label_set_text(lbl_mmax, "全选"); 
    lv_obj_center(lbl_mmax);
    
    lv_obj_add_event_cb(btn_market_max, [](lv_event_t *e){ 
        lv_slider_set_value(slider_market, lv_slider_get_max_value(slider_market), LV_ANIM_OFF); 
        lv_event_send(slider_market, LV_EVENT_VALUE_CHANGED, NULL); 
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * btn_market_conf = lv_btn_create(modal_market); 
    lv_obj_set_size(btn_market_conf, 90, 35); 
    lv_obj_align(btn_market_conf, LV_ALIGN_BOTTOM_LEFT, 0, 5); 
    lv_obj_set_style_bg_color(btn_market_conf, lv_color_hex(0x27ae60), 0);
    lv_obj_t * lbl_mconf = lv_label_create(btn_market_conf); 
    lv_obj_add_style(lbl_mconf, &style_cn, 0); 
    lv_label_set_text(lbl_mconf, "确认"); 
    lv_obj_center(lbl_mconf);
    
    lv_obj_add_event_cb(btn_market_conf, [](lv_event_t *e){
        int qty = lv_slider_get_value(slider_market); 
        int gid = market_item_id; 
        
        if (market_mode == 1) { 
            if (zh_player.crime_value >= 500 && zh_player.welfare_flag == 0) {
                zh_log("商会总管：滚出去！我不和通缉犯做生意！\n(罪恶>=500被商会封杀，你只能选择武力抢夺)");
                lv_obj_add_flag(modal_market, LV_OBJ_FLAG_HIDDEN); return;
            }
            
            int price = get_market_buy_price(gid);
            int cost = price * qty;
            if (zh_player.gold >= cost) {
                zh_player.gold -= cost; 
                long long total_val = (long long)zh_player.goods_buy_price[gid] * zh_player.goods_inventory[gid];
                zh_player.goods_inventory[gid] += qty;
                zh_player.goods_buy_price[gid] = (total_val + cost) / zh_player.goods_inventory[gid];
                static char log_buf[128]; snprintf(log_buf, sizeof(log_buf), "【交易成功】\n买入 %d 份 %s，花费 %d 铜贝", qty, zh_goods_names[gid], cost);
                zh_log(log_buf);
            } else { zh_log("金币不足！"); }

        } else if (market_mode == 2) { 
            // 卖出逻辑
            int sell_price = current_goods_prices[gid];
            
            // --- 副官：会计卖出提价 ---
            const ZH_Adjutant* adj_acc = get_adjutant_by_id(zh_player.eq_adj_accountant);
            if (adj_acc) {
                sell_price = sell_price * (1.0 + 0.05 * adj_acc->power_mult);
            }

            if (zh_player.goods_inventory[gid] >= qty) {
                int gain = sell_price * qty; 
                zh_player.gold += gain; 
                zh_player.goods_inventory[gid] -= qty;
                if (zh_player.goods_inventory[gid] == 0) zh_player.goods_buy_price[gid] = 0; 
                static char log_buf[128]; snprintf(log_buf, sizeof(log_buf), "【倾销成功】\n卖出 %d 份 %s，获得 %d 铜贝", qty, zh_goods_names[gid], gain);
                zh_log(log_buf);
            } else { zh_log("库存不足！"); }

        } else if (market_mode == 3) { 
            if (rand() % 100 < qty * 5) { 
                int fine = zh_player.gold / 2; 
                zh_player.gold -= fine; 
                zh_player.crime_value += qty * 10;
                static char log_buf[128]; snprintf(log_buf, sizeof(log_buf), "【盗窃被捕】\n罚没 %d 铜贝！罪恶值激增！", fine);
                zh_log(log_buf);
            } else { 
                long long total_val = (long long)zh_player.goods_buy_price[gid] * zh_player.goods_inventory[gid];
                zh_player.goods_inventory[gid] += qty;
                zh_player.goods_buy_price[gid] = total_val / zh_player.goods_inventory[gid];
                zh_player.crime_value += qty * 2;
                static char log_buf[128]; snprintf(log_buf, sizeof(log_buf), "【顺手牵羊】\n成功偷得 %d 份 %s！", qty, zh_goods_names[gid]);
                zh_log(log_buf);
            }
        } 
        lv_obj_add_flag(modal_market, LV_OBJ_FLAG_HIDDEN);
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * btn_market_canc = lv_btn_create(modal_market); 
    lv_obj_set_size(btn_market_canc, 90, 35); 
    lv_obj_align(btn_market_canc, LV_ALIGN_BOTTOM_RIGHT, 0, 5); 
    lv_obj_set_style_bg_color(btn_market_canc, lv_color_hex(0xc0392b), 0);
    lv_obj_t * lbl_mcanc = lv_label_create(btn_market_canc); 
    lv_obj_add_style(lbl_mcanc, &style_cn, 0); 
    lv_label_set_text(lbl_mcanc, "取消"); 
    lv_obj_center(lbl_mcanc);
    lv_obj_add_event_cb(btn_market_canc, [](lv_event_t *e){ lv_obj_add_flag(modal_market, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    // --- NPC 商店 UI 构建 ---
    modal_npc_shop = lv_obj_create(parent_scr); 
    lv_obj_set_size(modal_npc_shop, 220, 260); 
    lv_obj_center(modal_npc_shop);
    lv_obj_add_flag(modal_npc_shop, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_set_style_bg_color(modal_npc_shop, lv_color_hex(0x1a252f), 0);
    
    lv_obj_t * lbl_shop_title = lv_label_create(modal_npc_shop); 
    lv_obj_add_style(lbl_shop_title, &style_cn, 0); 
    lv_label_set_text(lbl_shop_title, "商店 / 学习"); 
    lv_obj_align(lbl_shop_title, LV_ALIGN_TOP_MID, 0, -10);
    
    list_npc_shop = lv_list_create(modal_npc_shop); 
    lv_obj_set_size(list_npc_shop, 200, 180); 
    lv_obj_align(list_npc_shop, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_bg_color(list_npc_shop, lv_color_hex(0x2c3e50), 0); 
    lv_obj_set_style_border_width(list_npc_shop, 0, 0);
    
    lv_obj_t * btn_shop_close = lv_btn_create(modal_npc_shop); 
    lv_obj_set_size(btn_shop_close, 100, 30); 
    lv_obj_align(btn_shop_close, LV_ALIGN_BOTTOM_MID, 0, 5); 
    lv_obj_set_style_bg_color(btn_shop_close, lv_color_hex(0xc0392b), 0);
    lv_obj_t * l_s_close = lv_label_create(btn_shop_close); 
    lv_obj_add_style(l_s_close, &style_cn, 0); 
    lv_label_set_text(l_s_close, "离开"); 
    lv_obj_center(l_s_close);
    lv_obj_add_event_cb(btn_shop_close, [](lv_event_t *e){ lv_obj_add_flag(modal_npc_shop, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);
}

// ==================== 3. 市场动作列表逻辑 ====================

static void add_market_action_btn(lv_obj_t * list, const char* txt, lv_event_cb_t cb, uint32_t color, void* user_data) {
    lv_obj_t * btn = lv_list_add_btn(list, LV_SYMBOL_PLAY, txt); 
    lv_obj_set_style_bg_color(btn, lv_color_hex(color), 0); 
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
    if(lbl){ 
        lv_obj_add_style(lbl, &style_cn, 0); 
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); 
    }
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
}

static void open_market_modal_cb(lv_event_t *e) {
    int gid = (int)(intptr_t)lv_event_get_user_data(e);
    market_item_id = gid; 
    market_mode = zh_market_state;
    int max_val = 0; 
    static char info_buf[256] = {0};
    
    if (market_mode == 1) { 
        // 购买
        int price = get_market_buy_price(gid);
        max_val = zh_player.gold / price;
        if(max_val > 999) max_val = 999;
        
        if (zh_player.welfare_flag == 1) {
            snprintf(info_buf, sizeof(info_buf), "【威压强卖】 %s\n恐吓价: %d 铜贝(3折!)\n持金可买: %d 份", zh_goods_names[gid], price, max_val);
        } else {
            const ZH_Adjutant* adj_acc = get_adjutant_by_id(zh_player.eq_adj_accountant);
            if (adj_acc) {
                snprintf(info_buf, sizeof(info_buf), "【买入】 %s\n会计还价: %d 铜贝\n持金可买: %d 份", zh_goods_names[gid], price, max_val);
            } else {
                snprintf(info_buf, sizeof(info_buf), "【买入】 %s\n单价: %d 铜贝\n持金可买: %d 份", zh_goods_names[gid], price, max_val);
            }
        }
    } else if (market_mode == 2) { 
        // 卖出
        max_val = zh_player.goods_inventory[gid];
        int buy_price = zh_player.goods_buy_price[gid];
        int sell_price = current_goods_prices[gid];
        
        // --- 副官：UI 提前计算会计加成的收购价 ---
        const ZH_Adjutant* adj_acc = get_adjutant_by_id(zh_player.eq_adj_accountant);
        if (adj_acc) {
            sell_price = sell_price * (1.0 + 0.05 * adj_acc->power_mult);
        }

        int profit = sell_price - buy_price;
        if (adj_acc) {
            snprintf(info_buf, sizeof(info_buf), "【卖出】 %s\n买入均价: %d 铜贝\n会计提价: %d 铜贝\n预期利润: %d 铜/份", zh_goods_names[gid], buy_price, sell_price, profit);
        } else {
            snprintf(info_buf, sizeof(info_buf), "【卖出】 %s\n买入均价: %d 铜贝\n当前收购: %d 铜贝\n预期利润: %d 铜/份", zh_goods_names[gid], buy_price, sell_price, profit);
        }
    } else if (market_mode == 3) { 
        // 盗窃
        max_val = 10; 
        snprintf(info_buf, sizeof(info_buf), "【盗窃】 %s\n单次最多偷取10份\n数量越多被捕风险越高！\n当前罪恶值: %d", zh_goods_names[gid], zh_player.crime_value);
    }

    if (max_val <= 0 && market_mode != 3) { 
        zh_log(market_mode == 1 ? "铜贝不足，买不起！" : "你没有这件商品！"); 
        return; 
    }

    lv_label_set_text(lbl_market_info, info_buf);
    lv_slider_set_range(slider_market, 1, max_val); 
    lv_slider_set_value(slider_market, 1, LV_ANIM_OFF);
    lv_label_set_text(lbl_market_qty, "滑动选择数量: 1");
    lv_obj_clear_flag(modal_market, LV_OBJ_FLAG_HIDDEN);
}

void refresh_market_action_list(lv_obj_t * list_zh_action, const ZH_Location* loc) {
    if (zh_market_state == 0) { 
        if (zh_player.crime_value < 500 || zh_player.welfare_flag == 1) {
            add_market_action_btn(list_zh_action, "买入商品", [](lv_event_t *e){ zh_market_state = 1; lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); }, 0x225522, NULL);
        } else {
            add_market_action_btn(list_zh_action, "买入商品 (遭到拒禁)", [](lv_event_t *e){ zh_log("商会总管：通缉犯滚出去！我们拒绝卖东西给你！"); }, 0x555555, NULL);
        }
        add_market_action_btn(list_zh_action, "卖出商品", [](lv_event_t *e){ zh_market_state = 2; lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); }, 0x552222, NULL);
        add_market_action_btn(list_zh_action, "尝试盗窃 (高风险)", [](lv_event_t *e){ zh_market_state = 3; lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); }, 0x8B0000, NULL);
        
        if (zh_player.welfare_flag == 0) {
            add_market_action_btn(list_zh_action, "殴打护卫 (武力威压)", [](lv_event_t *e){
                zh_player.current_monsters[0] = 20; 
                zh_player.current_monsters_hp[0] = zh_data_monsters[20].hp;
                zh_player.crime_value += 150;
                zh_player.welfare_flag = 1; 
                zh_log("【武力威压】你一脚踢翻了商会总管！大批护卫冲了出来保护金库！");
                start_turn_based_combat(0);
            }, 0xc0392b, NULL);

            add_market_action_btn(list_zh_action, "武装抢劫 (直接抢走货物)", [](lv_event_t *e){
                const ZH_Location* curr_loc = get_current_location_ptr();
                int start_idx = curr_loc->param1, count = curr_loc->param2; 
                if(start_idx + count > ZH_NUM_GOODS) count = ZH_NUM_GOODS - start_idx;
                
                for(int i = start_idx; i < start_idx + count; i++) {
                    zh_player.goods_inventory[i] += (20 + rand() % 21);
                }
                zh_player.crime_value += 800; 
                zh_player.welfare_flag = 1;   
                zh_player.current_monsters[0] = 24; 
                zh_player.current_monsters_hp[0] = zh_data_monsters[24].hp;
                zh_log("【武装洗劫】你疯狂砸碎了商会仓库，满载而归！\n随后商会的王牌铁卫向你发起了拼死绞杀！");
                start_turn_based_combat(0);
            }, 0x8B0000, NULL);
        } else {
            add_market_action_btn(list_zh_action, "商会已彻底屈服 (全场3折)", [](lv_event_t *e){}, 0x888800, NULL);
        }
    } 
    else if (zh_market_state == 1) { 
        add_market_action_btn(list_zh_action, "<- 返回市场主页", [](lv_event_t *e){ zh_market_state = 0; lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); }, 0x555555, NULL);
        int start_idx = loc->param1, count = loc->param2; 
        if(start_idx + count > ZH_NUM_GOODS) count = ZH_NUM_GOODS - start_idx;
        for(int i = start_idx; i < start_idx + count; i++) {
            char b_buf[64];
            // 提前计算，杜绝求值顺序带来的缓冲区覆写
            int buy_price = get_market_buy_price(i);
            if(zh_player.welfare_flag == 1) snprintf(b_buf, sizeof(b_buf), "强购 %s (%d铜)", zh_goods_names[i], buy_price);
            else snprintf(b_buf, sizeof(b_buf), "买入 %s (%d铜)", zh_goods_names[i], buy_price);
            add_market_action_btn(list_zh_action, b_buf, open_market_modal_cb, 0x2c3e50, (void*)(intptr_t)i);
        }
    } 
    else if (zh_market_state == 2) { 
        add_market_action_btn(list_zh_action, "<- 返回市场主页", [](lv_event_t *e){ zh_market_state = 0; lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); }, 0x555555, NULL);
        bool has_goods = false;
        
        const ZH_Adjutant* adj_acc = get_adjutant_by_id(zh_player.eq_adj_accountant);

        for(int i = 0; i < ZH_NUM_GOODS; i++) {
            if(zh_player.goods_inventory[i] > 0) { 
                has_goods = true; 
                
                int sell_price = current_goods_prices[i];
                if (adj_acc) sell_price = sell_price * (1.0 + 0.05 * adj_acc->power_mult);
                
                char s_buf[64]; 
                snprintf(s_buf, sizeof(s_buf), "卖出 %s (持%d/单价%d)", zh_goods_names[i], zh_player.goods_inventory[i], sell_price); 
                add_market_action_btn(list_zh_action, s_buf, open_market_modal_cb, 0x552222, (void*)(intptr_t)i); 
            }
        }
        if(!has_goods) add_market_action_btn(list_zh_action, "货舱空空如也", [](lv_event_t *e){}, 0x555555, NULL);
    } 
    else if (zh_market_state == 3) { 
        add_market_action_btn(list_zh_action, "<- 返回市场主页", [](lv_event_t *e){ zh_market_state = 0; lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL); }, 0x555555, NULL);
        int start_idx = loc->param1, count = loc->param2; 
        if(start_idx + count > ZH_NUM_GOODS) count = ZH_NUM_GOODS - start_idx;
        for(int i = start_idx; i < start_idx + count; i++) {
            char b_buf[64]; snprintf(b_buf, sizeof(b_buf), "盯上 %s", zh_goods_names[i]);
            add_market_action_btn(list_zh_action, b_buf, open_market_modal_cb, 0x8B0000, (void*)(intptr_t)i);
        }
    }
}

// ==================== 4. NPC 道具/黑市商店 UI 及暴力抢劫 ====================

void open_npc_shop_ui(int npc_idx) {
    lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clean(list_npc_shop);
    
    const ZH_NPC* npc = &zh_data_npcs[npc_idx];
    bool is_subdued = (zh_player.npc_status[npc->id] == 1);
    
    if (is_subdued) {
        zh_log("对方一边颤抖，一边惊恐地掏出商品...\n(由于你刚刚殴打了对方，现在全场强制半价交易！)");
    }

    if (!is_subdued) {
        lv_obj_t * btn_rob = lv_list_add_btn(list_npc_shop, LV_SYMBOL_WARNING, "武装洗劫该NPC (极危)");
        lv_obj_set_style_bg_color(btn_rob, lv_color_hex(0x8B0000), 0);
        lv_obj_t * lbl_r = lv_obj_get_child(btn_rob, 1); 
        if(lbl_r) { lv_obj_add_style(lbl_r, &style_cn, 0); lv_obj_set_style_text_color(lbl_r, lv_color_hex(0xFFFFFF), 0); }
        
        lv_obj_add_event_cb(btn_rob, [](lv_event_t *e){
            const ZH_NPC* c_npc = &zh_data_npcs[current_npc_idx];
            int robbed_count = 0;
            for(int i = 0; i < zh_data_fixed_items_count; i++) {
                bool can_sell = false;
                if(c_npc->shop_type == 1 && zh_data_fixed_items[i].type >= 1 && zh_data_fixed_items[i].type <= 5 && zh_data_fixed_items[i].id <= 30) can_sell = true;
                if(c_npc->shop_type == 2 && (zh_data_fixed_items[i].type == 6 || zh_data_fixed_items[i].type == 7 || zh_data_fixed_items[i].type == 9)) can_sell = true;
                if(c_npc->shop_type == 3 && (zh_data_fixed_items[i].type == 8 || zh_data_fixed_items[i].id >= 31)) can_sell = true;
                
                if(can_sell && rand() % 100 < 30) {
                    if(add_item_to_bag(zh_data_fixed_items[i].id)) robbed_count++;
                }
            }
            zh_player.crime_value += 350;
            zh_player.npc_status[c_npc->id] = 1;
            
            static char buf[256];
            snprintf(buf, sizeof(buf), "【洗劫得手】你一剑劈烂了柜台，洗劫了 %s，抢走了 %d 件物品！\n罪恶值暴涨！", c_npc->name, robbed_count);
            zh_log(buf);
            lv_obj_add_flag(modal_npc_shop, LV_OBJ_FLAG_HIDDEN);
            lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
        }, LV_EVENT_CLICKED, NULL);
    }

    for(int i = 0; i < zh_data_fixed_items_count; i++) {
        bool can_sell = false;
        if(npc->shop_type == 1 && zh_data_fixed_items[i].type >= 1 && zh_data_fixed_items[i].type <= 5 && zh_data_fixed_items[i].id <= 30) can_sell = true;
        if(npc->shop_type == 2 && (zh_data_fixed_items[i].type == 6 || zh_data_fixed_items[i].type == 7 || zh_data_fixed_items[i].type == 9)) can_sell = true;
        if(npc->shop_type == 3 && (zh_data_fixed_items[i].type == 8 || zh_data_fixed_items[i].id >= 31)) can_sell = true;
        
        if(can_sell) {
            static char buf[64];
            int c_price = zh_data_fixed_items[i].price;
            if (is_subdued) c_price = c_price / 2;
            
            if(npc->shop_type == 3) {
                int s_price = c_price / 1000 + 1;
                snprintf(buf, sizeof(buf), "%s %s (%d银)", is_subdued ? "威逼" : "兑换", zh_data_fixed_items[i].name, s_price);
            } else {
                snprintf(buf, sizeof(buf), "%s %s (%d铜)", is_subdued ? "强买" : "买", zh_data_fixed_items[i].name, c_price);
            }
            
            lv_obj_t * btn = lv_list_add_btn(list_npc_shop, LV_SYMBOL_LIST, buf);
            lv_obj_set_style_bg_color(btn, lv_color_hex(is_subdued ? 0x888800 : 0x2c3e50), 0);
            lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
            if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
            
            lv_obj_add_event_cb(btn, [](lv_event_t *e){
                int item_id = (int)(intptr_t)lv_event_get_user_data(e);
                ZH_Item item = get_item_by_id(item_id);
                
                const ZH_NPC* c_npc = &zh_data_npcs[current_npc_idx];
                bool is_subdued = (zh_player.npc_status[c_npc->id] == 1);
                int final_price = is_subdued ? item.price / 2 : item.price;
                bool success = false;
                
                if(c_npc->shop_type == 3) {
                    int s_price = final_price / 1000 + 1;
                    if(zh_player.silver >= s_price) { 
                        zh_player.silver -= s_price; success = true; 
                    } else { zh_log("银贝不足！"); }
                } else { 
                    if(zh_player.gold >= final_price) { 
                        zh_player.gold -= final_price; success = true; 
                    } else { zh_log("铜贝不足！"); }
                }
                
                if(success) {
                    bool added = false;
                    for(int k = 0; k < 50; k++) {
                        if(zh_player.inventory[k] == -1) { 
                            zh_player.inventory[k] = item.id; 
                            added = true; 
                            break; 
                        }
                    }
                    if(!added) { 
                        zh_log("背包已满！退款。"); 
                        if(c_npc->shop_type == 3) zh_player.silver += (final_price / 1000 + 1); 
                        else zh_player.gold += final_price; 
                    } else {
                        zh_log(is_subdued ? "对方含着泪交出了物品。" : "交易成功！");
                    }
                }
                lv_obj_add_flag(modal_npc_shop, LV_OBJ_FLAG_HIDDEN); 
                lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
            }, LV_EVENT_CLICKED, (void*)(intptr_t)zh_data_fixed_items[i].id);
        }
    }
    lv_obj_clear_flag(modal_npc_shop, LV_OBJ_FLAG_HIDDEN);
}

// ==================== 5. 传授技能商店 ====================

void open_npc_skill_shop_ui(int npc_idx) {
    lv_obj_add_flag(modal_npc, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_clean(list_npc_shop);
    const ZH_NPC* c_npc = &zh_data_npcs[npc_idx];
    bool is_subdued = (zh_player.npc_status[c_npc->id] == 1);
    
    if (is_subdued) {
        zh_log("在你的刀锋威胁下，导师浑身发抖，只敢收取一半的学费...");
    }

    for(int i = 0; i < zh_data_skills_count; i++) {
        if(zh_data_skills[i].region == c_npc->region_id || zh_data_skills[i].region == 0 || c_npc->region_id == 0) {
            bool learned = false;
            for(int k = 0; k < 160; k++) {
                if(zh_player.learned_skills[k] == zh_data_skills[i].id) {
                    learned = true;
                    break;
                }
            }
            
            if(!learned) {
                char buf[64]; 
                int cost = (zh_data_skills[i].power_mult == 0 ? 3000 : zh_data_skills[i].power_mult * 1000); 
                if (is_subdued) cost /= 2;
                
                snprintf(buf, sizeof(buf), "%s (%d铜)", zh_data_skills[i].name, cost);
                
                lv_obj_t * btn = lv_list_add_btn(list_npc_shop, LV_SYMBOL_FILE, buf);
                lv_obj_set_style_bg_color(btn, lv_color_hex(is_subdued ? 0x888800 : 0x2c3e50), 0);
                lv_obj_t * lbl = lv_obj_get_child(btn, 1); 
                if(lbl) { lv_obj_add_style(lbl, &style_cn, 0); lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); }
                
                lv_obj_add_event_cb(btn, [](lv_event_t *e){
                    int skill_id = (int)(intptr_t)lv_event_get_user_data(e);
                    const ZH_Skill* sk = NULL;
                    for(int j = 0; j < zh_data_skills_count; j++) {
                        if(zh_data_skills[j].id == skill_id) { sk = &zh_data_skills[j]; break; }
                    }
                    
                    const ZH_NPC* cur_npc = &zh_data_npcs[current_npc_idx];
                    bool subdued = (zh_player.npc_status[cur_npc->id] == 1);
                    int cost = (sk->power_mult == 0 ? 3000 : sk->power_mult * 1000); 
                    if (subdued) cost /= 2;

                    // 先检查是否有空槽位
                    int empty_slot = -1;
                    for(int k = 0; k < 160; k++) {
                        if(zh_player.learned_skills[k] <= 0) {
                            empty_slot = k;
                            break;
                        }
                    }
                    
                    if (empty_slot != -1) {
                        if(zh_player.gold >= cost) {
                            zh_player.gold -= cost;
                            zh_player.learned_skills[empty_slot] = skill_id;
                            
                            if(sk->type >= 1 && sk->type <= 4) {
                                for(int j = 0; j < 12; j++) {
                                    if(zh_player.eq_active_skills[j] <= 0) { zh_player.eq_active_skills[j] = skill_id; break; }
                                }
                            } else {
                                for(int j = 0; j < 4; j++) {
                                    if(zh_player.eq_passive_skills[j] <= 0) { zh_player.eq_passive_skills[j] = skill_id; break; }
                                }
                            }
                            zh_log(subdued ? "【强行夺取】\n导师屈辱地交出了卷轴，你掌握了新技能！" : "【顿悟】\n你成功掌握了新的技能！");
                        } else {
                            zh_log(subdued ? "就算打死他，你这钱也不够学费啊。" : "学费不足，导师摇了摇手指。");
                        }
                    } else {
                        zh_log("你的技能已经达到记忆极限，无法再学习了！");
                    }
                    lv_obj_add_flag(modal_npc_shop, LV_OBJ_FLAG_HIDDEN); 
                    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
                }, LV_EVENT_CLICKED, (void*)(intptr_t)zh_data_skills[i].id);
            }
        }
    }
    
    if (lv_obj_get_child_cnt(list_npc_shop) == 0) {
        lv_obj_t * l = lv_label_create(list_npc_shop); 
        lv_obj_add_style(l, &style_cn, 0); 
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(l, "\n无可传授的技能..."); 
        lv_obj_center(l);
    }
    lv_obj_clear_flag(modal_npc_shop, LV_OBJ_FLAG_HIDDEN);
}
void reset_market_ui_pointers() {
    modal_market = NULL;
    lbl_market_info = NULL;
    slider_market = NULL;
    lbl_market_qty = NULL;
    modal_npc_shop = NULL;
    list_npc_shop = NULL;
}