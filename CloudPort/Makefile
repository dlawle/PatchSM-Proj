# Project Name
TARGET = CloudPort

# Sources
CPP_SOURCES = CloudPort.cpp \
cloudseed_app.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/
CLOUDSEED_DIR = ../../CloudSeed

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

C_INCLUDES += \
-I../../CloudSeed \

LIBS += -lcloudseed
LIBDIR += -L../../CloudSeed/build