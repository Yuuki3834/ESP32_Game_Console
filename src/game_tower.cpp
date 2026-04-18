#include "global.h"
#include <esp_heap_caps.h>
#include <LittleFS.h>

#define MAP_SIZE 11
#define TILE_SIZE 21
#define MAX_FLOOR 51

enum TileType {
    T_EMPTY = 0, T_WALL = 1, T_HERO = 2,
    T_KEY_Y = 3, T_KEY_B = 4, T_KEY_R = 5,
    T_DOOR_Y = 6, T_DOOR_B = 7, T_DOOR_R = 8,
    T_POTION_R = 9, T_POTION_B = 10,
    T_GEM_R = 11, T_GEM_B = 12,
    T_STAIR_UP = 13, T_STAIR_DOWN = 14,
    T_SLIME_G = 15, T_SLIME_R = 16, T_BAT = 17, T_SLIME_B = 18,
    T_SKELETON = 19, T_MAGE = 20, T_ORC = 21, T_GUARD = 22,
    T_SWORD = 23, T_SHIELD = 24, T_SHOP = 25,
    T_NPC = 26, T_BOSS = 27, T_PRINCESS = 28
};

struct HeroStats {
    int hp, atk, def, gold, exp, keys_y, keys_b, keys_r, x, y;
};

const HeroStats init_hero = {1000, 10, 10, 0, 0, 1, 1, 1, 5, 10};

struct SaveData {
    HeroStats hero;
    int current_floor;
};

bool is_tower_started = false;

int (*map_data)[MAP_SIZE][MAP_SIZE] = NULL;

#define F_00 { {1,1,1,1,1,13,1,1,1,1,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,0,1,1,1,1,1,1,1,0,1}, {1,0,1,3,3,0,3,3,1,0,1}, {1,0,1,1,1,6,1,1,1,0,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,0,1,1,1,1,1,1,1,0,1}, {1,0,1,11,0,0,12,1,1,0,1}, {1,0,1,1,1,0,1,1,1,0,1}, {1,26,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,0,1,1,1,1,1} }
#define F_01 { {1,1,1,1,1,13,1,1,1,1,1}, {1,9,9,1,3,15,3,1,10,10,1}, {1,1,6,1,1,6,1,1,6,1,1}, {1,0,0,0,1,0,1,0,0,0,1}, {1,1,1,0,1,0,1,0,1,1,1}, {1,3,1,0,0,0,0,0,1,4,1}, {1,15,1,1,6,1,1,1,1,16,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,0,1,1,1,1,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_02 { {1,1,1,1,1,13,1,1,1,1,1}, {1,5,11,12,1,11,12,5,11,12,1}, {1,1,6,1,1,7,1,1,6,1,1}, {1,0,0,0,1,17,1,0,0,0,1}, {1,0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,0,0,0,0,1,0,1}, {1,0,1,1,1,6,1,1,1,0,1}, {1,0,0,0,0,15,0,0,0,0,1}, {1,1,1,1,1,0,1,1,1,1,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_03 { {1,1,1,1,1,13,1,1,1,1,1}, {1,23,3,1,19,0,19,1,3,24,1}, {1,15,15,1,6,1,6,1,15,15,1}, {1,1,6,1,0,1,0,1,6,1,1}, {1,9,0,0,0,25,0,0,0,9,1}, {1,0,1,1,0,1,0,1,1,0,1}, {1,4,16,1,18,18,0,1,16,4,1}, {1,1,6,1,1,1,0,1,6,1,1}, {1,0,0,0,0,1,0,0,0,0,1}, {1,0,1,1,0,0,0,1,1,0,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_04 { {1,1,1,1,1,13,1,1,1,1,1}, {1,9,11,9,1,22,1,10,12,10,1}, {1,1,8,1,1,0,1,1,8,1,1}, {1,0,0,0,1,0,1,0,0,0,1}, {1,0,1,1,1,7,1,1,1,0,1}, {1,0,1,5,21,0,21,4,1,0,1}, {1,0,1,1,1,1,1,1,1,0,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,6,1,1,1,1,1}, {1,3,15,16,17,18,17,16,15,3,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_05 { {1,1,1,1,1,13,1,1,1,1,1}, {1,11,0,0,1,22,1,0,0,12,1}, {1,1,6,1,1,0,1,1,6,1,1}, {1,3,0,20,0,0,0,21,0,3,1}, {1,1,1,1,1,6,1,1,1,1,1}, {1,0,0,0,0,25,0,0,0,0,1}, {1,6,1,1,1,1,1,1,1,6,1}, {1,0,1,9,15,16,17,10,1,0,1}, {1,0,1,1,1,1,1,1,1,0,1}, {1,3,0,0,0,0,0,0,0,3,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_06 { {1,1,1,1,1,13,1,1,1,1,1}, {1,3,15,1,0,0,0,1,16,3,1}, {1,1,6,1,0,1,0,1,6,1,1}, {1,9,0,1,18,1,17,1,0,10,1}, {1,1,1,1,0,1,0,1,1,1,1}, {1,0,0,7,0,0,0,6,0,0,1}, {1,1,1,1,1,1,1,1,1,0,1}, {1,11,0,19,0,0,0,20,0,0,1}, {1,1,6,1,1,1,1,1,1,1,1}, {1,4,0,0,0,0,0,0,0,3,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_07 { {1,1,1,1,1,13,1,1,1,1,1}, {1,0,0,0,0,21,1,12,1,11,1}, {1,1,1,1,1,0,1,6,1,6,1}, {1,3,0,16,1,0,1,0,0,0,1}, {1,1,6,1,1,0,1,1,1,1,1}, {1,0,0,0,1,0,0,0,0,3,1}, {1,0,1,1,1,1,1,1,6,1,1}, {1,0,1,9,15,1,17,0,0,0,1}, {1,0,1,1,1,1,1,1,1,0,1}, {1,3,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_08 { {1,1,1,1,1,13,1,1,1,1,1}, {1,11,1,3,1,20,1,3,1,12,1}, {1,6,1,6,1,0,1,6,1,6,1}, {1,0,1,15,1,0,1,15,1,0,1}, {1,0,1,1,1,6,1,1,1,0,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,0,1,1,1,1,1,1,1,0,1}, {1,16,1,9,0,0,0,10,1,17,1}, {1,6,1,1,1,0,1,1,1,6,1}, {1,3,0,0,0,19,0,0,0,3,1}, {1,1,1,1,1,14,1,1,1,1,1} }
#define F_BOSS { {1,1,1,1,1,1,1,1,1,1,1}, {1,0,0,0,0,28,0,0,0,0,1}, {1,0,1,1,1,8,1,1,1,0,1}, {1,0,1,0,0,0,0,0,1,0,1}, {1,0,1,0,22,27,22,0,1,0,1}, {1,0,1,0,0,0,0,0,1,0,1}, {1,0,1,1,1,6,1,1,1,0,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,0,1,1,1,1,1}, {1,0,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,14,1,1,1,1,1} }

