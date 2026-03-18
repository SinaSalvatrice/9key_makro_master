#include QMK_KEYBOARD_H
#include "i2c_master.h"
#include "gpio.h"

#define OLED_I2C_ADDR (OLED_DISPLAY_ADDRESS << 1)
#define OLED_CMD 0x00
#define OLED_DATA 0x40

static bool oled_send_cmd_bytes(const uint8_t *data, uint16_t length) {
    return i2c_transmit(OLED_I2C_ADDR, data, length, 100) == I2C_STATUS_SUCCESS;
}

static bool oled_send_data_bytes(const uint8_t *data, uint16_t length) {
    return i2c_write_register(OLED_I2C_ADDR, OLED_DATA, data, length, 100) == I2C_STATUS_SUCCESS;
}

static bool raw_oled_init(void) {
    static const uint8_t init1[] = {
        OLED_CMD,
        0xAE,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0xCF,
        0xD9, 0xF1,
        0xDB, 0x20,
        0xA4,
        0xA6,
        0x2E,
        0xAF
    };

    return oled_send_cmd_bytes(init1, sizeof(init1));
}

static bool raw_oled_set_full_window(void) {
    static const uint8_t window[] = {
        OLED_CMD,
        0x21, 0x00, 0x7F,
        0x22, 0x00, 0x07
    };

    return oled_send_cmd_bytes(window, sizeof(window));
}

static void raw_oled_fill_pattern(void) {
    static uint8_t pattern[128];

    for (uint8_t i = 0; i < sizeof(pattern); i++) {
        pattern[i] = (i & 1) ? 0xAA : 0x55;
    }

    if (!raw_oled_set_full_window()) {
        uprintf("OLED window set failed\n");
        return;
    }

    for (uint8_t page = 0; page < 8; page++) {
        if (oled_send_data_bytes(pattern, sizeof(pattern)) != true) {
            uprintf("OLED data page %d failed\n", page);
            return;
        }
    }
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

    if (status_3c == I2C_STATUS_SUCCESS) {
        if (raw_oled_init()) {
            uprintf("OLED raw init OK\n");
            raw_oled_fill_pattern();
        } else {
            uprintf("OLED raw init failed\n");
        }
    }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_1, KC_2, KC_3,
        KC_4, KC_5, KC_6,
        KC_7, KC_8, KC_9
    ),
};
