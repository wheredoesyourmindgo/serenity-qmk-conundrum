#include "symbol_rolls.h"

bool process_symbol_rolls(uint16_t keycode, keyrecord_t* record, uint8_t sym_layer) {
    if (!record->event.pressed) { return true; }
    if (!IS_LAYER_ON(sym_layer)) { return true; }

    // Tap shifted symbols (eg. ~ { ?) on press instead of holding them until release. Otherwise,
    // rolling into the next key (eg. ~ then /) releases Shift and presses that key in the same
    // report, and macOS can apply the old Shift to it (typing ? instead of /).
    if (IS_QK_MODS(keycode)) {
        tap_code16(keycode);
        // Skipping QMK's handling also skips using up a one-shot mod, so do it here. Held mods stay held.
        if (get_oneshot_mods()) {
            clear_oneshot_mods();
            send_keyboard_report();
        }
        return false;
    }

    return true;
}