void init_all_maps() {
    static const int STATIC_MAP_DATA[MAX_FLOOR][MAP_SIZE][MAP_SIZE] = {
        F_00, F_01, F_02, F_03, F_04, F_05, F_06, F_07, F_08, // 1~9层 (索引0~8)
        F_04, F_05, F_06, F_07, F_08,                         // 10~14层
        F_04, F_05, F_06, F_07, F_08,                         // 15~19层
        F_04, F_05, F_06, F_07, F_08,                         // 20~24层
        F_04, F_05, F_06, F_07, F_08,                         // 25~29层
        F_04, F_05, F_06, F_07, F_08,                         // 30~34层
        F_04, F_05, F_06, F_07, F_08,                         // 35~39层
        F_04, F_05, F_06, F_07, F_08,                         // 40~44层
        F_04, F_05, F_06, F_07, F_08,                         // 45~49层
        F_04,                                                 // 第50层 (索引49)
        F_BOSS                                                // 第51层决战 (索引50)
    };

    if (map_data == NULL) {
        map_data = (int (*)[MAP_SIZE][MAP_SIZE])heap_caps_malloc(
            MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int),
            MALLOC_CAP_SPIRAM
        );
        if (map_data == NULL) {
            map_data = (int (*)[MAP_SIZE][MAP_SIZE])malloc(
                MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int)
            );
        }
    }
    
    // 【关键修复】彻底清空内存，防止随机野数据污染引擎
    if (map_data != NULL) {
        memset(map_data, 0, MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int));
        memcpy(map_data, STATIC_MAP_DATA, sizeof(STATIC_MAP_DATA));
    }
}

HeroStats hero;
int current_floor = 0;

lv_obj_t * map_cont = NULL;
lv_obj_t * label_stats = NULL;
lv_obj_t * sys_modal = NULL;
lv_obj_t * shop_modal = NULL;
lv_obj_t * msg_modal = NULL;
lv_obj_t * lbl_msg_text = NULL;

struct Point {
    int x, y;
};

