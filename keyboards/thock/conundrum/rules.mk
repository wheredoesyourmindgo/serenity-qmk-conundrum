# project specific files
SRC += matrix.c

# Thock's capacitive key sensing (prebuilt, no source: Thock's ec_touch plus
# Microchip's QTouch library). It is built with the soft-float calling
# convention, so the whole firmware must match.
COMMON_VPATH += $(LIB_PATH)/ec_touch/include
LDFLAGS += -L$(LIB_PATH)/ec_touch/lib/ -lec_touch_4x12
ARM_ATSAM_FLOAT_ABI = softfp

#For platform and packs
ARM_ATSAM = SAMD51G18A
MCU = cortex-m4

# Bootloader selection
BOOTLOADER = uf2boot
FIRMWARE_FORMAT = uf2

# No Massdrop USB hub or shift register; no crystal (runs from the DFLL the
# bootloader set up); keys are sensed with the PTC
NO_USB2422 = yes
NO_SR_EXP = yes
OPT_DEFS += -DARM_ATSAM_DFLL_ONLY -DARM_ATSAM_PTC_ENABLE
