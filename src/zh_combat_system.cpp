#include "global.h"
#include "zongheng_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern ZH_Player zh_player;
extern void zh_log(const char * msg);
extern void refresh_zongheng_ui();
extern void player_die();
extern int get_total_atk();
extern int get_total_def();
extern int get_random_drop_id();
extern bool add_item_to_bag(int item_id);
extern void check_levelup();
extern ZH_Item get_item_by_id(int id);

// 引入副官系统的查询函数
extern const ZH_Adjutant* get_adjutant_by_id(int id);

// =========================================================================
// 【技能数据库】 
// =========================================================================
const ZH_Skill zh_data_skills[] = {
    // ------------------ 【区域 4：东方仙侠/武侠 (DNF鬼剑/格斗/修真)】 ------------------
    {1, "上挑", SKILL_ACTIVE_PHYSICAL, 4, 5, 1.2, 0, "基础剑术，造成120%物理伤害并附带浮空判定。"},
    {2, "连突刺", SKILL_ACTIVE_PHYSICAL, 4, 8, 1.5, 0, "快如闪电的连续刺击，造成150%伤害。"},
    {3, "三段斩", SKILL_ACTIVE_PHYSICAL, 4, 12, 1.8, 0, "滑动中连续斩击三次，造成180%伤害。"},
    {4, "拔刀斩", SKILL_ACTIVE_PHYSICAL, 4, 25, 2.5, 0, "居合剑法极致，一瞬即逝的剑气造成250%伤害。"},
    {5, "破军升龙击", SKILL_ACTIVE_PHYSICAL, 4, 30, 3.0, 0, "霸体前冲撞击并上挑，造成300%物理伤害。"},
    {6, "幻影剑舞", SKILL_ACTIVE_PHYSICAL, 4, 60, 5.5, 0, "在原地释放狂风暴雨般的斩击，造成550%惊人伤害！"},
    {7, "极·鬼剑术(暴风式)", SKILL_ACTIVE_PHYSICAL, 4, 120, 8.5, 0, "[觉醒] 召唤24把剑阵，斩碎虚无！造成850%绝对伤害！"},
    {8, "崩山击", SKILL_ACTIVE_PHYSICAL, 4, 15, 1.6, 0, "跃起砸击地面，造成160%伤害。"},
    {9, "怒气爆发", SKILL_ACTIVE_MAGIC, 4, 35, 3.2, 0, "脚下喷发血气，造成320%魔法伤害。"},
    {10, "嗜血", SKILL_ACTIVE_BUFF, 4, 20, 0, 50, "狂战士天赋，临时增加50点攻击力，但会变得脆弱。"},
    {11, "崩山裂地斩", SKILL_ACTIVE_PHYSICAL, 4, 75, 6.0, 0, "用血气砸碎大地，喷发岩浆，造成600%伤害。"},
    {12, "念气波", SKILL_ACTIVE_MAGIC, 4, 10, 1.4, 0, "将气凝聚射出，造成140%魔法伤害。"},
    {13, "狮子吼", SKILL_ACTIVE_MAGIC, 4, 25, 2.0, 0, "发出震耳欲聋的吼叫，造成200%魔法伤害。"},
    {14, "螺旋念气场", SKILL_ACTIVE_MAGIC, 4, 45, 4.0, 0, "制造巨大的念气漩涡绞杀敌人，造成400%伤害。"},
    {15, "飞龙在天", SKILL_ACTIVE_PHYSICAL, 4, 50, 4.5, 0, "降龙十八掌之一，造成450%物理伤害。"},
    {16, "万剑归宗", SKILL_ACTIVE_MAGIC, 4, 100, 7.0, 0, "[仙术] 御剑飞行，万剑穿心，700%魔法伤害。"},
    {17, "剑影迷踪", SKILL_ACTIVE_PHYSICAL, 4, 20, 1.8, 0, "化身为剑的影子，快速穿梭攻击敌人，造成180%伤害。"},
    {18, "暴走", SKILL_ACTIVE_BUFF, 4, 30, 0, 100, "进入暴走状态，临时增加100点攻击力，每秒损失生命。"},
    {19, "极·鬼剑术", SKILL_ACTIVE_PHYSICAL, 4, 150, 10.0, 0, "[觉醒] 召唤无数鬼剑肆虐！造成1000%毁灭伤害！"},

    // ------------------ 【区域 1：欧洲魔法/骑士 (圣职者/魔法师)】 ------------------
    {20, "纯白之刃", SKILL_ACTIVE_PHYSICAL, 1, 10, 1.5, 0, "凝聚光之刃劈砍，150%伤害。"},
    {21, "圣光十字", SKILL_ACTIVE_MAGIC, 1, 18, 2.0, 0, "画出神圣十字，造成200%魔法伤害。"},
    {22, "忏悔之锤", SKILL_ACTIVE_PHYSICAL, 1, 40, 3.5, 0, "召唤巨大的光之锤砸下，350%伤害。"},
    {23, "正义审判", SKILL_ACTIVE_MAGIC, 1, 65, 5.0, 0, "神圣阵法内天降光剑，造成500%伤害！"},
    {24, "天启之珠", SKILL_ACTIVE_BUFF, 1, 100, 0, 150, "[觉醒] 太阳升起，临时增加150点攻击与防御！"},
    {25, "缓慢愈合", SKILL_ACTIVE_HEAL, 1, 15, 0, 100, "圣光治愈伤口，恢复100点HP。"},
    {26, "圣光祈祷", SKILL_ACTIVE_HEAL, 1, 40, 0, 500, "大天使的赐福，恢复500点HP！"},
    {27, "魔法冰球", SKILL_ACTIVE_MAGIC, 1, 8, 1.3, 0, "发射冰球，130%魔法伤害。"},
    {28, "烈焰冲击", SKILL_ACTIVE_MAGIC, 1, 20, 2.2, 0, "从敌人脚下喷发火柱，220%魔法伤害。"},
    {29, "虚无之球", SKILL_ACTIVE_MAGIC, 1, 35, 3.0, 0, "发射高密度黑洞，持续绞杀，300%伤害。"},
    {30, "冰墙", SKILL_ACTIVE_BUFF, 1, 30, 0, 100, "在周围筑起冰墙，临时提升100点防御。"},
    {31, "杰克降临", SKILL_ACTIVE_MAGIC, 1, 55, 4.8, 0, "召唤巨大的南瓜陨石砸向敌人，480%伤害！"},
    {32, "极冰盛宴", SKILL_ACTIVE_MAGIC, 1, 70, 6.0, 0, "制造绝对零度的暴风雪，600%伤害。"},
    {33, "陨星幻灭", SKILL_ACTIVE_MAGIC, 1, 130, 9.0, 0, "[觉醒] 召唤宇宙陨石群摧毁一切，900%毁灭伤害！"},
    {34, "星辰坠落", SKILL_ACTIVE_MAGIC, 1, 45, 4.2, 0, "引导星辰之力砸向目标，420%魔法伤害。"},
    {35, "黑洞吞噬", SKILL_ACTIVE_MAGIC, 1, 60, 5.2, 0, "制造奇点撕裂目标防御，520%魔法伤害。"},
    {36, "时光倒流", SKILL_ACTIVE_HEAL, 1, 90, 0, 9999, "拨动时间齿轮，将生命值完全恢复到巅峰！"},
    {37, "空间切割", SKILL_ACTIVE_MAGIC, 1, 35, 3.5, 0, "斩开空间，350%真实感魔法伤害。"},
    {38, "光明惩戒", SKILL_ACTIVE_MAGIC, 1, 50, 4.5, 0, "神圣的制裁之光，450%魔法伤害。"},
    {39, "暗影收割", SKILL_ACTIVE_PHYSICAL, 3, 40, 3.8, 0, "死神镰刀扫过，380%物理伤害。"},

    // ------------------ 【区域 3：中东刺客/波斯 (刺客/死灵)】 ------------------
    {40, "割喉", SKILL_ACTIVE_PHYSICAL, 3, 12, 1.7, 0, "精准的匕首绝技，造成170%伤害。"},
    {41, "弧光闪", SKILL_ACTIVE_PHYSICAL, 3, 20, 2.2, 0, "化作一道残影穿透敌人，220%伤害。"},
    {42, "绝命瞬狱杀", SKILL_ACTIVE_PHYSICAL, 3, 50, 4.5, 0, "在夜色中连续绞杀敌人，450%伤害。"},
    {43, "月轮舞", SKILL_ACTIVE_PHYSICAL, 3, 85, 7.5, 0, "[觉醒] 化身为杀戮机器，造成750%巨额物理伤害！"},
    {44, "死灵之怨", SKILL_ACTIVE_MAGIC, 3, 18, 1.9, 0, "召唤怨灵缠绕目标，190%魔法伤害。"},
    {45, "巴拉克之野心", SKILL_ACTIVE_MAGIC, 3, 40, 3.8, 0, "暴君的大手捏碎敌人，380%伤害。"},
    {46, "沙漠风暴", SKILL_ACTIVE_MAGIC, 3, 30, 2.5, 0, "卷起锋利的狂沙，250%伤害。"},
    {47, "大马士革之舞", SKILL_ACTIVE_PHYSICAL, 3, 45, 4.0, 0, "弯刀如同水波般连斩，400%伤害。"},
    {48, "毒刃", SKILL_ACTIVE_PHYSICAL, 3, 15, 1.5, 0, "刀刃淬毒，造成150%伤害(无视部分护甲)。"},
    {49, "暗影诅咒", SKILL_ACTIVE_MAGIC, 3, 25, 2.8, 0, "诅咒敌人，造成280%魔法伤害并降低其攻击力。"},

    // ------------------ 【区域 5：美洲火器/血祭 (漫游/弹药/巫毒)】 ------------------
    {50, "爆头一击", SKILL_ACTIVE_PHYSICAL, 5, 15, 2.0, 0, "致命的左轮射击，200%物理伤害。"},
    {51, "乱射", SKILL_ACTIVE_PHYSICAL, 5, 35, 3.5, 0, "向四周疯狂倾泻子弹，350%伤害。"},
    {52, "双鹰回旋", SKILL_ACTIVE_PHYSICAL, 5, 55, 5.0, 0, "掷出带刃的左轮枪切割敌人，500%伤害。"},
    {53, "疯狂屠戮", SKILL_ACTIVE_PHYSICAL, 5, 100, 8.0, 0, "[觉醒] 枪斗术的极致，弹雨洗地，800%伤害！"},
    {54, "交叉射击", SKILL_ACTIVE_PHYSICAL, 5, 22, 2.4, 0, "交叉火力压制，240%伤害。"},
    {55, "凝固汽油弹", SKILL_ACTIVE_MAGIC, 5, 40, 3.6, 0, "扔出燃烧弹，造成360%火系魔法伤害。"},
    {56, "尼尔狙击", SKILL_ACTIVE_PHYSICAL, 5, 60, 5.5, 0, "呼叫远距离狙击支援，550%致命伤害。"},
    {57, "嗜魂之手", SKILL_ACTIVE_MAGIC, 5, 25, 2.0, 100, "吸取敌人血液，200%伤害并恢复100点生命。"},
    {58, "死亡抗拒", SKILL_ACTIVE_HEAL, 5, 15, 0, 300, "印加秘术，压榨法力恢复300点生命。"},
    {59, "血气之刃", SKILL_ACTIVE_PHYSICAL, 5, 45, 4.2, 0, "将血气化作利刃刺穿目标，420%伤害。"},
    
    // ------------------ 【区域 2：非洲图腾/巫医 (战斗法师/狂战士)】 ------------------
    {60, "碎霸", SKILL_ACTIVE_PHYSICAL, 2, 20, 2.2, 0, "挥舞大棒猛烈横扫，220%物理伤害。"},
    {61, "强袭流星打", SKILL_ACTIVE_PHYSICAL, 2, 45, 4.0, 0, "蓄力后如流星般撞击，400%伤害。"},
    {62, "煌龙偃月", SKILL_ACTIVE_PHYSICAL, 2, 70, 6.5, 0, "召唤巨大的图腾之矛钉死目标，650%伤害。"},
    {63, "变异苍蝇拍", SKILL_ACTIVE_MAGIC, 2, 18, 2.0, 0, "拍打出变异毒虫，200%魔法伤害。"},
    {64, "巫毒娃娃", SKILL_ACTIVE_MAGIC, 2, 35, 3.0, 0, "对草人施加诅咒，敌人承受300%真实魔法痛苦。"},
    {65, "图腾庇护", SKILL_ACTIVE_BUFF, 2, 25, 0, 80, "插下大地庇护图腾，临时增加80点防御力。"},
    {66, "猎豹冲刺", SKILL_ACTIVE_PHYSICAL, 2, 15, 1.8, 0, "如猎豹般撕咬，180%伤害。"},
    {67, "熊之怒吼", SKILL_ACTIVE_BUFF, 2, 30, 0, 100, "模仿熊的吼叫，临时增加100点攻击力。"},
    {68, "毒蘑菇陷阱", SKILL_ACTIVE_MAGIC, 2, 40, 3.5, 0, "放置毒蘑菇陷阱，触发时造成350%魔法伤害。"},
    {69, "地狱火焰", SKILL_ACTIVE_MAGIC, 2, 60, 5.0, 0, "召唤地狱火焰燃烧敌人，500%伤害。"},
    
    // ------------------ 【区域 6：澳洲狂化/生存 (自然使者)】 ------------------
    {70, "回旋镖风暴", SKILL_ACTIVE_PHYSICAL, 6, 25, 2.6, 0, "投掷出致命的回旋镖，260%伤害。"},
    {71, "大地践踏", SKILL_ACTIVE_PHYSICAL, 6, 35, 3.3, 0, "狂野踩踏大地引发震动，330%伤害。"},
    {72, "自然之怒", SKILL_ACTIVE_MAGIC, 6, 55, 4.8, 0, "引动荒野的落雷与狂风，480%伤害。"},
    {73, "生存本能", SKILL_ACTIVE_HEAL, 6, 30, 0, 400, "激发肾上腺素，瞬间恢复400点生命。"},
    {74, "狂犬撕咬", SKILL_ACTIVE_PHYSICAL, 6, 12, 1.5, 0, "野性攻击，150%伤害。"},
    {75, "毒蛇之牙", SKILL_ACTIVE_PHYSICAL, 6, 18, 2.0, 0, "如毒蛇般迅猛咬击，200%伤害。"},
    {76, "野性呼唤", SKILL_ACTIVE_BUFF, 6, 20, 0, 120, "召唤野兽的力量，临时增加120点攻击力。"},
    {77, "丛林迷踪", SKILL_ACTIVE_BUFF, 6, 25, 0, 100, "融入丛林的迷雾中，临时增加100点防御力。"},
    {78, "猛禽之爪", SKILL_ACTIVE_PHYSICAL, 6, 40, 3.5, 0, "如猛禽般撕裂敌人，350%伤害。"},
    {79, "自然之拥", SKILL_ACTIVE_HEAL, 6, 20, 0, 250, "大自然的怀抱，恢复250点HP。"},
    
    // ------------------ 【区域 7：大洋海神/风暴 (海族)】 ------------------
    {80, "怒涛穿刺", SKILL_ACTIVE_PHYSICAL, 7, 20, 2.3, 0, "用三叉戟猛刺，230%物理伤害。"},
    {81, "惊涛骇浪", SKILL_ACTIVE_MAGIC, 7, 40, 3.8, 0, "卷起海啸吞没敌人，380%魔法伤害。"},
    {82, "风暴之眼", SKILL_ACTIVE_MAGIC, 7, 65, 5.5, 0, "将敌人扯入飓风中心撕碎，550%伤害。"},
    {83, "海神之怒", SKILL_ACTIVE_MAGIC, 7, 110, 8.2, 0, "[觉醒] 亚特兰蒂斯的终极制裁，820%绝命伤害！"},
    {84, "潮汐治愈", SKILL_ACTIVE_HEAL, 7, 25, 0, 350, "温柔的海水洗涤伤口，恢复350HP。"},
    {85, "深海护盾", SKILL_ACTIVE_BUFF, 7, 35, 0, 120, "召唤水流护甲，临时增加120点防御。"},
    {86, "暴风之怒", SKILL_ACTIVE_BUFF, 7, 30, 0, 150, "引导风暴的力量，临时增加150点攻击力。"},
    {87, "海浪冲击", SKILL_ACTIVE_MAGIC, 7, 50, 4.5, 0, "召唤巨浪拍击敌人，450%伤害。"},
    {88, "深海之拥", SKILL_ACTIVE_HEAL, 7, 20, 0, 300, "深海的温暖怀抱，恢复300点HP。"},
    {89, "水龙卷", SKILL_ACTIVE_MAGIC, 7, 80, 6.0, 0, "形成巨大的水龙卷席卷敌人，600%伤害。"},

    // ------------------ 【补充 106】 ------------------
    {90, "暗影步刺", SKILL_ACTIVE_PHYSICAL, 3, 10, 1.4, 0, "背后偷袭，140%伤害。"},
    {91, "炎爆术", SKILL_ACTIVE_MAGIC, 1, 28, 2.7, 0, "巨大的火球炸裂，270%魔法伤害。"},
    {92, "治愈之风", SKILL_ACTIVE_HEAL, 4, 35, 0, 600, "气功师的温柔阵法，恢复600点HP。"},
    {93, "神枪手前踢", SKILL_ACTIVE_PHYSICAL, 5, 5, 1.1, 0, "体术，110%伤害。"},
    {94, "踏射", SKILL_ACTIVE_PHYSICAL, 5, 18, 1.9, 0, "踩住敌人开火，190%伤害。"},
    {95, "落花掌", SKILL_ACTIVE_PHYSICAL, 4, 12, 1.5, 0, "将敌人击飞的掌法，150%伤害。"},
    {96, "圆舞棍", SKILL_ACTIVE_PHYSICAL, 2, 22, 2.4, 0, "战法抓取技能，240%伤害。"},
    {97, "地裂·波动剑", SKILL_ACTIVE_MAGIC, 4, 10, 1.5, 0, "剑气扫地，150%魔法伤害。"},
    {98, "冰刃·波动剑", SKILL_ACTIVE_MAGIC, 4, 25, 2.6, 0, "喷发冰柱，260%魔法伤害。"},
    {99, "爆炎·波动剑", SKILL_ACTIVE_MAGIC, 4, 45, 4.2, 0, "引发剧烈爆炸的剑气，420%魔法伤害。"},
    {100, "邪光斩", SKILL_ACTIVE_MAGIC, 4, 30, 3.0, 0, "斩出巨大的交叉剑气，300%魔法伤害。"},
    {101, "暗天波动眼", SKILL_ACTIVE_MAGIC, 4, 100, 7.5, 0, "[觉醒] 开启修罗邪眼，粉碎空间，750%伤害！"},
    {102, "龙牙", SKILL_ACTIVE_PHYSICAL, 2, 8, 1.3, 0, "快速刺击，130%伤害。"},
    {103, "天击", SKILL_ACTIVE_PHYSICAL, 2, 8, 1.3, 0, "上挑攻击，130%伤害。"},
    {104, "嗜血重击", SKILL_ACTIVE_PHYSICAL, 2, 35, 3.5, 0, "不顾一切的重砸，350%伤害。"},
    {105, "圣水泼洒", SKILL_ACTIVE_MAGIC, 1, 15, 1.6, 0, "用圣水腐蚀邪恶，160%魔法伤害。"},
    {106, "诅咒之箭", SKILL_ACTIVE_MAGIC, 5, 20, 2.1, 0, "射出带有巫毒的箭矢，210%伤害。"},

    // =========================================================================
    // 【被动技能 (编号 201 - 253 共 50种被动)】
    // =========================================================================
    {201, "巨剑精通", SKILL_PASSIVE_STAT, 0, 0, 0, 20, "永久提升20%总物理攻击力。"},
    {202, "太刀精通", SKILL_PASSIVE_STAT, 0, 0, 0, 10, "永久提升10%攻击力与10%暴击率。"},
    {203, "左轮精通", SKILL_PASSIVE_STAT, 0, 0, 0, 15, "永久提升15%暴击率。"},
    {204, "板甲精通", SKILL_PASSIVE_STAT, 0, 0, 0, 30, "永久提升30%总防御力。"},
    {205, "皮甲精通", SKILL_PASSIVE_STAT, 0, 0, 0, 15, "永久提升15%闪避率。"},
    {206, "布甲精通", SKILL_PASSIVE_STAT, 0, 0, 0, 0,  "每次战斗自动恢复10%的魔法值。"},
    {210, "吸血鬼血脉", SKILL_PASSIVE_TRIG, 1, 0, 0, 0, "造成伤害时恢复伤害量10%的HP。"},
    {211, "荆棘之甲", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "受到怪物攻击反弹20%真实伤害。"},
    {212, "破釜沉舟", SKILL_PASSIVE_TRIG, 4, 0, 0, 0, "HP低于30%时，攻击力临时翻倍！"},
    {213, "魔法记忆", SKILL_PASSIVE_STAT, 1, 0, 0, 0, "主动技能的魔法消耗(MP)降低30%。"},
    {214, "刺客信条", SKILL_PASSIVE_STAT, 3, 0, 0, 20, "暴击率永久增加20%。"},
    {215, "武者步法", SKILL_PASSIVE_STAT, 4, 0, 0, 20, "闪避率永久增加20%。"},
    {216, "神之恩赐", SKILL_PASSIVE_TRIG, 1, 0, 0, 0, "受到致命伤害有30%几率保留1点HP。"},
    {217, "野性狂暴", SKILL_PASSIVE_TRIG, 6, 0, 0, 0, "使用普攻时有30%几率额外追击一次。"},
    {218, "海洋庇护", SKILL_PASSIVE_TRIG, 7, 0, 0, 0, "每回合开始时，恢复最大HP的5%。"},
    {219, "财富猎手", SKILL_PASSIVE_TRIG, 5, 0, 0, 0, "战斗胜利后，获得的铜贝数量增加50%。"},
    {220, "强健体魄", SKILL_PASSIVE_STAT, 0, 0, 0, 15, "防御力永久+15%。"},
    {221, "肌肉记忆", SKILL_PASSIVE_STAT, 0, 0, 0, 15, "攻击力永久+15%。"},
    {222, "鹰眼", SKILL_PASSIVE_STAT, 0, 0, 0, 10, "暴击率+10%。"},
    {223, "风之灵动", SKILL_PASSIVE_STAT, 0, 0, 0, 10, "闪避率+10%。"},
    {224, "冥想", SKILL_PASSIVE_STAT, 0, 0, 0, 0, "技能耗蓝降低15%。"},
    {225, "毒抗皮肤", SKILL_PASSIVE_STAT, 2, 0, 0, 10, "防御力+10%。"},
    {226, "信仰之盾", SKILL_PASSIVE_STAT, 1, 0, 0, 20, "防御力+20%。"},
    {227, "残影诀", SKILL_PASSIVE_STAT, 4, 0, 0, 25, "闪避率+25%！极限身法。"},
    {228, "致命本能", SKILL_PASSIVE_STAT, 3, 0, 0, 25, "暴击率+25%！极限杀意。"},
    {229, "魔力涌动", SKILL_PASSIVE_STAT, 1, 0, 0, 0, "技能耗蓝降低40%！"},
    {230, "血十字", SKILL_PASSIVE_TRIG, 4, 0, 0, 0, "HP低于50%时，暴击率提升30%。"},
    {231, "复仇之心", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "受到暴击后，下一击必然暴击。"},
    {232, "法力护盾", SKILL_PASSIVE_TRIG, 1, 0, 0, 0, "受到致命伤害时，消耗MP抵挡伤害。"},
    {233, "龙族血统", SKILL_PASSIVE_STAT, 0, 0, 0, 25, "全属性(攻防)提升15%。"},
    {234, "狂神之力", SKILL_PASSIVE_STAT, 0, 0, 0, 35, "攻击力提升35%，防御降低20%。"},
    {235, "龟甲功", SKILL_PASSIVE_STAT, 4, 0, 0, 40, "防御力提升40%，但闪避降为0。"},
    {236, "不屈意志", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "生命低于30%时防御力翻倍！"},
    {237, "剑术奥义", SKILL_PASSIVE_STAT, 0, 0, 0, 0, "物理技能伤害增加15%。"},
    {238, "元素奥义", SKILL_PASSIVE_STAT, 0, 0, 0, 0, "魔法技能伤害增加15%。"},
    {239, "愈合体质", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "每回合自动恢复2%的最大HP与MP。"},
    {240, "洞察", SKILL_PASSIVE_STAT, 0, 0, 0, 5, "暴击率与闪避率各+5%。"},
    {241, "巨人杀手", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "攻击等级高于自己的怪物伤害增加30%。"},
    {242, "法术反击", SKILL_PASSIVE_TRIG, 1, 0, 0, 0, "受到伤害时，反弹20%魔法伤害。"},
    {243, "嗜血渴望", SKILL_PASSIVE_TRIG, 4, 0, 0, 0, "暴击时吸取造成伤害30%的生命值！"},
    {244, "狂热", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "每次攻击都会使攻击力临时+5，无限叠加！"},
    {245, "不动如山", SKILL_PASSIVE_STAT, 0, 0, 0, 0, "放弃所有闪避率，换取全局免伤20%。"},
    {246, "风暴行者", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "成功闪避后，下一次攻击必定暴击。"},
    {247, "海神祝福", SKILL_PASSIVE_STAT, 7, 0, 0, 0, "受到所有来源的伤害降低10%。"},
    {248, "黄金眼", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "怪物掉落铜币再额外增加30%！"},
    {249, "经验学者", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "战斗胜利后获得的经验值提升30%。"},
    {250, "神圣庇护", SKILL_PASSIVE_TRIG, 1, 0, 0, 0, "致死保留1点生命的概率提高至50%！"},
    {251, "反制", SKILL_PASSIVE_TRIG, 0, 0, 0, 0, "被攻击时有20%概率自动反击一次普攻。"},
    {252, "魔力回流", SKILL_PASSIVE_TRIG, 1, 0, 0, 0, "每次造成伤害恢复 5 点 MP。"},
    {253, "极限过载", SKILL_PASSIVE_TRIG, 5, 0, 0, 0, "释放技能不耗蓝，但消耗等额HP！"}
};
const int zh_data_skills_count = sizeof(zh_data_skills) / sizeof(ZH_Skill);

