// Serenity userspace: QMK hooks shared by every keymap.
//
// Other parts live in:
//   helpers.c         tap_code*_no_mod() and other small helpers
//   long_press.c      LT(0, kc) tap / long-press shortcut keys
//   tapping.c         per-key tap-hold settings
//   rotary_encoder.c  rotary encoder
//   features/         self-contained features (one-shot mods, cmd-tab switcher, ...)
#include "serenity.h"
#include "stackable_oneshot.h"
#ifdef MOD_ROW_SHIFT_ENABLE
#    include "features/mod_row_shift.h"
#endif
#include "features/cmd_tab_switcher.h"
#include "features/symbol_rolls.h"
#include "features/mash_arrows.h"
#include "process_layer_lock.h" // QMK core Layer Lock (QK_LLCK)

bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        // Ignore Shift presses (e.g. for a double quote)
        case KC_LSFT:
        case KC_RSFT:
            return true;
        // Keycodes that continue Caps Word, with shift applied
        case KC_A ... KC_Z:
        case KC_MINS: // Types _ for CONSTANT_NAMES
            add_weak_mods(MOD_BIT(KC_LSFT));
            return true;
        // Keycodes that continue Caps Word, without shifting
        case KC_1 ... KC_0:
        case KC_BSPC:
        case KC_DEL:
        case KC_QUOT:
        case KC_UNDS:
            return true;
        default:
            return false; // Deactivate Caps Word
    }
}

// Escape first cancels one-shot mods, then a locked layer. Returns true if it cancelled something.
static bool escape_cancels_something(void) {
    if (stackable_oneshot_cancel()) {
        return true;
    }
    const uint8_t layer = get_highest_layer(layer_state);
    if (is_layer_locked(layer)) {
        layer_lock_off(layer);
        return true;
    }
    return false;
}

// Shake the pointer left and right so macOS's "Shake mouse pointer to locate" enlarges it.
// Runs from matrix_scan_user() so the keyboard keeps working while it shakes.
#define SHAKE_SWINGS 8           // Even, so the pointer ends about where it started
#define SHAKE_REPORTS_PER_SWING 3
#define SHAKE_INTERVAL_MS 10
#define SHAKE_DELTA 80

static uint8_t  shake_reports_left = 0;
static uint16_t shake_timer;

static void shake_mouse_scan(void) {
    if (shake_reports_left == 0 || timer_elapsed(shake_timer) < SHAKE_INTERVAL_MS) {
        return;
    }
    shake_timer = timer_read();
    shake_reports_left--;
    uint8_t swing = shake_reports_left / SHAKE_REPORTS_PER_SWING;
    // Keep any held mouse buttons in the report
    report_mouse_t report = mousekey_get_report();
    report.x              = (swing % 2) ? SHAKE_DELTA : -SHAKE_DELTA;
    report.y              = 0;
    host_mouse_send(&report);
}

#ifdef KEY_LOCK_ENABLE
// QMK's cancel_key_lock() forgets locked keys without releasing them, so a drag-lock (Key Lock
// then a mouse button) would stay pressed after leaving the Mouse layer. Mirror which keys Key
// Lock is holding so a layer change can release them.
#define MAX_LOCKED_KEYS 4

static bool    key_lock_watching = false;
static uint8_t locked_keys[MAX_LOCKED_KEYS];
static uint8_t locked_key_count = 0;

static bool forget_locked_key(uint8_t code) {
    for (uint8_t i = 0; i < locked_key_count; i++) {
        if (locked_keys[i] == code) {
            locked_keys[i] = locked_keys[--locked_key_count];
            return true;
        }
    }
    return false;
}

// Runs before Key Lock sees the press, following the same rules as process_key_lock()
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }
    if (keycode == QK_LOCK) {
        key_lock_watching = !key_lock_watching;
    } else if (keycode > 0xFF) {
        key_lock_watching = false; // Key Lock only locks basic keycodes
    } else if (key_lock_watching) {
        key_lock_watching = false;
        if (locked_key_count < MAX_LOCKED_KEYS) {
            locked_keys[locked_key_count++] = keycode;
        }
    } else {
        forget_locked_key(keycode); // Pressing a locked key again unlocks it
    }
    return true;
}

