/**
 * GSPro Controller for WT32-SC01 Plus V3.3
 *
 * ESP32-S3 with 3.5" IPS Touchscreen (480x320)
 * Bluetooth HID Keyboard for GSPro Golf Simulator
 *
 * Controls:
 * - Ctrl+M: Mulligan
 * - P: Pin Indicator
 * - J: Scout View
 * - Y: Heat Map
 * - F5: Free Flight
 * - Arrow Keys: Aim
 * - C/V: Tee Box Adjustment
 * - O: Flyover
 */

#include <Arduino.h>
#include <BleKeyboard.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lvgl.h>

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
        // Bus configuration
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

        // Panel configuration
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

        // Backlight configuration
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = 45;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // Touch configuration (FT6336)
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

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[480 * 20];
static lv_color_t buf2[480 * 20];

// Screen dimensions
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

// UI elements
static lv_obj_t *status_label;
static lv_obj_t *connection_label;
static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_arrow_btn;
static lv_style_t style_title;

// ============================================================================
// LVGL Display Flush Callback
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

// ============================================================================
// LVGL Touch Read Callback
// ============================================================================
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
// Button Event Handlers
// ============================================================================
void send_key(uint8_t key, bool with_ctrl = false, bool is_function = false) {
    if (!bleKeyboard.isConnected()) {
        lv_label_set_text(status_label, "Not Connected!");
        return;
    }

    if (with_ctrl) {
        bleKeyboard.press(KEY_LEFT_CTRL);
        delay(10);
        bleKeyboard.press(key);
        delay(50);
        bleKeyboard.releaseAll();
    } else if (is_function) {
        bleKeyboard.write(key);
    } else {
        bleKeyboard.write(key);
    }
}

// Mulligan (Ctrl+M)
static void btn_mulligan_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('m', true);
        lv_label_set_text(status_label, "Mulligan");
    }
}

// Pin Indicator (P)
static void btn_pin_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('p');
        lv_label_set_text(status_label, "Pin Indicator");
    }
}

// Scout View (J)
static void btn_scout_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('j');
        lv_label_set_text(status_label, "Scout View");
    }
}

// Heat Map (Y)
static void btn_heatmap_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('y');
        lv_label_set_text(status_label, "Heat Map");
    }
}

// Free Flight (F5)
static void btn_freeflight_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_F5);
        lv_label_set_text(status_label, "Free Flight");
    }
}

// Flyover (O)
static void btn_flyover_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('o');
        lv_label_set_text(status_label, "Flyover");
    }
}

// Tee Box Left (C)
static void btn_tee_left_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('c');
        lv_label_set_text(status_label, "Tee Left");
    }
}

// Tee Box Right (V)
static void btn_tee_right_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        send_key('v');
        lv_label_set_text(status_label, "Tee Right");
    }
}

// Arrow Up
static void btn_up_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_UP_ARROW);
        lv_label_set_text(status_label, "Aim Up");
    }
}

// Arrow Down
static void btn_down_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_DOWN_ARROW);
        lv_label_set_text(status_label, "Aim Down");
    }
}

// Arrow Left
static void btn_left_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_LEFT_ARROW);
        lv_label_set_text(status_label, "Aim Left");
    }
}

// Arrow Right
static void btn_right_handler(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bleKeyboard.write(KEY_RIGHT_ARROW);
        lv_label_set_text(status_label, "Aim Right");
    }
}