const ZH_Skill* get_skill_by_id(int id) {
    if(id <= 0) return NULL;
    for(int i=0; i<zh_data_skills_count; i++) {
        if(zh_data_skills[i].id == id) return &zh_data_skills[i];
    }
    return NULL;
}

// =========================================================================
// 【回合制状态机引擎与战斗变量】
// =========================================================================
static lv_obj_t* modal_combat = NULL;
static lv_obj_t* lbl_combat_log = NULL;
static lv_obj_t* lbl_monster_status = NULL;
static lv_obj_t* lbl_player_status = NULL;

static lv_obj_t* modal_combat_items = NULL;
static lv_obj_t* list_combat_items = NULL;
static lv_obj_t* combat_btn_grid = NULL;

static int combat_monster_idx = -1; 
static int current_monster_id = -1;
static int m_hp = 0;
static int m_max_hp = 0;
static int m_mp = 0;
static int m_max_mp = 0;

static int player_temp_atk_buff = 0;
static int player_temp_def_buff = 0;
static int player_temp_frenzy_atk = 0; 

static char combat_log_buffer[1024] = "";

static void process_monster_turn();
static void end_combat(bool player_won, bool fled);

// ==================== 安全重写后的追加日志函数 ====================
static void add_combat_log(const char* msg) {
    if(!lbl_combat_log || !msg) return;
    
    size_t msg_len = strlen(msg);
    size_t buf_size = sizeof(combat_log_buffer);
    
    // 终极保险：如果单条消息太长，直接抛弃旧日志
    if (msg_len >= buf_size / 2) {
        combat_log_buffer[0] = '\0'; // 遇到超长文本直接清空
    }
    
    // 动态检查剩余空间，留出充足余量 (包括换行符和终止符)
    if (strlen(combat_log_buffer) + msg_len + 2 >= buf_size) {
        // 当拼接后总长度可能越界时，进行安全的居中裁剪
        const char* p = combat_log_buffer + (buf_size / 2);
        // 向后寻找换行符以保证句子完整性
        while(*p && *p != '\n') p++;
        if(*p == '\n') p++;
        
        // 如果截断后剩余长度 + 新消息长度仍然越界，或者没找到换行，直接清空缓冲池
        if (*p == '\0' || (strlen(p) + msg_len + 2 >= buf_size)) {
            combat_log_buffer[0] = '\0';
        } else {
            // 将后半部分移到头部
            memmove(combat_log_buffer, p, strlen(p) + 1); 
        }
    }
    
    // 安全拼接，绝对防止溢出
    strncat(combat_log_buffer, msg, buf_size - strlen(combat_log_buffer) - 1);
    strncat(combat_log_buffer, "\n", buf_size - strlen(combat_log_buffer) - 1);
    
    lv_label_set_text(lbl_combat_log, combat_log_buffer);
}

