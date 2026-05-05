#include "global.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <esp_sntp.h>
#include <Preferences.h>
#include <math.h>
#include <TFT_eSPI.h>

// 声明外部启用的高清内置大字体
LV_FONT_DECLARE(lv_font_montserrat_32);
LV_FONT_DECLARE(lv_font_montserrat_48);

// 默认 API Key 仅作兼容用，实际走 wttr.in 不需要
#define WEATHER_API_KEY_DEFAULT "9241d28c048d40ee87c997c8f13c6df6"

extern TFT_eSPI tft;
extern volatile bool g_landscape_mode;

typedef enum {
    CLOCK_STYLE_DIGITAL = 0,
    CLOCK_STYLE_ANALOG  = 1,
    CLOCK_STYLE_MINIMAL = 2,
    CLOCK_STYLE_RETRO   = 3
} ClockStyle_t;

typedef enum {
    INIT_IDLE = 0, INIT_CONNECTING_WIFI, INIT_SYNCING_NTP,
    INIT_FETCHING_WEATHER, INIT_DONE_SUCCESS, INIT_DONE_FAIL
} InitState_t;

// ============ 内嵌城市列表 (和风 locationID + 中文城市名) ============
struct CityItem { const char* id; const char* name; };
static const CityItem g_cities[] = {
    {"101010100","北京"},{"101020100","上海"},{"101030100","天津"},{"101040100","重庆"},
    {"101100101","太原"},{"101090101","石家庄"},{"101080101","呼和浩特"},{"101070101","沈阳"},
    {"101060101","长春"},{"101050101","哈尔滨"},{"101190101","南京"},{"101210101","杭州"},
    {"101220101","合肥"},{"101230101","福州"},{"101240101","南昌"},{"101120101","济南"},
    {"101180101","郑州"},{"101200101","武汉"},{"101250101","长沙"},{"101280101","广州"},
    {"101300101","南宁"},{"101310101","海口"},{"101270101","成都"},{"101260101","贵阳"},
    {"101290101","昆明"},{"101140101","拉萨"},{"101110101","西安"},{"101160101","兰州"},
    {"101150101","西宁"},{"101170101","银川"},{"101130101","乌鲁木齐"},{"101320101","香港"},
    {"101330101","澳门"},{"101340101","台北"},
    {"101280601","深圳"},{"101280701","珠海"},{"101280800","佛山"},{"101281601","东莞"},
    {"101281701","中山"},{"101280501","汕头"},{"101281001","湛江"},{"101280201","韶关"},
    {"101280401","梅州"},{"101280301","惠州"},{"101280901","肇庆"},{"101281101","江门"},
    {"101281201","河源"},{"101281301","清远"},{"101281401","云浮"},{"101281501","潮州"},
    {"101281801","阳江"},{"101281901","揭阳"},{"101282001","茂名"},{"101282101","汕尾"},
    {"101210401","宁波"},{"101210201","湖州"},{"101210301","嘉兴"},{"101210501","绍兴"},
    {"101210601","台州"},{"101210701","温州"},{"101210801","丽水"},{"101210901","金华"},
    {"101211001","衢州"},{"101211101","舟山"},
    {"101190201","无锡"},{"101190301","镇江"},{"101190401","苏州"},{"101190501","南通"},
    {"101190601","扬州"},{"101190701","盐城"},{"101190801","徐州"},{"101190901","淮安"},
    {"101191001","连云港"},{"101191101","常州"},{"101191201","泰州"},{"101191301","宿迁"},
    {"101120201","青岛"},{"101120301","淄博"},{"101120401","德州"},{"101120501","烟台"},
    {"101120601","潍坊"},{"101120701","济宁"},{"101120801","泰安"},{"101120901","临沂"},
    {"101121001","菏泽"},{"101121101","滨州"},{"101121201","东营"},{"101121301","威海"},
    {"101121401","枣庄"},{"101121501","日照"},{"101121701","聊城"},
    {"101180201","安阳"},{"101180301","新乡"},{"101180401","许昌"},{"101180501","平顶山"},
    {"101180601","信阳"},{"101180701","南阳"},{"101180801","开封"},{"101180901","洛阳"},
    {"101181001","商丘"},{"101181101","焦作"},{"101181201","鹤壁"},{"101181301","濮阳"},
    {"101181401","周口"},{"101181501","漯河"},{"101181601","驻马店"},{"101181701","三门峡"},
    {"101200201","襄阳"},{"101200301","鄂州"},{"101200401","孝感"},{"101200501","黄冈"},
    {"101200601","黄石"},{"101200701","咸宁"},{"101200801","荆州"},{"101200901","宜昌"},
    {"101201001","恩施"},{"101201101","十堰"},{"101201301","随州"},{"101201401","荆门"},
    {"101250201","湘潭"},{"101250301","株洲"},{"101250401","衡阳"},{"101250501","郴州"},
    {"101250601","常德"},{"101250701","益阳"},{"101250801","娄底"},{"101250901","邵阳"},
    {"101251001","岳阳"},{"101251101","张家界"},{"101251201","怀化"},
    {"101270201","攀枝花"},{"101270301","自贡"},{"101270401","绵阳"},{"101270501","南充"},
    {"101270601","达州"},{"101271001","泸州"},{"101271101","宜宾"},{"101271201","内江"},
    {"101271401","乐山"},{"101271501","眉山"},{"101272001","德阳"},{"101272101","广元"},
    {"101090201","保定"},{"101090301","张家口"},{"101090401","承德"},{"101090501","唐山"},
    {"101090601","廊坊"},{"101090701","沧州"},{"101090801","衡水"},{"101090901","邢台"},
    {"101091001","邯郸"},{"101091101","秦皇岛"},
    {"101070201","大连"},{"101070301","鞍山"},{"101070401","抚顺"},{"101070501","本溪"},
    {"101070601","丹东"},{"101070701","锦州"},{"101070801","营口"},{"101071301","盘锦"},
    {"101071401","葫芦岛"},
    {"101100201","大同"},{"101100301","阳泉"},{"101100401","晋中"},{"101100501","长治"},
    {"101100601","晋城"},{"101100701","临汾"},{"101100801","运城"},{"101100901","朔州"},
    {"101101001","忻州"},{"101101100","吕梁"},
    {"101110201","咸阳"},{"101110300","延安"},{"101110401","榆林"},{"101110501","渭南"},
    {"101110701","安康"},{"101110801","汉中"},{"101110901","宝鸡"},
    {"101160201","定西"},{"101160301","平凉"},{"101160401","庆阳"},{"101160501","武威"},
    {"101160701","张掖"},{"101160801","酒泉"},{"101160901","天水"},{"101161301","白银"},
    {"101161401","嘉峪关"},
    {"101230201","厦门"},{"101230301","宁德"},{"101230401","莆田"},{"101230501","泉州"},
    {"101230601","漳州"},{"101230701","龙岩"},{"101230801","三明"},{"101230901","南平"},
    {"101220201","蚌埠"},{"101220301","芜湖"},{"101220401","淮南"},{"101220501","马鞍山"},
    {"101220601","安庆"},{"101220701","宿州"},{"101220801","阜阳"},{"101220901","亳州"},
    {"101221001","黄山"},{"101221101","滁州"},{"101221201","淮北"},{"101221301","铜陵"},
    {"101221401","宣城"},{"101221501","六安"},{"101221701","池州"},
    {"101240201","九江"},{"101240301","上饶"},{"101240401","抚州"},{"101240501","宜春"},
    {"101240601","吉安"},{"101240701","赣州"},{"101240801","景德镇"},
    {"101300301","柳州"},{"101300501","桂林"},{"101300601","梧州"},{"101300801","贵港"},
    {"101300901","玉林"},{"101301001","百色"},{"101301101","钦州"},{"101301301","北海"},
    {"101290201","大理"},{"101290401","曲靖"},{"101290501","保山"},{"101290701","玉溪"},
    {"101291401","丽江"},
    {"101260201","遵义"},{"101260301","安顺"},{"101260601","铜仁"},{"101260701","毕节"},
    {"101260801","六盘水"},
    {"101310201","三亚"},{"101310211","琼海"},{"101310205","儋州"},
    {"101050201","齐齐哈尔"},{"101050301","牡丹江"},{"101050401","佳木斯"},{"101050901","大庆"},
    {"101060201","吉林市"},{"101060401","四平"},{"101060501","通化"},
    {"101080201","包头"},{"101080501","通辽"},{"101080601","赤峰"},{"101080701","鄂尔多斯"},
    {"101130201","克拉玛依"},{"101130301","石河子"},{"101130901","喀什"},
};
static const int g_cities_count = sizeof(g_cities)/sizeof(g_cities[0]);

