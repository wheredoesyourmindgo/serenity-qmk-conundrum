// Small helpers shared by the userspace features.
#include "serenity.h"

// The bottom-right arrow cluster: Left (Hardware on hold) and three right-hand mod-taps
bool is_arrow_cluster_key(uint16_t keycode) {
    switch (keycode) {
        case LT(HRDWR, KC_LEFT):
        case RGUI_T(KC_DOWN):
        case RALT_T(KC_UP):
        case RCTL_T(KC_RIGHT):
            return true;
        default:
            return false;
    }
}

// True when both Shift and GUI are held or one-shot
bool check_mods_sft_and_gui(void) {
    uint8_t mods         = get_mods();
    uint8_t oneshot_mods = get_oneshot_mods();

    bool is_shift_active = (mods & MOD_MASK_SHIFT) || (oneshot_mods & MOD_MASK_SHIFT);
    bool is_gui_active   = (mods & MOD_MASK_GUI) || (oneshot_mods & MOD_MASK_GUI);

    return is_shift_active && is_gui_active;
}

// Tap `code` with all modifiers temporarily released, then restore them
void tap_code16_no_mod(uint16_t code) {
    uint8_t mod_state   = get_mods();
    uint8_t w_mod_state = get_weak_mods();
    uint8_t o_mod_state = get_oneshot_mods();
    if (mod_state || w_mod_state || o_mod_state) {
        del_mods(mod_state);
        del_weak_mods(w_mod_state);
        del_oneshot_mods(o_mod_state);
        send_keyboard_report();
        tap_code16(code);
        // Restore so held modifiers keep working after the tap
        add_mods(mod_state);
        add_weak_mods(w_mod_state);
        set_oneshot_mods(o_mod_state);
        send_keyboard_report();
    } else {
        tap_code16(code);
    }
}

void tap_code_no_mod(uint8_t code) {
    tap_code16_no_mod(code);
}

// Tap `code` with all modifiers released, leaving them released
void tap_code16_unset_mod(uint16_t code) {
    uint8_t mod_state   = get_mods();
    uint8_t w_mod_state = get_weak_mods();
    uint8_t o_mod_state = get_oneshot_mods();
    if (mod_state || w_mod_state || o_mod_state) {
        del_mods(mod_state);
        del_weak_mods(w_mod_state);
        del_oneshot_mods(o_mod_state);
        send_keyboard_report();
    }
    tap_code16(code);
}

void tap_code_unset_mod(uint8_t code) {
    tap_code16_unset_mod(code);
}
