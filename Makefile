EMCC ?= emcc
CC   ?= gcc

SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build
ASSETS_DIR  := assets

# this var cannot be named "SHELL" because
# $(SHELL) in make has a special meaning
EMSHELL := $(SRC_DIR)/shell.html

MD4C_DIR    := deps/md4c
MD4C_SRCS   := $(MD4C_DIR)/src/md4c.c \
               $(MD4C_DIR)/src/md4c-html.c \
               $(MD4C_DIR)/src/entity.c

rwildcard = \
	$(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2))) \
	$(filter $(subst *,%,$(2)),$(wildcard $(1)/$(2)))

SRCS   := $(call rwildcard,src/,*.c) $(MD4C_SRCS)
WEB_TARGET := $(BUILD_DIR)/index.html

INCLUDE_PATHS := \
	-Iinclude \
	-Ideps/vector \
	-I$(MD4C_DIR)/src

EM_CFLAGS := -O2 -Wall \
		 $(INCLUDE_PATHS) \
         $(shell empack-config --cflags sdl3 sdl3-image 2>/dev/null)

EMFLAGS := \
		-sUSE_SDL=3 \
		-sUSE_SDL_TTF=3 \
        -sMAX_WEBGL_VERSION=2 \
        -sFULL_ES3=1 \
        -sALLOW_MEMORY_GROWTH=1 \
        -sASYNCIFY=0 \
		--shell-file $(EMSHELL) \
		--preload-file $(ASSETS_DIR)

NATIVE_CFLAGS := -Og -Wall -g $(INCLUDE_PATHS)
NATIVE_LDFLAGS := -lSDL3 -lSDL3_ttf
NATIVE_TARGET := $(BUILD_DIR)/app

.PHONY: all submodules clean serve web native run
all: web native
web: submodules $(WEB_TARGET)
native: submodules $(NATIVE_TARGET)

submodules:
	git submodule update --init --recursive

$(WEB_TARGET): $(SRCS) $(EMSHELL)
	@mkdir -p $(BUILD_DIR)
	$(EMCC) $(EM_CFLAGS) $(SRCS) $(EMFLAGS) -o $(WEB_TARGET)

$(NATIVE_TARGET): $(SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(SRCS) $(NATIVE_CFLAGS) $(NATIVE_LDFLAGS) -o $@

run: native
	$(NATIVE_TARGET)

serve: all
	python3 -m http.server 8080 --directory $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
