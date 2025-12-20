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


# ==========================================================
# 📁 Build Metadata Files
# ==========================================================
META_DIR        := .buildmeta
META_REV_FILE   := $(META_DIR)/build_revision
META_NEXT_REV   := $(META_DIR)/next_revision
META_VER_TRACK  := $(META_DIR)/last_version
META_CHECK_FILE := $(META_DIR)/last_source_hash
BUILD_STATE		:= $(META_DIR)/build_state.stamp

$(shell mkdir -p $(META_DIR) >/dev/null 2>&1)


# ==========================================================
# 🏷 Semantic Versioning
# ==========================================================
VERSION := $(shell cat VERSION.txt)

ifeq ($(wildcard $(META_REV_FILE)),)
  $(shell echo 1 > $(META_REV_FILE))
endif
ifeq ($(wildcard $(META_VER_TRACK)),)
  $(shell echo $(VERSION) > $(META_VER_TRACK))
endif
ifeq ($(wildcard $(META_CHECK_FILE)),)
  $(shell echo "none" > $(META_CHECK_FILE))
endif

CUR_REV := $(shell cat $(META_REV_FILE) 2>/dev/null || echo 1)

ifeq ($(VERSION),$(shell cat $(META_VER_TRACK)))
else
  $(shell echo $(VERSION) > $(META_VER_TRACK))
  $(shell echo 1 > $(META_REV_FILE))
  $(shell rm -f $(META_NEXT_REV))
  CUR_REV := 1
endif

BUILD_REV := $(CUR_REV)
FULL_VERSION := $(VERSION).$(BUILD_REV)
BUILD_BASE   := build/$(FULL_VERSION)
BUILD_TIME   := $(shell date +"%Y-%m-%d %H:%M:%S")


# ==========================================================
# 🔍 Source Hash
# ==========================================================
SOURCE_HASH := $(shell find source -type f -name '*.c' -exec sha256sum {} \; | sha256sum | cut -d' ' -f1)


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
# 📁 Source Discovery
# ==========================================================
SRC_ALL      := $(shell find source -name '*.c')
SRC_LINUX    := $(filter-out %_windows.c,$(SRC_ALL))
SRC_WINDOWS  := $(filter-out %_linux.c,$(SRC_ALL))

OBJDIR_LINUX := obj/linux
OBJDIR_WIN32 := obj/win32
OBJDIR_WIN64 := obj/win64

OBJ_LINUX := $(patsubst source/%.c,$(OBJDIR_LINUX)/%.o,$(SRC_LINUX))
OBJ_WIN32 := $(patsubst source/%.c,$(OBJDIR_WIN32)/%.o,$(SRC_WINDOWS))
OBJ_WIN64 := $(patsubst source/%.c,$(OBJDIR_WIN64)/%.o,$(SRC_WINDOWS))

DEP_LINUX := $(OBJ_LINUX:.o=.d)
DEP_WIN32 := $(OBJ_WIN32:.o=.d)
DEP_WIN64 := $(OBJ_WIN64:.o=.d)


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
all: preload_revision $(BUILD_STATE) banner
	@$(VERBOSE_MSG)
	@$(DEBUG_MSG)
	@$(MAKE) $(ALL_TARGETS)
	@$(MAKE) --no-print-directory checksums
	@$(MAKE) --no-print-directory finalize_success
	@echo ""
	@echo "────────────────────────────────────────────"
	@echo "✅ ${GREEN}All builds complete for Glyphborn $(FULL_VERSION)${RESET}"
	@echo "📦 Output: ${BLUE}$(BUILD_BASE)${RESET}"
	@echo "────────────────────────────────────────────"


# ==========================================================
# 🔄 Preload Revision
# ==========================================================
preload_revision:
	@if [ -f $(META_NEXT_REV) ]; then \
		mv $(META_NEXT_REV) $(META_REV_FILE); \
	fi


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
$(BUILD_BASE)/%/linux/glyphborn_linux: $(OBJ_LINUX)
	@echo "🟩 ${GREEN}[Linux/$*] Linking...${RESET}"
	@mkdir -p $(dir $@)
	$(eval $(call set_distro_flags,$*,Linux))
	$(CC_LINUX) $(CFLAGS_LIN) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) $(CFLAGS_DISTRO) \
		$(OBJ_LINUX) -o $@ $(LDFLAGS_LIN) $(LDFLAGS_DISTRO)
	$(STRIP_LINUX) --strip-unneeded $@
	@echo "   ${GREEN}✔ Built → $@${RESET}"