// ============================================================================
// UI Setup
// ============================================================================
void setup_styles() {
    // Button style
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x2E7D32));  // Green
    lv_style_set_bg_grad_color(&style_btn, lv_color_hex(0x1B5E20));
    lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_btn, 2);
    lv_style_set_border_color(&style_btn, lv_color_hex(0x4CAF50));
    lv_style_set_shadow_width(&style_btn, 10);
    lv_style_set_shadow_color(&style_btn, lv_color_hex(0x000000));
    lv_style_set_shadow_ofs_y(&style_btn, 5);
    lv_style_set_text_color(&style_btn, lv_color_hex(0xFFFFFF));
    lv_style_set_pad_all(&style_btn, 10);

    // Pressed button style
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, lv_color_hex(0x1B5E20));
    lv_style_set_shadow_ofs_y(&style_btn_pressed, 2);

    // Arrow button style
    lv_style_init(&style_arrow_btn);
    lv_style_set_radius(&style_arrow_btn, 8);
    lv_style_set_bg_color(&style_arrow_btn, lv_color_hex(0x1565C0));  // Blue
    lv_style_set_bg_grad_color(&style_arrow_btn, lv_color_hex(0x0D47A1));
    lv_style_set_bg_grad_dir(&style_arrow_btn, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_arrow_btn, 2);
    lv_style_set_border_color(&style_arrow_btn, lv_color_hex(0x42A5F5));
    lv_style_set_text_color(&style_arrow_btn, lv_color_hex(0xFFFFFF));
    lv_style_set_text_font(&style_arrow_btn, &lv_font_montserrat_24);

    // Title style
    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, lv_color_hex(0x4CAF50));
    lv_style_set_text_font(&style_title, &lv_font_montserrat_20);
}

lv_obj_t* create_button(lv_obj_t *parent, const char *label, int x, int y,
                        int w, int h, lv_event_cb_t event_cb) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_center(lbl);

    return btn;
}

lv_obj_t* create_arrow_button(lv_obj_t *parent, const char *symbol, int x, int y,
                              int w, int h, lv_event_cb_t event_cb) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_add_style(btn, &style_arrow_btn, 0);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, symbol);
    lv_obj_center(lbl);

    return btn;
}