Point walk_path[121];
int path_len = 0, path_index = 0;
lv_timer_t * walk_timer = NULL;
const int dx[] = {0, 0, -1, 1}, dy[] = {-1, 1, 0, 0};

void stop_walking() {
    if (walk_timer) lv_timer_pause(walk_timer);
    path_index = path_len;
}

void update_hero_stats_ui() {
    if (!label_stats) return;
    lv_label_set_text_fmt(label_stats,
        "层:%d  金:%d  经:%d\n"
        "命:%d  攻:%d  防:%d\n"
        "钥: %d黄 %d蓝 %d红",
        current_floor, hero.gold, hero.exp,
        hero.hp, hero.atk, hero.def,
        hero.keys_y, hero.keys_b, hero.keys_r);
}

void change_floor(int next_floor, bool going_up) {
    if (next_floor < 0 || next_floor >= MAX_FLOOR) return;
    current_floor = next_floor;
    int target_stair = going_up ? T_STAIR_DOWN : T_STAIR_UP;
    bool found = false;
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (map_data[current_floor][y][x] == target_stair) {
                hero.x = x;
                hero.y = y;
                found = true;
                break;
            }
        }
        if (found) break;
    }
    update_hero_stats_ui();
    if (map_cont) lv_obj_invalidate(map_cont);
}

void show_message(const char* text) {
    stop_walking();
    if (!lbl_msg_text || !msg_modal) return;
    lv_label_set_text(lbl_msg_text, text);
    lv_obj_clear_flag(msg_modal, LV_OBJ_FLAG_HIDDEN);
}

struct EnemyDef {
    int hp, atk, def, gold, exp;
};

EnemyDef get_enemy_data(int tile_type, int floor) {
    EnemyDef base;
    switch (tile_type) {
        case T_SLIME_G: base = {80, 18, 1, 2, 1}; break;
        case T_SLIME_R: base = {100, 20, 3, 3, 2}; break;
        case T_BAT:     base = {100, 27, 5, 4, 2}; break;
        case T_SLIME_B: base = {200, 35, 10, 8, 5}; break;
        case T_SKELETON:base = {220, 45, 12, 11, 10}; break;
        case T_MAGE:    base = {115, 53, 27, 10, 7}; break;
        case T_ORC:     base = {300, 165, 80, 35, 30}; break;
        case T_GUARD:   base = {450, 180, 120, 43, 40}; break;
        case T_BOSS:    base = {9999, 1000, 500, 1000, 1000}; break;
        default: return {0, 0, 0, 0, 0};
    }
    if (tile_type == T_BOSS) {
        base.hp += floor * 200;
        base.atk += floor * 15;
        base.def += floor * 10;
    } else {
        base.hp += floor * 15;
        base.atk += floor * 3;
        base.def += floor * 1;
        base.gold += floor * 2;
    }
    return base;
}

int get_damage(int tile_type) {
    EnemyDef e = get_enemy_data(tile_type, current_floor);
    if (hero.atk <= e.def) return -1;

    int hero_damage = hero.atk - e.def;
    int turns = (e.hp + hero_damage - 1) / hero_damage;

    int enemy_damage = e.atk - hero.def;
    if (enemy_damage < 0) enemy_damage = 0;

    int total_damage = (turns - 1) * enemy_damage;
    if (total_damage >= hero.hp) return -1;

    return total_damage;
}

