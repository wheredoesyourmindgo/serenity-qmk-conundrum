// Copyright 2021 Google LLC.
// SPDX-License-Identifier: Apache-2.0

#include "mod_row_shift.h"
#include "serenity.h"

// One side of the bottom row: its outer layer key and its Ctrl, Alt and GUI keys
typedef struct {
    uint8_t layer;
    uint8_t shift;      // Shift bit this side holds
    uint8_t mods;       // This side's Ctrl, Alt and GUI bits
    bool    layer_held;  // Layer key is held (not tapped)
    bool    shift_held;  // Layer key is acting as Shift
    bool    shift_added; // Shift wasn't already held, so releasing it here is safe
} bottom_row_side_t;

static bottom_row_side_t left  = {ACTN, MOD_BIT(KC_LSFT), MOD_BIT(KC_LCTL) | MOD_BIT(KC_LALT) | MOD_BIT(KC_LGUI)};
static bottom_row_side_t right = {HRDWR, MOD_BIT(KC_RSFT), MOD_BIT(KC_RCTL) | MOD_BIT(KC_RALT) | MOD_BIT(KC_RGUI)};

static void add_shift(bottom_row_side_t *side) {
    side->shift_held  = true;
    side->shift_added = !(get_mods() & side->shift);
    register_mods(side->shift);
}

static void remove_shift(bottom_row_side_t *side) {
    side->shift_held = false;
    if (side->shift_added) {
        unregister_mods(side->shift);
    }
}

// Layer key held or released. Returns false to hold Shift instead of turning on the layer.
static bool process_layer_key(bottom_row_side_t *side, keyrecord_t *record) {
    if (record->event.pressed) {
        side->layer_held = true;
        if (get_mods() & side->mods) { // Modifier first
            add_shift(side);
            return false;
        }
    } else {
        side->layer_held = false;
        if (side->shift_held) {
            remove_shift(side);
        }
    }
    return true;
}

// Ctrl, Alt or GUI (`mod`) held or released as a modifier
static void process_mod_key(bottom_row_side_t *side, uint8_t mod, keyrecord_t *record) {
    if (record->event.pressed) {
        // Layer key first. Only on the layer itself, so e.g. Function (Action + Hardware) keeps its mods.
        if (side->layer_held && !side->shift_held && get_highest_layer(layer_state) == side->layer) {
            layer_off(side->layer);
            add_shift(side);
        }
    } else if (side->shift_held && !(get_mods() & side->mods & ~mod)) {
        // Last modifier released: back to the layer if its key is still held
        remove_shift(side);
        if (side->layer_held) {
            layer_on(side->layer);
        }
    }
}

bool process_mod_row_shift(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MO(ACTN):
            return process_layer_key(&left, record);
        case LT(HRDWR, KC_LEFT):
            if (record->tap.count == 0) {
                return process_layer_key(&right, record);
            }
            break;
        case KC_LCTL:
        case KC_LALT:
        case KC_LGUI:
            process_mod_key(&left, MOD_BIT(keycode), record);
            break;
        case KC_RCTL:
        case KC_RALT:
        case KC_RGUI:
            process_mod_key(&right, MOD_BIT(keycode), record);
            break;
        // Arrow cluster mod-taps, when held
        case RCTL_T(KC_RIGHT):
            if (record->tap.count == 0) {
                process_mod_key(&right, MOD_BIT(KC_RCTL), record);
            }
            break;
        case RALT_T(KC_UP):
            if (record->tap.count == 0) {
                process_mod_key(&right, MOD_BIT(KC_RALT), record);
            }
            break;
        case RGUI_T(KC_DOWN):
            if (record->tap.count == 0) {
                process_mod_key(&right, MOD_BIT(KC_RGUI), record);
            }
            break;
    }
    return true;
}
