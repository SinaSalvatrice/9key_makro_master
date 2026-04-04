
#include QMK_KEYBOARD_H
#include "gpio.h"
#include <stdio.h>

// ============================================================
// RGB / OLED selector build
//
// Notes:
// - This version stops relying on built-in RGBLIGHT animations for the
//   layer visuals and renders the LEDs directly per frame.
// - Adjust key_led_map[] if your WS2812 strip order is not left->right 1..9.
// - Current SELECT layer is now a proper layer launcher / style selector.
// ============================================================

#ifndef RGBLIGHT_LED_COUNT
#    define RGBLIGHT_LED_COUNT 9
#endif

#define PAD_KEY_COUNT        9
#define RGB_FRAME_MS         33
#define SELECT_STEP_MS       500
#define BUTTON_TAP_MS        180
#define BOOT_TOTAL_MS        2800

// ── Layer enum ──────────────────────────────────────────────
enum layers {
    _BASE,
    _WINDOW,
    _TEXT,
    _RGB,
    _SELECT,
    _LAYER_COUNT
};

// ── Custom keycodes for SELECT mode ─────────────────────────
enum custom_keycodes {
    SEL_BASE = SAFE_RANGE,
    SEL_WINDOW,
    SEL_TEXT,
    SEL_RGB,
    RGB_PROFILE
};

// ── Visual profile ──────────────────────────────────────────
// false = wild per-layer animations
// true  = reduced profile; only the currently selected layer key breathes
static bool rgb_minimal_mode = false;

// ── OLED / state tracking ───────────────────────────────────
static uint16_t last_keycode    = KC_NO;
static uint8_t  last_row        = 0;
static uint8_t  last_col        = 0;
static uint8_t  select_return_layer = _BASE;
static uint8_t  select_cursor       = 0;
static uint32_t select_cycle_timer  = 0;
static uint32_t rgb_frame_timer     = 0;
static uint32_t boot_start          = 0;

// ── Key -> LED mapping ──────────────────────────────────────
// Physical key numbering for OLED is row-major / left-to-right:
//
//  1 2 3
//  4 5 6
//  7 8 9
//
// If your strip snakes around, only change this array.
static const uint8_t key_led_map[PAD_KEY_COUNT] = {
    0, 1, 2,
    3, 4, 5,
    6, 7, 8
};

typedef struct {
    uint8_t layer;
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
    const char *name;
    bool selectable;
} select_slot_t;

// 9 physical keys, future-proofed for a 3x3 layer selector.
static const select_slot_t select_slots[PAD_KEY_COUNT] = {
    { _BASE,   160, 220, 120, "BASE",   true  },  // key 1
    { _WINDOW, 176, 240, 120, "WINDOW", true  },  // key 2
    { _TEXT,    96, 220, 110, "TXT",    true  },  // key 3
    { _RGB,    215, 240, 130, "RGB",    true  },  // key 4
    { _SELECT,   0,   0, 120, "SELECT", false },  // key 5 / style toggle
    { _SELECT,   0,   0,  24, "FREE",   false },  // key 6
    { _SELECT,   0,   0,  24, "FREE",   false },  // key 7
    { _SELECT,   0,   0,  24, "FREE",   false },  // key 8
    { _SELECT,   0,   0,  24, "FREE",   false },  // key 9
};

static const char *layer_name_short(uint8_t l) {
    switch (l) {
        case _BASE:   return "BASE";
        case _WINDOW: return "WIN";
        case _TEXT:   return "TXT";
        case _RGB:    return "RGB";
        case _SELECT: return "SEL";
        default:      return "BASE";
    }
}

static const char *layer_name_long(uint8_t l) {
    switch (l) {
        case _BASE:   return "BASE";
        case _WINDOW: return "WINDOW";
        case _TEXT:   return "TXT";
        case _RGB:    return "RGB";
        case _SELECT: return "SELECT";
        default:      return "BASE";
    }
}

static uint8_t active_layer_raw(void) {
    return get_highest_layer(layer_state | default_layer_state);
}

static uint8_t active_base_layer(void) {
    layer_state_t state = (layer_state | default_layer_state) & ~((layer_state_t)1 << _SELECT);
    uint8_t       l     = get_highest_layer(state);
    return (l >= _SELECT) ? _BASE : l;
}

