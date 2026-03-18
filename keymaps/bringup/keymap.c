#include QMK_KEYBOARD_H
#include "timer.h"
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

#ifdef RGBLIGHT_ENABLE
void keyboard_post_init_user(void) {
    // Initialize OLED power pin
    gpio_set_pin_output(GP25);
    gpio_write_pin_high(GP25);

    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3);
    rgblight_set_speed_noeeprom(128);
    rgblight_sethsv_noeeprom(170, 255, 80);
}
#endif

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

static void render_welcome_animation(void) {
    uint8_t frame = (timer_read32() / 500) % 4;

    oled_clear();
    switch (frame) {
        case 0:
            oled_set_cursor(3, 1);
            oled_write_ln_P(PSTR("9KEY"), false);
            oled_set_cursor(1, 3);
            oled_write_ln_P(PSTR("MAKRO"), false);
            oled_set_cursor(0, 5);
            oled_write_ln_P(PSTR("MASTER"), false);
            break;
        case 1:
            oled_set_cursor(1, 0);
            oled_write_ln_P(PSTR("9key_makro"), false);
            oled_set_cursor(2, 2);
            oled_write_ln_P(PSTR("_master"), false);
            oled_set_cursor(4, 5);
            oled_write_ln_P(PSTR("hello"), false);
            break;
        case 2:
            oled_set_cursor(0, 1);
            oled_write_ln_P(PSTR("> 9key <"), false);
            oled_set_cursor(1, 3);
            oled_write_ln_P(PSTR("makro"), false);
            oled_set_cursor(1, 5);
            oled_write_ln_P(PSTR("master"), false);
            break;
        default:
            oled_set_cursor(0, 1);
            oled_write_ln_P(PSTR("WELCOME TO"), false);
            oled_set_cursor(0, 3);
            oled_write_ln_P(PSTR("9key_makro"), false);
            oled_set_cursor(1, 5);
            oled_write_ln_P(PSTR("master"), false);
            break;
    }
}

bool oled_task_user(void) {
    render_welcome_animation();
    return false;
}
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_1, KC_2, KC_3,
        KC_4, KC_5, KC_6,
        KC_7, KC_8, KC_9
    ),
};
