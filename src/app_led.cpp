#include "global.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 42
#define NUM_PIXELS 1

lv_obj_t * scr_led = NULL;
lv_obj_t * sw_power = NULL;
lv_obj_t * slider_r = NULL; lv_obj_t * ta_r = NULL;
lv_obj_t * slider_g = NULL; lv_obj_t * ta_g = NULL;
lv_obj_t * slider_b = NULL; lv_obj_t * ta_b = NULL;
lv_obj_t * dd_mode = NULL;
lv_obj_t * dd_timer = NULL;
lv_obj_t * slider_on_time = NULL; lv_obj_t * lbl_on_time = NULL;
lv_obj_t * slider_off_time = NULL; lv_obj_t * lbl_off_time = NULL;
lv_obj_t * kb_num = NULL; 

static lv_timer_t * led_ui_timer = NULL;

volatile bool led_is_on = false;
volatile uint8_t led_r = 255, led_g = 255, led_b = 255;
volatile int led_mode = 0;
volatile uint32_t led_auto_off_time = 0;
uint16_t custom_on_ms = 500, custom_off_ms = 500;

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
TaskHandle_t ledTaskHandle;

static void update_rgb_ui_from_vars() {
    if(!slider_r || !slider_g || !slider_b || !ta_r || !ta_g || !ta_b) return; 

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
}

