#include "global.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 42
#define NUM_PIXELS 1

lv_obj_t * scr_led = NULL;
lv_obj_t * sw_power = NULL;
lv_obj_t * slider_r = NULL; lv_obj_t * ta_r = NULL;
lv_obj_t * slider_g = NULL; lv_obj_t * ta_g = NULL;
lv_obj_t * slider_b = NULL; lv_obj_t * ta_b = NULL;
lv_obj_t * dd_timer = NULL;
lv_obj_t * dd_mode = NULL;
lv_obj_t * ta_timer_min = NULL;  // 自定义定时输入框
lv_obj_t * slider_on_time = NULL; lv_obj_t * lbl_on_time = NULL; lv_obj_t * ta_on_time = NULL;
lv_obj_t * slider_off_time = NULL; lv_obj_t * lbl_off_time = NULL; lv_obj_t * ta_off_time = NULL;
lv_obj_t * kb_num = NULL;
lv_obj_t * color_preview = NULL;  // 圆形颜色预览

static lv_timer_t * led_ui_timer = NULL;

volatile bool led_is_on = false;
volatile uint8_t led_r = 255, led_g = 255, led_b = 255;
volatile int led_mode = 0;
volatile uint32_t led_auto_off_time = 0;
volatile uint16_t custom_on_ms = 500, custom_off_ms = 500;
static bool is_updating_ui = false;
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
TaskHandle_t ledTaskHandle;

// 模式名称
static const char * mode_names[] = {"常亮", "慢闪", "警灯", "SOS", "七彩", "自定义", "呼吸", "彩呼吸"};

static void update_color_preview() {
    if (!color_preview) return;
    lv_color_t current_color = lv_color_make(led_r, led_g, led_b);
    lv_obj_set_style_bg_color(color_preview, current_color, 0);
    lv_obj_set_style_shadow_color(color_preview, current_color, 0);
}

static void update_rgb_ui_from_vars() {
    if(!slider_r || !slider_g || !slider_b || !ta_r || !ta_g || !ta_b) return; 
    
    if (is_updating_ui) return; 
    
    is_updating_ui = true;

    if(lv_slider_get_value(slider_r) != led_r) lv_slider_set_value(slider_r, led_r, LV_ANIM_OFF);
    if(lv_slider_get_value(slider_g) != led_g) lv_slider_set_value(slider_g, led_g, LV_ANIM_OFF);
    if(lv_slider_get_value(slider_b) != led_b) lv_slider_set_value(slider_b, led_b, LV_ANIM_OFF);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", led_r);
    if(strcmp(lv_textarea_get_text(ta_r), buf) != 0) lv_textarea_set_text(ta_r, buf);

    snprintf(buf, sizeof(buf), "%d", led_g);
    if(strcmp(lv_textarea_get_text(ta_g), buf) != 0) lv_textarea_set_text(ta_g, buf);

    snprintf(buf, sizeof(buf), "%d", led_b);
    if(strcmp(lv_textarea_get_text(ta_b), buf) != 0) lv_textarea_set_text(ta_b, buf);
    
    update_color_preview();
    
    is_updating_ui = false;
}

