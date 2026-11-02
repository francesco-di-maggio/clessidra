# Project Name
TARGET = Clessidra

# Sources
CPP_SOURCES = Clessidra.cpp $(wildcard src/hw/*.cpp) $(wildcard src/core/*.cpp) $(wildcard src/ui/*.cpp)
C_INCLUDES += -Isrc

# Library Locations
LIBDAISY_DIR = lib/libDaisy
DAISYSP_DIR = lib/DaisySP

# ReverbSc
USE_DAISYSP_LGPL = 1

# Serial log over USB: make clean; make DEBUG=1

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

# Host tests, no board: the core, the mapping and the web build
test:
	@sh tests/run.sh

# The web page in build/web, served at http://localhost:8000
web:
	@sh web/build.sh

serve: web
	python3 -m http.server 8000 -d build/web

.PHONY: test web serve
