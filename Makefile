EMCC ?= emcc

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

SRCS   := $(call rwildcard,src/,*.c)
TARGET := $(BUILD_DIR)/index.html

INCLUDE_PATHS := \
	-Iinclude

CFLAGS := -O2 -Wall \
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

all: $(TARGET)

$(TARGET): $(SRCS) $(EMSHELL)
	@mkdir -p $(BUILD_DIR)
	$(EMCC) $(CFLAGS) $(SRCS) $(LDFLAGS) $(EMFLAGS) -o $(TARGET)

serve: all
	python3 -m http.server 8080 --directory $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean serve
