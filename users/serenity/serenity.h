// Serenity userspace: layers, custom keycodes and helpers shared by every keymap.
#pragma once

#include "quantum.h"
#include "shortcuts.h"

// Modifier state checks
#define MODS_RSFT (get_mods() & MOD_BIT(KC_RSFT))
#define MODS_LSFT (get_mods() & MOD_BIT(KC_LSFT))
#define MODS_RCTRL (get_mods() & MOD_BIT(KC_RCTL))
#define MODS_LCTRL (get_mods() & MOD_BIT(KC_LCTL))
#define MODS_RALT (get_mods() & MOD_BIT(KC_RALT))
#define MODS_LALT (get_mods() & MOD_BIT(KC_LALT))
#define MODS_RGUI (get_mods() & MOD_BIT(KC_RGUI))
#define MODS_LGUI (get_mods() & MOD_BIT(KC_LGUI))
#define MODS_SFT (MODS_LSFT || MODS_RSFT)
#define MODS_CTRL (MODS_LCTRL || MODS_RCTRL)
#define MODS_ALT (MODS_LALT || MODS_RALT)
#define MODS_GUI (MODS_LGUI || MODS_RGUI)
#define ONESHOT_MODS_ACTIVE (get_oneshot_mods())

// Website names (serenity-layout.fyi/firmware) in comments where they differ
enum layers {
    BASE,
    QWRTY,  // Qwerty
    NUMNAV, // Num/Nav
    SYMBL,  // Symbol
    ACTN,   // Action
    HRDWR,  // Hardware
    FUNC,   // Function
    OS,
    MOUSE,
};

enum custom_keycodes {
    CMD_TAB_PRV = SAFE_RANGE,
    CMD_TAB_NXT,
    ENC_BTN,
    SHAKE_MOUSE,
    NUM_DOT, // Num/Nav dot: Shift turns it into a comma (shift_keys.c)
};

// helpers.c
bool is_arrow_cluster_key(uint16_t keycode);
bool check_mods_sft_and_gui(void);
void tap_code_no_mod(uint8_t code);
void tap_code16_no_mod(uint16_t code);
void tap_code_unset_mod(uint8_t code);
void tap_code16_unset_mod(uint16_t code);

// long_press.c
bool process_long_press_keys(uint16_t keycode, keyrecord_t *record);

// rotary_encoder.c: a keymap can override this to handle the encoder itself (return false to skip the default)
bool encoder_update_keymap(uint8_t index, bool clockwise);
