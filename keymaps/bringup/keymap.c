#include QMK_KEYBOARD_H
#include "gpio.h"
#include <stdio.h>

// ── Layer enum ──────────────────────────────────────────────
enum layers {
    _BASE,
    _NAV,
    _EDIT,
    _MEDIA,
    _FN,
    _DEV,
    _DEV2,
    _RGB,
    _SELECT,
    _LAYER_COUNT  // always last
};

// ── Layer names ─────────────────────────────────────────────
static const char *layer_name(uint8_t l) {
    switch (l) {
        case _BASE:   return "BASE";
        case _NAV:    return "NAV";
        case _EDIT:   return "EDIT";
        case _MEDIA:  return "MEDIA";
        case _FN:     return "FN";
        case _DEV:    return "DEV";
        case _DEV2:   return "DEV2";
        case _RGB:    return "RGB";
        case _SELECT: return "SELECT";
        default:      return "BASE";
    }
}

// Short label (max 6 chars) for the legend grid
static const char *layer_name_short(uint8_t l) {
    switch (l) {
        case _BASE:   return "BASE";
        case _NAV:    return "NAV";
        case _EDIT:   return "EDIT";
        case _MEDIA:  return "MEDIA";
        case _FN:     return "FN";
        case _DEV:    return "DEV";
        case _DEV2:   return "DEV2";
        case _RGB:    return "RGB";
        case _SELECT: return "SEL";
        default:      return "???";
    }
}

// ── State tracked for OLED ──────────────────────────────────
static uint16_t last_keycode   = KC_NO;
static uint8_t  last_row       = 0;
static uint8_t  last_col       = 0;
static bool     encoder_btn_held = false;
static bool     oled_needs_clear = true;

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
        MO(_SELECT),            KC_UP,              LALT(KC_TAB),
        KC_LEFT,                KC_LEFT_GUI,        KC_RGHT,
        LCTL(KC_Z),             KC_DOWN,            LCTL(KC_R)
    ),

    [_NAV] = LAYOUT(
        MO(_SELECT),            KC_UP,              LALT(KC_TAB),
        KC_LEFT,                KC_ENT,             KC_RGHT,
        LCTL(KC_Z),             KC_DOWN,            LCTL(KC_R)
    ),

    [_EDIT] = LAYOUT(
        MO(_SELECT),            LCTL(KC_C),         LCTL(KC_V),
        LCTL(KC_X),             LCTL(KC_ENT),       KC_NO,
        LCTL(LSFT(KC_Z)),      KC_SPC,             KC_BSPC
    ),

    [_MEDIA] = LAYOUT(
        MO(_SELECT),            KC_MSEL,            KC_MNXT,
        KC_MRWD,                KC_MPLY,            KC_MFFD,
        KC_DOWN,                KC_MSTP,            KC_UP
    ),

    [_DEV] = LAYOUT(
        MO(_SELECT),            KC_NO,              KC_NO,
        KC_NO,                  KC_NO,              KC_NO,
        KC_NO,                  KC_NO,              KC_NO
    ),

    [_DEV2] = LAYOUT(
        MO(_SELECT),            TO(_EDIT),          TO(_MEDIA),
        TO(_FN),                TO(_BASE),          KC_NO,
        KC_NO,                  KC_NO,              KC_NO
    ),

    [_FN] = LAYOUT(
        MO(_SELECT),            KC_F14,             KC_F15,
        KC_F16,                 KC_F17,             KC_F18,
        KC_F19,                 KC_F20,             KC_F21
    ),

    [_RGB] = LAYOUT(
        MO(_SELECT),            UG_SPDD,            UG_TOGG,
        UG_HUEU,                UG_HUED,            UG_VALU,
        UG_SATU,                UG_SATD,            UG_VALD
    ),

    [_SELECT] = LAYOUT(
        KC_NO,                  TO(_BASE),          TO(_NAV),
        TO(_EDIT),              TO(_MEDIA),         TO(_RGB),
        TO(_DEV),               TO(_DEV2),          TO(_FN)
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

// ── Encoder: volume (normal) ────────────────────────────────
bool encoder_update_user(uint8_t index, bool clockwise) {
    (void)index;
    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }
    return false;
}

// ── Encoder button via matrix_scan (GP10) ───────────────────
void matrix_scan_user(void) {
    static bool was_pressed = false;
    bool pressed = (gpio_read_pin(ENCODER_BTN_PIN) == 0);

    if (pressed != was_pressed) {
        encoder_btn_held = pressed;
        oled_needs_clear = true;  // clear once on toggle
    }
    was_pressed = pressed;
}

// ── Init ────────────────────────────────────────────────────
void keyboard_post_init_user(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3);
    rgblight_set_speed_noeeprom(128);
    rgblight_sethsv_noeeprom(170, 255, 80);
#endif

    gpio_set_pin_output(GP25);
    gpio_write_pin_high(GP25);

    gpio_set_pin_input_high(ENCODER_BTN_PIN);
}

// ── OLED ────────────────────────────────────────────────────
#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

// ── Boot animation state ────────────────────────────────────
static uint32_t boot_timer   = 0;
static uint8_t  boot_phase   = 0;  // 0..3 = animation phases, 4 = done
#define BOOT_PHASE_DELAY 600        // ms per phase

