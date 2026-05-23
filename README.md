# ⚙️ Glyphborn

**Proprietary Software — DoItBetter Studio**

[![License: Proprietary](https://img.shields.io/badge/License-Proprietary-red.svg)](LICENSE)

Glyphborn is a multiplayer, classless, historical Viking-Age sandbox game that blends grounded survival, deep skill-based progression, hand-crafted exploration, player-driven politics, and immersive roleplay systems. Set in the 10th–11th century, players inhabit a shared Viking world where they define their identity through skills, reputation, and choices—no classes, no predetermined roles.

Built entirely in C with zero external dependencies, Glyphborn features a fully 3D environment with a cardinal-locked 2.5D camera perspective. It's designed as both a creative work and a long-term technical foundation for the upcoming **Damascus — The Steel Editor Suite**.

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
│   ├── skeletons/    # Skeletal animation files (.gban, .gbsk)
│   ├── tilesets/     # Tileset data (.bin)
│   ├── ui_skins/     # UI skin files (.gbskin)
│   └── volumes/      # Packed asset volumes (.dat) + asset map
├── docs/             # Technical documentation
├── externals/        # External SDKs (Steamworks, Yggdrasil)
├── includes/         # C header files (.h)
│   ├── generated/    # Auto-generated headers (Audio.h, Geometry.h, etc.)
│   ├── lighting/     # Lighting system headers
│   ├── maths/        # Math utilities
│   ├── skeleton/     # Animation skeleton system
│   └── world/        # World system headers
├── obj/              # Object files from compilation
├── source/           # C source files (.c)
│   ├── achievements/ # Achievement system
│   ├── audio/        # Audio system (platform-specific)
│   ├── generated/    # Auto-generated source files (Geometry.c, Collision.c, etc.)
│   ├── input/        # Input handling
│   ├── platform/     # Platform abstraction (Windows/Linux)
│   ├── render/       # Rendering system
│   └── world/        # World management (chunk loading, streaming)
├── tools/            # Build tools and Damascus suite
│   └── build/        # Build scripts (pack_assets.py, embed_audio.py, embed_data.py)
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
- **Pure C Implementation**: No external libraries or frameworks beyond OS APIs.
- **Modular Subsystems**: Isolated systems for platform, rendering, audio, input, UI, world simulation, etc.
- **Software Rendering**: Custom rasterizer with depth buffering, supporting 3D geometry and textures.
- **Chunk-Based World**: 3x3 cell grid around player, with deterministic loading/unloading.
- **Platform Abstraction**: Unified APIs for Windows (Win32) and Linux (Xlib), with support for custom platforms (Yggdrasil).

### Key Systems
| Subsystem | Description | Key Files |
|-----------|-------------|-----------|
| **Platform** | Window management, event polling, timing, asset volumes | `platform.h/c`, platform-specific impls |
| **Renderer** | Software rasterizer, framebuffers, depth testing | `render.h/c`, `sketch.h/c` |
| **World** | Chunk streaming, geometry, collision, tilesets | `world/world.h`, geometry/collision systems |
| **Game** | Main loop, camera, UI, achievements | `game.h/c`, `camera.h/c`, `ui.h/c` |
| **Audio** | Sound playback, platform backends | `audio.h/c`, platform-specific impls |
| **UI** | Immediate-mode UI, nineslice rendering, skin system | `ui.h/c`, `ui_skin.h/c` |
| **Input** | Keyboard/gamepad abstraction | `input.h/c` |
| **Maths** | Vectors, matrices, transformations | `maths/` directory |
| **Lighting** | Directional lighting | `lighting/directional_light.h` |
| **Skeleton** | Animation system for 3D models | `skeleton/` directory |

### Data Pipeline

The build pipeline processes all game data before compilation through a two-stage system:

**Stage 1 — Asset Volume Packing (`pack_assets.py`)**

All large game data files (`.bin`, `.gbaud`, `.mtx`, `.hdr`, `.gban`, `.gbsk`) are packed into fixed 4GB volume files at `data/volumes/data_000.dat`, `data_001.dat`, etc. A manifest (`asset_map.json`) records the global offset and size of every asset. This avoids executable bloat and keeps shipped files under AV thresholds, while still providing direct memory-mapped access at runtime via `platform_get_asset()`.

**Stage 2 — Code Generation (`embed_data.py`, `embed_audio.py`)**

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
- 🐧 **Linux** (native GCC)
- 🪟 **Windows** x86/x64 (MinGW-w64)

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

# Build with options
make verbose=false  # Disable verbose output
make debug=true     # Enable debug mode

# Clean up
make clean          # Remove object files
make distclean      # Remove all build artifacts
```

### Output Structure
Builds are organized as:
```
build/<version>/<distro>/<platform>/
```
Example: `build/1.0.0/Steam/win64/glyphborn_win64.exe`

### Running
The game executable must be run with `data/volumes/` in the same directory. The volume files contain all large game assets and are required at runtime.

```
glyphborn_win64.exe
data/
  volumes/
    data_000.dat
    asset_map.json
```

The `ascii_tileset`, world matrix, world headers, and UI skins are embedded in the executable and require no external files.

---

## 🚧 Project Status

Glyphborn is in active development as both a game and a technical foundation. The repository is publicly visible for transparency and portfolio purposes but remains proprietary.

The underlying runtime and tools will be rebranded and open-sourced under **Damascus — The Steel Editor Suite** in the future. Timeline and licensing details will be announced upon release.

---

## 🧾 License & Ownership

**Copyright © 2025–2026 DoItBetter Studio**

All rights reserved. This software and documentation are proprietary intellectual property of DoItBetter Studio.

No license is granted for use, copying, modification, distribution, or derivative works without prior written permission.

DoItBetter Studio reserves the right to relicense under an open-source license upon official release.

---

> *"Strive for more than just Perfection"*  
> — DoItBetter Studio