void walk_timer_cb(lv_timer_t * timer) {
    if (path_index >= path_len) {
        lv_timer_pause(timer);
        return;
    }
    int nx = walk_path[path_index].x, ny = walk_path[path_index].y;
    int target_tile = map_data[current_floor][ny][nx];
    bool can_move = false;

    if (target_tile == T_EMPTY) {
        can_move = true;
    } else if (target_tile == T_KEY_Y) {
        hero.keys_y++; can_move = true;
    } else if (target_tile == T_KEY_B) {
        hero.keys_b++; can_move = true;
    } else if (target_tile == T_KEY_R) {
        hero.keys_r++; can_move = true;
    } else if (target_tile == T_POTION_R) {
        hero.hp += 200; can_move = true;
    } else if (target_tile == T_POTION_B) {
        hero.hp += 500; can_move = true;
    } else if (target_tile == T_GEM_R) {
        hero.atk += 3; can_move = true;
    } else if (target_tile == T_GEM_B) {
        hero.def += 3; can_move = true;
    } else if (target_tile == T_SWORD) {
        hero.atk += 10; can_move = true;
    } else if (target_tile == T_SHIELD) {
        hero.def += 10; can_move = true;
    } else if (target_tile == T_DOOR_Y) {
        if (hero.keys_y > 0) {
            hero.keys_y--; can_move = true;
        } else {
            stop_walking(); return;
        }
    } else if (target_tile == T_DOOR_B) {
        if (hero.keys_b > 0) {
            hero.keys_b--; can_move = true;
        } else {
            stop_walking(); return;
        }
    } else if (target_tile == T_DOOR_R) {
        if (hero.keys_r > 0) {
            hero.keys_r--; can_move = true;
        } else {
            stop_walking(); return;
        }
    } else if (target_tile == T_STAIR_UP) {
        stop_walking(); change_floor(current_floor + 1, true); return;
    } else if (target_tile == T_STAIR_DOWN) {
        stop_walking(); change_floor(current_floor - 1, false); return;
    } else if (target_tile == T_SHOP) {
        stop_walking();
        if (shop_modal) lv_obj_clear_flag(shop_modal, LV_OBJ_FLAG_HIDDEN);
        return;
    } else if (target_tile == T_NPC) {
        show_message("【神秘老人】\n欢迎来到纪元魔塔！\n第 51 层有魔王守护着公主，\n努力向上攀登吧，勇士！");
        can_move = true; return;
    } else if (target_tile == T_PRINCESS) {
        show_message("【公主】\n勇士！你终于来救我了！\n太感谢你了，我们一起回王国吧！\n\n- 纪元魔塔 完美通关！ -");
        can_move = true;
    } else if ((target_tile >= T_SLIME_G && target_tile <= T_GUARD) || target_tile == T_BOSS) {
        int dmg = get_damage(target_tile);
        if (dmg == -1) {
            stop_walking(); return;
        } else {
            hero.hp -= dmg;
            EnemyDef e = get_enemy_data(target_tile, current_floor);
            hero.gold += e.gold;
            hero.exp += e.exp;
            can_move = true;
            if (target_tile == T_BOSS) {
                show_message("【魔王 ZENO】\n不...这不可能！\n我竟然会被一个人类打败！\n啊啊啊啊啊啊啊！");
            }
        }
    }

    if (can_move) {
        map_data[current_floor][ny][nx] = T_EMPTY;
        hero.x = nx;
        hero.y = ny;
        path_index++;
        if (map_cont) lv_obj_invalidate(map_cont);
        update_hero_stats_ui();
    } else {
        stop_walking();
    }
}

bool find_path(int sx, int sy, int ex, int ey) {
    struct Node {
        int x, y, px, py;
    };
    Node q[128];
    int head = 0, tail = 0;
    bool visited[MAP_SIZE][MAP_SIZE] = {false};
    Point parent[MAP_SIZE][MAP_SIZE];

    q[tail++] = {sx, sy, -1, -1};
    visited[sy][sx] = true;
    bool found = false;

    while (head < tail) {
        Node curr = q[head++];
        if (curr.x == ex && curr.y == ey) {
            found = true;
            break;
        }
        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i], ny = curr.y + dy[i];
            if (nx >= 0 && nx < MAP_SIZE && ny >= 0 && ny < MAP_SIZE && !visited[ny][nx]) {
                if (map_data[current_floor][ny][nx] == T_EMPTY || (nx == ex && ny == ey)) {
                    visited[ny][nx] = true;
                    parent[ny][nx] = {curr.x, curr.y};
                    q[tail++] = {nx, ny, curr.x, curr.y};
                }
            }
        }
    }
    if (!found) return false;
    path_len = 0;
    int cx = ex, cy = ey;
    while (cx != sx || cy != sy) {
        walk_path[path_len++] = {cx, cy};
        Point p = parent[cy][cx];
        cx = p.x;
        cy = p.y;
    }
    for (int i = 0; i < path_len / 2; i++) {
        Point temp = walk_path[i];
        walk_path[i] = walk_path[path_len - 1 - i];
        walk_path[path_len - 1 - i] = temp;
    }
    return true;
}