void led_task(void *pvParameters) {
    pixels.begin();
    pixels.setBrightness(255);
    pixels.show();

    bool last_led_state = false;
    uint8_t last_r = 0, last_g = 0, last_b = 0;
    bool needs_update = true;

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
        } else {
            if (!last_led_state) {
                last_led_state = true;
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
            }
            pixels.show();
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void slider_event_cb(lv_event_t * e) {
    if (!slider_r || !slider_g || !slider_b) return;
    led_r = lv_slider_get_value(slider_r);
    led_g = lv_slider_get_value(slider_g);
    led_b = lv_slider_get_value(slider_b);
    update_rgb_ui_from_vars();
}

static void ta_event_cb(lv_event_t * e) {
    if (!ta_r || !ta_g || !ta_b) return;
    led_r = constrain(atoi(lv_textarea_get_text(ta_r)), 0, 255);
    led_g = constrain(atoi(lv_textarea_get_text(ta_g)), 0, 255);
    led_b = constrain(atoi(lv_textarea_get_text(ta_b)), 0, 255);
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

void create_rgb_row(lv_obj_t * parent, const char * name, lv_color_t color, lv_obj_t ** sld, lv_obj_t ** ta) {
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_set_size(row, 210, 45);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * lbl = lv_label_create(row);
    lv_obj_add_style(lbl, &style_cn, 0);
    lv_label_set_text(lbl, name);
    lv_obj_set_style_text_color(lbl, color, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 5, 0);

    *sld = lv_slider_create(row);
    lv_obj_set_size(*sld, 100, 10);
    lv_obj_align(*sld, LV_ALIGN_LEFT_MID, 25, 0);
    lv_slider_set_range(*sld, 0, 255);
    lv_obj_set_style_bg_color(*sld, color, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(*sld, color, LV_PART_KNOB);
    lv_obj_add_event_cb(*sld, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    *ta = lv_textarea_create(row);
    lv_obj_set_size(*ta, 60, 35);
    lv_obj_align(*ta, LV_ALIGN_RIGHT_MID, -5, 0); 
    lv_textarea_set_one_line(*ta, true);
    lv_textarea_set_accepted_chars(*ta, "0123456789");
    lv_textarea_set_max_length(*ta, 3);
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
        ta_r = NULL; ta_g = NULL; ta_b = NULL; dd_mode = NULL; dd_timer = NULL;
        slider_on_time = NULL; lbl_on_time = NULL; slider_off_time = NULL; lbl_off_time = NULL;
        kb_num = NULL; 
    }
    if (led_ui_timer != NULL) {
        lv_timer_del(led_ui_timer);
        led_ui_timer = NULL;
    }

    scr_led = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_led, lv_color_hex(0x2c3e50), 0);

    kb_num = lv_keyboard_create(scr_led);
    lv_keyboard_set_mode(kb_num, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_size(kb_num, 240, 120);
    lv_obj_align(kb_num, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(kb_num, LV_OBJ_FLAG_HIDDEN); 

    lv_obj_t * title = lv_label_create(scr_led);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, LV_SYMBOL_TINT " RGB 氛围灯");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 15, 15);

    lv_obj_t * btn_back = lv_btn_create(scr_led);
    lv_obj_set_size(btn_back, 60, 35);
    lv_obj_align(btn_back, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x34495e), 0);
    lv_obj_t * lbl_back = lv_label_create(btn_back);
    lv_obj_add_style(lbl_back, &style_cn, 0);
    lv_label_set_text(lbl_back, "返回"); lv_obj_center(lbl_back);
    lv_obj_add_event_cb(btn_back, [](lv_event_t *e){
        if (led_ui_timer) lv_timer_pause(led_ui_timer); // ✅ 离开时暂停
        // 清理内存
        lv_scr_load(scr_menu);
        lv_obj_del_async(scr_led);
        scr_led = NULL;
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * cont = lv_obj_create(scr_led);
    lv_obj_set_size(cont, 240, 265);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * row_top = lv_obj_create(cont);
    lv_obj_set_size(row_top, 220, 50);
    lv_obj_set_style_bg_color(row_top, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(row_top, 0, 0);
    lv_obj_clear_flag(row_top, LV_OBJ_FLAG_SCROLLABLE);

    sw_power = lv_switch_create(row_top);
    lv_obj_align(sw_power, LV_ALIGN_LEFT_MID, 0, 0);
    if(led_is_on) lv_obj_add_state(sw_power, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw_power, [](lv_event_t *e){
        led_is_on = lv_obj_has_state(sw_power, LV_STATE_CHECKED);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    led_ui_timer = lv_timer_create([](lv_timer_t *t) {
        if(sw_power && !led_is_on && lv_obj_has_state(sw_power, LV_STATE_CHECKED)) {
            lv_obj_clear_state(sw_power, LV_STATE_CHECKED);
        }
    }, 100, NULL);

    dd_timer = lv_dropdown_create(row_top);
    lv_obj_set_width(dd_timer, 120);
    lv_obj_align(dd_timer, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_style(dd_timer, &style_cn, 0);
    lv_dropdown_set_options(dd_timer, "不自动关闭\n1 分钟后\n5 分钟后\n10 分钟后\n30 分钟后\n60 分钟后");
    
    lv_obj_t * list_timer = lv_dropdown_get_list(dd_timer);
    if(list_timer != NULL) lv_obj_add_style(list_timer, &style_cn, 0);
    
    lv_obj_add_event_cb(dd_timer, [](lv_event_t *e){
        int min_opts[] = {0, 1, 5, 10, 30, 60};
        int idx = lv_dropdown_get_selected(dd_timer);
        if(idx == 0) led_auto_off_time = 0;
        else led_auto_off_time = millis() + (min_opts[idx] * 60000);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * panel_rgb = lv_obj_create(cont);
    lv_obj_set_size(panel_rgb, 220, 150); 
    lv_obj_set_style_pad_all(panel_rgb, 5, 0); 
    lv_obj_set_style_pad_row(panel_rgb, 2, 0); 
    lv_obj_set_style_bg_color(panel_rgb, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(panel_rgb, 0, 0);
    lv_obj_clear_flag(panel_rgb, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(panel_rgb, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel_rgb, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    create_rgb_row(panel_rgb, "R", lv_color_hex(0xFF5555), &slider_r, &ta_r);
    create_rgb_row(panel_rgb, "G", lv_color_hex(0x55FF55), &slider_g, &ta_g);
    create_rgb_row(panel_rgb, "B", lv_color_hex(0x5555FF), &slider_b, &ta_b);
    
    update_rgb_ui_from_vars();

    lv_obj_t * panel_colors = lv_obj_create(cont);
    lv_obj_set_size(panel_colors, 220, 50);
    lv_obj_set_style_bg_color(panel_colors, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(panel_colors, 0, 0);
    lv_obj_set_flex_flow(panel_colors, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panel_colors, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(panel_colors, LV_OBJ_FLAG_SCROLLABLE);

    uint32_t preset_c[] = {0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0x8B00FF, 0xFFFFFF};
    for(int i=0; i<8; i++) {
        lv_obj_t * btn = lv_btn_create(panel_colors);
        lv_obj_set_size(btn, 20, 30);
        lv_obj_set_style_bg_color(btn, lv_color_hex(preset_c[i]), 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            uint32_t c = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
            led_r = (c >> 16) & 0xFF; led_g = (c >> 8) & 0xFF; led_b = c & 0xFF;
            led_mode = 0; 
            lv_dropdown_set_selected(dd_mode, 0);
            update_rgb_ui_from_vars();
        }, LV_EVENT_CLICKED, (void*)(uintptr_t)preset_c[i]);
    }

    dd_mode = lv_dropdown_create(cont);
    lv_obj_set_width(dd_mode, 220);
    lv_obj_add_style(dd_mode, &style_cn, 0);
    lv_dropdown_set_options(dd_mode, "手动/常亮\n红色慢闪\n红蓝警灯\n白光 SOS\n七彩交替\n自定义闪烁");
    lv_dropdown_set_selected(dd_mode, led_mode);
    lv_obj_t * list_mode = lv_dropdown_get_list(dd_mode);
    if(list_mode != NULL) lv_obj_add_style(list_mode, &style_cn, 0);

    lv_obj_add_event_cb(dd_mode, [](lv_event_t *e){
        led_mode = lv_dropdown_get_selected(dd_mode);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * panel_custom = lv_obj_create(cont);
    lv_obj_set_size(panel_custom, 220, 90);
    lv_obj_set_style_bg_color(panel_custom, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(panel_custom, 0, 0);
    lv_obj_clear_flag(panel_custom, LV_OBJ_FLAG_SCROLLABLE);

    lbl_on_time = lv_label_create(panel_custom);
    lv_obj_add_style(lbl_on_time, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_on_time, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(lbl_on_time, LV_ALIGN_TOP_LEFT, 0, 0);
    slider_on_time = lv_slider_create(panel_custom);
    lv_obj_set_size(slider_on_time, 120, 10);
    lv_obj_align(slider_on_time, LV_ALIGN_TOP_RIGHT, 0, 5);
    lv_slider_set_range(slider_on_time, 50, 2000);
    lv_slider_set_value(slider_on_time, custom_on_ms, LV_ANIM_OFF);

    lbl_off_time = lv_label_create(panel_custom);
    lv_obj_add_style(lbl_off_time, &style_cn, 0);
    lv_obj_set_style_text_color(lbl_off_time, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(lbl_off_time, LV_ALIGN_BOTTOM_LEFT, 0, -5);
    slider_off_time = lv_slider_create(panel_custom);
    lv_obj_set_size(slider_off_time, 120, 10);
    lv_obj_align(slider_off_time, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_slider_set_range(slider_off_time, 50, 2000);
    lv_slider_set_value(slider_off_time, custom_off_ms, LV_ANIM_OFF);

    static auto update_flash_labels = [](lv_event_t *e){
        if (!slider_on_time || !slider_off_time) return;
        custom_on_ms = lv_slider_get_value(slider_on_time);
        custom_off_ms = lv_slider_get_value(slider_off_time);
        lv_label_set_text_fmt(lbl_on_time, "亮起: %d ms", custom_on_ms);
        lv_label_set_text_fmt(lbl_off_time, "熄灭: %d ms", custom_off_ms);
    };
    lv_obj_add_event_cb(slider_on_time, update_flash_labels, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(slider_off_time, update_flash_labels, LV_EVENT_VALUE_CHANGED, NULL);
    update_flash_labels(NULL);
    
    // 在 build_led_scene() 结尾添加：
    if (led_ui_timer) lv_timer_resume(led_ui_timer); // ✅ 进入时恢复定时器
}