void led_task(void *pvParameters) {
    pixels.begin();
    pixels.setBrightness(255);
    pixels.show();

    bool last_led_state = false;
    uint8_t last_r = 0, last_g = 0, last_b = 0;
    bool needs_update = true;
    int current_running_mode = -1;

    while(1) {
        uint32_t t = millis();

        if (led_auto_off_time > 0 && t >= led_auto_off_time) {
            led_is_on = false;
            led_auto_off_time = 0;
        }

        if (!led_is_on) {
            if (last_led_state) {  
                pixels.clear();
                pixels.show();
                last_led_state = false;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            if (!last_led_state) {
                last_led_state = true;
                needs_update = true; 
            }
            
            if (current_running_mode != led_mode) {
                current_running_mode = led_mode;
                needs_update = true;
            }
            
            switch(led_mode) {
                case 0:
                    if (led_r != last_r || led_g != last_g || led_b != last_b || needs_update) {
                        pixels.setPixelColor(0, led_r, led_g, led_b);
                        pixels.show();
                        last_r = led_r; last_g = led_g; last_b = led_b;
                        needs_update = false;
                    }
                    break;
                case 1:
                    if ((t / 1000) % 2 == 0) pixels.setPixelColor(0, 255, 0, 0);
                    else pixels.clear();
                    break;
                case 2:
                    {
                        uint32_t pt = t % 800;
                        if (pt < 100) pixels.setPixelColor(0, 255, 0, 0);
                        else if (pt < 150) pixels.clear();
                        else if (pt < 250) pixels.setPixelColor(0, 255, 0, 0);
                        else if (pt < 400) pixels.clear();
                        else if (pt < 500) pixels.setPixelColor(0, 0, 0, 255);
                        else if (pt < 550) pixels.clear();
                        else if (pt < 650) pixels.setPixelColor(0, 0, 0, 255);
                        else pixels.clear();
                    }
                    break;
                case 3:
                    {
                        const char* sos = "1010100011101110111000101010000000";
                        if (sos[(t / 100) % 34] == '1') pixels.setPixelColor(0, 255, 255, 255);
                        else pixels.clear();
                    }
                    break;
                case 4:
                    {
                        uint32_t c[] = {0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0x8B00FF};
                        pixels.setPixelColor(0, c[(t / 500) % 7]);
                    }
                    break;
                case 5:
                    {
                        uint32_t cycle = custom_on_ms + custom_off_ms;
                        if (cycle > 0) {
                            if (t % cycle < custom_on_ms) pixels.setPixelColor(0, led_r, led_g, led_b);
                            else pixels.clear();
                        }
                    }
                    break;
                case 6:
                    {
                        uint32_t cycle = t % 2000;
                        float factor = cycle < 1000 ? (float)cycle / 1000.0 : (float)(2000 - cycle) / 1000.0;
                        factor = factor * factor;
                        pixels.setPixelColor(0, led_r * factor, led_g * factor, led_b * factor);
                    }
                    break;
                case 7:
                    {
                        uint32_t c[] = {0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0x8B00FF};
                        uint32_t cycle_index = (t / 2000) % 7;
                        uint32_t cycle_t = t % 2000; 
                        float factor = cycle_t < 1000 ? (float)cycle_t / 1000.0 : (float)(2000 - cycle_t) / 1000.0;
                        factor = factor * factor;
                        
                        uint8_t r = ((c[cycle_index] >> 16) & 0xFF) * factor;
                        uint8_t g = ((c[cycle_index] >> 8) & 0xFF) * factor;
                        uint8_t b = (c[cycle_index] & 0xFF) * factor;
                        
                        pixels.setPixelColor(0, r, g, b);
                    }
                    break;
            }
            pixels.show();
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

static void slider_event_cb(lv_event_t * e) {
    if (!slider_r || !slider_g || !slider_b) return;
    
    if (is_updating_ui) return;

    led_r = lv_slider_get_value(slider_r);
    led_g = lv_slider_get_value(slider_g);
    led_b = lv_slider_get_value(slider_b);
    
    if (led_mode != 0) {
        led_mode = 0;
    }
    
    if (!led_is_on) {
        led_is_on = true;
        if (sw_power) lv_obj_add_state(sw_power, LV_STATE_CHECKED);
    }

    update_rgb_ui_from_vars();
}

static void ta_event_cb(lv_event_t * e) {
    if (!ta_r || !ta_g || !ta_b) return;
    
    if (is_updating_ui) return;

    led_r = constrain(atoi(lv_textarea_get_text(ta_r)), 0, 255);
    led_g = constrain(atoi(lv_textarea_get_text(ta_g)), 0, 255);
    led_b = constrain(atoi(lv_textarea_get_text(ta_b)), 0, 255);
    
    if (led_mode != 0) {
        led_mode = 0;
    }
    
    if (!led_is_on) {
        led_is_on = true;
        if (sw_power) lv_obj_add_state(sw_power, LV_STATE_CHECKED);
    }

    update_rgb_ui_from_vars();
}

static void ta_focus_cb(lv_event_t * e) {
    if (!kb_num) return; 
    lv_obj_t * ta = lv_event_get_target(e);
    if(lv_event_get_code(e) == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb_num, ta);
        lv_obj_clear_flag(kb_num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(kb_num);
    } else if(lv_event_get_code(e) == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb_num, NULL);
        lv_obj_add_flag(kb_num, LV_OBJ_FLAG_HIDDEN);
    }
}

// 创建圆角卡片样式
static lv_obj_t * create_card(lv_obj_t * parent, int16_t width, int16_t height) {
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_size(card, width, height);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_80, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_all(card, 8, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

// 创建RGB滑块行 - 紧凑横向布局
static void create_rgb_row_compact(lv_obj_t * parent, const char * name, lv_color_t color, lv_obj_t ** sld, lv_obj_t ** ta, lv_align_t align) {
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), 35);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * lbl = lv_label_create(row);
    lv_obj_add_style(lbl, &style_cn, 0);
    lv_label_set_text(lbl, name);
    lv_obj_set_style_text_color(lbl, color, 0);
    lv_obj_set_style_text_font(lbl, &my_font_cn_16, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

    *sld = lv_slider_create(row);
    lv_obj_set_size(*sld, 90, 8);
    lv_obj_align(*sld, LV_ALIGN_LEFT_MID, 25, 0);
    lv_slider_set_range(*sld, 0, 255);
    lv_obj_set_style_bg_color(*sld, color, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(*sld, lv_color_hex(0x1a252f), LV_PART_MAIN);
    lv_obj_set_style_bg_color(*sld, color, LV_PART_KNOB);
    lv_obj_set_style_radius(*sld, 10, 0);
    lv_obj_add_event_cb(*sld, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    *ta = lv_textarea_create(row);
    lv_obj_set_size(*ta, 40, 28);
    lv_obj_align(*ta, LV_ALIGN_RIGHT_MID, 0, 0); 
    lv_textarea_set_one_line(*ta, true);
    lv_textarea_set_accepted_chars(*ta, "0123456789");
    lv_textarea_set_max_length(*ta, 3);
    lv_obj_set_style_bg_color(*ta, lv_color_hex(0x1a252f), 0);
    lv_obj_set_style_bg_opa(*ta, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(*ta, 8, 0);
    lv_obj_set_style_text_color(*ta, color, 0);
    lv_obj_add_event_cb(*ta, ta_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(*ta, ta_focus_cb, LV_EVENT_ALL, NULL);
}

void build_led_scene() {
    if (ledTaskHandle == NULL) {
        xTaskCreatePinnedToCore(led_task, "LEDTask", 4096, NULL, 1, &ledTaskHandle, 1);
    }

    if (scr_led != NULL) {
        lv_obj_del_async(scr_led);
        scr_led = NULL;
        sw_power = NULL; slider_r = NULL; slider_g = NULL; slider_b = NULL;
        ta_r = NULL; ta_g = NULL; ta_b = NULL; dd_timer = NULL; dd_mode = NULL; ta_timer_min = NULL;
        slider_on_time = NULL; lbl_on_time = NULL; ta_on_time = NULL;
        slider_off_time = NULL; lbl_off_time = NULL; ta_off_time = NULL;
        kb_num = NULL; color_preview = NULL;
    }
    if (led_ui_timer != NULL) {
        lv_timer_del(led_ui_timer);
        led_ui_timer = NULL;
    }

    scr_led = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_led, lv_color_hex(0x1a252f), 0);

    kb_num = lv_keyboard_create(scr_led);
    lv_keyboard_set_mode(kb_num, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_size(kb_num, 240, 120);
    lv_obj_align(kb_num, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(kb_num, LV_OBJ_FLAG_HIDDEN); 

    // 标题栏
    lv_obj_t * title_bar = lv_obj_create(scr_led);
    lv_obj_set_size(title_bar, lv_pct(100), 40);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_pad_all(title_bar, 0, 0);
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(title_bar);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, LV_SYMBOL_TINT " RGB 氛围灯");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 5, 0);

    lv_obj_t * btn_back = lv_btn_create(title_bar);
    lv_obj_set_size(btn_back, 55, 30);
    lv_obj_align(btn_back, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_radius(btn_back, 15, 0);
    lv_obj_t * lbl_back = lv_label_create(btn_back);
    lv_obj_add_style(lbl_back, &style_cn, 0);
    lv_label_set_text(lbl_back, "返回"); 
    lv_obj_center(lbl_back);
    lv_obj_add_event_cb(btn_back, [](lv_event_t *e){
        if (led_ui_timer) {
            lv_timer_del(led_ui_timer);
            led_ui_timer = NULL;
        }
        lv_scr_load(scr_menu);
        lv_obj_del_async(scr_led);
        scr_led = NULL;
        sw_power = NULL;
        slider_r = NULL;
        slider_g = NULL;
        slider_b = NULL;
        ta_r = NULL;
        ta_g = NULL;
        ta_b = NULL;
        dd_timer = NULL;
        slider_on_time = NULL;
        lbl_on_time = NULL;
        slider_off_time = NULL;
        lbl_off_time = NULL;
        kb_num = NULL;
        color_preview = NULL;
        dd_mode = NULL;
        ta_timer_min = NULL;
        ta_on_time = NULL;
        ta_off_time = NULL;
    }, LV_EVENT_CLICKED, NULL);

    // 可滚动内容区域
    lv_obj_t * cont = lv_obj_create(scr_led);
    lv_obj_set_size(cont, lv_pct(100), 268);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 5, 0);
    lv_obj_set_style_pad_row(cont, 4, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLL_ELASTIC);

    // === 第一行: 开关 + 颜色预览 + 定时器 ===
    lv_obj_t * row_top = create_card(cont, 225, 45);
    lv_obj_set_flex_flow(row_top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_top, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 电源开关
    lv_obj_t * lbl_power = lv_label_create(row_top);
    lv_obj_add_style(lbl_power, &style_cn, 0);
    lv_label_set_text(lbl_power, "开关");
    lv_obj_set_style_text_color(lbl_power, lv_color_hex(0xecf0f1), 0);
    
    sw_power = lv_switch_create(row_top);
    lv_obj_set_size(sw_power, 35, 18);
    if(led_is_on) lv_obj_add_state(sw_power, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw_power, lv_color_hex(0x2ecc71), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw_power, [](lv_event_t *e){
        led_is_on = lv_obj_has_state(sw_power, LV_STATE_CHECKED);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    // 圆形颜色预览 
    color_preview = lv_obj_create(row_top);
    lv_obj_set_size(color_preview, 20, 20);
    lv_obj_set_style_radius(color_preview, 10, 0);
    lv_obj_set_style_bg_color(color_preview, lv_color_make(led_r, led_g, led_b), 0);
    lv_obj_set_style_border_color(color_preview, lv_color_hex(0xecf0f1), 0);
    lv_obj_set_style_border_width(color_preview, 1, 0);
    lv_obj_set_style_shadow_width(color_preview, 8, 0);
    lv_obj_set_style_shadow_spread(color_preview, 2, 0);
    lv_obj_set_style_shadow_color(color_preview, lv_color_make(led_r, led_g, led_b), 0);

    // 定时器下拉
    dd_timer = lv_dropdown_create(row_top);
    lv_obj_set_width(dd_timer, 85);
    lv_obj_add_style(dd_timer, &style_cn, 0);
    lv_obj_set_style_radius(dd_timer, 10, 0);
    lv_obj_set_style_bg_color(dd_timer, lv_color_hex(0x1a252f), 0);
    lv_obj_set_style_text_color(dd_timer, lv_color_hex(0xecf0f1), 0);
    lv_dropdown_set_options(dd_timer, "不限\n1分钟\n5分钟\n10分钟\n30分钟\n60分钟\n自定义");
    
    lv_obj_t * list_timer = lv_dropdown_get_list(dd_timer);
    if(list_timer != NULL) {
        lv_obj_add_style(list_timer, &style_cn, 0);
        lv_obj_set_style_radius(list_timer, 10, 0);
    }
    
    lv_obj_add_event_cb(dd_timer, [](lv_event_t *e){
        int idx = lv_dropdown_get_selected(dd_timer);
        if(idx == 6) {
            // 自定义模式：显示输入框
            if (ta_timer_min) lv_obj_clear_flag(ta_timer_min, LV_OBJ_FLAG_HIDDEN);
        } else {
            if (ta_timer_min) lv_obj_add_flag(ta_timer_min, LV_OBJ_FLAG_HIDDEN);
            int min_opts[] = {0, 1, 5, 10, 30, 60};
            if(idx == 0) led_auto_off_time = 0;
            else led_auto_off_time = millis() + (min_opts[idx] * 60000);
        }
    }, LV_EVENT_VALUE_CHANGED, NULL);

    // 自定义定时输入框（默认隐藏）
    ta_timer_min = lv_textarea_create(row_top);
    lv_obj_set_size(ta_timer_min, 35, 20);
    lv_obj_align(ta_timer_min, LV_ALIGN_RIGHT_MID, -95, 0);
    lv_textarea_set_one_line(ta_timer_min, true);
    lv_textarea_set_accepted_chars(ta_timer_min, "0123456789");
    lv_textarea_set_max_length(ta_timer_min, 3);
    lv_textarea_set_text(ta_timer_min, "1");
    lv_obj_set_style_bg_color(ta_timer_min, lv_color_hex(0x1a252f), 0);
    lv_obj_set_style_bg_opa(ta_timer_min, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ta_timer_min, 6, 0);
    lv_obj_set_style_text_color(ta_timer_min, lv_color_hex(0xecf0f1), 0);
    lv_obj_add_flag(ta_timer_min, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(ta_timer_min, ta_focus_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ta_timer_min, [](lv_event_t *e){
        int mins = constrain(atoi(lv_textarea_get_text(ta_timer_min)), 0, 999);
        if(mins > 0) led_auto_off_time = millis() + (mins * 60000);
        else led_auto_off_time = 0;
    }, LV_EVENT_READY, NULL);

    led_ui_timer = lv_timer_create([](lv_timer_t *t) {
        if(sw_power && !led_is_on && lv_obj_has_state(sw_power, LV_STATE_CHECKED)) {
            lv_obj_clear_state(sw_power, LV_STATE_CHECKED);
        }
    }, 100, NULL);

    // === RGB 颜色调节面板 ===
    lv_obj_t * panel_rgb = create_card(cont, 225, 145);
    lv_obj_set_flex_flow(panel_rgb, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel_rgb, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(panel_rgb, 3, 0);
    
    lv_obj_t * lbl_rgb_title = lv_label_create(panel_rgb);
    lv_obj_add_style(lbl_rgb_title, &style_cn, 0);
    lv_label_set_text(lbl_rgb_title, "颜色调节");
    lv_obj_set_style_text_color(lbl_rgb_title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(lbl_rgb_title, LV_ALIGN_TOP_MID, 0, 0);

    create_rgb_row_compact(panel_rgb, "R", lv_color_hex(0xFF5555), &slider_r, &ta_r, LV_ALIGN_LEFT_MID);
    create_rgb_row_compact(panel_rgb, "G", lv_color_hex(0x55FF55), &slider_g, &ta_g, LV_ALIGN_LEFT_MID);
    create_rgb_row_compact(panel_rgb, "B", lv_color_hex(0x5555FF), &slider_b, &ta_b, LV_ALIGN_LEFT_MID);
    
    update_rgb_ui_from_vars();

    // === 预设颜色 - 圆形按钮 ===
    lv_obj_t * panel_colors = create_card(cont, 225, 38);
    lv_obj_set_flex_flow(panel_colors, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panel_colors, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    uint32_t preset_c[] = {0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0x8B00FF, 0xFFFFFF};
    for(int i=0; i<8; i++) {
        lv_obj_t * btn = lv_btn_create(panel_colors);
        lv_obj_set_size(btn, 22, 22);
        lv_obj_set_style_radius(btn, 11, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(preset_c[i]), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xecf0f1), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_shadow_width(btn, 8, 0);
        lv_obj_set_style_shadow_spread(btn, 3, 0);
        lv_obj_set_style_shadow_color(btn, lv_color_hex(preset_c[i]), 0);
        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            uint32_t c = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
            led_r = (c >> 16) & 0xFF; led_g = (c >> 8) & 0xFF; led_b = c & 0xFF;
            led_mode = 0;
            if (dd_mode) lv_dropdown_set_selected(dd_mode, 0);
            update_rgb_ui_from_vars();
        }, LV_EVENT_CLICKED, (void*)(uintptr_t)preset_c[i]);
    }

    // === 模式选择 - 下拉菜单 ===
    dd_mode = lv_dropdown_create(cont);
    lv_obj_set_width(dd_mode, 220);
    lv_obj_add_style(dd_mode, &style_cn, 0);
    lv_obj_set_style_radius(dd_mode, 10, 0);
    lv_obj_set_style_bg_color(dd_mode, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_bg_opa(dd_mode, LV_OPA_80, 0);
    lv_obj_set_style_text_color(dd_mode, lv_color_hex(0xecf0f1), 0);
    lv_obj_set_style_pad_all(dd_mode, 8, 0);
    lv_dropdown_set_options(dd_mode, "手动/常亮\n红色慢闪\n红蓝警灯\n白光 SOS\n七彩交替\n自定义闪烁\n单色呼吸\n七彩呼吸");
    lv_dropdown_set_selected(dd_mode, led_mode);
    lv_obj_t * list_mode = lv_dropdown_get_list(dd_mode);
    if(list_mode != NULL) {
        lv_obj_add_style(list_mode, &style_cn, 0);
        lv_obj_set_style_radius(list_mode, 10, 0);
    }

    lv_obj_add_event_cb(dd_mode, [](lv_event_t *e){
        led_mode = lv_dropdown_get_selected(dd_mode);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    // === 自定义闪烁参数 ===
    lv_obj_t * panel_custom = create_card(cont, 225, 130);
    lv_obj_set_flex_flow(panel_custom, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel_custom, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(panel_custom, 4, 0);
    
    lv_obj_t * lbl_custom_title = lv_label_create(panel_custom);
    lv_obj_add_style(lbl_custom_title, &style_cn, 0);
    lv_label_set_text(lbl_custom_title, "自定义闪烁");
    lv_obj_set_style_text_color(lbl_custom_title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(lbl_custom_title, LV_ALIGN_TOP_MID, 0, 0);

    // 亮起时间 - 标签在上，滑块+输入框在下
    lv_obj_t * group_on = lv_obj_create(panel_custom);
    lv_obj_set_size(group_on, lv_pct(100), 55);
    lv_obj_set_style_bg_opa(group_on, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(group_on, 0, 0);
    lv_obj_set_style_pad_all(group_on, 0, 0);
    lv_obj_set_style_pad_row(group_on, 2, 0);
    lv_obj_clear_flag(group_on, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(group_on, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(group_on, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lbl_on_time = lv_label_create(group_on);
    lv_obj_add_style(lbl_on_time, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_on_time, lv_color_hex(0x2ecc71), 0);

    lv_obj_t * ctrl_on = lv_obj_create(group_on);
    lv_obj_set_size(ctrl_on, lv_pct(100), 20);
    lv_obj_set_style_bg_opa(ctrl_on, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_on, 0, 0);
    lv_obj_set_style_pad_all(ctrl_on, 0, 0);
    lv_obj_clear_flag(ctrl_on, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(ctrl_on, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_on, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    slider_on_time = lv_slider_create(ctrl_on);
    lv_obj_set_size(slider_on_time, 120, 8);
    lv_slider_set_range(slider_on_time, 0, 20000);
    lv_slider_set_value(slider_on_time, custom_on_ms, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_on_time, lv_color_hex(0x2ecc71), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_on_time, lv_color_hex(0x1a252f), LV_PART_MAIN);
    lv_obj_set_style_radius(slider_on_time, 10, 0);
    lv_obj_add_event_cb(slider_on_time, [](lv_event_t *e){
        custom_on_ms = lv_slider_get_value(slider_on_time);
        lv_label_set_text_fmt(lbl_on_time, "亮起: %dms", custom_on_ms);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", custom_on_ms);
        lv_textarea_set_text(ta_on_time, buf);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    ta_on_time = lv_textarea_create(ctrl_on);
    lv_obj_set_size(ta_on_time, 45, 18);
    lv_textarea_set_one_line(ta_on_time, true);
    lv_textarea_set_accepted_chars(ta_on_time, "0123456789");
    lv_textarea_set_max_length(ta_on_time, 5);
    lv_obj_set_style_bg_color(ta_on_time, lv_color_hex(0x1a252f), 0);
    lv_obj_set_style_bg_opa(ta_on_time, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ta_on_time, 4, 0);
    lv_obj_set_style_text_color(ta_on_time, lv_color_hex(0x2ecc71), 0);
    lv_obj_set_style_pad_all(ta_on_time, 2, 0);
    lv_obj_set_style_pad_hor(ta_on_time, 3, 0);
    lv_obj_add_event_cb(ta_on_time, ta_focus_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ta_on_time, [](lv_event_t *e){
        custom_on_ms = constrain(atoi(lv_textarea_get_text(ta_on_time)), 0, 20000);
        if(slider_on_time) lv_slider_set_value(slider_on_time, custom_on_ms, LV_ANIM_OFF);
        lv_label_set_text_fmt(lbl_on_time, "亮起: %dms", custom_on_ms);
    }, LV_EVENT_READY, NULL);

    // 熄灭时间 - 标签在上，滑块+输入框在下
    lv_obj_t * group_off = lv_obj_create(panel_custom);
    lv_obj_set_size(group_off, lv_pct(100), 55);
    lv_obj_set_style_bg_opa(group_off, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(group_off, 0, 0);
    lv_obj_set_style_pad_all(group_off, 0, 0);
    lv_obj_set_style_pad_row(group_off, 2, 0);
    lv_obj_clear_flag(group_off, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(group_off, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(group_off, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lbl_off_time = lv_label_create(group_off);
    lv_obj_add_style(lbl_off_time, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_off_time, lv_color_hex(0xe74c3c), 0);

    lv_obj_t * ctrl_off = lv_obj_create(group_off);
    lv_obj_set_size(ctrl_off, lv_pct(100), 20);
    lv_obj_set_style_bg_opa(ctrl_off, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_off, 0, 0);
    lv_obj_set_style_pad_all(ctrl_off, 0, 0);
    lv_obj_clear_flag(ctrl_off, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(ctrl_off, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_off, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    slider_off_time = lv_slider_create(ctrl_off);
    lv_obj_set_size(slider_off_time, 120, 8);
    lv_slider_set_range(slider_off_time, 0, 20000);
    lv_slider_set_value(slider_off_time, custom_off_ms, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_off_time, lv_color_hex(0xe74c3c), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_off_time, lv_color_hex(0x1a252f), LV_PART_MAIN);
    lv_obj_set_style_radius(slider_off_time, 10, 0);
    lv_obj_add_event_cb(slider_off_time, [](lv_event_t *e){
        custom_off_ms = lv_slider_get_value(slider_off_time);
        lv_label_set_text_fmt(lbl_off_time, "熄灭: %dms", custom_off_ms);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", custom_off_ms);
        lv_textarea_set_text(ta_off_time, buf);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    ta_off_time = lv_textarea_create(ctrl_off);
    lv_obj_set_size(ta_off_time, 45, 18);
    lv_textarea_set_one_line(ta_off_time, true);
    lv_textarea_set_accepted_chars(ta_off_time, "0123456789");
    lv_textarea_set_max_length(ta_off_time, 5);
    lv_obj_set_style_bg_color(ta_off_time, lv_color_hex(0x1a252f), 0);
    lv_obj_set_style_bg_opa(ta_off_time, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ta_off_time, 4, 0);
    lv_obj_set_style_text_color(ta_off_time, lv_color_hex(0xe74c3c), 0);
    lv_obj_set_style_pad_all(ta_off_time, 2, 0);
    lv_obj_set_style_pad_hor(ta_off_time, 3, 0);
    lv_obj_add_event_cb(ta_off_time, ta_focus_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ta_off_time, [](lv_event_t *e){
        custom_off_ms = constrain(atoi(lv_textarea_get_text(ta_off_time)), 0, 20000);
        if(slider_off_time) lv_slider_set_value(slider_off_time, custom_off_ms, LV_ANIM_OFF);
        lv_label_set_text_fmt(lbl_off_time, "熄灭: %dms", custom_off_ms);
    }, LV_EVENT_READY, NULL);

    // 初始化标签文本
    lv_label_set_text_fmt(lbl_on_time, "亮起: %dms", custom_on_ms);
    lv_label_set_text_fmt(lbl_off_time, "熄灭: %dms", custom_off_ms);
    {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", custom_on_ms);
        lv_textarea_set_text(ta_on_time, buf);
        snprintf(buf, sizeof(buf), "%d", custom_off_ms);
        lv_textarea_set_text(ta_off_time, buf);
    }
    
    if (led_ui_timer) lv_timer_resume(led_ui_timer);
}
