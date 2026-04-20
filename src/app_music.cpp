#include "global.h"
#include <SD_MMC.h>
#include "Audio.h"
#include <vector>
#include <String.h>
#include <cstring>
#include <strings.h>

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
            // 【修复】必须加锁，内部会关闭文件流
            if (xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                audio.stopSong();
                xSemaphoreGive(sd_mutex);
            } else {
                // 如果无法获取互斥锁，跳过本次循环，下个周期重试
                continue;
            }
            is_playing = false;
            req_stop = false;
        }
        
        if (req_play && req_song_idx >= 0 && req_song_idx < (int)playlist.size()) {
            char path[300]; // 【修复】：加大缓冲区，确保容纳 "/ + 256字符"
            snprintf(path, sizeof(path), "/%s", playlist[req_song_idx].c_str());
            
            // 【修复】切换歌曲时的 stopSong 也必须纳入锁保护
            if (xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                audio.stopSong();
                audio.connecttoFS(SD_MMC, path);
                xSemaphoreGive(sd_mutex);
                vTaskDelay(pdMS_TO_TICKS(2));
            } else {
                // 如果无法获取互斥锁，跳过本次循环，下个周期重试
                continue;
            }
            
            current_song_idx = req_song_idx;
            is_playing = true;
            req_play = false;
        }
        
        if (req_pause) {
            // 【修复】暂停/恢复也涉及到底层DMA和流控，加锁最安全
            if (xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                audio.pauseResume();
                is_playing = !is_playing; // 确保硬件操作成功后再翻转状态
                xSemaphoreGive(sd_mutex);
            } else {
                // 如果无法获取互斥锁，跳过本次循环，下个周期重试
                continue;
            }
            req_pause = false;
        }
        
        if (req_volume_changed) {
            audio.setVolume(req_volume);
            req_volume_changed = false;
        }
        
        if (audio.isRunning()) {
            // 使用互斥锁保护 SD 卡访问
            if (xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                audio.loop();
                xSemaphoreGive(sd_mutex);
                // 移除 taskYIELD(); 替换为：
                vTaskDelay(1); // 1ms 足够让其他等锁的任务（如电子书）切入，且保证音频流畅
            } else {
                vTaskDelay(pdMS_TO_TICKS(1)); // 获取不到锁时也要 Delay，防止死循环卡死 Core 0
                continue;
            }
        } else {
            if (is_playing) {
                is_playing = false;
            }
            vTaskDelay(pdMS_TO_TICKS(10)); // 没在播放时才休息
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
    // 使用互斥锁保护 SD 卡访问
    if (xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
        File root = SD_MMC.open("/");
        if (root) {
            File file = root.openNextFile();
            while (file) {
                const char* filename = file.name();
                // 提取扩展名并检查
                const char* dot = strrchr(filename, '.');
                if (!file.isDirectory() && dot && strcasecmp(dot, ".mp3") == 0) {
                    playlist.push_back(String(filename));
                }
                File next_file = root.openNextFile();
                file.close();
                file = next_file;
            }
            root.close();
        }
        xSemaphoreGive(sd_mutex);
    } else {
        // 如果无法获取互斥锁，返回以保护UI线程
        return;
    }
    
    playlist_initialized = true;
}

void build_music_scene() {
    init_playlist_once(); 
    if (audioTaskHandle == NULL) {
        xTaskCreatePinnedToCore(audio_task, "AudioTask", 16384, NULL, 2, &audioTaskHandle, 0);
    }
    if (scr_music != NULL) {
        lv_obj_del_async(scr_music);
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
        req_stop = true;
        if (music_ui_timer) {
            lv_timer_del(music_ui_timer);
            music_ui_timer = NULL;
        }
        // 清理内存并立即断开所有全局 UI 指针
        lv_scr_load(scr_menu);
        lv_obj_del_async(scr_music);
        scr_music = NULL;
        list_music = NULL;
        lbl_song_title = NULL;
        btn_play = NULL;
        lbl_play_icon = NULL;
        slider_vol = NULL;
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
    
    // 在 build_music_scene() 结尾处添加：
    if (music_ui_timer) lv_timer_resume(music_ui_timer); // ✅ 进入时恢复定时器
}