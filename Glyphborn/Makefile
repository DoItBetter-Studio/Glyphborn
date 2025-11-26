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
#   🧩 Auto build_info.h header (semantic version + revision)
#   🧼 Stripping + SHA256 integrity checks
#   🧠 WSL / MSYS environment protection
#   🐞 Debug + Verbose boolean toggles
#   🔄 Auto-incrementing build revision system
#
# Usage:
#   make                → Build all targets (default release)
#   make DEBUG=true     → Build with symbols + console window
#   make VERBOSE=false  → Quiet mode (no warnings)
#   make clean          → Clean build artifacts
#   make distclean      → Full wipe including build revision
# ==========================================================


# ==========================================================
# 🧠 Environment Sanity Check
# ----------------------------------------------------------
# Prevent accidental use in plain Windows CMD/PowerShell
# ==========================================================
ifeq ($(OS),Windows_NT)
  UNAME_S := $(shell uname -s 2>/dev/null)
  ifneq (,$(findstring MINGW,$(UNAME_S)))
  else ifneq (,$(findstring MSYS,$(UNAME_S)))
  else ifneq (,$(findstring Linux,$(UNAME_S)))
  else
    $(error ❌ ERROR: Run under WSL, MSYS2, or Linux — \
            not native Windows CMD/PowerShell.)
  endif
endif

# --- Build metadata directory (keeps repo clean) ---
META_DIR        := .buildmeta
# next revision to use
META_REV_FILE   := $(META_DIR)/build_revision
# last seen VERSION.txt
META_VER_TRACK  := $(META_DIR)/last_version
# last successful build time
META_TS_FILE    := $(META_DIR)/build_timestamp
# "<full_version> <timestamp>"
META_LAST_FILE  := $(META_DIR)/last_success.txt

# ensure dir exists when Make starts evaluating
$(shell mkdir -p $(META_DIR) >/dev/null 2>&1)

# ==========================================================
# 🏷 Semantic Versioning (Major.Minor.Build.Revision)
# ----------------------------------------------------------
# Reads VERSION.txt (e.g. 1.0.0)
# Increments .build_revision only once per successful full build.
# ==========================================================
VERSION := $(shell cat VERSION.txt)

# init files if missing
ifeq ($(wildcard $(META_REV_FILE)),)
  $(shell echo 1 > $(META_REV_FILE))
endif
ifeq ($(wildcard $(META_VER_TRACK)),)
  $(shell echo $(VERSION) > $(META_VER_TRACK))
endif

# reset revision if VERSION.txt changed
ifeq ($(VERSION),$(shell cat $(META_VER_TRACK)))
  BUILD_REV := $(shell cat $(META_REV_FILE))
else
  BUILD_REV := 1
  $(shell echo $(VERSION) > $(META_VER_TRACK))
  $(shell echo $(BUILD_REV) > $(META_REV_FILE))
endif

FULL_VERSION := $(VERSION).$(BUILD_REV)

# --- Output directory ---
BUILD_BASE := build/$(FULL_VERSION)

# --- Optional timestamp for logs only ---
BUILD_TIME := $(shell date +"%Y-%m-%d %H:%M:%S")

# ==========================================================
# 🎯 Build Targets
# ==========================================================
DISTROS := Vanilla Steam GOG

CC_LINUX  = /usr/bin/gcc
CC_WIN64  = /usr/bin/x86_64-w64-mingw32-gcc
CC_WIN32  = /usr/bin/i686-w64-mingw32-gcc

STRIP_LINUX = /usr/bin/strip
STRIP_WIN64 = /usr/bin/x86_64-w64-mingw32-strip
STRIP_WIN32 = /usr/bin/i686-w64-mingw32-strip


# ==========================================================
# ⚙️ Verbose / Debug Mode Configuration
# ==========================================================
VERBOSE ?= true
DEBUG   ?= false

# --- Verbose output ---
ifneq (,$(filter $(VERBOSE),true yes on 1))
  WARNFLAGS := -Wall -Wextra
  VERBOSE_MSG := echo "🔍 Verbose mode enabled (showing compiler warnings)"
