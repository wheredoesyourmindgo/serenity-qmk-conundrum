// Custom Shift Keys table for the getreuer/custom_shift_keys community module (options in config.h).
//
// The module reads this table from the keymap, so rules.mk passes this file to QMK via
// INTROSPECTION_KEYMAP_C, which compiles it alongside each keymap.c. Don't add it to SRC.
#include "serenity.h"

_Static_assert(CUSTOM_SHIFT_KEYS_NEGMODS == MOD_MASK_GUI, "CUSTOM_SHIFT_KEYS_NEGMODS in config.h must equal MOD_MASK_GUI");

const custom_shift_key_t custom_shift_keys[] = {
    {KC_BACKSPACE, KC_DELETE}, // Shift Backspace is delete forward
    {NUM_DOT, KC_COMM},        // Num/Nav: Shift dot is a thousands separator, never >
};
