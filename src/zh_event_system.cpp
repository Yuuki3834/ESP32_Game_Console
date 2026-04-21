#include <Arduino.h>
#include "zongheng_types.h"
#include <stdio.h>
#include <string.h>

// ==========================================
// 辅助函数定义
// ==========================================

// 安全扣血 (确保血量至少为1，并追加文本)
static void safe_hurt(int dmg, char* buf, size_t buf_size) {
    zh_player.hp -= dmg;
    if (zh_player.hp < 1) zh_player.hp = 1;
    
    if (buf != NULL && buf_size > 0) {
        size_t len = strlen(buf);
        if (buf_size > len + 1) {
            snprintf(buf + len, buf_size - len, " (生命 -%d)", dmg);
        }
    }
}

// 安全恢复生命 (不超过最大生命值)
static void safe_heal(int amount) {
    zh_player.hp += amount;
    if (zh_player.hp > zh_player.max_hp) {
        zh_player.hp = zh_player.max_hp;
    }
}

// 安全扣除金币 (防止扣成负数，返回实际扣除的金额)
static int safe_lose_gold(int amount) {
    if (zh_player.gold < amount) {
        amount = zh_player.gold;
    }
    zh_player.gold -= amount;
    return amount;
}


// ==========================================
// 陆地随机事件
// ==========================================
void trigger_random_land_event(char* current_log_buf, size_t buf_size) {
    // 1. 特殊条件强制触发事件 (通缉)
    if (zh_player.crime_value >= 300 && rand() % 100 < 25) {
        int dmg = 100 + zh_player.crime_value / 2;
        zh_player.hp -= dmg;
        if(zh_player.hp < 1) zh_player.hp = 1;
        snprintf(current_log_buf, buf_size, "【官方通缉】你遭遇了全城大搜捕的锦衣卫/皇家火枪队！\n突围时身负重伤！(生命-%d)", dmg);
        return; 
    }
    
    // 2. 特殊条件强制触发事件 (声望)
    if (zh_player.reputation >= 800 && rand() % 100 < 15) {
        int drop = get_random_drop_id();
        add_item_to_bag(drop);
        const char* item_name = get_item_by_id(drop).name;
        snprintf(current_log_buf, buf_size, "【万民敬仰】当地总督亲自迎接了你，并赠送了宝物！\n获得：[%s]", item_name);
        return;
    }

    // 3. 30%概率无事发生
    if (rand() % 100 >= 30) return;

    // 4. 常规随机事件 (0~59)
    int ev_id = rand() % 60; 
    static char ev_buf[256];
    ev_buf[0] = '\0'; // 每次使用前清空静态缓冲区

    switch(ev_id) {
        // --- 正面财富事件 ---
        case 0: { int g = rand()%100+50; zh_player.gold += g; snprintf(ev_buf, sizeof(ev_buf), "你在路边的草丛里捡到了一个钱袋！获得 %d 铜贝。", g); break; }
        case 1: { int g = rand()%300+100; zh_player.gold += g; snprintf(ev_buf, sizeof(ev_buf), "你救助了一位迷路的富商，他塞给你 %d 铜贝作为答谢。", g); break; }
        case 2: zh_player.silver += 1; snprintf(ev_buf, sizeof(ev_buf), "你用锄头挖地时，挖到了一块闪闪发光的银块！获得 1 枚银贝！"); break;
        case 3: { int g = rand()%200+50; zh_player.gold += g; snprintf(ev_buf, sizeof(ev_buf), "你发现了一处隐蔽的强盗藏匿点，挖出了 %d 铜贝。", g); break; }
        case 4: zh_player.gold += 500; snprintf(ev_buf, sizeof(ev_buf), "你在一片废墟中发现了一坛埋藏的铜贝，共 500 枚！"); break;
        case 5: { int g = rand()%150+50; zh_player.gold += g; snprintf(ev_buf, sizeof(ev_buf), "你用捕兽夹捉住了一只肥硕的野猪，卖了 %d 铜贝。", g); break; }
        case 6: zh_player.silver += 2; snprintf(ev_buf, sizeof(ev_buf), "你在河边洗澡时，在水底摸到了 2 枚沉甸甸的银贝！"); break;
        case 7: { int g = rand()%50+10; zh_player.gold += g; snprintf(ev_buf, sizeof(ev_buf), "地上有一枚铜币...咦，下面还压着一堆！获得 %d 铜贝。", g); break; }
        case 8: zh_player.debt = 0; snprintf(ev_buf, sizeof(ev_buf), "你的债主破产了！你终于不用还钱了！"); break;
        case 9: { int g = rand()%400+100; zh_player.gold += g; snprintf(ev_buf, sizeof(ev_buf), "你在一个破旧的宝箱里找到了 %d 铜贝。", g); break; }

        // --- 负面财富事件 ---
        case 10: { int g = safe_lose_gold(zh_player.gold * 0.1); snprintf(ev_buf, sizeof(ev_buf), "你被一伙劫匪拦路抢劫！丢失了 %d 铜贝。", g); break; }
        case 11: { int g = safe_lose_gold(rand()%100+20); snprintf(ev_buf, sizeof(ev_buf), "你乘坐的马车在颠簸的山路上散架了，损失了 %d 铜贝。", g); break; }
        case 12: { 
            if (zh_player.silver > 0) { 
                zh_player.silver--; 
                snprintf(ev_buf, sizeof(ev_buf), "一个扒手趁你不备，偷走了你 1 枚银贝！");
            } else {
                snprintf(ev_buf, sizeof(ev_buf), "一个扒手想偷你的东西，但发现你是个穷鬼，悻悻离去。");
            } 
            break; 
        }
        case 13: { int g = safe_lose_gold(rand()%200+50); snprintf(ev_buf, sizeof(ev_buf), "你被误认为是间谍，被关了几天，出来时被敲诈了 %d 铜贝。", g); break; }
        case 14: snprintf(ev_buf, sizeof(ev_buf), "你被一头愤怒的野牛冲撞了！背包被撞翻，财物散落一地，大部分遗失。"); break;
        case 15: { int g = safe_lose_gold(zh_player.gold * 0.05); snprintf(ev_buf, sizeof(ev_buf), "你买了一张劣质的藏宝图，结果一无所获，被骗了 %d 铜贝。", g); break; }
        case 16: { int g = safe_lose_gold(rand()%300+100); snprintf(ev_buf, sizeof(ev_buf), "一场突如其来的暴雨冲毁了你的营地，你丢了 %d 铜贝。", g); break; }
        case 17: zh_player.gold = 0; snprintf(ev_buf, sizeof(ev_buf), "你遇到了一场奇怪的幻觉，疯狂挥霍才发现是梦！(金币清零)"); break;
        case 18: { int g = safe_lose_gold(rand()%50+10); snprintf(ev_buf, sizeof(ev_buf), "你帮助的旅人是个骗子，你的盘缠损失了 %d 铜贝。", g); break; }
        case 19: { int g = safe_lose_gold(rand()%80+20); snprintf(ev_buf, sizeof(ev_buf), "为了通过一个检查站，你不得不贿赂了守卫 %d 铜贝。", g); break; }

        // --- 增益与恢复事件 ---
        case 20: zh_player.hp = zh_player.max_hp; snprintf(ev_buf, sizeof(ev_buf), "你发现了一个古老的泉水，沐浴其中让你恢复了所有伤痛！(生命全满)"); break;
        case 21: safe_heal(30); snprintf(ev_buf, sizeof(ev_buf), "你采集了一些鲜嫩的野菜，味道不错，恢复了 30 点生命。"); break;
        case 22: safe_heal(50); snprintf(ev_buf, sizeof(ev_buf), "路过的药师送了你一瓶伤药，恢复了 50 点生命。"); break;
        case 23: zh_player.max_hp += 10; safe_heal(10); snprintf(ev_buf, sizeof(ev_buf), "你在隐秘山洞参悟锻体秘籍，体魄得到强化！(最大生命+10)"); break;
        case 24: safe_heal(80); snprintf(ev_buf, sizeof(ev_buf), "你在被遗弃的营地里找到了干粮和水，恢复了 80 生命。"); break;
        case 25: zh_player.hp = zh_player.max_hp; zh_player.crime_value = 0; snprintf(ev_buf, sizeof(ev_buf), "你偶然闯入了一片被祝福的圣地。(生命满，罪恶清零)"); break;
        case 26: zh_player.atk += 2; snprintf(ev_buf, sizeof(ev_buf), "你模仿一位舞剑老人的动作，掌握了一招新的剑法！(攻击力+2)"); break;
        case 27: zh_player.def += 2; snprintf(ev_buf, sizeof(ev_buf), "你学会了如何在崎岖的地形中保持身体的平衡。(防御力+2)"); break;
        case 28: zh_player.exp += 300; snprintf(ev_buf, sizeof(ev_buf), "你解读了一块奇异的石碑，获得了大量知识！(获得300经验)"); break;
        case 29: zh_player.hp = zh_player.max_hp; snprintf(ev_buf, sizeof(ev_buf), "你在森林深处找到一个宁静的避风港，安稳地休息了一晚。"); break;

        // --- 负面受伤事件 ---
        case 30: snprintf(ev_buf, sizeof(ev_buf), "你踩到了一根隐藏的尖刺，小腿被划破了！"); safe_hurt(30, ev_buf, sizeof(ev_buf)); break;
        case 31: snprintf(ev_buf, sizeof(ev_buf), "一阵妖风袭来，你被吹倒在地，擦伤了脸颊！"); safe_hurt(20, ev_buf, sizeof(ev_buf)); break;
        case 32: snprintf(ev_buf, sizeof(ev_buf), "你不小心被断裂的树枝绊倒，摔了个狗啃泥！"); safe_hurt(40, ev_buf, sizeof(ev_buf)); break;
        case 33: snprintf(ev_buf, sizeof(ev_buf), "误食了有毒的浆果，肚子剧痛难忍..."); safe_hurt(25, ev_buf, sizeof(ev_buf)); break;
        case 34: snprintf(ev_buf, sizeof(ev_buf), "你踏入了一片沼泽，深陷其中，费了九牛二虎之力才爬出。"); safe_hurt(50, ev_buf, sizeof(ev_buf)); break;
        case 35: snprintf(ev_buf, sizeof(ev_buf), "一只巨型蜘蛛从树上落下，用蛛网缠住了你！"); safe_hurt(35, ev_buf, sizeof(ev_buf)); break;
        case 36: snprintf(ev_buf, sizeof(ev_buf), "空气中弥漫着腐臭的气味，你吸入了毒气。"); safe_hurt(45, ev_buf, sizeof(ev_buf)); break;
        case 37: snprintf(ev_buf, sizeof(ev_buf), "你在黑暗中看不清路，一头撞在岩壁上。"); safe_hurt(20, ev_buf, sizeof(ev_buf)); break;
        case 38: snprintf(ev_buf, sizeof(ev_buf), "一群愤怒的野蜂将你团团围住！"); safe_hurt(15, ev_buf, sizeof(ev_buf)); break;
        case 39: snprintf(ev_buf, sizeof(ev_buf), "山路崎岖，你脚下一滑，滚下了一个小坡！"); safe_hurt(60, ev_buf, sizeof(ev_buf)); break;

        // --- 物品及综合事件 ---
        case 40:
            if(add_item_to_bag(101)) snprintf(ev_buf, sizeof(ev_buf), "你在一个废弃的营地里发现了一瓶【微型生命药水】。");
            else snprintf(ev_buf, sizeof(ev_buf), "你在废弃营地找到了药水，可惜背包已满。");
            break;
        case 41:
            if(add_item_to_bag(104)) snprintf(ev_buf, sizeof(ev_buf), "一位老者给了你一瓶闪着红光的药剂，说是【狂暴药剂】。");
            else snprintf(ev_buf, sizeof(ev_buf), "老者想送你药剂，可惜你的背包又满了。");
            break;
        case 42: {
            int drop = get_random_drop_id();
            if(add_item_to_bag(drop)) snprintf(ev_buf, sizeof(ev_buf), "你在遗忘的角落发现了闪光的东西，获得：【%s】！", get_item_by_id(drop).name);
            else snprintf(ev_buf, sizeof(ev_buf), "你在地上发现了一件宝物，但背包太满了。");
            break;
        }
        case 43: zh_player.crime_value += 30; snprintf(ev_buf, sizeof(ev_buf), "你闯入了某个秘密集会，被当成了间谍！(罪恶值+30)"); break;
        case 44: {
            if (zh_player.crime_value > 0) zh_player.crime_value = (zh_player.crime_value > 20) ? zh_player.crime_value - 20 : 0;
            snprintf(ev_buf, sizeof(ev_buf), "你帮助了一位遭遇麻烦的村民，官方对你的印象改善。(罪恶值下降)"); 
            break;
        }
        case 45:
            if(add_item_to_bag(206)) snprintf(ev_buf, sizeof(ev_buf), "你捡起了一枚闪闪发光的金币...获得【被诅咒的金币】！");
            else snprintf(ev_buf, sizeof(ev_buf), "路边有一枚诡异的金币，你小心地避开了它。");
            break;
        case 46:
            if(add_item_to_bag(215)) snprintf(ev_buf, sizeof(ev_buf), "在枯骨之间，你发现了一朵娇艳欲滴的花朵。获得【血色玫瑰】。");
            else snprintf(ev_buf, sizeof(ev_buf), "一朵美丽的红玫瑰，你背不动它。");
            break;
        case 47: zh_player.exp += 300; snprintf(ev_buf, sizeof(ev_buf), "你在一个隐士那里学到了一些生存技巧！(获得300经验)"); break;
        case 48:
            if(add_item_to_bag(107)) snprintf(ev_buf, sizeof(ev_buf), "你在古老神龛下发现了一张泛黄的卷轴，写着【赦免赎罪券】！");
            else snprintf(ev_buf, sizeof(ev_buf), "神龛下有古卷，可惜你的背包已满。");
            break;
        case 49: 
            zh_player.level++; zh_player.max_hp+=30; zh_player.hp=zh_player.max_hp; zh_player.atk+=6; zh_player.def+=4; 
            snprintf(ev_buf, sizeof(ev_buf), "【奇迹降临】你触摸了一块古老的石碑，碑文力量注入身体！等级提升！"); 
            break;

        // --- 条件判定事件 ---
        case 50: if(zh_player.reputation > 200) { zh_player.gold += 800; snprintf(ev_buf, sizeof(ev_buf), "村民仰慕你的威名，自发为你筹集了 800 铜贝！"); } else snprintf(ev_buf, sizeof(ev_buf), "路过的村民看了你一眼，匆匆走开。"); break;
        case 51: if(zh_player.crime_value > 100) { safe_hurt(60, ev_buf, sizeof(ev_buf)); snprintf(ev_buf, sizeof(ev_buf), "几个受害者的家属在暗巷用木棍袭击了你！"); } else snprintf(ev_buf, sizeof(ev_buf), "你在暗巷里发现了一只野猫。"); break;
        case 52: if(zh_player.reputation > 300) { zh_player.hp = zh_player.max_hp; snprintf(ev_buf, sizeof(ev_buf), "当地名医听闻你的义举，免费为你治好了所有的伤！"); } else snprintf(ev_buf, sizeof(ev_buf), "路边有一株枯萎的草药。"); break;
        case 53: if(zh_player.crime_value > 200) { int loss = safe_lose_gold(zh_player.gold * 0.2); snprintf(ev_buf, sizeof(ev_buf), "黑帮认为你捞过界了，强行收取了你 %d 铜贝保护费！", loss); } else snprintf(ev_buf, sizeof(ev_buf), "你平安穿过了贫民窟。"); break;
        case 54: if(zh_player.reputation > 400) { zh_player.silver += 2; snprintf(ev_buf, sizeof(ev_buf), "商会为了结交你这位大英雄，赠送了 2 枚银贝！"); } else snprintf(ev_buf, sizeof(ev_buf), "商队的马车绝尘而去。"); break;
        case 55: if(zh_player.crime_value > 50) { zh_player.crime_value += 30; snprintf(ev_buf, sizeof(ev_buf), "你被通缉的画像贴满了大街小巷，罪恶值持续上升！"); } else snprintf(ev_buf, sizeof(ev_buf), "你看到布告栏上贴着别人的通缉令。"); break;
        case 56: if(zh_player.reputation > 100 && zh_player.crime_value > 0) { zh_player.crime_value = 0; snprintf(ev_buf, sizeof(ev_buf), "由于你最近的好名声，总督大赦了你以前的罪行！"); } else snprintf(ev_buf, sizeof(ev_buf), "今天的天气真不错。"); break;
        case 57: if(zh_player.crime_value > 150) { safe_hurt(100, ev_buf, sizeof(ev_buf)); snprintf(ev_buf, sizeof(ev_buf), "狂热的赏金猎人设下陷阱，你九死一生才逃脱！"); } else snprintf(ev_buf, sizeof(ev_buf), "你避开了一个捕兽夹。"); break;
        case 58: if(zh_player.reputation > 600) { zh_player.exp += 1000; snprintf(ev_buf, sizeof(ev_buf), "隐世高人听闻你的侠名，破例指点你一番！(经验大涨)"); } else snprintf(ev_buf, sizeof(ev_buf), "你在山顶吹了吹风。"); break;
        case 59: if(zh_player.crime_value > 400) { zh_player.gold = 0; zh_player.silver = 0; snprintf(ev_buf, sizeof(ev_buf), "【倾家荡产】官方抄了你的随身营地，你失去了所有金银！"); } else snprintf(ev_buf, sizeof(ev_buf), "巡逻兵向你点头致意。"); break;         
    }

    // 5. 安全地将日志追加到主缓冲区
    size_t cur_len = strlen(current_log_buf);
    const char* header = "\n\n【途中的遭遇】\n";
    
    if (buf_size > cur_len) {
        // 一次性完成拼接，避免多次 strncat 导致性能下降和溢出风险
        snprintf(current_log_buf + cur_len, buf_size - cur_len, "%s%s", header, ev_buf);
    }
}


