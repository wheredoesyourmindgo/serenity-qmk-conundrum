#pragma once

#include QMK_KEYBOARD_H

/**
 * Cmd-Tab Switcher (macOS-style)
 *
 * Summary
 * -------
 * Emulates macOS Cmd+Tab app switcher with smooth “scrub” behavior.
 * - Starts switcher on first press
 * - Speeds up timeout while scrubbing left/right
 * - Cancels on any layer change, on Esc, or after inactivity
 * - Plays nice with an encoder that holds RGUI (scrub) while LGUI is used for Cmd-Tab
 *
 * Keycodes (expected)
 * -------------------
 * - CMD_TAB_NXT : move right (Cmd+Tab)
 * - CMD_TAB_PRV : move left  (Cmd+Shift+Tab)
 * - OS_EXPOSE   : when active, sends UpArrow to show the selected app's windows
 * - KC_ESC      : when active, closes the switcher without switching
 *
 * Hooks to wire
 * -------------
 * In your keymap:
 *   bool process_record_user(uint16_t keycode, keyrecord_t *record) {
 *     if (!process_cmd_tab_switcher(keycode, record)) return false;
 *     // ...rest...
 *     return true;
 *   }
 *
 *   void matrix_scan_user(void) {
 *     cmd_tab_switcher_matrix_scan_user();
 *     // ...rest...
 *   }
 *
 *   layer_state_t layer_state_set_user(layer_state_t state) {
 *     cmd_tab_switcher_layer_state(state);
 *     return state;
 *   }
 *
 * Configuration (config.h)
 * ------------------------
 * #define CMD_TAB_TIMEOUT      2000  // ms until switcher auto-closes
 * #define CMD_TAB_FAST_TIMEOUT 600   // ms once scrubbing
 */

#ifndef CMD_TAB_TIMEOUT
#    define CMD_TAB_TIMEOUT 2000
#endif
#ifndef CMD_TAB_FAST_TIMEOUT
#    define CMD_TAB_FAST_TIMEOUT 600
#endif

void cmd_tab_previous(void);
void cmd_tab_next(void);
void cmd_tab_switcher_layer_state(layer_state_t state);
void cmd_tab_switcher_matrix_scan_user(void);
bool process_cmd_tab_switcher(uint16_t keycode, keyrecord_t* record);
