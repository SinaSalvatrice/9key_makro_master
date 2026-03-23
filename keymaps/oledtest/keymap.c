#include QMK_KEYBOARD_H

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

[0] = LAYOUT(

    KC_1, KC_2, KC_3,

    KC_4, KC_5, KC_6,

    KC_7, KC_8, KC_9

),    /////////////////////okay, wenn ich in diese keymap erweitern will, und um die funktionen der zerlegten  keymap einbauen will, wie sollte ich vorgehen? weil wenn ich das über codex mache, kommt immer ///////////// Compiling: quantum/keymap_introspection.c                                                           [ERRORS]

|

|

|

gmake: *** [builddefs/common_rules.mk:362: .build/obj_9key_makro_master_oledtest/quantum/keymap_introspection.o] Error 1

Error: Process completed with exi}; ///////////////