# ⚙️ Glyphborn

**Proprietary Software — DoItBetter Studio**

[![License: Proprietary](https://img.shields.io/badge/License-Proprietary-red.svg)](LICENSE)

Glyphborn is a multiplayer, classless, historical Viking-Age sandbox game that blends grounded survival, deep skill-based progression, hand-crafted exploration, player-driven politics, and immersive roleplay systems. Set in the 10th–11th century, players inhabit a shared Viking world where they define their identity through skills, reputation, and choices—no classes, no predetermined roles.

Built in C17, Glyphborn uses platform APIs, OpenGL 3.3, X11/GLX on Linux, and the bundled Steamworks integration where applicable. It features a fully 3D environment with a cardinal-locked 2.5D camera perspective. It's designed as both a creative work and a long-term technical foundation for the upcoming **Damascus — The Steel Editor Suite**.

This repository is an active internal development branch and is not open source at this time.

---

## 📁 Project Structure

```
Glyphborn/
├── assets/           # Raw game assets (audio, images, models, tilesets, maps)
├── build/            # Compiled binaries and distributions (versioned)
├── data/             # Game data
│   ├── audio/        # Audio files (.gbaud)
│   ├── layouts/      # World layout geometry and collision (.bin)
│   ├── registry/     # World registry JSON files
│   ├── models/       # Model and animation data
│   ├── tilesets/     # Tileset data (.bin)
│   ├── ui_skins/     # UI skin files (.gbskin)
│   └── volumes/      # Packed asset volumes (.dat) + asset map
├── docs/             # Technical documentation
├── externals/        # External SDKs (Steamworks, Yggdrasil)
├── includes/         # C header files (.h)
│   ├── generated/    # Auto-generated data bindings
│   ├── audio/        # Audio interfaces and formats
│   ├── core/         # Core runtime types
│   ├── dialogue/     # Dialogue and localization interfaces
│   ├── entities/     # Entity and skill systems
│   ├── game/         # Game state and gameplay interfaces
│   ├── lighting/     # Lighting system headers
│   ├── maths/        # Math utilities
│   ├── models/       # Model and animation interfaces
│   ├── platform/     # Platform abstraction headers
│   ├── render/       # OpenGL and camera interfaces
│   ├── save/         # Save/load interfaces
│   ├── ui/           # UI interfaces
│   └── world/        # World system headers
├── obj/              # Object files from compilation
├── source/           # C source files (.c)
│   ├── achievements/ # Achievement system
│   ├── audio/        # Audio system (platform-specific)
│   ├── core/         # Core runtime code
│   ├── dialogue/     # Dialogue and localization
│   ├── entity/       # Entity and skill systems
│   ├── game/         # Game state and main gameplay flow
│   ├── generated/    # Auto-generated data bindings
│   ├── input/        # Input handling
│   ├── models/       # Model and animation code
│   ├── platform/     # Platform backends (Windows/Linux)
│   ├── render/       # OpenGL renderer, camera, and mesh code
│   ├── save_load/    # Save/load backends
│   ├── ui/           # UI implementation
│   └── world/        # World management (chunk loading, streaming)
├── tools/            # Build tools and Damascus suite prototypes
│   ├── atlas/        # Atlas data tooling
│   ├── build/        # Packing, code generation, and version scripts
│   ├── echo/         # Echo audio tooling
│   ├── font_tool/    # Font tooling
│   └── mapper/       # Mapper world authoring
├── GDD.md            # Game Design Document
├── LICENSE           # Proprietary license
├── Makefile          # Cross-platform build system
└── README.md         # This file
```

---

## 🎮 Game Features

### Core Pillars
- **Exploration**: Handcrafted Viking world with fjords, forests, mountains, and multi-floor interiors. Travel by foot, horse, or longship with no fast travel.
- **Survival**: Grounded mechanics including hunger, warmth, weather effects, injury, and fatigue.
- **Skill Progression**: Classless system where skills grow through use, unlocking crafting recipes, bonuses, and techniques (e.g., combat, woodworking, trading).
- **Combat**: Real-time, skill-based combat on a true 3D tile grid with verticality, stamina, and positioning.
- **Multiplayer**: 1–10 player private/co-op worlds with RP-focused community servers. Supports emergent player factions and governance.

### World & Setting
- Seamless open-world with continuous chunk-based streaming (32³ tile volumes).
- Historical authenticity: Pagan beliefs, regional cultures, and political tensions between Norse paganism and Christianity.
- Optional Seer path for symbolic visions and insights.

---

## 🧠 Technical Overview

### Architecture
- **C17 Implementation**: A native C17 runtime with platform APIs and OpenGL 3.3; no game engine or large framework dependency.
- **Modular Subsystems**: Isolated systems for platform, rendering, audio, input, UI, world simulation, etc.
- **OpenGL Rendering**: OpenGL 3.3 core rendering for world and skinned meshes, with a software framebuffer/UI rasterizer for selected drawing paths and debug utilities.
- **Chunk-Based World**: 3x3 cell grid around player, with deterministic loading/unloading.
- **Platform Abstraction**: Unified APIs for Windows (Win32) and Linux (Xlib), with support for custom platforms (Yggdrasil).

### Key Systems
| Subsystem | Description | Key Files |
|-----------|-------------|-----------|
| **Platform** | Window management, event polling, timing, asset volumes | `platform/`, platform-specific impls |
| **Renderer** | OpenGL 3.3 world and skinned-mesh rendering, framebuffers, camera, and debug rasterization | `render/`, `world/world_render_gl.c` |
| **World** | Chunk streaming, geometry, collision, tilesets | `world/world.h`, geometry/collision systems |
| **Game** | Main loop, camera, UI, dialogue, entities, and achievements | `game/`, `render/camera.c`, `ui/`, `dialogue/` |
| **Audio** | Sound playback, platform backends | `audio/`, platform-specific impls |
| **UI** | Immediate-mode UI, nineslice rendering, skin system | `ui/`, `ui_skin.c` |
| **Input** | Keyboard/gamepad abstraction | `input/`, platform-specific impls |
| **Maths** | Vectors, matrices, transformations | `maths/` directory |
| **Lighting** | Directional lighting | `lighting/directional_light.h` |
| **Models** | Mesh, material, skeleton, and animation systems | `models/` directory |
| **Save/Load** | Region files and platform-specific save handling | `save/`, `save_load/` |

### Data Pipeline

The build pipeline processes all game data before compilation through a two-stage system:

**Stage 1 — Asset Volume Packing (`pack_assets.py`)**

All large game data files (`.bin`, `.gbaud`, `.mtx`, `.hdr`, `.gbani`, `.gbsk`, `.gbmsh`, `.gbmat`, `.locale`) are packed into fixed 4GB volume files at `data/volumes/data_000.dat`, `data_001.dat`, etc. A manifest (`asset_map.json`) records the volume, offset, and size of every asset. This avoids executable bloat while still providing direct asset access at runtime via `platform_get_asset()`.

**Stage 2 — Code Generation (`embed_data.py`)**

These scripts read `asset_map.json` and generate typed C headers and source files with offset macros for each asset. Generated files live in `includes/generated/` and `source/generated/` and are never edited by hand.

**Embedded Data (stays in the executable)**

A small set of engine-critical data is embedded directly via `ld -r -b binary` and never goes through the volume system:
- `ascii_tileset.h` — the engine's sole bitmap font (special, will never change)
- `.gbskin` files — compiled UI skins
- `world_matrix.mtx` — world streaming infrastructure
- `world_headers.hdr` — world streaming infrastructure

**UI Skin Format (`.gbskin`)**

UI skins are authored in **Pigment** (part of the Damascus suite) and exported as `.gbskin` binary files. The format is:

```
[magic:         4 bytes  — 'G','B','U','I']
[palette_count: uint32_t]
[palettes:      palette_count * 256 * uint32_t ARGB]
[per element:
    width:      uint32_t
    height:     uint32_t
    pixels:     width * height * uint32_t ARGB]
```

Elements are written in a fixed order matching the `UISkin` struct. Skins support multiple palette variants for runtime color swapping without touching pixel data.

**Versioning**

The build system generates versioned outputs with SHA256 checksums. Revision numbers increment only when source files change, enabling incremental builds.

### Localization and Generated Bindings

Localization data is compiled into locale blobs and exposed to C through generated files in `includes/generated/`. `LocaleBindings.h` provides generated direct-index constants for entries in the currently active locale:

- `Locales` defines the locale string-table entries, such as the block containing `LOCALES_EN_US` and `LOCALES_PT_BR`.
- `Ui` defines UI text entries, such as `UI_MENU_START_GAME` and `UI_MENU_SETTINGS`.

The locale selected by `locale_set_active(locale_index)` is resolved from the locale array independently of the generated string IDs. Once a locale is active, UI code passes a generated binding directly to `locale_get_string_terminated()`:

```c
ui_button(x, y, width, height,
          locale_get_string_terminated(UI_MENU_START_GAME),
          UI_COLOR_BLACK);
```

For language-selection controls, the locale array index is kept separate from the string-table ID. A selector can iterate over the locale entries and use its loop index for activation while using the corresponding generated binding for display:

```c
for (int32_t i = 0; i < LOCALES_COUNT; i++)
{
  if (ui_button(x, y, width, height,
          locale_get_string_terminated(LOCALES_EN_US + i),
          UI_COLOR_BLACK))
  {
    locale_set_active(i);
  }
}
```

The generated binding is a direct index into `g_ActiveLocale`; `locale_set_active(i)` selects a separate entry in the locale array. This keeps locale selection independent from generated string-ID numbering as more bindings are added. `locale_get_string()` also exposes the byte length for code that needs length-prefixed text; `locale_get_string_terminated()` copies the value into the locale scratch buffer and adds a null terminator for standard C/UI APIs. Generated bindings and generated locale data are build outputs and must not be edited by hand.

---

## 🧰 Development Tools — Damascus Suite

The Damascus suite is a set of C# WinForms authoring tools developed alongside the engine. Each tool owns a specific resource domain and outputs a binary format the engine consumes directly.

| Tool | Purpose | Output Format |
|------|---------|---------------|
| **Atlas** | World and spatial data engine | `.mtx` |
| **Mapper** | Map and world authoring | `.area` (formerly `.gbm`) |
| **Echo** | Audio authoring and abstraction | `.gbaud` |
| **Pigment** | UI skin authoring | `.gbskin` |
| **Forge** | 3D model authoring | TBD |

Each tool also has a lossless project format for editing:
- Atlas: `.mtx`
- Mapper: `.area`
- Echo: `.json`
- Pigment: `.pigment`
- Forge: TBD

Tools are developed in separate repositories and are planned for open-source release under Damascus.

---

## 🧱 Building & Running

### Prerequisites
- GCC (Linux) or MinGW-w64 (Windows cross-compile)
- Make
- Python 3 (for build scripts)

### Supported Platforms
- 🐧 **Linux** (GCC, X11/GLX, OpenGL 3.3)
- 🪟 **Windows** x86/x64 (MinGW-w64, Win32, OpenGL 3.3)
- ⚙️ **Yggdrasil** (toolchain support exists, but the Makefile target is currently disabled)

### Supported Distributions
- Vanilla (standalone)
- Steam (with Steamworks SDK)
- GOG (future)

### Build Commands
```bash
# Navigate to project root
cd ./Glyphborn/

# Build all targets
make

# Build with options (Make variables are uppercase)
make VERBOSE=false  # Disable compiler warnings
make DEBUG=true      # Enable debug symbols and a console window

# Clean up
make clean          # Remove object files
make distclean      # Remove all build artifacts
```

### Output Structure
Builds are organized as:
```
build/<version>/<distro>/<platform>/
```
Example: `build/0.0.1.185/Vanilla/win64/glyphborn_win64.exe`

### Running
The game executable must be run with its generated `data/` directory in the same directory. The volume files contain the large game assets and are required at runtime.

```
glyphborn_win64.exe
data/
  data_000.dat
  asset_map.json
```

The `ascii_tileset`, world matrix, world headers, and UI skins are embedded in the executable and require no external files.

---

## 🚧 Project Status

Glyphborn is in active development as both a game and a technical foundation. The repository is publicly visible for transparency and portfolio purposes but remains proprietary.

The underlying runtime and tools may be rebranded and open-sourced under **Damascus — The Steel Editor Suite** in the future. Timeline and licensing details will be announced upon release.

---

## 🧾 License & Ownership

**Copyright © 2025–2026 DoItBetter Studio**

All rights reserved. This software and documentation are proprietary intellectual property of DoItBetter Studio.

No license is granted for use, copying, modification, distribution, or derivative works without prior written permission.

DoItBetter Studio reserves the right to relicense under an open-source license upon official release.

---

> *"Strive for more than just Perfection"*  
> — DoItBetter Studio