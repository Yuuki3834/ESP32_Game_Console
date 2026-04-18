#include "global.h"
#include <SD_MMC.h>
#include <LittleFS.h>
#include <vector>

lv_obj_t * scr_reader = NULL;
lv_obj_t * scr_reading = NULL;
lv_obj_t * list_bookshelf = NULL;
lv_obj_t * modal_import = NULL;
lv_obj_t * list_sd_files = NULL;
lv_obj_t * modal_menu = NULL;

lv_obj_t * lbl_content = NULL;
lv_obj_t * overlay_menu = NULL;
lv_obj_t * slider_brightness = NULL;
lv_obj_t * slider_spacing = NULL;
lv_style_t style_read_text; 

std::vector<String> bookshelf_files;
File current_book;
uint32_t file_size = 0;
uint32_t current_pos = 0;
bool is_night_mode = false;
bool is_remove_mode = false;
int line_spacing = 5;
int screen_brightness = 255;

void load_bookshelf() {
    bookshelf_files.clear();
    if (LittleFS.exists("/books.dat")) {
        File f = LittleFS.open("/books.dat", FILE_READ);
        while (f.available()) {
            String path = f.readStringUntil('\n');
            path.trim();
            if (path.length() > 0) bookshelf_files.push_back(path);
        }
        f.close();
    }
}

void save_bookshelf() {
    File f = LittleFS.open("/books.dat", FILE_WRITE);
    for (String path : bookshelf_files) {
        f.println(path);
    }
    f.close();
}

void refresh_bookshelf_ui();

void import_book(const char* path) {
    for(String p : bookshelf_files) { if(p == path) return; } 
    bookshelf_files.push_back(path);
    save_bookshelf();
    lv_obj_add_flag(modal_import, LV_OBJ_FLAG_HIDDEN);
    refresh_bookshelf_ui();
}

void remove_book(int index) {
    bookshelf_files.erase(bookshelf_files.begin() + index);
    save_bookshelf();
    refresh_bookshelf_ui();
}

void init_backlight() {
    static bool init_done = false;
    if (!init_done) {
        ledcSetup(1, 5000, 8); 
        ledcAttachPin(45, 1);
        ledcWrite(1, screen_brightness);
        init_done = true;
    }
}

void read_page(uint32_t pos) {
    if (!current_book) return;
    current_book.seek(pos);
    
    char buf[513] = {0}; 
    int bytesRead = current_book.read((uint8_t*)buf, 512);
    int originalRead = bytesRead; 
    
    if (bytesRead > 0) {
        while (bytesRead > 0) {
            uint8_t last_byte = buf[bytesRead - 1];
            if ((last_byte & 0x80) == 0) {
                break; 
            } else if ((last_byte & 0xC0) == 0x80) {
                bytesRead--; 
            } else {
                bytesRead--; 
                break;
            }
        }
        buf[bytesRead] = '\0';
        
        if (bytesRead < originalRead) {
            current_book.seek(current_book.position() - (originalRead - bytesRead));
        }
        current_pos = pos;
    } else {
        strcpy(buf, "\n\n      --- 本书完 ---");
    }
    lv_label_set_text(lbl_content, buf);
}

void jump_chapter(bool forward) {
    extern volatile bool req_pause;
    extern volatile bool is_playing;
    if(is_playing) {
        req_pause = true; // 阅读时强制暂停音乐，保护 SD 总线
    }
    if (!current_book || file_size == 0) return;
    uint32_t search_pos = current_pos;
    
    if (forward) {
        search_pos += 450; 
        while (search_pos < file_size && search_pos < current_pos + 60000) { 
            current_book.seek(search_pos);
            char buf[512];
            int len = current_book.read((uint8_t*)buf, 500);
            if (len <= 0) break;
            buf[len] = 0;
            char* p1 = strstr(buf, "第");
            if (p1) {
                char* p2 = strstr(p1, "章");
                if (p2 && (p2 - p1 < 30)) { 
                    current_pos = search_pos + (p1 - buf);
                    read_page(current_pos);
                    return;
                }
            }
            search_pos += 450;

            vTaskDelay(pdMS_TO_TICKS(1)); 
        }
        current_pos = (current_pos + 10000 < file_size) ? current_pos + 10000 : file_size - 500;
    } else {
        current_pos = (current_pos > 10000) ? current_pos - 10000 : 0; 
    }
    read_page(current_pos);
}

