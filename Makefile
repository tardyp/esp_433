APP_NAME := cp1101Tests
ARDUINO_LIBS := AUnit
ARDUINO := $(HOME)/Documents/Arduino
CPPFLAGS := -g
ARDUINO_LIB_DIRS := \
	$(ARDUINO)/libraries

include libraries/UnixHostDuino/UnixHostDuino.mk

