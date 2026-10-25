// Copyright 2021 Google LLC.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include QMK_KEYBOARD_H

/**
 * Mod Row Shift: Shift on the bottom row
 *
 * With the hand down on the bottom row, the outer layer key on each side (Action on the
 * left, Hardware on the right) acts as Shift when used with that side's Ctrl, Alt or GUI.
 * Pressing all four with one hand gives Hyper (Ctrl+Alt+GUI+Shift); fewer give e.g. Cmd+Shift.
 *
 * - Modifier first: the layer key holds Shift instead of turning on its layer.
 * - Layer key first: pressing a modifier turns the layer off and adds Shift. Releasing the
 *   last modifier removes Shift and turns the layer back on if its key is still held.
 *
 * A locked layer (Layer Lock) is left alone: its key unlocks it and its modifiers are plain.
 *
 * Call from `process_record_user`, before Layer Lock so it sees every layer key release, and
 * return its result:
 *   if (!process_mod_row_shift(keycode, record)) { return false; }
 */

bool process_mod_row_shift(uint16_t keycode, keyrecord_t* record);
