#include "global.h"

void build_main_menu() {
    scr_menu = lv_obj_create(NULL);
    
    // 深灰蓝背景
    lv_obj_set_style_bg_color(scr_menu, lv_color_hex(0x2c3e50), 0);

    // 顶部标题
    lv_obj_t * title = lv_label_create(scr_menu);
    lv_obj_add_style(title, &style_cn, 0); 
    lv_label_set_text(title, "Zyzx 的游戏机");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 15, 15);

    // 标题下方的分割线
    lv_obj_t * line = lv_line_create(scr_menu);
    static lv_point_t line_points[] = { {15, 45}, {225, 45} };
    lv_line_set_points(line, line_points, 2);
    lv_obj_set_style_line_color(line, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_line_width(line, 2, 0);

    // 创建一个 Flex 容器来存放所有列表按钮，支持上下滑动
    lv_obj_t * btn_container = lv_obj_create(scr_menu);
    lv_obj_set_size(btn_container, 240, 265);
    lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(btn_container, 12, 0); // 卡片间距
    lv_obj_set_style_pad_top(btn_container, 10, 0);

    // 辅助闭包函数：用于快速创建风格统一的列表项按钮
    auto create_menu_btn = [&](const char* txt, lv_event_cb_t cb) {
        lv_obj_t * btn = lv_btn_create(btn_container);
        lv_obj_set_size(btn, 210, 50);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x34495e), 0);
        lv_obj_set_style_radius(btn, 8, 0);       // 卡片圆角
        lv_obj_set_style_shadow_width(btn, 0, 0); // 扁平化
        
        // 左侧的图标与文字
        lv_obj_t * lbl = lv_label_create(btn);
        lv_obj_add_style(lbl, &style_cn, 0);
        lv_label_set_text(lbl, txt);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xecf0f1), 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 10, 0);

        // 右侧的引导箭头
        lv_obj_t * arrow = lv_label_create(btn);
        lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_color(arrow, lv_color_hex(0x95a5a6), 0);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -5, 0);
        
        if (cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
        return btn;
    };

// 1. 魔塔
    create_menu_btn(LV_SYMBOL_IMAGE "  魔塔", [](lv_event_t *e){
        if (scr_tower == NULL) build_tower_scene(); 
        
        if (!is_tower_started) { 
            if (!load_tower_game()) reset_tower_game(); 
        }
        lv_scr_load_anim(scr_tower, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    });

    // 2. 纵横四海
    create_menu_btn(LV_SYMBOL_SHUFFLE "  纵横四海", [](lv_event_t *e){
        if (scr_zongheng == NULL) {
            build_zongheng_scene();
            if (!load_zongheng_game()) reset_zongheng_game();
        }
        lv_scr_load_anim(scr_zongheng, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    });

    // 3. 北京浮生记
    create_menu_btn(LV_SYMBOL_HOME "  北京浮生记", [](lv_event_t *e){
        if (scr_beijing == NULL) {
            build_beijing_scene();
            if (!load_beijing_game()) reset_beijing_game();
        }
        lv_scr_load_anim(scr_beijing, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    });

    // 4. 音乐播放器
    create_menu_btn(LV_SYMBOL_AUDIO "  音乐播放器", [](lv_event_t *e){
        if (scr_music == NULL) build_music_scene();
        lv_scr_load_anim(scr_music, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    });

    // 5. 电子书
    create_menu_btn(LV_SYMBOL_FILE "  TXT 电子书", [](lv_event_t *e){
        if (scr_reader == NULL) build_reader_scene();
        lv_scr_load_anim(scr_reader, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    });

        // 6. RGB 氛围灯 
    create_menu_btn(LV_SYMBOL_TINT "  RGB 氛围灯", [](lv_event_t *e){
        if (scr_led == NULL) build_led_scene();
        lv_scr_load_anim(scr_led, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    });
}