// ============ 状态变量 ============
static lv_obj_t *scr_weather = NULL;
static lv_obj_t *lbl_time = NULL;
static lv_obj_t *lbl_date = NULL;
static lv_obj_t *lbl_weather_info = NULL;
static lv_obj_t *lbl_temp_info = NULL;
static lv_obj_t *kb_weather = NULL;

static lv_obj_t *g_hour_hand = NULL;
static lv_obj_t *g_min_hand = NULL;
static lv_obj_t *g_sec_hand = NULL;
static lv_point_t g_hour_pts[2];
static lv_point_t g_min_pts[2];
static lv_point_t g_sec_pts[2];

static ClockStyle_t g_clock_style = CLOCK_STYLE_DIGITAL;
static bool g_weather_running = false;
static volatile bool g_time_synced = false;

static char g_wifi_ssid[33] = {0};
static char g_wifi_password[65] = {0};
static char g_weather_city[33] = "101100101";
static char g_weather_city_display[33] = "太原";
static char g_weather_api_key[65] = WEATHER_API_KEY_DEFAULT;

static volatile char g_weather_cond[32] = "--";
static volatile char g_weather_tmp[8] = "--";
static volatile char g_weather_rh[8] = "--";
static volatile uint32_t g_last_weather_update = 0;

static time_t g_rawtime;
static struct tm g_timeinfo;

static lv_obj_t *g_style_btns[4] = {0};
static volatile InitState_t g_init_state = INIT_IDLE;
static volatile bool g_net_task_running = false;
static lv_timer_t *g_clock_timer = NULL;
static lv_timer_t *g_init_poll_timer = NULL;
static lv_timer_t *g_scan_poll_timer = NULL;
static volatile bool g_scan_done = false;

// ============ 前向声明 ============
static void build_style_select_page();
static void build_wifi_config_page();
static void build_wifi_scan_page();
static void build_city_select_page();
static void build_weather_clock_page();

// ============ 横竖屏切换 ============
static void enter_landscape() {
    if (g_landscape_mode) return;
    g_landscape_mode = true;
    tft.setRotation(1);
    tft.fillScreen(0x0000);
    lv_disp_t *d = lv_disp_get_default();
    if (d && d->driver) {
        d->driver->hor_res = 320;
        d->driver->ver_res = 240;
        lv_disp_drv_update(d, d->driver);
    }
}

static void exit_landscape() {
    if (!g_landscape_mode) return;
    g_landscape_mode = false;
    tft.setRotation(0);
    tft.fillScreen(0x0000);
    lv_disp_t *d = lv_disp_get_default();
    if (d && d->driver) {
        d->driver->hor_res = 240;
        d->driver->ver_res = 320;
        lv_disp_drv_update(d, d->driver);
    }
}

static void ensure_ap_sta_mode() {
    wifi_mode_t m = WiFi.getMode();
    if (m != WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

static void weather_switch_screen(void (*builder)()) {
    lv_obj_t *old_scr = scr_weather;
    scr_weather = NULL;
    lbl_time = lbl_date = lbl_weather_info = lbl_temp_info = kb_weather = NULL;
    g_hour_hand = g_min_hand = g_sec_hand = NULL;
    exit_landscape();
    builder();
    if (scr_weather) lv_scr_load(scr_weather);
    if (old_scr) lv_obj_del_async(old_scr);
}

static void safe_kb_focus_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_FOCUSED) {
        if (!kb_weather) return;
        lv_obj_t *ta = lv_event_get_target(e);
        lv_keyboard_set_textarea(kb_weather, ta);
        lv_obj_clear_flag(kb_weather, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(kb_weather);
        lv_obj_scroll_to_view_recursive(ta, LV_ANIM_ON);
    } else if (code == LV_EVENT_DEFOCUSED) {
        if (kb_weather) lv_obj_add_flag(kb_weather, LV_OBJ_FLAG_HIDDEN);
    }
}

static const char* find_city_name(const char* id) {
    for (int i = 0; i < g_cities_count; i++) {
        if (strcmp(g_cities[i].id, id) == 0) return g_cities[i].name;
    }
    return id;
}

static const char* translate_weather_cond(const char* en) {
    if (!en || !*en) return "--";
    String s = en;
    s.toLowerCase();
    s.replace(" ", ""); s.replace(",", ""); s.replace(".", "");
    struct E { const char* k; const char* v; };
    static const E m[] = {
        {"sunny","晴"},{"clear","晴"},
        {"partlycloudy","多云"},{"cloudy","阴"},{"overcast","阴"},
        {"mist","雾"},{"fog","雾"},{"freezingfog","冻雾"},{"haze","霾"},
        {"patchyrainpossible","小雨"},{"patchyrainnearby","小雨"},
        {"patchysnowpossible","小雪"},{"patchysleetpossible","雨夹雪"},
        {"patchyfreezingdrizzlepossible","冻雨"},
        {"thunderyoutbreakspossible","雷阵雨"},
        {"blowingsnow","吹雪"},{"blizzard","暴雪"},
        {"patchylightdrizzle","小雨"},{"lightdrizzle","小雨"},
        {"freezingdrizzle","冻雨"},{"heavyfreezingdrizzle","冻雨"},
        {"patchylightrain","小雨"},{"lightrain","小雨"},
        {"moderaterainattimes","中雨"},{"moderaterain","中雨"},
        {"heavyrainattimes","大雨"},{"heavyrain","大雨"},
        {"lightfreezingrain","冻雨"},{"moderateorheavyfreezingrain","冻雨"},
        {"lightsleet","雨夹雪"},{"moderateorheavysleet","雨夹雪"},
        {"patchylightsnow","小雪"},{"lightsnow","小雪"},
        {"patchymoderatesnow","中雪"},{"moderatesnow","中雪"},
        {"patchyheavysnow","大雪"},{"heavysnow","大雪"},
        {"icepellets","冰雹"},
        {"lightrainshower","阵雨"},{"lightrainshowers","阵雨"},
        {"moderateorheavyrainshower","阵雨"},{"torrentialrainshower","暴雨"},
        {"lightsleetshowers","雨夹雪"},{"moderateorheavysleetshowers","雨夹雪"},
        {"lightsnowshowers","阵雪"},{"moderateorheavysnowshowers","阵雪"},
        {"lightshowersoficepellets","冰雹"},
        {"moderateorheavyshowersoficepellets","冰雹"},
        {"patchylightrainwiththunder","雷阵雨"},
        {"moderateorheavyrainwiththunder","雷雨"},
        {"patchylightsnowwiththunder","雷阵雪"},
        {"moderateorheavysnowwiththunder","雷阵雪"},
        {"thundery","雷雨"},
    };
    int n = sizeof(m)/sizeof(m[0]);
    for (int i = 0; i < n; i++) if (s == m[i].k) return m[i].v;
    if (s.indexOf("thunder") >= 0) return "雷雨";
    if (s.indexOf("rain") >= 0)    return "雨";
    if (s.indexOf("snow") >= 0)    return "雪";
    if (s.indexOf("drizzle") >= 0) return "小雨";
    if (s.indexOf("sleet") >= 0)   return "雨夹雪";
    if (s.indexOf("hail") >= 0)    return "冰雹";
    if (s.indexOf("cloud") >= 0)   return "多云";
    if (s.indexOf("overcast") >= 0) return "阴";
    if (s.indexOf("clear") >= 0 || s.indexOf("sun") >= 0) return "晴";
    if (s.indexOf("fog") >= 0 || s.indexOf("mist") >= 0)  return "雾";
    if (s.indexOf("haze") >= 0)    return "霾";
    return "未知";
}

static void load_weather_config() {
    Preferences prefs;
    prefs.begin("weather", true);
    String ssid = prefs.getString("ssid", "");
    String pwd  = prefs.getString("pwd", "");
    String city = prefs.getString("city", "101100101");
    String api  = prefs.getString("api", WEATHER_API_KEY_DEFAULT);
    prefs.end();
    if (ssid.length() > 0) strlcpy(g_wifi_ssid, ssid.c_str(), sizeof(g_wifi_ssid));
    if (pwd.length() > 0) strlcpy(g_wifi_password, pwd.c_str(), sizeof(g_wifi_password));
    strlcpy(g_weather_city, city.c_str(), sizeof(g_weather_city));
    strlcpy(g_weather_api_key, api.c_str(), sizeof(g_weather_api_key));
    strlcpy(g_weather_city_display, find_city_name(g_weather_city), sizeof(g_weather_city_display));
}

static void save_weather_config() {
    Preferences prefs;
    prefs.begin("weather", false);
    prefs.putString("ssid", g_wifi_ssid);
    prefs.putString("pwd", g_wifi_password);
    prefs.putString("city", g_weather_city);
    prefs.putString("api", g_weather_api_key);
    prefs.end();
}

static bool connect_wifi_station(const char* ssid, const char* password) {
    if (strlen(ssid) == 0) return false;
    if (WiFi.isConnected() && WiFi.SSID() == String(ssid)) return true;
    ensure_ap_sta_mode();
    WiFi.disconnect(false, false);
    vTaskDelay(pdMS_TO_TICKS(200));
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;
    }
    return WiFi.status() == WL_CONNECTED;
}