// ==========================================
// 航海随机事件
// ==========================================
void trigger_sailing_event(char* msg, size_t buf_size) {
    int r = rand() % 100;
    
    bool immune_pirate = has_item_in_bag(202); // 黑胡子的航海图
    bool immune_storm = has_item_in_bag(220);  // 黄金罗盘
    bool mermaid_boost = has_item_in_bag(201); // 人鱼的眼泪

    const ZH_Adjutant* nav = get_adjutant_by_id(zh_player.eq_adj_navigator);
    if (nav != NULL) {
        immune_storm = true;
    }
    bool is_legendary_nav = (nav != NULL && nav->rarity == 1); 

    // 1. 特殊判定：罪恶值过高
    if(zh_player.crime_value >= 50 && r < 20) {
        if (has_item_in_bag(213)) {
             snprintf(msg, buf_size, "【皇家特权】\n海警拦截了你，但看到皇家特许贸易证后敬礼放行！");
        } else {
            snprintf(msg, buf_size, "【海警突袭】\n由于你罪恶值过高，被海警炮击拦截！\n损失一半生命并被罚没铜贝！");
            zh_player.hp /= 2; 
            if(zh_player.hp < 1) zh_player.hp = 1; 
            zh_player.gold -= (zh_player.gold * 0.3);
        }
        return;
    }

    // 2. 随机航海事件 (0~14) -> 修复了原本 % 14 永远遇不到 case 14 的Bug
    int ev = rand() % 15;
    
    switch(ev) {
        case 0: 
            snprintf(msg, buf_size, "【风平浪静】\n海鸥伴随左右，一路顺风。"); 
            break;
        case 1:
            if(!immune_storm) {
                snprintf(msg, buf_size, "【突发风暴】\n狂风大作，船只剧烈摇晃，桅杆断裂砸伤了你！");
                safe_hurt(30, msg, buf_size);
            } else {
                if (nav != NULL) 
                    snprintf(msg, buf_size, "【规避风暴】\n航海长[%s]展现出惊人经验，舰队安全冲出风暴区！", nav->name);
                else 
                    snprintf(msg, buf_size, "【规避风暴】\n黄金罗盘闪烁，你提前绕开了致命的风暴区！");
            }
            break;
        case 2:
            if(!immune_pirate) {
                int loss = safe_lose_gold(zh_player.gold * 0.15);
                snprintf(msg, buf_size, "【海盗打劫】\n遭遇黑帆海盗团！你被迫交出 %d 铜贝保命。", loss);
            } else {
                snprintf(msg, buf_size, "【海盗退让】\n黑胡子的航海图让你航行在安全水域，海盗无法追踪！");
            }
            break;
        case 3:
            if (r < 30 || mermaid_boost) {
                zh_player.hp = zh_player.max_hp;
                snprintf(msg, buf_size, "【人鱼祝福】\n迷雾中传来了空灵歌声，人鱼的歌声让你伤痛全消！(生命全满)");
            } else {
                snprintf(msg, buf_size, "【迷雾海域】\n在浓雾中航行了许久，有惊无险地驶出。");
            }
            break;
        case 4: {
            int g = rand()%500+200;
            zh_player.gold += g;
            snprintf(msg, buf_size, "【打捞漂流物】\n水手在海面上捞起几个密封的木箱，获得了 %d 铜贝！", g);
            break;
        }
        case 5:
            if (is_legendary_nav) {
                snprintf(msg, buf_size, "【物资统筹】\n传奇航海长合理调配了果蔬物资，船员免受坏血病之苦！");
            } else {
                snprintf(msg, buf_size, "【坏血病蔓延】\n船上缺乏新鲜蔬菜，你感觉身体极度虚弱。");
                safe_hurt(25, msg, buf_size);
            }
            break;
        case 6:
            zh_player.crime_value += 10;
            snprintf(msg, buf_size, "【幽灵船擦肩】\n一艘破败的绿光幽灵船从旁驶过，诡异的气息让你受到诅咒。(罪恶值上升)");
            break;
        case 7: {
            int g = safe_lose_gold(rand()%300+100);
            snprintf(msg, buf_size, "【水手哗变】\n底层水手因为待遇太差发生暴乱！你花钱摆平了他们，损失 %d 铜贝。", g);
            break;
        }
        case 8:
            if (is_legendary_nav) {
                snprintf(msg, buf_size, "【海怪惊扰】\n深海触手刚刚浮现，就被传奇航海长超前的火炮威慑击退！");
            } else {
                snprintf(msg, buf_size, "【海怪触手】\n深海中伸出巨大的触手拍击甲板，你被震飞受重伤！");
                safe_hurt(60, msg, buf_size);
            }
            break;
        case 9:
            if(add_item_to_bag(202)) snprintf(msg, buf_size, "【漂流瓶】\n你捞起一个瓶子，里面竟然装着【黑胡子的航海图】！");
            else snprintf(msg, buf_size, "【漂流瓶】\n里面有一张寻宝图，可惜你背包满了拿不了。");
            break;
        case 10: {
            int g = rand()%300+100;
            zh_player.gold += g;
            snprintf(msg, buf_size, "【友好商船】\n遇到一艘东方商船，顺路做了一笔小买卖，赚了 %d 铜贝。", g);
            break;
        }
        case 11:
            if (is_legendary_nav) {
                snprintf(msg, buf_size, "【规避漩涡】\n传奇航海长操舵如神，借着大漩涡的离心力完美加速脱险！");
            } else {
                snprintf(msg, buf_size, "【遭遇大漩涡】\n船只差点被卷入海底深渊！拼死逃离时受了内伤。");
                safe_hurt(50, msg, buf_size);
            }
            break;
        case 12:
            zh_player.hp = zh_player.max_hp;
            snprintf(msg, buf_size, "【巨型蓝鳍鲔鱼】\n船员钓上了一条巨大的鲔鱼，今晚全船吃刺身大餐！(生命全满)");
            break;
        case 13:
            zh_player.silver += 1;
            snprintf(msg, buf_size, "【暗礁沉船】\n退潮时发现了一艘古代沉船，你在船舱里摸到了 1 枚银贝！");
            break;
        case 14:
            zh_player.exp += 200;
            snprintf(msg, buf_size, "【罕见海鸟】\n一只巨大的海鸟落在你的船头，它带来的幸运让你经验大涨！(EXP+200)");
            break; 
    }
}