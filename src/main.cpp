// ==================== 头文件 ====================
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>
#include "global.h"

// ==================== 硬件引脚定义 ====================
// 显示屏相关
#define TFT_BL 45        // TFT背光控制引脚

// I2C相关
#define I2C_SCL 15       // I2C时钟线
#define I2C_SDA 16       // I2C数据线
#define TP_RST  18       // 触摸屏复位引脚
#define FT6336_ADDR 0x38 // FT6336触摸芯片I2C地址
#define ES8311_ADDR 0x18 // ES8311音频解码芯片I2C地址

// 音频相关
#define I2S_BCLK 5       // I2S位时钟
#define I2S_LRCK 7       // I2S左右声道时钟
#define I2S_DOUT 6       // I2S数据输出
#define AMP_EN   1       // SC8002B功放使能引脚

// ==================== 全局变量定义 ====================
// 显示相关
TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t* buf1 = NULL;
static lv_color_t* buf2 = NULL;

// LVGL界面相关
lv_obj_t * scr_menu;
lv_obj_t * scr_tower;
lv_style_t style_cn;

// SD 卡互斥锁
SemaphoreHandle_t sd_mutex = NULL;

// ==================== 触摸屏相关函数 ====================
// 初始化触摸屏
void initTouch() {
    pinMode(TP_RST, OUTPUT);
    digitalWrite(TP_RST, LOW);
    delay(20);
    digitalWrite(TP_RST, HIGH);
    delay(50);
    
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setTimeout(100);
}

// 获取触摸坐标，返回是否检测到有效触摸
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

// ==================== 音频相关函数 ====================
// 初始化ES8311音频解码芯片
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

    // 1. 芯片复位
    writeReg(0x00, 0x1F);
    delay(10);
    writeReg(0x00, 0x00);

    // 2. 时钟与电源管理
    writeReg(0x01, 0x3F); // 开启系统所有时钟
    writeReg(0x0D, 0x01); // 启动时钟生成器
    writeReg(0x0B, 0x00); // 开启系统电源
    writeReg(0x12, 0x00); // 开启 DAC 电源
    writeReg(0x13, 0x10); // 开启 Vref
    writeReg(0x14, 0x1A); // 开启模拟电源
    writeReg(0x15, 0x06); // 使能左右声道输出

    // 3. I2S 接口配置 (从模式, 16-bit 飞利浦标准 I2S)
    writeReg(0x16, 0x00);
    writeReg(0x17, 0x00);

    // 4. 音量与输出路由
    writeReg(0x32, 0xBF); // 设置 DAC 硬件数字音量为 0dB (最大)
    writeReg(0x37, 0x08); // 将 DAC 信号路由到模拟输出
    
    Serial.println("ES8311 音频解码芯片配置完成!");
}

// ==================== LVGL显示相关函数 ====================
// LVGL显示刷新回调
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1), h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}

// LVGL触摸屏读取回调
void my_touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
    uint16_t tx, ty;
    if(getTouchPos(tx, ty)) {
        // 【增加钳制】防止飞点
        if(tx >= 240) tx = 239;
        if(ty >= 320) ty = 319;
        
        data->state = LV_INDEV_STATE_PR;
        data->point.x = tx;
        data->point.y = ty;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// ==================== 内存管理函数 ====================
// 分配显示缓冲区内存
bool allocateDisplayBuffers() {
    // 强制从内部SRAM中分配支持DMA的内存，避免PSRAM导致的SPI DMA访问问题
    buf1 = (lv_color_t*)heap_caps_malloc(240 * 160 * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!buf1) {
        Serial.println("FATAL ERROR: buf1 allocation failed!");
        return false;
    }
    
    buf2 = (lv_color_t*)heap_caps_malloc(240 * 160 * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!buf2) {
        Serial.println("FATAL ERROR: buf2 allocation failed!");
        heap_caps_free(buf1);
        buf1 = NULL;
        return false;
    }
    
    return true;
}

// ==================== 主程序 ====================
// Arduino初始化函数
void setup() {
    // 基础初始化
    Serial.begin(115200);
    srand(esp_random());

    // 文件系统初始化
    SD_MMC.setPins(38, 40, 39, 41, 48, 47);
    if (!SD_MMC.begin("/sdcard", true)) Serial.println("SD卡挂载失败!");

    if (!LittleFS.begin(true)) Serial.println("LittleFS 挂载失败!");

    // 音频功放初始化
    pinMode(AMP_EN, OUTPUT);
    digitalWrite(AMP_EN, LOW);

    // 显示缓冲区分配
    if (!allocateDisplayBuffers()) {
        Serial.println("System Halted due to memory allocation failure!");
        while(1) { delay(1000); }
    }
    
    // TFT显示屏初始化
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.invertDisplay(true);
    tft.setRotation(0);
    
    // 硬件初始化
    initTouch();
    initES8311();

    // LVGL初始化
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 240 * 160);

    // 注册LVGL显示驱动
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // 注册LVGL输入设备驱动
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // 中文字体样式初始化
    lv_style_init(&style_cn);
    lv_style_set_text_font(&style_cn, &my_font_cn_16);

    // 初始化 SD 卡互斥锁
    sd_mutex = xSemaphoreCreateMutex();
    if (sd_mutex == NULL) {
        Serial.println("FATAL ERROR: Failed to create SD mutex!");
        while(1) { delay(1000); }
    }
    
    // 构建并加载主菜单
    build_main_menu();
    lv_scr_load(scr_menu);
}

// Arduino主循环函数
void loop() {
    uint32_t time_till_next = lv_timer_handler();
    if(time_till_next < 5) time_till_next = 5;
    delay(time_till_next);
}