static void time_sync_callback(struct timeval *tv) { g_time_synced = true; }

static void sync_ntp_time() {
    if (!WiFi.isConnected()) return;
    static bool ntp_cb_set = false;
    if (!ntp_cb_set) {
        sntp_set_time_sync_notification_cb(time_sync_callback);
        ntp_cb_set = true;
    }
    configTzTime("CST-8", "ntp.aliyun.com", "pool.ntp.org", "time.apple.com");
    int timeout = 40;
    while (!g_time_synced && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(500));
        timeout--;
    }
}

static String url_encode(const String &s) {
    String out; out.reserve(s.length() * 3);
    for (size_t i = 0; i < s.length(); i++) {
        uint8_t c = (uint8_t)s[i];
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') || c == '-' || c == '_' || c == '.' || c == '~') {
            out += (char)c;
        } else {
            char buf[4]; sprintf(buf, "%%%02X", c); out += buf;
        }
    }
    return out;
}

static void fetch_weather_data() {
    g_last_weather_update = millis();
    if (!WiFi.isConnected()) {
        strlcpy((char*)g_weather_cond, "无网络", sizeof(g_weather_cond));
        return;
    }

    WiFiClient client;
    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(10000);
    http.setReuse(false);
    http.setUserAgent("curl/7.88.0");

    String city = g_weather_city_display;
    if (city.length() == 0) city = "Beijing";
    String url = "http://wttr.in/" + url_encode(city) + "?format=%C_%t_%h&lang=zh";

    Serial.printf("[Weather] GET %s\n", url.c_str());
    if (!http.begin(client, url)) {
        strlcpy((char*)g_weather_cond, "建立失败", sizeof(g_weather_cond));
        return;
    }
    int code = http.GET();
    String body;
    if (code > 0) body = http.getString();
    http.end();

    if (code == 200 && body.length() > 0) {
        body.trim();
        if (body.startsWith("Unknown")) {
            strlcpy((char*)g_weather_cond, "未知城市", sizeof(g_weather_cond));
            strlcpy((char*)g_weather_tmp, "--", sizeof(g_weather_tmp));
            strlcpy((char*)g_weather_rh, "--", sizeof(g_weather_rh));
            return;
        }
        int i1 = body.indexOf('_');
        int i2 = (i1 > 0) ? body.indexOf('_', i1 + 1) : -1;
        if (i1 > 0 && i2 > 0) {
            String cond = body.substring(0, i1);
            String temp = body.substring(i1 + 1, i2);
            String rh   = body.substring(i2 + 1);
            temp.replace("+", ""); temp.replace("°C", ""); temp.replace("°F", ""); temp.trim();
            rh.replace("%", ""); rh.trim();
            if (cond.length() == 0) cond = "--";
            const char* zh_cond = translate_weather_cond(cond.c_str());
            if (temp.length() == 0) temp = "--";
            if (rh.length()   == 0) rh   = "--";
            strlcpy((char*)g_weather_cond, zh_cond, sizeof(g_weather_cond));
            strlcpy((char*)g_weather_tmp,  temp.c_str(), sizeof(g_weather_tmp));
            strlcpy((char*)g_weather_rh,   rh.c_str(),   sizeof(g_weather_rh));
        } else {
            strlcpy((char*)g_weather_cond, "格式错", sizeof(g_weather_cond));
        }
    } else {
        snprintf((char*)g_weather_cond, sizeof(g_weather_cond), "HTTP%d", code);
    }
}

