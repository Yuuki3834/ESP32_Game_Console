#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INVENTORY_SIZE 50

// ==========================================
// 数据结构与字典定义
// ==========================================

struct BountyDef { 
    int id; 
    const char* target_name; 
    int monster_id; 
    int loc_id; 
    int rep_reward; 
    int gold_reward; 
    const char* desc; 
};

const BountyDef bounty_list[] = {
    {0, "【通缉】美第奇叛刃",   24, 34,   150, 15000,  "威尼斯血腥地下祭坛。此人窃取了重宝，护甲极高！"},
    {1, "【通缉】食人族大巫医", 27, 274,  200, 18000,  "开普敦食人族大帐。生食人肉，法力通天。"},
    {2, "【通缉】堕落婆罗门",   39, 513,  250, 25000,  "卡利卡特象牙盗猎王营帐。精通邪恶的印度古法术。"},
    {3, "【通缉】大明水师叛将", 48, 754,  300, 35000,  "泉州东厂督主别院附近。武艺极其高强，手持神兵。"},
    {4, "【通缉】幽灵舰队提督", 56, 994,  400, 50000,  "百慕大边缘迷雾。免疫绝大部分暴击伤害，极度危险！"},
    {5, "【通缉】史前巨鳄霸王", 64, 1234, 500, 65000,  "悉尼食人鳄河口。比战舰还要巨大的远古恶兽！"},
    {6, "【通缉】火神佩蕾分身", 72, 1354, 800, 100000, "檀香山火神祭坛。岩浆化身，靠近者灰飞烟灭！"}
};
const int bounty_count = sizeof(bounty_list) / sizeof(BountyDef);

enum SType { SQ_TALK, SQ_MOVE, SQ_KILL, SQ_COLLECT };

struct StoryStep { 
    SType type; 
    int target_id; 
    int count; 
    const char* npc_name; 
    const char* dialog; 
};

struct StoryDef { 
    int id; 
    const char* name; 
    int start_loc; 
    int step_count; 
    StoryStep steps[15]; 
    int r_gold; 
    int r_rep; 
    int r_item; 
};

