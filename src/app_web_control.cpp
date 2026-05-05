#include "global.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_task_wdt.h>
#include <Adafruit_NeoPixel.h>
#include <SD_MMC.h>
#include <FS.h>

// ==================== WiFi配置 ====================
#define WIFI_SSID "wwd2"
#define WIFI_PASSWORD "12345687"
#define WIFI_AP_IP IPAddress(192, 168, 4, 1)
#define WIFI_GATEWAY IPAddress(192, 168, 4, 1)
#define WIFI_SUBNET IPAddress(255, 255, 255, 0)

// ==================== LED硬件配置 ====================
#define LED_PIN 42
#define NUM_PIXELS 1

// ==================== 外部变量引用 (来自app_led.cpp) ====================
extern volatile bool led_is_on;
extern volatile uint8_t led_r, led_g, led_b;
extern volatile int led_mode;
extern volatile uint32_t led_auto_off_time;
extern volatile uint16_t custom_on_ms, custom_off_ms;
extern TaskHandle_t ledTaskHandle;

// ==================== 声明LED任务函数 (来自app_led.cpp) ====================
extern void led_task(void *pvParameters);

// ==================== 魔塔游戏变量引用 (来自game_tower.cpp) ====================
struct HeroStats { int hp, atk, def, gold, exp, keys_y, keys_b, keys_r, x, y; };
extern HeroStats hero;
extern int current_floor;

// ==================== 2048游戏变量引用 (来自game_2048.cpp) ====================
extern int score_2048;
extern int best_score_2048;

// ==================== 游戏控制模式变量 ====================
// 游戏控制模式: 0=无/LED控制, 1=LED控制, 2=魔塔, 3=2048
volatile uint8_t web_game_mode = 0;
volatile bool web_control_active = false;

// ==================== 全局对象 ====================
static WebServer server(80);
static DNSServer dnsServer;
static bool webControlInitialized = false;
static bool wifiConnected = false;

// 捕获最近的控制来源，用于调试
static bool lastControlFromWeb = false;