// ============ 更新 UI ============
static void update_clock_ui() {
    if (!g_time_synced || !g_weather_running || !scr_weather) return;
    time(&g_rawtime);
    localtime_r(&g_rawtime, &g_timeinfo);
    char buf[80];
    const char* week_days[] = {"日","一","二","三","四","五","六"};
    
    if (g_clock_style == CLOCK_STYLE_DIGITAL) {
        snprintf(buf, sizeof(buf), "%02d:%02d", g_timeinfo.tm_hour, g_timeinfo.tm_min);
        if (lbl_time) lv_label_set_text(lbl_time, buf);
        
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d  %s", 1900+g_timeinfo.tm_year, 1+g_timeinfo.tm_mon, g_timeinfo.tm_mday, week_days[g_timeinfo.tm_wday]);
        if (lbl_date) lv_label_set_text(lbl_date, buf);
        
        snprintf(buf, sizeof(buf), "%s   %s 度", (const char*)g_weather_cond, (const char*)g_weather_tmp);
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, buf);
        
        snprintf(buf, sizeof(buf), "%02d 秒  |  湿度 %s%%", g_timeinfo.tm_sec, (const char*)g_weather_rh);
        if (lbl_temp_info) lv_label_set_text(lbl_temp_info, buf);
        
    } else if (g_clock_style == CLOCK_STYLE_ANALOG) {
        const int cx = 120, cy = 140;
        float sec_a = g_timeinfo.tm_sec * 6.0f * 3.14159f / 180.0f;
        float min_a = (g_timeinfo.tm_min + g_timeinfo.tm_sec/60.0f) * 6.0f * 3.14159f / 180.0f;
        float hr_a  = ((g_timeinfo.tm_hour % 12) + g_timeinfo.tm_min/60.0f) * 30.0f * 3.14159f / 180.0f;
        g_hour_pts[0] = {cx, cy}; g_hour_pts[1] = { (lv_coord_t)(cx + sinf(hr_a)*45), (lv_coord_t)(cy - cosf(hr_a)*45) };
        g_min_pts[0]  = {cx, cy}; g_min_pts[1]  = { (lv_coord_t)(cx + sinf(min_a)*65), (lv_coord_t)(cy - cosf(min_a)*65) };
        g_sec_pts[0]  = {cx, cy}; g_sec_pts[1]  = { (lv_coord_t)(cx + sinf(sec_a)*75), (lv_coord_t)(cy - cosf(sec_a)*75) };
        
        if (g_hour_hand) lv_line_set_points(g_hour_hand, g_hour_pts, 2);
        if (g_min_hand)  lv_line_set_points(g_min_hand, g_min_pts, 2);
        if (g_sec_hand)  lv_line_set_points(g_sec_hand, g_sec_pts, 2);
        
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d 周%s", 1900+g_timeinfo.tm_year, 1+g_timeinfo.tm_mon, g_timeinfo.tm_mday, week_days[g_timeinfo.tm_wday]);
        if (lbl_date) lv_label_set_text(lbl_date, buf);
        snprintf(buf, sizeof(buf), "%s  %s度  湿度%s%%", (const char*)g_weather_cond, (const char*)g_weather_tmp, (const char*)g_weather_rh);
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, buf);
        
    } else if (g_clock_style == CLOCK_STYLE_MINIMAL) {
        snprintf(buf, sizeof(buf), "%02d:%02d", g_timeinfo.tm_hour, g_timeinfo.tm_min);
        if (lbl_time) lv_label_set_text(lbl_time, buf);
        
        snprintf(buf, sizeof(buf), "%04d/%02d/%02d  周%s", 1900+g_timeinfo.tm_year, 1+g_timeinfo.tm_mon, g_timeinfo.tm_mday, week_days[g_timeinfo.tm_wday]);
        if (lbl_date) lv_label_set_text(lbl_date, buf);
        
        snprintf(buf, sizeof(buf), "%s   %s度   湿度 %s%%", (const char*)g_weather_cond, (const char*)g_weather_tmp, (const char*)g_weather_rh);
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, buf);
        
        snprintf(buf, sizeof(buf), ":%02d", g_timeinfo.tm_sec);
        if (lbl_temp_info) lv_label_set_text(lbl_temp_info, buf);
        
    } else if (g_clock_style == CLOCK_STYLE_RETRO) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", g_timeinfo.tm_hour, g_timeinfo.tm_min, g_timeinfo.tm_sec);
        if (lbl_time) lv_label_set_text(lbl_time, buf);
        
        snprintf(buf, sizeof(buf), "%02d-%02d-%02d  周%s", (1900+g_timeinfo.tm_year)%100, 1+g_timeinfo.tm_mon, g_timeinfo.tm_mday, week_days[g_timeinfo.tm_wday]);
        if (lbl_date) lv_label_set_text(lbl_date, buf);
        
        snprintf(buf, sizeof(buf), "%s  %s度  %s%%RH", (const char*)g_weather_cond, (const char*)g_weather_tmp, (const char*)g_weather_rh);
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, buf);
    }
}

static void clock_timer_cb(lv_timer_t *timer) {
    if (!g_weather_running) {
        if (g_clock_timer) { lv_timer_del(g_clock_timer); g_clock_timer = NULL; }
        return;
    }
    if (!scr_weather || !lv_obj_is_valid(scr_weather)) {
        g_weather_running = false;
        if (g_clock_timer) { lv_timer_del(g_clock_timer); g_clock_timer = NULL; }
        return;
    }
    update_clock_ui();
    if (millis() - g_last_weather_update > 300000 && WiFi.isConnected()) {
        xTaskCreatePinnedToCore([](void* p){ fetch_weather_data(); vTaskDelete(NULL); },
            "WeatherFetch", 8192, NULL, 1, NULL, 0);
    }
}

// ============ WiFi 扫描 ============
static void scan_poll_timer_cb(lv_timer_t *timer) {
    if (!g_scan_done) return;
    if (g_scan_poll_timer) { lv_timer_del(g_scan_poll_timer); g_scan_poll_timer = NULL; }
    lv_obj_t *list = (lv_obj_t *)timer->user_data;
    if (!list || !lv_obj_is_valid(list)) return;
    lv_obj_clean(list);
    int16_t result = WiFi.scanComplete();
    if (result < 0) {
        lv_obj_t *lbl = lv_label_create(list);
        lv_obj_add_style(lbl, &style_cn, 0);
        lv_label_set_text(lbl, "扫描失败");
    } else if (result == 0) {
        lv_obj_t *lbl = lv_label_create(list);
        lv_obj_add_style(lbl, &style_cn, 0);
        lv_label_set_text(lbl, "未发现网络");
    } else {
        for (int i = 0; i < result && i < 15; i++) {
            String ssid = WiFi.SSID(i);
            if (ssid.length() == 0) continue;
            lv_obj_t *btn = lv_list_add_btn(list, LV_SYMBOL_WIFI, ssid.c_str());
            lv_obj_add_event_cb(btn, [](lv_event_t *e){
                lv_obj_t *target = lv_event_get_target(e);
                lv_obj_t *parent = lv_obj_get_parent(target);
                const char *t = lv_list_get_btn_text(parent, target);
                if (t) strlcpy(g_wifi_ssid, t, sizeof(g_wifi_ssid));
                weather_switch_screen(build_wifi_config_page);
            }, LV_EVENT_CLICKED, NULL);
        }
    }
    WiFi.scanDelete();
}

static void build_wifi_scan_page() {
    scr_weather = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x2c3e50), 0);
    lv_obj_t *title = lv_label_create(scr_weather);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, "扫描 WiFi");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_t *list = lv_list_create(scr_weather);
    lv_obj_set_size(list, 220, 220);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 10);
    lv_obj_t *lbl = lv_label_create(list);
    lv_obj_add_style(lbl, &style_cn, 0);
    lv_label_set_text(lbl, "正在扫描...");
    ensure_ap_sta_mode();
    WiFi.disconnect(false, false);
    vTaskDelay(pdMS_TO_TICKS(100));
    g_scan_done = false;
    xTaskCreatePinnedToCore([](void*){ WiFi.scanDelete(); WiFi.scanNetworks(false, true); g_scan_done = true; vTaskDelete(NULL); },
        "WifiScan", 4096, NULL, 1, NULL, 0);
    g_scan_poll_timer = lv_timer_create(scan_poll_timer_cb, 500, (void*)list);
    lv_obj_t *btn = lv_btn_create(scr_weather);
    lv_obj_set_size(btn, 80, 30);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_t *lb = lv_label_create(btn);
    lv_obj_add_style(lb, &style_cn, 0);
    lv_label_set_text(lb, "返回");
    lv_obj_center(lb);
    lv_obj_add_event_cb(btn, [](lv_event_t*){
        if (g_scan_poll_timer) { lv_timer_del(g_scan_poll_timer); g_scan_poll_timer = NULL; }
        weather_switch_screen(build_wifi_config_page);
    }, LV_EVENT_CLICKED, NULL);
}

// ============ 城市选择页 ============
static lv_obj_t *g_city_list = NULL;
static lv_obj_t *g_city_search = NULL;

