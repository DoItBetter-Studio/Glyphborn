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
├── data/             # Embedded game data (layouts, registry, skeletons, world files)
├── docs/             # Technical documentation (render, world, sketch systems)
├── externals/        # External dependencies (if any)
├── includes/         # C header files (.h)
│   ├── game/         # Game logic headers (difficulty, skills)
│   ├── lighting/     # Lighting system headers
│   ├── maths/        # Math utilities
│   ├── skeleton/     # Animation skeleton system
│   └── world/        # World system headers (geometry, collision, tilesets)
├── obj/              # Object files from compilation
├── resources/        # Processed resources (fonts, images, audio)
├── source/           # C source files (.c)
│   ├── achievements/ # Achievement system
│   ├── audio/        # Audio system (platform-specific)
│   ├── game/         # Main game logic
│   ├── input/        # Input handling
│   ├── platform/     # Platform abstraction (Windows/Linux)
│   ├── render/       # Rendering system (software rasterizer)
│   └── world/        # World management (chunk loading, streaming)
├── tools/            # Development tools (Atlas, Mapper, etc.)
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
| **Platform** | Window management, event polling, timing | `platform.h/c`, platform-specific impls |
| **Renderer** | Software rasterizer, framebuffers, depth testing | `render.h/c`, `sketch.h/c` |
| **World** | Chunk streaming, geometry, collision, tilesets | `world/world.h`, geometry/collision systems |
| **Game** | Main loop, camera, UI, achievements | `game.h/c`, `camera.h/c`, `ui.h/c` |
| **Audio** | Sound playback, platform backends | `audio.h/c`, platform-specific impls |
| **Input** | Keyboard/gamepad abstraction | `input.h/c` |
| **Maths** | Vectors, matrices, transformations | `maths/` directory |
| **Lighting** | Directional lighting, shadows | `lighting/directional_light.h` |
| **Skeleton** | Animation system for 3D models | `skeleton/` directory |

### Data Pipeline
- **Embedded Binaries**: World data, tilesets, skeletons compiled into executable.
- **Asset Processing**: Raw assets in `assets/` processed into `resources/` and `data/`.
- **Versioning**: Build system generates versioned outputs with checksums.

---

## 🧰 Development Tools

The project includes a toolchain for content creation:

- **Atlas**: World and spatial data engine
- **Mapper**: Map and world authoring tool
- **Echo**: Audio system abstraction
- **Build Tools**: Cross-platform compilation with dependency tracking

Tools are developed in C# and stored in separate repositories.

---

## 🧱 Building & Running

### Prerequisites
- GCC (Linux) or MinGW-w64 (Windows cross-compile)
- Make

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
make clean          # Remove build artifacts
make distclean      # Remove versioning history
```

### Output Structure
Builds are organized as:
```
build/<version>/<distro>/<platform>/
```
Example: `build/1.0.0/Steam/win64/glyphborn_win64.exe`

### Running
Execute the built binary. On Windows, a debug console is automatically allocated.

---

## 🚧 Project Status

Glyphborn is in active development as both a game and a technical foundation. The repository is publicly visible for transparency and portfolio purposes but remains proprietary.

The underlying runtime and tools will be rebranded and open-sourced under **Damascus — The Steel Editor Suite** in the future. Timeline and licensing details will be announced upon release.

For updates, follow DoItBetter Studio.

---

## 🧾 License & Ownership

**Copyright © 2025–2026 DoItBetter Studio**

All rights reserved. This software and documentation are proprietary intellectual property of DoItBetter Studio.

No license is granted for use, copying, modification, distribution, or derivative works without prior written permission.

DoItBetter Studio reserves the right to relicense under an open-source license upon official release.

---

> *"Strive for more than just Perfection"*  
> — DoItBetter Studio