# Project Name
TARGET = DelayTests

# Sources
CPP_SOURCES = DelayTests.cpp \
control.cpp \
engine.cpp \
fx/Grain.cpp \

C_INCLUDES += \
-I. \
-Ifx 

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
