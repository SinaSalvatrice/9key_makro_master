#include QMK_KEYBOARD_H
#include "gpio.h"
#include <stdio.h>

// ── Layer enum ──────────────────────────────────────────────
enum layers {
    _BASE,
    _WINDOW,
    _TEXT,
    _RGB,
    _SELECT,
    _LAYER_COUNT  // always last
};


// Short label (max 6 chars) for the legend grid
static const char *layer_name_short(uint8_t l) {
    switch (l) {
        case _BASE:   return "BASE";
        case _WINDOW:    return "WIN";
        case _TEXT:   return "TXT";
        case _RGB:    return "RGB";
        case _SELECT: return "SEL";
        default:      return "BASE";
    }
}

// ── State tracked for OLED ──────────────────────────────────
static uint16_t last_keycode = KC_NO;
static uint8_t  last_row     = 0;
static uint8_t  last_col     = 0;
static bool     legend_active = false;

static uint8_t active_layer(void) {
    return get_highest_layer(layer_state | default_layer_state);
}

// prev / next layer (wrapping, skip _SELECT)
static uint8_t prev_layer(uint8_t l) {
    uint8_t p = (l == 0) ? (_SELECT - 1) : (l - 1);
    return p;
}
static uint8_t next_layer(uint8_t l) {
    uint8_t n = l + 1;
    if (n >= _SELECT) n = 0;
    return n;
}

// ── Keymaps ─────────────────────────────────────────────────
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
        MO(_SELECT),            KC_UP,              KC_SPC,
        KC_LEFT,                KC_ENT,             KC_RGHT,
        LCTL(KC_Z),             KC_DOWN,            LCTL(KC_R)
    ),

    [_WINDOW] = LAYOUT(
        MO(_SELECT),            KC_HOME,           KC_NO,
        LGUI(LCTL(KC_LEFT)),    KC_NO,           LGUI(LCTL(KC_RGHT)),
        LSA(LALT(KC_TAB)),      KC_END,             LALT(KC_TAB)
    ),

    [_TEXT] = LAYOUT(
        MO(_SELECT),            LCTL(KC_A),         LCTL(KC_C),
        LCTL(KC_X),             LCTL(KC_V),         KC_RGHT,
        KC_HOME,                KC_SPACE,           KC_BSPC
    ),

    [_RGB] = LAYOUT(
        MO(_SELECT),            UG_SPDD,            UG_TOGG,
        UG_HUEU,                UG_HUED,            UG_VALU,
        UG_SATU,                UG_SATD,            UG_VALD
    ),


    [_SELECT] = LAYOUT(
        KC_NO,                  TO(_WINDOW),        TO(_TEXT),
        TO(_RGB),               TO(_RGB),           LCTL(KC_F),
        LCTL(KC_S),             MS_BTN2,             TO(_BASE)
    ),
};

// ── Fallback combo: R0C0 + R2C2 → _BASE ────────────────────
static bool r0c0_held = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Track R0C0 for fallback combo
    if (record->event.key.row == 0 && record->event.key.col == 0) {
        r0c0_held = record->event.pressed;
    }
    if (record->event.key.row == 2 && record->event.key.col == 2) {
        if (record->event.pressed && r0c0_held) {
            layer_move(_BASE);
            return false;
        }
    }

    // Track last key press for OLED display
    if (record->event.pressed) {
        last_keycode = keycode;
        last_row     = record->event.key.row;
        last_col     = record->event.key.col;
    }

    return true;
}

// ── Encoder: layer-dependent ────────────────────────────────
bool encoder_update_user(uint8_t index, bool clockwise) {
    (void)index;
    switch (active_layer()) {
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
            {
                // Compute the base layer sitting underneath _SELECT
                uint8_t base = get_highest_layer(layer_state & ~((layer_state_t)1 << _SELECT));
                layer_move(clockwise ? next_layer(base) : prev_layer(base));
                layer_on(_SELECT);  // keep _SELECT on top — encoder button is still held
            }
            break;
        default:
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
            break;
    }
    return false;
}

// ── Encoder button via matrix_scan (GP8) ────────────────────
// ── Selector button via matrix_scan (GP12) ──────────────────
void matrix_scan_user(void) {
    // Encoder button: momentary _SELECT — hold to enter, release to exit.
    // layer_on/layer_off preserves the base layer through cycling.
    static bool enc_was_pressed = false;
    bool        enc_pressed     = (gpio_read_pin(ENCODER_BTN_PIN) == 0);

    if (enc_pressed && !enc_was_pressed) {
        layer_on(_SELECT);
    } else if (!enc_pressed && enc_was_pressed) {
        layer_off(_SELECT);
    }
    enc_was_pressed = enc_pressed;

    // Selector button: hold = momentary _TEXT; quick tap = toggle OLED legend.
    static bool     sel_was_pressed  = false;
    static uint8_t  sel_prev_layer   = _BASE;
    static uint32_t sel_press_time   = 0;
    bool            sel_pressed      = (gpio_read_pin(SELECTOR_BTN_PIN) == 0);

    if (sel_pressed && !sel_was_pressed) {
        sel_prev_layer = active_layer();
        sel_press_time = timer_read32();
        layer_move(_TEXT);
    } else if (!sel_pressed && sel_was_pressed) {
        if (timer_elapsed32(sel_press_time) < 200) {
            // Quick tap: toggle legend, stay on current layer
            legend_active = !legend_active;
        }
        layer_move(sel_prev_layer);
    }
    sel_was_pressed = sel_pressed;
}

