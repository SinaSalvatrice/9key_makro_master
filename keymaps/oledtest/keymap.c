#include QMK_KEYBOARD_H

enum layers {
    _BASE,
    _NAV,
    _EDIT,
    _MEDIA,
    _FN,
    _RGB,
    _SELECT,
};

static uint16_t last_keycode = KC_NO;
static uint8_t  last_row     = 0;
static uint8_t  last_col     = 0;

static const char *layer_name(uint8_t layer) {
    switch (layer) {
        case 0:
            return "BASE";
        default:
            return "UNK";
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        last_keycode = keycode;
        last_row     = record->event.key.row;
        last_col     = record->event.key.col;
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
    oled_write_ln(layer_name(get_highest_layer(layer_state | default_layer_state)), false);

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

    [_NAV] = LAYOUT(
        MO(_SELECT), KC_UP,   KC_RGHT,
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
        KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO
    ),

    [_SELECT] = LAYOUT(
        TO(_NAV),  TO(_EDIT),  TO(_MEDIA),
        TO(_FN),   TO(_BASE),  TO(_RGB),
        KC_NO,     KC_NO,      KC_NO
    )
};