const StoryDef story_list[] = {
    // ---------------- 欧洲区域 ----------------
    {0, "【史诗】十字军的圣杯", 0, 9, {
        {SQ_TALK, 5, 1, "红衣主教", "圣杯在雅典遗失，神圣的荣光需要你找回。"},
        {SQ_MOVE, 208, 1, "内心独白", "（你来到了帕特农残影）"},
        {SQ_KILL, 74, 5, "神庙守卫", "（击杀5名堕落圣殿骑士）想拿圣杯？做梦！"},
        {SQ_TALK, 209, 1, "斯巴达后裔", "圣杯被海盗抢去了马赛。"},
        {SQ_MOVE, 167, 1, "内心独白", "（前往普罗旺斯丘陵寻找海盗）"},
        {SQ_KILL, 7, 8, "内心独白", "（击杀8名走私海盗）"},
        {SQ_COLLECT, 252, 5, "黑市商人", "交出5块精铁矿石，我把圣杯的底座给你。"},
        {SQ_MOVE, 0, 1, "内心独白", "（带着圣杯返回威尼斯）"},
        {SQ_TALK, 5, 1, "红衣主教", "主会永远庇佑你！"}
    }, 70000, 1200, 226},
    
    {1, "【史诗】无敌舰队之谜", 0, 7, {
        {SQ_TALK, 6, 1, "威尼斯总督", "调查无敌舰队在伦敦附近沉没的真相。"},
        {SQ_MOVE, 47, 1, "内心独白", "（抵达泰晤士郊野）"},
        {SQ_KILL, 11, 6, "内心独白", "（击杀6名叛逃雇佣兵获取情报）"},
        {SQ_TALK, 49, 1, "吉普赛女巫", "大海的怒火源自阿姆斯特丹的老风车。"},
        {SQ_MOVE, 129, 1, "内心独白", "（前往老风车磨坊）"},
        {SQ_KILL, 15, 6, "内心独白", "（击杀6名黑暗邪教徒）"},
        {SQ_TALK, 6, 1, "威尼斯总督", "原来是邪教徒搞的鬼，这是大明传来的珍宝，赏你。"}
    }, 55000, 750, 10},

    {2, "【史诗】狂欢节暗杀", 0, 6, {
        {SQ_TALK, 6, 1, "威尼斯总督", "狂欢节有人密谋暗杀我，去暗巷替我清理刺客。"},
        {SQ_MOVE, 7, 1, "内心独白", "（潜入威尼斯暗巷）"},
        {SQ_KILL, 17, 8, "暗杀者", "（击杀8名威尼斯暗杀者）"},
        {SQ_COLLECT, 103, 2, "总督亲卫", "总督中毒了！急需2瓶超强生命药水！"},
        {SQ_MOVE, 34, 1, "内心独白", "（杀入血腥地下祭坛捣毁老巢）"},
        {SQ_TALK, 6, 1, "威尼斯总督", "危机解除，威尼斯永远欢迎你。"}
    }, 40000, 500, 20},

    {3, "【中型】波河平原剿匪", 0, 3, { {SQ_TALK, 10, 1, "治安官", "去波河平原杀5个流氓。"}, {SQ_KILL, 0, 5, "内心独白", "（击杀5名街头流氓）"}, {SQ_TALK, 10, 1, "治安官", "治安好多了。"} }, 10000, 100, 0},
    {4, "【中型】收集止血草", 0, 2, { {SQ_TALK, 13, 1, "炼金药水铺", "给我去野外搜集5株止血草。"}, {SQ_COLLECT, 108, 5, "炼金药水铺", "草药很新鲜。"} }, 8000, 80, 0},
    {5, "【中型】跨国押运", 0, 3, { {SQ_TALK, 8, 1, "美第奇银行", "把这笔巨款押送到伦敦。"}, {SQ_MOVE, 40, 1, "内心独白", "（小心海盗，抵达伦敦）"}, {SQ_TALK, 40, 1, "伦敦总督", "资金已安全入库。"} }, 25000, 250, 0},
    {6, "【中型】驱逐泰晤士野狗", 40, 3, { {SQ_TALK, 40, 1, "伦敦市民", "泰晤士郊野野狗伤人。"}, {SQ_KILL, 2, 5, "内心独白", "（击杀5只野狗）"}, {SQ_TALK, 40, 1, "伦敦市民", "终于安全了。"} }, 8000, 100, 0},
    {7, "【中型】罗卡角探查", 80, 3, { {SQ_TALK, 80, 1, "里斯本船长", "去特茹河入海口清理一下海盗。"}, {SQ_KILL, 7, 4, "内心独白", "（击杀4个走私海盗）"}, {SQ_TALK, 80, 1, "里斯本船长", "航道畅通了。"} }, 12000, 120, 0},
    {8, "【中型】低地沼泽除魔", 120, 3, { {SQ_TALK, 120, 1, "阿姆斯特丹守卫", "低地沼泽的水鬼又闹事了。"}, {SQ_KILL, 9, 6, "内心独白", "（击杀6只泥潭怪）"}, {SQ_TALK, 120, 1, "阿姆斯特丹守卫", "花都开得更艳了。"} }, 11000, 110, 0},
    {9, "【中型】普罗旺斯剿匪", 160, 3, { {SQ_TALK, 160, 1, "马赛商人", "我的香水被丘陵上的强盗劫了。"}, {SQ_KILL, 13, 5, "内心独白", "（击杀5名拦路强盗）"}, {SQ_TALK, 160, 1, "马赛商人", "感谢你挽回了损失。"} }, 13000, 130, 0},
    {10, "【中型】雅典神庙的守卫", 200, 3, { {SQ_TALK, 200, 1, "雅典学者", "伯罗奔尼撒荒路不安全，去清理一下。"}, {SQ_KILL, 11, 4, "内心独白", "（击杀4名叛逃佣兵）"}, {SQ_TALK, 200, 1, "雅典学者", "智慧的火花再次燃起。"} }, 14000, 140, 0},

    // ---------------- 非洲区域 ----------------
    {11, "【史诗】黄金王国的远征", 240, 8, {
        {SQ_TALK, 246, 1, "大酋长", "索法拉的黄金之地被军阀占领，我们需要夺回来。"},
        {SQ_MOVE, 400, 1, "内心独白", "（你乘船抵达索法拉）"},
        {SQ_KILL, 11, 8, "内心独白", "（击杀8名叛逃雇佣兵）"},
        {SQ_TALK, 403, 1, "巫医", "我的草药能治愈战士，给我收集 5 个止血草。"},
        {SQ_COLLECT, 108, 5, "巫医", "药熬好了，去星空之崖发起总攻吧！"},
        {SQ_MOVE, 408, 1, "内心独白", "（站在赤道星空下，准备决战）"},
        {SQ_KILL, 38, 2, "内心独白", "（击杀2个象牙盗猎大枭）"},
        {SQ_TALK, 246, 1, "大酋长", "祖先的土地夺回了！这是部落的神器！"}
    }, 60000, 800, 201},

    {12, "【史诗】血钻的诅咒", 240, 7, {
        {SQ_TALK, 248, 1, "酋长地窖", "一颗被诅咒的血钻流落到了达喀尔，带来无尽的灾厄。"},
        {SQ_MOVE, 360, 1, "内心独白", "（你抵达达喀尔）"},
        {SQ_MOVE, 367, 1, "内心独白", "（进入塞内加尔毒林）"},
        {SQ_KILL, 31, 5, "内心独白", "（击杀5个巫毒血魂）"},
        {SQ_TALK, 369, 1, "巫毒祭祀", "你要洗清钻石的诅咒，必须去斑马大草原用鲜血祭奠。"},
        {SQ_MOVE, 264, 1, "内心独白", "（回到开普敦的斑马大草原）"},
        {SQ_TALK, 248, 1, "酋长地窖", "钻石终于失去了邪气，可以作为财宝了。"}
    }, 50000, 700, 12},

    {13, "【史诗】草原霸主的陨落", 240, 5, {
        {SQ_TALK, 242, 1, "猎手营地", "斑马大草原上出现了一头变异的狮王，咬死了我们好几个兄弟。"},
        {SQ_MOVE, 264, 1, "内心独白", "（你踏入草原，草丛中传来低沉的咆哮...）"},
        {SQ_KILL, 8, 6, "营地猎手", "（先清理外围，击杀6只丛林饿狼！）"},
        {SQ_KILL, 28, 2, "营地猎手", "（终于激怒了正主！击杀2头草原黄金狮王）"},
        {SQ_TALK, 245, 1, "巫医大帐", "你是真正的勇士！这瓶狂暴药剂敬你！"}
    }, 30000, 450, 104},

    {14, "【中型】猎杀鬣狗", 240, 3, { {SQ_TALK, 242, 1, "营地猎手", "外面的斑马大草原鬣狗泛滥。"}, {SQ_KILL, 8, 4, "内心独白", "（击杀4只丛林饿狼代指鬣狗）"}, {SQ_TALK, 242, 1, "营地猎手", "干得好。"} }, 8000, 80, 0},
    {15, "【中型】巫医的草药", 240, 2, { {SQ_TALK, 245, 1, "巫医", "我需要3朵凝神花来做仪式。"}, {SQ_COLLECT, 109, 3, "巫医", "图腾会感谢你。"} }, 10000, 100, 0},
    {16, "【中型】护送象牙", 240, 3, { {SQ_TALK, 254, 1, "集市老板", "把这批象牙送到亚历山大。"}, {SQ_MOVE, 280, 1, "内心独白", "（抵达亚历山大）"}, {SQ_TALK, 280, 1, "商人", "象牙收到了，给你尾款。"} }, 22000, 200, 0},
    {17, "【中型】金字塔防线", 280, 3, { {SQ_TALK, 280, 1, "守卫", "尼罗河三角洲有盗墓贼。"}, {SQ_KILL, 5, 5, "内心独白", "（击杀5个地精代指小偷）"}, {SQ_TALK, 280, 1, "守卫", "法老安息了。"} }, 9000, 90, 0},
    {18, "【中型】突尼斯除蝎", 320, 3, { {SQ_TALK, 320, 1, "市民", "撒哈拉边缘蝎子扎人。"}, {SQ_KILL, 75, 4, "内心独白", "（击杀4只巨蝎）"}, {SQ_TALK, 320, 1, "市民", "安全了。"} }, 11000, 110, 0},
    {19, "【中型】达喀尔的毒蛇", 360, 3, { {SQ_TALK, 360, 1, "市民", "塞内加尔毒林蛇太多。"}, {SQ_KILL, 36, 3, "内心独白", "（击杀3条眼镜王蛇）"}, {SQ_TALK, 360, 1, "市民", "这下能去采药了。"} }, 13000, 130, 0},
    {20, "【中型】索法拉保卫战", 400, 3, { {SQ_TALK, 400, 1, "总督", "荒原上的犀牛失控了。"}, {SQ_KILL, 26, 4, "内心独白", "（击杀4头长颈鹿代指猛兽）"}, {SQ_TALK, 400, 1, "总督", "赏金拿好。"} }, 14000, 140, 0},
    {21, "【中型】蒙巴萨的寒冷", 440, 3, { {SQ_TALK, 440, 1, "酋长", "乞力马扎罗的雪兽伤人。"}, {SQ_KILL, 79, 3, "内心独白", "（击杀3个雪女代指雪兽）"}, {SQ_TALK, 440, 1, "酋长", "谢谢勇士。"} }, 15000, 150, 0},

    // ---------------- 印度/中东区域 ----------------
    {22, "【史诗】阿修罗的复苏", 480, 8, {
        {SQ_TALK, 485, 1, "湿婆神庙祭司", "阿修罗古战场的封印松动了，邪气外泄！"},
        {SQ_MOVE, 511, 1, "内心独白", "（你踏入阴风阵阵的阿修罗古战场）"},
        {SQ_KILL, 33, 8, "内心独白", "（击杀8个苦行僧邪影）"},
        {SQ_TALK, 499, 1, "书院学者", "需要用深海的龙涎香才能重新封印它。"},
        {SQ_COLLECT, 242, 1, "书院学者", "准备好了？去阿修罗地宫吧！"},
        {SQ_MOVE, 514, 1, "内心独白", "（你深入了恐怖的地宫）"},
        {SQ_KILL, 35, 1, "内心独白", "（击杀千手阿修罗像！）"},
        {SQ_TALK, 485, 1, "湿婆神庙祭司", "印度的天空重新恢复了清明。"}
    }, 75000, 1500, 241},

    {23, "【史诗】莫卧儿的秘宝", 480, 7, {
        {SQ_TALK, 486, 1, "土邦王公", "我祖辈的秘宝被偷到了锡兰。"},
        {SQ_MOVE, 680, 1, "内心独白", "（你乘船抵达锡兰）"},
        {SQ_KILL, 77, 6, "内心独白", "（击杀6名波斯弯刀刺客夺回地图）"},
        {SQ_TALK, 689, 1, "蓝宝石私矿", "秘宝其实藏在孟买的德干荒原。"},
        {SQ_MOVE, 647, 1, "内心独白", "（抵达孟买德干荒原）"},
        {SQ_KILL, 38, 4, "内心独白", "（击杀4名盗猎大枭抢回宝箱）"},
        {SQ_TALK, 486, 1, "土邦王公", "太感谢了，这套盔甲送你！"}
    }, 58000, 800, 17},

    {24, "【史诗】香料航线的霸权", 480, 6, {
        {SQ_TALK, 494, 1, "大巴扎老板", "葡萄牙人的舰队封锁了亚丁，去教训他们！"},
        {SQ_MOVE, 600, 1, "内心独白", "（驶向红海咽喉亚丁）"},
        {SQ_MOVE, 607, 1, "内心独白", "（进入红海焦岩带）"},
        {SQ_KILL, 48, 4, "内心独白", "（击杀4名叛将代指敌军司令）"},
        {SQ_COLLECT, 252, 5, "大巴扎老板", "给我5块精铁矿石修复战船。"},
        {SQ_TALK, 494, 1, "大巴扎老板", "航线终于被打通了！"}
    }, 45000, 600, 103}, 

    {25, "【中型】清理猛虎", 480, 3, { {SQ_TALK, 490, 1, "巡卫", "猛虎密林的老虎下山了。"}, {SQ_KILL, 34, 5, "内心独白", "（击杀5只孟加拉虎）"}, {SQ_TALK, 490, 1, "巡卫", "百姓安全了。"} }, 11000, 100, 0},
    {26, "【中型】收集神圣草药", 480, 2, { {SQ_TALK, 493, 1, "药师", "给我带 5 朵凝神花来。"}, {SQ_COLLECT, 109, 5, "药师", "这药效极佳。"} }, 10000, 80, 0},
    {27, "【中型】惩治恶霸", 480, 3, { {SQ_TALK, 487, 1, "贫民", "暗巷里的恶霸欺压我们。"}, {SQ_KILL, 0, 8, "内心独白", "（击杀8个流氓）"}, {SQ_TALK, 487, 1, "贫民", "你真是菩萨下凡。"} }, 15000, 180, 0},
    {28, "【中型】海峡走私者", 520, 3, { {SQ_TALK, 520, 1, "总督", "博斯普鲁斯海峡有走私者。"}, {SQ_KILL, 7, 5, "内心独白", "（击杀5名海盗）"}, {SQ_TALK, 520, 1, "总督", "干得漂亮。"} }, 12000, 120, 0},
    {29, "【中型】沙暴元素之乱", 560, 3, { {SQ_TALK, 560, 1, "学者", "波斯大漠的沙暴元素暴走了。"}, {SQ_KILL, 78, 4, "内心独白", "（击杀4只沙暴元素）"}, {SQ_TALK, 560, 1, "学者", "风沙停歇了。"} }, 14000, 140, 0},
    {30, "【中型】红海清剿", 600, 3, { {SQ_TALK, 600, 1, "商人", "红海海盗抢了我的货。"}, {SQ_KILL, 7, 6, "内心独白", "（击杀6名海盗）"}, {SQ_TALK, 600, 1, "商人", "大恩不言谢。"} }, 16000, 160, 0},
    {31, "【中型】贫民窟镇暴", 640, 3, { {SQ_TALK, 640, 1, "长官", "德干荒原流寇作乱。"}, {SQ_KILL, 13, 5, "内心独白", "（击杀5名强盗）"}, {SQ_TALK, 640, 1, "长官", "终于安静了。"} }, 13000, 130, 0},
    {32, "【中型】宝石大盗", 680, 3, { {SQ_TALK, 680, 1, "矿主", "有人在雨林里抢宝石。"}, {SQ_KILL, 77, 4, "内心独白", "（击杀4名刺客）"}, {SQ_TALK, 680, 1, "矿主", "矿区保住了。"} }, 15000, 150, 0},

    // ---------------- 亚洲区域 ----------------
    {33, "【史诗】大明海魂", 720, 13, {
        {SQ_TALK, 720, 1, "戚家军老卒", "年轻人，倭寇近日在清源山麓集结，你可愿替大明探探虚实？"},
        {SQ_MOVE, 746, 1, "内心独白", "（你来到了清源山麓，四周弥漫着肃杀之气...）"},
        {SQ_KILL, 42, 5, "戚家军老卒", "干得好！杀他们个片甲不留！(需击杀5名倭寇大名)"},
        {SQ_TALK, 723, 1, "神机营教头", "倭寇的甲胄太厚，我需要一些上好的铁矿来打造破甲铳。"},
        {SQ_COLLECT, 252, 3, "神机营教头", "去野外探索，给我带回 3 块【精铁矿石】(打怪或搜集)。"},
        {SQ_TALK, 726, 1, "市舶使大人", "本官听闻了你的勇武。拿着这封信，去澳门找弗朗机商人弄些火炮。"},
        {SQ_MOVE, 840, 1, "内心独白", "（历经长途航行，你抵达了澳门濠镜澳...）"},
        {SQ_TALK, 849, 1, "佛郎机商人", "火炮可以给你，但你要替我去妈阁庙后山清理一下盘踞的海盗！"},
        {SQ_MOVE, 847, 1, "内心独白", "（你来到了妈阁庙后山，海盗们正在分赃。）"},
        {SQ_KILL, 44, 4, "佛郎机商人", "（击杀4名天字杀手）去死吧，贪婪的海盗！"},
        {SQ_TALK, 849, 1, "佛郎机商人", "干得漂亮！火炮装船了，回泉州复命吧。"},
        {SQ_MOVE, 720, 1, "内心独白", "（满载火炮的战舰顺利驶回了泉州港...）"},
        {SQ_TALK, 726, 1, "市舶使大人", "大明水师如虎添翼！这块天外陨铁赏给你了！"}
    }, 50000, 800, 208},

    {34, "【史诗】寻找建文帝", 720, 9, {
        {SQ_TALK, 730, 1, "锦衣卫百户", "朝廷密令，追查建文帝踪迹，去长崎打听打听。"},
        {SQ_MOVE, 760, 1, "内心独白", "（你抵达了日本长崎）"},
        {SQ_KILL, 42, 6, "浪人", "（去富士山脉击杀6名倭寇大名获取情报）"},
        {SQ_TALK, 769, 1, "甲贺忍者", "有人在汉城看到了留着长须的中原和尚。"},
        {SQ_MOVE, 800, 1, "内心独白", "（你马不停蹄赶到汉城）"},
        {SQ_COLLECT, 103, 3, "高丽参农", "给我3瓶超强生命药水，我告诉你那和尚去哪了。"},
        {SQ_MOVE, 808, 1, "内心独白", "（来到天池绝顶，空无一人，只留下一件袈裟）"},
        {SQ_KILL, 79, 4, "内心独白", "（突然杀出4名长白山雪女）"},
        {SQ_TALK, 730, 1, "锦衣卫百户", "他果然还在世...这个消息绝不能泄露，赏你了。"}
    }, 65000, 900, 11},

    {35, "【史诗】南洋降头术", 720, 8, {
        {SQ_TALK, 728, 1, "大通钱庄", "我的南洋船队中了邪，快去马六甲找巫师解咒！"},
        {SQ_MOVE, 880, 1, "内心独白", "（你航行至马六甲）"},
        {SQ_MOVE, 887, 1, "内心独白", "（深入闷热橡胶雨林）"},
        {SQ_KILL, 8, 6, "内心独白", "（击杀6只丛林饿狼）"},
        {SQ_TALK, 889, 1, "南洋香料大王", "想要解药？去雅加达猎杀巨蜥拿毒腺。"},
        {SQ_MOVE, 927, 1, "内心独白", "（登陆科莫多巨蜥岛）"},
        {SQ_KILL, 9, 8, "内心独白", "（击杀8只泥潭怪逼出巨蜥）"},
        {SQ_TALK, 728, 1, "大通钱庄", "伙计们得救了！这是你的重赏！"}
    }, 48000, 650, 303},

    {36, "【中型】清剿倭寇", 720, 3, { {SQ_TALK, 726, 1, "市舶使", "倭寇泛滥，去泉州湾暗礁区清理一下。"}, {SQ_KILL, 42, 5, "内心独白", "（击杀5名倭寇）"}, {SQ_TALK, 726, 1, "市舶使", "大明海疆清平。"} }, 15000, 200, 0},
    {37, "【中型】收集精铁", 720, 2, { {SQ_TALK, 723, 1, "神机营工匠", "急需5块精铁矿石打造火炮。"}, {SQ_COLLECT, 252, 5, "神机营工匠", "多谢了！"} }, 12000, 150, 0},
    {38, "【中型】护送密函", 720, 3, { {SQ_TALK, 730, 1, "锦衣卫", "把密函送到长崎的甲贺忍者手里。"}, {SQ_MOVE, 760, 1, "内心独白", "（抵达长崎）"}, {SQ_TALK, 769, 1, "甲贺忍者", "信我收到了。"} }, 20000, 300, 0},
    {39, "【中型】长崎野武士", 760, 3, { {SQ_TALK, 760, 1, "町长", "富士山脚有野武士拦路。"}, {SQ_KILL, 42, 4, "内心独白", "（击杀4名武士）"}, {SQ_TALK, 760, 1, "町长", "道路通畅了。"} }, 13000, 130, 0},
    {40, "【中型】汉城驱虎", 800, 3, { {SQ_TALK, 800, 1, "府尹", "长白山猛虎伤人。"}, {SQ_KILL, 34, 4, "内心独白", "（击杀4只猛虎）"}, {SQ_TALK, 800, 1, "府尹", "百姓安居乐业。"} }, 14000, 140, 0},
    {41, "【中型】妈阁庙香火", 840, 3, { {SQ_TALK, 840, 1, "庙祝", "后山的海盗抢了香火钱。"}, {SQ_KILL, 44, 4, "内心独白", "（击杀4名杀手）"}, {SQ_TALK, 840, 1, "庙祝", "妈祖保佑你。"} }, 12000, 150, 0},
    {42, "【中型】马六甲毒虫", 880, 3, { {SQ_TALK, 880, 1, "商人", "橡胶林的毒蛇太多了。"}, {SQ_KILL, 36, 5, "内心独白", "（击杀5条毒蛇）"}, {SQ_TALK, 880, 1, "商人", "这下能割胶了。"} }, 11000, 110, 0},
    {43, "【中型】雅加达巨蜥", 920, 3, { {SQ_TALK, 920, 1, "土著", "巨蜥跑到村里来了！"}, {SQ_KILL, 9, 5, "内心独白", "（击杀5只怪物）"}, {SQ_TALK, 920, 1, "土著", "感谢神明。"} }, 15000, 160, 0},

    // ---------------- 美洲区域 ----------------
    {44, "【史诗】海盗王的遗产", 960, 11, {
        {SQ_TALK, 962, 1, "黑帆大副", "船长临死前留下一张图纸，指向百慕大，但我需要人手。"},
        {SQ_MOVE, 985, 1, "内心独白", "（你潜入了海盗王藏宝海湾，这里遍地是亡灵...）"},
        {SQ_KILL, 49, 5, "黑帆大副", "（击杀5名骷髅船长）夺回被他们霸占的航海罗盘残骸！"},
        {SQ_TALK, 964, 1, "巫毒女巫", "咯咯咯...这罗盘被诅咒了。给我一些止血草，我用来配置洗灵液。"},
        {SQ_COLLECT, 108, 4, "巫毒女巫", "去野外搜集，给我带来 4 株【止血草】。"},
        {SQ_TALK, 964, 1, "巫毒女巫", "草药足够了。咯咯...现在罗盘指向了百慕大的深处..."},
        {SQ_MOVE, 991, 1, "内心独白", "（你驶入了百慕大边缘迷雾，罗盘发出耀眼的金光！）"},
        {SQ_KILL, 56, 1, "内心独白", "（在迷雾深处遭遇了传说中的幽灵提督！）"},
        {SQ_TALK, 962, 1, "黑帆大副", "不可思议，你竟然活着带回了财宝！"},
        {SQ_TALK, 970, 1, "西班牙警署", "不许动！海盗！把宝物交出来！"},
        {SQ_TALK, 962, 1, "黑帆大副", "我们冲出了海军的包围网！这是说好的分成！"}
    }, 60000, 700, 220},

    {45, "【史诗】阿兹特克的血祭", 960, 7, {
        {SQ_TALK, 965, 1, "大教堂神父", "维拉克鲁斯的异教徒正在进行邪恶的血祭！去阻止他们！"},
        {SQ_MOVE, 1080, 1, "内心独白", "（你赶到了维拉克鲁斯）"},
        {SQ_MOVE, 1087, 1, "内心独白", "（爬上太阳金字塔阶梯）"},
        {SQ_KILL, 50, 6, "内心独白", "（击杀6名阿兹特克血祭司）"},
        {SQ_COLLECT, 251, 5, "内心独白", "（破坏祭坛需要5块铜矿石）"},
        {SQ_KILL, 82, 2, "内心独白", "（击杀2只太阳神守卫彻底粉碎神像！）"},
        {SQ_TALK, 965, 1, "大教堂神父", "你拯救了无数无辜的灵魂，这是圣光的赐福。"}
    }, 55000, 850, 26},

    {46, "【史诗】加勒比幽灵舰队", 960, 6, {
        {SQ_TALK, 966, 1, "总督", "最近幽灵船频频袭击商船，去调查圣多明各海岸。"},
        {SQ_MOVE, 1160, 1, "内心独白", "（抵达圣多明各）"},
        {SQ_MOVE, 1167, 1, "内心独白", "（来到哥伦布海岸）"},
        {SQ_KILL, 49, 8, "内心独白", "（击杀8只骷髅船长）"},
        {SQ_COLLECT, 103, 3, "总督", "用3瓶超强生命药水净化被诅咒的船首像。"},
        {SQ_TALK, 966, 1, "总督", "干得好，加勒比海恢复了安宁。"}
    }, 50000, 600, 23},

    {47, "【中型】鲨鱼危机", 960, 3, { {SQ_TALK, 960, 1, "码头长", "外海全吃吃人鲨！"}, {SQ_KILL, 66, 4, "内心独白", "（击杀4只鲨鱼怪）"}, {SQ_TALK, 960, 1, "码头长", "终于能出海了。"} }, 13000, 120, 0},
    {48, "【中型】镇压暴动", 960, 3, { {SQ_TALK, 970, 1, "警署", "甘蔗林的奴隶暴动了！"}, {SQ_KILL, 13, 6, "内心独白", "（击杀6名暴徒）"}, {SQ_TALK, 970, 1, "警署", "秩序恢复了。"} }, 15000, 150, 0},
    {49, "【中型】火枪图纸", 960, 3, { {SQ_TALK, 963, 1, "火绳枪铺", "把这份图纸送到波士顿。"}, {SQ_MOVE, 1040, 1, "内心独白", "（抵达波士顿）"}, {SQ_TALK, 1040, 1, "军需官", "图纸收到了。"} }, 18000, 180, 0},
    {50, "【中型】里约雨林探险", 1000, 3, { {SQ_TALK, 1000, 1, "探险家", "亚马逊边缘有毒蛙。"}, {SQ_KILL, 83, 4, "内心独白", "（击杀4只毒物）"}, {SQ_TALK, 1000, 1, "探险家", "雨林安全多了。"} }, 12000, 120, 0},
    {51, "【中型】阿巴拉契亚剿匪", 1040, 3, { {SQ_TALK, 1040, 1, "镇长", "山里全是印第安强盗。"}, {SQ_KILL, 13, 5, "内心独白", "（击杀5名强盗）"}, {SQ_TALK, 1040, 1, "镇长", "上帝保佑你。"} }, 14000, 140, 0},
    {52, "【中型】维拉克鲁斯之战", 1080, 3, { {SQ_TALK, 1080, 1, "神父", "异教徒在神庙聚集。"}, {SQ_KILL, 50, 4, "内心独白", "（击杀4名祭司）"}, {SQ_TALK, 1080, 1, "神父", "圣光必胜。"} }, 15000, 150, 0},
    {53, "【中型】白银大盗", 1120, 3, { {SQ_TALK, 1120, 1, "军官", "有人抢了白银骡队。"}, {SQ_KILL, 7, 5, "内心独白", "（击杀5名海盗）"}, {SQ_TALK, 1120, 1, "军官", "白银保住了。"} }, 16000, 160, 0},
    {54, "【中型】钟楼异响", 1160, 3, { {SQ_TALK, 1160, 1, "市民", "钟楼上有蝙蝠怪物。"}, {SQ_KILL, 14, 4, "内心独白", "（击杀4群吸血蝙蝠）"}, {SQ_TALK, 1160, 1, "市民", "钟声终于清脆了。"} }, 11000, 110, 0},

    // ---------------- 大洋洲区域 ----------------
    {55, "【史诗】大洋洲的淘金狂潮", 1200, 7, {
        {SQ_TALK, 1206, 1, "悉尼总督", "去墨尔本的矿脉调查金矿失窃案。"},
        {SQ_MOVE, 1240, 1, "内心独白", "（航行至墨尔本）"},
        {SQ_MOVE, 1247, 1, "内心独白", "（进入大洋路破碎海岸）"},
        {SQ_KILL, 62, 6, "内心独白", "（击杀6名淘金疯灵）"},
        {SQ_TALK, 1249, 1, "丛林大盗", "黄金被运往了塔斯马尼亚。"},
        {SQ_MOVE, 1287, 1, "内心独白", "（前往霍巴特的恶魔巢穴）"},
        {SQ_TALK, 1206, 1, "悉尼总督", "金库填满了，干得好。"}
    }, 60000, 800, 211},

    {56, "【史诗】史前巨兽的苏醒", 1200, 6, {
        {SQ_TALK, 1202, 1, "淘金者", "大堡礁附近出现了巨大怪物，砸沉了好多船！"},
        {SQ_MOVE, 1231, 1, "内心独白", "（来到食人鳄河口）"},
        {SQ_KILL, 64, 2, "内心独白", "（极其艰难地击杀2头霸王鳄！）"},
        {SQ_COLLECT, 108, 8, "淘金者", "我受伤了，给我8株止血草救命！"},
        {SQ_KILL, 61, 5, "内心独白", "（继续击杀5只大堡礁毒水母清理海域）"},
        {SQ_TALK, 1202, 1, "淘金者", "这辈子不想再出海了，这把枪给你。"}
    }, 58000, 750, 15},

    {57, "【史诗】流放地大暴动", 1200, 6, {
        {SQ_TALK, 1210, 1, "皇家骑警", "犯人隔离营暴动了，快去镇压！"},
        {SQ_MOVE, 1207, 1, "内心独白", "（冲入隔离营）"},
        {SQ_KILL, 58, 8, "内心独白", "（击杀8名暴动狱霸）"},
        {SQ_TALK, 1210, 1, "皇家骑警", "他们逃往了蓝山深谷，追！"},
        {SQ_MOVE, 1225, 1, "内心独白", "（深入蓝山雾林）"},
        {SQ_TALK, 1210, 1, "皇家骑警", "首恶伏诛，悉尼保住了。"}
    }, 45000, 600, 13},

    {58, "【中型】清理毒蜘蛛", 1200, 3, { {SQ_TALK, 1200, 1, "市民", "红土中心的蜘蛛泛滥。"}, {SQ_KILL, 5, 6, "内心独白", "（击杀6只蜘蛛怪）"}, {SQ_TALK, 1200, 1, "市民", "终于不用担惊受怕了。"} }, 12000, 120, 0},
    {59, "【中型】收集桉树叶", 1200, 2, { {SQ_TALK, 1213, 1, "草药商", "去弄 5 株止血草(代指桉树叶)。"}, {SQ_COLLECT, 108, 5, "草药商", "这叶子很纯正。"} }, 9000, 90, 0},
    {60, "【中型】镇压死斗场", 1200, 3, { {SQ_TALK, 1217, 1, "看守", "死斗场的人疯了。"}, {SQ_KILL, 11, 4, "内心独白", "（击杀4名暴徒）"}, {SQ_TALK, 1217, 1, "看守", "规矩就是规矩。"} }, 15000, 150, 0},
    {61, "【中型】大洋路危机", 1240, 3, { {SQ_TALK, 1240, 1, "矿工", "大洋路有强盗拦路。"}, {SQ_KILL, 13, 5, "内心独白", "（击杀5名强盗）"}, {SQ_TALK, 1240, 1, "矿工", "金子保住了。"} }, 14000, 140, 0},
    {62, "【中型】恶魔的咆哮", 1280, 3, { {SQ_TALK, 1280, 1, "捕鲸人", "塔斯马尼亚恶魔咬死了人。"}, {SQ_KILL, 59, 4, "内心独白", "（击杀4只恶魔）"}, {SQ_TALK, 1280, 1, "捕鲸人", "上帝保佑。"} }, 16000, 160, 0},

    // ---------------- 神话/深海区域 ----------------
    {63, "【史诗】沉没的神域", 1320, 9, {
        {SQ_TALK, 1324, 1, "观星人", "星象大变！古老的亚特兰蒂斯封印松动了，去石像群看看！"},
        {SQ_MOVE, 1348, 1, "内心独白", "（波利尼西亚石像似乎在哭泣...怪物涌现！）"},
        {SQ_KILL, 67, 6, "观星人", "（击杀6名波利尼西亚像鬼）神圣的碎片掉落了。"},
        {SQ_TALK, 1333, 1, "草药师", "想进入地心世界，你需要【凝神花】来抵御深渊的低语。"},
        {SQ_COLLECT, 109, 5, "草药师", "去野外找 5 朵【凝神花】给我。"},
        {SQ_TALK, 1324, 1, "观星人", "一切准备就绪，快去火神祭坛！"},
        {SQ_MOVE, 1351, 1, "内心独白", "（火山喷发，岩浆拦住了你的去路！）"},
        {SQ_KILL, 70, 4, "观星人", "（击杀4名岩浆怪）通道打开了，那是通往地心的巨门！"},
        {SQ_TALK, 1324, 1, "观星人", "你见证了神话！去解开最后的秘密吧！"}
    }, 75000, 1000, 246},

    {64, "【史诗】火神的愤怒", 1320, 5, {
        {SQ_TALK, 1326, 1, "大酋长", "佩蕾女神在发怒，熔岩洞的怪物冲出来了！"},
        {SQ_MOVE, 1347, 1, "内心独白", "（前往莫纳罗亚熔岩洞）"},
        {SQ_KILL, 70, 8, "内心独白", "（疯狂击杀8只岩浆怪）"},
        {SQ_COLLECT, 103, 2, "大酋长", "战士们被烧伤，需要2瓶超强生命药水！"},
        {SQ_TALK, 1326, 1, "大酋长", "火山终于平息了，这是海神的鳞甲！"}
    }, 55000, 800, 32},

    {65, "【史诗】深海巨怪的复仇", 1320, 5, {
        {SQ_TALK, 1334, 1, "集市商人", "海沟里的巨怪爬上了岸，商船全毁了！"},
        {SQ_MOVE, 1349, 1, "内心独白", "（你靠近深海海沟边缘）"},
        {SQ_KILL, 52, 6, "内心独白", "（击杀6只深海大章鱼怪）"},
        {SQ_COLLECT, 252, 5, "集市商人", "快收集 5 块精铁矿石加固码头！"},
        {SQ_TALK, 1334, 1, "集市商人", "码头保住了，我们安全了。"}
    }, 50000, 750, 33},

    {66, "【中型】清理巨蟹", 1320, 3, { {SQ_TALK, 1320, 1, "岛民", "海滩上的螃蟹大得吃人。"}, {SQ_KILL, 65, 5, "内心独白", "（击杀5只巨蟹）"}, {SQ_TALK, 1320, 1, "岛民", "今晚吃蟹肉大餐！"} }, 14000, 140, 0},
    {67, "【中型】采集深海海藻", 1320, 2, { {SQ_TALK, 1333, 1, "草药师", "给我带 5 朵凝神花来。"}, {SQ_COLLECT, 109, 5, "草药师", "很好。"} }, 10000, 100, 0},
    {68, "【中型】镇抚海盗", 1320, 3, { {SQ_TALK, 1330, 1, "护卫", "海盗在林子里闹事。"}, {SQ_KILL, 7, 5, "内心独白", "（击杀5名海盗）"}, {SQ_TALK, 1330, 1, "护卫", "干得漂亮。"} }, 15000, 150, 0},
    {69, "【中型】海沟探秘", 1360, 3, { {SQ_TALK, 1360, 1, "关岛土著", "海沟里爬出了恶鬼。"}, {SQ_KILL, 85, 4, "内心独白", "（击杀4只鳐鱼怪）"}, {SQ_TALK, 1360, 1, "关岛土著", "海沟恢复了平静。"} }, 18000, 180, 0},
};
const int story_count = sizeof(story_list) / sizeof(StoryDef);


