// Copyright 2021 Google LLC.
// SPDX-License-Identifier: Apache-2.0

#include "mash_arrows.h"
#include "serenity.h"

// The cluster's mod-taps: the modifier each one holds and the arrow it types
static const struct {
    uint8_t mod;
    uint8_t arrow;
} cluster_mod_taps[] = {
    {MOD_BIT(KC_RGUI), KC_DOWN},
    {MOD_BIT(KC_RALT), KC_UP},
    {MOD_BIT(KC_RCTL), KC_RIGHT},
};

bool process_mash_arrows(uint16_t keycode, keyrecord_t* record) {
    // Only arrow taps, and not while Mod Row Shift is held (cluster modifiers are deliberate then)
    if (!is_arrow_cluster_key(keycode) || !record->event.pressed || record->tap.count == 0 || MODS_RSFT) {
        return true;
    }
    // Type the arrow of each cluster key held as a modifier, leaving other modifiers alone
    for (uint8_t i = 0; i < ARRAY_SIZE(cluster_mod_taps); ++i) {
        if (get_mods() & cluster_mod_taps[i].mod) {
            del_mods(cluster_mod_taps[i].mod);
            tap_code(cluster_mod_taps[i].arrow);
        }
    }
    return true; // QMK types this key's arrow
}
