// ── Keymaps ─────────────────────────────────────────────────
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
        MO(_SELECT),            KC_UP,              KC_BSPC,
        KC_LEFT,                KC_ENT,             KC_RGHT,
        LCTL(KC_Z),             KC_DOWN,            LCTL(KC_R)
    ),

    [_WINDOW] = LAYOUT(
        MO(_SELECT),            KC_PGDN,           KC_NO,
        LGUI(LCTL(KC_LEFT)),    KC_PGDN,           LGUI(LCTL(KC_RGHT)),
        LSA(LALT(KC_TAB)),      KC_NO,             LALT(KC_TAB)
    ),

    [_TEXT] = LAYOUT(
        MO(_SELECT),            LCTL(KC_A),         LCTL(KC_C),
        LCTL(KC_X),             LCTL(KC_V),         KC_RGHT,
        KC_HOME,                KC_SPACE,           KC_END
    ),

    [_RGB] = LAYOUT(
        MO(_SELECT),            UG_SPDD,            UG_TOGG,
        UG_HUEU,                UG_HUED,            UG_VALU,
        UG_SATU,                UG_SATD,            UG_VALD
    ),


    [_SELECT] = LAYOUT(
        KC_NO,                  TO(_WINDOW),        TO(_TEXT),
        TO(_RGB),               TO(_RGB),           LCTL(KC_F),
        LCTL(KC_S),             MS_BTN2,             TO(_BASE)
    ),
};

// ── Fallback combo: R0C0 + R2C2 → _BASE ────────────────────
static bool r0c0_held = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Track R0C0 for fallback combo
    if (record->event.key.row == 0 && record->event.key.col == 0) {
        r0c0_held = record->event.pressed;
    }
    if (record->event.key.row == 2 && record->event.key.col == 2) {
        if (record->event.pressed && r0c0_held) {
            layer_move(_BASE);
            return false;
        }
    }

    // Track last key press for OLED display
    if (record->event.pressed) {
        last_keycode = keycode;
        last_row     = record->event.key.row;
        last_col     = record->event.key.col;
    }

    return true;
}

// ── Encoder: layer-dependent ────────────────────────────────
bool encoder_update_user(uint8_t index, bool clockwise) {
    (void)index;
    switch (active_layer()) {
        case _BASE:
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
            break;
        case _WINDOW:
            tap_code(clockwise ? KC_PGDN : KC_PGUP);
            break;
        case _TEXT:
            register_code(KC_LCTL);
            tap_code(clockwise ? KC_RGHT : KC_LEFT);
            unregister_code(KC_LCTL);
            break;
        case _RGB:
            tap_code16(clockwise ? UG_VALU : UG_VALD);
            break;
        case _SELECT:
            layer_move(clockwise ? next_layer(active_layer()) : prev_layer(active_layer()));
            break;
        default:
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
            break;
    }
    return false;
}

// ── Encoder button via matrix_scan (GP8) ────────────────────
// ── Selector button via matrix_scan (GP12) ──────────────────
void matrix_scan_user(void) {
    static bool enc_was_pressed = false;
    bool        enc_pressed     = (gpio_read_pin(ENCODER_BTN_PIN) == 0);

    if (enc_pressed && !enc_was_pressed) {
        legend_active = !legend_active;  // toggle legend on press
    }
    enc_was_pressed = enc_pressed;

    static bool sel_was_pressed = false;
    static uint8_t sel_prev_layer = _BASE;
    bool        sel_pressed     = (gpio_read_pin(SELECTOR_BTN_PIN) == 0);

    if (sel_pressed && !sel_was_pressed) {
        sel_prev_layer = active_layer();
        layer_move(_TEXT);
    } else if (!sel_pressed && sel_was_pressed) {
        layer_move(sel_prev_layer);
    }
    sel_was_pressed = sel_pressed;
}