// ==================== HTML页面 ====================
static const char MAIN_PAGE[] PROGMEM = R"rawlcd(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>RGB LED 控制器</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
body {
  font-family: -apple-system, 'Helvetica Neue', Arial, sans-serif;
  background: linear-gradient(135deg, #0a0a1a 0%, #1a1a3e 50%, #0a0a1a 100%);
  color: #e0e0e0;
  min-height: 100vh;
  padding: 16px;
  overflow-x: hidden;
}
.header {
  text-align: center;
  padding: 20px 0 16px;
  font-size: 22px;
  font-weight: 700;
  background: linear-gradient(90deg, #ff6b6b, #51cf66, #339af0, #cc5de8);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
  letter-spacing: 2px;
}
.status-bar {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12px;
  margin-bottom: 20px;
  padding: 12px;
  background: rgba(255,255,255,0.05);
  border-radius: 16px;
  backdrop-filter: blur(10px);
}
.power-btn {
  width: 60px; height: 60px;
  border-radius: 50%;
  border: 3px solid #555;
  background: #2a2a2a;
  color: #666;
  font-size: 28px;
  cursor: pointer;
  transition: all 0.3s;
  display: flex;
  align-items: center;
  justify-content: center;
}
.power-btn.on {
  border-color: #51cf66;
  background: rgba(81,207,102,0.2);
  color: #51cf66;
  box-shadow: 0 0 20px rgba(81,207,102,0.4);
}
.color-preview {
  width: 60px; height: 60px;
  border-radius: 50%;
  border: 2px solid rgba(255,255,255,0.3);
  transition: all 0.3s;
  box-shadow: 0 0 15px rgba(0,0,0,0.5);
}
.section {
  background: rgba(255,255,255,0.05);
  border-radius: 20px;
  padding: 16px;
  margin-bottom: 16px;
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255,255,255,0.08);
}
.section-title {
  font-size: 15px;
  font-weight: 600;
  color: #888;
  margin-bottom: 12px;
  text-transform: uppercase;
  letter-spacing: 1px;
}
.rgb-control {
  display: flex;
  flex-direction: column;
  gap: 12px;
}
.rgb-row {
  display: flex;
  align-items: center;
  gap: 12px;
}
.rgb-label {
  width: 20px;
  font-weight: 700;
  font-size: 16px;
}
.rgb-label.r { color: #ff6b6b; }
.rgb-label.g { color: #51cf66; }
.rgb-label.b { color: #339af0; }
.rgb-slider {
  flex: 1;
  -webkit-appearance: none;
  appearance: none;
  height: 8px;
  border-radius: 4px;
  outline: none;
  cursor: pointer;
}
.rgb-slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 24px; height: 24px;
  border-radius: 50%;
  cursor: pointer;
  border: 2px solid rgba(255,255,255,0.5);
  box-shadow: 0 2px 6px rgba(0,0,0,0.3);
}
.rgb-slider.r { background: linear-gradient(90deg, #333, #ff6b6b); }
.rgb-slider.r::-webkit-slider-thumb { background: #ff6b6b; }
.rgb-slider.g { background: linear-gradient(90deg, #333, #51cf66); }
.rgb-slider.g::-webkit-slider-thumb { background: #51cf66; }
.rgb-slider.b { background: linear-gradient(90deg, #333, #339af0); }
.rgb-slider.b::-webkit-slider-thumb { background: #339af0; }
.rgb-value {
  width: 40px;
  text-align: right;
  font-size: 14px;
  color: #aaa;
  font-family: monospace;
}
.preset-colors {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
  justify-content: center;
  padding: 8px 0;
}
.preset-btn {
  width: 44px; height: 44px;
  border-radius: 50%;
  border: 2px solid rgba(255,255,255,0.2);
  cursor: pointer;
  transition: all 0.2s;
  position: relative;
}
.preset-btn:active {
  transform: scale(0.9);
}
.preset-btn.active {
  border-color: #fff;
  box-shadow: 0 0 12px rgba(255,255,255,0.4);
}
.mode-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 8px;
}
.mode-btn {
  padding: 12px 8px;
  border-radius: 12px;
  border: 2px solid rgba(255,255,255,0.1);
  background: rgba(255,255,255,0.03);
  color: #ccc;
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
  text-align: center;
}
.mode-btn:active { transform: scale(0.95); }
.mode-btn.active {
  border-color: #cc5de8;
  background: rgba(204,93,232,0.15);
  color: #fff;
  box-shadow: 0 0 10px rgba(204,93,232,0.3);
}
.timer-section {
  display: flex;
  flex-direction: column;
  gap: 10px;
}
.timer-options {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}
.timer-btn {
  padding: 10px 16px;
  border-radius: 10px;
  border: 2px solid rgba(255,255,255,0.1);
  background: rgba(255,255,255,0.03);
  color: #ccc;
  font-size: 13px;
  cursor: pointer;
  transition: all 0.2s;
}
.timer-btn:active { transform: scale(0.95); }
.timer-btn.active {
  border-color: #ffd43b;
  background: rgba(255,212,59,0.15);
  color: #fff;
}
.custom-time {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-top: 8px;
}
.custom-time input {
  width: 80px;
  padding: 8px 12px;
  border-radius: 8px;
  border: 1px solid rgba(255,255,255,0.2);
  background: rgba(0,0,0,0.3);
  color: #fff;
  font-size: 14px;
  text-align: center;
}
.custom-time label {
  color: #888;
  font-size: 13px;
}
.custom-flash {
  display: flex;
  flex-direction: column;
  gap: 12px;
}
.flash-row {
  display: flex;
  align-items: center;
  gap: 12px;
}
.flash-row label {
  width: 50px;
  font-size: 13px;
  color: #888;
}
.flash-row input[type="range"] {
  flex: 1;
  -webkit-appearance: none;
  height: 6px;
  border-radius: 3px;
  background: #333;
  outline: none;
}
.flash-row input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 20px; height: 20px;
  border-radius: 50%;
  background: #cc5de8;
  cursor: pointer;
}
.flash-row .val {
  width: 55px;
  text-align: right;
  font-size: 12px;
  color: #888;
  font-family: monospace;
}
</style>
</head>
<body>
<div class="header">✦ RGB LED 氛围灯 ✦</div>

<div class="status-bar">
  <button class="power-btn" id="powerBtn" onclick="togglePower()">
    <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
      <path d="M18.36 6.64a9 9 0 1 1-12.73 0"/>
      <line x1="12" y1="2" x2="12" y2="12"/>
    </svg>
  </button>
  <div class="color-preview" id="colorPreview"></div>
  <div style="flex:1">
    <div style="text-align:center;font-size:14px;color:#888" id="statusText">LED 已关闭</div>
  </div>
</div>

<div class="section">
  <div class="section-title">🎨 颜色调节</div>
  <div class="rgb-control">
    <div class="rgb-row">
      <span class="rgb-label r">R</span>
      <input type="range" class="rgb-slider r" id="sliderR" min="0" max="255" value="255" oninput="updateRGB()">
      <span class="rgb-value" id="valR">255</span>
    </div>
    <div class="rgb-row">
      <span class="rgb-label g">G</span>
      <input type="range" class="rgb-slider g" id="sliderG" min="0" max="255" value="255" oninput="updateRGB()">
      <span class="rgb-value" id="valG">255</span>
    </div>
    <div class="rgb-row">
      <span class="rgb-label b">B</span>
      <input type="range" class="rgb-slider b" id="sliderB" min="0" max="255" value="255" oninput="updateRGB()">
      <span class="rgb-value" id="valB">255</span>
    </div>
  </div>
</div>

<div class="section">
  <div class="section-title">💡 预设颜色</div>
  <div class="preset-colors" id="presetColors"></div>
</div>

<div class="section">
  <div class="section-title">🎭 效果模式</div>
  <div class="mode-grid" id="modeGrid"></div>
</div>

<div class="section">
  <div class="section-title">⏰ 定时关闭</div>
  <div class="timer-section">
    <div class="timer-options" id="timerOptions"></div>
    <div class="custom-time">
      <input type="number" id="customMin" min="1" max="999" value="1" placeholder="分钟">
      <label>自定义分钟</label>
      <button class="timer-btn" onclick="setCustomTimer()" style="padding:8px 16px">设置</button>
    </div>
  </div>
</div>

<div class="section">
  <div class="section-title">⚡ 自定义闪烁</div>
  <div class="custom-flash">
    <div class="flash-row">
      <label>亮起</label>
      <input type="range" id="onTime" min="0" max="20000" value="500" oninput="updateFlash()">
      <span class="val" id="onTimeVal">500ms</span>
    </div>
    <div class="flash-row">
      <label>熄灭</label>
      <input type="range" id="offTime" min="0" max="20000" value="500" oninput="updateFlash()">
      <span class="val" id="offTimeVal">500ms</span>
    </div>
  </div>
</div>

<!-- 天气时钟配网入口 -->
<div class="section" style="margin-top:20px;border:2px dashed rgba(255,212,59,0.3)">
  <div class="section-title" style="color:#ffd43b">🌤 天气时钟</div>
  <div style="text-align:center;padding:8px">
    <a href="/weather" style="padding:12px 24px;border-radius:10px;border:2px solid #ffd43b;background:rgba(255,212,59,0.1);color:#ffd43b;font-size:14px;text-decoration:none;display:inline-block">WiFi配置</a>
  </div>
</div>

<!-- 游戏选择入口 -->
<div class="section" style="margin-top:20px;border:2px dashed rgba(81,207,102,0.3)">
  <div class="section-title" style="color:#51cf66">🎮 网页游戏控制</div>
  <div style="text-align:center;padding:8px">
    <button onclick="showGameMenu()" style="padding:12px 24px;border-radius:10px;border:2px solid #51cf66;background:rgba(81,207,102,0.1);color:#51cf66;font-size:14px;cursor:pointer">进入游戏控制</button>
  </div>
</div>

<!-- 游戏选择菜单 -->
<div id="gameMenu" style="display:none">
  <div class="section">
    <div class="section-title">选择游戏</div>
    <div style="display:grid;grid-template-columns:repeat(3,1fr);gap:10px">
      <button onclick="startGame(2)" style="padding:16px 8px;border-radius:12px;border:2px solid rgba(255,255,255,0.15);background:rgba(255,255,255,0.05);color:#ccc;font-size:13px;cursor:pointer">🗼<br>魔塔</button>
      <button onclick="startGame(3)" style="padding:16px 8px;border-radius:12px;border:2px solid rgba(255,255,255,0.15);background:rgba(255,255,255,0.05);color:#ccc;font-size:13px;cursor:pointer">🎲<br>2048</button>
    </div>
  </div>
  <div class="section">
    <button onclick="hideGameMenu()" style="width:100%;padding:12px;border-radius:10px;border:2px solid rgba(255,100,100,0.3);background:rgba(255,100,100,0.1);color:#ff6b6b;font-size:14px;cursor:pointer">← 返回</button>
  </div>
</div>

<!-- 魔塔游戏控制 -->
<div id="towerControl" style="display:none">
  <div class="section">
    <div class="section-title">📊 角色状态</div>
    <div id="towerInfo" style="background:rgba(0,0,0,0.3);border-radius:10px;padding:10px;font-size:12px;line-height:1.6;color:#aaa;font-family:monospace">加载中...</div>
  </div>
  <div class="section">
    <div class="section-title">🎮 方向控制</div>
    <div style="display:grid;grid-template-columns:repeat(3,1fr);gap:8px;max-width:180px;margin:0 auto">
      <div></div>
      <button onclick="towerMove(0,-1)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(255,255,255,0.2);background:rgba(255,255,255,0.05);color:#fff;font-size:18px;cursor:pointer">▲</button>
      <div></div>
      <button onclick="towerMove(-1,0)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(255,255,255,0.2);background:rgba(255,255,255,0.05);color:#fff;font-size:18px;cursor:pointer">◀</button>
      <button onclick="towerMove(0,1)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(81,207,102,0.3);background:rgba(81,207,102,0.1);color:#51cf66;font-size:18px;cursor:pointer">▼</button>
      <button onclick="towerMove(1,0)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(255,255,255,0.2);background:rgba(255,255,255,0.05);color:#fff;font-size:18px;cursor:pointer">▶</button>
    </div>
  </div>
  <div class="section">
    <button onclick="exitGameControl()" style="width:100%;padding:14px;border-radius:10px;border:2px solid rgba(255,100,100,0.3);background:rgba(255,100,100,0.1);color:#ff6b6b;font-size:15px;cursor:pointer">🚪 退出游戏控制</button>
  </div>
</div>

<!-- 2048游戏控制 -->
<div id="game2048Control" style="display:none">
  <div class="section">
    <div class="section-title">📊 2048 状态</div>
    <div id="game2048Info" style="background:rgba(0,0,0,0.3);border-radius:10px;padding:10px;font-size:14px;line-height:1.6;color:#aaa;text-align:center;font-family:monospace">加载中...</div>
  </div>
  <div class="section">
    <div class="section-title">🎮 方向控制</div>
    <div style="display:grid;grid-template-columns:repeat(3,1fr);gap:8px;max-width:180px;margin:0 auto">
      <div></div>
      <button onclick="game2048Move(0,-1)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(255,255,255,0.2);background:rgba(255,255,255,0.05);color:#fff;font-size:18px;cursor:pointer">▲</button>
      <div></div>
      <button onclick="game2048Move(-1,0)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(255,255,255,0.2);background:rgba(255,255,255,0.05);color:#fff;font-size:18px;cursor:pointer">◀</button>
      <button onclick="game2048Move(0,1)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(81,207,102,0.3);background:rgba(81,207,102,0.1);color:#51cf66;font-size:18px;cursor:pointer">▼</button>
      <button onclick="game2048Move(1,0)" style="width:50px;height:50px;border-radius:10px;border:2px solid rgba(255,255,255,0.2);background:rgba(255,255,255,0.05);color:#fff;font-size:18px;cursor:pointer">▶</button>
    </div>
  </div>
  <div class="section">
    <button onclick="exitGameControl()" style="width:100%;padding:14px;border-radius:10px;border:2px solid rgba(255,100,100,0.3);background:rgba(255,100,100,0.1);color:#ff6b6b;font-size:15px;cursor:pointer">🚪 退出游戏控制</button>
  </div>
</div>

<script>
const PRESETS = [
  {c:'#FF0000',r:255,g:0,b:0}, {c:'#FF7F00',r:255,g:127,b:0},
  {c:'#FFFF00',r:255,g:255,b:0}, {c:'#00FF00',r:0,g:255,b:0},
  {c:'#00FFFF',r:0,g:255,b:255}, {c:'#0000FF',r:0,g:0,b:255},
  {c:'#8B00FF',r:139,g:0,b:255}, {c:'#FFFFFF',r:255,g:255,b:255}
];
const MODES = ['常亮','慢闪','警灯','SOS','七彩','自定义','呼吸','彩呼吸'];
const TIMERS = [
  {t:'不限',v:0}, {t:'1分钟',v:1}, {t:'5分钟',v:5},
  {t:'10分钟',v:10}, {t:'30分钟',v:30}, {t:'60分钟',v:60}
];

let currentMode = 0;
let currentTimer = 0;
let ledOn = false;

// 初始化预设颜色按钮
const presetContainer = document.getElementById('presetColors');
PRESETS.forEach((p, i) => {
  const btn = document.createElement('button');
  btn.className = 'preset-btn';
  btn.style.background = p.c;
  btn.style.boxShadow = '0 0 8px ' + p.c;
  btn.onclick = () => setPreset(i);
  presetContainer.appendChild(btn);
});

// 初始化模式按钮
const modeContainer = document.getElementById('modeGrid');
MODES.forEach((m, i) => {
  const btn = document.createElement('button');
  btn.className = 'mode-btn' + (i === 0 ? ' active' : '');
  btn.textContent = m;
  btn.onclick = () => setMode(i);
  modeContainer.appendChild(btn);
});

// 初始化定时器按钮
const timerContainer = document.getElementById('timerOptions');
TIMERS.forEach((t, i) => {
  const btn = document.createElement('button');
  btn.className = 'timer-btn' + (i === 0 ? ' active' : '');
  btn.textContent = t.t;
  btn.onclick = () => setTimer(i);
  timerContainer.appendChild(btn);
});

function updatePreview() {
  const r = document.getElementById('sliderR').value;
  const g = document.getElementById('sliderG').value;
  const b = document.getElementById('sliderB').value;
  const preview = document.getElementById('colorPreview');
  preview.style.background = 'rgb(' + r + ',' + g + ',' + b + ')';
  preview.style.boxShadow = '0 0 20px rgb(' + r + ',' + g + ',' + b + ')';
}

function updateStatus() {
  const statusText = document.getElementById('statusText');
  const powerBtn = document.getElementById('powerBtn');
  if (ledOn) {
    statusText.textContent = 'LED 运行中 | ' + MODES[currentMode];
    powerBtn.className = 'power-btn on';
  } else {
    statusText.textContent = 'LED 已关闭';
    powerBtn.className = 'power-btn';
  }
}

function sendCommand(path, params) {
  const url = '/cmd' + path + '?' + new URLSearchParams(params).toString();
  fetch(url).then(r => r.json()).then(data => {
    if (data.sync) {
      // 同步返回的当前状态
      if (typeof data.r === 'number') {
        document.getElementById('sliderR').value = data.r;
        document.getElementById('valR').textContent = data.r;
      }
      if (typeof data.g === 'number') {
        document.getElementById('sliderG').value = data.g;
        document.getElementById('valG').textContent = data.g;
      }
      if (typeof data.b === 'number') {
        document.getElementById('sliderB').value = data.b;
        document.getElementById('valB').textContent = data.b;
      }
      if (typeof data.mode === 'number') {
        currentMode = data.mode;
        updateModeButtons();
      }
      if (typeof data.on === 'boolean') {
        ledOn = data.on;
        updateStatus();
      }
      if (typeof data.onTime === 'number') {
        document.getElementById('onTime').value = data.onTime;
        document.getElementById('onTimeVal').textContent = data.onTime + 'ms';
      }
      if (typeof data.offTime === 'number') {
        document.getElementById('offTime').value = data.offTime;
        document.getElementById('offTimeVal').textContent = data.offTime + 'ms';
      }
      updatePreview();
    }
  }).catch(e => console.error(e));
}

function togglePower() {
  ledOn = !ledOn;
  sendCommand('/power', {on: ledOn ? '1' : '0'});
  updateStatus();
}

function updateRGB() {
  const r = document.getElementById('sliderR').value;
  const g = document.getElementById('sliderG').value;
  const b = document.getElementById('sliderB').value;
  document.getElementById('valR').textContent = r;
  document.getElementById('valG').textContent = g;
  document.getElementById('valB').textContent = b;
  updatePreview();
  currentMode = 0;
  updateModeButtons();
  sendCommand('/rgb', {r: r, g: g, b: b});
  if (!ledOn) { ledOn = true; updateStatus(); }
}

function setPreset(idx) {
  const p = PRESETS[idx];
  document.getElementById('sliderR').value = p.r;
  document.getElementById('sliderG').value = p.g;
  document.getElementById('sliderB').value = p.b;
  document.getElementById('valR').textContent = p.r;
  document.getElementById('valG').textContent = p.g;
  document.getElementById('valB').textContent = p.b;
  currentMode = 0;
  updateModeButtons();
  updatePreview();
  sendCommand('/rgb', {r: p.r, g: p.g, b: p.b});
  if (!ledOn) { ledOn = true; updateStatus(); }
  // 高亮选中
  document.querySelectorAll('.preset-btn').forEach((b, i) => {
    b.classList.toggle('active', i === idx);
  });
}

function setMode(idx) {
  currentMode = idx;
  updateModeButtons();
  sendCommand('/mode', {m: idx});
  if (!ledOn) { ledOn = true; updateStatus(); }
}

function updateModeButtons() {
  document.querySelectorAll('.mode-btn').forEach((b, i) => {
    b.classList.toggle('active', i === currentMode);
  });
}

function setTimer(idx) {
  currentTimer = TIMERS[idx].v;
  document.querySelectorAll('.timer-btn').forEach((b, i) => {
    b.classList.toggle('active', i === idx);
  });
  sendCommand('/timer', {min: TIMERS[idx].v});
}

function setCustomTimer() {
  const mins = parseInt(document.getElementById('customMin').value) || 1;
  sendCommand('/timer', {min: mins});
  document.querySelectorAll('.timer-btn').forEach(b => b.classList.remove('active'));
}

function updateFlash() {
  const onT = document.getElementById('onTime').value;
  const offT = document.getElementById('offTime').value;
  document.getElementById('onTimeVal').textContent = onT + 'ms';
  document.getElementById('offTimeVal').textContent = offT + 'ms';
  sendCommand('/flash', {on: onT, off: offT});
}

// ==================== 游戏控制函数 ====================
let currentGameMode = 0;

function showGameMenu() {
  document.getElementById('gameMenu').style.display = 'block';
}

function hideGameMenu() {
  document.getElementById('gameMenu').style.display = 'none';
}

function startGame(mode) {
  currentGameMode = mode;
  fetch('/cmd/game?mode=' + mode).then(r => r.json()).then(data => {
    if (data.ok) {
      hideGameMenu();
      if (mode === 2) {
        // 魔塔 - 显示方向控制
        document.getElementById('towerControl').style.display = 'block';
        updateTowerInfo();
      } else if (mode === 3) {
        // 2048 - 显示方向控制
        document.getElementById('game2048Control').style.display = 'block';
        update2048Info();
      }
    }
  }).catch(e => console.error(e));
}

function towerMove(dx, dy) {
  fetch('/cmd/tower?dx=' + dx + '&dy=' + dy).then(r => r.json()).then(data => {
    if (data.floor !== undefined) {
      updateTowerInfoDisplay(data);
    }
  }).catch(e => console.error(e));
}

function updateTowerInfo() {
  fetch('/cmd/tower?action=status').then(r => r.json()).then(data => {
    if (data.floor !== undefined) {
      updateTowerInfoDisplay(data);
    }
  }).catch(e => console.error(e));
}

function updateTowerInfoDisplay(data) {
  const info = document.getElementById('towerInfo');
  if (info) {
    info.innerHTML = '楼层:' + data.floor + ' 金钱:' + data.gold + ' 经验:' + data.exp + '<br>' +
      '生命:' + data.hp + ' 攻击:' + data.atk + ' 防御:' + data.def + '<br>' +
      '钥匙:' + data.keys_y + '黄 ' + data.keys_b + '蓝 ' + data.keys_r + '红';
  }
}

function exitGameControl() {
  fetch('/cmd/game?mode=0').then(r => r.json()).then(data => {
    currentGameMode = 0;
    document.getElementById('towerControl').style.display = 'none';
    document.getElementById('game2048Control').style.display = 'none';
    document.getElementById('gameMenu').style.display = 'none';
  }).catch(e => console.error(e));
}

function game2048Move(dx, dy) {
  fetch('/cmd/2048?dx=' + dx + '&dy=' + dy).then(r => r.json()).then(data => {
    if (data.score !== undefined) {
      update2048InfoDisplay(data);
    }
  }).catch(e => console.error(e));
}

function update2048Info() {
  fetch('/cmd/2048?action=status').then(r => r.json()).then(data => {
    if (data.score !== undefined) {
      update2048InfoDisplay(data);
    }
  }).catch(e => console.error(e));
}

function update2048InfoDisplay(data) {
  const info = document.getElementById('game2048Info');
  if (info) {
    info.innerHTML = '<span style="color:#fff;font-size:18px">当前分数: ' + data.score + '</span><br>' +
                     '<span style="color:#edc22e;font-size:16px">最高分: ' + data.best_score + '</span>';
  }
}

// 定时同步游戏状态
setInterval(() => {
  if (currentGameMode === 2) {
    updateTowerInfo();
  } else if (currentGameMode === 3) {
    update2048Info();
  }
}, 1500);

// 定时同步状态
setInterval(() => {
  fetch('/status').then(r => r.json()).then(data => {
    document.getElementById('sliderR').value = data.r;
    document.getElementById('sliderG').value = data.g;
    document.getElementById('sliderB').value = data.b;
    document.getElementById('valR').textContent = data.r;
    document.getElementById('valG').textContent = data.g;
    document.getElementById('valB').textContent = data.b;
    currentMode = data.mode;
    updateModeButtons();
    document.getElementById('onTime').value = data.onTime;
    document.getElementById('offTime').value = data.offTime;
    document.getElementById('onTimeVal').textContent = data.onTime + 'ms';
    document.getElementById('offTimeVal').textContent = data.offTime + 'ms';
    if (data.on !== ledOn) {
      ledOn = data.on;
      updateStatus();
    }
    updatePreview();
  }).catch(e => {});
}, 2000);

// 初始化
updatePreview();
updateStatus();
</script>
</body>
</html>
)rawlcd";

// ==================== WiFi初始化 ====================
static void initWiFiAP() {
  Serial.println("[WiFi] 启动AP模式...");
  
  // 【关键修复】确保模式为 AP+STA
  WiFi.mode(WIFI_AP_STA);
  
  // 【改进】只断开 STA 连接，不清除 NVS 中保存的 WiFi 信息
  // 使用 false, false 避免擦除存储的凭证，防止与天气时钟的 STA 重连流程冲突
  WiFi.disconnect(false, false);
  delay(100);
  
  // 配置AP，强制指定信道为 6（避免默认信道跳变）
  WiFi.softAPConfig(WIFI_AP_IP, WIFI_GATEWAY, WIFI_SUBNET);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, 6, 0, 4);
  
  delay(200);
  
  Serial.print("[WiFi] AP已启动: ");
  Serial.println(WIFI_SSID);
  Serial.print("[WiFi] IP地址: ");
  Serial.println(WiFi.softAPIP());
  
  // 启动DNS重定向
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", WIFI_AP_IP);
  
  wifiConnected = true;
}

// ==================== Captive Portal 检测响应 ====================
// 这些是各大厂商用来检测是否需要登录门户的URL
static const char CAPTIVE_PORTAL_RESPONSE[] PROGMEM = R"raw(
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8

<html><head><meta http-equiv="refresh" content="0;url=http://192.168.4.1"></head>
<body><a href="http://192.168.4.1">点击连接 LED 控制器</a></body>
</html>
)raw";

static void handleCaptivePortal() {
  // Google (Android)
  if (server.uri() == "/generate_204") {
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(200, "text/plain", "OK");
    return;
  }
  // Apple (iOS/macOS)
  if (server.uri() == "/hotspot-detect.html") {
    server.send(200, "text/html", CAPTIVE_PORTAL_RESPONSE);
    return;
  }
  // Microsoft (Windows)
  if (server.uri() == "/connecttest.txt") {
    server.send(200, "text/plain", "true");
    return;
  }
  // Canon
  if (server.uri() == "/canonical.html") {
    server.send(200, "text/html", CAPTIVE_PORTAL_RESPONSE);
    return;
  }
  // 其他未知请求也重定向到主页
  server.send(200, "text/html", CAPTIVE_PORTAL_RESPONSE);
}

// ==================== HTTP处理函数 ====================
static void handleRoot() {
  server.send(200, "text/html", MAIN_PAGE);
}

static void handleStatus() {
  String json = "{";
  json += "\"on\":" + String(led_is_on ? "true" : "false");
  json += ",\"r\":" + String(led_r);
  json += ",\"g\":" + String(led_g);
  json += ",\"b\":" + String(led_b);
  json += ",\"mode\":" + String(led_mode);
  json += ",\"onTime\":" + String(custom_on_ms);
  json += ",\"offTime\":" + String(custom_off_ms);
  json += "}";
  server.send(200, "application/json", json);
}

static void handleCommand() {
  String path = server.uri();
  
  // 读取参数
  String paramOn = server.arg("on");
  String paramR = server.arg("r");
  String paramG = server.arg("g");
  String paramB = server.arg("b");
  String paramM = server.arg("m");
  String paramMin = server.arg("min");
  String paramFlashOn = server.arg("on");
  String paramFlashOff = server.arg("off");
  
  if (path == "/cmd/power" && paramOn.length() > 0) {
    led_is_on = (paramOn == "1");
    lastControlFromWeb = true;
  }
  else if (path == "/cmd/rgb" && paramR.length() > 0) {
    led_r = constrain(paramR.toInt(), 0, 255);
    led_g = constrain(paramG.toInt(), 0, 255);
    led_b = constrain(paramB.toInt(), 0, 255);
    led_mode = 0;  // RGB调节自动切换到常亮模式
    if (!led_is_on) led_is_on = true;
    lastControlFromWeb = true;
  }
  else if (path == "/cmd/mode" && paramM.length() > 0) {
    led_mode = constrain(paramM.toInt(), 0, 7);
    if (!led_is_on) led_is_on = true;
    lastControlFromWeb = true;
  }
  else if (path == "/cmd/timer" && paramMin.length() > 0) {
    int mins = paramMin.toInt();
    if (mins <= 0) {
      led_auto_off_time = 0;
    } else {
      led_auto_off_time = millis() + (uint32_t)(mins * 60000UL);
    }
    lastControlFromWeb = true;
  }
  else if (path == "/cmd/flash") {
    if (paramFlashOn.length() > 0 && paramFlashOff.length() > 0) {
      custom_on_ms = constrain(paramFlashOn.toInt(), 0, 20000);
      custom_off_ms = constrain(paramFlashOff.toInt(), 0, 20000);
    }
    lastControlFromWeb = true;
  }
 
  // 返回当前状态用于同步
  String json = "{";
  json += "\"r\":" + String(led_r);
  json += ",\"g\":" + String(led_g);
  json += ",\"b\":" + String(led_b);
  json += ",\"mode\":" + String(led_mode);
  json += ",\"on\":" + String(led_is_on ? "true" : "false");
  json += ",\"onTime\":" + String(custom_on_ms);
  json += ",\"offTime\":" + String(custom_off_ms);
  json += ",\"sync\":true";
  json += "}";
  server.send(200, "application/json", json);
}

// ==================== 游戏控制处理 ====================
static void handleGameCommand() {
  String paramMode = server.arg("mode");
  
  if (paramMode.length() > 0) {
    uint8_t mode = paramMode.toInt();
    
    if (mode == 0) {
      // 退出游戏控制 - 同步保存并返回主页
      web_game_mode = 0;
      web_control_active = false;
      
      // 保存当前游戏状态
      if (is_tower_started) {
        save_tower_game();
        is_tower_started = false;
        Serial.println("[WebGame] 魔塔游戏已保存");
      }
      if (is_2048_started) {
        save_2048_game();
        is_2048_started = false;
        Serial.println("[WebGame] 2048游戏已保存");
      }
      
      // 返回主页（同步切换屏幕）
      if (scr_menu != NULL) {
        lv_scr_load(scr_menu);
      }
      // 删除游戏屏幕
      if (scr_tower != NULL) {
        lv_obj_del_async(scr_tower);
        scr_tower = NULL;
      }
      if (scr_2048 != NULL) {
        lv_obj_del_async(scr_2048);
        scr_2048 = NULL;
      }
      
      Serial.println("[WebGame] 退出网页游戏控制，已返回主页");
    } else if (mode == 2) {
      // 进入魔塔游戏控制模式 - 同步打开魔塔游戏
      web_game_mode = mode;
      web_control_active = true;
      
      // 构建魔塔场景（如未构建）
      if (scr_tower == NULL) {
        build_tower_scene();
      }
      
      // 加载或重置游戏
      if (!is_tower_started) {
        if (!load_tower_game()) {
          reset_tower_game();
        }
      }
      
      // 切换到魔塔屏幕
      lv_scr_load_anim(scr_tower, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
      
      Serial.print("[WebGame] 进入魔塔游戏控制模式: ");
      Serial.println(mode);
    } else if (mode == 3) {
      // 进入2048游戏控制模式 - 同步打开2048游戏
      web_game_mode = mode;
      web_control_active = true;
      
      // 构建2048场景（如未构建）
      if (scr_2048 == NULL) {
        build_2048_scene();
      }
      
      // 加载或重置游戏
      if (!is_2048_started) {
        if (!load_2048_game()) {
          reset_2048_game();
        }
      }
      
      // 切换到2048屏幕
      lv_scr_load_anim(scr_2048, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
      
      Serial.print("[WebGame] 进入2048游戏控制模式: ");
      Serial.println(mode);
    }
  }
  
  server.send(200, "application/json", "{\"ok\":true}");
}

static void handleTowerCommand() {
  String action = server.arg("action");
  String paramDx = server.arg("dx");
  String paramDy = server.arg("dy");
  
  // 如果不是魔塔模式，返回错误
  if (web_game_mode != 2) {
    server.send(200, "application/json", "{\"error\":\"not_in_tower_mode\"}");
    return;
  }
  
  // 处理移动
  if (paramDx.length() > 0 && paramDy.length() > 0) {
    int dx = paramDx.toInt();
    int dy = paramDy.toInt();
    tower_move(dx, dy);
  }
  
  // 返回魔塔状态
  String json = "{";
  json += "\"floor\":" + String(current_floor);
  json += ",\"gold\":" + String(hero.gold);
  json += ",\"exp\":" + String(hero.exp);
  json += ",\"hp\":" + String(hero.hp);
  json += ",\"atk\":" + String(hero.atk);
  json += ",\"def\":" + String(hero.def);
  json += ",\"keys_y\":" + String(hero.keys_y);
  json += ",\"keys_b\":" + String(hero.keys_b);
  json += ",\"keys_r\":" + String(hero.keys_r);
  json += ",\"x\":" + String(hero.x);
  json += ",\"y\":" + String(hero.y);
  json += "}";
  server.send(200, "application/json", json);
}

static void handle2048Command() {
  String action = server.arg("action");
  String paramDx = server.arg("dx");
  String paramDy = server.arg("dy");
  
  // 如果不是2048模式，返回错误
  if (web_game_mode != 3) {
    server.send(200, "application/json", "{\"error\":\"not_in_2048_mode\"}");
    return;
  }
  
  // 处理移动
  if (paramDx.length() > 0 && paramDy.length() > 0) {
    int dx = paramDx.toInt();
    int dy = paramDy.toInt();
    game_2048_move(dx, dy);
  }
  
  // 返回2048状态
  String json = "{";
  json += "\"score\":" + String(score_2048);
  json += ",\"best_score\":" + String(best_score_2048);
  json += "}";
  server.send(200, "application/json", json);
}

static void handleNotFound() {
  // 所有未知请求都走Captive Portal重定向
  handleCaptivePortal();
}

// ==================== 天气时钟配网处理函数声明 ====================
static void handleWeatherConfig();
static void handleWeatherScan();
static void handleWeatherSaveConfig();
static void handleWeatherStatus();

// ==================== 初始化Web控制 ====================
void initWebLEDControl() {
  if (webControlInitialized) return;
  
  // 初始化LED任务（不依赖触屏应用）
  if (ledTaskHandle == NULL) {
    pinMode(LED_PIN, OUTPUT);
    xTaskCreatePinnedToCore(led_task, "LEDTask", 4096, NULL, 1, &ledTaskHandle, 1);
    Serial.println("[WebLED] LED任务已创建");
  }
  
  // 初始化WiFi AP
  initWiFiAP();
  
  // 注册HTTP路由
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/cmd/power", handleCommand);
  server.on("/cmd/rgb", handleCommand);
  server.on("/cmd/mode", handleCommand);
  server.on("/cmd/timer", handleCommand);
  server.on("/cmd/flash", handleCommand);
  // 游戏控制路由
  server.on("/cmd/game", handleGameCommand);
  server.on("/cmd/tower", handleTowerCommand);
  server.on("/cmd/2048", handle2048Command);
  
  // 天气时钟配网路由
  server.on("/weather", handleWeatherConfig);
  server.on("/cmd/weather/scan", handleWeatherScan);
  server.on("/cmd/weather/config", handleWeatherSaveConfig);
  server.on("/cmd/weather/status", handleWeatherStatus);
     
  // 启动服务器
  server.begin();
  server.onNotFound(handleNotFound);
  
  webControlInitialized = true;
  Serial.println("[WebLED] Web服务器已启动");
}

// ==================== 循环处理 (必须在loop中调用) ====================
void webLEDControlLoop() {
  if (!webControlInitialized) return;
  
  dnsServer.processNextRequest();
  server.handleClient();
}

// ==================== 天气时钟配网页面 ====================
static const char WEATHER_CONFIG_PAGE[] PROGMEM = R"rawlcd(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>天气时钟 - WiFi配置</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
  font-family: -apple-system, 'Helvetica Neue', Arial, sans-serif;
  background: linear-gradient(135deg, #0a0a1a 0%, #1a1a3e 50%, #0a0a1a 100%);
  color: #e0e0e0;
  min-height: 100vh;
  padding: 16px;
}
.header {
  text-align: center;
  padding: 16px 0 12px;
  font-size: 20px;
  font-weight: 700;
  color: #51cf66;
}
.back-btn {
  display: inline-block;
  padding: 8px 16px;
  margin-bottom: 12px;
  border-radius: 8px;
  border: 2px solid rgba(255,255,255,0.2);
  background: rgba(255,255,255,0.05);
  color: #ccc;
  font-size: 14px;
  cursor: pointer;
  text-decoration: none;
}
.section {
  background: rgba(255,255,255,0.05);
  border-radius: 16px;
  padding: 16px;
  margin-bottom: 16px;
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255,255,255,0.08);
}
.section-title {
  font-size: 14px;
  font-weight: 600;
  color: #888;
  margin-bottom: 12px;
  text-transform: uppercase;
  letter-spacing: 1px;
}
.form-group {
  margin-bottom: 12px;
}
.form-group label {
  display: block;
  font-size: 13px;
  color: #aaa;
  margin-bottom: 6px;
}
.form-group input, .form-group select {
  width: 100%;
  padding: 10px 12px;
  border-radius: 8px;
  border: 1px solid rgba(255,255,255,0.2);
  background: rgba(0,0,0,0.3);
  color: #fff;
  font-size: 14px;
}
.form-group input:focus {
  outline: none;
  border-color: #51cf66;
}
.wifi-list {
  max-height: 200px;
  overflow-y: auto;
}
.wifi-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 10px;
  border-radius: 8px;
  margin-bottom: 6px;
  background: rgba(0,0,0,0.2);
  cursor: pointer;
  transition: all 0.2s;
}
.wifi-item:hover, .wifi-item.selected {
  background: rgba(81,207,102,0.15);
  border: 1px solid rgba(81,207,102,0.3);
}
.wifi-item .ssid {
  font-size: 14px;
  color: #fff;
}
.wifi-item .signal {
  font-size: 12px;
  color: #888;
}
.btn {
  width: 100%;
  padding: 12px;
  border-radius: 10px;
  border: none;
  font-size: 15px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
}
.btn-primary {
  background: linear-gradient(135deg, #51cf66, #27ae60);
  color: #fff;
}
.btn-primary:active {
  transform: scale(0.98);
}
.btn-secondary {
  background: rgba(255,255,255,0.1);
  color: #ccc;
  margin-top: 10px;
}
.status {
  text-align: center;
  padding: 12px;
  border-radius: 10px;
  margin-top: 12px;
  font-size: 14px;
  display: none;
}
.status.success { background: rgba(81,207,102,0.2); color: #51cf66; display: block; }
.status.error { background: rgba(255,107,107,0.2); color: #ff6b6b; display: block; }
.status.loading { background: rgba(255,212,59,0.2); color: #ffd43b; display: block; }
.btn-locate {
  background: linear-gradient(135deg, #339af0, #2980b9);
  color: #fff;
  margin-top: 8px;
  padding: 10px;
  font-size: 14px;
}
.locate-status {
  text-align: center;
  padding: 8px;
  font-size: 13px;
  color: #888;
  display: none;
}
.locate-status.active { display: block; color: #ffd43b; }
.locate-status.success { display: block; color: #51cf66; }
.locate-status.fail { display: block; color: #ff6b6b; }
</style>
</head>
<body>
<a href="/" class="back-btn">← 返回</a>
<div class="header">🌤 天气时钟 - WiFi配置</div>

<div class="section">
  <div class="section-title">📡 扫描WiFi网络</div>
  <button class="btn btn-secondary" onclick="scanWiFi()" id="scanBtn">扫描网络</button>
  <div class="wifi-list" id="wifiList" style="margin-top:12px"></div>
</div>

<div class="section">
  <div class="section-title">⚙️ 手动配置</div>
  <div class="form-group">
    <label>WiFi名称 (SSID)</label>
    <input type="text" id="ssid" placeholder="输入WiFi名称">
  </div>
  <div class="form-group">
    <label>WiFi密码</label>
    <input type="password" id="password" placeholder="输入WiFi密码">
  </div>
  <div class="form-group">
    <label>城市 (用于天气)</label>
    <select id="city" onchange="onCityChange()" style="width:100%;padding:10px;border-radius:8px;border:1px solid #444;background:#2a2a3a;color:#fff;font-size:14px">
      <option value="101010100">北京</option>
      <option value="101020100">上海</option>
      <option value="101030100">天津</option>
      <option value="101040100">重庆</option>
      <option value="101100101">太原</option>
      <option value="101090101">石家庄</option>
      <option value="101080101">呼和浩特</option>
      <option value="101070101">沈阳</option>
      <option value="101060101">长春</option>
      <option value="101050101">哈尔滨</option>
      <option value="101190101">南京</option>
      <option value="101210101">杭州</option>
      <option value="101220101">合肥</option>
      <option value="101230101">福州</option>
      <option value="101240101">南昌</option>
      <option value="101120101">济南</option>
      <option value="101180101">郑州</option>
      <option value="101200101">武汉</option>
      <option value="101250101">长沙</option>
      <option value="101280101">广州</option>
      <option value="101300101">南宁</option>
      <option value="101310101">海口</option>
      <option value="101270101">成都</option>
      <option value="101260101">贵阳</option>
      <option value="101290101">昆明</option>
      <option value="101140101">拉萨</option>
      <option value="101110101">西安</option>
      <option value="101160101">兰州</option>
      <option value="101150101">西宁</option>
      <option value="101170101">银川</option>
      <option value="101130101">乌鲁木齐</option>
      <option value="101320101">香港</option>
      <option value="101330101">澳门</option>
      <option value="101340101">台北</option>
      <option value="101280601">深圳</option>
      <option value="101280701">珠海</option>
      <option value="101280800">佛山</option>
      <option value="101281601">东莞</option>
      <option value="101281701">中山</option>
      <option value="101280501">汕头</option>
      <option value="101281001">湛江</option>
      <option value="101280201">韶关</option>
      <option value="101280401">梅州</option>
      <option value="101280301">惠州</option>
      <option value="101280901">肇庆</option>
      <option value="101281101">江门</option>
      <option value="101281201">河源</option>
      <option value="101281301">清远</option>
      <option value="101281401">云浮</option>
      <option value="101281501">潮州</option>
      <option value="101281801">阳江</option>
      <option value="101281901">揭阳</option>
      <option value="101282001">茂名</option>
      <option value="101282101">汕尾</option>
      <option value="101210401">宁波</option>
      <option value="101210201">湖州</option>
      <option value="101210301">嘉兴</option>
      <option value="101210501">绍兴</option>
      <option value="101210601">台州</option>
      <option value="101210701">温州</option>
      <option value="101210801">丽水</option>
      <option value="101210901">金华</option>
      <option value="101211001">衢州</option>
      <option value="101211101">舟山</option>
      <option value="101190201">无锡</option>
      <option value="101190301">镇江</option>
      <option value="101190401">苏州</option>
      <option value="101190501">南通</option>
      <option value="101190601">扬州</option>
      <option value="101190701">盐城</option>
      <option value="101190801">徐州</option>
      <option value="101190901">淮安</option>
      <option value="101191001">连云港</option>
      <option value="101191101">常州</option>
      <option value="101191201">泰州</option>
      <option value="101191301">宿迁</option>
      <option value="101120201">青岛</option>
      <option value="101120301">淄博</option>
      <option value="101120401">德州</option>
      <option value="101120501">烟台</option>
      <option value="101120601">潍坊</option>
      <option value="101120701">济宁</option>
      <option value="101120801">泰安</option>
      <option value="101120901">临沂</option>
      <option value="101121001">菏泽</option>
      <option value="101121101">滨州</option>
      <option value="101121201">东营</option>
      <option value="101121301">威海</option>
      <option value="101121401">枣庄</option>
      <option value="101121501">日照</option>
      <option value="101121701">聊城</option>
      <option value="101180201">安阳</option>
      <option value="101180301">新乡</option>
      <option value="101180401">许昌</option>
      <option value="101180501">平顶山</option>
      <option value="101180601">信阳</option>
      <option value="101180701">南阳</option>
      <option value="101180801">开封</option>
      <option value="101180901">洛阳</option>
      <option value="101181001">商丘</option>
      <option value="101181101">焦作</option>
      <option value="101181201">鹤壁</option>
      <option value="101181301">濮阳</option>
      <option value="101181401">周口</option>
      <option value="101181501">漯河</option>
      <option value="101181601">驻马店</option>
      <option value="101181701">三门峡</option>
      <option value="101200201">襄阳</option>
      <option value="101200301">鄂州</option>
      <option value="101200401">孝感</option>
      <option value="101200501">黄冈</option>
      <option value="101200601">黄石</option>
      <option value="101200701">咸宁</option>
      <option value="101200801">荆州</option>
      <option value="101200901">宜昌</option>
      <option value="101201001">恩施</option>
      <option value="101201101">十堰</option>
      <option value="101201301">随州</option>
      <option value="101201401">荆门</option>
      <option value="101250201">湘潭</option>
      <option value="101250301">株洲</option>
      <option value="101250401">衡阳</option>
      <option value="101250501">郴州</option>
      <option value="101250601">常德</option>
      <option value="101250701">益阳</option>
      <option value="101250801">娄底</option>
      <option value="101250901">邵阳</option>
      <option value="101251001">岳阳</option>
      <option value="101251101">张家界</option>
      <option value="101251201">怀化</option>
      <option value="101270201">攀枝花</option>
      <option value="101270301">自贡</option>
      <option value="101270401">绵阳</option>
      <option value="101270501">南充</option>
      <option value="101270601">达州</option>
      <option value="101271001">泸州</option>
      <option value="101271101">宜宾</option>
      <option value="101271201">内江</option>
      <option value="101271401">乐山</option>
      <option value="101271501">眉山</option>
      <option value="101272001">德阳</option>
      <option value="101272101">广元</option>
      <option value="101090201">保定</option>
      <option value="101090301">张家口</option>
      <option value="101090401">承德</option>
      <option value="101090501">唐山</option>
      <option value="101090601">廊坊</option>
      <option value="101090701">沧州</option>
      <option value="101090801">衡水</option>
      <option value="101090901">邢台</option>
      <option value="101091001">邯郸</option>
      <option value="101091101">秦皇岛</option>
      <option value="101070201">大连</option>
      <option value="101070301">鞍山</option>
      <option value="101070401">抚顺</option>
      <option value="101070501">本溪</option>
      <option value="101070601">丹东</option>
      <option value="101070701">锦州</option>
      <option value="101070801">营口</option>
      <option value="101071301">盘锦</option>
      <option value="101071401">葫芦岛</option>
      <option value="101100201">大同</option>
      <option value="101100301">阳泉</option>
      <option value="101100401">晋中</option>
      <option value="101100501">长治</option>
      <option value="101100601">晋城</option>
      <option value="101100701">临汾</option>
      <option value="101100801">运城</option>
      <option value="101100901">朔州</option>
      <option value="101101001">忻州</option>
      <option value="101101100">吕梁</option>
      <option value="101110201">咸阳</option>
      <option value="101110300">延安</option>
      <option value="101110401">榆林</option>
      <option value="101110501">渭南</option>
      <option value="101110701">安康</option>
      <option value="101110801">汉中</option>
      <option value="101110901">宝鸡</option>
      <option value="101160201">定西</option>
      <option value="101160301">平凉</option>
      <option value="101160401">庆阳</option>
      <option value="101160501">武威</option>
      <option value="101160701">张掖</option>
      <option value="101160801">酒泉</option>
      <option value="101160901">天水</option>
      <option value="101161301">白银</option>
      <option value="101161401">嘉峪关</option>
      <option value="101230201">厦门</option>
      <option value="101230301">宁德</option>
      <option value="101230401">莆田</option>
      <option value="101230501">泉州</option>
      <option value="101230601">漳州</option>
      <option value="101230701">龙岩</option>
      <option value="101230801">三明</option>
      <option value="101230901">南平</option>
      <option value="101220201">蚌埠</option>
      <option value="101220301">芜湖</option>
      <option value="101220401">淮南</option>
      <option value="101220501">马鞍山</option>
      <option value="101220601">安庆</option>
      <option value="101220701">宿州</option>
      <option value="101220801">阜阳</option>
      <option value="101220901">亳州</option>
      <option value="101221001">黄山</option>
      <option value="101221101">滁州</option>
      <option value="101221201">淮北</option>
      <option value="101221301">铜陵</option>
      <option value="101221401">宣城</option>
      <option value="101221501">六安</option>
      <option value="101221701">池州</option>
      <option value="101240201">九江</option>
      <option value="101240301">上饶</option>
      <option value="101240401">抚州</option>
      <option value="101240501">宜春</option>
      <option value="101240601">吉安</option>
      <option value="101240701">赣州</option>
      <option value="101240801">景德镇</option>
      <option value="101300301">柳州</option>
      <option value="101300501">桂林</option>
      <option value="101300601">梧州</option>
      <option value="101300801">贵港</option>
      <option value="101300901">玉林</option>
      <option value="101301001">百色</option>
      <option value="101301101">钦州</option>
      <option value="101301301">北海</option>
      <option value="101290201">大理</option>
      <option value="101290401">曲靖</option>
      <option value="101290501">保山</option>
      <option value="101290701">玉溪</option>
      <option value="101291401">丽江</option>
      <option value="101260201">遵义</option>
      <option value="101260301">安顺</option>
      <option value="101260601">铜仁</option>
      <option value="101260701">毕节</option>
      <option value="101260801">六盘水</option>
      <option value="101310201">三亚</option>
      <option value="101310211">琼海</option>
      <option value="101310205">儋州</option>
      <option value="101050201">齐齐哈尔</option>
      <option value="101050301">牡丹江</option>
      <option value="101050401">佳木斯</option>
      <option value="101050901">大庆</option>
      <option value="101060201">吉林市</option>
      <option value="101060401">四平</option>
      <option value="101060501">通化</option>
      <option value="101080201">包头</option>
      <option value="101080501">通辽</option>
      <option value="101080601">赤峰</option>
      <option value="101080701">鄂尔多斯</option>
      <option value="101130201">克拉玛依</option>
      <option value="101130301">石河子</option>
      <option value="101130901">喀什</option>
      <option value="custom">自定义LocationID...</option>
    </select>
    <input type="text" id="cityCustom" placeholder="输入和风LocationID（如101010100）" style="display:none;margin-top:8px;width:100%;padding:10px;border-radius:8px;border:1px solid #444;background:#2a2a3a;color:#fff;font-size:14px">
    <button class="btn btn-locate" onclick="autoLocate()" id="locateBtn">📍 根据网络自动定位</button>
    <div class="locate-status" id="locateStatus"></div>
  </div>
  <div class="form-group">
    <label>和风天气 API Key</label>
    <input type="text" id="apiKey" placeholder="输入API Key" value="">
  </div>
</div>

<div class="section">
  <button class="btn btn-primary" onclick="saveConfig()">💾 保存并测试连接</button>
  <div class="status" id="status"></div>
</div>

<script>
let selectedSsid = '';

// [FIX 3] 城市选择下拉框处理函数
function onCityChange() {
  const citySelect = document.getElementById('city');
  const cityCustom = document.getElementById('cityCustom');
  if (citySelect.value === 'custom') {
    cityCustom.style.display = 'block';
    cityCustom.focus();
  } else {
    cityCustom.style.display = 'none';
  }
}

// 城市名 → LocationID 映射字典
const CITY_TO_LOCATIONID = {
  'beijing': '101010100', 'bj': '101010100',
  'shanghai': '101020100', 'sh': '101020100',
  'guangzhou': '101280101', 'gz': '101280101',
  'shenzhen': '101280601', 'sz': '101280601',
  'hangzhou': '101210101', 'hz': '101210101',
  'chengdu': '101270101', 'cd': '101270101',
  'wuhan': '101200101', 'wh': '101200101',
  'nanjing': '101190101', 'nj': '101190101',
  'chongqing': '101040100', 'cq': '101040100',
  'xian': '101110101', "xi'an": '101110101',
  'tianjin': '101030100', 'tj': '101030100'
};

function getCityValue() {
  const citySelect = document.getElementById('city');
  const cityCustom = document.getElementById('cityCustom');
  if (citySelect.value === 'custom') {
    let val = (cityCustom.value || '').trim().toLowerCase();
    // 如果输入的是城市名，尝试映射成 LocationID
    if (CITY_TO_LOCATIONID[val]) return CITY_TO_LOCATIONID[val];
    return val || '101010100';
  }
  return citySelect.value;
}

function autoLocate() {
  const locateBtn = document.getElementById('locateBtn');
  const locateStatus = document.getElementById('locateStatus');

  locateBtn.disabled = true;
  locateBtn.textContent = '📍 正在定位...';
  locateStatus.className = 'locate-status active';
  locateStatus.textContent = '正在通过网络获取位置信息...';

  // Use ip-api.com for geolocation (free, no key needed)
  fetch('http://ip-api.com/json/?fields=city,regionName,country,status')
    .then(r => r.json())
    .then(data => {
      locateBtn.disabled = false;
      locateBtn.textContent = '📍 根据网络自动定位';
    
      if (data.status === 'success' && data.city) {
        const cityLower = data.city.toLowerCase();
        const citySelect = document.getElementById('city');
        const cityCustom = document.getElementById('cityCustom');
      
        // 尝试通过字典映射成 LocationID
        const locationId = CITY_TO_LOCATIONID[cityLower];
        if (locationId) {
          // 尝试匹配下拉框选项
          let matched = false;
          for (let i = 0; i < citySelect.options.length; i++) {
            if (citySelect.options[i].value === locationId) {
              citySelect.selectedIndex = i;
              cityCustom.style.display = 'none';
              matched = true;
              break;
            }
          }
          if (!matched) {
            citySelect.value = 'custom';
            cityCustom.style.display = 'block';
            cityCustom.value = locationId;
          }
        } else {
          // 无法映射，让用户手动输入
          citySelect.value = 'custom';
          cityCustom.style.display = 'block';
          cityCustom.value = cityLower;
        }
      
        locateStatus.className = 'locate-status success';
        locateStatus.textContent = '✓ 定位成功: ' + data.city + ', ' + (data.regionName || '') + ', ' + (data.country || '');
      } else {
        locateStatus.className = 'locate-status fail';
        locateStatus.textContent = '✗ 定位失败，请手动选择城市';
      }
    })
    .catch(e => {
      locateBtn.disabled = false;
      locateBtn.textContent = '📍 根据网络自动定位';
    
      // Fallback: try ipinfo.io
      fetch('https://ipinfo.io/json')
        .then(r => r.json())
        .then(data => {
          if (data.city) {
            const cityLower = data.city.toLowerCase();
            const citySelect = document.getElementById('city');
            const cityCustom = document.getElementById('cityCustom');
          
            // 尝试通过字典映射成 LocationID
            const locationId = CITY_TO_LOCATIONID[cityLower];
            if (locationId) {
              let matched = false;
              for (let i = 0; i < citySelect.options.length; i++) {
                if (citySelect.options[i].value === locationId) {
                  citySelect.selectedIndex = i;
                  cityCustom.style.display = 'none';
                  matched = true;
                  break;
                }
              }
              if (!matched) {
                citySelect.value = 'custom';
                cityCustom.style.display = 'block';
                cityCustom.value = locationId;
              }
            } else {
              citySelect.value = 'custom';
              cityCustom.style.display = 'block';
              cityCustom.value = cityLower;
            }
          
            locateStatus.className = 'locate-status success';
            locateStatus.textContent = '✓ 定位成功: ' + data.city + ', ' + (data.region || '');
          } else {
            locateStatus.className = 'locate-status fail';
            locateStatus.textContent = '✗ 定位失败，请手动选择城市';
          }
        })
        .catch(e2 => {
          locateStatus.className = 'locate-status fail';
          locateStatus.textContent = '✗ 无法连接定位服务（需要设备连接外网）';
        });
    });
}

function scanWiFi() {
  const btn = document.getElementById('scanBtn');
  const list = document.getElementById('wifiList');
  btn.textContent = '扫描中...';
  btn.disabled = true;
  list.innerHTML = '<div style="text-align:center;color:#888;padding:20px">正在扫描WiFi网络...</div>';
  
  fetch('/cmd/weather/scan')
    .then(r => r.json())
    .then(data => {
      btn.textContent = '重新扫描';
      btn.disabled = false;
      
      if (data.wifi && data.wifi.length > 0) {
        list.innerHTML = '';
        data.wifi.forEach(w => {
          const item = document.createElement('div');
          item.className = 'wifi-item';
          item.innerHTML = '<span class="ssid">📶 ' + w.ssid + '</span>' +
                           '<span class="signal">' + w.rssi + 'dBm</span>';
          item.onclick = () => selectWifi(w.ssid, item);
          list.appendChild(item);
        });
      } else {
        list.innerHTML = '<div style="text-align:center;color:#888;padding:20px">未找到WiFi网络</div>';
      }
    })
    .catch(e => {
      btn.textContent = '扫描失败，重试';
      btn.disabled = false;
      list.innerHTML = '<div style="text-align:center;color:#ff6b6b;padding:20px">扫描失败</div>';
    });
}

function selectWifi(ssid, elem) {
  selectedSsid = ssid;
  document.getElementById('ssid').value = ssid;
  document.querySelectorAll('.wifi-item').forEach(i => i.classList.remove('selected'));
  elem.classList.add('selected');
}

function saveConfig() {
  const ssid = document.getElementById('ssid').value;
  const password = document.getElementById('password').value;
  const city = getCityValue();
  const apiKey = document.getElementById('apiKey').value;
  const status = document.getElementById('status');

  if (!ssid) {
    status.className = 'status error';
    status.textContent = '请输入WiFi名称';
    return;
  }

  status.className = 'status loading';
  status.textContent = '正在保存配置...';

  fetch('/cmd/weather/config?ssid=' + encodeURIComponent(ssid) +
      '&password=' + encodeURIComponent(password) +
      '&city=' + encodeURIComponent(city) +
      '&apiKey=' + encodeURIComponent(apiKey))
    .then(r => r.json())
    .then(data => {
      if (data.ok) {
        status.className = 'status success';
        status.innerHTML = '✓ ' + data.msg;
      } else {
        status.className = 'status error';
        status.textContent = '✗ 保存失败';
      }
    })
    .catch(e => {
      status.className = 'status error';
      status.textContent = '✗ 请求失败';
    });
}

// 页面加载时自动扫描
window.onload = () => scanWiFi();
</script>
</body>
</html>
)rawlcd";

// ==================== WiFi扫描结果缓存 ====================
// 移除未使用的变量

// ==================== 天气时钟配网处理函数 ====================
// 天气时钟默认API Key
#define WEATHER_API_KEY_DEFAULT "9241d28c048d40ee87c997c8f13c6df6"


static void handleWeatherConfig() {
  String html = WEATHER_CONFIG_PAGE;
  // 自动填入默认 API Key
  html.replace("value=\"\"", "value=\"" WEATHER_API_KEY_DEFAULT "\"");
  server.send(200, "text/html", html);
}

static void handleWeatherScan() {
  // 网页端使用同步扫描最稳定
  WiFi.mode(WIFI_AP_STA);
  WiFi.scanDelete();
  
  // 同步扫描，等待扫描完成
  int wifi = WiFi.scanNetworks(false, false, false, 300);
  String json = "{\"wifi\":[";
  for (int i = 0; i < wifi && i < 20; i++) {
    if (i > 0) json += ",";
    String ssid = WiFi.SSID(i);
    ssid.replace("\\", "\\\\");
    ssid.replace("\"", "\\\"");
    json += "{\"ssid\":\"" + ssid + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i));
    json += ",\"encrypted\":" + String(WiFi.encryptionType(i)) + "}";
  }
  json += "]}";
  WiFi.scanDelete();
  server.send(200, "application/json", json);
}

static void handleWeatherSaveConfig() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String city = server.arg("city");
  String apiKey = server.arg("apiKey");
  if (apiKey.length() < 5) apiKey = WEATHER_API_KEY_DEFAULT;

  update_weather_config_from_web(ssid.c_str(), password.c_str(), city.c_str(), apiKey.c_str());

  // 【关键修复5】不再在此处发起后台 WiFi 连接！避免由于 AP 信道跳变瞬间踢掉手机，同时避免与屏幕任务互相打架抢夺底层硬件。
  String json = "{";
  json += "\"ok\":true";
  json += ",\"saved\":true";
  json += ",\"msg\":\"配置已保存，请在设备屏幕上点击【确定】连接\"";
  json += "}";
  server.send(200, "application/json", json);
}

// WiFi连接状态查询接口（简化版，仅返回当前 STA 连接状态）
static void handleWeatherStatus() {
  String json = "{";
  json += "\"status\":\"idle\"";
  json += ",\"connected\":" + String(WiFi.isConnected() ? "true" : "false");
  if (WiFi.isConnected()) {
    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  }
  json += "}";
  server.send(200, "application/json", json);
}