else
  WARNFLAGS :=
  VERBOSE_MSG := echo "🤫 Verbose mode disabled (quiet build)"
endif

# --- Debug mode ---
ifneq (,$(filter $(DEBUG),true yes on 1))
  CFLAGS_DEBUG := -g -DDEBUG
  SUBSYSTEM := console
  DEBUG_MSG := echo "🐞 Debug mode enabled (symbols + console window)"
else
  CFLAGS_DEBUG :=
  CFLAGS_BASE += -O2
  SUBSYSTEM := windows
  DEBUG_MSG := echo "🚀 Release mode (optimized, no console)"
endif


# ==========================================================
# 🔧 Base Compiler / Linker Flags
# ==========================================================
CFLAGS_BASE = -std=c17 $(WARNFLAGS) -Iincludes -Iresources
CFLAGS_LIN  = $(CFLAGS_BASE) -I/c/linux/include
LDFLAGS_LIN = -L/c/linux/lib -lX11 -lXext -lXrandr -lXrender -lasound -lm
LDFLAGS_WIN = -luser32 -lgdi32 -ldsound -lkernel32 -lwinmm -lxinput -lm


# ==========================================================
# 🎮 Steamworks SDK Paths
# ==========================================================
STEAM_SDK_PATH        := externals/steamworks
STEAM_SDK_INCLUDE     := $(STEAM_SDK_PATH)/public
STEAM_SDK_LIB_LINUX   := $(STEAM_SDK_PATH)/redistributable_bin/linux64
STEAM_SDK_LIB_WIN64   := $(STEAM_SDK_PATH)/redistributable_bin/win64
STEAM_SDK_LIB_WIN32   := $(STEAM_SDK_PATH)/redistributable_bin