static void map_draw_event_cb(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);
    lv_area_t obj_coords;
    lv_obj_get_coords(obj, &obj_coords);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.radius = 2;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_color_hex(0xFFFFFF);
    label_dsc.font = &my_font_cn_16;

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            int tile = map_data[current_floor][y][x];
            if (x == hero.x && y == hero.y) tile = T_HERO;
            const char * tile_text = "";

            switch (tile) {
                case T_EMPTY:      continue;
                case T_WALL:       rect_dsc.bg_color = lv_color_hex(0x555555); break;
                case T_HERO:       rect_dsc.bg_color = lv_color_hex(0x0088FF); tile_text = "勇"; break;
                case T_KEY_Y:      rect_dsc.bg_color = lv_color_hex(0xDDDD00); tile_text = "黄"; break;
                case T_KEY_B:      rect_dsc.bg_color = lv_color_hex(0x0000DD); tile_text = "蓝"; break;
                case T_KEY_R:      rect_dsc.bg_color = lv_color_hex(0xDD0000); tile_text = "红"; break;
                case T_DOOR_Y:     rect_dsc.bg_color = lv_color_hex(0xDDAA00); tile_text = "门"; break;
                case T_DOOR_B:     rect_dsc.bg_color = lv_color_hex(0x0000DD); tile_text = "门"; break;
                case T_DOOR_R:     rect_dsc.bg_color = lv_color_hex(0xDD0000); tile_text = "门"; break;
                case T_POTION_R:   rect_dsc.bg_color = lv_color_hex(0xDD0000); tile_text = "药"; break;
                case T_POTION_B:   rect_dsc.bg_color = lv_color_hex(0x0000DD); tile_text = "药"; break;
                case T_GEM_R:      rect_dsc.bg_color = lv_color_hex(0xDD0000); tile_text = "攻"; break;
                case T_GEM_B:      rect_dsc.bg_color = lv_color_hex(0x0000DD); tile_text = "防"; break;
                case T_SWORD:      rect_dsc.bg_color = lv_color_hex(0x00AAAA); tile_text = "剑"; break;
                case T_SHIELD:     rect_dsc.bg_color = lv_color_hex(0xAAAAAA); tile_text = "盾"; break;
                case T_SHOP:       rect_dsc.bg_color = lv_color_hex(0xAA00AA); tile_text = "商"; break;
                case T_STAIR_UP:   rect_dsc.bg_color = lv_color_hex(0x884400); tile_text = "上"; break;
                case T_STAIR_DOWN: rect_dsc.bg_color = lv_color_hex(0x884400); tile_text = "下"; break;
                case T_NPC:        rect_dsc.bg_color = lv_color_hex(0x55AA55); tile_text = "老"; break;
                case T_BOSS:       rect_dsc.bg_color = lv_color_hex(0x000000); tile_text = "王"; break;
                case T_PRINCESS:   rect_dsc.bg_color = lv_color_hex(0xFF00FF); tile_text = "主"; break;
                case T_SLIME_G:    rect_dsc.bg_color = lv_color_hex(0x00AA00); tile_text = "史"; break;
                case T_SLIME_R:    rect_dsc.bg_color = lv_color_hex(0xAA0000); tile_text = "史"; break;
                case T_SLIME_B:    rect_dsc.bg_color = lv_color_hex(0x222222); tile_text = "史"; break;
                case T_BAT:        rect_dsc.bg_color = lv_color_hex(0x880088); tile_text = "蝠"; break;
                case T_SKELETON:   rect_dsc.bg_color = lv_color_hex(0xAAAAAA); tile_text = "骷"; break;
                case T_MAGE:       rect_dsc.bg_color = lv_color_hex(0xAA00AA); tile_text = "法"; break;
                case T_ORC:        rect_dsc.bg_color = lv_color_hex(0xAA5500); tile_text = "兽"; break;
                case T_GUARD:      rect_dsc.bg_color = lv_color_hex(0xDDDD00); tile_text = "卫"; break;
                default:           rect_dsc.bg_color = lv_color_hex(0x333333); break;
            }

            lv_area_t tile_area;
            tile_area.x1 = obj_coords.x1 + x * TILE_SIZE;
            tile_area.y1 = obj_coords.y1 + y * TILE_SIZE;
            tile_area.x2 = tile_area.x1 + TILE_SIZE - 2;
            tile_area.y2 = tile_area.y1 + TILE_SIZE - 2;
            lv_draw_rect(draw_ctx, &rect_dsc, &tile_area);

            if (strlen(tile_text) > 0) {
                lv_point_t txt_size;
                lv_txt_get_size(&txt_size, tile_text, label_dsc.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
                lv_area_t txt_area;
                txt_area.x1 = tile_area.x1 + ((TILE_SIZE - 2) - txt_size.x) / 2;
                txt_area.y1 = tile_area.y1 + ((TILE_SIZE - 2) - txt_size.y) / 2;
                txt_area.x2 = txt_area.x1 + txt_size.x;
                txt_area.y2 = txt_area.y1 + txt_size.y;
                lv_draw_label(draw_ctx, &label_dsc, &txt_area, tile_text, NULL);
            }
        }
    }
}