static void get_player_battle_stats(int* atk, int* def, int* crit, int* dodge) {
    *atk = get_total_atk() + player_temp_atk_buff + player_temp_frenzy_atk;
    *def = get_total_def() + player_temp_def_buff;
    *crit = zh_player.base_crit; 
    *dodge = zh_player.base_dodge; 
    
    // 载入被动技能属性
    for(int i=0; i<4; i++) {
        int sid = zh_player.eq_passive_skills[i];
        if(sid <= 0) continue;
        const ZH_Skill* sk = get_skill_by_id(sid);
        if(!sk) continue;
        
        if(sk->id == 201) *atk = (*atk) * 1.2;
        if(sk->id == 202) { *atk = (*atk) * 1.1; *crit += 10; }
        if(sk->id == 203) *crit += 15;
        if(sk->id == 204) *def = (*def) * 1.3;
        if(sk->id == 205) *dodge += 15;
        if(sk->id == 206) ; // MP恢复逻辑独立处理
        if(sk->id == 212 && zh_player.hp < zh_player.max_hp * 0.3) *atk *= 2; 
        if(sk->id == 214) *crit += 20;
        if(sk->id == 215) *dodge += 20;
        if(sk->id == 220) *def = (*def) * 1.15;
        if(sk->id == 221) *atk = (*atk) * 1.15;
        if(sk->id == 222) *crit += 10;
        if(sk->id == 223) *dodge += 10;
        if(sk->id == 227) *dodge += 25;
        if(sk->id == 228) *crit += 25;
        if(sk->id == 230 && zh_player.hp < zh_player.max_hp * 0.5) *crit += 30; 
        if(sk->id == 233) { *atk *= 1.15; *def *= 1.15; }
        if(sk->id == 234) { *atk *= 1.35; *def *= 0.8; }
        if(sk->id == 235) { *def *= 1.4; *dodge = 0; }
        if(sk->id == 236 && zh_player.hp < zh_player.max_hp * 0.3) *def *= 2;
        if(sk->id == 240) { *crit += 5; *dodge += 5; }
        if(sk->id == 245) { *dodge = 0; } 
    }

    // 载入副官加成 (冲锋队长 - 增加暴击)
    const ZH_Adjutant* adj_assault = get_adjutant_by_id(zh_player.eq_adj_assault);
    if (adj_assault) {
        *crit += 15 * adj_assault->power_mult; 
    }
}

