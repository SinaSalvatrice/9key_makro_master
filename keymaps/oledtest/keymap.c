#include QMK_KEYBOARD_H
#include "i2c_master.h"
#include "gpio.h"

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

void keyboard_post_init_user(void) {
    gpio_set_pin_output(ONBOARD_LED_PIN);
    gpio_write_pin_high(ONBOARD_LED_PIN);

    i2c_status_t status_3c;
    i2c_status_t status_3d;

    i2c_init();
    status_3c = i2c_ping_address(0x3C << 1, 100);
    status_3d = i2c_ping_address(0x3D << 1, 100);

    uprintf("OLED probe start\n");
    uprintf("I2C 0x3C status: %d\n", status_3c);
    uprintf("I2C 0x3D status: %d\n", status_3d);
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