static void map_click_event_cb(lv_event_t * e) {
    lv_indev_t * indev = lv_indev_get_act();
    if (indev == NULL) return;

    lv_point_t p;
    lv_indev_get_point(indev, &p);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);

    int target_x = (p.x - coords.x1) / TILE_SIZE;
    int target_y = (p.y - coords.y1) / TILE_SIZE;

    if (find_path(hero.x, hero.y, target_x, target_y)) {
        path_index = 0;
        if (walk_timer) lv_timer_resume(walk_timer);
    }
}

bool has_tower_save() {
    return LittleFS.exists("/tower.sav");
}

void save_tower_game() {
    if (map_data == NULL) return;

    File file = LittleFS.open("/tower.sav", FILE_WRITE);
    if (!file) {
        Serial.println("存档文件创建失败");
        return;
    }

    SaveData save = {hero, current_floor};
    file.write((uint8_t*)&save, sizeof(save));
    file.write((uint8_t*)map_data, MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int));
    file.close();

    Serial.println("✓ 游戏已存入 LittleFS");
    show_message("【系统】\n当前游戏进度已成功存档！");
}

bool load_tower_game() {
    if (!LittleFS.exists("/tower.sav")) return false;

    File file = LittleFS.open("/tower.sav", FILE_READ);
    if (!file) return false;

    size_t expected_size = sizeof(SaveData) + MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int);
    if (file.size() != expected_size) {
        file.close();
        return false;
    }

    if (map_data == NULL) {
        map_data = (int (*)[MAP_SIZE][MAP_SIZE])heap_caps_malloc(
            MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int), MALLOC_CAP_SPIRAM);
        if (map_data == NULL) {
            map_data = (int (*)[MAP_SIZE][MAP_SIZE])malloc(
                MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int));
        }
    }

    // 【关键修复】确保堆空间分配后强制清零，防崩溃
    if (map_data != NULL) {
        memset(map_data, 0, MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int));
    }

    SaveData save;
    file.read((uint8_t*)&save, sizeof(save));
    file.read((uint8_t*)map_data, MAX_FLOOR * MAP_SIZE * MAP_SIZE * sizeof(int));
    file.close();

    hero = save.hero;
    current_floor = save.current_floor;

    is_tower_started = true;
    refresh_tower_ui();
    return true;
}

bool buy_item(int cost, int &stat, int gain) {
    if (hero.gold >= cost) {
        hero.gold -= cost;
        stat += gain;
        update_hero_stats_ui();
        return true;
    }
    return false;
}

void refresh_tower_ui() {
    stop_walking();
    update_hero_stats_ui();
    if (map_cont) lv_obj_invalidate(map_cont);
}

void reset_tower_game() {
    hero = init_hero;
    current_floor = 0;
    init_all_maps();
    is_tower_started = true;
    refresh_tower_ui();
}

