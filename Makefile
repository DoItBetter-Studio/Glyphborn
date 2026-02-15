# ==========================================================
# 🧱 Glyphborn Master Makefile
# Comprehensive cross-platform build system
# ----------------------------------------------------------
# Targets:
#   🐧 Linux  (native GCC)
#   🪟 Windows x86 / x64 (MinGW cross-compile)
#
# Supported Distros:
#   - Vanilla  (standalone)
#   - Steam    (Steamworks SDK)
#   - GOG      (future integration)
#
# Features:
#   ✅ Multi-platform / multi-distro matrix
#   ✅ Version/revision embedding via compiler flags
#   ✅ Source-hash-based skip system (no unnecessary rebuilds)
#   ✅ Revision increments only when source changed
#   🧼 Stripping + SHA256 checksums
#   🧠 WSL / MSYS environment protection
#   🐞 Debug + Verbose toggles
# ==========================================================


# ==========================================================
# 🎨 Colors
# ==========================================================
GREEN  := \033[1;32m
BLUE   := \033[1;34m
YELLOW := \033[1;33m
RED    := \033[1;31m
GRAY   := \033[0;37m
BOLD   := \033[1m
RESET  := \033[0m


# ==========================================================
# 🧠 Environment Sanity Check
# ==========================================================
ifeq ($(OS),Windows_NT)
  UNAME_S := $(shell uname -s 2>/dev/null)
  ifneq (,$(findstring MINGW,$(UNAME_S)))
  else ifneq (,$(findstring MSYS,$(UNAME_S)))
  else ifneq (,$(findstring Linux,$(UNAME_S)))
  else
    $(error ❌ ${RED}ERROR:${RESET} Run under WSL, MSYS2, or Linux — not CMD/PowerShell.)
  endif
endif


# Skip versioning for non-build targets
ifeq ($(MAKECMDGOALS),clean)
    SKIP_VERSIONING := true
endif
ifeq ($(MAKECMDGOALS),distclean)
    SKIP_VERSIONING := true
endif
ifeq ($(MAKECMDGOALS),buildver)
    SKIP_VERSIONING := true
endif

ifeq ($(SKIP_VERSIONING),)
FORCE ?= false

VERSION_META := $(shell tools/build/version.sh $(FORCE))

ifeq ($(VERSION_META),SKIP)
	$(info 🔁 No changes detected — skipping build)
	@exit 0
endif

include $(VERSION_META)

# Now that FULL_VERSION exists, define build paths
BUILD_BASE := build/$(FULL_VERSION)
BUILD_TIME := $(shell date +"%Y-%m-%d %H:%M:%S")
endif

# ==========================================================
# 🔧 Compiler Version Flags
# ==========================================================
CFLAGS_VERSION = \
    -DGB_VERSION=\"$(VERSION)\" \
    -DGB_REVISION=\"$(BUILD_REV)\" \
    -DGB_FULL_VERSION=\"$(FULL_VERSION)\"


# ==========================================================
# 🎯 Build Targets / Toolchains
# ==========================================================
DISTROS := Vanilla Steam GOG

CC_LINUX  = /usr/bin/gcc
CC_WIN64  = /usr/bin/x86_64-w64-mingw32-gcc
CC_WIN32  = /usr/bin/i686-w64-mingw32-gcc

STRIP_LINUX = /usr/bin/strip
STRIP_WIN64 = /usr/bin/x86_64-w64-mingw32-strip
STRIP_WIN32 = /usr/bin/i686-w64-mingw32-strip


# ==========================================================
# ⚙️ Verbose / Debug Mode
# ==========================================================
VERBOSE ?= true
DEBUG   ?= false

ifneq (,$(filter $(VERBOSE),true))
  WARNFLAGS := -Wall -Wextra
  VERBOSE_MSG := echo "🔍 ${BLUE}Verbose mode enabled (showing compiler warnings)${RESET}"
else
  WARNFLAGS :=
  VERBOSE_MSG := echo "🤫 ${GRAY}Verbose mode disabled (quiet build)${RESET}"
endif

ifneq (,$(filter $(DEBUG),true))
  CFLAGS_DEBUG := -g -DDEBUG
  SUBSYSTEM := console
  DEBUG_MSG := echo "🐞 ${YELLOW}Debug mode enabled (symbols + console window)${RESET}"
else
  CFLAGS_DEBUG :=
  CFLAGS_BASE += -O2
  SUBSYSTEM := windows
  DEBUG_MSG := echo "🚀 ${GREEN}Release mode (optimized)${RESET}"
endif


# ==========================================================
# 🔧 Base Compiler / Linker Flags
# ==========================================================
CFLAGS_BASE = -std=c17 $(WARNFLAGS) -Iincludes -Iresources -MMD -MP

CFLAGS_LIN  = $(CFLAGS_BASE) -I/c/linux/include
LDFLAGS_LIN = -L/c/linux/lib -lX11 -lXext -lXrandr -lXrender -lasound -lm
LDFLAGS_WIN = -luser32 -lgdi32 -ldsound -lkernel32 -lwinmm -lxinput -lm


# ==========================================================
# 🎮 Steamworks SDK
# ==========================================================
STEAM_SDK_PATH        := externals/steamworks
STEAM_SDK_INCLUDE     := $(STEAM_SDK_PATH)/public
STEAM_SDK_LIB_LINUX   := $(STEAM_SDK_PATH)/redistributable_bin/linux64
STEAM_SDK_LIB_WIN64   := $(STEAM_SDK_PATH)/redistributable_bin/win64
STEAM_SDK_LIB_WIN32   := $(STEAM_SDK_PATH)/redistributable_bin


# ==========================================================
# 🧩 Per-Distro Flag Injection
# ==========================================================
define set_distro_flags
  ifeq ($1,Steam)
    CFLAGS_DISTRO = -DDISTRO_STEAM -I$(STEAM_SDK_INCLUDE)
    ifeq ($2,Linux)
      LDFLAGS_DISTRO = -L$(STEAM_SDK_LIB_LINUX) -lsteam_api
    else ifeq ($2,Win64)
      LDFLAGS_DISTRO = -L$(STEAM_SDK_LIB_WIN64) -lsteam_api64
    else ifeq ($2,Win32)
      LDFLAGS_DISTRO = -L$(STEAM_SDK_LIB_WIN32) -lsteam_api
    endif
  else ifeq ($1,GOG)
    CFLAGS_DISTRO = -DDISTRO_GOG -Iexternals/gog
    LDFLAGS_DISTRO =
  else
    CFLAGS_DISTRO = -DDISTRO_VANILLA
    LDFLAGS_DISTRO =
  endif
endef


# ==========================================================
# 🧠 World Data Code Generation
# ==========================================================

GENERATED_SOURCE 	:= source/generated
GENERATED_HEADERS 	:= includes/generated
GENERATED_FILES 	:= \
    $(GENERATED_SOURCE)/Geometry.c \
    $(GENERATED_HEADERS)/Geometry.h \
    $(GENERATED_SOURCE)/Collision.c \
    $(GENERATED_HEADERS)/Collision.h \
    $(GENERATED_SOURCE)/Tileset_Regional.c \
    $(GENERATED_HEADERS)/Tileset_Regional.h \
    $(GENERATED_SOURCE)/Tileset_Local.c \
    $(GENERATED_HEADERS)/Tileset_Local.h \
    $(GENERATED_SOURCE)/Tileset_Interior.c \
    $(GENERATED_HEADERS)/Tileset_Interior.h

REGISTRY_JSON := $(shell find data/registry -name '*.json')


# ==========================================================
# 📁 Source Discovery
# ==========================================================
SRC_ALL     	:= $(shell find source -name '*.c')
SRC_LINUX   	:= $(filter-out %_windows.c,$(SRC_ALL))
SRC_WINDOWS 	:= $(filter-out %_linux.c,$(SRC_ALL))

OBJDIR_LINUX 	:= obj/linux
OBJDIR_WIN32 	:= obj/win32
OBJDIR_WIN64 	:= obj/win64

OBJ_LINUX		:= $(patsubst source/%.c,$(OBJDIR_LINUX)/%.o,$(SRC_LINUX))
OBJ_WIN32		:= $(patsubst source/%.c,$(OBJDIR_WIN32)/%.o,$(SRC_WINDOWS))
OBJ_WIN64		:= $(patsubst source/%.c,$(OBJDIR_WIN64)/%.o,$(SRC_WINDOWS))

DEP_LINUX 		:= $(OBJ_LINUX:.o=.d)
DEP_WIN32 		:= $(OBJ_WIN32:.o=.d)
DEP_WIN64 		:= $(OBJ_WIN64:.o=.d)


# ==========================================================
# 📁 Data Discovery
# ==========================================================

DATA_EXTRA		:= $(shell find data -name '*.mtx' -o -name '*.hdr')
DATA_BIN		:= $(shell find data -name '*.bin')

DATA_ALL		:= $(DATA_BIN) $(DATA_EXTRA)

OBJ_DATA_LINUX	:= $(patsubst data/%, obj/data/linux/%.o, $(DATA_ALL))
OBJ_DATA_WIN32	:= $(patsubst data/%, obj/data/win32/%.o, $(DATA_ALL))
OBJ_DATA_WIN64	:= $(patsubst data/%, obj/data/win64/%.o, $(DATA_ALL))


# ==========================================================
# 🌍 Full Target Matrix
# ==========================================================
ALL_TARGETS := $(foreach D,$(DISTROS), \
	$(BUILD_BASE)/$(D)/linux/glyphborn_linux \
	$(BUILD_BASE)/$(D)/win32/glyphborn_win32.exe \
	$(BUILD_BASE)/$(D)/win64/glyphborn_win64.exe)


# ==========================================================
# 🏗 Default Rule
# ==========================================================
all: banner
	@$(VERBOSE_MSG)
	@$(DEBUG_MSG)
	@$(MAKE) $(ALL_TARGETS)
	@$(MAKE) --no-print-directory checksums
	@$(MAKE) --no-print-directory postbuild
	@echo ""
	@echo "────────────────────────────────────────────"
	@echo "✅ ${GREEN}All builds complete for Glyphborn $(FULL_VERSION)${RESET}"
	@echo "📦 Output: ${BLUE}$(BUILD_BASE)${RESET}"
	@echo "────────────────────────────────────────────"


# ==========================================================
# 🎬 Build Banner
# ==========================================================
banner:
	@echo ""
	@echo "${BOLD}===========================================${RESET}"
	@echo "🚀 ${BLUE}Building Glyphborn v$(FULL_VERSION)${RESET}"
	@echo "🕒 Build Time: ${YELLOW}$(BUILD_TIME)${RESET}"
	@echo "${BOLD}===========================================${RESET}"
	@echo ""

# ==========================================================
# 🐧 Linux Build
# ==========================================================
$(BUILD_BASE)/%/linux/glyphborn_linux: $(OBJ_LINUX) $(OBJ_DATA_LINUX)
	@echo "🟩 ${GREEN}[Linux/$*] Linking...${RESET}"
	@mkdir -p $(dir $@)
	$(eval $(call set_distro_flags,$*,Linux))
	$(CC_LINUX) $(CFLAGS_LIN) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) $(CFLAGS_DISTRO) \
		$(OBJ_LINUX) $(OBJ_DATA_LINUX) -o $@ $(LDFLAGS_LIN) $(LDFLAGS_DISTRO)
	$(STRIP_LINUX) --strip-unneeded $@
	@echo "   ${GREEN}✔ Built → $@${RESET}"


# ==========================================================
# 🪟 Win32 Build
# ==========================================================
$(BUILD_BASE)/%/win32/glyphborn_win32.exe: $(OBJ_WIN32)  $(OBJ_DATA_WIN32)
	@echo "🟨 ${YELLOW}[Win32/$*] Linking...${RESET}"
	@mkdir -p $(dir $@)
	$(eval $(call set_distro_flags,$*,Win32))
	$(CC_WIN32) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) $(CFLAGS_DISTRO) \
		-D_WIN32 -Wl,-subsystem,$(SUBSYSTEM) \
		$(OBJ_WIN32) $(OBJ_DATA_WIN32) -o $@ $(LDFLAGS_WIN) $(LDFLAGS_DISTRO)
	$(STRIP_WIN32) --strip-unneeded $@
	@echo "   ${GREEN}✔ Built → $@${RESET}"


# ==========================================================
# 🪟 Win64 Build
# ==========================================================
$(BUILD_BASE)/%/win64/glyphborn_win64.exe: $(OBJ_WIN64) $(OBJ_DATA_WIN64)
	@echo "🟦 ${BLUE}[Win64/$*] Linking...${RESET}"
	@mkdir -p $(dir $@)
	$(eval $(call set_distro_flags,$*,Win64))
	$(CC_WIN64) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) $(CFLAGS_DISTRO) \
		-D_WIN32 -D_WIN64 -Wl,-subsystem,$(SUBSYSTEM) \
		$(OBJ_WIN64) $(OBJ_DATA_WIN64) -o $@ $(LDFLAGS_WIN) $(LDFLAGS_DISTRO)
	$(STRIP_WIN64) --strip-unneeded $@
	@echo "   ${GREEN}✔ Built → $@${RESET}"



$(GENERATED_FILES): tools/build/embed_data.py $(REGISTRY_JSON)
	@echo "🧠 Generating world data C files..."
	@python3 tools/build/embed_data.py

# ==========================================================
# 🧱 Compilation Rules
# ==========================================================
$(OBJDIR_LINUX)/%.o: source/%.c $(OBJ_DATA_LINUX) $(GENERATED_FILES)
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Linux): %s${RESET}\n" $<
	@$(CC_LINUX) $(CFLAGS_LIN) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) -c $< -o $@

$(OBJDIR_WIN32)/%.o: source/%.c $(OBJ_DATA_WIN32) $(GENERATED_FILES)
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Win32): %s${RESET}\n" $<
	@$(CC_WIN32) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) -D_WIN32 -c $< -o $@

