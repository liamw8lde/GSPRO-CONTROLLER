/**
 * GSPro Controller for WT32-SC01 Plus V3.3
 * ULTRA MODERN UI EDITION
 *
 * ESP32-S3 with 3.5" IPS Touchscreen (480x320)
 * Bluetooth HID Keyboard for GSPro Golf Simulator
 *
 * Features:
 * - Glassmorphism UI design
 * - Neon glow effects
 * - Smooth animations
 * - Modern dark theme with accent colors
 */

#include <Arduino.h>
#include <BleKeyboard.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lvgl.h>

// ============================================================================
// Color Palette - Modern Neon Theme
// ============================================================================
#define COLOR_BG_DARK       0x0D0D0F
#define COLOR_BG_CARD       0x1A1A2E
#define COLOR_BG_CARD_LIGHT 0x252542
#define COLOR_ACCENT_CYAN   0x00F5FF
#define COLOR_ACCENT_PINK   0xFF006E
#define COLOR_ACCENT_PURPLE 0x8B5CF6
#define COLOR_ACCENT_GREEN  0x00FF88
#define COLOR_ACCENT_ORANGE 0xFF9500
#define COLOR_ACCENT_BLUE   0x3B82F6
#define COLOR_TEXT_PRIMARY  0xFFFFFF
#define COLOR_TEXT_SECONDARY 0x9CA3AF
#define COLOR_TEXT_MUTED    0x4B5563
#define COLOR_GLASS_BORDER  0x374151
#define COLOR_SUCCESS       0x10B981
#define COLOR_ERROR         0xEF4444
#define COLOR_WARNING       0xF59E0B

// ============================================================================
// Display Configuration for WT32-SC01 Plus
// ============================================================================
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7796 _panel_instance;
    lgfx::Bus_Parallel8 _bus_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_FT5x06 _touch_instance;

public:
    LGFX(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.freq_write = 20000000;
            cfg.pin_wr = 47;
            cfg.pin_rd = -1;
            cfg.pin_rs = 0;
            cfg.pin_d0 = 9;
            cfg.pin_d1 = 46;
            cfg.pin_d2 = 3;
            cfg.pin_d3 = 8;
            cfg.pin_d4 = 18;
            cfg.pin_d5 = 17;
            cfg.pin_d6 = 16;
            cfg.pin_d7 = 15;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = -1;
            cfg.pin_rst = 4;
            cfg.pin_busy = -1;
            cfg.memory_width = 320;
            cfg.memory_height = 480;
            cfg.panel_width = 320;
            cfg.panel_height = 480;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 1;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;
            _panel_instance.config(cfg);
        }
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = 45;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
        {
            auto cfg = _touch_instance.config();
            cfg.x_min = 0;
            cfg.x_max = 319;
            cfg.y_min = 0;
            cfg.y_max = 479;
            cfg.pin_int = 7;
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;
            cfg.i2c_port = 1;
            cfg.i2c_addr = 0x38;
            cfg.pin_sda = 6;
            cfg.pin_scl = 5;
            cfg.freq = 400000;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
        setPanel(&_panel_instance);
    }
};

// ============================================================================
// Global Objects
// ============================================================================
LGFX lcd;
BleKeyboard bleKeyboard("GSPro Controller", "ESP32", 100);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[480 * 30];
static lv_color_t buf2[480 * 30];

static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

// UI Elements
static lv_obj_t *status_label;
static lv_obj_t *status_icon;
static lv_obj_t *connection_indicator;
static lv_obj_t *connection_label;
static lv_obj_t *main_container;
static lv_obj_t *settings_container;
static lv_obj_t *main_screen;
static lv_obj_t *settings_screen;

// Settings screen UI elements
static lv_obj_t *wifi_status_label;
static lv_obj_t *wifi_list;
static lv_obj_t *ota_status_label;
static lv_obj_t *ota_progress_bar;
static lv_obj_t *bt_status_label;

// Screen state
enum ScreenState {
    SCREEN_MAIN,
    SCREEN_SETTINGS
};
static ScreenState current_screen = SCREEN_MAIN;

// WiFi state
static String selected_ssid = "";
static bool wifi_scanning = false;
static unsigned long last_wifi_scan = 0;

// Styles
static lv_style_t style_glass_card;
static lv_style_t style_neon_btn;
static lv_style_t style_neon_btn_pressed;
static lv_style_t style_arrow_btn;
static lv_style_t style_arrow_btn_pressed;
static lv_style_t style_special_btn;
static lv_style_t style_special_btn_pressed;

// Animation
static lv_anim_t pulse_anim;

// ============================================================================
// LVGL Callbacks
// ============================================================================
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
    lcd.endWrite();
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    bool touched = lcd.getTouch(&touchX, &touchY);
    if (touched) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// ============================================================================
// Keyboard Functions
// ============================================================================
void send_key(uint8_t key, bool with_ctrl = false) {
    if (!bleKeyboard.isConnected()) {
        lv_label_set_text(status_label, "NOT CONNECTED");
        lv_obj_set_style_text_color(status_label, lv_color_hex(COLOR_ERROR), 0);
        return;
    }

    if (with_ctrl) {
        bleKeyboard.press(KEY_LEFT_CTRL);
        delay(10);
        bleKeyboard.press(key);
        delay(50);
        bleKeyboard.releaseAll();
    } else {
        bleKeyboard.write(key);
    }
}

void update_status(const char* text, uint32_t color) {
    lv_label_set_text(status_label, text);
    lv_obj_set_style_text_color(status_label, lv_color_hex(color), 0);
}

