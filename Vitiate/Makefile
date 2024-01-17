# Project Name
TARGET = Vitiate

# Sources
CPP_SOURCES = Vitiate.cpp \
resources.cpp \
dsp/filter_bank.cpp \
dsp/modulator.cpp \
dsp/oscillator.cpp \
dsp/vocoder.cpp \

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/
STMLIB_DIR = ../../stmlib/

C_INCLUDES += \
-I. \
-Idsp \
-I../../stmlib/dsp \
-I../../stmlib/utils \
-I../../stmlib/system \

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