static void release_locked_keys(void) {
    cancel_key_lock();
    key_lock_watching = false;
    while (locked_key_count > 0) {
        unregister_code(locked_keys[--locked_key_count]);
    }
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Shift + GUI + Backspace deletes to end of line (Ctrl-K)
    if (keycode == KC_BACKSPACE && check_mods_sft_and_gui()) {
        if (record->event.pressed) {
            tap_code16_no_mod(LCTL(KC_K));
            return false;
        }
        // Mods are restored after tap_code16_no_mod(), so clear one-shot mods on release
        if (ONESHOT_MODS_ACTIVE) {
            clear_oneshot_mods();
            send_keyboard_report();
        }
    }

    // Feature order matters: each one can consume the key and stop the rest.
    // QMK also runs Layer Lock after this function, but by then the features below may have
    // consumed layer keys (MO/LT). Run it here first so those keys still unlock a locked layer.
    if (!process_layer_lock(keycode, record)) {
        return false;
    }
#ifdef MOD_ROW_SHIFT_ENABLE
    if (!process_mod_row_shift(keycode, record)) {
        return false;
    }
#endif
    if (!process_cmd_tab_switcher(keycode, record)) {
        return false;
    }
    if (!process_symbol_rolls(keycode, record, SYMBL)) {
        return false;
    }
    if (!process_mash_arrows(keycode, record)) {
        return false;
    }
    if (!process_long_press_keys(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case LSFT(LGUI(KC_BSPC)):
            if (record->event.pressed) {
                tap_code16(LCTL(KC_K)); // Delete to end of line
            }
            break;
        case NUM_DOT: // Num/Nav dot. Shifted, the Custom Shift Keys module sends a comma instead
            if (record->event.pressed) {
                register_code(KC_DOT);
            } else {
                unregister_code(KC_DOT);
            }
            return false;
        case ENC_BTN: // Encoder push button
            if (record->event.pressed) {
                // Held or tapped one-shot mods pick the action
                const uint8_t mods = get_mods() | get_oneshot_mods();
                if (mods & MOD_MASK_SHIFT) {
                    tap_code16_no_mod(OS_DRKMD_TGL);
                } else if (mods & MOD_MASK_CTRL) {
                    tap_code16_no_mod(ZOOM_RESET);
                } else if (mods & MOD_MASK_ALT) {
                    tap_code16_no_mod(ZOOM_RESET_APP);
                } else if (mods & MOD_MASK_GUI) {
                    tap_code16_no_mod(LGUI(KC_H)); // Hide app
                } else {
                    tap_code(KC_MUTE);
                }
                stackable_oneshot_cancel(); // A tapped one-shot applies to this press only
            }
            return false;
        case SHAKE_MOUSE:
            if (record->event.pressed) {
                shake_reports_left = SHAKE_SWINGS * SHAKE_REPORTS_PER_SWING;
                shake_timer        = timer_read();
            }
            break;
        case KC_ESC:
            if (record->event.pressed && escape_cancels_something()) {
                return false; // Don't send Escape
            }
            break;
        case LT(MOUSE, KC_ESC):
            if (record->tap.count > 0 && record->event.pressed && escape_cancels_something()) {
                return false; // Don't send Escape
            }
            break;
    }
    return true;
}

void keyboard_post_init_user(void) {
#ifdef QWERTY_BASE
    set_single_default_layer(QWRTY);
#endif
}

layer_state_t layer_state_set_user(layer_state_t state) {
    cmd_tab_switcher_layer_state(state);

    // Cancel one-shot mods when moving to most layers
    switch (get_highest_layer(state)) {
        case BASE:
        case ACTN:
        case HRDWR:
            break; // Keep them (ACTN and HRDWR need them for function keys)
        default:
            stackable_oneshot_cancel();
            break;
    }

    state = update_tri_layer_state(state, NUMNAV, SYMBL, OS);
    state = update_tri_layer_state(state, HRDWR, ACTN, FUNC);

#ifdef KEY_LOCK_ENABLE
    release_locked_keys();
#endif

    return state;
}

void matrix_scan_user(void) {
    cmd_tab_switcher_matrix_scan_user();
    shake_mouse_scan();
}