// ============================================================================
// Button Event Handlers
// ============================================================================
static void btn_mulligan_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('m', true);
        update_status("MULLIGAN", COLOR_ACCENT_PINK);
    }
}

static void btn_pin_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('p');
        update_status("PIN INDICATOR", COLOR_ACCENT_CYAN);
    }
}

static void btn_scout_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('j');
        update_status("SCOUT VIEW", COLOR_ACCENT_PURPLE);
    }
}

static void btn_heatmap_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('y');
        update_status("HEAT MAP", COLOR_ACCENT_ORANGE);
    }
}

static void btn_freeflight_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_F5);
        update_status("FREE FLIGHT", COLOR_ACCENT_GREEN);
    }
}

static void btn_flyover_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('o');
        update_status("FLYOVER", COLOR_ACCENT_BLUE);
    }
}

static void btn_tee_left_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('c');
        update_status("TEE LEFT", COLOR_ACCENT_CYAN);
    }
}

static void btn_tee_right_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('v');
        update_status("TEE RIGHT", COLOR_ACCENT_CYAN);
    }
}

static void btn_up_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_UP_ARROW);
        update_status("AIM UP", COLOR_ACCENT_GREEN);
    }
}

static void btn_down_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_DOWN_ARROW);
        update_status("AIM DOWN", COLOR_ACCENT_GREEN);
    }
}

static void btn_left_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_LEFT_ARROW);
        update_status("AIM LEFT", COLOR_ACCENT_GREEN);
    }
}

static void btn_right_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_RIGHT_ARROW);
        update_status("AIM RIGHT", COLOR_ACCENT_GREEN);
    }
}

// ============================================================================
// Settings Screen Functions
// ============================================================================
void show_settings_screen();
void show_main_screen();

static void btn_settings_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        show_settings_screen();
    }
}

static void btn_back_to_main_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        show_main_screen();
    }
}

static void btn_wifi_scan_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_label_set_text(wifi_status_label, "Scanning...");
        wifi_scanning = true;
        WiFi.scanDelete();
        WiFi.scanNetworks(true);  // Async scan
    }
}

static void btn_wifi_disconnect_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        WiFi.disconnect();
        lv_label_set_text(wifi_status_label, "Disconnected");
    }
}

static void btn_ota_update_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_label_set_text(ota_status_label, "OTA Ready - Listening on network");
        // OTA is already initialized, just update status
    }
}

static void btn_deep_sleep_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // Show countdown
        lcd.fillScreen(TFT_BLACK);
        lcd.setTextColor(0x07FF, TFT_BLACK);  // Cyan
        lcd.setTextSize(2);
        lcd.setCursor(150, 140);
        lcd.println("Entering Deep Sleep...");
        lcd.setCursor(150, 170);
        lcd.println("Touch to wake up");
        delay(2000);

        // Turn off display backlight to save power
        lcd.setBrightness(0);

        // Configure timer wake-up every 1 second to check for touch
        // This uses minimal power while allowing touch detection
        esp_sleep_enable_timer_wakeup(1000000); // 1 second in microseconds

        // Enter deep sleep
        esp_deep_sleep_start();
    }
}

static void btn_bt_reconnect_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_label_set_text(bt_status_label, "Restarting BLE...");
        bleKeyboard.end();
        delay(1000);
        bleKeyboard.begin();
        lv_label_set_text(bt_status_label, "BLE Restarted - Pairing...");
    }
}

