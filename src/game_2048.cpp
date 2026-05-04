#include "global.h"
#include <Preferences.h>

lv_obj_t *scr_2048 = NULL;
lv_obj_t *grid_2048 = NULL;
lv_obj_t *lbl_score = NULL;
lv_obj_t *lbl_best = NULL;
lv_obj_t *modal_2048_sys = NULL;
lv_obj_t *modal_2048_over = NULL;
lv_obj_t *lbl_2048_over_text = NULL;
lv_obj_t *modal_2048_msg = NULL;
lv_obj_t *lbl_2048_msg_content = NULL;

int board[4][4];
int score_2048 = 0;
int best_score_2048 = 0;
bool is_2048_started = false;

struct SaveData2048 {
    int board[4][4];
    int score;
    int best_score;
};

void show_2048_msg(const char* msg) {
    if (!modal_2048_msg || !lbl_2048_msg_content) return;
    lv_label_set_text(lbl_2048_msg_content, msg);
    lv_obj_clear_flag(modal_2048_msg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(modal_2048_msg);
}

void add_random_tile() {
    int empty[16][2];
    int count = 0;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (board[r][c] == 0) {
                empty[count][0] = r;
                empty[count][1] = c;
                count++;
            }
        }
    }
    if (count > 0) {
        int idx = rand() % count;
        board[empty[idx][0]][empty[idx][1]] = (rand() % 10 < 9) ? 2 : 4;
    }
}

bool check_game_over() {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (board[r][c] == 0) return false;
            if (r < 3 && board[r][c] == board[r+1][c]) return false;
            if (c < 3 && board[r][c] == board[r][c+1]) return false;
        }
    }
    return true;
}

bool slide_array(int arr[4]) {
    bool moved = false;
    int pos = 0;
    for (int i = 0; i < 4; i++) {
        if (arr[i] != 0) {
            if (i != pos) {
                arr[pos] = arr[i];
                arr[i] = 0;
                moved = true;
            }
            pos++;
        }
    }
    for (int i = 0; i < 3; i++) {
        if (arr[i] != 0 && arr[i] == arr[i+1]) {
            arr[i] *= 2;
            score_2048 += arr[i];
            arr[i+1] = 0;
            moved = true;
            for (int j = i + 1; j < 3; j++) {
                arr[j] = arr[j+1];
            }
            arr[3] = 0;
        }
    }
    return moved;
}

bool game_2048_move_internal(int dx, int dy) {
    bool moved = false;
    if (dx == -1) { 
        for (int r = 0; r < 4; r++) {
            if (slide_array(board[r])) moved = true;
        }
    } else if (dx == 1) { 
        for (int r = 0; r < 4; r++) {
            int temp[4] = {board[r][3], board[r][2], board[r][1], board[r][0]};
            if (slide_array(temp)) {
                board[r][3] = temp[0]; board[r][2] = temp[1]; board[r][1] = temp[2]; board[r][0] = temp[3];
                moved = true;
            }
        }
    } else if (dy == -1) { 
        for (int c = 0; c < 4; c++) {
            int temp[4] = {board[0][c], board[1][c], board[2][c], board[3][c]};
            if (slide_array(temp)) {
                board[0][c] = temp[0]; board[1][c] = temp[1]; board[2][c] = temp[2]; board[3][c] = temp[3];
                moved = true;
            }
        }
    } else if (dy == 1) { 
        for (int c = 0; c < 4; c++) {
            int temp[4] = {board[3][c], board[2][c], board[1][c], board[0][c]};
            if (slide_array(temp)) {
                board[3][c] = temp[0]; board[2][c] = temp[1]; board[1][c] = temp[2]; board[0][c] = temp[3];
                moved = true;
            }
        }
    }
    return moved;
}

