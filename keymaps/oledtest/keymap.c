#include QMK_KEYBOARD_H

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

bool oled_task_user(void) {
    oled_clear();
    oled_set_cursor(0, 1);
    oled_write_ln_P(PSTR("WELCOME TO"), false);
    oled_set_cursor(0, 3);
    oled_write_ln_P(PSTR("9key_makro"), false);
    oled_set_cursor(1, 5);
    oled_write_ln_P(PSTR("master"), false);
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
