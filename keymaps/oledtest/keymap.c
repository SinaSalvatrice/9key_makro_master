// This file only needs to provide the keymap for the oledtest keymap.
// All shared logic is in ../via/keymap.c

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_1,         KC_2,        KC_3,
        KC_4,         MO(_SELECT), KC_6,
        KC_7,         KC_8,        KC_9
    ),

    [_NAV] = LAYOUT(
        KC_LEFT, KC_UP,   KC_RGHT,
        KC_HOME, KC_ENT,  KC_END,
        KC_PGDN, KC_DOWN, KC_PGUP
    ),

    [_EDIT] = LAYOUT(
        LCTL(KC_Z), LCTL(KC_C), LCTL(KC_V),
        LCTL(KC_X), KC_ENT,     KC_BSPC,
        LCTL(KC_A), KC_SPC,     LCTL(LSFT(KC_Z))
    ),

    [_MEDIA] = LAYOUT(
        KC_MPRV, KC_MPLY, KC_MNXT,
        KC_MUTE, KC_MSTP, KC_VOLU,
        KC_NO,   KC_NO,   KC_VOLD
    ),

    [_FN] = LAYOUT(
        KC_F13, KC_F14, KC_F15,
        KC_F16, KC_F17, KC_F18,
        KC_F19, KC_F20, KC_F21
    ),

    [_RGB] = LAYOUT(
        UG_TOGG, UG_HUEU, UG_HUED,
        UG_SATU, UG_VALU, UG_VALD,
        UG_SATD, UG_NEXT, UG_PREV
    ),

    [_SELECT] = LAYOUT(
        TO(_NAV),  TO(_EDIT), TO(_MEDIA),
        TO(_FN),   KC_TRNS,   TO(_RGB),
        KC_NO,     KC_NO,     KC_NO
    )
};