# ==========================================================
# 🧩 Distro-Specific Flag Injection
# Usage: $(eval $(call set_distro_flags,<DistroName>,<PlatformName>))
# ==========================================================
define set_distro_flags
  ifeq ($1,Steam)
    CFLAGS_DISTRO = -DDISTRO_STEAM -I$(STEAM_SDK_INCLUDE)
    ifeq ($2,Linux)
      LDFLAGS_DISTRO = -L$(STEAM_SDK_LIB_LINUX) -lsteam_api
    else ifeq ($2,Win64)
      LDFLAGS_DISTRO = -L$(STEAM_SDK_LIB_WIN64)/steam_api64.lib
    else ifeq ($2,Win32)
      LDFLAGS_DISTRO = -L$(STEAM_SDK_LIB_WIN32)/steam_api.lib
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
# 🎨 Colors
# ==========================================================
GREEN  := \033[1;32m
BLUE   := \033[1;34m
YELLOW := \033[1;33m
GRAY   := \033[0;37m
RESET  := \033[0m


# ==========================================================
# 📁 Source & Asset Discovery
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


# ==========================================================
# 🎯 Full Target Matrix (Linux, Win32, Win64 × Distro)
# ==========================================================
ALL_TARGETS := $(foreach D,$(DISTROS), \
	$(BUILD_BASE)/$(D)/linux/glyphborn_linux \
	$(BUILD_BASE)/$(D)/win32/glyphborn_win32.exe \
	$(BUILD_BASE)/$(D)/win64/glyphborn_win64.exe)


# ==========================================================
# 🏗 Default Rule — Build Everything
# ==========================================================
all: banner
	@$(VERBOSE_MSG)
	@$(DEBUG_MSG)
	@$(MAKE) $(ALL_TARGETS)
	@echo ""
	@echo "────────────────────────────────────────────"
	@echo "✅ ${GREEN}All builds complete for Glyphborn $(FULL_VERSION)${RESET}"
	@echo "📦 Output: ${YELLOW}$(BUILD_BASE)${RESET}"
	@echo "────────────────────────────────────────────"
	@$(MAKE) checksums
	@$(MAKE) finalize_success


# ==========================================================
# 🎬 Build Banner
# ==========================================================
banner:
	@echo ""
	@echo "==========================================="
	@echo "🚀 ${BLUE}Building Glyphborn v$(FULL_VERSION)${RESET}"
	@echo "🕒 Build Time: $(BUILD_TIME)"
	@echo "==========================================="
	@echo ""


# ==========================================================
# 🐧 Linux Build Rule
# ==========================================================
$(BUILD_BASE)/%/linux/glyphborn_linux: $(OBJ_LINUX)
	@echo "🟩 [Linux/$*] Compiling..."
	@mkdir -p $(dir $@)
	@$(MAKE) --no-print-directory gen-build-info PLATFORM="Linux" DISTRO="$*"
	$(eval $(call set_distro_flags,$*,Linux))
	$(CC_LINUX) $(CFLAGS_LIN) $(CFLAGS_DEBUG) $(CFLAGS_DISTRO) $^ -o $@ $(LDFLAGS_LIN) $(LDFLAGS_DISTRO)
	$(STRIP_LINUX) --strip-unneeded $@
	@echo "✅ ${GREEN}Linux/$* built → $@${RESET}"


# ==========================================================
# 🪟 Windows x64 Build Rule
# ==========================================================
$(BUILD_BASE)/%/win64/glyphborn_win64.exe: $(OBJ_WIN64)
	@echo "🟦 [Win64/$*] Compiling..."
	@mkdir -p $(dir $@)
	@$(MAKE) --no-print-directory gen-build-info PLATFORM="Windows x64" DISTRO="$*"
	$(eval $(call set_distro_flags,$*,Win64))
	$(CC_WIN64) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_DISTRO) -D_WIN32 -D_WIN64 \
		-Wl,-subsystem,$(SUBSYSTEM) $(OBJ_WIN64) -o $@ $(LDFLAGS_WIN) $(LDFLAGS_DISTRO)
	$(STRIP_WIN64) --strip-unneeded $@
	@echo "✅ ${GREEN}Win64/$* built → $@${RESET}"


# ==========================================================
# 🪟 Windows x86 Build Rule
# ==========================================================
$(BUILD_BASE)/%/win32/glyphborn_win32.exe: $(OBJ_WIN32)
	@echo "🟨 [Win32/$*] Compiling..."
	@mkdir -p $(dir $@)
	@$(MAKE) --no-print-directory gen-build-info PLATFORM="Windows x86" DISTRO="$*"
	$(eval $(call set_distro_flags,$*,Win32))
	$(CC_WIN32) $(CFLAGS_BASE) $(CFLAGS_DEBUG) $(CFLAGS_DISTRO) -D_WIN32 \
		-Wl,-subsystem,$(SUBSYSTEM) $(OBJ_WIN32) -o $@ $(LDFLAGS_WIN) $(LDFLAGS_DISTRO)
	$(STRIP_WIN32) --strip-unneeded $@
	@echo "✅ ${GREEN}Win32/$* built → $@${RESET}"


# ==========================================================
# 🧩 Auto-Generated Build Info Header
# ==========================================================
gen-build-info:
	@mkdir -p includes
	@echo "// Auto-generated build info" > includes/build_info.h
	@echo "#pragma once" >> includes/build_info.h
	@echo "#define BUILD_VERSION        \"$(VERSION)\"" >> includes/build_info.h
	@echo "#define BUILD_REVISION       \"$(BUILD_REV)\"" >> includes/build_info.h
	@echo "#define BUILD_FULL_VERSION   \"$(FULL_VERSION)\"" >> includes/build_info.h
	@echo "#define BUILD_PLATFORM       \"$(PLATFORM)\"" >> includes/build_info.h
	@echo "#define BUILD_DISTRO         \"$(DISTRO)\"" >> includes/build_info.h
	@echo "#define BUILD_TIMESTAMP      \"$(BUILD_TIME)\"" >> includes/build_info.h


# ==========================================================
# ⚙️ Object Compilation Rules
# ==========================================================
$(OBJDIR_LINUX)/%.o: source/%.c
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Linux): %s${RESET}\n" $<
	@$(CC_LINUX) $(CFLAGS_LIN) $(CFLAGS_DEBUG) -c $< -o $@

$(OBJDIR_WIN64)/%.o: source/%.c
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Win64): %s${RESET}\n" $<
	@$(CC_WIN64) $(CFLAGS_BASE) $(CFLAGS_DEBUG) -D_WIN32 -D_WIN64 -c $< -o $@

$(OBJDIR_WIN32)/%.o: source/%.c
	@mkdir -p $(dir $@)
	@printf "🔧 ${GRAY}Compiling (Win32): %s${RESET}\n" $<
	@$(CC_WIN32) $(CFLAGS_BASE) $(CFLAGS_DEBUG) -D_WIN32 -c $< -o $@


# ==========================================================
# 📜 Version Info Utility (shows last successful build)
# ==========================================================
version:
	@echo ""
	@echo "────────────────────────────────────────────"
	@echo "🧱 ${BLUE}Glyphborn Build Information${RESET}"
	@echo "────────────────────────────────────────────"
	@VERS=$$(cat VERSION.txt 2>/dev/null || echo "0.0.0"); \
	if [ -f "$(META_LAST_FILE)" ]; then \
		read FULL TS < "$(META_LAST_FILE)"; \
		echo "🔢 Last Success: ${YELLOW}$$FULL${RESET}"; \
		echo "🕒 Built At:     ${YELLOW}$$TS${RESET}"; \
	else \
		NEXTREV=$$(cat "$(META_REV_FILE)" 2>/dev/null || echo 1); \
		echo "🔢 Last Success: ${YELLOW}(none yet)${RESET}"; \
		echo "🕒 Built At:     ${YELLOW}(n/a)${RESET}"; \
		echo "➡️  Next Build Will Use: ${YELLOW}$$VERS.$$NEXTREV${RESET}"; \
	fi; \
	if [ -f includes/build_info.h ]; then \
		echo "📄 build_info.h: ${GREEN}present ✅${RESET}"; \
	else \
	 echo "📄 build_info.h: ${YELLOW}missing ⚠️${RESET}"; \
	fi
	@echo "────────────────────────────────────────────"


clean:
	rm -rf obj build includes/build_info.h
	@echo "🧽 ${YELLOW}Cleaned all build artifacts.${RESET}"

distclean: clean
	rm -rf $(META_DIR)
	@echo "🧽 ${YELLOW}Removed build metadata (.buildmeta).${RESET}"


# ==========================================================
# 🔒 SHA256 Integrity Check Summary
# ==========================================================
checksums:
	@echo ""
	@echo "────────────────────────────────────────────"
	@echo "🔑 ${BLUE}SHA256 Checksums (for integrity tracking)${RESET}"
	@echo "────────────────────────────────────────────"
	@echo ""
	@find $(BUILD_BASE) -type f \( \
		-name "glyphborn_linux" -o \
		-name "glyphborn_win32.exe" -o \
		-name "glyphborn_win64.exe" \
	\) -print0 | sort -z | xargs -0 sha256sum | tee $(BUILD_BASE)/checksums.txt
	@echo ""
	@echo "📄 ${YELLOW}Checksums saved to: $(BUILD_BASE)/checksums.txt${RESET}"
	@echo "────────────────────────────────────────────"

# ==========================================================
# 🔁 Post-Build Auto Increment (only after successful build)
# ==========================================================
.PHONY: finalize_success
finalize_success:
	@echo "$(BUILD_TIME)" > $(META_TS_FILE)
	@printf "%s %s\n" "$(FULL_VERSION)" "$(BUILD_TIME)" > $(META_LAST_FILE)
	@oldrev=$$(cat $(META_REV_FILE)); \
	newrev=$$((oldrev + 1)); \
	echo $$newrev > $(META_REV_FILE); \
	echo "🔁 Next build revision prepared → $${newrev}"