static void render_boot(void) {
    if (boot_phase == 0 && boot_timer == 0) {
        oled_clear();  // one-time clear at start
        boot_timer = timer_read32();
    }

    uint32_t elapsed = timer_elapsed32(boot_timer);

    // Phase 0: "9KEY" centered (always shown)
    oled_set_cursor(8, 2);
    oled_write_P(PSTR("I"), false);

    // Phase 1: + "MAKRO"
    if (elapsed >= BOOT_PHASE_DELAY) {
        oled_set_cursor(7, 4);
        oled_write_P(PSTR("AM"), false);
    }
    // Phase 2: + "MASTER"
    if (elapsed >= BOOT_PHASE_DELAY * 2) {
        oled_set_cursor(7, 6);
        oled_write_P(PSTR("ROOT"), false);
    }
    // Phase 3: hold complete logo briefly, then transition
    if (elapsed >= BOOT_PHASE_DELAY * 4) {
        boot_phase = 4;
        oled_needs_clear = true;  // clear once when switching to normal view
    }
}

// ── Keycode short name (max 7 chars for grid cell) ──────────
static void get_key_label(uint16_t kc, char *buf, uint8_t buflen) {
    const char *s = get_keycode_string(kc);
    // Strip "KC_" prefix for brevity if present
    if (s[0] == 'K' && s[1] == 'C' && s[2] == '_') {
        s += 3;
    }
    snprintf(buf, buflen, "%.*s", buflen - 1, s);
}

// ── Render the layer header line ────────────────────────────
// Format:  "< NAV   [BASE]   EDIT >"
// Line 0 of the OLED (21 chars wide)
static void render_header(uint8_t cur_layer) {
    char line[22];
    const char *prev = layer_name_short(prev_layer(cur_layer));
    const char *curr = layer_name(cur_layer);
    const char *next = layer_name_short(next_layer(cur_layer));

    snprintf(line, sizeof(line), "%-5s [%-6s] %5s", prev, curr, next);
    oled_set_cursor(0, 0);
    oled_write_ln(line, false);
}

// ── Render: key-press info (normal view) ────────────────────
static void render_keyinfo(void) {
    char buf[22];

    // Line 1: separator
    oled_set_cursor(0, 1);
    oled_write_ln_P(PSTR("---------------------"), false);

    // Line 2: function / key name
    char label[18];
    get_key_label(last_keycode, label, sizeof(label));
    snprintf(buf, sizeof(buf), "Key:  %-15s", label);
    oled_set_cursor(0, 2);
    oled_write_ln(buf, false);

    // Line 3: raw keycode hex
    snprintf(buf, sizeof(buf), "Code: 0x%04X", last_keycode);
    oled_set_cursor(0, 3);
    oled_write_ln(buf, false);

    // Line 4: matrix position
    snprintf(buf, sizeof(buf), "Pos:  R%dC%d", last_row, last_col);
    oled_set_cursor(0, 4);
    oled_write_ln(buf, false);

    // Lines 5-7: blank
    oled_set_cursor(0, 5);
    oled_write_ln_P(PSTR(""), false);
    oled_set_cursor(0, 6);
    oled_write_ln_P(PSTR(""), false);
    oled_set_cursor(0, 7);
    oled_write_ln_P(PSTR(""), false);
}

// ── Render: layer key legend (encoder btn held) ─────────────
// Shows a 3×3 grid of short key labels for the current layer
static void render_legend(uint8_t cur_layer) {
    char line[22];

    // Line 1: separator
    oled_set_cursor(0, 1);
    oled_write_ln_P(PSTR("---------------------"), false);

    // 3 rows × 3 cols
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        uint16_t kc0 = keymaps[cur_layer][r][0];
        uint16_t kc1 = keymaps[cur_layer][r][1];
        uint16_t kc2 = keymaps[cur_layer][r][2];

        char l0[8], l1[8], l2[8];
        get_key_label(kc0, l0, sizeof(l0));
        get_key_label(kc1, l1, sizeof(l1));
        get_key_label(kc2, l2, sizeof(l2));

        snprintf(line, sizeof(line), "%-7s%-7s%-7s", l0, l1, l2);
        oled_set_cursor(0, 2 + r);
        oled_write_ln(line, false);
    }

    // Line 5: hint
    oled_set_cursor(0, 5);
    oled_write_ln_P(PSTR("  [Enc] = Legend"), false);

    oled_set_cursor(0, 6);
    oled_write_ln_P(PSTR(""), false);
    oled_set_cursor(0, 7);
    oled_write_ln_P(PSTR(""), false);
}

// ── Main OLED task ──────────────────────────────────────────
bool oled_task_user(void) {
    // Boot animation first
    if (boot_phase < 4) {
        render_boot();
        return false;
    }

    // One-time clear when transitioning from boot or between modes
    if (oled_needs_clear) {
        oled_clear();
        oled_needs_clear = false;
    }

    uint8_t cur = active_layer();
    render_header(cur);

    if (encoder_btn_held) {
        render_legend(cur);
    } else {
        render_keyinfo();
    }

    return false;
}

#endif // OLED_ENABLE