// ==========================================
// 辅助与状态函数
// ==========================================

static int get_item_count_in_bag(int item_id) {
    int cnt = 0;
    for (int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        if (zh_player.inventory[i] == item_id) cnt++;
    }
    return cnt;
}

static bool remove_item_from_bag(int item_id, int count) {
    if (get_item_count_in_bag(item_id) < count) return false;
    
    int removed = 0;
    for (int i = 0; i < MAX_INVENTORY_SIZE && removed < count; i++) {
        if (zh_player.inventory[i] == item_id) { 
            zh_player.inventory[i] = -1; 
            removed++; 
        }
    }
    return true;
}

lv_obj_t* list_q_story = NULL;
lv_obj_t* modal_q_detail = NULL;
lv_obj_t* lbl_q_info = NULL;

int cur_view_id = -1;
int cur_view_type = 0; // 0: 剧情, 1: 悬赏, 2: 日常委托

void reset_quest_ui_pointers() {
    list_q_story = NULL;
    modal_q_detail = NULL;
    lbl_q_info = NULL;
}


// ==========================================
// UI 文本格式化
// ==========================================

static void format_story_quest_detail(char* buf, size_t buf_size) {
    if (cur_view_id >= story_count) return;
    const StoryDef& sd = story_list[cur_view_id];
    
    if (zh_player.story_status[cur_view_id] == 0) {
        snprintf(buf, buf_size, "%s\n\n阶段数目: %d 步\n【事成奖励】\n%d 铜贝\n%d 声望\n额外物品: %s", 
            sd.name, sd.step_count, sd.r_gold, sd.r_rep, 
            sd.r_item > 0 ? get_item_by_id(sd.r_item).name : "无");
    } 
    else if (zh_player.story_status[cur_view_id] == 1) { 
        int step = zh_player.story_progress[cur_view_id];
        if (step >= sd.step_count) return; 
        
        const StoryStep& ss = sd.steps[step];
        int cur_val = 0;
        const char* obj_type_str = "";

        switch (ss.type) {
            case SQ_KILL:    cur_val = zh_player.story_counter[cur_view_id]; obj_type_str = "前往野外猎杀指定怪物"; break;
            case SQ_COLLECT: cur_val = get_item_count_in_bag(ss.target_id);  obj_type_str = "去野外搜集或包里备好物品"; break;
            case SQ_TALK:    obj_type_str = "前往指定地点对话"; break;
            case SQ_MOVE:    obj_type_str = "移动到指定区域"; break;
        }
        
        snprintf(buf, buf_size, "【%s】 (第%d步)\n\n[%s]:\n \"%s\"\n\n目标: %s\n进度: %d / %d", 
            sd.name, step + 1, ss.npc_name, ss.dialog, obj_type_str, cur_val, ss.count);
    } else {
        snprintf(buf, buf_size, "该剧情已完结。");
    }
}