// ============================================================================
// Style Setup
// ============================================================================
void setup_styles() {
    // Glass Card Style
    lv_style_init(&style_glass_card);
    lv_style_set_bg_color(&style_glass_card, lv_color_hex(COLOR_BG_CARD));
    lv_style_set_bg_opa(&style_glass_card, LV_OPA_90);
    lv_style_set_border_width(&style_glass_card, 1);
    lv_style_set_border_color(&style_glass_card, lv_color_hex(COLOR_GLASS_BORDER));
    lv_style_set_border_opa(&style_glass_card, LV_OPA_50);
    lv_style_set_radius(&style_glass_card, 16);
    lv_style_set_shadow_width(&style_glass_card, 20);
    lv_style_set_shadow_color(&style_glass_card, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_glass_card, LV_OPA_30);
    lv_style_set_pad_all(&style_glass_card, 12);

    // Neon Button Style (Main actions)
    lv_style_init(&style_neon_btn);
    lv_style_set_radius(&style_neon_btn, 12);
    lv_style_set_bg_color(&style_neon_btn, lv_color_hex(COLOR_BG_CARD_LIGHT));
    lv_style_set_bg_grad_color(&style_neon_btn, lv_color_hex(COLOR_BG_CARD));
    lv_style_set_bg_grad_dir(&style_neon_btn, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_neon_btn, 2);
    lv_style_set_border_color(&style_neon_btn, lv_color_hex(COLOR_ACCENT_CYAN));
    lv_style_set_border_opa(&style_neon_btn, LV_OPA_70);
    lv_style_set_shadow_width(&style_neon_btn, 15);
    lv_style_set_shadow_color(&style_neon_btn, lv_color_hex(COLOR_ACCENT_CYAN));
    lv_style_set_shadow_opa(&style_neon_btn, LV_OPA_40);
    lv_style_set_shadow_spread(&style_neon_btn, 2);
    lv_style_set_text_color(&style_neon_btn, lv_color_hex(COLOR_TEXT_PRIMARY));
    lv_style_set_text_font(&style_neon_btn, &lv_font_montserrat_14);
    lv_style_set_pad_all(&style_neon_btn, 8);
    lv_style_set_transform_width(&style_neon_btn, 0);
    lv_style_set_transform_height(&style_neon_btn, 0);

    // Neon Button Pressed
    lv_style_init(&style_neon_btn_pressed);
    lv_style_set_bg_color(&style_neon_btn_pressed, lv_color_hex(COLOR_ACCENT_CYAN));
    lv_style_set_bg_grad_color(&style_neon_btn_pressed, lv_color_hex(0x00A5AA));
    lv_style_set_shadow_width(&style_neon_btn_pressed, 25);
    lv_style_set_shadow_opa(&style_neon_btn_pressed, LV_OPA_70);
    lv_style_set_text_color(&style_neon_btn_pressed, lv_color_hex(COLOR_BG_DARK));
    lv_style_set_transform_width(&style_neon_btn_pressed, -2);
    lv_style_set_transform_height(&style_neon_btn_pressed, -2);

    // Arrow Button Style
    lv_style_init(&style_arrow_btn);
    lv_style_set_radius(&style_arrow_btn, 14);
    lv_style_set_bg_color(&style_arrow_btn, lv_color_hex(COLOR_BG_CARD_LIGHT));
    lv_style_set_border_width(&style_arrow_btn, 2);
    lv_style_set_border_color(&style_arrow_btn, lv_color_hex(COLOR_ACCENT_GREEN));
    lv_style_set_border_opa(&style_arrow_btn, LV_OPA_80);
    lv_style_set_shadow_width(&style_arrow_btn, 12);
    lv_style_set_shadow_color(&style_arrow_btn, lv_color_hex(COLOR_ACCENT_GREEN));
    lv_style_set_shadow_opa(&style_arrow_btn, LV_OPA_50);
    lv_style_set_text_color(&style_arrow_btn, lv_color_hex(COLOR_ACCENT_GREEN));
    lv_style_set_text_font(&style_arrow_btn, &lv_font_montserrat_20);

    // Arrow Button Pressed
    lv_style_init(&style_arrow_btn_pressed);
    lv_style_set_bg_color(&style_arrow_btn_pressed, lv_color_hex(COLOR_ACCENT_GREEN));
    lv_style_set_text_color(&style_arrow_btn_pressed, lv_color_hex(COLOR_BG_DARK));
    lv_style_set_shadow_opa(&style_arrow_btn_pressed, LV_OPA_90);
    lv_style_set_transform_width(&style_arrow_btn_pressed, -2);
    lv_style_set_transform_height(&style_arrow_btn_pressed, -2);

    // Special Button Style (Mulligan, Free Flight)
    lv_style_init(&style_special_btn);
    lv_style_set_radius(&style_special_btn, 12);
    lv_style_set_bg_color(&style_special_btn, lv_color_hex(COLOR_BG_CARD_LIGHT));
    lv_style_set_border_width(&style_special_btn, 2);
    lv_style_set_border_color(&style_special_btn, lv_color_hex(COLOR_ACCENT_PINK));
    lv_style_set_border_opa(&style_special_btn, LV_OPA_80);
    lv_style_set_shadow_width(&style_special_btn, 15);
    lv_style_set_shadow_color(&style_special_btn, lv_color_hex(COLOR_ACCENT_PINK));
    lv_style_set_shadow_opa(&style_special_btn, LV_OPA_50);
    lv_style_set_text_color(&style_special_btn, lv_color_hex(COLOR_TEXT_PRIMARY));
    lv_style_set_text_font(&style_special_btn, &lv_font_montserrat_14);
    lv_style_set_pad_all(&style_special_btn, 8);

    // Special Button Pressed
    lv_style_init(&style_special_btn_pressed);
    lv_style_set_bg_color(&style_special_btn_pressed, lv_color_hex(COLOR_ACCENT_PINK));
    lv_style_set_text_color(&style_special_btn_pressed, lv_color_hex(COLOR_BG_DARK));
    lv_style_set_shadow_opa(&style_special_btn_pressed, LV_OPA_80);
}

// ============================================================================
// UI Component Creation
// ============================================================================
lv_obj_t* create_section_label(lv_obj_t *parent, const char *text, int x, int y) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(label, x, y);
    return label;
}

lv_obj_t* create_neon_button(lv_obj_t *parent, const char *label, int x, int y,
                              int w, int h, lv_event_cb_t event_cb,
                              lv_style_t *style, lv_style_t *style_pr) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_add_style(btn, style, 0);
    lv_obj_add_style(btn, style_pr, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_anim_time(btn, 100, 0);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_center(lbl);

    return btn;
}

