# users/serenity is a copy of serenity-qmk's, which uses community modules (QMK 0.28+).
# QMK 0.26 has no module system, so the modules are built here as plain sources and wired up by
# compat.h/.c. Keep this list in step with keymap.json in serenity-qmk.
SERENITY_MODULES = getreuer/custom_shift_keys wheredoesyourmindgo/stackable_oneshot getreuer/select_word getreuer/layer_lock
VPATH += $(addprefix modules/,$(SERENITY_MODULES)) modules/getreuer
SRC += $(addsuffix .c,$(notdir $(SERENITY_MODULES)))
SRC += compat.c
OPT_DEFS += -DQMK_COMPAT_H=\"compat.h\"

# users/serenity turns LTO on to save space on small boards. QMK warns it often fails on
# arm_atsam, and the Conundrum has room to spare
override LTO_ENABLE = no

# The Custom Shift Keys table needs the module's introspection code beside it. override keeps
# users/serenity/rules.mk, which is read after this file, from setting it back to shift_keys.c
override INTROSPECTION_KEYMAP_C = serenity_introspection.c