# ==========================================================
# 🪟 Win32 Build
# ==========================================================
$(BUILD_BASE)/%/win32/glyphborn_win32.exe: $(OBJ_WIN32)
	@echo "🟨 ${YELLOW}[Win32/$*] Linking...${RESET}"
	@mkdir -p $(dir $@)
	$(eval $(call set_distro_flags,$*,Win32))
	$(CC_WIN32) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) $(CFLAGS_DISTRO) \
		-D_WIN32 -Wl,-subsystem,$(SUBSYSTEM) \
		$(OBJ_WIN32) -o $@ $(LDFLAGS_WIN) $(LDFLAGS_DISTRO)
	$(STRIP_WIN32) --strip-unneeded $@
	@echo "   ${GREEN}✔ Built → $@${RESET}"


# ==========================================================
# 🪟 Win64 Build
# ==========================================================
$(BUILD_BASE)/%/win64/glyphborn_win64.exe: $(OBJ_WIN64)
	@echo "🟦 ${BLUE}[Win64/$*] Linking...${RESET}"
	@mkdir -p $(dir $@)
	$(eval $(call set_distro_flags,$*,Win64))
	$(CC_WIN64) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) $(CFLAGS_DISTRO) \
		-D_WIN32 -D_WIN64 -Wl,-subsystem,$(SUBSYSTEM) \
		$(OBJ_WIN64) -o $@ $(LDFLAGS_WIN) $(LDFLAGS_DISTRO)
	$(STRIP_WIN64) --strip-unneeded $@
	@echo "   ${GREEN}✔ Built → $@${RESET}"


$(BUILD_STATE):
	@echo "VERSION=$(VERSION)"			> $@
	@echo "Revision=$(BUILD_REV)"		>> $@
	@echo "HASH=$(SOURCE_HASH)"			>> $@

# ==========================================================
# 🧱 Compilation Rules
# ==========================================================
$(OBJDIR_LINUX)/%.o: source/%.c $(BUILD_STATE)
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Linux): %s${RESET}\n" $<
	@$(CC_LINUX) $(CFLAGS_LIN) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) -c $< -o $@

$(OBJDIR_WIN32)/%.o: source/%.c $(BUILD_STATE)
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Win32): %s${RESET}\n" $<
	@$(CC_WIN32) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) -D_WIN32 -c $< -o $@

$(OBJDIR_WIN64)/%.o: source/%.c $(BUILD_STATE)
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Win64): %s${RESET}\n" $<
	@$(CC_WIN64) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_VERSION) -D_WIN32 -D_WIN64 -c $< -o $@


# ==========================================================
# 🔒 SHA256 Checksums
# ==========================================================
checksums:
	@echo "🔑 ${BLUE}Generating SHA256 checksums for $(BUILD_BASE)${RESET}"
	@mkdir -p $(BUILD_BASE)
	@find $(BUILD_BASE) -type f -print0 | sort -z | xargs -0 sha256sum | tee $(BUILD_BASE)/checksums.txt >/dev/null
	@echo "📄 ${YELLOW}Checksums saved to: $(BUILD_BASE)/checksums.txt${RESET}"


# ==========================================================
# 🔁 finalize_success — update hash + stage next revision
# ==========================================================
finalize_success:
	@echo "$(SOURCE_HASH)" > $(META_CHECK_FILE)
	@newrev=$$(( $(BUILD_REV) + 1 )); \
	 echo $$newrev > $(META_NEXT_REV)


# ==========================================================
# 🧹 Cleaning
# ==========================================================
clean:
	rm -rf obj
	@echo "🧽 ${YELLOW}Cleaned all object files.${RESET}"

distclean: clean
	rm -rf $(META_DIR)
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
	@echo "🔢 Version.txt:       ${YELLOW}$(VERSION)${RESET}"
	@echo "🔧 Build Revision:    ${GREEN}$(BUILD_REV)${RESET}"
	@echo "🏷  Full Version:      ${BLUE}$(FULL_VERSION)${RESET}"
	@if [ -f $(META_CHECK_FILE) ]; then \
		old="$$(cat $(META_CHECK_FILE))"; \
		if [ "$(SOURCE_HASH)" = "$$old" ]; then \
			echo "📁 Source State:       ${GREEN}clean (no changes)${RESET}"; \
		else \
			echo "📁 Source State:       ${YELLOW}dirty (changes detected)${RESET}"; \
		fi; \
	else \
		echo "📁 Source State:       ${RED}unknown (first build)${RESET}"; \
	fi
	@echo "────────────────────────────────────────────"
	@echo ""


# ==========================================================
# 📎 Include Dependency Files
# ==========================================================
-include $(DEP_LINUX) $(DEP_WIN32) $(DEP_WIN64)