void build_tower_scene() {
    if (scr_tower != NULL) {
        lv_obj_del_async(scr_tower);
        scr_tower = NULL;
        map_cont = NULL;
        label_stats = NULL;
        sys_modal = NULL;
        shop_modal = NULL;
        msg_modal = NULL;
        lbl_msg_text = NULL;
    }

    if (walk_timer != NULL) {
        lv_timer_del(walk_timer);
        walk_timer = NULL;
    }

    scr_tower = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_tower, lv_color_hex(0x000000), 0);

    walk_timer = lv_timer_create(walk_timer_cb, 80, NULL);
    lv_timer_pause(walk_timer);

    map_cont = lv_obj_create(scr_tower);
    lv_obj_set_size(map_cont, MAP_SIZE * TILE_SIZE, MAP_SIZE * TILE_SIZE);
    lv_obj_align(map_cont, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_opa(map_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(map_cont, 0, 0);
    lv_obj_clear_flag(map_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(map_cont, map_draw_event_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(map_cont, map_click_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * panel_stats = lv_obj_create(scr_tower);
    lv_obj_set_size(panel_stats, 230, 78);
    lv_obj_align(panel_stats, LV_ALIGN_BOTTOM_MID, 0, -3);
    lv_obj_set_style_bg_color(panel_stats, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_color(panel_stats, lv_color_hex(0x555555), 0);
    lv_obj_set_style_pad_all(panel_stats, 8, 0);
    lv_obj_clear_flag(panel_stats, LV_OBJ_FLAG_SCROLLABLE);

    label_stats = lv_label_create(panel_stats);
    lv_obj_add_style(label_stats, &style_cn, 0);
    lv_obj_set_style_text_color(label_stats, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_stats, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * btn_sys = lv_btn_create(panel_stats);
    lv_obj_set_size(btn_sys, 45, 45);
    lv_obj_align(btn_sys, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_sys, lv_color_hex(0x444444), 0);
    lv_obj_t * label_sys = lv_label_create(btn_sys);
    lv_obj_add_style(label_sys, &style_cn, 0);
    lv_label_set_text(label_sys, "系统");
    lv_obj_center(label_sys);
    lv_obj_add_event_cb(btn_sys, [](lv_event_t *e) {
        if (sys_modal) lv_obj_clear_flag(sys_modal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    sys_modal = lv_obj_create(scr_tower);
    lv_obj_set_size(sys_modal, 160, 215);
    lv_obj_center(sys_modal);
    lv_obj_set_style_bg_color(sys_modal, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(sys_modal, 240, 0);
    lv_obj_set_style_border_width(sys_modal, 2, 0);
    lv_obj_set_style_border_color(sys_modal, lv_color_hex(0x666666), 0);
    lv_obj_set_style_shadow_width(sys_modal, 30, 0);
    lv_obj_set_style_shadow_color(sys_modal, lv_color_hex(0x000000), 0);
    lv_obj_add_flag(sys_modal, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * btn_resume = lv_btn_create(sys_modal);
    lv_obj_set_size(btn_resume, 120, 35);
    lv_obj_align(btn_resume, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_t * lbl_resume = lv_label_create(btn_resume);
    lv_obj_add_style(lbl_resume, &style_cn, 0);
    lv_label_set_text(lbl_resume, "继续游戏");
    lv_obj_center(lbl_resume);
    lv_obj_add_event_cb(btn_resume, [](lv_event_t *e) {
        if (sys_modal) lv_obj_add_flag(sys_modal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_save = lv_btn_create(sys_modal);
    lv_obj_set_size(btn_save, 120, 35);
    lv_obj_align(btn_save, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_bg_color(btn_save, lv_color_hex(0x0055AA), 0);
    lv_obj_t * lbl_save = lv_label_create(btn_save);
    lv_obj_add_style(lbl_save, &style_cn, 0);
    lv_label_set_text(lbl_save, "保存游戏");
    lv_obj_center(lbl_save);
    lv_obj_add_event_cb(btn_save, [](lv_event_t *e) {
        if (sys_modal) lv_obj_add_flag(sys_modal, LV_OBJ_FLAG_HIDDEN);
        save_tower_game();
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_restart = lv_btn_create(sys_modal);
    lv_obj_set_size(btn_restart, 120, 35);
    lv_obj_align(btn_restart, LV_ALIGN_TOP_MID, 0, 105);
    lv_obj_set_style_bg_color(btn_restart, lv_color_hex(0xAA0000), 0);
    lv_obj_t * lbl_restart = lv_label_create(btn_restart);
    lv_obj_add_style(lbl_restart, &style_cn, 0);
    lv_label_set_text(lbl_restart, "重新开始");
    lv_obj_center(lbl_restart);
    lv_obj_add_event_cb(btn_restart, [](lv_event_t *e) {
        reset_tower_game();
        if (sys_modal) lv_obj_add_flag(sys_modal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_exit = lv_btn_create(sys_modal);
    lv_obj_set_size(btn_exit, 120, 35);
    lv_obj_align(btn_exit, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_t * lbl_exit = lv_label_create(btn_exit);
    lv_obj_add_style(lbl_exit, &style_cn, 0);
    lv_label_set_text(lbl_exit, "返回主页");
    lv_obj_center(lbl_exit);
    lv_obj_add_event_cb(btn_exit, [](lv_event_t *e) {
        if (sys_modal) lv_obj_add_flag(sys_modal, LV_OBJ_FLAG_HIDDEN);
        // 【修改】直接删除 walk_timer 并置为 NULL
        if (walk_timer != NULL) {
            lv_timer_del(walk_timer);
            walk_timer = NULL;
        }
        // 保存游戏并清理内存
        save_tower_game();
        if (map_data != NULL) {
            free(map_data);
            map_data = NULL;
        }
        lv_scr_load(scr_menu);
        lv_obj_del_async(scr_tower);
        scr_tower = NULL;
    }, LV_EVENT_CLICKED, NULL);

    msg_modal = lv_obj_create(scr_tower);
    lv_obj_set_size(msg_modal, 220, 160);
    lv_obj_align(msg_modal, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_bg_color(msg_modal, lv_color_hex(0x001122), 0);
    lv_obj_set_style_bg_opa(msg_modal, 245, 0);
    lv_obj_set_style_border_color(msg_modal, lv_color_hex(0x8888FF), 0);
    lv_obj_set_style_shadow_width(msg_modal, 30, 0);
    lv_obj_set_style_shadow_color(msg_modal, lv_color_hex(0x000000), 0);
    lv_obj_add_flag(msg_modal, LV_OBJ_FLAG_HIDDEN);

    lbl_msg_text = lv_label_create(msg_modal);
    lv_obj_add_style(lbl_msg_text, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_msg_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_msg_text, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t * btn_msg_ok = lv_btn_create(msg_modal);
    lv_obj_set_size(btn_msg_ok, 80, 35);
    lv_obj_align(btn_msg_ok, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_t * lbl_msg_ok = lv_label_create(btn_msg_ok);
    lv_obj_add_style(lbl_msg_ok, &style_cn, 0);
    lv_label_set_text(lbl_msg_ok, "确认");
    lv_obj_center(lbl_msg_ok);
    lv_obj_add_event_cb(btn_msg_ok, [](lv_event_t *e) {
        if (msg_modal) lv_obj_add_flag(msg_modal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    shop_modal = lv_obj_create(scr_tower);
    lv_obj_set_size(shop_modal, 200, 240);
    lv_obj_center(shop_modal);
    lv_obj_set_style_bg_color(shop_modal, lv_color_hex(0x221133), 0);
    lv_obj_set_style_bg_opa(shop_modal, 245, 0);
    lv_obj_set_style_border_color(shop_modal, lv_color_hex(0xAA00AA), 0);
    lv_obj_set_style_shadow_width(shop_modal, 30, 0);
    lv_obj_set_style_shadow_color(shop_modal, lv_color_hex(0x000000), 0);
    lv_obj_add_flag(shop_modal, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * lbl_shop_title = lv_label_create(shop_modal);
    lv_obj_add_style(lbl_shop_title, &style_cn, 0);
    lv_label_set_text(lbl_shop_title, "贪婪商店\n给我20金币，换取力量！");
    lv_obj_set_style_text_color(lbl_shop_title, lv_color_hex(0xFFFF00), 0);
    lv_obj_align(lbl_shop_title, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_t * btn_buy_hp = lv_btn_create(shop_modal);
    lv_obj_set_size(btn_buy_hp, 160, 35);
    lv_obj_align(btn_buy_hp, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_t * lbl_buy_hp = lv_label_create(btn_buy_hp);
    lv_obj_add_style(lbl_buy_hp, &style_cn, 0);
    lv_label_set_text(lbl_buy_hp, "生命 +800");
    lv_obj_center(lbl_buy_hp);
    lv_obj_add_event_cb(btn_buy_hp, [](lv_event_t *e) {
        buy_item(20, hero.hp, 800);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_buy_atk = lv_btn_create(shop_modal);
    lv_obj_set_size(btn_buy_atk, 160, 35);
    lv_obj_align(btn_buy_atk, LV_ALIGN_TOP_MID, 0, 95);
    lv_obj_t * lbl_buy_atk = lv_label_create(btn_buy_atk);
    lv_obj_add_style(lbl_buy_atk, &style_cn, 0);
    lv_label_set_text(lbl_buy_atk, "攻击 +4");
    lv_obj_center(lbl_buy_atk);
    lv_obj_add_event_cb(btn_buy_atk, [](lv_event_t *e) {
        buy_item(20, hero.atk, 4);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_buy_def = lv_btn_create(shop_modal);
    lv_obj_set_size(btn_buy_def, 160, 35);
    lv_obj_align(btn_buy_def, LV_ALIGN_TOP_MID, 0, 140);
    lv_obj_t * lbl_buy_def = lv_label_create(btn_buy_def);
    lv_obj_add_style(lbl_buy_def, &style_cn, 0);
    lv_label_set_text(lbl_buy_def, "防御 +4");
    lv_obj_center(lbl_buy_def);
    lv_obj_add_event_cb(btn_buy_def, [](lv_event_t *e) {
        buy_item(20, hero.def, 4);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_close_shop = lv_btn_create(shop_modal);
    lv_obj_set_size(btn_close_shop, 160, 35);
    lv_obj_align(btn_close_shop, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(btn_close_shop, lv_color_hex(0x555555), 0);
    lv_obj_t * lbl_close_shop = lv_label_create(btn_close_shop);
    lv_obj_add_style(lbl_close_shop, &style_cn, 0);
    lv_label_set_text(lbl_close_shop, "离开");
    lv_obj_center(lbl_close_shop);
    lv_obj_add_event_cb(btn_close_shop, [](lv_event_t *e) {
        if (shop_modal) lv_obj_add_flag(shop_modal, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);
}