void game_2048_move(int dx, int dy) {
    if (check_game_over()) return;
    if (game_2048_move_internal(dx, dy)) {
        add_random_tile();
        if (score_2048 > best_score_2048) best_score_2048 = score_2048;
        refresh_2048_ui();
        if (check_game_over()) {
            if (modal_2048_over) {
                lv_label_set_text_fmt(lbl_2048_over_text, "游戏结束!\n得分: %d", score_2048);
                lv_obj_clear_flag(modal_2048_over, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void grid_2048_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) game_2048_move(-1, 0);
        else if (dir == LV_DIR_RIGHT) game_2048_move(1, 0);
        else if (dir == LV_DIR_TOP) game_2048_move(0, -1);
        else if (dir == LV_DIR_BOTTOM) game_2048_move(0, 1);
    }
    else if (code == LV_EVENT_RELEASED) {
        // 释放事件 - 根据释放位置判断滑动方向
        lv_indev_t * indev = lv_indev_get_act();
        if (indev == NULL) return;
        
        lv_point_t p;
        lv_indev_get_point(indev, &p);
        
        // 获取grid_2048的坐标区域
        lv_area_t coords;
        lv_obj_get_coords(grid_2048, &coords);
        
        // 检查点击是否在grid区域内
        if (p.x >= coords.x1 && p.x <= coords.x2 && p.y >= coords.y1 && p.y <= coords.y2) {
            // 计算相对于grid中心的偏移
            int16_t dx = p.x - coords.x1;
            int16_t dy = p.y - coords.y1;
            int16_t grid_w = coords.x2 - coords.x1;
            int16_t grid_h = coords.y2 - coords.y1;
            int16_t center_x = grid_w / 2;
            int16_t center_y = grid_h / 2;
            
            // 优先判断方向偏移更大的轴
            if (abs(dx - center_x) > abs(dy - center_y)) {
                if (dx >= center_x) game_2048_move(1, 0);
                else game_2048_move(-1, 0);
            } else {
                if (dy >= center_y) game_2048_move(0, 1);
                else game_2048_move(0, -1);
            }
        }
    }
}

static void grid_2048_draw_cb(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);
    lv_area_t obj_coords;
    lv_obj_get_coords(obj, &obj_coords);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.radius = 6;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.font = &my_font_cn_16;

    int cell_size = 50;
    int gap = 6;
    int start_x = obj_coords.x1 + gap;
    int start_y = obj_coords.y1 + gap;

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = board[r][c];
            
            uint32_t bg_color = 0xcdc1b4;
            uint32_t text_color = 0x776e65;
            switch(val) {
                case 2: bg_color = 0xeee4da; break;
                case 4: bg_color = 0xede0c8; break;
                case 8: bg_color = 0xf2b179; text_color = 0xf9f6f2; break;
                case 16: bg_color = 0xf59563; text_color = 0xf9f6f2; break;
                case 32: bg_color = 0xf67c5f; text_color = 0xf9f6f2; break;
                case 64: bg_color = 0xf65e3b; text_color = 0xf9f6f2; break;
                case 128: bg_color = 0xedcf72; text_color = 0xf9f6f2; break;
                case 256: bg_color = 0xedcc61; text_color = 0xf9f6f2; break;
                case 512: bg_color = 0xedc850; text_color = 0xf9f6f2; break;
                case 1024: bg_color = 0xedc53f; text_color = 0xf9f6f2; break;
                case 2048: bg_color = 0xedc22e; text_color = 0xf9f6f2; break;
                default: 
                    if (val > 2048) { bg_color = 0x3c3a32; text_color = 0xf9f6f2; }
                    break;
            }

            rect_dsc.bg_color = lv_color_hex(bg_color);
            label_dsc.color = lv_color_hex(text_color);

            lv_area_t cell_area;
            cell_area.x1 = start_x + c * (cell_size + gap);
            cell_area.y1 = start_y + r * (cell_size + gap);
            cell_area.x2 = cell_area.x1 + cell_size - 1;
            cell_area.y2 = cell_area.y1 + cell_size - 1;

            lv_draw_rect(draw_ctx, &rect_dsc, &cell_area);

            if (val > 0) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", val);
                
                lv_point_t txt_size;
                lv_txt_get_size(&txt_size, buf, label_dsc.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
                
                lv_area_t txt_area;
                txt_area.x1 = cell_area.x1 + (cell_size - txt_size.x) / 2;
                txt_area.y1 = cell_area.y1 + (cell_size - txt_size.y) / 2;
                txt_area.x2 = txt_area.x1 + txt_size.x;
                txt_area.y2 = txt_area.y1 + txt_size.y;
                
                lv_draw_label(draw_ctx, &label_dsc, &txt_area, buf, NULL);
            }
        }
    }
}

