// users/serenity is written for QMK 0.28+. This header fills in what QMK 0.26 lacks: quantum.h
// includes it (rules.mk sets QMK_COMPAT_H), and compat.c calls the community modules' hooks.
//
// Community modules: stands in for the header QMK 0.28 generates from keymap.json
#pragma once

#define ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(...)

#define COMMUNITY_MODULE_CUSTOM_SHIFT_KEYS_ENABLE
#define COMMUNITY_MODULE_STACKABLE_ONESHOT_ENABLE
#define COMMUNITY_MODULE_SELECT_WORD_ENABLE
#define COMMUNITY_MODULE_LAYER_LOCK_ENABLE

// Module keycodes, in the keyboard-level range (thock/conundrum defines none of its own)
enum community_module_keycodes {
    // wheredoesyourmindgo/stackable_oneshot
    STACKABLE_ONESHOT_LCTL = QK_KB_0,
    STACKABLE_ONESHOT_LSFT,
    STACKABLE_ONESHOT_LALT,
    STACKABLE_ONESHOT_LGUI,
    STACKABLE_ONESHOT_RCTL,
    STACKABLE_ONESHOT_RSFT,
    STACKABLE_ONESHOT_RALT,
    STACKABLE_ONESHOT_RGUI,
    // getreuer/select_word
    SELECT_WORD,
    SELECT_WORD_BACK,
    SELECT_LINE,
    SELECT_LINE_UP,
    // getreuer/layer_lock (QMK core's Layer Lock arrived after 0.26)
    QK_LAYER_LOCK,
};

#define SOSM_LC STACKABLE_ONESHOT_LCTL
#define SOSM_LS STACKABLE_ONESHOT_LSFT
#define SOSM_LA STACKABLE_ONESHOT_LALT
#define SOSM_LG STACKABLE_ONESHOT_LGUI
#define SOSM_RC STACKABLE_ONESHOT_RCTL
#define SOSM_RS STACKABLE_ONESHOT_RSFT
#define SOSM_RA STACKABLE_ONESHOT_RALT
#define SOSM_RG STACKABLE_ONESHOT_RGUI

#define SELWORD SELECT_WORD
#define SELWBAK SELECT_WORD_BACK
#define SELLINE SELECT_LINE
#define SELLUP SELECT_LINE_UP

#define QK_LLCK QK_LAYER_LOCK

// Module hooks
bool process_record_custom_shift_keys(uint16_t keycode, keyrecord_t *record);
bool process_record_stackable_oneshot(uint16_t keycode, keyrecord_t *record);
bool process_record_stackable_oneshot_kb(uint16_t keycode, keyrecord_t *record);
bool process_record_select_word(uint16_t keycode, keyrecord_t *record);
bool process_record_select_word_kb(uint16_t keycode, keyrecord_t *record);
bool process_record_layer_lock(uint16_t keycode, keyrecord_t *record);
void housekeeping_task_stackable_oneshot(void);
void housekeeping_task_select_word(void);
void housekeeping_task_layer_lock(void);

// Core functions added after QMK 0.26

// Sets the default layer without saving it to EEPROM
static inline void set_single_default_layer(uint8_t default_layer) {
    default_layer_set((layer_state_t)1 << default_layer);
}
