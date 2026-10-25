// Rotary encoder behavior (Ristretto). Turn direction plus held mods/layer pick the action.
#include "serenity.h"
#include "features/cmd_tab_switcher.h"

#ifdef ENCODER_ENABLE
__attribute__((weak)) bool encoder_update_keymap(uint8_t index, bool clockwise) {
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (!encoder_update_keymap(index, clockwise)) {
        return false;
    }
    if (index != 0) {
        return false;
    }

    bool hrdwr = IS_LAYER_ON(HRDWR);
    bool actn  = IS_LAYER_ON(ACTN);

    if (MODS_GUI) {
        // Use the cmd-tab switcher so double tap can close apps only while it's active
        clockwise ? cmd_tab_next() : cmd_tab_previous();
    } else if (MODS_SFT && !hrdwr && !actn) {
        tap_code16_no_mod(clockwise ? DISP_BRI : DISP_DIM);
    } else if (MODS_CTRL) {
        tap_code16_no_mod(clockwise ? ZOOM_IN : ZOOM_OUT);
    } else if (MODS_ALT) {
        tap_code16_no_mod(clockwise ? ZOOM_IN_APP : ZOOM_OUT_APP);
    } else if (actn) {
        if (MODS_SFT) {
            tap_code16_no_mod(clockwise ? REDO : UNDO);
        } else {
            tap_code16(clockwise ? OS_NXT_TAB : OS_PRV_TAB);
        }
    } else if (hrdwr) {
        if (MODS_SFT) {
            tap_code_no_mod(clockwise ? KC_RIGHT : KC_LEFT);
        } else {
            tap_code(clockwise ? KC_DOWN : KC_UP);
        }
    } else {
        tap_code16(clockwise ? KC_VOLU : KC_VOLD);
    }
    return false;
}
#endif