static void format_bounty_detail(char* buf, size_t buf_size) {
    if (cur_view_id >= bounty_count) return;
    const BountyDef& bd = bounty_list[cur_view_id];
    
    const char* loc_name = "未知秘境";
    for (int i = 0; i < zh_data_locations_count; i++) {
        if (zh_data_locations[i].id == bd.loc_id) {
            loc_name = zh_data_locations[i].name;
            break;
        }
    }
    
    snprintf(buf, buf_size, "%s\n\n出没地：【%s】\n情报：%s\n\n【赏金】\n%d 铜贝, %d 声望",
        bd.target_name, loc_name, bd.desc, bd.gold_reward, bd.rep_reward);
}

static void format_daily_quest_detail(char* buf, size_t buf_size) {
    int req_cnt = zh_player.quest_progress & 0xFFFF;
    int cur_cnt = (zh_player.quest_progress >> 16) & 0xFFFF;
    
    if (zh_player.quest_id == 1) {
        const char* m_name = (zh_player.quest_target >= 0 && zh_player.quest_target < zh_data_monsters_count) 
            ? zh_data_monsters[zh_player.quest_target].name : "未知怪物";
        snprintf(buf, buf_size, "【市民委托 - 讨伐】\n\n请前往野外，保护城镇。\n目标: 猎杀【%s】\n进度: %d / %d 只", 
            m_name, cur_cnt, req_cnt);
    } 
    else if (zh_player.quest_id == 2) {
        int cur_have = get_item_count_in_bag(zh_player.quest_target);
        const char* item_name = get_item_by_id(zh_player.quest_target).name;
        snprintf(buf, buf_size, "【市民委托 - 搜集】\n\n请前往野外探索，搜集指定的物资。\n目标: 搜集【%s】\n完成后去找任何市民交付。\n进度: %d / %d 份", 
            item_name, cur_have, req_cnt);
    } 
    else if (zh_player.quest_id == 3) {
        const char* t_name = "未知区域";
        for (int j = 0; j < zh_data_locations_count; j++) {
            if (zh_data_locations[j].id == zh_player.quest_target) t_name = zh_data_locations[j].name;
        }
        snprintf(buf, buf_size, "【市民委托 - 送货】\n\n请将这个沉重的包裹送到指定地点。\n目标: 前往【%s】\n到达后找NPC递交包裹。\n报酬很丰厚！", t_name);
    }
}