static void rebuild_city_list(const char* filter) {
    if (!g_city_list) return;
    lv_obj_clean(g_city_list);
    for (int i = 0; i < g_cities_count; i++) {
        if (filter && strlen(filter) > 0 && !strstr(g_cities[i].name, filter)) continue;
        lv_obj_t *btn = lv_list_add_btn(g_city_list, LV_SYMBOL_GPS, g_cities[i].name);
        lv_obj_t *lbl = lv_obj_get_child(btn, 1);
        if (lbl) lv_obj_add_style(lbl, &style_cn, 0);
        lv_obj_add_event_cb(btn, [](lv_event_t *e){
            const char* id = (const char*)lv_event_get_user_data(e);
            for (int j = 0; j < g_cities_count; j++) {
                if (strcmp(g_cities[j].id, id) == 0) {
                    strlcpy(g_weather_city, g_cities[j].id, sizeof(g_weather_city));
                    strlcpy(g_weather_city_display, g_cities[j].name, sizeof(g_weather_city_display));
                    break;
                }
            }
            weather_switch_screen(build_wifi_config_page);
        }, LV_EVENT_CLICKED, (void*)g_cities[i].id);
    }
}

static void city_search_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        const char* txt = lv_textarea_get_text(lv_event_get_target(e));
        rebuild_city_list(txt);
    }
    safe_kb_focus_cb(e);
}

static void build_city_select_page() {
    scr_weather = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x2c3e50), 0);
    kb_weather = lv_keyboard_create(scr_weather);
    lv_obj_add_flag(kb_weather, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *title = lv_label_create(scr_weather);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, "选择城市");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);
    g_city_search = lv_textarea_create(scr_weather);
    lv_textarea_set_one_line(g_city_search, true);
    lv_textarea_set_placeholder_text(g_city_search, "搜索城市...");
    lv_obj_set_size(g_city_search, 220, 32);
    lv_obj_align(g_city_search, LV_ALIGN_TOP_MID, 0, 34);
    lv_obj_add_event_cb(g_city_search, city_search_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(g_city_search, city_search_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(g_city_search, city_search_cb, LV_EVENT_DEFOCUSED, NULL);
    g_city_list = lv_list_create(scr_weather);
    lv_obj_set_size(g_city_list, 220, 210);
    lv_obj_align(g_city_list, LV_ALIGN_TOP_MID, 0, 72);
    lv_obj_set_style_bg_color(g_city_list, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(g_city_list, 0, 0);
    rebuild_city_list(NULL);
    lv_obj_t *btn_back = lv_btn_create(scr_weather);
    lv_obj_set_size(btn_back, 80, 30);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x7f8c8d), 0);
    lv_obj_t *lb = lv_label_create(btn_back);
    lv_obj_add_style(lb, &style_cn, 0);
    lv_label_set_text(lb, "返回");
    lv_obj_center(lb);
    lv_obj_add_event_cb(btn_back, [](lv_event_t*){ weather_switch_screen(build_wifi_config_page); },
        LV_EVENT_CLICKED, NULL);
}

// ============ 初始化任务 ============
static void init_weather_network_task(void* param) {
    g_net_task_running = true;
    g_init_state = INIT_CONNECTING_WIFI;
    if (g_wifi_ssid[0]) connect_wifi_station(g_wifi_ssid, g_wifi_password);
    if (WiFi.status() != WL_CONNECTED) {
        g_init_state = INIT_DONE_FAIL;
        g_net_task_running = false;
        vTaskDelete(NULL);
        return;
    }
    g_init_state = INIT_SYNCING_NTP;
    sync_ntp_time();
    g_init_state = INIT_FETCHING_WEATHER;
    fetch_weather_data();
    if (g_net_task_running) {
        bool weather_ok = (strcmp((char*)g_weather_tmp, "--") != 0);
        g_init_state = (g_time_synced || weather_ok) ? INIT_DONE_SUCCESS : INIT_DONE_FAIL;
    }
    g_net_task_running = false;
    vTaskDelete(NULL);
}

static void init_poll_timer_cb(lv_timer_t *timer) {
    if (g_init_state == INIT_DONE_SUCCESS) {
        if (g_init_poll_timer) { lv_timer_del(g_init_poll_timer); g_init_poll_timer = NULL; }
        g_weather_running = true;
        lv_obj_t *old_scr = scr_weather;
        scr_weather = NULL;
        lbl_time = lbl_date = lbl_weather_info = lbl_temp_info = kb_weather = NULL;
        g_hour_hand = g_min_hand = g_sec_hand = NULL;
        build_weather_clock_page();
        if (scr_weather) lv_scr_load(scr_weather);
        if (old_scr) lv_obj_del(old_scr);
        if (g_clock_timer) lv_timer_del(g_clock_timer);
        g_clock_timer = lv_timer_create(clock_timer_cb, 1000, NULL);
        g_init_state = INIT_IDLE;
    } else if (g_init_state == INIT_DONE_FAIL) {
        if (g_init_poll_timer) { lv_timer_del(g_init_poll_timer); g_init_poll_timer = NULL; }
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, "连接失败");
        g_init_state = INIT_IDLE;
    } else if (g_init_state == INIT_CONNECTING_WIFI) {
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, "正在连接WiFi...");
    } else if (g_init_state == INIT_SYNCING_NTP) {
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, "正在同步时间...");
    } else if (g_init_state == INIT_FETCHING_WEATHER) {
        if (lbl_weather_info) lv_label_set_text(lbl_weather_info, "正在获取天气...");
    }
}

// ============ 样式选择页 ============
static void style_btn_event_cb(lv_event_t *e) {
    int idx = (int)(uintptr_t)lv_event_get_user_data(e);
    g_clock_style = (ClockStyle_t)idx;
    for (int i = 0; i < 4; i++) {
        if (g_style_btns[i]) lv_obj_set_style_bg_color(g_style_btns[i], lv_color_hex(0x34495e), 0);
    }
    if (g_style_btns[idx]) lv_obj_set_style_bg_color(g_style_btns[idx], lv_color_hex(0x27ae60), 0);
}

static void confirm_btn_event_cb(lv_event_t *e) {
    if (g_init_state != INIT_IDLE) return;
    g_time_synced = false;
    g_init_state = INIT_CONNECTING_WIFI;
    if (g_init_poll_timer) lv_timer_del(g_init_poll_timer);
    g_init_poll_timer = lv_timer_create(init_poll_timer_cb, 300, NULL);
    if (!lbl_weather_info) {
        lbl_weather_info = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_weather_info, &style_cn, 0);
        lv_obj_align(lbl_weather_info, LV_ALIGN_CENTER, 0, 80);
    }
    lv_label_set_text(lbl_weather_info, "正在连接...");
    lv_obj_set_style_text_color(lbl_weather_info, lv_color_hex(0xffd43b), 0);
    if (xTaskCreate(init_weather_network_task, "WeatherNet", 16384, NULL, 2, NULL) != pdPASS) {
        g_init_state = INIT_DONE_FAIL;
    }
}

static void back_btn_event_cb(lv_event_t *e) {
    g_weather_running = false;
    if (g_clock_timer) { lv_timer_del(g_clock_timer); g_clock_timer = NULL; }
    if (g_init_poll_timer) { lv_timer_del(g_init_poll_timer); g_init_poll_timer = NULL; }
    if (g_scan_poll_timer) { lv_timer_del(g_scan_poll_timer); g_scan_poll_timer = NULL; }
    exit_landscape();
    if (scr_menu) lv_scr_load(scr_menu);
    if (scr_weather) { lv_obj_del(scr_weather); scr_weather = NULL; }
    lbl_time = lbl_date = lbl_weather_info = lbl_temp_info = kb_weather = NULL;
    g_hour_hand = g_min_hand = g_sec_hand = NULL;
    if (!g_net_task_running) g_init_state = INIT_IDLE;
}

