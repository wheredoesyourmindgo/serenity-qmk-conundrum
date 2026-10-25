// Calls the community modules' hooks, as QMK 0.28's module system would (see compat.h).
// Modules run before the user hooks, in keymap.json order from serenity-qmk, and Layer Lock runs
// after them, where QMK 0.28 runs its core Layer Lock.
#include "quantum.h"
#include "custom_shift_keys.h"
#include "stackable_oneshot.h"
#include "select_word.h"
#include "layer_lock.h"

__attribute__((weak)) bool process_record_stackable_oneshot_kb(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__((weak)) bool process_record_select_word_kb(uint16_t keycode, keyrecord_t *record) {
    return true;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_custom_shift_keys(keycode, record)) {
        return false;
    }
    if (!process_record_stackable_oneshot(keycode, record)) {
        return false;
    }
    if (!process_record_select_word(keycode, record)) {
        return false;
    }
    if (!process_record_user(keycode, record)) {
        return false;
    }
    return process_record_layer_lock(keycode, record);
}

void housekeeping_task_kb(void) {
#if STACKABLE_ONESHOT_TIMEOUT > 0
    housekeeping_task_stackable_oneshot();
#endif
    housekeeping_task_select_word();
    housekeeping_task_layer_lock();
    housekeeping_task_user();
}
