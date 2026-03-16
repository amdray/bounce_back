TARGET = bounce_back
OBJS = src/main.o src/resource_loader.o src/level_loader.o src/tile_metadata.o src/tile_animation.o src/tileset_loader.o src/bg_layer.o src/tile_transform.o src/level_renderer.o src/enemy_renderer.o src/player_masks.o src/player.o src/input.o src/camera.o src/exit_door.o src/foreground_pass.o src/hud.o src/hud_font.o src/font_atlas.o src/font9_atlas.o src/font12_atlas.o src/font23_atlas.o src/font24.o src/menu.o src/sound.o src/save.o src/game_assets.o src/texture_loader.o

# ---- Release folder setup ----
RELEASE_DIR = release
ORIGINAL_RES_DIR ?= original_code/bounce_back_s60.jar.src/res

# SDL2 libraries for PSP (порядок важен!)
LIBS = -lSDL2main -lSDL2_image -lSDL2 -ljpeg -lpng -lz -lGL -lpspvram -lpspgu -lpspge -lpspaudio -lpsppower -lpsphprm -lpspirkeyb -lm -lpspvfpu

# PSP-specific settings
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

# Use large memory model (32MB)
PSP_LARGE_MEMORY = 1

# EBOOT metadata
PSP_EBOOT = $(RELEASE_DIR)/EBOOT.PBP
PSP_EBOOT_SFO = $(RELEASE_DIR)/PARAM.SFO
EXTRA_TARGETS = $(PSP_EBOOT)
PSP_EBOOT_TITLE = Bounce Back
SFOFLAGS += -d DISC_ID=BBACK0001

# Include PSPSDK build rules
PSPDEV ?= $(HOME)/pspdev
override PSPSDK := $(PSPDEV)/psp/sdk
export PATH := $(PSPDEV)/bin:$(PATH)
include $(PSPSDK)/lib/build.mak

$(RELEASE_DIR):
	@mkdir -p $@

$(PSP_EBOOT_SFO): | $(RELEASE_DIR)

$(PSP_EBOOT): | $(RELEASE_DIR)

.PHONY: default
default: $(EXTRA_TARGETS)
	@echo "Creating release folder..."
	@mkdir -p $(RELEASE_DIR)
	@if [ ! -d "$(ORIGINAL_RES_DIR)" ]; then \
		echo "ERROR: missing '$(ORIGINAL_RES_DIR)' (original resources)."; \
		echo "Put original resources under original_code and re-run make, or override ORIGINAL_RES_DIR."; \
		exit 1; \
	fi
	@rsync -ru --size-only "$(ORIGINAL_RES_DIR)/" "$(RELEASE_DIR)/res/"
	@rm -f bounce_back.elf
	@echo "Done: release/EBOOT.PBP + release/res/"

.DEFAULT_GOAL := default