static void build_style_select_page() {
    g_init_state = INIT_IDLE;
    for (int i = 0; i < 4; i++) g_style_btns[i] = NULL;
    scr_weather = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x2c3e50), 0);
    lv_obj_t *title = lv_label_create(scr_weather);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, "天气时钟");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_t *subtitle = lv_label_create(scr_weather);
    lv_obj_add_style(subtitle, &style_cn, 0);
    lv_label_set_text(subtitle, "选择时钟样式");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x95a5a6), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 40);
    const char* names[] = {"数字时钟","模拟时钟","极简时钟","复古时钟"};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_btn_create(scr_weather);
        g_style_btns[i] = btn;
        lv_obj_set_size(btn, 200, 40);
        lv_obj_align(btn, LV_ALIGN_CENTER, 0, -65 + i * 50);
        lv_obj_set_style_bg_color(btn, lv_color_hex(i == g_clock_style ? 0x27ae60 : 0x34495e), 0);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_t *lb = lv_label_create(btn);
        lv_obj_add_style(lb, &style_cn, 0);
        lv_label_set_text(lb, names[i]);
        lv_obj_set_style_text_color(lb, lv_color_hex(0xecf0f1), 0);
        lv_obj_center(lb);
        lv_obj_add_event_cb(btn, style_btn_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }
    lv_obj_t *ctn = lv_obj_create(scr_weather);
    lv_obj_set_size(ctn, 240, 45);
    lv_obj_align(ctn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(ctn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctn, 0, 0);
    lv_obj_set_flex_flow(ctn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctn, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ctn, LV_OBJ_FLAG_SCROLLABLE);
    auto mk_btn = [&](const char* txt, uint32_t col, lv_event_cb_t cb){
        lv_obj_t *b = lv_btn_create(ctn);
        lv_obj_set_size(b, 60, 35);
        lv_obj_set_style_bg_color(b, lv_color_hex(col), 0);
        lv_obj_t *lb = lv_label_create(b);
        lv_obj_add_style(lb, &style_cn, 0);
        lv_label_set_text(lb, txt);
        lv_obj_center(lb);
        lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
    };
    mk_btn("返回", 0x7f8c8d, back_btn_event_cb);
    mk_btn("确定", 0x27ae60, confirm_btn_event_cb);
    mk_btn("配网", 0x2980b9, [](lv_event_t*){ weather_switch_screen(build_wifi_config_page); });
}

// ============ WiFi 配置页 ============
static void wifi_save_btn_event_cb(lv_event_t *e) {
    lv_obj_t *page = lv_scr_act();
    int idx = 0;
    for (int i = 0; i < (int)lv_obj_get_child_cnt(page); i++) {
        lv_obj_t *child = lv_obj_get_child(page, i);
        if (lv_obj_check_type(child, &lv_textarea_class)) {
            const char* t = lv_textarea_get_text(child);
            if (idx == 0) strlcpy(g_wifi_ssid, t, sizeof(g_wifi_ssid));
            else if (idx == 1) strlcpy(g_wifi_password, t, sizeof(g_wifi_password));
            else if (idx == 2) strlcpy(g_weather_api_key, t, sizeof(g_weather_api_key));
            idx++;
        }
    }
    save_weather_config();
    weather_switch_screen(build_style_select_page);
}

