#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SD_MMC.h>
#include <esp_heap_caps.h>
#include "global.h"
#include <Preferences.h>

#define TFT_BL 45

#define I2C_SCL 15
#define I2C_SDA 16
#define TP_RST  18
#define FT6336_ADDR 0x38
#define ES8311_ADDR 0x18

#define I2S_BCLK 5
#define I2S_LRCK 7
#define I2S_DOUT 6
#define AMP_EN   1

TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t* buf1 = NULL;
static lv_color_t* buf2 = NULL;

lv_obj_t * scr_menu;
lv_obj_t * scr_tower;
lv_style_t style_cn;

SemaphoreHandle_t sd_mutex = NULL;

// ★ 新增：横屏模式开关。天气时钟「极简」样式开启，其余页面保持竖屏
volatile bool g_landscape_mode = false;

void initTouch() {
    pinMode(TP_RST, OUTPUT);
    digitalWrite(TP_RST, LOW);
    delay(20);
    digitalWrite(TP_RST, HIGH);
    delay(50);
    
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setTimeout(100);
}

bool getTouchPos(uint16_t &x, uint16_t &y) {
    Wire.beginTransmission(FT6336_ADDR);
    Wire.write(0x02);
    if (Wire.endTransmission() != 0) return false;
    
    Wire.requestFrom((uint8_t)FT6336_ADDR, (uint8_t)5);
    if (Wire.available() >= 5) {
        uint8_t pts = Wire.read() & 0x0F;
        uint8_t xh = Wire.read(), xl = Wire.read();
        uint8_t yh = Wire.read(), yl = Wire.read();
        if (pts > 0 && pts <= 2) {
            x = ((xh & 0x0F) << 8) | xl;
            y = ((yh & 0x0F) << 8) | yl;
            return true;
        }
    }
    return false;
}

void initES8311() {
    Wire.beginTransmission(ES8311_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("未检测到 ES8311 音频解码芯片!");
        return;
    }

    auto writeReg = [](uint8_t reg, uint8_t val) {
        Wire.beginTransmission(ES8311_ADDR);
        Wire.write(reg);
        Wire.write(val);
        Wire.endTransmission();
    };

    writeReg(0x00, 0x1F);
    delay(10);
    writeReg(0x00, 0x00);

    writeReg(0x01, 0x3F);
    writeReg(0x0D, 0x01);
    writeReg(0x0B, 0x00);
    writeReg(0x12, 0x00);
    writeReg(0x13, 0x10);
    writeReg(0x14, 0x1A);
    writeReg(0x15, 0x06);

    writeReg(0x16, 0x00);
    writeReg(0x17, 0x00);

    writeReg(0x32, 0xBF);
    writeReg(0x37, 0x08);
    
    Serial.println("ES8311 音频解码芯片配置完成!");
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1), h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
    uint16_t tx, ty;
    if(getTouchPos(tx, ty)) {
        if(tx >= 240) tx = 239;
        if(ty >= 320) ty = 319;
        
        if (g_landscape_mode) {
            // 物理 240×320 → 逻辑 320×240 (tft rotation=1)
            data->point.x = ty;                 // 0..319
            data->point.y = 239 - tx;           // 0..239
        } else {
            data->point.x = tx;
            data->point.y = ty;
        }
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

bool allocateDisplayBuffers() {
    size_t buf_size = 240 * 160 * sizeof(lv_color_t);

    buf1 = (lv_color_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
    buf2 = (lv_color_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
    if (!buf1 || !buf2) {
        Serial.println("FATAL ERROR: PSRAM buffer allocation failed!");
        if (buf1) heap_caps_free(buf1);
        if (buf2) heap_caps_free(buf2);
        return false;
    }
    Serial.printf("[Display] LVGL buffers allocated in PSRAM (%zu bytes each)\n", buf_size);
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    
    srand(esp_random());

    SD_MMC.setPins(38, 40, 39, 41, 48, 47);
    if (!SD_MMC.begin("/sdcard", true)) Serial.println("SD卡挂载失败!");

    pinMode(AMP_EN, OUTPUT);
    digitalWrite(AMP_EN, LOW);

    if (!allocateDisplayBuffers()) {
        Serial.println("System Halted due to memory allocation failure!");
        while(1) { delay(1000); }
    }
    
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.invertDisplay(true);
    tft.setRotation(0);
    
    initTouch();
    initES8311();

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 240 * 160);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    lv_style_init(&style_cn);
    lv_style_set_text_font(&style_cn, &my_font_cn_16);

    sd_mutex = xSemaphoreCreateMutex();
    if (sd_mutex == NULL) {
        Serial.println("FATAL ERROR: Failed to create SD mutex!");
        while(1) { delay(1000); }
    }
    
    build_main_menu();
    lv_scr_load(scr_menu);
    
    initWebLEDControl();
}

void loop() {
    uint32_t time_till_next = lv_timer_handler();
    if(time_till_next < 5) time_till_next = 5;
    delay(time_till_next);
    
    webLEDControlLoop();
}
