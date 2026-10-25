#pragma once

#define TAPPING_TERM 200
#define TAPPING_TERM_PER_KEY
// #define TAPPING_TOGGLE 2

// Mouse key speed and acceleration.
#undef MOUSEKEY_DELAY
#define MOUSEKEY_DELAY 0
#undef MOUSEKEY_INTERVAL
#define MOUSEKEY_INTERVAL 16
#undef MOUSEKEY_WHEEL_DELAY
#define MOUSEKEY_WHEEL_DELAY 0
#undef MOUSEKEY_MAX_SPEED
#define MOUSEKEY_MAX_SPEED 6
#undef MOUSEKEY_TIME_TO_MAX
#define MOUSEKEY_TIME_TO_MAX 64

// #define RGBLIGHT_SLEEP true
// #define DEBUG_ACTION

// #define IGNORE_MOD_TAP_INTERRUPT_PER_KEY
#define PERMISSIVE_HOLD_PER_KEY
// Enable per key rapid switch from tap to hold, disables double tap hold auto-repeat.
#define QUICK_TAP_TERM_PER_KEY
#define RETRO_TAPPING_PER_KEY
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
// Only used to keep overlapping arrow-cluster presses as arrows (get_chordal_hold() in tapping.c)
#define CHORDAL_HOLD

// Caps Word, using RSFT_T(KC_ENT) so both shifts is not ideal, use double tap left shift instead
// #define BOTH_SHIFTS_TURNS_ON_CAPS_WORD
#define DOUBLE_TAP_SHIFT_TURNS_ON_CAPS_WORD

// Custom Shift Keys (getreuer/custom_shift_keys community module, table in shift_keys.c)
// Skip when GUI is held, so Shift+GUI+Backspace reaches process_record_user (delete to end of line)
// Must be a number, not MOD_MASK_GUI: the module tests it with #if, where QMK's MOD_BIT_* enum
// values read as 0 and the check silently disappears. 0x88 is MOD_MASK_GUI (checked in shift_keys.c).
#define CUSTOM_SHIFT_KEYS_NEGMODS 0x88

// Select Word (getreuer/select_word community module): send macOS word and line selection hotkeys
#define SELECT_WORD_OS_MAC

// #define LEADER_PER_KEY_TIMING
// #define LEADER_TIMEOUT 250

// Keyboard specific
// #if defined SIGNUM_KEYBOARD
// #undef MOUSEKEY_MOVE_DELTA
// #define MOUSEKEY_MOVE_DELTA 5
//#elif defined BOARDWALK_KEYBOARD
// #define MOUSEKEY_INTERVAL       12
/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed, defaults to 5 for Boardwalk */
//  #undef DEBOUNCE
//  #define DEBOUNCE 10
// #elif defined PLANCK_KEYBOARD
//     #if ENCODER_ENABLE
//     #define ENCODER_RESOLUTION 2
//     #endif
// #elif defined RISTRETTO_KEYBOARD
// #define ENCODER_DIRECTION_FLIP
// #endif