static uint8_t slot_for_layer(uint8_t layer) {
    for (uint8_t i = 0; i < PAD_KEY_COUNT; i++) {
        if (select_slots[i].selectable && select_slots[i].layer == layer) {
            return i;
        }
    }
    return 0;
}

static uint8_t next_select_slot(uint8_t idx, bool clockwise) {
    for (uint8_t step = 0; step < PAD_KEY_COUNT; step++) {
        idx = clockwise ? ((idx + 1) % PAD_KEY_COUNT) : ((idx + PAD_KEY_COUNT - 1) % PAD_KEY_COUNT);
        if (select_slots[idx].selectable || idx == 4) {
            return idx;
        }
    }
    return idx;
}

static void enter_select_mode(void) {
    select_return_layer = active_base_layer();
    select_cursor       = slot_for_layer(select_return_layer);
    select_cycle_timer  = timer_read32();
    layer_move(_SELECT);
}

static void leave_select_mode(void) {
    layer_move(select_return_layer);
}

static uint8_t lerp8(uint8_t a, uint8_t b, uint8_t t) {
    return (uint8_t)(a + (((int16_t)b - (int16_t)a) * t) / 255);
}

static uint8_t triwave8_period(uint32_t now, uint16_t period_ms, uint8_t phase) {
    if (period_ms == 0) {
        return 0;
    }
    uint32_t t = (now + ((uint32_t)phase * period_ms) / 255) % period_ms;
    uint32_t x = (t * 510UL) / period_ms;
    return (x > 255) ? (uint8_t)(510 - x) : (uint8_t)x;
}

static uint8_t pulse_val(uint32_t now, uint16_t period, uint8_t phase, uint8_t min_v, uint8_t max_v) {
    uint8_t tri = triwave8_period(now, period, phase);
    return (uint8_t)(min_v + ((uint16_t)(max_v - min_v) * tri) / 255);
}

#ifdef RGBLIGHT_ENABLE
static void set_key_hsv(uint8_t key_index, uint8_t h, uint8_t s, uint8_t v) {
    if (key_index >= PAD_KEY_COUNT) return;
    uint8_t led = key_led_map[key_index];
    if (led >= RGBLIGHT_LED_COUNT) return;
    rgblight_sethsv_at(h, s, v, led);
}

static void clear_all_keys(void) {
    for (uint8_t i = 0; i < PAD_KEY_COUNT; i++) {
        set_key_hsv(i, 0, 0, 0);
    }
}

static void render_minimal_profile(uint8_t layer) {
    clear_all_keys();

    if (layer == _SELECT) {
        const select_slot_t *slot = &select_slots[select_cursor];
        uint8_t val = pulse_val(timer_read32(), 2200, 0, 14, 96);
        set_key_hsv(select_cursor, slot->hue, slot->sat, val);
        return;
    }

    uint8_t slot = slot_for_layer(layer);
    const select_slot_t *info = &select_slots[slot];
    uint8_t val = pulse_val(timer_read32(), 2400, 0, 12, 90);
    set_key_hsv(slot, info->hue, info->sat, val);
}

static void render_base_wild(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < PAD_KEY_COUNT; i++) {
        uint8_t mix = triwave8_period(now, 5200, i * 17);
        uint8_t hue = lerp8(150, 205, mix);  // cyan -> violet
        uint8_t val = pulse_val(now, 2600, i * 11, 18, 88);
        set_key_hsv(i, hue, 220, val);
    }
}

static void render_window_wild(void) {
    uint32_t now        = timer_read32();
    uint8_t  spike      = (now / 110) % PAD_KEY_COUNT;
    uint16_t flash_tick = now % 900;
    bool     flash      = flash_tick < 95;

    for (uint8_t i = 0; i < PAD_KEY_COUNT; i++) {
        uint8_t dist = (i > spike) ? (i - spike) : (spike - i);
        uint8_t hue  = 176;
        uint8_t sat  = 245;
        uint8_t val  = pulse_val(now, 1800, i * 19, 10, 40);

        if (dist == 0) {
            hue = 168;
            val = 160;
        } else if (dist == 1) {
            hue = 176;
            val = 90;
        }

        if (flash && (i % 2 == 0)) {
            sat = 30;
            val = 180;
        }

        set_key_hsv(i, hue, sat, val);
    }
}

