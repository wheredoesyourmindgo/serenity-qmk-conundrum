#include "cmd_tab_switcher.h"
#include "serenity.h"

static bool     is_cmd_tab_active     = false;
static bool     is_cmd_tab_held       = false;
static uint16_t cmd_tab_timer         = 0;
static uint16_t cmd_tab_timer_timeout = CMD_TAB_TIMEOUT;

static void cancel_cmd_tab(void) {
    unregister_mods(MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT));
    is_cmd_tab_active     = false;
    cmd_tab_timer_timeout = CMD_TAB_TIMEOUT;
}

// Open the switcher, or speed up the timeout if it's already open (ie. when moving left and right in switcher)
static void open_cmd_tab(void) {
    if (!is_cmd_tab_active) {
        is_cmd_tab_active = true;
        register_mods(MOD_BIT(KC_LGUI));
    } else {
        cmd_tab_timer_timeout = CMD_TAB_FAST_TIMEOUT;
    }
}

void cmd_tab_switcher_layer_state(layer_state_t state) {
    // Any layer change (eg. letting go of the OS layer) picks the selected app
    if (is_cmd_tab_active) {
        cancel_cmd_tab();
    }
}

void cmd_tab_switcher_matrix_scan_user(void) {
    if (is_cmd_tab_active) {
        // RGUI mod will be active when encoder is in use so don't cancel cmd shift once it's starts programmatically with cmd_tab_next/previous(). This works since RGUI is used by encoder and LGUI is used by cmd tab switcher.
        if (timer_elapsed(cmd_tab_timer) > cmd_tab_timer_timeout && !is_cmd_tab_held && !MODS_RGUI) {
            cancel_cmd_tab();
        }
    }
}

void cmd_tab_previous(void) {
    open_cmd_tab();
    register_mods(MOD_BIT(KC_LSFT));
    tap_code(KC_TAB);
    unregister_mods(MOD_BIT(KC_LSFT));
    cmd_tab_timer = timer_read();
}

void cmd_tab_next(void) {
    open_cmd_tab();
    tap_code(KC_TAB);
    cmd_tab_timer = timer_read();
}

bool process_cmd_tab_switcher(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case CMD_TAB_NXT:
        case CMD_TAB_PRV: {
            bool reverse = keycode == CMD_TAB_PRV;
            if (record->event.pressed) {
                open_cmd_tab();
                is_cmd_tab_held = true;
                if (reverse) {
                    register_mods(MOD_BIT(KC_LSFT));
                } else {
                    unregister_mods(MOD_BIT(KC_LSFT));
                }
                tap_code(KC_TAB);
            } else {
                // Start the timer when the key is released, not pressed
                cmd_tab_timer   = timer_read();
                is_cmd_tab_held = false;
                if (reverse) {
                    unregister_mods(MOD_BIT(KC_LSFT));
                }
            }
            break;
        }
        case LGUI(KC_Q):
        case LGUI(KC_H):
            if (record->event.pressed) {
                if (is_cmd_tab_active) {
                    cmd_tab_timer_timeout = CMD_TAB_TIMEOUT;
                }
            }
            break;
        case OS_EXPOSE:
            if (record->event.pressed) {
                if (is_cmd_tab_active) {
                    tap_code(KC_UP);
                    return false;
                }
            }
            break;
        case KC_ESC:
            // macOS closes the switcher on Cmd+Esc; release Cmd too so firmware and OS stay in sync
            if (record->event.pressed) {
                if (is_cmd_tab_active) {
                    tap_code(KC_ESC);
                    cancel_cmd_tab();
                    return false;
                }
            }
            break;
        // instantly switch to app after RGUI is let go when cmd tab is activated using encoder and RGUI
        case KC_RGUI:
        case RGUI_T(KC_LEFT):
            if (!record->event.pressed) {
                if (is_cmd_tab_active && !MODS_LGUI) {
                    cancel_cmd_tab();
                }
            }
            break;
    }

    return true;
}
