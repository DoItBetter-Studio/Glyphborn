# Glyphborn Engine (Private Dev Build)

Glyphborn is a modular, cross-platform game engine written entirely in C. It’s built for clarity, control, and creative authorship—designed to teach by example and honor the lineage of classic game development.

This build is shared privately for testing and feedback. It is not licensed for public distribution.

---

## 🧠 Philosophy

- **Transparency Over Abstraction**: Every system is handcrafted in C with minimal dependencies.
- **Modular Design**: Subsystems are isolated and extensible—platform, rendering, input, UI, simulation, and tooling.
- **Cross-Platform Parity**: Symmetrical layers for Win32 and Xlib ensure consistent behavior across Windows and Linux.
- **Legacy-Driven**: Inspired by the spirit of classic engines—every byte matters, every system teaches.
- **Creative Authorship**: Built to support immersive, player-driven worlds with AI-powered storytelling.

---

## 🧱 Core Systems

| Subsystem        | Description                                                                 |
|------------------|-----------------------------------------------------------------------------|
| Platform Layer   | Windowing, input, and rendering for Win32 and Xlib                          |
| Renderer         | Software rasterizer with DS-style 3D pipeline and framebuffer layering      |
| Input System     | Unified keyboard/gamepad handling across platforms                          |
| UI System        | Modular UI with buttons, labels, panels, and directional navigation         |
| Font System      | Custom ASCII font renderer for debug overlays and UI text                   |
| Timing & Pacing  | Frame timing logic for consistent simulation across platforms               |
| Character Tools  | Modular asset pipeline for pivot-based animation and stylized visuals       |
| Debug Console    | Planned: integrated console for runtime inspection and command input        |
| World Simulation | Experimental: AI-driven systems for dynamic storytelling and authorship     |

---

## 🛠️ Building

Glyphborn is designed to be built with minimal tooling. You can use `gcc`, `clang`, `Makefile`, or even a simple `.bat` or `.sh` script.

Example (Linux):

```bash
gcc -o glyphborn \
    main.c \
    platform_linux.c \
    render_linux.c \
    input_linux.c \
    audio_linux.c \  # Stubbed for now
    ui.c \
    game.c \
    camera.c \
    -lX11