// ── RGB: per-layer colour & effect ───────────────────────────
#ifdef RGBLIGHT_ENABLE
layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
        case _RGB:
            // Fast rainbow mood — all LEDs flash through hues rapidly, like waterdrops
            rgblight_mode_noeeprom(RGBLIGHT_MODE_RAINBOW_MOOD);
            rgblight_set_speed_noeeprom(220);
            break;
        case _BASE:
        default:
            // Slow moving rainbow swirl — flowing colour wave, gently rhythmic
            rgblight_mode_noeeprom(RGBLIGHT_MODE_RAINBOW_SWIRL);
            rgblight_set_speed_noeeprom(40);
            break;
        // _WINDOW, _TEXT, _SELECT — will get their own colour & effect
    }
    return state;
}
#endif

// ── Init ────────────────────────────────────────────────────
void keyboard_post_init_user(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_enable_noeeprom();
    layer_state_set_user(layer_state);  // apply BASE colours at boot
#endif
    gpio_set_pin_output(GP25);
    gpio_write_pin_high(GP25);

    gpio_set_pin_input_high(ENCODER_BTN_PIN);
    gpio_set_pin_input_high(SELECTOR_BTN_PIN);
}

// ── OLED ────────────────────────────────────────────────────
#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

// Write exactly 21 chars to one row — no stale pixels possible.
static void write_line(uint8_t row, const char *str) {
    char buf[22];
    snprintf(buf, sizeof(buf), "%-21.21s", str);
    oled_set_cursor(0, row);
    oled_write(buf, false);
}

// ── Boot animation ───────────────────────────────────────────
static uint32_t boot_start = 0;
#define BOOT_TOTAL_MS 2800

static void render_boot(void) {
    if (boot_start == 0) boot_start = timer_read32() | 1;
    uint32_t t = timer_elapsed32(boot_start);

    // All 8 rows written every frame — no leftover pixels
    write_line(0, "");
    write_line(1, "");
    write_line(2, t >=  800 ? "          I AM" : "             I");
    write_line(3, t >= 1200 ? "          ROOT" : "");
    write_line(4, "");
    write_line(6, "");
    write_line(7, "");
}

// ── Key label (max buflen-1 chars) ──────────────────────────
static void get_key_label(uint16_t kc, char *buf, uint8_t buflen) {
    if (kc == MO(_SELECT)) {
        snprintf(buf, buflen, "SEL");
        return;
    }
    const char *s = get_keycode_string(kc);
    if (s[0] == 'K' && s[1] == 'C' && s[2] == '_') s += 3;
    snprintf(buf, buflen, "%.*s", buflen - 1, s);
}

// ── Layer header (row 0): prev [CUR] next ───────────────────
static void render_header(uint8_t layer) {
    char line[22];
    const char *p = layer_name_short(prev_layer(layer));
    const char *c = layer_name_short(layer);
    const char *n = layer_name_short(next_layer(layer));
    snprintf(line, sizeof(line), "%-5s [%-5s] %-5s", p, c, n);
    write_line(0, line);
}

// ── Normal view: last key info (rows 1–7) ───────────────────
static void render_keyinfo(void) {
    char buf[22];
    char label[15];
    get_key_label(last_keycode, label, sizeof(label));

    write_line(1, "---------------------");
    snprintf(buf, sizeof(buf), "Key:  %.14s", label);
    write_line(2, buf);
    snprintf(buf, sizeof(buf), "Code: 0x%04X", last_keycode);
    write_line(3, buf);
    snprintf(buf, sizeof(buf), "Pos:  R%uC%u", last_row, last_col);
    write_line(4, buf);
    write_line(5, "");
    write_line(6, "");
    write_line(7, "");
}

// ── Legend view: 3×3 key grid (rows 1–7) ────────────────────
static void render_legend(uint8_t layer) {
    char line[22];
    write_line(1, "---------------------");

    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        char l0[8], l1[8], l2[8];
        get_key_label(keymaps[layer][r][0], l0, sizeof(l0));
        get_key_label(keymaps[layer][r][1], l1, sizeof(l1));
        get_key_label(keymaps[layer][r][2], l2, sizeof(l2));
        snprintf(line, sizeof(line), "%-7s%-7s%-7s", l0, l1, l2);
        write_line(2 + r, line);
    }

    write_line(5, "  [Enc btn]=toggle");
    write_line(6, "");
    write_line(7, "");
}

// ── Main OLED task ──────────────────────────────────────────
bool oled_task_user(void) {
    if (boot_start == 0 || timer_elapsed32(boot_start) < BOOT_TOTAL_MS) {
        render_boot();
        return false;
    }

    uint8_t cur = active_layer();
    render_header(cur);

    if (legend_active) {
        render_legend(cur);
    } else {
        render_keyinfo();
    }

    return false;
}

#endif // OLED_ENABLE