static int get_actual_mp_cost(const ZH_Skill* sk) {
    if(!sk) return 0;
    int cost = sk->mp_cost;
    for(int i=0; i<4; i++) {
        int sid = zh_player.eq_passive_skills[i];
        if(sid == 213) cost = cost * 70 / 100;
        if(sid == 224) cost = cost * 85 / 100;
        if(sid == 229) cost = cost * 60 / 100;
        // 删除了 if(sid == 253) cost = 0; 这一行
    }
    return cost;
}

static void update_combat_status() {
    static char buf[128];
    const ZH_Monster* m = &zh_data_monsters[current_monster_id];
    snprintf(buf, sizeof(buf), "敌:[%s] Lv.%d\nHP:%d / %d", m->name, m->level, m_hp, m_max_hp);
    lv_label_set_text(lbl_monster_status, buf);
    snprintf(buf, sizeof(buf), "我:[%s] Lv.%d\nHP:%d/%d  MP:%d/%d", 
            zh_player.name, zh_player.level, zh_player.hp, zh_player.max_hp, zh_player.mp, zh_player.max_mp);
    lv_label_set_text(lbl_player_status, buf);
}

// ==== 战斗中处理道具逻辑 ====
static void process_player_item(int inv_idx) {
    lv_obj_add_flag(modal_combat_items, LV_OBJ_FLAG_HIDDEN);
    int iid = zh_player.inventory[inv_idx];
    ZH_Item it = get_item_by_id(iid);
    zh_player.inventory[inv_idx] = -1; // 消耗道具

    static char log_buf[256];
    if(it.type == 6 || (it.type == 9 && it.id >= 326 && it.id <= 340)) { 
        zh_player.hp += it.value; if(zh_player.hp > zh_player.max_hp) zh_player.hp = zh_player.max_hp;
        if(it.id == 106 || it.id == 339 || it.id == 340 || it.id == 412) { zh_player.hp = zh_player.max_hp; zh_player.mp = zh_player.max_mp; }
        if(it.id == 413) { zh_player.mp += 1000; if(zh_player.mp>zh_player.max_mp) zh_player.mp=zh_player.max_mp; }
        if(it.id == 417) { zh_player.crime_value = 0; } // 忘忧草清除罪恶
        snprintf(log_buf, sizeof(log_buf), "使用了【%s】！生命/状态恢复。", it.name);
    } 
    else if (it.type == 7 || (it.type == 9 && it.id >= 341 && it.id <= 350)) { 
        player_temp_atk_buff += it.value; player_temp_def_buff += (it.value / 2); 
        if(it.id == 414) { zh_player.atk += 10; zh_player.def += 10; snprintf(log_buf, sizeof(log_buf), "洗髓灵芝让你脱胎换骨，永久攻防+10！"); }
        else snprintf(log_buf, sizeof(log_buf), "使用了【%s】！攻击与防御大幅提升！", it.name);
    }
    else if (it.type == 9 && it.id >= 301 && it.id <= 325) { 
        int dmg = it.value; m_hp -= dmg;
        snprintf(log_buf, sizeof(log_buf), "丢出【%s】！造成 %d 点无视防御的真实伤害！", it.name, dmg);
    }

    add_combat_log(log_buf); update_combat_status();
    if (m_hp <= 0) end_combat(true, false); else process_monster_turn();
}