static void render_text_wild(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < PAD_KEY_COUNT; i++) {
        uint8_t drift = triwave8_period(now, 7600, i * 13);
        uint8_t hue   = lerp8(88, 112, drift);  // green -> teal-ish green
        uint8_t val   = pulse_val(now, 3600, i * 9, 10, 58);
        set_key_hsv(i, hue, 210, val);
    }
}

static void render_rgb_wild(void) {
    uint32_t now       = timer_read32();
    bool     beatflash = ((now % 1100) < 120);

    for (uint8_t i = 0; i < PAD_KEY_COUNT; i++) {
        uint8_t hue = (uint8_t)((now / 18) + i * 28);
        uint8_t val = pulse_val(now, 1400, i * 15, 34, beatflash ? 180 : 120);
        set_key_hsv(i, hue, 255, val);
    }
}

static void render_select_wild(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < PAD_KEY_COUNT; i++) {
        const select_slot_t *slot = &select_slots[i];
        uint8_t val = slot->selectable ? 34 : 10;
        uint8_t sat = slot->sat;
        uint8_t hue = slot->hue;

        if (i == 4) {  // center key = neutral SELECT/style slot
            sat = 0;
            val = 24;
        }

        if (i == select_cursor) {
            if (slot->selectable) {
                val = pulse_val(now, 900, 0, 80, 170);
            } else {
                sat = 0;
                val = pulse_val(now, 900, 0, 46, 120);
            }
        }

        set_key_hsv(i, hue, sat, val);
    }
}

static void render_rgb_layer_visuals(void) {
    uint8_t layer = active_layer_raw();

    if (rgb_minimal_mode) {
        render_minimal_profile(layer);
        return;
    }

    switch (layer) {
        case _BASE:
            render_base_wild();
            break;
        case _WINDOW:
            render_window_wild();
            break;
        case _TEXT:
            render_text_wild();
            break;
        case _RGB:
            render_rgb_wild();
            break;
        case _SELECT:
            render_select_wild();
            break;
        default:
            render_base_wild();
            break;
    }
}
#endif

// ── Keymaps ─────────────────────────────────────────────────
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        MO(_SELECT), KC_UP,        KC_SPC,
        KC_LEFT,     KC_ENT,       KC_RGHT,
        LCTL(KC_Z),  KC_DOWN,      LCTL(KC_R)
    ),

    [_WINDOW] = LAYOUT(
        MO(_SELECT),         KC_HOME,            KC_NO,
        LGUI(LCTL(KC_LEFT)), KC_NO,              LGUI(LCTL(KC_RGHT)),
        LSA(LALT(KC_TAB)),   KC_END,             LALT(KC_TAB)
    ),

    [_TEXT] = LAYOUT(
        MO(_SELECT), LCTL(KC_A), LCTL(KC_C),
        LCTL(KC_X),  LCTL(KC_V), KC_RGHT,
        KC_HOME,     KC_SPC,     KC_BSPC
    ),

    [_RGB] = LAYOUT(
        MO(_SELECT), UG_SPDD, UG_TOGG,
        UG_HUEU,     UG_HUED, UG_VALU,
        UG_SATU,     UG_SATD, UG_VALD
    ),

    // 1..9 = physical key numbering left to right
    [_SELECT] = LAYOUT(
        SEL_BASE,   SEL_WINDOW, SEL_TEXT,
        SEL_RGB,    RGB_PROFILE, KC_NO,
        KC_NO,      KC_NO,      KC_NO
    ),
};

// ── Fallback combo: R0C0 + R2C2 → _BASE ────────────────────
static bool r0c0_held = false;

// ── Track SELECT entry so MO(_SELECT) remembers the base ───
layer_state_t layer_state_set_user(layer_state_t state) {
    static layer_state_t last_state = 0;

    bool select_now    = layer_state_cmp(state, _SELECT);
    bool select_before = layer_state_cmp(last_state, _SELECT);

    if (select_now && !select_before) {
        layer_state_t without_select = (state | default_layer_state) & ~((layer_state_t)1 << _SELECT);
        uint8_t base = get_highest_layer(without_select);
        if (base >= _SELECT) {
            base = _BASE;
        }
        select_return_layer = base;
        select_cursor       = slot_for_layer(base);
        select_cycle_timer  = timer_read32();
    }

    last_state = state;
    return state;
}

