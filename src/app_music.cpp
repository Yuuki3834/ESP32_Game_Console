#include "global.h"
#include <SD_MMC.h>
#include "Audio.h"
#include <vector>

lv_obj_t * scr_music = NULL;
lv_obj_t * list_music = NULL;
lv_obj_t * lbl_song_title = NULL;
lv_obj_t * btn_play = NULL;
lv_obj_t * lbl_play_icon = NULL;
lv_obj_t * slider_vol = NULL;

static lv_timer_t * music_ui_timer = NULL;

Audio audio;
TaskHandle_t audioTaskHandle = NULL;

std::vector<String> playlist;
int current_song_idx = -1;
volatile bool is_playing = false;
static bool playlist_initialized = false;

volatile bool req_play = false;
volatile int req_song_idx = -1;
volatile bool req_pause = false;
volatile bool req_stop = false;
volatile int req_volume = 15;
volatile bool req_volume_changed = false;

void play_song_by_index(int idx);

void audio_task(void *pvParameters) {
    audio.setPinout(5, 7, 6, -1, 4);
    audio.setVolume(15); 
    
    while(1) {
        if (req_stop) {
            audio.stopSong();
            is_playing = false;
            req_stop = false;
        }
        
        if (req_play && req_song_idx >= 0 && req_song_idx < (int)playlist.size()) {
            audio.stopSong();
            String path = String("/") + playlist[req_song_idx];
            audio.connecttoFS(SD_MMC, path.c_str());
            current_song_idx = req_song_idx;
            is_playing = true;
            req_play = false;
        }
        
        if (req_pause) {
            audio.pauseResume();
            is_playing = !is_playing;
            req_pause = false;
        }
        
        if (req_volume_changed) {
            audio.setVolume(req_volume);
            req_volume_changed = false;
        }
        
        if (audio.isRunning()) {
            audio.loop();
            vTaskDelay(pdMS_TO_TICKS(1));
        } else {
            if (is_playing) {
                is_playing = false; 
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void play_song_by_index(int idx) {
    if (playlist.empty() || idx < 0 || idx >= (int)playlist.size()) return;
    
    req_song_idx = idx;
    req_play = true;
    
    if (lbl_song_title) lv_label_set_text(lbl_song_title, playlist[idx].c_str());
    if (lbl_play_icon) lv_label_set_text(lbl_play_icon, LV_SYMBOL_PAUSE);
}

void init_playlist_once() {
    if (playlist_initialized) return;

    playlist.clear();
    File root = SD_MMC.open("/");
    if (root) {
        File file = root.openNextFile();
        while (file) {
            String filename = file.name();
            if (!file.isDirectory() && (filename.endsWith(".mp3") || filename.endsWith(".MP3"))) {
                playlist.push_back(filename);
            }
            file = root.openNextFile();
        }
        root.close();  
    }
    
    playlist_initialized = true;
}

void build_music_scene() {
    init_playlist_once(); 
    if (audioTaskHandle == NULL) {
        xTaskCreatePinnedToCore(audio_task, "AudioTask", 16384, NULL, 2, &audioTaskHandle, 0);
    }
    if (scr_music != NULL) { 
        lv_obj_del(scr_music); 
        scr_music = NULL; 
        list_music = NULL;
        lbl_song_title = NULL;
        btn_play = NULL;
        lbl_play_icon = NULL;
        slider_vol = NULL;
    }
    
    if (music_ui_timer != NULL) {
        lv_timer_del(music_ui_timer);
        music_ui_timer = NULL;
    }

    scr_music = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_music, lv_color_hex(0x1a1a2e), 0);

    lv_obj_t * btn_back = lv_btn_create(scr_music);
    lv_obj_set_size(btn_back, 60, 35);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x16213e), 0);
    lv_obj_t * lbl_back = lv_label_create(btn_back);
    lv_obj_add_style(lbl_back, &style_cn, 0);
    lv_label_set_text(lbl_back, "返回");
    lv_obj_center(lbl_back);
    
    lv_obj_add_event_cb(btn_back, [](lv_event_t *e){
        req_stop = true; // 仅停止后台播放，但不清空任何指针
        // [删除这几行]
        // if (music_ui_timer != NULL) { lv_timer_del(music_ui_timer); music_ui_timer = NULL; }
        // list_music = NULL; lbl_song_title = NULL; btn_play = NULL;
        // lbl_play_icon = NULL; slider_vol = NULL;
        
        lv_scr_load_anim(scr_menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * panel_control = lv_obj_create(scr_music);
    lv_obj_set_size(panel_control, 230, 110);
    lv_obj_align(panel_control, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_color(panel_control, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_border_width(panel_control, 0, 0);

    lbl_song_title = lv_label_create(panel_control);
    lv_obj_add_style(lbl_song_title, &style_cn, 0);
    if (current_song_idx >= 0 && current_song_idx < (int)playlist.size()) {
        lv_label_set_text(lbl_song_title, playlist[current_song_idx].c_str());
    } else {
        lv_label_set_text(lbl_song_title, "请选择歌曲...");
    }
    lv_obj_set_style_text_color(lbl_song_title, lv_color_hex(0xe94560), 0);
    lv_label_set_long_mode(lbl_song_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(lbl_song_title, 200);
    lv_obj_align(lbl_song_title, LV_ALIGN_TOP_MID, 0, -10);

    lv_obj_t * btn_prev = lv_btn_create(panel_control);
    lv_obj_set_size(btn_prev, 45, 45);
    lv_obj_align(btn_prev, LV_ALIGN_LEFT_MID, 10, 5);
    lv_obj_t * lbl_prev = lv_label_create(btn_prev);
    lv_label_set_text(lbl_prev, LV_SYMBOL_PREV); lv_obj_center(lbl_prev);
    lv_obj_add_event_cb(btn_prev, [](lv_event_t *e){
        if (!playlist.empty()) {
            int next_idx = (current_song_idx - 1 < 0) ? playlist.size() - 1 : current_song_idx - 1;
            play_song_by_index(next_idx);
        }
    }, LV_EVENT_CLICKED, NULL);

    btn_play = lv_btn_create(panel_control);
    lv_obj_set_size(btn_play, 55, 55);
    lv_obj_align(btn_play, LV_ALIGN_CENTER, 0, 5);
    lbl_play_icon = lv_label_create(btn_play);
    lv_label_set_text(lbl_play_icon, is_playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY); 
    lv_obj_center(lbl_play_icon);
    
    lv_obj_add_event_cb(btn_play, [](lv_event_t *e){
        if (current_song_idx == -1 && !playlist.empty()) {
            play_song_by_index(0);
        } else if (is_playing) {
            req_pause = true;
            if(lbl_play_icon) lv_label_set_text(lbl_play_icon, LV_SYMBOL_PLAY);
        } else {
            req_pause = true;
            if(lbl_play_icon) lv_label_set_text(lbl_play_icon, LV_SYMBOL_PAUSE);
        }
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_next = lv_btn_create(panel_control);
    lv_obj_set_size(btn_next, 45, 45);
    lv_obj_align(btn_next, LV_ALIGN_RIGHT_MID, -10, 5);
    lv_obj_t * lbl_next = lv_label_create(btn_next);
    lv_label_set_text(lbl_next, LV_SYMBOL_NEXT); lv_obj_center(lbl_next);
    lv_obj_add_event_cb(btn_next, [](lv_event_t *e){
        if (!playlist.empty()) {
            int next_idx = (current_song_idx + 1) % playlist.size();
            play_song_by_index(next_idx);
        }
    }, LV_EVENT_CLICKED, NULL);

    slider_vol = lv_slider_create(scr_music);
    lv_obj_set_size(slider_vol, 150, 10);
    lv_obj_align(slider_vol, LV_ALIGN_TOP_MID, 15, 165);
    lv_slider_set_range(slider_vol, 0, 21);
    lv_slider_set_value(slider_vol, req_volume, LV_ANIM_OFF);
    
    lv_obj_t * lbl_vol = lv_label_create(scr_music);
    lv_label_set_text(lbl_vol, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(lbl_vol, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(lbl_vol, slider_vol, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    lv_obj_add_event_cb(slider_vol, [](lv_event_t *e){
        lv_obj_t * slider = lv_event_get_target(e);
        int vol = lv_slider_get_value(slider);
        req_volume = vol;
        req_volume_changed = true;
    }, LV_EVENT_VALUE_CHANGED, NULL);

    list_music = lv_list_create(scr_music);
    lv_obj_set_size(list_music, 230, 130);
    lv_obj_align(list_music, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(list_music, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(list_music, 0, 0);

    if (playlist.empty()) {
        lv_obj_t * lbl_empty = lv_label_create(list_music);
        lv_obj_add_style(lbl_empty, &style_cn, 0);
        lv_label_set_text(lbl_empty, "SD卡无 MP3 文件");
        lv_obj_set_style_text_color(lbl_empty, lv_color_hex(0x888888), 0);
        lv_obj_center(lbl_empty);
    } else {
        for (int i = 0; i < (int)playlist.size(); i++) {
            lv_obj_t * btn = lv_list_add_btn(list_music, LV_SYMBOL_AUDIO, playlist[i].c_str());
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x0f3460), 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x1a1a2e), 0);
            lv_obj_t * lbl = lv_obj_get_child(btn, 1);
            if (lbl) {
                lv_obj_add_style(lbl, &style_cn, 0);
                lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
            }
            lv_obj_add_event_cb(btn, [](lv_event_t *e){
                int target_idx = (int)(intptr_t)lv_event_get_user_data(e);
                play_song_by_index(target_idx);
            }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        }
    }

    music_ui_timer = lv_timer_create([](lv_timer_t *t){
        if (!lbl_play_icon) return;

        if (!is_playing) {
            if(strcmp(lv_label_get_text(lbl_play_icon), LV_SYMBOL_PAUSE) == 0) {
                lv_label_set_text(lbl_play_icon, LV_SYMBOL_PLAY);
                
                if (!playlist.empty() && current_song_idx >= 0) {
                    int next_idx = (current_song_idx + 1) % playlist.size();
                    play_song_by_index(next_idx);
                }
            }
        }
    }, 500, NULL);
}