static void build_wifi_config_page() {
    scr_weather = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x2c3e50), 0);
    kb_weather = lv_keyboard_create(scr_weather);
    lv_obj_add_flag(kb_weather, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *title = lv_label_create(scr_weather);
    lv_obj_add_style(title, &style_cn, 0);
    lv_label_set_text(title, "WiFi 配置");
    lv_obj_set_style_text_color(title, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // UI组件代码同上省略，保持不变
    lv_obj_t *l1 = lv_label_create(scr_weather);
    lv_obj_add_style(l1, &style_cn, 0);
    lv_label_set_text(l1, "WiFi名称:");
    lv_obj_set_style_text_color(l1, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(l1, LV_ALIGN_TOP_LEFT, 15, 40);
    lv_obj_t *txt_ssid = lv_textarea_create(scr_weather);
    lv_textarea_set_one_line(txt_ssid, true);
    lv_obj_set_size(txt_ssid, 155, 32);
    lv_obj_align(txt_ssid, LV_ALIGN_TOP_LEFT, 15, 60);
    if (g_wifi_ssid[0]) lv_textarea_set_text(txt_ssid, g_wifi_ssid);
    lv_obj_add_event_cb(txt_ssid, safe_kb_focus_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(txt_ssid, safe_kb_focus_cb, LV_EVENT_DEFOCUSED, NULL);
    
    lv_obj_t *btn_scan = lv_btn_create(scr_weather);
    lv_obj_set_size(btn_scan, 40, 32);
    lv_obj_align(btn_scan, LV_ALIGN_TOP_RIGHT, -15, 60);
    lv_obj_set_style_bg_color(btn_scan, lv_color_hex(0x2980b9), 0);
    lv_obj_t *sci = lv_label_create(btn_scan);
    lv_label_set_text(sci, LV_SYMBOL_REFRESH);
    lv_obj_center(sci);
    lv_obj_add_event_cb(btn_scan, [](lv_event_t*){ weather_switch_screen(build_wifi_scan_page); }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *l2 = lv_label_create(scr_weather);
    lv_obj_add_style(l2, &style_cn, 0);
    lv_label_set_text(l2, "WiFi密码:");
    lv_obj_set_style_text_color(l2, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(l2, LV_ALIGN_TOP_LEFT, 15, 100);
    lv_obj_t *txt_pwd = lv_textarea_create(scr_weather);
    lv_textarea_set_one_line(txt_pwd, true);
    lv_textarea_set_password_mode(txt_pwd, true);
    lv_obj_set_size(txt_pwd, 205, 32);
    lv_obj_align(txt_pwd, LV_ALIGN_TOP_LEFT, 15, 120);
    if (g_wifi_password[0]) lv_textarea_set_text(txt_pwd, g_wifi_password);
    lv_obj_add_event_cb(txt_pwd, safe_kb_focus_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(txt_pwd, safe_kb_focus_cb, LV_EVENT_DEFOCUSED, NULL);
    
    lv_obj_t *l3 = lv_label_create(scr_weather);
    lv_obj_add_style(l3, &style_cn, 0);
    lv_label_set_text(l3, "城市:");
    lv_obj_set_style_text_color(l3, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(l3, LV_ALIGN_TOP_LEFT, 15, 160);
    lv_obj_t *btn_city = lv_btn_create(scr_weather);
    lv_obj_set_size(btn_city, 205, 32);
    lv_obj_align(btn_city, LV_ALIGN_TOP_LEFT, 15, 180);
    lv_obj_set_style_bg_color(btn_city, lv_color_hex(0x3498db), 0);
    lv_obj_t *lbc = lv_label_create(btn_city);
    lv_obj_add_style(lbc, &style_cn, 0);
    char citybuf[48];
    snprintf(citybuf, sizeof(citybuf), LV_SYMBOL_GPS " %s", g_weather_city_display);
    lv_label_set_text(lbc, citybuf);
    lv_obj_center(lbc);
    lv_obj_add_event_cb(btn_city, [](lv_event_t*){ weather_switch_screen(build_city_select_page); }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *l4 = lv_label_create(scr_weather);
    lv_obj_add_style(l4, &style_cn, 0);
    lv_label_set_text(l4, "API Key:");
    lv_obj_set_style_text_color(l4, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(l4, LV_ALIGN_TOP_LEFT, 15, 220);
    lv_obj_t *txt_api = lv_textarea_create(scr_weather);
    lv_textarea_set_one_line(txt_api, true);
    lv_obj_set_size(txt_api, 205, 32);
    lv_obj_align(txt_api, LV_ALIGN_TOP_LEFT, 15, 240);
    lv_textarea_set_text(txt_api, g_weather_api_key);
    lv_obj_add_event_cb(txt_api, safe_kb_focus_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(txt_api, safe_kb_focus_cb, LV_EVENT_DEFOCUSED, NULL);
    
    lv_obj_t *bs = lv_btn_create(scr_weather);
    lv_obj_set_size(bs, 80, 32);
    lv_obj_align(bs, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(bs, lv_color_hex(0x27ae60), 0);
    lv_obj_t *lbs = lv_label_create(bs);
    lv_obj_add_style(lbs, &style_cn, 0);
    lv_label_set_text(lbs, "保存");
    lv_obj_center(lbs);
    lv_obj_add_event_cb(bs, wifi_save_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *bb = lv_btn_create(scr_weather);
    lv_obj_set_size(bb, 60, 32);
    lv_obj_align(bb, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(bb, lv_color_hex(0x7f8c8d), 0);
    lv_obj_t *lbb = lv_label_create(bb);
    lv_obj_add_style(lbb, &style_cn, 0);
    lv_label_set_text(lbb, "返回");
    lv_obj_center(lbb);
    lv_obj_add_event_cb(bb, [](lv_event_t*){ weather_switch_screen(build_style_select_page); }, LV_EVENT_CLICKED, NULL);
}

// ============ 时钟页面 ============
static void clock_touch_event_cb(lv_event_t *e) {
    g_weather_running = false;
    if (g_clock_timer) { lv_timer_del(g_clock_timer); g_clock_timer = NULL; }
    exit_landscape();
    if (scr_menu) lv_scr_load(scr_menu);
    if (scr_weather) { lv_obj_del(scr_weather); scr_weather = NULL; }
    lbl_time = lbl_date = lbl_weather_info = lbl_temp_info = kb_weather = NULL;
    g_hour_hand = g_min_hand = g_sec_hand = NULL;
    if (!g_net_task_running) g_init_state = INIT_IDLE;
}

static void build_weather_clock_page() {
    if (g_clock_style == CLOCK_STYLE_MINIMAL) enter_landscape();
    else exit_landscape();
    scr_weather = lv_obj_create(NULL);
    lv_obj_clear_flag(scr_weather, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(scr_weather, 0, 0);
    lv_obj_set_style_border_width(scr_weather, 0, 0);
    
    auto mk_line = [&](lv_obj_t *par, int w, lv_align_t al, int x, int y, uint32_t col){
        lv_obj_t *ln = lv_obj_create(par);
        lv_obj_set_size(ln, w, 2);
        lv_obj_align(ln, al, x, y);
        lv_obj_set_style_bg_color(ln, lv_color_hex(col), 0);
        lv_obj_set_style_border_width(ln, 0, 0);
        lv_obj_set_style_radius(ln, 1, 0);
        lv_obj_set_style_pad_all(ln, 0, 0);
        lv_obj_clear_flag(ln, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(ln, LV_OBJ_FLAG_CLICKABLE);
        return ln;
    };
    
    if (g_clock_style == CLOCK_STYLE_DIGITAL) {
        // ============ 高清数字时钟 ============
        lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x0a1628), 0);
        lv_obj_set_style_bg_grad_color(scr_weather, lv_color_hex(0x1e3a5f), 0);
        lv_obj_set_style_bg_grad_dir(scr_weather, LV_GRAD_DIR_VER, 0);
        
        lv_obj_t *lbl_city = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_city, &style_cn, 0);
        lv_label_set_text(lbl_city, g_weather_city_display);
        lv_obj_set_style_text_color(lbl_city, lv_color_hex(0x4cc9f0), 0);
        lv_obj_align(lbl_city, LV_ALIGN_TOP_MID, 0, 20);
        mk_line(scr_weather, 180, LV_ALIGN_TOP_MID, 0, 48, 0x2a4166);
        
        // ★ 高清超大时间 (使用原生 48 像素字体)
        lbl_time = lv_label_create(scr_weather);
        lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_48, 0);
        lv_label_set_text(lbl_time, "--:--");
        lv_obj_set_style_text_color(lbl_time, lv_color_hex(0x00ff88), 0);
        lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, -45); // 微调位置让布局更和谐
        
        // 日期
        lbl_date = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_date, &style_cn, 0);
        lv_label_set_text(lbl_date, "加载中...");
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0xcccccc), 0);
        lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 15);
        mk_line(scr_weather, 180, LV_ALIGN_CENTER, 0, 40, 0x2a4166);
        
        // 天气信息
        lbl_weather_info = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_weather_info, &style_cn, 0);
        lv_label_set_text(lbl_weather_info, "-- -- 度");
        lv_obj_set_style_text_color(lbl_weather_info, lv_color_hex(0xffd700), 0);
        lv_obj_align(lbl_weather_info, LV_ALIGN_CENTER, 0, 65);
        
        // 附带信息（秒和湿度）
        lbl_temp_info = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_temp_info, &style_cn, 0);
        lv_label_set_text(lbl_temp_info, "--秒 | 湿度 --%");
        lv_obj_set_style_text_color(lbl_temp_info, lv_color_hex(0x88aacc), 0);
        lv_obj_align(lbl_temp_info, LV_ALIGN_CENTER, 0, 95);
        
        lv_obj_t *hint = lv_label_create(scr_weather);
        lv_obj_add_style(hint, &style_cn, 0);
        lv_label_set_text(hint, "点击任意处返回");
        lv_obj_set_style_text_color(hint, lv_color_hex(0x556688), 0);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);

    } else if (g_clock_style == CLOCK_STYLE_ANALOG) {
        // ============ 模拟时钟 ============
        lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x1a2332), 0);
        lv_obj_set_style_bg_grad_color(scr_weather, lv_color_hex(0x2d3e50), 0);
        lv_obj_set_style_bg_grad_dir(scr_weather, LV_GRAD_DIR_VER, 0);
        lv_obj_t *lbl_city = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_city, &style_cn, 0);
        lv_label_set_text(lbl_city, g_weather_city_display);
        lv_obj_set_style_text_color(lbl_city, lv_color_hex(0xffd700), 0);
        lv_obj_align(lbl_city, LV_ALIGN_TOP_MID, 0, 12);
        lv_obj_t *face = lv_obj_create(scr_weather);
        lv_obj_set_size(face, 200, 200);
        lv_obj_align(face, LV_ALIGN_CENTER, 0, -10);
        lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(face, lv_color_hex(0xf8f5e8), 0);
        lv_obj_set_style_border_color(face, lv_color_hex(0xc9a86b), 0);
        lv_obj_set_style_border_width(face, 4, 0);
        lv_obj_set_style_pad_all(face, 0, 0);
        lv_obj_clear_flag(face, LV_OBJ_FLAG_SCROLLABLE);
        const char* nums[] = {"12","1","2","3","4","5","6","7","8","9","10","11"};
        for (int i = 0; i < 12; i++) {
            float a = i * 30.0f * 3.14159f / 180.0f;
            int x = 96 + (int)(sinf(a) * 82) - 8;
            int y = 96 - (int)(cosf(a) * 82) - 10;
            lv_obj_t *n = lv_label_create(face);
            lv_label_set_text(n, nums[i]);
            lv_obj_set_style_text_color(n, (i%3==0) ? lv_color_hex(0xc0392b) : lv_color_hex(0x2c3e50), 0);
            lv_obj_set_pos(n, x, y);
        }
        lv_obj_t *dot = lv_obj_create(face);
        lv_obj_set_size(dot, 10, 10);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, lv_color_hex(0xc0392b), 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_pos(dot, 91, 91);
        g_hour_pts[0] = {120, 140}; g_hour_pts[1] = {120, 105};
        g_hour_hand = lv_line_create(scr_weather);
        lv_line_set_points(g_hour_hand, g_hour_pts, 2);
        lv_obj_set_style_line_color(g_hour_hand, lv_color_hex(0x2c3e50), 0);
        lv_obj_set_style_line_width(g_hour_hand, 6, 0);
        lv_obj_set_style_line_rounded(g_hour_hand, true, 0);
        g_min_pts[0] = {120, 140}; g_min_pts[1] = {120, 80};
        g_min_hand = lv_line_create(scr_weather);
        lv_line_set_points(g_min_hand, g_min_pts, 2);
        lv_obj_set_style_line_color(g_min_hand, lv_color_hex(0x34495e), 0);
        lv_obj_set_style_line_width(g_min_hand, 4, 0);
        lv_obj_set_style_line_rounded(g_min_hand, true, 0);
        g_sec_pts[0] = {120, 140}; g_sec_pts[1] = {120, 68};
        g_sec_hand = lv_line_create(scr_weather);
        lv_line_set_points(g_sec_hand, g_sec_pts, 2);
        lv_obj_set_style_line_color(g_sec_hand, lv_color_hex(0xc0392b), 0);
        lv_obj_set_style_line_width(g_sec_hand, 2, 0);
        lv_obj_set_style_line_rounded(g_sec_hand, true, 0);
        lbl_date = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_date, &style_cn, 0);
        lv_label_set_text(lbl_date, "加载中...");
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0xecf0f1), 0);
        lv_obj_align(lbl_date, LV_ALIGN_BOTTOM_MID, 0, -40);
        lbl_weather_info = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_weather_info, &style_cn, 0);
        lv_label_set_text(lbl_weather_info, "-- -- 度");
        lv_obj_set_style_text_color(lbl_weather_info, lv_color_hex(0xffd700), 0);
        lv_obj_align(lbl_weather_info, LV_ALIGN_BOTTOM_MID, 0, -15);

    } else if (g_clock_style == CLOCK_STYLE_MINIMAL) {
        // ============ 高清极简时钟 ============
        lv_obj_set_size(scr_weather, 320, 240);
        lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x050518), 0);
        lv_obj_set_style_bg_grad_color(scr_weather, lv_color_hex(0x0a0a28), 0);
        lv_obj_set_style_bg_grad_dir(scr_weather, LV_GRAD_DIR_VER, 0);
        
        lv_obj_t *city_lbl = lv_label_create(scr_weather);
        lv_obj_add_style(city_lbl, &style_cn, 0);
        lv_label_set_text(city_lbl, g_weather_city_display);
        lv_obj_set_style_text_color(city_lbl, lv_color_hex(0x5faee3), 0);
        lv_obj_align(city_lbl, LV_ALIGN_TOP_LEFT, 22, 16);
        
        lbl_date = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_date, &style_cn, 0);
        lv_label_set_text(lbl_date, "--/--");
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0x6b7a8f), 0);
        lv_obj_align(lbl_date, LV_ALIGN_TOP_RIGHT, -22, 16);
        mk_line(scr_weather, 280, LV_ALIGN_TOP_MID, 0, 44, 0x1a2438);
        
        // ★ 高清大时间 (原生 48 像素字体)
        lbl_time = lv_label_create(scr_weather);
        lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_48, 0);
        lv_label_set_text(lbl_time, "--:--");
        lv_obj_set_style_text_color(lbl_time, lv_color_hex(0xffffff), 0);
        lv_obj_align(lbl_time, LV_ALIGN_CENTER, -15, 0); 
        
        // 秒 (原生 32 像素字体，作为后缀)
        lbl_temp_info = lv_label_create(scr_weather);
        lv_obj_set_style_text_font(lbl_temp_info, &lv_font_montserrat_32, 0);
        lv_label_set_text(lbl_temp_info, ":00");
        lv_obj_set_style_text_color(lbl_temp_info, lv_color_hex(0x5faee3), 0);
        lv_obj_align_to(lbl_temp_info, lbl_time, LV_ALIGN_OUT_RIGHT_BOTTOM, 22, -8);
        
        mk_line(scr_weather, 280, LV_ALIGN_BOTTOM_MID, 0, -44, 0x1a2438);
        
        lbl_weather_info = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_weather_info, &style_cn, 0);
        lv_label_set_text(lbl_weather_info, "加载中...");
        lv_obj_set_style_text_color(lbl_weather_info, lv_color_hex(0xa0b8d0), 0);
        lv_obj_align(lbl_weather_info, LV_ALIGN_BOTTOM_MID, 0, -20);
        
        lv_obj_t *hint = lv_label_create(scr_weather);
        lv_obj_add_style(hint, &style_cn, 0);
        lv_label_set_text(hint, "点击返回");
        lv_obj_set_style_text_color(hint, lv_color_hex(0x2a3448), 0);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 12, -6);

    } else if (g_clock_style == CLOCK_STYLE_RETRO) {
        // ============ 高清复古时钟 ============
        lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x3d2817), 0);
        lv_obj_set_style_bg_grad_color(scr_weather, lv_color_hex(0x1a0f00), 0);
        lv_obj_set_style_bg_grad_dir(scr_weather, LV_GRAD_DIR_VER, 0);
        
        lv_obj_t *lbl_city = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_city, &style_cn, 0);
        lv_label_set_text(lbl_city, g_weather_city_display);
        lv_obj_set_style_text_color(lbl_city, lv_color_hex(0xffb74d), 0);
        lv_obj_align(lbl_city, LV_ALIGN_TOP_MID, 0, 18);
        mk_line(scr_weather, 200, LV_ALIGN_TOP_MID, 0, 42, 0x8b6914);
        
        // ★ 高清大时间 (原生 32 像素字体，为了装得下时分秒)
        lbl_time = lv_label_create(scr_weather);
        lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_32, 0);
        lv_label_set_text(lbl_time, "--:--:--");
        lv_obj_set_style_text_color(lbl_time, lv_color_hex(0x7fff7f), 0);
        lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, -45);
        
        lbl_date = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_date, &style_cn, 0);
        lv_label_set_text(lbl_date, "--/--/--");
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0xffaa44), 0);
        lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 10);
        mk_line(scr_weather, 200, LV_ALIGN_CENTER, 0, 35, 0x8b6914);
        
        lbl_weather_info = lv_label_create(scr_weather);
        lv_obj_add_style(lbl_weather_info, &style_cn, 0);
        lv_label_set_text(lbl_weather_info, "加载中...");
        lv_obj_set_style_text_color(lbl_weather_info, lv_color_hex(0xffe082), 0);
        lv_obj_align(lbl_weather_info, LV_ALIGN_CENTER, 0, 65);
        
        mk_line(scr_weather, 200, LV_ALIGN_BOTTOM_MID, 0, -35, 0x8b6914);
        lv_obj_t *hint = lv_label_create(scr_weather);
        lv_obj_add_style(hint, &style_cn, 0);
        lv_label_set_text(hint, "点击返回");
        lv_obj_set_style_text_color(hint, lv_color_hex(0x8b6914), 0);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);
    }
    lv_obj_add_flag(scr_weather, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr_weather, clock_touch_event_cb, LV_EVENT_CLICKED, NULL);
}

// ============ 对外接口 ============
void build_weather_clock_scene() {
    load_weather_config();
    build_style_select_page();
    lv_scr_load_anim(scr_weather, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void update_weather_config_from_web(const char* ssid, const char* pwd, const char* city, const char* api) {
    if (ssid) strlcpy(g_wifi_ssid, ssid, sizeof(g_wifi_ssid));
    if (pwd) strlcpy(g_wifi_password, pwd, sizeof(g_wifi_password));
    if (city) {
        strlcpy(g_weather_city, city, sizeof(g_weather_city));
        strlcpy(g_weather_city_display, find_city_name(city), sizeof(g_weather_city_display));
    }
    if (api) strlcpy(g_weather_api_key, api, sizeof(g_weather_api_key));
    save_weather_config();
}