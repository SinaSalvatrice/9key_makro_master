#include QMK_KEYBOARD_H
#include "gpio.h"

void keyboard_post_init_user(void) {
    gpio_set_pin_output(OLED_POWER_PIN);
    gpio_write_pin_high(OLED_POWER_PIN);
    wait_ms(50);

    gpio_set_pin_output(ONBOARD_LED_PIN);
    gpio_write_pin_high(ONBOARD_LED_PIN);
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

bool oled_task_user(void) {
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_ln_P(PSTR("OLED TEST"), false);
    oled_write_ln_P(PSTR("9KEY MAKRO"), false);
    oled_write_ln_P(PSTR("MASTER"), false);
    oled_write_ln_P(PSTR("ADDR 0x3C"), false);
    oled_write_ln_P(PSTR("PWR GP25"), false);
    oled_write_ln_P(PSTR("I2C GP0/GP1"), false);
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