void create_ui() {
    setup_styles();

    // Set dark background
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x1A1A1A), 0);

    // Title
    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "GSPro Controller");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    // Connection status
    connection_label = lv_label_create(lv_scr_act());
    lv_label_set_text(connection_label, "BT: Connecting...");
    lv_obj_set_style_text_color(connection_label, lv_color_hex(0xFFA726), 0);
    lv_obj_set_style_text_font(connection_label, &lv_font_montserrat_12, 0);
    lv_obj_align(connection_label, LV_ALIGN_TOP_RIGHT, -10, 8);

    // Status label
    status_label = lv_label_create(lv_scr_act());
    lv_label_set_text(status_label, "Ready");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_16, 0);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -8);

    // ========== LEFT COLUMN - Main Controls ==========
    int col1_x = 10;
    int btn_w = 100;
    int btn_h = 45;
    int spacing = 52;
    int start_y = 35;

    // Mulligan (Ctrl+M)
    create_button(lv_scr_act(), "Mulligan", col1_x, start_y, btn_w, btn_h, btn_mulligan_handler);

    // Pin Indicator (P)
    create_button(lv_scr_act(), "Pin", col1_x, start_y + spacing, btn_w, btn_h, btn_pin_handler);

    // Scout View (J)
    create_button(lv_scr_act(), "Scout", col1_x, start_y + spacing*2, btn_w, btn_h, btn_scout_handler);

    // Heat Map (Y)
    create_button(lv_scr_act(), "Heat Map", col1_x, start_y + spacing*3, btn_w, btn_h, btn_heatmap_handler);

    // Flyover (O)
    create_button(lv_scr_act(), "Flyover", col1_x, start_y + spacing*4, btn_w, btn_h, btn_flyover_handler);

    // ========== MIDDLE COLUMN - Arrow Keys ==========
    int arrow_size = 55;
    int arrow_center_x = 190;
    int arrow_center_y = 160;

    // Arrow Up
    create_arrow_button(lv_scr_act(), LV_SYMBOL_UP,
                       arrow_center_x, arrow_center_y - arrow_size - 5,
                       arrow_size, arrow_size, btn_up_handler);

    // Arrow Down
    create_arrow_button(lv_scr_act(), LV_SYMBOL_DOWN,
                       arrow_center_x, arrow_center_y + 5,
                       arrow_size, arrow_size, btn_down_handler);

    // Arrow Left
    create_arrow_button(lv_scr_act(), LV_SYMBOL_LEFT,
                       arrow_center_x - arrow_size - 5, arrow_center_y - arrow_size/2,
                       arrow_size, arrow_size, btn_left_handler);

    // Arrow Right
    create_arrow_button(lv_scr_act(), LV_SYMBOL_RIGHT,
                       arrow_center_x + arrow_size + 5, arrow_center_y - arrow_size/2,
                       arrow_size, arrow_size, btn_right_handler);

    // Aim label
    lv_obj_t *aim_label = lv_label_create(lv_scr_act());
    lv_label_set_text(aim_label, "AIM");
    lv_obj_set_style_text_color(aim_label, lv_color_hex(0x42A5F5), 0);
    lv_obj_set_style_text_font(aim_label, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(aim_label, arrow_center_x + 15, arrow_center_y - 20);

    // ========== RIGHT COLUMN - Additional Controls ==========
    int col2_x = 365;

    // Free Flight (F5)
    create_button(lv_scr_act(), "Free Flight", col2_x, start_y, btn_w, btn_h, btn_freeflight_handler);

    // Tee Box section
    lv_obj_t *tee_label = lv_label_create(lv_scr_act());
    lv_label_set_text(tee_label, "Tee Box");
    lv_obj_set_style_text_color(tee_label, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(tee_label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(tee_label, col2_x + 20, start_y + spacing + 5);

    // Tee Left (C)
    lv_obj_t *btn_tee_l = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_tee_l, col2_x, start_y + spacing + 22);
    lv_obj_set_size(btn_tee_l, 48, 40);
    lv_obj_add_style(btn_tee_l, &style_btn, 0);
    lv_obj_add_style(btn_tee_l, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_tee_l, btn_tee_left_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_tee_l = lv_label_create(btn_tee_l);
    lv_label_set_text(lbl_tee_l, "C");
    lv_obj_center(lbl_tee_l);

    // Tee Right (V)
    lv_obj_t *btn_tee_r = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_tee_r, col2_x + 52, start_y + spacing + 22);
    lv_obj_set_size(btn_tee_r, 48, 40);
    lv_obj_add_style(btn_tee_r, &style_btn, 0);
    lv_obj_add_style(btn_tee_r, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_tee_r, btn_tee_right_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_tee_r = lv_label_create(btn_tee_r);
    lv_label_set_text(lbl_tee_r, "V");
    lv_obj_center(lbl_tee_r);

    // Key hints
    lv_obj_t *hints = lv_label_create(lv_scr_act());
    lv_label_set_text(hints,
        "Ctrl+M  P  J  Y  O\n"
        "F5  C/V  Arrows");
    lv_obj_set_style_text_color(hints, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_font(hints, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(hints, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(hints, LV_ALIGN_BOTTOM_RIGHT, -10, -25);
}

// ============================================================================
// Setup
// ============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("GSPro Controller Starting...");

    // Initialize display
    lcd.init();
    lcd.setRotation(1);  // Landscape
    lcd.setBrightness(200);
    lcd.fillScreen(TFT_BLACK);

    // Show boot message
    lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    lcd.setTextSize(2);
    lcd.setCursor(120, 140);
    lcd.println("GSPro Controller");
    lcd.setTextSize(1);
    lcd.setCursor(150, 170);
    lcd.println("Initializing...");

    // Initialize BLE Keyboard
    bleKeyboard.begin();
    Serial.println("BLE Keyboard Started");

    // Initialize LVGL
    lv_init();

    // Initialize display buffer
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * 20);

    // Initialize display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize touch driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Create UI
    create_ui();

    Serial.println("UI Created - Ready!");
}

// ============================================================================
// Main Loop
// ============================================================================
unsigned long lastConnectionCheck = 0;
bool wasConnected = false;

void loop() {
    lv_timer_handler();

    // Update connection status every 500ms
    if (millis() - lastConnectionCheck > 500) {
        lastConnectionCheck = millis();

        bool isConnected = bleKeyboard.isConnected();
        if (isConnected != wasConnected) {
            wasConnected = isConnected;
            if (isConnected) {
                lv_label_set_text(connection_label, "BT: Connected");
                lv_obj_set_style_text_color(connection_label, lv_color_hex(0x4CAF50), 0);
                Serial.println("Bluetooth Connected!");
            } else {
                lv_label_set_text(connection_label, "BT: Disconnected");
                lv_obj_set_style_text_color(connection_label, lv_color_hex(0xF44336), 0);
                Serial.println("Bluetooth Disconnected!");
            }
        }
    }

    delay(5);
}
