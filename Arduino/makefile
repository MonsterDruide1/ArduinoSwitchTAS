#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

# Set the MCU accordingly to your device (e.g. at90usb1286 for a Teensy 2.0++, or atmega16u2 for an Arduino UNO R3)
MCU          = atmega16u2
ARCH         = AVR8
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = Joystick
SRC          = $(TARGET).c Descriptors.c mainC.cpp $(LUFA_SRC_USB)
LUFA_PATH    = ./lufa/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/
LD_FLAGS     =

ARDUINO_PATH = /home/adrian/Schreibtisch/arduino-1.8.10-linux64/arduino-1.8.10/hardware/arduino/avr/cores/arduino
SRC         += $(ARDUINO_PATH)/HardwareSerial.cpp
SRC         += $(ARDUINO_PATH)/HardwareSerial1.cpp
SRC         += $(ARDUINO_PATH)/Print.cpp
SRC         += $(ARDUINO_PATH)/Stream.cpp
SRC         += $(ARDUINO_PATH)/WString.cpp
CXX_FLAGS   += -I$(ARDUINO_PATH) -I./HoodLoader2/avr/variants/HoodLoader2
CC_FLAGS    += -I$(ARDUINO_PATH) -I./HoodLoader2/avr/variants/HoodLoader2

# Default target
all:

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk

# Target for LED/buzzer to alert when print is done
with-alert: all
with-alert: CC_FLAGS += -DALERT_WHEN_DONE