void create_header() {
    // Header container
    lv_obj_t *header = lv_obj_create(lv_scr_act());
    lv_obj_set_size(header, 480, 50);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Logo/Title
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "GSPRO");
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(title, 15, 12);

    lv_obj_t *subtitle = lv_label_create(header);
    lv_label_set_text(subtitle, "CONTROLLER");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(COLOR_ACCENT_CYAN), 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(subtitle, 100, 18);

    // Connection indicator (pulsing dot)
    connection_indicator = lv_obj_create(header);
    lv_obj_set_size(connection_indicator, 12, 12);
    lv_obj_set_pos(connection_indicator, 380, 19);
    lv_obj_set_style_radius(connection_indicator, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(connection_indicator, lv_color_hex(COLOR_WARNING), 0);
    lv_obj_set_style_border_width(connection_indicator, 0, 0);
    lv_obj_set_style_shadow_width(connection_indicator, 8, 0);
    lv_obj_set_style_shadow_color(connection_indicator, lv_color_hex(COLOR_WARNING), 0);
    lv_obj_set_style_shadow_opa(connection_indicator, LV_OPA_70, 0);

    // Connection label
    connection_label = lv_label_create(header);
    lv_label_set_text(connection_label, "PAIRING");
    lv_obj_set_style_text_color(connection_label, lv_color_hex(COLOR_WARNING), 0);
    lv_obj_set_style_text_font(connection_label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(connection_label, 398, 17);
}

void create_status_bar() {
    // Bottom status bar
    lv_obj_t *status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, 460, 36);
    lv_obj_align(status_bar, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_add_style(status_bar, &style_glass_card, 0);
    lv_obj_set_style_pad_all(status_bar, 8, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    // Status icon
    status_icon = lv_label_create(status_bar);
    lv_label_set_text(status_icon, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(status_icon, lv_color_hex(COLOR_ACCENT_GREEN), 0);
    lv_obj_set_style_text_font(status_icon, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(status_icon, 5, 2);

    // Status label
    status_label = lv_label_create(status_bar);
    lv_label_set_text(status_label, "READY");
    lv_obj_set_style_text_color(status_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(status_label, 30, 3);

    // Keyboard hint
    lv_obj_t *hint = lv_label_create(status_bar);
    lv_label_set_text(hint, "Ctrl+M | P | J | Y | F5 | O | C/V | Arrows");
    lv_obj_set_style_text_color(hint, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_align(hint, LV_ALIGN_RIGHT_MID, -5, 0);
}

void create_main_controls() {
    // Left panel - View Controls
    lv_obj_t *left_panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(left_panel, 145, 215);
    lv_obj_set_pos(left_panel, 8, 55);
    lv_obj_add_style(left_panel, &style_glass_card, 0);
    lv_obj_clear_flag(left_panel, LV_OBJ_FLAG_SCROLLABLE);

    create_section_label(left_panel, "VIEWS", 0, -5);

    int btn_w = 118;
    int btn_h = 38;
    int spacing = 43;
    int start_y = 15;

    // Pin Indicator
    create_neon_button(left_panel, "PIN", 0, start_y, btn_w, btn_h,
                       btn_pin_handler, &style_neon_btn, &style_neon_btn_pressed);

    // Scout View
    create_neon_button(left_panel, "SCOUT", 0, start_y + spacing, btn_w, btn_h,
                       btn_scout_handler, &style_neon_btn, &style_neon_btn_pressed);

    // Heat Map
    create_neon_button(left_panel, "HEAT MAP", 0, start_y + spacing*2, btn_w, btn_h,
                       btn_heatmap_handler, &style_neon_btn, &style_neon_btn_pressed);

    // Flyover
    create_neon_button(left_panel, "FLYOVER", 0, start_y + spacing*3, btn_w, btn_h,
                       btn_flyover_handler, &style_neon_btn, &style_neon_btn_pressed);
}

void create_center_controls() {
    // Center panel - Aim Controls
    lv_obj_t *center_panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(center_panel, 170, 215);
    lv_obj_set_pos(center_panel, 158, 55);
    lv_obj_add_style(center_panel, &style_glass_card, 0);
    lv_obj_clear_flag(center_panel, LV_OBJ_FLAG_SCROLLABLE);

    create_section_label(center_panel, "AIM CONTROL", 35, -5);

    int arrow_size = 48;
    int center_x = 61;  // (170 - 48) / 2
    int center_y = 83;

    // Up
    lv_obj_t *btn_up = lv_btn_create(center_panel);
    lv_obj_set_size(btn_up, arrow_size, arrow_size);
    lv_obj_set_pos(btn_up, center_x, center_y - arrow_size - 5);
    lv_obj_add_style(btn_up, &style_arrow_btn, 0);
    lv_obj_add_style(btn_up, &style_arrow_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_up, btn_up_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_up = lv_label_create(btn_up);
    lv_label_set_text(lbl_up, LV_SYMBOL_UP);
    lv_obj_center(lbl_up);

    // Down
    lv_obj_t *btn_down = lv_btn_create(center_panel);
    lv_obj_set_size(btn_down, arrow_size, arrow_size);
    lv_obj_set_pos(btn_down, center_x, center_y + 5);
    lv_obj_add_style(btn_down, &style_arrow_btn, 0);
    lv_obj_add_style(btn_down, &style_arrow_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_down, btn_down_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_down = lv_label_create(btn_down);
    lv_label_set_text(lbl_down, LV_SYMBOL_DOWN);
    lv_obj_center(lbl_down);

    // Left
    lv_obj_t *btn_left = lv_btn_create(center_panel);
    lv_obj_set_size(btn_left, arrow_size, arrow_size);
    lv_obj_set_pos(btn_left, center_x - arrow_size - 5, center_y - arrow_size/2);
    lv_obj_add_style(btn_left, &style_arrow_btn, 0);
    lv_obj_add_style(btn_left, &style_arrow_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_left, btn_left_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_left = lv_label_create(btn_left);
    lv_label_set_text(lbl_left, LV_SYMBOL_LEFT);
    lv_obj_center(lbl_left);

    // Right
    lv_obj_t *btn_right = lv_btn_create(center_panel);
    lv_obj_set_size(btn_right, arrow_size, arrow_size);
    lv_obj_set_pos(btn_right, center_x + arrow_size + 5, center_y - arrow_size/2);
    lv_obj_add_style(btn_right, &style_arrow_btn, 0);
    lv_obj_add_style(btn_right, &style_arrow_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_right, btn_right_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_right = lv_label_create(btn_right);
    lv_label_set_text(lbl_right, LV_SYMBOL_RIGHT);
    lv_obj_center(lbl_right);

    // Center decoration
    lv_obj_t *center_dot = lv_obj_create(center_panel);
    lv_obj_set_size(center_dot, 20, 20);
    lv_obj_set_pos(center_dot, center_x + 14, center_y - 10);
    lv_obj_set_style_radius(center_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center_dot, lv_color_hex(COLOR_BG_CARD), 0);
    lv_obj_set_style_border_width(center_dot, 2, 0);
    lv_obj_set_style_border_color(center_dot, lv_color_hex(COLOR_ACCENT_GREEN), 0);
    lv_obj_set_style_border_opa(center_dot, LV_OPA_50, 0);
}

void create_right_controls() {
    // Right panel - Special Actions
    lv_obj_t *right_panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(right_panel, 135, 215);
    lv_obj_set_pos(right_panel, 335, 55);
    lv_obj_add_style(right_panel, &style_glass_card, 0);
    lv_obj_clear_flag(right_panel, LV_OBJ_FLAG_SCROLLABLE);

    create_section_label(right_panel, "ACTIONS", 0, -5);

    int btn_w = 108;
    int btn_h = 42;

    // Mulligan - Special pink button
    create_neon_button(right_panel, "MULLIGAN", 0, 15, btn_w, btn_h,
                       btn_mulligan_handler, &style_special_btn, &style_special_btn_pressed);

    // Free Flight - Special pink button
    create_neon_button(right_panel, "FREE FLT", 0, 65, btn_w, btn_h,
                       btn_freeflight_handler, &style_special_btn, &style_special_btn_pressed);

    // Tee Box section
    create_section_label(right_panel, "TEE BOX", 0, 115);

    // Tee buttons container
    int tee_y = 135;

    // Tee Left
    lv_obj_t *btn_tee_l = lv_btn_create(right_panel);
    lv_obj_set_size(btn_tee_l, 52, 42);
    lv_obj_set_pos(btn_tee_l, 0, tee_y);
    lv_obj_add_style(btn_tee_l, &style_neon_btn, 0);
    lv_obj_add_style(btn_tee_l, &style_neon_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_tee_l, btn_tee_left_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_tee_l = lv_label_create(btn_tee_l);
    lv_label_set_text(lbl_tee_l, LV_SYMBOL_LEFT " C");
    lv_obj_center(lbl_tee_l);

    // Tee Right
    lv_obj_t *btn_tee_r = lv_btn_create(right_panel);
    lv_obj_set_size(btn_tee_r, 52, 42);
    lv_obj_set_pos(btn_tee_r, 56, tee_y);
    lv_obj_add_style(btn_tee_r, &style_neon_btn, 0);
    lv_obj_add_style(btn_tee_r, &style_neon_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_tee_r, btn_tee_right_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_tee_r = lv_label_create(btn_tee_r);
    lv_label_set_text(lbl_tee_r, "V " LV_SYMBOL_RIGHT);
    lv_obj_center(lbl_tee_r);
}

void create_settings_screen() {
    // Settings screen container (hidden by default)
    settings_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(settings_screen, screenWidth, screenHeight);
    lv_obj_set_pos(settings_screen, 0, 0);
    lv_obj_set_style_bg_color(settings_screen, lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_bg_opa(settings_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(settings_screen, 0, 0);
    lv_obj_set_style_radius(settings_screen, 0, 0);
    lv_obj_set_style_pad_all(settings_screen, 0, 0);
    lv_obj_add_flag(settings_screen, LV_OBJ_FLAG_HIDDEN);  // Hidden by default
    lv_obj_clear_flag(settings_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Header for settings
    lv_obj_t *settings_header = lv_obj_create(settings_screen);
    lv_obj_set_size(settings_header, 480, 50);
    lv_obj_set_pos(settings_header, 0, 0);
    lv_obj_set_style_bg_color(settings_header, lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_bg_opa(settings_header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(settings_header, 0, 0);
    lv_obj_set_style_radius(settings_header, 0, 0);
    lv_obj_set_style_pad_all(settings_header, 0, 0);
    lv_obj_clear_flag(settings_header, LV_OBJ_FLAG_SCROLLABLE);

    // Back button
    lv_obj_t *btn_back = lv_btn_create(settings_header);
    lv_obj_set_size(btn_back, 70, 35);
    lv_obj_set_pos(btn_back, 10, 8);
    lv_obj_add_style(btn_back, &style_neon_btn, 0);
    lv_obj_add_style(btn_back, &style_neon_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_back, btn_back_to_main_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " BACK");
    lv_obj_center(lbl_back);

    // Settings title
    lv_obj_t *settings_title = lv_label_create(settings_header);
    lv_label_set_text(settings_title, "SETTINGS");
    lv_obj_set_style_text_color(settings_title, lv_color_hex(COLOR_ACCENT_CYAN), 0);
    lv_obj_set_style_text_font(settings_title, &lv_font_montserrat_24, 0);
    lv_obj_align(settings_title, LV_ALIGN_CENTER, 0, 0);

    // ========== WiFi Panel ==========
    lv_obj_t *wifi_panel = lv_obj_create(settings_screen);
    lv_obj_set_size(wifi_panel, 220, 240);
    lv_obj_set_pos(wifi_panel, 10, 55);
    lv_obj_add_style(wifi_panel, &style_glass_card, 0);
    lv_obj_clear_flag(wifi_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *wifi_title = lv_label_create(wifi_panel);
    lv_label_set_text(wifi_title, LV_SYMBOL_WIFI " WiFi");
    lv_obj_set_style_text_color(wifi_title, lv_color_hex(COLOR_ACCENT_CYAN), 0);
    lv_obj_set_style_text_font(wifi_title, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(wifi_title, 0, -5);

    wifi_status_label = lv_label_create(wifi_panel);
    lv_label_set_text(wifi_status_label, "Disconnected");
    lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
    lv_obj_set_style_text_font(wifi_status_label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(wifi_status_label, 0, 25);

    // WiFi Scan button
    lv_obj_t *btn_scan = lv_btn_create(wifi_panel);
    lv_obj_set_size(btn_scan, 90, 35);
    lv_obj_set_pos(btn_scan, 0, 50);
    lv_obj_add_style(btn_scan, &style_neon_btn, 0);
    lv_obj_add_style(btn_scan, &style_neon_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_scan, btn_wifi_scan_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_scan = lv_label_create(btn_scan);
    lv_label_set_text(lbl_scan, "SCAN");
    lv_obj_center(lbl_scan);

    // WiFi Disconnect button
    lv_obj_t *btn_disconnect = lv_btn_create(wifi_panel);
    lv_obj_set_size(btn_disconnect, 90, 35);
    lv_obj_set_pos(btn_disconnect, 100, 50);
    lv_obj_add_style(btn_disconnect, &style_special_btn, 0);
    lv_obj_add_style(btn_disconnect, &style_special_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_disconnect, btn_wifi_disconnect_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_disconnect = lv_label_create(btn_disconnect);
    lv_label_set_text(lbl_disconnect, "DISCONNECT");
    lv_obj_center(lbl_disconnect);

    // WiFi networks list (scrollable)
    wifi_list = lv_textarea_create(wifi_panel);
    lv_obj_set_size(wifi_list, 190, 120);
    lv_obj_set_pos(wifi_list, 0, 95);
    lv_textarea_set_text(wifi_list, "Press SCAN to find networks");
    lv_obj_set_style_text_font(wifi_list, &lv_font_montserrat_12, 0);
    lv_textarea_set_cursor_click_pos(wifi_list, false);

    // ========== Bluetooth Panel ==========
    lv_obj_t *bt_panel = lv_obj_create(settings_screen);
    lv_obj_set_size(bt_panel, 220, 115);
    lv_obj_set_pos(bt_panel, 240, 55);
    lv_obj_add_style(bt_panel, &style_glass_card, 0);
    lv_obj_clear_flag(bt_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *bt_title = lv_label_create(bt_panel);
    lv_label_set_text(bt_title, LV_SYMBOL_BLUETOOTH " Bluetooth");
    lv_obj_set_style_text_color(bt_title, lv_color_hex(COLOR_ACCENT_PURPLE), 0);
    lv_obj_set_style_text_font(bt_title, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(bt_title, 0, -5);

    bt_status_label = lv_label_create(bt_panel);
    lv_label_set_text(bt_status_label, "Device: GSPro Controller");
    lv_obj_set_style_text_color(bt_status_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
    lv_obj_set_style_text_font(bt_status_label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(bt_status_label, 0, 25);

    // BT Reconnect button
    lv_obj_t *btn_bt_reconnect = lv_btn_create(bt_panel);
    lv_obj_set_size(btn_bt_reconnect, 190, 38);
    lv_obj_set_pos(btn_bt_reconnect, 0, 55);
    lv_obj_add_style(btn_bt_reconnect, &style_neon_btn, 0);
    lv_obj_add_style(btn_bt_reconnect, &style_neon_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_bt_reconnect, btn_bt_reconnect_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_bt_reconnect = lv_label_create(btn_bt_reconnect);
    lv_label_set_text(lbl_bt_reconnect, "RESTART BLE");
    lv_obj_center(lbl_bt_reconnect);

    // ========== OTA Panel ==========
    lv_obj_t *ota_panel = lv_obj_create(settings_screen);
    lv_obj_set_size(ota_panel, 220, 115);
    lv_obj_set_pos(ota_panel, 240, 180);
    lv_obj_add_style(ota_panel, &style_glass_card, 0);
    lv_obj_clear_flag(ota_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *ota_title = lv_label_create(ota_panel);
    lv_label_set_text(ota_title, LV_SYMBOL_DOWNLOAD " OTA Update");
    lv_obj_set_style_text_color(ota_title, lv_color_hex(COLOR_ACCENT_ORANGE), 0);
    lv_obj_set_style_text_font(ota_title, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(ota_title, 0, -5);

    ota_status_label = lv_label_create(ota_panel);
    lv_label_set_text(ota_status_label, "Ready for updates");
    lv_obj_set_style_text_color(ota_status_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
    lv_obj_set_style_text_font(ota_status_label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(ota_status_label, 0, 25);

    ota_progress_bar = lv_bar_create(ota_panel);
    lv_obj_set_size(ota_progress_bar, 190, 15);
    lv_obj_set_pos(ota_progress_bar, 0, 48);
    lv_bar_set_value(ota_progress_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(ota_progress_bar, lv_color_hex(COLOR_BG_CARD), 0);
    lv_obj_set_style_bg_color(ota_progress_bar, lv_color_hex(COLOR_ACCENT_ORANGE), LV_PART_INDICATOR);

    // ========== Power Panel ==========
    lv_obj_t *power_panel = lv_obj_create(settings_screen);
    lv_obj_set_size(power_panel, 450, 60);
    lv_obj_set_pos(power_panel, 10, 305);
    lv_obj_add_style(power_panel, &style_glass_card, 0);
    lv_obj_clear_flag(power_panel, LV_OBJ_FLAG_SCROLLABLE);

    // Deep Sleep button
    lv_obj_t *btn_sleep = lv_btn_create(power_panel);
    lv_obj_set_size(btn_sleep, 200, 45);
    lv_obj_set_pos(btn_sleep, 120, 0);
    lv_obj_add_style(btn_sleep, &style_special_btn, 0);
    lv_obj_add_style(btn_sleep, &style_special_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_sleep, btn_deep_sleep_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_border_color(btn_sleep, lv_color_hex(COLOR_ERROR), 0);
    lv_obj_set_style_shadow_color(btn_sleep, lv_color_hex(COLOR_ERROR), 0);
    lv_obj_t *lbl_sleep = lv_label_create(btn_sleep);
    lv_label_set_text(lbl_sleep, LV_SYMBOL_POWER " DEEP SLEEP");
    lv_obj_set_style_text_font(lbl_sleep, &lv_font_montserrat_16, 0);
    lv_obj_center(lbl_sleep);

    lv_obj_t *power_hint = lv_label_create(power_panel);
    lv_label_set_text(power_hint, "Touch screen to wake (auto-detects)");
    lv_obj_set_style_text_color(power_hint, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_set_style_text_font(power_hint, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(power_hint, 95, 28);
}

void show_settings_screen() {
    if (main_screen) lv_obj_add_flag(main_screen, LV_OBJ_FLAG_HIDDEN);
    if (settings_screen) lv_obj_clear_flag(settings_screen, LV_OBJ_FLAG_HIDDEN);
    current_screen = SCREEN_SETTINGS;
}

void show_main_screen() {
    if (settings_screen) lv_obj_add_flag(settings_screen, LV_OBJ_FLAG_HIDDEN);
    if (main_screen) lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_HIDDEN);
    current_screen = SCREEN_MAIN;
}

void create_ui() {
    setup_styles();

    // Set dark background with subtle gradient effect
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    // Create main screen container
    main_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_screen, screenWidth, screenHeight);
    lv_obj_set_pos(main_screen, 0, 0);
    lv_obj_set_style_bg_opa(main_screen, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_screen, 0, 0);
    lv_obj_set_style_radius(main_screen, 0, 0);
    lv_obj_set_style_pad_all(main_screen, 0, 0);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Create UI components on main screen
    create_header();
    create_main_controls();
    create_center_controls();
    create_right_controls();
    create_status_bar();

    // Add settings button to header
    lv_obj_t *header = lv_obj_get_child(main_screen, 0);  // Get header
    lv_obj_t *btn_settings = lv_btn_create(header);
    lv_obj_set_size(btn_settings, 80, 35);
    lv_obj_set_pos(btn_settings, 280, 8);
    lv_obj_add_style(btn_settings, &style_neon_btn, 0);
    lv_obj_add_style(btn_settings, &style_neon_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_settings, btn_settings_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_settings = lv_label_create(btn_settings);
    lv_label_set_text(lbl_settings, LV_SYMBOL_SETTINGS " SETTINGS");
    lv_obj_set_style_text_font(lbl_settings, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_settings);

    // Create settings screen
    create_settings_screen();
}

// ============================================================================
// Connection Animation
// ============================================================================
void pulse_anim_cb(void *var, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t *)var, v, 0);
}

void start_pulse_animation() {
    lv_anim_init(&pulse_anim);
    lv_anim_set_var(&pulse_anim, connection_indicator);
    lv_anim_set_values(&pulse_anim, LV_OPA_40, LV_OPA_COVER);
    lv_anim_set_time(&pulse_anim, 800);
    lv_anim_set_playback_time(&pulse_anim, 800);
    lv_anim_set_repeat_count(&pulse_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&pulse_anim, pulse_anim_cb);
    lv_anim_start(&pulse_anim);
}

// ============================================================================
// Setup
// ============================================================================
void setup() {
    Serial.begin(115200);

    // Check if waking from deep sleep
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    bool woke_from_sleep = false;

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Woke from timer - checking for touch");
        woke_from_sleep = true;

        // Initialize display briefly to check touch
        lcd.init();
        lcd.setRotation(1);

        // Quick touch check
        uint16_t x, y;
        bool touched = lcd.getTouch(&x, &y);

        if (!touched) {
            // No touch detected, go back to sleep immediately
            Serial.println("No touch - returning to sleep");
            lcd.setBrightness(0);
            esp_sleep_enable_timer_wakeup(1000000); // 1 second
            esp_deep_sleep_start();
        }

        // Touch detected, continue with full boot
        Serial.println("Touch detected - waking up!");
    }

    Serial.println("GSPro Controller - Ultra Modern UI");

    // Initialize display (only if not already initialized from wake check)
    if (!woke_from_sleep) {
        lcd.init();
        lcd.setRotation(1);
    }
    lcd.setBrightness(220);
    lcd.fillScreen(TFT_BLACK);

    // Boot animation
    lcd.setTextColor(0x07FF, TFT_BLACK);  // Cyan
    lcd.setTextSize(3);
    lcd.setCursor(130, 120);
    lcd.println("GSPRO");
    lcd.setTextColor(0xFFFF, TFT_BLACK);
    lcd.setTextSize(1);
    lcd.setCursor(130, 160);
    lcd.println("Initializing controller...");

    // Initialize BLE Keyboard
    bleKeyboard.begin();
    Serial.println("BLE Keyboard Started");

    // Initialize LVGL
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * 30);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Create UI
    create_ui();
    start_pulse_animation();

    Serial.println("UI Ready!");

    // Initialize WiFi in STA mode
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi initialized in Station mode");

    // Initialize ArduinoOTA
    ArduinoOTA.setHostname("GSProController");
    ArduinoOTA.setPassword("gspro2024");  // Change this password!

    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("OTA Start updating " + type);
        if (ota_status_label) {
            lv_label_set_text(ota_status_label, "Updating firmware...");
        }
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA End");
        if (ota_status_label) {
            lv_label_set_text(ota_status_label, "Update complete! Rebooting...");
        }
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        unsigned int percent = (progress / (total / 100));
        Serial.printf("OTA Progress: %u%%\r", percent);
        if (ota_progress_bar) {
            lv_bar_set_value(ota_progress_bar, percent, LV_ANIM_OFF);
        }
        if (ota_status_label) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Updating: %u%%", percent);
            lv_label_set_text(ota_status_label, buf);
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        String errorMsg = "Error: ";
        if (error == OTA_AUTH_ERROR) errorMsg += "Auth Failed";
        else if (error == OTA_BEGIN_ERROR) errorMsg += "Begin Failed";
        else if (error == OTA_CONNECT_ERROR) errorMsg += "Connect Failed";
        else if (error == OTA_RECEIVE_ERROR) errorMsg += "Receive Failed";
        else if (error == OTA_END_ERROR) errorMsg += "End Failed";
        Serial.println(errorMsg);
        if (ota_status_label) {
            lv_label_set_text(ota_status_label, errorMsg.c_str());
        }
    });

    ArduinoOTA.begin();
    Serial.println("OTA initialized. Hostname: GSProController, Password: gspro2024");
}

// ============================================================================
// Main Loop
// ============================================================================
unsigned long lastConnectionCheck = 0;
bool wasConnected = false;

void loop() {
    lv_timer_handler();

    // Handle OTA updates
    ArduinoOTA.handle();

    // Update connection status every 500ms
    if (millis() - lastConnectionCheck > 500) {
        lastConnectionCheck = millis();

        bool isConnected = bleKeyboard.isConnected();
        if (isConnected != wasConnected) {
            wasConnected = isConnected;

            if (isConnected) {
                lv_label_set_text(connection_label, "LINKED");
                lv_obj_set_style_text_color(connection_label, lv_color_hex(COLOR_SUCCESS), 0);
                lv_obj_set_style_bg_color(connection_indicator, lv_color_hex(COLOR_SUCCESS), 0);
                lv_obj_set_style_shadow_color(connection_indicator, lv_color_hex(COLOR_SUCCESS), 0);
                lv_anim_del(connection_indicator, pulse_anim_cb);
                lv_obj_set_style_opa(connection_indicator, LV_OPA_COVER, 0);
                update_status("CONNECTED", COLOR_SUCCESS);
                Serial.println("Bluetooth Connected!");
            } else {
                lv_label_set_text(connection_label, "PAIRING");
                lv_obj_set_style_text_color(connection_label, lv_color_hex(COLOR_WARNING), 0);
                lv_obj_set_style_bg_color(connection_indicator, lv_color_hex(COLOR_WARNING), 0);
                lv_obj_set_style_shadow_color(connection_indicator, lv_color_hex(COLOR_WARNING), 0);
                start_pulse_animation();
                update_status("DISCONNECTED", COLOR_ERROR);
                Serial.println("Bluetooth Disconnected!");
            }
        }

        // Update WiFi status in settings screen
        if (current_screen == SCREEN_SETTINGS && wifi_status_label) {
            if (WiFi.status() == WL_CONNECTED) {
                String status = "Connected: " + WiFi.SSID();
                lv_label_set_text(wifi_status_label, status.c_str());
            } else if (!wifi_scanning) {
                lv_label_set_text(wifi_status_label, "Disconnected");
            }
        }

        // Update Bluetooth status in settings screen
        if (current_screen == SCREEN_SETTINGS && bt_status_label) {
            if (bleKeyboard.isConnected()) {
                lv_label_set_text(bt_status_label, "Status: Connected");
            } else {
                lv_label_set_text(bt_status_label, "Status: Waiting for pairing...");
            }
        }
    }

    // Handle WiFi scan results
    if (wifi_scanning) {
        int n = WiFi.scanComplete();
        if (n >= 0) {
            wifi_scanning = false;

            if (n == 0) {
                lv_textarea_set_text(wifi_list, "No networks found");
                lv_label_set_text(wifi_status_label, "No networks found");
            } else {
                String networks = "";
                for (int i = 0; i < n && i < 15; i++) {  // Limit to 15 networks
                    networks += WiFi.SSID(i);
                    networks += " (";
                    networks += WiFi.RSSI(i);
                    networks += " dBm)";
                    if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) {
                        networks += " *";
                    }
                    networks += "\n";
                }
                lv_textarea_set_text(wifi_list, networks.c_str());

                char buf[64];
                snprintf(buf, sizeof(buf), "Found %d networks", n);
                lv_label_set_text(wifi_status_label, buf);

                Serial.printf("Found %d networks\n", n);
            }
        } else if (n == WIFI_SCAN_FAILED) {
            wifi_scanning = false;
            lv_textarea_set_text(wifi_list, "Scan failed");
            lv_label_set_text(wifi_status_label, "Scan failed");
        }
    }

    delay(5);
}