static void refresh_quest_detail() {
    if (cur_view_id < 0) return;
    static char buf[512];
    
    if (cur_view_type == 0) {
        format_story_quest_detail(buf, sizeof(buf));
    } else if (cur_view_type == 1) {
        format_bounty_detail(buf, sizeof(buf));
    } else if (cur_view_type == 2) {
        format_daily_quest_detail(buf, sizeof(buf));
    }
    
    lv_label_set_text(lbl_q_info, buf);
}


// ==========================================
// 任务操作与推进逻辑
// ==========================================

static void try_advance_story(int story_id) {
    if (zh_player.story_status[story_id] == 0) {
        zh_player.story_status[story_id] = 1; 
        zh_player.story_progress[story_id] = 0; 
        zh_player.story_counter[story_id] = 0;
        zh_log("剧情已接取！跟随指引行动吧。");
        return;
    }

    int step = zh_player.story_progress[story_id];
    const StoryDef& sd = story_list[story_id];
    
    if (step >= sd.step_count) {
        zh_player.story_status[story_id] = 2;
        return;
    }
    
    const StoryStep& ss = sd.steps[step];
    bool is_final_step = (step == sd.step_count - 1);
    
    // 检查背包空间 (仅最后一步发奖励时)
    if (is_final_step && sd.r_item > 0) {
        int empty_slots = 0;
        for (int k = 0; k < MAX_INVENTORY_SIZE; k++) {
            if (zh_player.inventory[k] == -1) empty_slots++;
        }
        int expected_empty = empty_slots + (ss.type == SQ_COLLECT ? ss.count : 0);
        
        if (expected_empty <= 0) {
            zh_log("推进失败：背包已满！请先清理背包以接收任务奖励。");
            return;
        }
    }

    bool pass = false;
    switch (ss.type) {
        case SQ_TALK:
            pass = (zh_player.location_id == ss.target_id);
            if (!pass) zh_log("交付失败：你必须亲自前往指定地点！");
            break;
        case SQ_MOVE:
            pass = (zh_player.location_id == ss.target_id);
            if (!pass) zh_log("推进失败：请移动到剧情指定的区域。");
            break;
        case SQ_KILL:
            pass = (zh_player.story_counter[story_id] >= ss.count);
            if (!pass) zh_log("推进失败：击杀数量不足，去野外找找看。");
            break;
        case SQ_COLLECT:
            pass = remove_item_from_bag(ss.target_id, ss.count);
            if (!pass) zh_log("交付失败：背包中没有足够的指定物品！");
            break;
    }
    
    if (pass) {
        zh_player.story_progress[story_id]++;
        zh_player.story_counter[story_id] = 0;
        
        if (zh_player.story_progress[story_id] >= sd.step_count) {
            zh_player.story_status[story_id] = 2;
            zh_player.gold += sd.r_gold;
            zh_player.reputation += sd.r_rep;
            if (sd.r_item > 0) add_item_to_bag(sd.r_item);
            
            static char w[128];
            snprintf(w, sizeof(w), "【剧情完结】\n获得 %d 铜贝，声望暴涨！", sd.r_gold);
            zh_log(w);
        } else {
            zh_log("【阶段达成】\n剧情已推进到下一阶段！");
        }
    }
}

