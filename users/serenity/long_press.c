// Tap / long-press keys: LT(0, kc) keys whose tap and hold each send a shortcut.
// The keycodes are defined in shortcuts.h.
#include "serenity.h"

// On press, send `tap_keycode` if tapped or `hold_keycode` if held
static bool tap_or_hold(keyrecord_t *record, uint16_t tap_keycode, uint16_t hold_keycode) {
    if (record->event.pressed) {
        tap_code16(record->tap.count > 0 ? tap_keycode : hold_keycode);
    }
    return false; // Skip default handling.
}

// True on the press that becomes a hold (taps return false)
static bool is_hold_press(keyrecord_t *record) {
    return record->event.pressed && record->tap.count == 0;
}

// Press a screenshot shortcut, then Space to switch it to window capture
static void screenshot_window(uint16_t shortcut) {
    tap_code16(shortcut);
    wait_ms(100); // Give macOS time to show the screenshot cursor
    tap_code(KC_SPC);
}

bool process_long_press_keys(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // Window management (Rectangle)
        case WNDW_MAX_LP:
            return tap_or_hold(record, WNDW_MAX, WNDW_RSTR);
        case WNDW_CNTR_LP:
            return tap_or_hold(record, WNDW_CNTR, WNDW_LWP);
        case WNDW_LFT_LP:
            return tap_or_hold(record, WNDW_LFT_HLF, WNDW_LFT_FILL);
        case WNDW_RGHT_LP:
            return tap_or_hold(record, WNDW_RGHT_HLF, WNDW_RGHT_FILL);
        case WNDW_VRT_MAX_LP:
            return tap_or_hold(record, WNDW_ALMST_MAX, WNDW_VRT_MAX);

        // Spotlight: tap searches, hold searches in a new Finder window
        case OS_SPTLGHT_LP:
            return tap_or_hold(record, OS_SPTLGHT, OS_SPTLGHT_FNDR);

        // Screenshots: tap saves to file, hold copies to clipboard
        case OS_SCRN_SHT_LP:
            return tap_or_hold(record, OS_SCRN_SHT, OS_SCRN_SHT_CLP);
        case OS_SCRN_SHT_SLCT_LP:
            return tap_or_hold(record, OS_SCRN_SHT_SLCT, OS_SCRN_SHT_CLP_SLCT);
        case OS_SCRN_SHT_WNDW_LP:
            if (record->event.pressed) {
                screenshot_window(record->tap.count > 0 ? OS_SCRN_SHT_SLCT : OS_SCRN_SHT_CLP_SLCT);
            }
            return false;

        // Keyboard settings: hold-only, doing what DB_TOGG, EE_CLR, QK_BOOT and UG_TOGG do
        case DB_TOGG_LP:
            if (is_hold_press(record)) {
                debug_enable ^= 1;
            }
            return false;
        case EE_CLR_LP:
            if (is_hold_press(record)) {
                eeconfig_disable();
                soft_reset_keyboard();
            }
            return false;
        case QK_BOOT_LP:
            if (is_hold_press(record)) {
                reset_keyboard();
            }
            return false;
        case UG_TOGG_LP: // UG_TOGG can't be tapped from code, so toggle the lights directly
            if (is_hold_press(record)) {
#if defined(RGBLIGHT_ENABLE)
                rgblight_toggle();
#endif
#if defined(RGB_MATRIX_ENABLE)
                rgb_matrix_toggle();
#endif
            }
            return false;
        case TGL_LYT_LP: // Switch the default layer between Serenity and QWERTY (resets on unplug)
            if (is_hold_press(record)) {
                set_single_default_layer(get_highest_layer(default_layer_state) == BASE ? QWRTY : BASE);
            }
            return false;
    }
    return true;
}
