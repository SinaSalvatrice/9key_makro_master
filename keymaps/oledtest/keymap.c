

#include QMK_KEYBOARD_H
#include "quantum.h"
#include "process_keycode/process_tap_dance.h"

// QMK keycode and function guards (for custom builds)
#ifndef MS_WHLU
#define MS_WHLU KC_NO
#endif
#ifndef MS_WHLD
#define MS_WHLD KC_NO
#endif

#include "../../9key_makro_master.h"

// Fallbacks for QMK types/macros if not present (for non-QMK build environments)
#ifndef keyrecord_t
typedef struct { struct { uint8_t row, col; bool pressed; } event; } keyrecord_t;
#endif
#ifndef MATRIX_ROWS
#define MATRIX_ROWS 3
#endif
#ifndef MATRIX_COLS
#define MATRIX_COLS 3
#endif
#ifndef LAYOUT
#define LAYOUT( \
#ifndef KC_RGHT
#define KC_RGHT KC_NO
#endif
#ifndef KC_LEFT
#define KC_LEFT KC_NO
#endif
#ifndef KC_VOLU
#define KC_VOLU KC_NO
#endif
#endif
#ifndef KC_1
#define KC_1 1
#define KC_2 2
#define KC_3 3
#define KC_4 4
#define KC_5 5
#define KC_6 6
#define KC_7 7
#define KC_8 8
#define KC_9 9
#endif
#ifndef KC_VOLD
#define KC_VOLD KC_NO
#endif
#ifndef KC_TAB
#define KC_TAB KC_NO
#endif
#ifndef LGUI
#define LGUI(x) (x)
#endif
#ifndef LALT
#define LALT(x) (x)
#endif
#ifndef tap_code
#define tap_code(x)
#endif
#ifndef tap_code16
#define tap_code16(x)
#endif
#ifndef layer_on
#define layer_on(x)
#endif
#ifndef layer_off
#define layer_off(x)
#endif
#ifndef layer_move
#define layer_move(x)
#endif
#ifndef get_highest_layer
#define get_highest_layer(x) 0
#endif
#ifndef layer_state
#define layer_state 0
#endif
#ifndef default_layer_state
#define default_layer_state 0
#endif
#include <stdint.h>
#include <stdbool.h>

// --- Hardware Pin/Function Guards (add your actual pin/func definitions if not present) ---
#ifndef ENCODER_BTN_PIN
#define ENCODER_BTN_PIN 0 // <-- Set to your actual encoder button pin
#endif
#ifndef GP25
#define GP25 25 // <-- Set to your actual OLED power pin
#endif
#ifndef KC_NO
#define KC_NO 0
#endif

// Dummy stubs for hardware functions if not present (remove if defined elsewhere)
#ifndef gpio_set_pin_input_high
static inline void gpio_set_pin_input_high(int pin) {}
#endif
#ifndef gpio_set_pin_output
static inline void gpio_set_pin_output(int pin) {}
#endif
#ifndef gpio_write_pin_high
static inline void gpio_write_pin_high(int pin) {}
#endif
#ifndef gpio_read_pin
static inline int gpio_read_pin(int pin) { return 1; }
#endif
#ifndef refresh_feedback
static inline void refresh_feedback(void) {}
#endif

// --- Layer and Selector Definitions (from help&defines) ---
enum layers {
    _BASE,
    _NAV,
    _EDIT,
    _MEDIA,
    _FN,
    _RGB,
    _SELECT,
};

static bool     selector_active = false;
static uint8_t  selector_origin = _BASE;
static uint8_t  selector_target = _BASE;
static uint16_t last_keycode    = KC_NO;
static uint8_t  last_row        = 0;
static uint8_t  last_col        = 0;

static uint8_t active_layer(void) {
    return get_highest_layer(layer_state | default_layer_state);
}

static uint8_t visual_layer(void) {
    return selector_active ? _SELECT : active_layer();
}

static const char *layer_name(uint8_t layer) {
    switch (layer) {
        case _BASE:
            return "BASE";
        case _NAV:
            return "NAV";
        case _EDIT:
            return "EDIT";
        case _MEDIA:
            return "MEDIA";
        case _FN:
            return "FN";
        case _RGB:
            return "RGB";
        case _SELECT:
            return "SELECT";
        default:
            return "UNK";
    }
}


// --- Encoder and Selector Logic (from encoder) ---
void keyboard_post_init_user(void) {
    gpio_set_pin_input_high(ENCODER_BTN_PIN);

    // Initialize OLED power pin
    gpio_set_pin_output(GP25);
    gpio_write_pin_high(GP25);

    refresh_feedback();
}

static void begin_selector(void) {
    if (selector_active) {
        return;
    }

    selector_origin = active_layer();
    if (selector_origin == _SELECT) {
        selector_origin = _BASE;
    }

    selector_target = selector_origin;
    selector_active = true;
    layer_on(_SELECT);
    refresh_feedback();
}

static void finish_selector(void) {
    if (!selector_active) {
        return;
    }

    selector_active = false;
    layer_off(_SELECT);
    layer_move(selector_target);
    refresh_feedback();
}

static void rotate_selector(bool clockwise) {
    if (clockwise) {
        selector_target = (selector_target + 1) % _SELECT;
    } else {
        selector_target = (selector_target == _BASE) ? (_SELECT - 1) : (selector_target - 1);
    }
}

void matrix_scan_user(void) {
    static bool was_pressed = false;
    bool        pressed     = gpio_read_pin(ENCODER_BTN_PIN) == 0;

    if (pressed && !was_pressed) {
        begin_selector();
    }

    if (!pressed && was_pressed) {
        finish_selector();
    }

    was_pressed = pressed;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    (void)index;

    if (selector_active) {
        rotate_selector(clockwise);
        return false;
    }

    switch (active_layer()) {
        case _BASE:
            clockwise ? tap_code(MS_WHLU) : tap_code(MS_WHLD);
            break;
        case _NAV:
            clockwise ? tap_code16(LGUI(KC_TAB)) : tap_code16(LALT(KC_TAB));
            break;
        case _EDIT:
            clockwise ? tap_code(KC_RGHT) : tap_code(KC_LEFT);
            break;
        case _MEDIA:
            clockwise ? tap_code(KC_VOLU) : tap_code(KC_VOLD);
            break;
        case _FN:
            clockwise ? tap_code(KC_RGHT) : tap_code(KC_LEFT);
            break;
        case _RGB:
#ifdef RGBLIGHT_ENABLE
            clockwise ? rgblight_increase_val_noeeprom() : rgblight_decrease_val_noeeprom();
#endif
            break;
        default:
            break;
    }

    return false;
}

// --- Key event tracking and process_record_user ---
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        last_keycode = keycode;
        last_row     = record->event.row;
        last_col     = record->event.col;
    }
    return true;
}


#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

bool oled_task_user(void) {
    char keycode_hex[8];

    oled_clear();
    oled_write_P(PSTR("Layer "), false);
    oled_write_ln(layer_name(visual_layer()), false);

    snprintf(keycode_hex, sizeof(keycode_hex), "0x%04X", last_keycode);

    oled_write_P(PSTR("Key   "), false);
    oled_write_ln(keycode_hex, false);
    oled_write_P(PSTR("Pos   "), false);
    oled_write(get_u8_str(last_row, ' '), false);
    oled_write_P(PSTR(","), false);
    oled_write_ln(get_u8_str(last_col, ' '), false);

    return false;
}
#endif


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_1, KC_2, KC_3,
        KC_4, KC_5, KC_6,
        KC_7, KC_8, KC_9
    ),
    // Add additional layers here as needed
};