static void handle_quest_action(lv_event_t *e) {
    if (cur_view_type == 0) {
        try_advance_story(cur_view_id);
    } else if (cur_view_type == 2) {
        zh_log("请通过与城内任意平民/商人对话，或直接前往指定地点交付委托！");
    } else {
        zh_log("通缉令无法在此放弃，请直接前往指定地点击杀目标！");
    }
    
    lv_obj_add_flag(modal_q_detail, LV_OBJ_FLAG_HIDDEN);
    lv_async_call([](void*){ refresh_quest_ui(); }, NULL);
    lv_async_call([](void*){ refresh_zongheng_ui(); }, NULL);
}


// ==========================================
// 构建 UI 与 刷新
// ==========================================

void build_quest_ui(lv_obj_t* screen, lv_obj_t* parent_tab) {
    lv_obj_set_style_bg_color(parent_tab, lv_color_hex(0x1a252f), 0);
    lv_obj_set_style_pad_all(parent_tab, 0, 0);

    list_q_story = lv_list_create(parent_tab);
    lv_obj_set_size(list_q_story, 240, 160);
    lv_obj_align(list_q_story, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(list_q_story, lv_color_hex(0x1a252f), 0);
    lv_obj_set_style_border_width(list_q_story, 0, 0);

    modal_q_detail = lv_obj_create(screen);
    lv_obj_set_size(modal_q_detail, 220, 290);
    lv_obj_center(modal_q_detail);
    lv_obj_add_flag(modal_q_detail, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(modal_q_detail, lv_color_hex(0x2c3e50), 0);

    lbl_q_info = lv_label_create(modal_q_detail);
    lv_obj_add_style(lbl_q_info, &style_cn, 0);
    lv_obj_set_width(lbl_q_info, 200);
    lv_obj_set_style_text_color(lbl_q_info, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_q_info, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // 操作按钮 (接取/推进)
    lv_obj_t* btn_do = lv_btn_create(modal_q_detail);
    lv_obj_set_size(btn_do, 90, 35);
    lv_obj_align(btn_do, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(btn_do, lv_color_hex(0x27ae60), 0);
    
    lv_obj_t* ld = lv_label_create(btn_do); 
    lv_obj_add_style(ld, &style_cn, 0); 
    lv_label_set_text(ld, "推进/接取"); 
    lv_obj_center(ld);
    lv_obj_add_event_cb(btn_do, handle_quest_action, LV_EVENT_CLICKED, NULL);

    // 返回按钮
    lv_obj_t* btn_c = lv_btn_create(modal_q_detail);
    lv_obj_set_size(btn_c, 90, 35);
    lv_obj_align(btn_c, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(btn_c, lv_color_hex(0x7f8c8d), 0);
    
    lv_obj_t* lc = lv_label_create(btn_c); 
    lv_obj_add_style(lc, &style_cn, 0); 
    lv_label_set_text(lc, "返回"); 
    lv_obj_center(lc);
    lv_obj_add_event_cb(btn_c, [](lv_event_t *e){ 
        lv_obj_add_flag(modal_q_detail, LV_OBJ_FLAG_HIDDEN); 
    }, LV_EVENT_CLICKED, NULL);
}

// 辅助函数：创建列表按钮并绑定事件
static void create_quest_list_btn(lv_obj_t* list, const char* symbol, const char* text, uint32_t color_hex, int view_type, int item_id) {
    lv_obj_t* b = lv_list_add_btn(list, symbol, text);
    lv_obj_set_style_bg_color(b, lv_color_hex(color_hex), 0);
    
    lv_obj_t * lbl = lv_obj_get_child(b, 1); 
    if (lbl) { 
        lv_obj_add_style(lbl, &style_cn, 0); 
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0); 
    }
    
    // 将 view_type 存入临时结构或通过闭包处理。这里我们仍然利用闭包改变全局状态。
    // 为了防止悬空，使用一个包含类型的结构体或者直接依靠捕获（LVGL C接口最好不用高级捕获，这里用全局+回调方式）：
    lv_obj_add_event_cb(b, [](lv_event_t *e) {
        // user_data 的高 16 位存 type，低 16 位存 id
        uint32_t data = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
        cur_view_type = (data >> 16) & 0xFFFF;
        cur_view_id   = data & 0xFFFF;
        
        refresh_quest_detail();
        lv_obj_clear_flag(modal_q_detail, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, (void*)(uintptr_t)((view_type << 16) | (item_id & 0xFFFF)));
}

// 辅助函数：创建小标题
static void create_quest_section_title(lv_obj_t* list, const char* text, uint32_t color_hex) {
    lv_obj_t* l = lv_label_create(list); 
    lv_obj_add_style(l, &style_cn, 0);
    lv_label_set_text(l, text); 
    lv_obj_set_style_text_color(l, lv_color_hex(color_hex), 0);
}

void refresh_quest_ui() {
    if (!list_q_story) return;
    lv_obj_clean(list_q_story);
    int current_city_base = (zh_player.location_id / 40) * 40;

    // 1. 日常委托
    if (zh_player.quest_id > 0) {
        create_quest_section_title(list_q_story, "* 进行中的市民委托", 0x2ecc71);
        const char* q_name = (zh_player.quest_id == 1) ? "讨伐野兽" : 
                             (zh_player.quest_id == 2) ? "搜集物资" : "加急送货";
        create_quest_list_btn(list_q_story, LV_SYMBOL_BELL, q_name, 0x16a085, 2, 0);
    }

    // 2. 进行中的剧情
    create_quest_section_title(list_q_story, "\n* 进行中的剧情", 0xF1C40F);
    for (int i = 0; i < story_count; i++) {
        if (zh_player.story_status[i] == 1) {
            create_quest_list_btn(list_q_story, LV_SYMBOL_PLAY, story_list[i].name, 0x2980b9, 0, i);
        }
    }

    // 3. 本地可接剧情
    create_quest_section_title(list_q_story, "\n* 本地可接剧情", 0xBDC3C7);
    for (int i = 0; i < story_count; i++) {
        if (zh_player.story_status[i] == 0 && story_list[i].start_loc == current_city_base) {
            create_quest_list_btn(list_q_story, LV_SYMBOL_FILE, story_list[i].name, 0x2c3e50, 0, i);
        }
    }

    // 4. 悬赏BOSS
    create_quest_section_title(list_q_story, "\n* 悬赏Boss", 0xE74C3C);
    if (zh_player.active_bounty_id >= 0 && zh_player.active_bounty_id < bounty_count) {
        create_quest_list_btn(list_q_story, LV_SYMBOL_WARNING, bounty_list[zh_player.active_bounty_id].target_name, 0x8B0000, 1, zh_player.active_bounty_id);
    }
}


// ==========================================
// 全局事件系统接入点
// ==========================================

void open_tavern_bounty_board() {
    if (zh_player.active_bounty_id != -1) {
        zh_log("酒馆老板：你身上已经有通缉令了，先把那家伙干掉再来揭榜！");
        return;
    }
    
    int r_bounty = rand() % bounty_count;
    zh_player.active_bounty_id = r_bounty;
    
    const char* loc_name = "未知秘境";
    for (int i = 0; i < zh_data_locations_count; i++) {
        if (zh_data_locations[i].id == bounty_list[r_bounty].loc_id) {
            loc_name = zh_data_locations[i].name;
            break;
        }
    }
    
    static char buf[256];
    snprintf(buf, sizeof(buf), "【揭榜成功】\n你接下了针对【%s】的悬赏！\n老板：去【%s】找他，极其危险！", 
        bounty_list[r_bounty].target_name, loc_name);
    zh_log(buf);
    
    lv_async_call([](void*){ refresh_quest_ui(); }, NULL);
}

int check_bounty_spawn(int loc_id) {
    if (zh_player.active_bounty_id >= 0 && zh_player.active_bounty_id < bounty_count) {
        if (bounty_list[zh_player.active_bounty_id].loc_id == loc_id) {
            return bounty_list[zh_player.active_bounty_id].monster_id;
        }
    }
    return -1;
}

void process_quest_kill(int monster_id) {
    // 1. 处理日常击杀委托
    if (zh_player.quest_id == 1 && zh_player.quest_target == monster_id) {
        int req_cnt = zh_player.quest_progress & 0xFFFF;
        int cur_cnt = (zh_player.quest_progress >> 16) & 0xFFFF;
        if (cur_cnt < req_cnt) { 
            cur_cnt++;
            zh_player.quest_progress = (cur_cnt << 16) | req_cnt;
            if (cur_cnt >= req_cnt) {
                zh_log("【委托达成】\n你杀够了怪物，快回城找市民交付吧！");
            }
        }
    }

    // 2. 处理进行中的剧情击杀任务
    for (int i = 0; i < story_count; i++) {
        if (zh_player.story_status[i] == 1) {
            int step = zh_player.story_progress[i];
            if (step < story_list[i].step_count) {
                const StoryStep& ss = story_list[i].steps[step];
                if (ss.type == SQ_KILL && ss.target_id == monster_id) {
                    zh_player.story_counter[i]++;
                    
                    if (zh_player.story_counter[i] == ss.count) {
                        static char q_msg[128];
                        snprintf(q_msg, sizeof(q_msg), "【剧情更新】\n《%s》阶段目标已击杀完毕！", story_list[i].name);
                        zh_log(q_msg);
                    }
                }
            }
        }
    }
    
    // 3. 处理悬赏击杀
    if (zh_player.active_bounty_id >= 0 && zh_player.active_bounty_id < bounty_count) {
        const BountyDef& bd = bounty_list[zh_player.active_bounty_id];
        
        if (bd.monster_id == monster_id) {
            zh_player.reputation += bd.rep_reward;
            zh_player.gold += bd.gold_reward;
            
            // 降低罪恶值
            if (zh_player.crime_value > 0) {
                zh_player.crime_value -= (bd.rep_reward / 2);
                if (zh_player.crime_value < 0) zh_player.crime_value = 0;
            }
            
            zh_player.active_bounty_id = -1; 
            
            static char buf[256];
            snprintf(buf, sizeof(buf), "【绝杀通缉犯！】\n你割下了首级！获得 %d 铜贝，声望(+%d)！", bd.gold_reward, bd.rep_reward);
            zh_log(buf);
            
            lv_async_call([](void*){ refresh_quest_ui(); }, NULL);
        }
    }
}