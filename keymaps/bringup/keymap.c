#include QMK_KEYBOARD_H
#include "gpio.h"

bool encoder_update_user(uint8_t index, bool clockwise) {
    (void)index;

    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }

    return false;
}

void keyboard_post_init_user(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3);
    rgblight_set_speed_noeeprom(128);
    rgblight_sethsv_noeeprom(170, 255, 80);
#endif

    // Power on OLED via GP25
    gpio_set_pin_output(GP25);
    gpio_write_pin_high(GP25);
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

static bool oled_initialized = false;

bool oled_task_user(void) {
    if (oled_initialized) {
        return false;
    }
    oled_initialized = true;

    oled_clear();
    oled_set_cursor(2, 1);
    oled_write_ln_P(PSTR("9KEY"), false);
    oled_set_cursor(1, 3);
    oled_write_ln_P(PSTR("MAKRO"), false);
    oled_set_cursor(0, 5);
    oled_write_ln_P(PSTR("MASTER"), false);

    return false;
}
#endif

enum layers {
    _BASE,
    _NAV,
    _EDIT,
    _MEDIA,
    _FN,
    _DEV,
    _DEV2,
    _RGB,
    _SELECT
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
        MO(_SELECT),                KC_UP,              LALT(KC_TAB),
        KC_LEFT,                KC_LEFT_GUI,        KC_RGHT,
        LCTL(KC_Z),             KC_DOWN,            LCTL(KC_R)
    ),

    [_NAV] = LAYOUT(
        MO(_SELECT),                KC_UP,              LALT(KC_TAB),
        KC_LEFT,                KC_ENT,             KC_RGHT,
        LCTL(KC_Z),             KC_DOWN,            LCTL(KC_R)
    ),

    [_EDIT] = LAYOUT(
        MO(_SELECT),                LCTL(KC_C),         LCTL(KC_V),
        LCTL(KC_X),             LCTL(KC_ENT),       KC_NO,
        LCTL(LSFT(KC_Z)),       KC_SPC,             KC_BSPC
    ),

    [_MEDIA] = LAYOUT(
        MO(_SELECT),                KC_MSEL,            KC_MNXT,
        KC_MRWD,                KC_MPLY,            KC_MFFD,
        KC_DOWN,                KC_MSTP,            KC_UP
    ),

   [_DEV] = LAYOUT(
        MO(_SELECT),                KC_NO,             KC_NO,
        KC_NO,                  KC_NO,             KC_NO,
        KC_NO,                  KC_NO,             KC_NO
    ),

    [_DEV2] = LAYOUT(
        MO(_SELECT),                TO(_EDIT),          TO(_MEDIA),
        TO(_FN),                TO(_BASE),          KC_NO,
        KC_NO,                  KC_NO,              KC_NO
    ),

    [_FN] = LAYOUT(
        MO(_SELECT),                KC_F14,            KC_F15,
        KC_F16,                 KC_F17,            KC_F18,
        KC_F19,                 KC_F20,            KC_F21
    ),

    [_RGB] = LAYOUT(
        MO(_SELECT),            UG_SPDD,            UG_TOGG,
        UG_HUEU,                UG_HUED,            UG_VALU,
        UG_SATU,                UG_SATD,            UG_VALD
    ),

    [_SELECT] = LAYOUT(
        KC_NO,                 TO(_BASE),           TO(_NAV),
        TO(_EDIT),             TO(_MEDIA),          TO(_RGB),
        TO(_DEV),               TO(_DEV2),          TO(_FN)
    ),
};

// Fallback: holding R0C0 (top-left) and pressing R2C2 (bottom-right) returns to _BASE on any layer.
static bool r0c0_held = false;

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

    return true;
}
