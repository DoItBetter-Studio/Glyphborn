# ⚙️ Glyphborn

**Proprietary Software — DoItBetter Studio**

Glyphborn is an open-world adventure game written entirely in C with zero external dependencies, featuring a fully 3D environment with a cardinal-locked 2.5D camera perspective.

Built on a custom modular runtime designed for clarity, determinism, and complete platform control, Glyphborn serves as both a creative work and a long-term technical foundation.

Development began in August 2025 as part of DoItBetter Studio's long-term effort to build a fully embedded game alongside the tooling and engine architecture that will eventually become:

**Damascus — The Steel Editor Suite**

This repository is an active internal development branch and is not open source at this time.

---

## 🎮 Game Overview

Glyphborn is a seamless open-world adventure game inspired by the exploration-driven design of classic creature-collection RPGs, built on a continuous chunk-based world system.

**World Architecture**  
The entire world exists as a seamless, streaming environment with **no map transitions**. Each chunk is a 32³ tile volume (32×32×32), and the world streams continuously as you explore. Indoor spaces, outdoor areas, and vertical exploration all exist in the same unified space—no loading screens, no fade-to-black, just pure continuity.

**Visual Style**  
Full 3D geometry with a cardinal-locked 2.5D camera perspective. The camera can rotate to face North, South, East, or West, providing strategic viewpoints while maintaining a consistent gameplay feel reminiscent of classic RPGs.

**Technical Foundation**  
Built entirely in C with **zero external dependencies** beyond the operating system itself. No middleware. No third-party engines. Every system from chunk streaming to rendering is handcrafted and embedded directly into the runtime.

**Design Philosophy**  
Focused on handcrafted systemic worlds, emergent gameplay, and runtime storytelling, with an emphasis on clarity, determinism, and seamless exploration in every subsystem.

---

## 🧠 Engineering Philosophy

Glyphborn is built around disciplined systems design:

**Transparency Over Abstraction**  
No hidden middleware. No opaque frameworks. Every subsystem is readable and intentional.

**Deterministic Modularity**  
Each system (input, render, audio, UI, simulation) is isolated and replaceable.

**Platform Symmetry**  
Windows and Linux maintain mirrored behavior via Win32 and Xlib parity.

**Tool-Driven Creation**  
Runtime systems are paired with dedicated external authoring tools.

**Creative Authorship**  
Designed to support handcrafted systemic worlds and runtime storytelling.

---

## 🧩 Runtime Systems

The game is built on a set of modular, handcrafted subsystems:

| Subsystem | Description |
|-----------|-------------|
| **Platform Layer** | Unified window, context, and input management for Win32 and Xlib |
| **Renderer** | Software/OpenGL hybrid rasterizer with structured 3D tile pipeline |
| **Input System** | Unified keyboard/gamepad abstraction |
| **Audio System** | Platform-specific backend (WinMM / ALSA) |
| **UI System** | Modular layout and menu framework |
| **Timing & Looping** | Frame-consistent delta timing and pacing |
| **Achievements** | Platform-aware integration (local, Steam, GOG) |
| **World Simulation** | Chunk-based streaming system (32³ volumes), seamless world continuity, experimental AI, events, and narrative systems |
| **Build System** | Cross-platform Makefile with versioned output and distro separation |

---

## 🧰 Ecosystem Toolchain

Development of Glyphborn is supported by a split-tool architecture.

- The game runtime remains in **C**.
- Authoring and pipeline tools are developed in **C#**.

| Tool | Purpose |
|------|---------|
| **Atlas** | World and spatial data engine |
| **Echo** | Audio system abstraction |
| **Mapper** | Map and world authoring tool |
| **Additional tools** | Planned under the Steel Editor Suite |

Each tool exists as a standalone repository to support modular development and independent versioning.

---

## 🧱 Build Pipeline

The game uses a cross-platform Makefile supporting:

### Supported Platforms

- 🐧 **Linux** (native GCC)
- 🪟 **Windows** x86 / x64 (MinGW cross-compile)

### Supported Distribution Targets

- Vanilla
- Steam
- GOG

### Usage

```bash
cd ./Glyphborn/

# Build all targets
make

# Disable verbose compiler output
make verbose=false

# Enable debug mode
make debug=true

# Clean build artifacts
make clean

# Remove versioning history and timestamps
make distclean
```

Build output is automatically structured:

```
build/<version>/<distro>/<platform>/
```

**Example:**

```
build/1.0.0/Steam/win64/glyphborn_win64.exe
```

---

## 🚧 Project Status

Glyphborn is in active development as both a game and a technical foundation.

This repository is publicly visible for transparency and portfolio purposes but is **not open source**.

The game's underlying runtime and associated tools are planned for future rebranding and open-source release under:

**Damascus — The Steel Editor Suite**

Timeline and licensing details will be announced upon official release.

---

## 🧾 Ownership & License

**Copyright © 2025–2026 DoItBetter Studio**

All rights reserved.

This software and associated documentation are proprietary intellectual property of DoItBetter Studio.

No license is granted to use, copy, modify, distribute, sublicense, reverse engineer, or create derivative works without prior written permission.

DoItBetter Studio reserves the right to relicense this software under an open-source license upon official release.

---

> *"Every line teaches. Every frame matters."*  
> — DoItBetter Studio