// Per-key tap-hold settings (enabled by the *_PER_KEY options in config.h).
#include "serenity.h"

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_LSFT: // Longer window for double-tap Shift to turn on Caps Word
            return 300;
        case DB_TOGG_LP: // Hold-only keyboard settings: a little under a second, so a brush can't fire them
        case EE_CLR_LP:
        case QK_BOOT_LP:
        case UG_TOGG_LP:
        case TGL_LYT_LP:
            return 800;
        default:
            return TAPPING_TERM;
    }
}

// Right Shift and the arrow-cluster mod-taps switch to hold as soon as another key is
// tapped (permissive hold) or pressed (hold on other key press), so they can be used quickly.
static bool is_quick_hold_key(uint16_t keycode) {
    return keycode == RSFT_T(KC_ENT) || is_arrow_cluster_key(keycode);
}

// Arrow cluster: pressing another arrow while one is still undecided doesn't make it a hold.
// What comes next decides: releasing an arrow types them all as arrows, while pressing any
// other key (or holding past the tapping term) makes them modifiers and the Hardware layer.
bool get_chordal_hold(uint16_t tap_hold_keycode, keyrecord_t *tap_hold_record, uint16_t other_keycode, keyrecord_t *other_record) {
    return !(is_arrow_cluster_key(tap_hold_keycode) && is_arrow_cluster_key(other_keycode));
}

bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
    return is_quick_hold_key(keycode);
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    return is_quick_hold_key(keycode);
}

uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // Allow tap-then-hold auto-repeat on the arrow keys and Escape
        case LT(HRDWR, KC_LEFT):
        case RGUI_T(KC_DOWN):
        case RALT_T(KC_UP):
        case RCTL_T(KC_RIGHT):
        case LT(MOUSE, KC_ESC):
            return QUICK_TAP_TERM - 20;
        // Everything else: tap-then-hold activates the hold function instead of auto-repeat
        default:
            return 0;
    }
}

bool get_retro_tapping(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LT(MOUSE, KC_ESC): // Holding and releasing with no other key still sends Escape
            return true;
        default:
            return false;
    }
}