static void select_target_layer(uint8_t layer) {
    select_return_layer = layer;
    layer_move(layer);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.key.row == 0 && record->event.key.col == 0) {
        r0c0_held = record->event.pressed;
    }
    if (record->event.key.row == 2 && record->event.key.col == 2) {
        if (record->event.pressed && r0c0_held) {
            layer_move(_BASE);
            return false;
        }
    }

    if (record->event.pressed) {
        last_keycode = keycode;
        last_row     = record->event.key.row;
        last_col     = record->event.key.col;
    }

    switch (keycode) {
        case SEL_BASE:
            if (record->event.pressed) {
                select_target_layer(_BASE);
            }
            return false;
        case SEL_WINDOW:
            if (record->event.pressed) {
                select_target_layer(_WINDOW);
            }
            return false;
        case SEL_TEXT:
            if (record->event.pressed) {
                select_target_layer(_TEXT);
            }
            return false;
        case SEL_RGB:
            if (record->event.pressed) {
                select_target_layer(_RGB);
            }
            return false;
        case RGB_PROFILE:
            if (record->event.pressed) {
                rgb_minimal_mode = !rgb_minimal_mode;
                select_cycle_timer = timer_read32();
            }
            return false;
    }

    return true;
}

// ── Encoder: layer-dependent ────────────────────────────────
bool encoder_update_user(uint8_t index, bool clockwise) {
    (void)index;

    switch (active_layer_raw()) {
        case _BASE:
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
            break;
        case _WINDOW:
            tap_code(clockwise ? KC_PGDN : KC_PGUP);
            break;
        case _TEXT:
            register_code(KC_LCTL);
            tap_code(clockwise ? KC_RGHT : KC_LEFT);
            unregister_code(KC_LCTL);
            break;
        case _RGB:
            tap_code16(clockwise ? UG_VALU : UG_VALD);
            break;
        case _SELECT:
            select_cursor = next_select_slot(select_cursor, clockwise);
            select_cycle_timer = timer_read32();
            break;
        default:
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
            break;
    }
    return false;
}

// ── Buttons + animation pump ────────────────────────────────
void matrix_scan_user(void) {
    // Encoder button: hold = momentary _SELECT
    static bool enc_was_pressed = false;
    bool        enc_pressed     = (gpio_read_pin(ENCODER_BTN_PIN) == 0);

    if (enc_pressed && !enc_was_pressed) {
        enter_select_mode();
    } else if (!enc_pressed && enc_was_pressed && active_layer_raw() == _SELECT) {
        leave_select_mode();
    }
    enc_was_pressed = enc_pressed;

    // Selector button:
    // quick tap  = toggle RGB profile
    // hold       = momentary _SELECT
    static bool     sel_was_pressed      = false;
    static bool     sel_hold_entered     = false;
    static uint32_t sel_press_time       = 0;
    bool            sel_pressed          = (gpio_read_pin(SELECTOR_BTN_PIN) == 0);

    if (sel_pressed && !sel_was_pressed) {
        sel_press_time   = timer_read32();
        sel_hold_entered = false;
    } else if (sel_pressed && sel_was_pressed && !sel_hold_entered) {
        if (timer_elapsed32(sel_press_time) > BUTTON_TAP_MS) {
            enter_select_mode();
            sel_hold_entered = true;
        }
    } else if (!sel_pressed && sel_was_pressed) {
        if (sel_hold_entered) {
            if (active_layer_raw() == _SELECT) {
                leave_select_mode();
            }
        } else if (timer_elapsed32(sel_press_time) < BUTTON_TAP_MS) {
            rgb_minimal_mode = !rgb_minimal_mode;
        }
    }
    sel_was_pressed = sel_pressed;

    if (active_layer_raw() == _SELECT && timer_elapsed32(select_cycle_timer) >= SELECT_STEP_MS) {
        select_cursor = next_select_slot(select_cursor, true);
        select_cycle_timer = timer_read32();
    }

#ifdef RGBLIGHT_ENABLE
    if (timer_elapsed32(rgb_frame_timer) >= RGB_FRAME_MS) {
        rgb_frame_timer = timer_read32();
        render_rgb_layer_visuals();
    }
#endif
}

