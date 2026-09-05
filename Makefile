EMCC ?= emcc
CC   ?= gcc

SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build
ASSETS_DIR  := assets

# this var cannot be named "SHELL" because
# $(SHELL) in make has a special meaning
EMSHELL := $(SRC_DIR)/shell.html

rwildcard = \
	$(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2))) \
	$(filter $(subst *,%,$(2)),$(wildcard $(1)/$(2)))

SRCS   := $(call rwildcard,$(SRC_DIR),*.c)
ASSET_FILES := $(call rwildcard,$(ASSETS_DIR),*)

INCLUDE_PATHS := \
	-Iinclude \
	-Ideps/vector

WEB_CFLAGS := -O2 -Wall \
		 $(INCLUDE_PATHS) \
         $(shell empack-config --cflags sdl3 sdl3-image 2>/dev/null)

WEB_EMFLAGS := \
		-sUSE_SDL=3 \
		-sUSE_SDL_TTF=3 \
        -sMAX_WEBGL_VERSION=2 \
        -sFULL_ES3=1 \
        -sALLOW_MEMORY_GROWTH=1 \
        -sASYNCIFY=0 \
		--shell-file $(EMSHELL) \
		--preload-file $(ASSETS_DIR)

WEB_TARGET := $(BUILD_DIR)/index.html

NATIVE_CFLAGS := -Og -Wall -g $(INCLUDE_PATHS)
NATIVE_LDFLAGS := -lSDL3 -lSDL3_ttf -lm
NATIVE_TARGET := $(BUILD_DIR)/app

WEB_OBJS    := $(SRCS:%=$(BUILD_DIR)/web/%.o)
NATIVE_OBJS := $(SRCS:%=$(BUILD_DIR)/native/%.o)

.PHONY: all submodules clean serve web native run
all: web native
web: submodules $(WEB_TARGET)
native: submodules $(NATIVE_TARGET)

submodules:
	git submodule update --init --recursive

$(BUILD_DIR)/web/%.o: %
	@mkdir -p $(dir $@)
	$(EMCC) $(WEB_CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/native/%.o: %
	@mkdir -p $(dir $@)
	$(CC) $(NATIVE_CFLAGS) -MMD -MP -c -o $@ $<

$(WEB_TARGET): $(WEB_OBJS) $(ASSET_FILES) $(EMSHELL)
	@mkdir -p $(BUILD_DIR)
	$(EMCC) $(WEB_OBJS) $(WEB_EMFLAGS) -o $(WEB_TARGET)

$(NATIVE_TARGET): $(NATIVE_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(NATIVE_OBJS) $(NATIVE_LDFLAGS) -o $@

run: native
	$(NATIVE_TARGET)

serve: all
	python3 -m http.server 8080 --directory $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

-include $(WEB_OBJS:.o=.d) $(NATIVE_OBJS:.o=.d)
