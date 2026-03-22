# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

This is a Qt/C++ application built with qmake.

```bash
qmake Excellence.pro
make
```

On macOS the app builds as `Excellence.app`. The project has a subdirectory structure — the root `.pro` file pulls in `src/src.pro` and `src/plugins/plugins.pro`.

There are no automated tests; testing is manual UI interaction.

## Architecture

**Excellence** is a pixel art / indexed-color image editor targeting Commodore Amiga-style workflows.

### Core Concepts

- **Buffer** (`buffer.{h,cpp}`) — the central image data store. Nearly all operations read from or write to the Buffer. It manages the pixel data, palette, and undo state.
- **BufferView** (`bufferview.{h,cpp}`) — the Qt widget that renders the Buffer with zoom/pan. It receives mouse events and forwards them to the active Tool.
- **Pen / PenTip / Brush** — layered drawing primitives. `PenTip` is the lowest-level pixel writer; `Pen` adds color logic; `Brush` adds bitmap-stamp support.

### Tool System

All tools inherit from `Tool` (two subtypes: `Modify` and `Zoom`). The active tool is set on `BufferView` and receives mouse press/move/release events.

Current tools: `DrawTool`, `LineTool`, `RectangleTool`, `BrushTool`, `PaletteTool`, `ZoomTool`, `UndoTool`, `ClearTool`.

When adding a new tool, follow the pattern in `rectangletool.{h,cpp}` (the most recent addition). Tools may draw a temporary preview during mouse movement (before the mouse button is released) using the temporary-result drawing mechanism introduced in a recent commit.

### Palette System

Images are indexed-color. The palette is part of `Buffer`. `PaletteButton` shows individual palette entries; `PaletteQuantizer` / `spatial_color_quant.h` handle color reduction. Palette operations (copy, swap, remap) live in `mainwindow.cpp` under the Palette menu.

### File Format Plugins

Image format support is plugin-based (Qt plugin API). Plugins live under `src/plugins/`:
- **ilbm** — Commodore Amiga IFF ILBM format (the primary target format)
- **raw** — Raw binary pixel data

Each plugin has its own `.pro` file and a `.json` metadata file. The main app loads plugins at startup (see `main.cpp`).

### Drawing Algorithms

`Algorithms` (`algorithms.{h,cpp}`) contains Bresenham-style `line()`, `rectangle()`, and `fillRectangle()` helpers used by multiple tools.

### Undo

`UndoBuffer` (`undobuffer.{h,cpp}`) manages undo history. Tools snapshot state through `Buffer` before committing changes.

## Project documents

Three planning documents live at the repository root:

- **`Brilliance.md`** — Complete feature inventory of the original Amiga application Brilliance (the reference for this project), extracted from its manual.
- **`Features.md`** — Prioritized subset of Brilliance features selected for reimplementation (animation and Amiga hardware specifics excluded).
- **`TODO.md`** — Features not yet implemented, cross-referenced with GitHub issues at https://github.com/Vesuri/excellence/issues. Each item links to the corresponding issue which contains implementation details.
