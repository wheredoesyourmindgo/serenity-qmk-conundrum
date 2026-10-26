ARM_ATSAM_DIR = protocol/arm_atsam

SRC += $(ARM_ATSAM_DIR)/adc.c
SRC += $(ARM_ATSAM_DIR)/clks.c
SRC += $(ARM_ATSAM_DIR)/d51_util.c
SRC += $(ARM_ATSAM_DIR)/i2c_master.c
ifeq ($(RGB_MATRIX_DRIVER),custom)
  SRC += $(ARM_ATSAM_DIR)/md_rgb_matrix_programs.c
  SRC += $(ARM_ATSAM_DIR)/md_rgb_matrix.c
endif
SRC += $(ARM_ATSAM_DIR)/main_arm_atsam.c
# Boards without Massdrop's 595 shift register set NO_SR_EXP = yes
ifneq ($(strip $(NO_SR_EXP)), yes)
  SRC += $(ARM_ATSAM_DIR)/shift_register.c
  SRC += $(ARM_ATSAM_DIR)/spi_master.c
else
  OPT_DEFS += -DNO_SR_EXP
endif
SRC += $(ARM_ATSAM_DIR)/startup.c
SRC += $(ARM_ATSAM_DIR)/watchdog.c

SRC += $(ARM_ATSAM_DIR)/usb/main_usb.c
SRC += $(ARM_ATSAM_DIR)/usb/udc.c
SRC += $(ARM_ATSAM_DIR)/usb/udi_cdc.c
SRC += $(ARM_ATSAM_DIR)/usb/udi_hid.c
SRC += $(ARM_ATSAM_DIR)/usb/udi_hid_kbd.c
SRC += $(ARM_ATSAM_DIR)/usb/udi_hid_kbd_desc.c
SRC += $(ARM_ATSAM_DIR)/usb/ui.c
SRC += $(ARM_ATSAM_DIR)/usb/usb.c
SRC += $(ARM_ATSAM_DIR)/usb/usb_device_udd.c
SRC += $(ARM_ATSAM_DIR)/usb/usb_util.c

# Boards without Massdrop's USB2422 hub set NO_USB2422 = yes
ifneq ($(strip $(NO_USB2422)), yes)
  SRC += $(ARM_ATSAM_DIR)/usb/usb_hub.c
  SRC += $(DRIVER_PATH)/usb2422.c
else
  OPT_DEFS += -DNO_USB2422
endif

# Search Path
VPATH += $(TMK_DIR)/$(ARM_ATSAM_DIR)