// ==== 刷新战斗道具弹窗 ====
static void open_combat_item_menu() {
    lv_obj_clean(list_combat_items);
    lv_obj_t * btn_close = lv_list_add_btn(list_combat_items, LV_SYMBOL_CLOSE, "取消使用道具");
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0xc0392b), 0);
    lv_obj_t * lbl_c = lv_obj_get_child(btn_close, 1); if(lbl_c) lv_obj_add_style(lbl_c, &style_cn, 0);
    lv_obj_add_event_cb(btn_close, [](lv_event_t *e){ lv_obj_add_flag(modal_combat_items, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    bool has_item = false;
    for(int i=0; i<50; i++) {
        int iid = zh_player.inventory[i];
        if(iid != -1) {
            ZH_Item it = get_item_by_id(iid);
            if(it.type == 6 || it.type == 7 || it.type == 9) {
                has_item = true;
                char buf[64]; snprintf(buf, sizeof(buf), "%s", it.name);
                lv_obj_t* btn = lv_list_add_btn(list_combat_items, LV_SYMBOL_CHARGE, buf);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x27ae60), 0);
                lv_obj_t * lbl = lv_obj_get_child(btn, 1); if(lbl) lv_obj_add_style(lbl, &style_cn, 0);
                
                lv_obj_add_event_cb(btn, [](lv_event_t* e){
                    int inv_idx = (int)(intptr_t)lv_event_get_user_data(e);
                    process_player_item(inv_idx);
                }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
            }
        }
    }
    if(!has_item) {
        lv_obj_t * l_emp = lv_label_create(list_combat_items); lv_obj_add_style(l_emp, &style_cn, 0);
        lv_label_set_text(l_emp, "\n背包里没有可以在战斗中\n使用的道具或药水。"); lv_obj_center(l_emp);
    }
    lv_obj_clear_flag(modal_combat_items, LV_OBJ_FLAG_HIDDEN);
}

static void end_combat(bool player_won, bool fled) {
    lv_obj_add_flag(modal_combat, LV_OBJ_FLAG_HIDDEN);
    player_temp_atk_buff = 0; player_temp_def_buff = 0; player_temp_frenzy_atk = 0;

    // 如果逃跑成功
    if (fled) {
        static char flee_buf[256];
        strcpy(flee_buf, "【战斗结束】\n你狼狈地逃跑了！");
        // 船医赛后恢复效果
        const ZH_Adjutant* adj_doc = get_adjutant_by_id(zh_player.eq_adj_doctor);
        if (adj_doc) {
            int heal = zh_player.max_hp * 0.2 * adj_doc->power_mult;
            zh_player.hp += heal;
            if(zh_player.hp > zh_player.max_hp) zh_player.hp = zh_player.max_hp;
            snprintf(flee_buf + strlen(flee_buf), 256 - strlen(flee_buf), "\n船医【%s】为你疗伤，恢复了 %d 点生命。", adj_doc->name, heal);
        }
        zh_log(flee_buf);
        lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
        return;
    }
    
    // 如果玩家战死
    if (!player_won) { player_die(); return; }

    // 如果玩家获胜
    const ZH_Monster* m = &zh_data_monsters[current_monster_id];
    int final_gold = m->gold_drop; int final_exp = m->exp_drop;
    for(int i=0; i<4; i++) {
        if(zh_player.eq_passive_skills[i] == 219) final_gold *= 1.5;
        if(zh_player.eq_passive_skills[i] == 248) final_gold *= 1.3;
        if(zh_player.eq_passive_skills[i] == 249) final_exp *= 1.3;
    }

    zh_player.exp += final_exp; zh_player.gold += final_gold;
    zh_player.current_monsters[combat_monster_idx] = -1; 
    process_quest_kill(m->id);
    static char win_buf[512];
    snprintf(win_buf, sizeof(win_buf), "击杀【%s】！\n经验+%d, 铜贝+%d", m->name, final_exp, final_gold);

    // 战利品掉落判定
    if (rand() % 100 < 35) { 
        int drop_id = get_random_drop_id();
        if (add_item_to_bag(drop_id)) {
            static char drop_msg[128];
            // 提前计算，杜绝求值顺序带来的缓冲区覆写
            const char* item_name = get_item_by_id(drop_id).name;
            snprintf(drop_msg, sizeof(drop_msg), "\n掉落并拾取：【%s】！", item_name);
            strncat(win_buf, drop_msg, sizeof(win_buf) - strlen(win_buf) - 1);
        }
    }
    
    // 船医赛后恢复效果
    const ZH_Adjutant* adj_doc = get_adjutant_by_id(zh_player.eq_adj_doctor);
    if (adj_doc) {
        int heal = zh_player.max_hp * 0.2 * adj_doc->power_mult;
        zh_player.hp += heal;
        if(zh_player.hp > zh_player.max_hp) zh_player.hp = zh_player.max_hp;
        static char doc_msg[128];
        snprintf(doc_msg, sizeof(doc_msg), "\n船医【%s】为你包扎伤口，恢复 %d 点生命。", adj_doc->name, heal);
        strncat(win_buf, doc_msg, sizeof(win_buf) - strlen(win_buf) - 1);
    }
    
    zh_log(win_buf); check_levelup(); lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
}

static void process_monster_turn() {
    if (m_hp <= 0) return; 
    const ZH_Monster* m = &zh_data_monsters[current_monster_id];
    static char log_buf[256];
    int p_atk, p_def, p_crit, p_dodge; get_player_battle_stats(&p_atk, &p_def, &p_crit, &p_dodge);
    
    // --- 罪恶惩罚：路人补刀 / 赏金猎人 ---
    if (zh_player.crime_value >= 150 && rand() % 100 < 15) {
        int g_dmg = zh_player.level * 6 + 50 + (zh_player.crime_value / 10);
        zh_player.hp -= g_dmg;
        add_combat_log("【天网恢恢】暗处的赏金猎人朝你放了冷枪！(生命扣减)");
    }
    // --- 声望奖励：侠客义举 ---
    if (zh_player.reputation >= 200 && rand() % 100 < 18) {
        int a_dmg = m->hp * 0.15; if (a_dmg < 20) a_dmg = 20; 
        m_hp -= a_dmg;
        add_combat_log("【名震四海】一名路过的侠士认出了你，拔刀相助重创怪物！");
    }

    if (rand() % 100 < p_dodge) {
        snprintf(log_buf, sizeof(log_buf), "%s 的攻击被你【闪避】了！", m->name);
        for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 246) strcat(log_buf, " (风暴行者充能)");
        add_combat_log(log_buf); update_combat_status(); return;
    }

    bool used_skill = false;
    if (m->level >= 15 && rand() % 100 < 30 && m_mp >= 20) {
        int random_skill_id = (m->region_id == 4) ? ((rand()%2==0)? 4:12) : 
                              (m->region_id == 1) ? ((rand()%2==0)? 22:28) : 
                              (m->region_id == 3) ? 41 : 
                              (m->region_id == 5) ? 50 : 8; 

        const ZH_Skill* m_sk = get_skill_by_id(random_skill_id);
        if (m_sk) {
            used_skill = true; m_mp -= 20; 
            int dmg = (m->atk * m_sk->power_mult) - p_def; if (dmg < 1) dmg = 1;
            for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 245) dmg *= 0.8; 
            for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 247) dmg *= 0.9; 
            
            zh_player.hp -= dmg;
            snprintf(log_buf, sizeof(log_buf), "%s 使用了【%s】！对你造成 %d 伤害！", m->name, m_sk->name, dmg);
        }
    }

    if (!used_skill) {
        bool is_crit = (rand() % 100 < 10); 
        int dmg = m->atk - p_def; if (dmg < 1) dmg = 1; if (is_crit) dmg *= 2;
        
        for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 245) dmg *= 0.8;
        for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 247) dmg *= 0.9;
        
        zh_player.hp -= dmg;
        snprintf(log_buf, sizeof(log_buf), "%s 发动攻击！%s造成 %d 伤害。", m->name, is_crit ? "【暴击】" : "", dmg);
    }
    add_combat_log(log_buf);

    for(int i=0; i<4; i++) {
        if(zh_player.eq_passive_skills[i] == 211 || zh_player.eq_passive_skills[i] == 242) {
            int thorn_dmg = (m->atk - p_def > 0 ? m->atk - p_def : 1) * 0.2;
            if(thorn_dmg > 0) { m_hp -= thorn_dmg; char tb[64]; snprintf(tb, sizeof(tb), "反伤装甲反弹 %d 伤害！", thorn_dmg); add_combat_log(tb); }
        }
    }
    
    if (zh_player.hp <= 0) {
        for(int i=0; i<4; i++) {
            int sid = zh_player.eq_passive_skills[i]; int prob = (sid == 250) ? 50 : (sid == 216 ? 30 : 0);
            if (prob > 0 && rand() % 100 < prob) { zh_player.hp = 1; add_combat_log("【神迹降临】你死死保住了最后一口气！(生命剩1)"); break; }
        }
    }
    update_combat_status(); 
    
    if (zh_player.hp <= 0) {
        end_combat(false, false);
    } else if (m_hp <= 0) {
        end_combat(true, false); 
    }
}