void open_book(const char* path) {
    extern volatile bool req_pause;
    extern volatile bool is_playing;
    if(is_playing) {
        req_pause = true; // 阅读时强制暂停音乐，保护 SD 总线
    }
    if (current_book) current_book.close();
    current_book = SD_MMC.open(path, FILE_READ);
    if (!current_book) return;
    file_size = current_book.size();
    
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style_read_text);
        style_inited = true;
    }

    lv_style_set_text_font(&style_read_text, &my_font_cn_16);
    lv_style_set_text_line_space(&style_read_text, line_spacing);
    lv_obj_remove_style_all(lbl_content); 
    lv_obj_add_style(lbl_content, &style_read_text, 0);
    
    init_backlight();
    read_page(0);
    lv_obj_add_flag(overlay_menu, LV_OBJ_FLAG_HIDDEN); 
    lv_scr_load_anim(scr_reading, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
}

static void reading_screen_click_cb(lv_event_t * e) {
    lv_indev_t * indev = lv_indev_get_act();
    lv_point_t p;
    lv_indev_get_point(indev, &p);
    
    if (!lv_obj_has_flag(overlay_menu, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(overlay_menu, LV_OBJ_FLAG_HIDDEN); 
        return;
    }

    if (p.x < 80) {
        uint32_t target = (current_pos > 450) ? current_pos - 450 : 0;
        read_page(target);
    } else if (p.x > 160) {
        read_page(current_pos + 450);
    } else {
        lv_obj_clear_flag(overlay_menu, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(overlay_menu);
    }
}

void refresh_bookshelf_ui() {
    lv_obj_clean(list_bookshelf);
    if (bookshelf_files.empty()) {
        lv_obj_t * lbl = lv_label_create(list_bookshelf);
        lv_obj_add_style(lbl, &style_cn, 0);
        lv_label_set_text(lbl, "书架空空如也\n请从菜单导入书籍");
        lv_obj_set_style_text_color(lbl, lv_color_hex(0x888888), 0);
        lv_obj_center(lbl);
        return;
    }

    for (int i = 0; i < bookshelf_files.size(); i++) {
        String fname = bookshelf_files[i].substring(bookshelf_files[i].lastIndexOf('/') + 1);
        lv_obj_t * btn = lv_list_add_btn(list_bookshelf, is_remove_mode ? LV_SYMBOL_TRASH : LV_SYMBOL_FILE, fname.c_str());
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x34495e), 0);
        lv_obj_t * lbl = lv_obj_get_child(btn, 1);
        if (lbl) {
            lv_obj_add_style(lbl, &style_cn, 0);
            lv_obj_set_style_text_color(lbl, is_remove_mode ? lv_color_hex(0xff5555) : lv_color_hex(0xecf0f1), 0);
        }
        
        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            int idx = (int)(intptr_t)lv_event_get_user_data(e);
            if (is_remove_mode) remove_book(idx);
            else open_book(bookshelf_files[idx].c_str());
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
}

void open_import_modal() {
    lv_obj_clean(list_sd_files);
    File root = SD_MMC.open("/");
    if (root) {
        File file = root.openNextFile();
        while (file) {
            String fname = file.name();
            if (!file.isDirectory() && (fname.endsWith(".txt") || fname.endsWith(".TXT"))) {
                lv_obj_t * btn = lv_list_add_btn(list_sd_files, LV_SYMBOL_FILE, fname.c_str());
                lv_obj_t * lbl = lv_obj_get_child(btn, 1);
                if (lbl) lv_obj_add_style(lbl, &style_cn, 0);
                
                String full_path = "/" + fname;
                char * path_ptr = strdup(full_path.c_str());
                
                lv_obj_add_event_cb(btn, [](lv_event_t *e){
                    import_book((const char*)lv_event_get_user_data(e));
                }, LV_EVENT_CLICKED, path_ptr);

                lv_obj_add_event_cb(btn, [](lv_event_t *e){
                    char * ptr = (char*)lv_event_get_user_data(e);
                    if(ptr) free(ptr);
                }, LV_EVENT_DELETE, path_ptr);
            }
            File next_file = root.openNextFile();
            file.close();  // 显式释放上一轮的文件句柄
            file = next_file;
        }
        root.close();  // 释放目录句柄
    }
    lv_obj_clear_flag(modal_import, LV_OBJ_FLAG_HIDDEN);
}

void build_reader_scene() {
    load_bookshelf();

    if (scr_reader != NULL) { 
        lv_obj_del(scr_reader); 
        scr_reader = NULL; 
        
        list_bookshelf = NULL;
        modal_import = NULL;
        list_sd_files = NULL;
        modal_menu = NULL;
    }
    scr_reader = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_reader, lv_color_hex(0x2c3e50), 0);

    lv_obj_t * title = lv_label_create(scr_reader);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, "TXT 书架");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 15, 15);

    lv_obj_t * btn_menu = lv_btn_create(scr_reader);
    lv_obj_set_size(btn_menu, 60, 35);
    lv_obj_align(btn_menu, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_bg_color(btn_menu, lv_color_hex(0x34495e), 0);
    lv_obj_t * lbl_menu = lv_label_create(btn_menu);
    lv_obj_add_style(lbl_menu, &style_cn, 0);
    lv_label_set_text(lbl_menu, "菜单"); lv_obj_center(lbl_menu);
    lv_obj_add_event_cb(btn_menu, [](lv_event_t *e){ lv_obj_clear_flag(modal_menu, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    list_bookshelf = lv_list_create(scr_reader);
    lv_obj_set_size(list_bookshelf, 230, 260);
    lv_obj_align(list_bookshelf, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(list_bookshelf, lv_color_hex(0x2c3e50), 0);
    lv_obj_set_style_border_width(list_bookshelf, 0, 0);
    refresh_bookshelf_ui();

    modal_menu = lv_obj_create(scr_reader);
    lv_obj_set_size(modal_menu, 160, 200);
    lv_obj_center(modal_menu);
    lv_obj_set_style_bg_color(modal_menu, lv_color_hex(0x1a252f), 0);
    lv_obj_add_flag(modal_menu, LV_OBJ_FLAG_HIDDEN);

    const char* m_opts[] = {"导入书籍", "移除书籍", "返回主页", "关闭菜单"};
    for(int i=0; i<4; i++) {
        lv_obj_t * b = lv_btn_create(modal_menu);
        lv_obj_set_size(b, 130, 35);
        lv_obj_align(b, LV_ALIGN_TOP_MID, 0, i*45);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x34495e), 0);
        lv_obj_t * l = lv_label_create(b); lv_obj_add_style(l, &style_cn, 0);
        lv_label_set_text(l, m_opts[i]); lv_obj_center(l);
        lv_obj_add_event_cb(b, [](lv_event_t *e){
            int id = (int)(intptr_t)lv_event_get_user_data(e);
            lv_obj_add_flag(modal_menu, LV_OBJ_FLAG_HIDDEN);
            if(id == 0) open_import_modal();
            if(id == 1) { is_remove_mode = !is_remove_mode; refresh_bookshelf_ui(); }
            if(id == 2) {if (current_book) current_book.close(); lv_scr_load_anim(scr_menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
}
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    modal_import = lv_obj_create(scr_reader);
    lv_obj_set_size(modal_import, 220, 280);
    lv_obj_center(modal_import);
    lv_obj_set_style_bg_color(modal_import, lv_color_hex(0x1a252f), 0);
    lv_obj_add_flag(modal_import, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_t * lbl_imp = lv_label_create(modal_import);
    lv_obj_add_style(lbl_imp, &style_cn, 0); lv_label_set_text(lbl_imp, "选择要导入的 TXT");
    lv_obj_set_style_text_color(lbl_imp, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_imp, LV_ALIGN_TOP_MID, 0, 0);

    list_sd_files = lv_list_create(modal_import);
    lv_obj_set_size(list_sd_files, 200, 190);
    lv_obj_align(list_sd_files, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t * btn_cls_imp = lv_btn_create(modal_import);
    lv_obj_set_size(btn_cls_imp, 100, 35);
    lv_obj_align(btn_cls_imp, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_cls_imp, lv_color_hex(0xaa3333), 0);
    lv_obj_t * l_ci = lv_label_create(btn_cls_imp); lv_obj_add_style(l_ci, &style_cn, 0);
    lv_label_set_text(l_ci, "取消"); lv_obj_center(l_ci);
    lv_obj_add_event_cb(btn_cls_imp, [](lv_event_t *e){ lv_obj_add_flag(modal_import, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_CLICKED, NULL);

    if (scr_reading != NULL) { 
        lv_obj_del(scr_reading); 
        scr_reading = NULL; 
        lbl_content = NULL;
        overlay_menu = NULL;
        slider_brightness = NULL;
        slider_spacing = NULL;
        overlay_menu = NULL;
        slider_brightness = NULL;
        slider_spacing = NULL;
    }
    scr_reading = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_reading, lv_color_hex(0xf4ecd8), 0); 
    
    lv_obj_add_flag(scr_reading, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr_reading, reading_screen_click_cb, LV_EVENT_CLICKED, NULL);

    lbl_content = lv_label_create(scr_reading);
    lv_obj_set_width(lbl_content, 230);
    lv_obj_align(lbl_content, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_add_flag(lbl_content, LV_OBJ_FLAG_CLICK_FOCUSABLE); 

    overlay_menu = lv_obj_create(scr_reading);
    lv_obj_set_size(overlay_menu, 240, 320);
    lv_obj_center(overlay_menu);
    lv_obj_set_style_bg_color(overlay_menu, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(overlay_menu, 200, 0);
    lv_obj_set_style_border_width(overlay_menu, 0, 0);
    lv_obj_add_flag(overlay_menu, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(overlay_menu, LV_OBJ_FLAG_CLICKABLE); 
    
    lv_obj_t * t_bar = lv_obj_create(overlay_menu);
    lv_obj_set_size(t_bar, 240, 50);
    lv_obj_align(t_bar, LV_ALIGN_TOP_MID, 0, -15);
    lv_obj_set_style_bg_color(t_bar, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_width(t_bar, 0, 0);

    lv_obj_t * btn_exit_read = lv_btn_create(t_bar);
    lv_obj_set_size(btn_exit_read, 60, 35);
    lv_obj_align(btn_exit_read, LV_ALIGN_LEFT_MID, -10, 0);
    lv_obj_set_style_bg_color(btn_exit_read, lv_color_hex(0x552222), 0);
    lv_obj_t * l_er = lv_label_create(btn_exit_read); lv_obj_add_style(l_er, &style_cn, 0);
    lv_label_set_text(l_er, "书架"); lv_obj_center(l_er);
    lv_obj_add_event_cb(btn_exit_read, [](lv_event_t *e){
        if (current_book) current_book.close();
        lv_scr_load_anim(scr_reader, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, false);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_night = lv_btn_create(t_bar);
    lv_obj_set_size(btn_night, 60, 35);
    lv_obj_align(btn_night, LV_ALIGN_RIGHT_MID, 10, 0);
    lv_obj_set_style_bg_color(btn_night, lv_color_hex(0x444444), 0);
    lv_obj_t * l_nt = lv_label_create(btn_night); lv_obj_add_style(l_nt, &style_cn, 0);
    lv_label_set_text(l_nt, "夜间"); lv_obj_center(l_nt);
    lv_obj_add_event_cb(btn_night, [](lv_event_t *e){
        is_night_mode = !is_night_mode;
        if (is_night_mode) {
            lv_obj_set_style_bg_color(scr_reading, lv_color_hex(0x111111), 0);
            lv_obj_set_style_text_color(lbl_content, lv_color_hex(0x666666), 0);
        } else {
            lv_obj_set_style_bg_color(scr_reading, lv_color_hex(0xf4ecd8), 0);
            lv_obj_set_style_text_color(lbl_content, lv_color_hex(0x222222), 0);
        }
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * b_bar = lv_obj_create(overlay_menu);
    lv_obj_set_size(b_bar, 240, 140);
    lv_obj_align(b_bar, LV_ALIGN_BOTTOM_MID, 0, 15);
    lv_obj_set_style_bg_color(b_bar, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_width(b_bar, 0, 0);

    lv_obj_t * btn_c_prev = lv_btn_create(b_bar);
    lv_obj_set_size(btn_c_prev, 90, 35);
    lv_obj_align(btn_c_prev, LV_ALIGN_TOP_LEFT, -5, -5);
    lv_obj_t * l_cp = lv_label_create(btn_c_prev); lv_obj_add_style(l_cp, &style_cn, 0);
    lv_label_set_text(l_cp, "上一章"); lv_obj_center(l_cp);
    lv_obj_add_event_cb(btn_c_prev, [](lv_event_t *e){ jump_chapter(false); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_c_next = lv_btn_create(b_bar);
    lv_obj_set_size(btn_c_next, 90, 35);
    lv_obj_align(btn_c_next, LV_ALIGN_TOP_RIGHT, 5, -5);
    lv_obj_t * l_cn = lv_label_create(btn_c_next); lv_obj_add_style(l_cn, &style_cn, 0);
    lv_label_set_text(l_cn, "下一章"); lv_obj_center(l_cn);
    lv_obj_add_event_cb(btn_c_next, [](lv_event_t *e){ jump_chapter(true); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_bri = lv_label_create(b_bar);
    lv_obj_add_style(lbl_bri, &style_cn, 0); lv_label_set_text(lbl_bri, "亮度");
    lv_obj_set_style_text_color(lbl_bri, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(lbl_bri, LV_ALIGN_LEFT_MID, -5, 10);
    
    slider_brightness = lv_slider_create(b_bar);
    lv_obj_set_size(slider_brightness, 140, 10);
    lv_obj_align(slider_brightness, LV_ALIGN_RIGHT_MID, 5, 10);
    lv_slider_set_range(slider_brightness, 10, 255);
    lv_slider_set_value(slider_brightness, 255, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_brightness, [](lv_event_t *e){
        screen_brightness = lv_slider_get_value(lv_event_get_target(e));
        ledcWrite(1, screen_brightness);
    }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * lbl_sp = lv_label_create(b_bar);
    lv_obj_add_style(lbl_sp, &style_cn, 0); lv_label_set_text(lbl_sp, "间距");
    lv_obj_set_style_text_color(lbl_sp, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(lbl_sp, LV_ALIGN_BOTTOM_LEFT, -5, -5);

    slider_spacing = lv_slider_create(b_bar);
    lv_obj_set_size(slider_spacing, 140, 10);
    lv_obj_align(slider_spacing, LV_ALIGN_BOTTOM_RIGHT, 5, -5);
    lv_slider_set_range(slider_spacing, 0, 20);
    lv_slider_set_value(slider_spacing, 5, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_spacing, [](lv_event_t *e){
        line_spacing = lv_slider_get_value(lv_event_get_target(e));
        lv_style_set_text_line_space(&style_read_text, line_spacing);
        lv_obj_report_style_change(&style_read_text); 
    }, LV_EVENT_VALUE_CHANGED, NULL);
}