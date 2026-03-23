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

static const char *layer_name(uint8_t layer) {
    switch (layer) {
        case _BASE:   return "BASE";
        case _NAV:    return "NAV";
        case _EDIT:   return "EDIT";
        case _MEDIA:  return "MEDIA";
        case _FN:     return "FN";
        case _RGB:    return "RGB";
        case _SELECT: return "SELECT";
        default:      return "UNK";
    }
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

bool oled_task_user(void) {
    oled_clear();
    oled_write_P(PSTR("Layer "), false);
    oled_write_ln(layer_name(get_highest_layer(layer_state | default_layer_state)), false);
    return false;
}
#endif

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
        KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO
    ),

    [_SELECT] = LAYOUT(
        TO(_NAV),  TO(_EDIT), TO(_MEDIA),
        TO(_FN),   KC_TRNS,   TO(_RGB),
        KC_NO,     KC_NO,     KC_NO
    )

};

    #if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]   = { ENCODER_CCW_CW(MS_WHLD, MS_WHLU) },
    [_NAV]    = { ENCODER_CCW_CW(KC_LEFT, KC_RGHT) },
    [_EDIT]   = { ENCODER_CCW_CW(KC_LEFT, KC_RGHT) },
    [_MEDIA]  = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [_FN]     = { ENCODER_CCW_CW(KC_DOWN, KC_UP) },
    [_RGB]    = { ENCODER_CCW_CW(KC_NO,   KC_NO)   },
    [_SELECT] = { ENCODER_CCW_CW(KC_NO,   KC_NO)   },

    #endif

};