static void process_player_action(int action_type, int active_skill_idx) {
    static char log_buf[256]; const ZH_Monster* m = &zh_data_monsters[current_monster_id];

    for(int i=0; i<4; i++) {
        if(zh_player.eq_passive_skills[i] == 218) zh_player.hp += zh_player.max_hp * 0.05;
        if(zh_player.eq_passive_skills[i] == 206) zh_player.mp += zh_player.max_mp * 0.1;
        if(zh_player.eq_passive_skills[i] == 239) { zh_player.hp += zh_player.max_hp * 0.02; zh_player.mp += zh_player.max_mp * 0.02; }
    }
    if(zh_player.hp > zh_player.max_hp) zh_player.hp = zh_player.max_hp; if(zh_player.mp > zh_player.max_mp) zh_player.mp = zh_player.max_mp;

    if (action_type == 0) { 
        int esc_chance = 50 + (zh_player.level - m->level)*2;
        // 载入副官加成 (冲锋队长 - 增加逃跑率)
        const ZH_Adjutant* adj_assault = get_adjutant_by_id(zh_player.eq_adj_assault);
        if (adj_assault) esc_chance += 30 * adj_assault->power_mult;

        if (rand() % 100 < esc_chance) end_combat(false, true);
        else { add_combat_log("逃跑失败！怪物挡住了去路！"); process_monster_turn(); }
        return;
    }

    int p_atk, p_def, p_crit, p_dodge; get_player_battle_stats(&p_atk, &p_def, &p_crit, &p_dodge);
    int raw_dmg = 0; bool is_crit = (rand() % 100 < p_crit);

    auto do_dmg_passive = [&](int final_dmg) {
        m_hp -= final_dmg;
        for(int i=0; i<4; i++) {
            if(zh_player.eq_passive_skills[i] == 210) zh_player.hp += final_dmg * 0.1;
            if(zh_player.eq_passive_skills[i] == 243 && is_crit) zh_player.hp += final_dmg * 0.3;
            if(zh_player.eq_passive_skills[i] == 252) zh_player.mp += 5;
            if(zh_player.eq_passive_skills[i] == 244) player_temp_frenzy_atk += 5;
        }
        if(zh_player.hp > zh_player.max_hp) zh_player.hp = zh_player.max_hp; if(zh_player.mp > zh_player.max_mp) zh_player.mp = zh_player.max_mp;
    };

    if (action_type == 1) { 
        raw_dmg = p_atk; if (is_crit) raw_dmg *= 2;
        int final_dmg = raw_dmg - m->def; if (final_dmg < 1) final_dmg = 1;
        for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 241 && m->level > zh_player.level) final_dmg *= 1.3;
        
        do_dmg_passive(final_dmg); snprintf(log_buf, sizeof(log_buf), "你挥舞武器！%s造成 %d 伤害。", is_crit ? "【爆】" : "", final_dmg);
        
        bool combo = false; for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 217 && rand()%100 < 30) combo = true;
        if(combo) { m_hp -= final_dmg; strcat(log_buf, "【野性连击】再次造成同等伤害！"); }
        add_combat_log(log_buf);

    } else if (action_type == 2) { 
        int sid = zh_player.eq_active_skills[active_skill_idx];
        if (sid <= 0) { add_combat_log("槽位为空！"); return; }
        const ZH_Skill* sk = get_skill_by_id(sid);
        if (!sk) { add_combat_log("数据错乱：未知技能！"); return; }
        
        int actual_mp_cost = get_actual_mp_cost(sk);
        bool overload = false;
        for(int i=0; i<4; i++) if(zh_player.eq_passive_skills[i] == 253) overload = true;

        if (overload) {
            if(zh_player.hp <= actual_mp_cost) {
                add_combat_log("生命值不足以过载释放！");
                return;
            }
            zh_player.hp -= actual_mp_cost;
        } else {
            if (zh_player.mp < actual_mp_cost) {
                add_combat_log("魔法(MP)不足，释放失败！");
                return;
            }
            zh_player.mp -= actual_mp_cost;
        }

        if (sk->type == SKILL_ACTIVE_PHYSICAL || sk->type == SKILL_ACTIVE_MAGIC) {
            int base_atk = (sk->type == SKILL_ACTIVE_MAGIC) ? p_atk * 1.2 : p_atk; 
            raw_dmg = base_atk * sk->power_mult; if (is_crit) raw_dmg *= 2;
            int def_calc = (sk->type == SKILL_ACTIVE_MAGIC) ? m->def * 0.5 : m->def; 
            int final_dmg = raw_dmg - def_calc; if (final_dmg < 1) final_dmg = 1;

            for(int i=0; i<4; i++) {
                if(zh_player.eq_passive_skills[i] == 241 && m->level > zh_player.level) final_dmg *= 1.3;
                if(zh_player.eq_passive_skills[i] == 237 && sk->type == SKILL_ACTIVE_PHYSICAL) final_dmg *= 1.15;
                if(zh_player.eq_passive_skills[i] == 238 && sk->type == SKILL_ACTIVE_MAGIC) final_dmg *= 1.15;
            }
            do_dmg_passive(final_dmg); snprintf(log_buf, sizeof(log_buf), "【%s】！%s造成 %d 伤害！", sk->name, is_crit ? "[爆]" : "", final_dmg);
            
        } else if (sk->type == SKILL_ACTIVE_HEAL) {
            zh_player.hp += sk->effect_value; if(zh_player.hp > zh_player.max_hp) zh_player.hp = zh_player.max_hp;
            snprintf(log_buf, sizeof(log_buf), "【%s】恢复了 %d 点生命！", sk->name, sk->effect_value);
        } else if (sk->type == SKILL_ACTIVE_BUFF) {
            if(sk->effect_value >= 100) { player_temp_atk_buff += sk->effect_value; player_temp_def_buff += sk->effect_value; }
            else player_temp_atk_buff += sk->effect_value;
            snprintf(log_buf, sizeof(log_buf), "【%s】属性提升！", sk->name);
        }
        add_combat_log(log_buf);
    }

    update_combat_status();
    if (m_hp <= 0) end_combat(true, false); else process_monster_turn();
}