void build_2048_scene() {
    if (scr_2048 != NULL) {
        lv_obj_del_async(scr_2048);
        scr_2048 = NULL;
    }

    scr_2048 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_2048, lv_color_hex(0xfaf8ef), 0);
    lv_obj_add_flag(scr_2048, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr_2048, [](lv_event_t * e) {
        grid_2048 = NULL;
        lbl_score = NULL;
        lbl_best = NULL;
        modal_2048_sys = NULL;
        modal_2048_over = NULL;
        lbl_2048_over_text = NULL;
        modal_2048_msg = NULL;
        lbl_2048_msg_content = NULL;
    }, LV_EVENT_DELETE, NULL);

    lv_obj_t * panel_top = lv_obj_create(scr_2048);
    lv_obj_set_size(panel_top, 230, 60);
    lv_obj_align(panel_top, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_color(panel_top, lv_color_hex(0xfaf8ef), 0);
    lv_obj_set_style_border_width(panel_top, 0, 0);
    lv_obj_clear_flag(panel_top, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(panel_top);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776e65), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * score_box = lv_obj_create(panel_top);
    lv_obj_set_size(score_box, 70, 45);
    lv_obj_align(score_box, LV_ALIGN_RIGHT_MID, -75, 0);
    lv_obj_set_style_bg_color(score_box, lv_color_hex(0xbbada0), 0);
    lv_obj_set_style_radius(score_box, 5, 0);
    lv_obj_set_style_border_width(score_box, 0, 0);
    lv_obj_clear_flag(score_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * lbl_score_title = lv_label_create(score_box);
    lv_obj_add_style(lbl_score_title, &style_cn, 0);
    lv_label_set_text(lbl_score_title, "分数");
    lv_obj_set_style_text_color(lbl_score_title, lv_color_hex(0xeee4da), 0);
    lv_obj_align(lbl_score_title, LV_ALIGN_TOP_MID, 0, -5);

    lbl_score = lv_label_create(score_box);
    lv_obj_add_style(lbl_score, &style_cn, 0);
    lv_label_set_text(lbl_score, "0");
    lv_obj_set_style_text_color(lbl_score, lv_color_hex(0xffffff), 0);
    lv_obj_align(lbl_score, LV_ALIGN_BOTTOM_MID, 0, 5);

    lv_obj_t * best_box = lv_obj_create(panel_top);
    lv_obj_set_size(best_box, 70, 45);
    lv_obj_align(best_box, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(best_box, lv_color_hex(0xbbada0), 0);
    lv_obj_set_style_radius(best_box, 5, 0);
    lv_obj_set_style_border_width(best_box, 0, 0);
    lv_obj_clear_flag(best_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * lbl_best_title = lv_label_create(best_box);
    lv_obj_add_style(lbl_best_title, &style_cn, 0);
    lv_label_set_text(lbl_best_title, "最高分");
    lv_obj_set_style_text_color(lbl_best_title, lv_color_hex(0xeee4da), 0);
    lv_obj_align(lbl_best_title, LV_ALIGN_TOP_MID, 0, -5);

    lbl_best = lv_label_create(best_box);
    lv_obj_add_style(lbl_best, &style_cn, 0);
    lv_label_set_text(lbl_best, "0");
    lv_obj_set_style_text_color(lbl_best, lv_color_hex(0xffffff), 0);
    lv_obj_align(lbl_best, LV_ALIGN_BOTTOM_MID, 0, 5);

    grid_2048 = lv_obj_create(scr_2048);
    lv_obj_set_size(grid_2048, 230, 230);
    lv_obj_align(grid_2048, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_style_bg_color(grid_2048, lv_color_hex(0xbbada0), 0);
    lv_obj_set_style_radius(grid_2048, 6, 0);
    lv_obj_set_style_border_width(grid_2048, 0, 0);
    lv_obj_clear_flag(grid_2048, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(grid_2048, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_add_event_cb(grid_2048, grid_2048_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(grid_2048, grid_2048_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t * btn_sys = lv_btn_create(scr_2048);
    lv_obj_set_size(btn_sys, 60, 30);
    lv_obj_align(btn_sys, LV_ALIGN_TOP_LEFT, 5, 30);
    lv_obj_set_style_bg_color(btn_sys, lv_color_hex(0x8f7a66), 0);
    lv_obj_t * lbl_sys = lv_label_create(btn_sys);
    lv_obj_add_style(lbl_sys, &style_cn, 0);
    lv_label_set_text(lbl_sys, "菜单");
    lv_obj_center(lbl_sys);
    lv_obj_add_event_cb(btn_sys,[](lv_event_t *e) {
        if (modal_2048_sys) lv_obj_clear_flag(modal_2048_sys, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    modal_2048_sys = lv_obj_create(scr_2048);
    lv_obj_set_size(modal_2048_sys, 160, 215);
    lv_obj_center(modal_2048_sys);
    lv_obj_set_style_bg_color(modal_2048_sys, lv_color_hex(0xbbada0), 0);
    lv_obj_set_style_border_width(modal_2048_sys, 2, 0);
    lv_obj_set_style_border_color(modal_2048_sys, lv_color_hex(0x8f7a66), 0);
    lv_obj_add_flag(modal_2048_sys, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * btn_resume = lv_btn_create(modal_2048_sys);
    lv_obj_set_size(btn_resume, 120, 35);
    lv_obj_align(btn_resume, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_bg_color(btn_resume, lv_color_hex(0x8f7a66), 0);
    lv_obj_t * lbl_resume = lv_label_create(btn_resume);
    lv_obj_add_style(lbl_resume, &style_cn, 0);
    lv_label_set_text(lbl_resume, "继续游戏");
    lv_obj_center(lbl_resume);
    lv_obj_add_event_cb(btn_resume,[](lv_event_t *e) {
        if (modal_2048_sys) lv_obj_add_flag(modal_2048_sys, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_save = lv_btn_create(modal_2048_sys);
    lv_obj_set_size(btn_save, 120, 35);
    lv_obj_align(btn_save, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_bg_color(btn_save, lv_color_hex(0x8f7a66), 0);
    lv_obj_t * lbl_save = lv_label_create(btn_save);
    lv_obj_add_style(lbl_save, &style_cn, 0);
    lv_label_set_text(lbl_save, "保存游戏");
    lv_obj_center(lbl_save);
    lv_obj_add_event_cb(btn_save,[](lv_event_t *e) {
        if (modal_2048_sys) lv_obj_add_flag(modal_2048_sys, LV_OBJ_FLAG_HIDDEN);
        save_2048_game();
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_restart = lv_btn_create(modal_2048_sys);
    lv_obj_set_size(btn_restart, 120, 35);
    lv_obj_align(btn_restart, LV_ALIGN_TOP_MID, 0, 105);
    lv_obj_set_style_bg_color(btn_restart, lv_color_hex(0x8f7a66), 0);
    lv_obj_t * lbl_restart = lv_label_create(btn_restart);
    lv_obj_add_style(lbl_restart, &style_cn, 0);
    lv_label_set_text(lbl_restart, "重新开始");
    lv_obj_center(lbl_restart);
    lv_obj_add_event_cb(btn_restart,[](lv_event_t *e) {
        reset_2048_game();
        if (modal_2048_sys) lv_obj_add_flag(modal_2048_sys, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_exit = lv_btn_create(modal_2048_sys);
    lv_obj_set_size(btn_exit, 120, 35);
    lv_obj_align(btn_exit, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_style_bg_color(btn_exit, lv_color_hex(0x8f7a66), 0);
    lv_obj_t * lbl_exit = lv_label_create(btn_exit);
    lv_obj_add_style(lbl_exit, &style_cn, 0);
    lv_label_set_text(lbl_exit, "返回主页");
    lv_obj_center(lbl_exit);
    lv_obj_add_event_cb(btn_exit,[](lv_event_t *e) {
        if (modal_2048_sys) lv_obj_add_flag(modal_2048_sys, LV_OBJ_FLAG_HIDDEN);
        save_2048_game();
        lv_scr_load(scr_menu);
        lv_obj_del_async(scr_2048);
        scr_2048 = NULL;
    }, LV_EVENT_CLICKED, NULL);

    modal_2048_over = lv_obj_create(scr_2048);
    lv_obj_set_size(modal_2048_over, 180, 130);
    lv_obj_center(modal_2048_over);
    lv_obj_set_style_bg_color(modal_2048_over, lv_color_hex(0xeee4da), 0);
    lv_obj_set_style_bg_opa(modal_2048_over, 220, 0);
    lv_obj_add_flag(modal_2048_over, LV_OBJ_FLAG_HIDDEN);

    lbl_2048_over_text = lv_label_create(modal_2048_over);
    lv_obj_add_style(lbl_2048_over_text, &style_cn, 0);
    lv_label_set_text(lbl_2048_over_text, "游戏结束!");
    lv_obj_set_style_text_color(lbl_2048_over_text, lv_color_hex(0x776e65), 0);
    lv_obj_align(lbl_2048_over_text, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_align(lbl_2048_over_text, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t * btn_over_restart = lv_btn_create(modal_2048_over);
    lv_obj_set_size(btn_over_restart, 100, 35);
    lv_obj_align(btn_over_restart, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(btn_over_restart, lv_color_hex(0x8f7a66), 0);
    lv_obj_t * lbl_over_restart = lv_label_create(btn_over_restart);
    lv_obj_add_style(lbl_over_restart, &style_cn, 0);
    lv_label_set_text(lbl_over_restart, "重新开始");
    lv_obj_center(lbl_over_restart);
    lv_obj_add_event_cb(btn_over_restart,[](lv_event_t *e) {
        reset_2048_game();
        if (modal_2048_over) lv_obj_add_flag(modal_2048_over, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    // 消息提示弹窗
    modal_2048_msg = lv_obj_create(scr_2048);
    lv_obj_set_size(modal_2048_msg, 200, 100);
    lv_obj_center(modal_2048_msg);
    lv_obj_set_style_bg_color(modal_2048_msg, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(modal_2048_msg, 2, 0);
    lv_obj_set_style_border_color(modal_2048_msg, lv_color_hex(0x8f7a66), 0);
    lv_obj_add_flag(modal_2048_msg, LV_OBJ_FLAG_HIDDEN);

    lbl_2048_msg_content = lv_label_create(modal_2048_msg);
    lv_obj_add_style(lbl_2048_msg_content, &style_cn, 0);
    lv_label_set_text(lbl_2048_msg_content, "");
    lv_obj_set_style_text_color(lbl_2048_msg_content, lv_color_hex(0x776e65), 0);
    lv_obj_align(lbl_2048_msg_content, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_align(lbl_2048_msg_content, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *btn_msg_close = lv_btn_create(modal_2048_msg);
    lv_obj_set_size(btn_msg_close, 60, 28);
    lv_obj_align(btn_msg_close, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(btn_msg_close, lv_color_hex(0x8f7a66), 0);
    lv_obj_t *lbl_msg_close = lv_label_create(btn_msg_close);
    lv_obj_add_style(lbl_msg_close, &style_cn, 0);
    lv_label_set_text(lbl_msg_close, "确定");
    lv_obj_center(lbl_msg_close);
    lv_obj_add_event_cb(btn_msg_close, [](lv_event_t *e) {
        if (modal_2048_msg) lv_obj_add_flag(modal_2048_msg, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_CLICKED, NULL);

    refresh_2048_ui();
}

void refresh_2048_ui() {
    if (!scr_2048) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", score_2048);
    if (lbl_score) lv_label_set_text(lbl_score, buf);
    snprintf(buf, sizeof(buf), "%d", best_score_2048);
    if (lbl_best) lv_label_set_text(lbl_best, buf);
    if (grid_2048) lv_obj_invalidate(grid_2048);
}

void reset_2048_game() {
    memset(board, 0, sizeof(board));
    score_2048 = 0;
    is_2048_started = true;
    add_random_tile();
    add_random_tile();
    refresh_2048_ui();
}

Preferences prefs2048;

bool has_2048_save() {
    prefs2048.begin("g2048", true);
    bool exists = prefs2048.isKey("save");
    prefs2048.end();
    return exists;
}

void save_2048_game() {
    prefs2048.begin("g2048", false);
    SaveData2048 save;
    memcpy(save.board, board, sizeof(board));
    save.score = score_2048;
    save.best_score = best_score_2048;
    prefs2048.putBytes("save", &save, sizeof(save));
    prefs2048.end();
    show_2048_msg("游戏进度已成功保存！");
}

bool load_2048_game() {
    prefs2048.begin("g2048", true);
    if (!prefs2048.isKey("save")) { prefs2048.end(); return false; }
    SaveData2048 save;
    prefs2048.getBytes("save", &save, sizeof(save));
    prefs2048.end();
    memcpy(board, save.board, sizeof(board));
    score_2048 = save.score;
    best_score_2048 = save.best_score;
    is_2048_started = true;
    refresh_2048_ui();
    return true;
}