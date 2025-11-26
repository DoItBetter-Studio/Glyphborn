# ⚙️ Glyphborn Engine (Private Dev Build)

Glyphborn is a modular, cross-platform game engine written entirely in C, built for clarity, control, and creative authorship.
Every subsystem is explicit and educational — no hidden layers, no frameworks, just pure systems engineering.

This repository represents the active development branch for internal testing and tooling integration.
It is not licensed or intended for public distribution.

---

## 🧠 Philosophy

- Transparency Over Abstraction — Every line of code is visible and understandable; no opaque middleware.
- Symmetry by Design — Windows and Linux run identical code paths via Win32 and Xlib parity.
- Deterministic Modularity — Each system (input, render, audio, UI, etc.) is fully isolated and replaceable.
- Tool-Driven Creation — Glyphborn’s ecosystem is expanding with external C# tools for mapping, tiling, and asset pipelines.
- Creative Authorship — Built to support handcrafted, systemic worlds and runtime storytelling.

---

## 🧩 Core Engine Systems

| Subsystem        | Description                                                                                |
| ---------------- | ------------------------------------------------------------------------------------------ |
| Platform Layer   | Unified window, context, and input management for Win32 and Xlib                           |
| Renderer         | Software/GL hybrid rasterizer with DS-style 3D tile pipeline                               |
| Input System     | Unified keyboard/gamepad abstraction (DirectInput/XInput/Xlib)                             |
| Audio System     | Platform-specific sound backend (WinMM / ALSA)                                             |
| UI System        | Modular layout engine for in-game menus and editor tooling                                 |
| Timing & Looping | Frame-consistent delta timing and pacing across all targets                                |
| Achievements     | Platform-aware achievement framework (local, Steam, GOG)                                   |
| World Simulation | Early experimental systems for AI, events, and procedural narrative                        |
| Build System     | Cross-platform Makefile with auto-timestamping, checksum, stripping, and distro separation |

---

## 🧰 Toolchain (WIP)

Glyphborn is transitioning toward a split-tool architecture, where runtime systems remain in C and editors leverage C# + OpenGL (via OpenTK).

| Tool            | Language                | Purpose                                                   |
| --------------- | ----------------------- | --------------------------------------------------------- |
| Mapper          | C# (OpenTK + ImGui.NET) | 3D tile editor for `.map.bin` creation                  |
| Matrix Editor   | C#                      | Manages large-scale chunk/matrix linking between maps     |
| Tileset Tool    | C#                      | Author and validate tileset manifests (`.tileset.json`) |
| Asset Converter | C (legacy)              | Low-level conversion utilities for mesh/material formats  |

These tools share the same binary map format as the C runtime, ensuring seamless editor ↔ engine interchange.

---

## 🧱 Build Pipeline

The Glyphborn Makefile supports multi-platform, multi-distro, and debug/verbose configurations out of the box.

### Supported Targets

- 🐧 Linux (native GCC)
- 🪟 Windows x86 / x64 (cross-compiled via MinGW)
- 💿 Distros: Vanilla, Steam, GOG

### Usage

```
cd ./Glyphborn/

# Build everything (all platforms + distros)
make

# Disable verbose compiler output
make verbose=false

# Enable debugging & console subsystem
make debug=true

# Clean all build artifacts
make clean

# Remove timestamp and history
make distclean
```

Output is organized automatically under versioned directories:

    build/`<version> `/ `<distro>` / `<platform> `/

Example:

    build/1.0.0/Steam/win64/glyphborn_win64.exe

---

## 🧾 License & Status

This branch is a private development build intended for internal use and testing.
Redistribution, resale, or public hosting of the compiled binaries or source code is not permitted.

---

## 🧭 Roadmap Snapshot

- [X] Unified Makefile with cross-compile parity
- [X] Local achievement system
- [X] Steamworks integration (in-progress)
- [ ] GOG Galaxy bindings
- [ ] Tileset & Mapper tools (C# / OpenTK)
- [ ] Full runtime entity layer
- [ ] In-engine debug console
- [ ] Story graph / narrative simulation

---

> “Every line teaches. Every frame matters.”
> — DoItBetter Studios
>