void start_turn_based_combat(int monster_idx) {
    combat_monster_idx = monster_idx;
    current_monster_id = zh_player.current_monsters[monster_idx];
    const ZH_Monster* m = &zh_data_monsters[current_monster_id];

    m_max_hp = m->hp; m_hp = zh_player.current_monsters_hp[monster_idx];
    m_max_mp = 100 + m->level*5; m_mp = m_max_mp; 
    
    if(zh_player.max_mp == 0) { 
        zh_player.max_mp = 100 + zh_player.level*10; zh_player.mp = zh_player.max_mp; 
        zh_player.base_crit = 5; zh_player.base_dodge = 5;
    }

    if (!modal_combat) {
        modal_combat = lv_obj_create(scr_zongheng);
        lv_obj_set_size(modal_combat, 240, 320);
        lv_obj_center(modal_combat);
        lv_obj_set_style_bg_color(modal_combat, lv_color_hex(0x1a1a1a), 0);
        lv_obj_clear_flag(modal_combat, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(modal_combat, 0, 0);

        // 1. 顶部状态
        lv_obj_t* cont_status = lv_obj_create(modal_combat);
        lv_obj_set_size(cont_status, 230, 50);
        lv_obj_align(cont_status, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_set_style_bg_color(cont_status, lv_color_hex(0x222222), 0);
        lv_obj_set_style_border_width(cont_status, 0, 0);
        lv_obj_set_style_pad_all(cont_status, 6, 0);
        
        lv_obj_add_flag(cont_status, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(cont_status, LV_SCROLLBAR_MODE_AUTO);
        lv_obj_set_flex_flow(cont_status, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cont_status, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(cont_status, 6, 0);
        
        lbl_monster_status = lv_label_create(cont_status);
        lv_obj_add_style(lbl_monster_status, &style_cn, 0);
        lv_obj_set_style_text_color(lbl_monster_status, lv_color_hex(0xFF5555), 0);
        
        lbl_player_status = lv_label_create(cont_status);
        lv_obj_add_style(lbl_player_status, &style_cn, 0);
        lv_obj_set_style_text_color(lbl_player_status, lv_color_hex(0x00FF00), 0);

        // 2. 战斗日志区
        lv_obj_t* cont_log = lv_obj_create(modal_combat);
        lv_obj_set_size(cont_log, 230, 170);
        lv_obj_align(cont_log, LV_ALIGN_TOP_MID, 0, 60); 
        lv_obj_set_style_bg_color(cont_log, lv_color_hex(0x000000), 0);
        lv_obj_set_style_border_width(cont_log, 0, 0);
        
        lbl_combat_log = lv_label_create(cont_log);
        lv_obj_add_style(lbl_combat_log, &style_cn, 0);
        lv_obj_set_style_text_color(lbl_combat_log, lv_color_hex(0x00FF00), 0);
        lv_obj_set_width(lbl_combat_log, 210);

        // 3. 底部带滚动条的 15 格按钮网格
        combat_btn_grid = lv_obj_create(modal_combat);
        lv_obj_set_size(combat_btn_grid, 230, 110);
        lv_obj_align(combat_btn_grid, LV_ALIGN_BOTTOM_MID, 0, -5);
        lv_obj_set_flex_flow(combat_btn_grid, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_style_bg_opa(combat_btn_grid, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(combat_btn_grid, 0, 0);
        lv_obj_set_style_pad_all(combat_btn_grid, 0, 0);
        lv_obj_set_style_pad_row(combat_btn_grid, 4, 0);
        lv_obj_set_style_pad_column(combat_btn_grid, 4, 0);
        lv_obj_add_flag(combat_btn_grid, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(combat_btn_grid, LV_SCROLLBAR_MODE_AUTO);

        auto create_grid_btn = [](lv_obj_t* p, int idx) {
            lv_obj_t* btn = lv_btn_create(p);
            lv_obj_set_size(btn, 74, 34);  
            lv_obj_t* lbl = lv_label_create(btn);
            lv_obj_add_style(lbl, &style_cn, 0);
            lv_obj_set_width(lbl, 68);
            lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_center(lbl);
            
            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                int i = (int)(intptr_t)lv_event_get_user_data(e);
                if(i == 0) process_player_action(1, 0); 
                else if(i == 1) open_combat_item_menu(); 
                else if(i == 2) process_player_action(0, 0); 
                else process_player_action(2, i - 3);    
            }, LV_EVENT_CLICKED, (void*)(intptr_t)idx);
            return btn;
        };

        for(int i=0; i<15; i++) create_grid_btn(combat_btn_grid, i);

        // 4. 战斗专属弹窗物品栏
        modal_combat_items = lv_obj_create(modal_combat);
        lv_obj_set_size(modal_combat_items, 230, 260);
        lv_obj_align(modal_combat_items, LV_ALIGN_BOTTOM_MID, 0, -5);
        lv_obj_set_style_bg_color(modal_combat_items, lv_color_hex(0x1a252f), 0);
        lv_obj_add_flag(modal_combat_items, LV_OBJ_FLAG_HIDDEN);
        
        lv_obj_t * lbl_title = lv_label_create(modal_combat_items);
        lv_obj_add_style(lbl_title, &style_cn, 0);
        lv_label_set_text(lbl_title, "使用战斗道具");
        lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 0);
        
        list_combat_items = lv_list_create(modal_combat_items);
        lv_obj_set_size(list_combat_items, 210, 210);
        lv_obj_align(list_combat_items, LV_ALIGN_TOP_MID, 0, 25);
        lv_obj_set_style_bg_color(list_combat_items, lv_color_hex(0x2c3e50), 0);
        lv_obj_set_style_border_width(list_combat_items, 0, 0);
    }

    strcpy(combat_log_buffer, "=== 战斗开始 ===\n");
    char logb[128]; snprintf(logb, sizeof(logb), "野生的 Lv.%d [%s] 出现了！", m->level, m->name); add_combat_log(logb);
    
    // 重置 15 格按钮状态
    for(int i=0; i<15; i++) {
        lv_obj_t* btn = lv_obj_get_child(combat_btn_grid, i);
        lv_obj_t* lbl = lv_obj_get_child(btn, 0);
        lv_obj_clear_state(btn, LV_STATE_DISABLED);

        if(i == 0) { lv_label_set_text(lbl, "普攻"); lv_obj_set_style_bg_color(btn, lv_color_hex(0x882222), 0); } 
        else if(i == 1) { lv_label_set_text(lbl, "道具"); lv_obj_set_style_bg_color(btn, lv_color_hex(0x27ae60), 0); } 
        else if(i == 2) { lv_label_set_text(lbl, "逃跑"); lv_obj_set_style_bg_color(btn, lv_color_hex(0x444444), 0); } 
        else {
            int s_idx = i - 3; 
            int sid = zh_player.eq_active_skills[s_idx];
            if(sid > 0) {
                const ZH_Skill* sk = get_skill_by_id(sid);
                if (sk) {
                    lv_label_set_text(lbl, sk->name); 
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0x004488), 0);
                } else {
                    lv_label_set_text(lbl, "未知技能"); 
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0);
                }
            } else {
                lv_label_set_text(lbl, "- 未装配 -"); lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0);
                lv_obj_add_state(btn, LV_STATE_DISABLED);
            }
        }
    }

    update_combat_status(); lv_obj_clear_flag(modal_combat, LV_OBJ_FLAG_HIDDEN);
}

void reset_combat_ui_pointers() {
    modal_combat = NULL;
    lbl_combat_log = NULL;
    lbl_monster_status = NULL;
    lbl_player_status = NULL;
    modal_combat_items = NULL;
    list_combat_items = NULL;
    combat_btn_grid = NULL;
}