// ── Init ────────────────────────────────────────────────────
void keyboard_post_init_user(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
#endif

    gpio_set_pin_output(GP25);
    gpio_write_pin_high(GP25);

    gpio_set_pin_input_high(ENCODER_BTN_PIN);
    gpio_set_pin_input_high(SELECTOR_BTN_PIN);

    select_cursor      = slot_for_layer(_BASE);
    select_cycle_timer = timer_read32();
    rgb_frame_timer    = timer_read32();
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

static void write_line(uint8_t row, const char *str) {
    char buf[22];
    snprintf(buf, sizeof(buf), "%-21.21s", str);
    oled_set_cursor(0, row);
    oled_write(buf, false);
}

static void get_key_label(uint16_t kc, char *buf, uint8_t buflen) {
    switch (kc) {
        case SEL_BASE:
            snprintf(buf, buflen, "BASE");
            return;
        case SEL_WINDOW:
            snprintf(buf, buflen, "WIN");
            return;
        case SEL_TEXT:
            snprintf(buf, buflen, "TXT");
            return;
        case SEL_RGB:
            snprintf(buf, buflen, "RGB");
            return;
        case RGB_PROFILE:
            snprintf(buf, buflen, "STYLE");
            return;
    }

    if (kc == MO(_SELECT)) {
        snprintf(buf, buflen, "SEL");
        return;
    }

    const char *s = get_keycode_string(kc);
    if (s[0] == 'K' && s[1] == 'C' && s[2] == '_') s += 3;
    snprintf(buf, buflen, "%.*s", buflen - 1, s);
}

static void render_boot(void) {
    if (boot_start == 0) {
        boot_start = timer_read32() | 1;
    }

    uint32_t t = timer_elapsed32(boot_start);

    write_line(0, "");
    write_line(1, "");
    write_line(2, t >=  800 ? "          I AM" : "             I");
    write_line(3, t >= 1200 ? "          ROOT" : "");
    write_line(4, "");
    write_line(5, "");
    write_line(6, "");
    write_line(7, "");
}

static void render_header(uint8_t layer) {
    char line[22];
    snprintf(line, sizeof(line), "%-6s  FX:%s",
             layer_name_short(layer),
             rgb_minimal_mode ? "QUIET" : "WILD");
    write_line(0, line);
}

static void render_keyinfo(void) {
    char buf[22];
    char label[16];
    get_key_label(last_keycode, label, sizeof(label));

    write_line(1, "---------------------");
    snprintf(buf, sizeof(buf), "Layer: %s", layer_name_long(active_base_layer()));
    write_line(2, buf);
    snprintf(buf, sizeof(buf), "Key:   %.14s", label);
    write_line(3, buf);
    snprintf(buf, sizeof(buf), "Code:  0x%04X", last_keycode);
    write_line(4, buf);
    snprintf(buf, sizeof(buf), "Pos:   %u (%u,%u)",
             (last_row * 3) + last_col + 1, last_row, last_col);
    write_line(5, buf);
    write_line(6, "");
    write_line(7, "");
}

static void render_select_view(void) {
    char buf[22];
    const select_slot_t *slot = &select_slots[select_cursor];

    write_line(0, "SELECT     PREVIEW");
    write_line(1, "---------------------");
    snprintf(buf, sizeof(buf), "Key:   %u", select_cursor + 1);
    write_line(2, buf);
    snprintf(buf, sizeof(buf), "Layer: %s", slot->name);
    write_line(3, buf);
    snprintf(buf, sizeof(buf), "FX:    %s", rgb_minimal_mode ? "QUIET" : "WILD");
    write_line(4, buf);
    write_line(5, slot->selectable ? "Press key = enter" : "Center = FX toggle");
    write_line(6, "Hold = auto cycle");
    write_line(7, "Enc  = manual step");
}

bool oled_task_user(void) {
    if (boot_start == 0 || timer_elapsed32(boot_start) < BOOT_TOTAL_MS) {
        render_boot();
        return false;
    }

    if (active_layer_raw() == _SELECT) {
        render_select_view();
    } else {
        render_header(active_base_layer());
        render_keyinfo();
    }

    return false;
}
#endif
