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

# The web page: the wasm, then the site in web/dist. web-dev serves it at
# http://localhost:4321/clessidra/ and reloads on changes
wasm:
	@sh web/build.sh

web: wasm
	npm --prefix web run build

web-dev: wasm
	npm --prefix web run dev

# The manual: the manual page of the built site, printed to
# web/public/clessidra-manual.pdf. Run after changing the controls, then make web.
manual: web
	@node web/manual.mjs

.PHONY: test wasm web web-dev manual
