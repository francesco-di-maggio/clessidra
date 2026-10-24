# Project Name
TARGET = Clessidra

# Sources
CPP_SOURCES = Clessidra.cpp $(wildcard touch/*.cpp) $(wildcard grain/*.cpp) $(wildcard ui/*.cpp)

# Library Locations
LIBDAISY_DIR = lib/libDaisy
DAISYSP_DIR = lib/DaisySP

# ReverbSc
USE_DAISYSP_LGPL = 1

# Serial log over USB: make clean; make LOG=1
ifeq ($(LOG), 1)
C_DEFS += -DCLESSIDRA_LOG
endif

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

CPP_STANDARD = -std=gnu++17

# No OS loader on the Seed enforces segment permissions, so this binutils warning doesn't apply.
LDFLAGS += -Wl,--no-warn-rwx-segments

libs:
	$(MAKE) -C $(LIBDAISY_DIR)
	$(MAKE) -C $(DAISYSP_DIR)

.PHONY: libs

# Host tests, no board: the engine and the mapping
test:
	@sh tests/run.sh

.PHONY: test