$(OBJDIR_WIN64)/%.o: source/%.c $(OBJ_DATA_WIN64) $(GENERATED_FILES)
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Win64): %s${RESET}\n" $<
	@$(CC_WIN64) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) -D_WIN32 -D_WIN64 -c $< -o $@

obj/data/linux/%.o: data/%
	@mkdir -p $(dir $@)
	@printf "📦 ${GRAY}Embedding binary: %s${RESET}\n" $<
	@ld -r -b binary $< -o $@

obj/data/win32/%.o: data/%
	@mkdir -p $(dir $@)
	@printf "📦 Embedding binary (Win32): %s\n" $<
	@i686-w64-mingw32-ld -r -b binary $< -o $@

obj/data/win64/%.o: data/%
	@mkdir -p $(dir $@)
	@printf "📦 Embedding binary (Win64): %s\n" $<
	@x86_64-w64-mingw32-ld -r -b binary $< -o $@


# ==========================================================
# 🔒 SHA256 Checksums
# ==========================================================
checksums:
	@echo "🔑 ${BLUE}Generating SHA256 checksums for $(BUILD_BASE)${RESET}"
	@mkdir -p $(BUILD_BASE)
	@find $(BUILD_BASE) -type f -print0 | sort -z | xargs -0 sha256sum | tee $(BUILD_BASE)/checksums.txt >/dev/null
	@echo "📄 ${YELLOW}Checksums saved to: $(BUILD_BASE)/checksums.txt${RESET}"


# ==========================================================
# 🧹 Cleaning
# ==========================================================
clean:
	rm -rf obj
	@echo "🧽 ${YELLOW}Cleaned all object files.${RESET}"

distclean: clean
	rm -rf build
	@echo "🧽 ${YELLOW}Removed metadata and all builds.${RESET}"


# ==========================================================
# ℹ️ Build Version Info
# ==========================================================
buildver:
	@echo ""
	@echo "────────────────────────────────────────────"
	@echo "📦 ${BLUE}Glyphborn Build Version Info${RESET}"
	@echo "────────────────────────────────────────────"
	@tools/build/printver.sh
	@echo "────────────────────────────────────────────"
	@echo ""


postbuild:
	@echo "$(VERSION)" > tools/build/state/last_version
	@echo "$(CURRENT_HASH)" > tools/build/state/last_source_hash
	@echo "$(REVISION)" > tools/build/state/build_revision


# ==========================================================
# 📎 Include Dependency Files
# ==========================================================
-include $(DEP_LINUX) $(DEP_WIN32) $(DEP_WIN64)
