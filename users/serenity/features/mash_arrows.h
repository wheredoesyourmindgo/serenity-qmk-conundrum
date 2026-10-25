// Copyright 2021 Google LLC.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include QMK_KEYBOARD_H

/**
 * Mash Arrows: sloppy presses on the arrow cluster always type arrows
 *
 * The cluster is LT(HRDWR, KC_LEFT), RGUI_T(KC_DOWN), RALT_T(KC_UP) and RCTL_T(KC_RIGHT).
 * Overlapping presses within the tapping term are handled by get_chordal_hold() in tapping.c.
 * This covers an arrow held past the tapping term (so it became a modifier) when another
 * arrow is tapped: both arrows are typed, and only that cluster modifier is released.
 *
 * Skipped while Mod Row Shift (mod_row_shift.c) holds Right Shift, since the cluster
 * modifiers are then used on purpose.
 *
 * Call from `process_record_user` and return its result:
 *   if (!process_mash_arrows(keycode, record)) { return false; }
 */

bool process_mash_arrows(uint16_t keycode, keyrecord_t* record);
