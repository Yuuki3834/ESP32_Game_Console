#include "global.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_task_wdt.h>
#include <Adafruit_NeoPixel.h>

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

// ==================== 游戏控制模式变量 ====================
// 游戏控制模式: 0=无/LED控制, 1=LED控制, 2=魔塔
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
    document.getElementById('gameMenu').style.display = 'none';
  }).catch(e => console.error(e));
}

// 定时同步魔塔状态
setInterval(() => {
  if (currentGameMode === 2) {
    updateTowerInfo();
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
  
  // 配置AP
  WiFi.softAPConfig(WIFI_AP_IP, WIFI_GATEWAY, WIFI_SUBNET);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
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
      // 退出游戏控制
      web_game_mode = 0;
      web_control_active = false;
      Serial.println("[WebGame] 退出网页游戏控制");
    } else if (mode == 2) {
      // 进入游戏控制模式
      web_game_mode = mode;
      web_control_active = true;
      Serial.print("[WebGame] 进入游戏控制模式: ");
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

static void handleNotFound() {
  // 所有未知请求都走Captive Portal重定向
  handleCaptivePortal();
}

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

