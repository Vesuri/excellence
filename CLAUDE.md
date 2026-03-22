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

Current tools: `DrawTool`, `LineTool`, `RectangleTool`, `BrushTool`, `CurveTool`, `ConnectedLinesTool`, `AirTool`, `FillTool`, `TextTool`, `CarveBrushTool`, `PaletteTool`, `ZoomTool`, `UndoTool`, `ClearTool`.

When adding a new tool, follow the pattern in `rectangletool.{h,cpp}`. Tools may draw a temporary preview during mouse movement (before the mouse button is released) using the temporary-result drawing mechanism: `hover()` returns the rect to save/restore, and `move()` with `mouseButton_ == Qt::NoButton` draws the preview.

#### Tool paradigms

**Mode cycling on repeated activation** — Tools with multiple sub-modes (e.g. outline vs. filled, quadratic vs. Bezier curve, connected lines vs. filled polygon) share a single toolbar button. Clicking the button when the tool is already active cycles to the next mode. Implemented in `activate()` by checking `buffer_->tool() == this`. See `RectangleTool`, `CurveTool`, `ConnectedLinesTool`.

**Right mouse button draws with background/erase color** — Most drawing tools treat the right mouse button as "draw with background color" (i.e., erase). The tool records the button at the start of the operation (`mouseButton_` from the base class, or an `erasing_` flag set in `press()`) and uses `pen()->erase()` instead of `pen()->paint()` throughout. Do not re-check the button mid-operation; record the intent at press time to keep all phases consistent. See `LineTool`, `CurveTool`.

**Tool-managed undo buffer for drag preview** — When a tool needs to show a live preview while a mouse button is held down (the Buffer's built-in hover/move undo only runs with `mouseButton_ == Qt::NoButton`), the tool holds its own `UndoBuffer *undoBuffer_` member. Pattern: save area in `press()`, then in each `move()` call: `undoBuffer_->apply(buffer_)`, `delete undoBuffer_`, recompute the save rect, allocate a new `UndoBuffer`, draw the preview. On `release()`: `undoBuffer_->apply(buffer_)`, `delete undoBuffer_`, draw the permanent result. See `LineTool`, `CurveTool`.

**UndoBuffer bounds safety** — `UndoBuffer::apply()` uses `pos_` with no bounds check. Any rect saved into an `UndoBuffer` (including `hover()` return values) must be clipped to `buffer_->image().rect()` before use, otherwise `setPixel` will be called with out-of-bounds coordinates.

### Palette System

Images are indexed-color. The palette is part of `Buffer`. `PaletteButton` shows individual palette entries; `PaletteQuantizer` / `spatial_color_quant.h` handle color reduction. Palette operations (copy, swap, remap) live in `mainwindow.cpp` under the Palette menu.

### File Format Plugins

Image format support is plugin-based (Qt plugin API). Plugins live under `src/plugins/`:
- **ilbm** — Commodore Amiga IFF ILBM format (the primary target format)
- **raw** — Raw binary pixel data

Each plugin has its own `.pro` file and a `.json` metadata file. The main app loads plugins at startup (see `main.cpp`).

### Tool Grid Layout

The toolbar is a two-row `QGridLayout` (`ui->toolsLayout`). Each tool declares its own position in `addButtonToGridLayout()`. The layout follows the Brilliance toolbox order — see the memory file `project_brilliance_toolbox_layout.md` for the full reference. Current positions:

**Row 0:** Clear(0), Palette(1), Draw(2), Line(3), ConnectedLines(4), Curve(5), Rectangle(6), Ellipse(7), Airbrush(8), Fill(9), Text(10), Brush(11), Undo(12)

Note: Brush(11) cycles between Rectangle and Freehand (carve) modes on repeated activation — CarveBrushTool no longer exists as a separate tool.

**Row 1:** PenTipTool(10), DrawModeTool(11), Zoom(12) — Zoom is under Undo. PenTipTool and DrawModeTool do not replace the active drawing tool when clicked. DrawModeTool is a checkable toggle: unchecked = Normal (Color) mode, checked = uses the selected mode. Right-click opens the options panel. Remaining row-1 slots are reserved for unimplemented Brilliance tools (Animation, Anim-Brush, Grid Lock, etc.).

### Brush Handle

`Brush` has a `handleOffset_` (`QPoint`) that is subtracted from the stamp point so a chosen anchor pixel aligns with the cursor. Defaults to the image center. `paint()`, `erase()`, and `rect()` all apply the offset via `point - handleOffset_`.

### Drawing Algorithms

`Algorithms` (`algorithms.{h,cpp}`) contains:
- Bresenham-style `line()`, `rectangle()`, `fillRectangle()`, `ellipse()`, `fillEllipse()` — used by multiple tools
- `floodFill(QImage&, seed, targetColor, fillColor)` — scanline flood fill returning the changed rect; shared by `DrawTool` (FilledShape) and `ConnectedLinesTool` (FilledPolygon)

### Palette Quantization

`MainWindow::convertToIndexed(QImage)` converts any RGB image to the current buffer palette using nearest-color (squared RGB distance) matching. Use this whenever an external image (clipboard, file) needs to be mapped to the indexed canvas. The higher-quality `PaletteQuantizer` / `spatial_color_quant.h` path is used when reducing a full image to a new palette (e.g. on file open).

### Undo

`UndoBuffer` (`undobuffer.{h,cpp}`) manages undo history. Tools snapshot state through `Buffer` before committing changes.

### Image Menu

`mainwindow.cpp` implements image-level operations under the Image menu. Actions are declared in `mainwindow.ui` and connected in `MainWindow::MainWindow()`. Adding a new Image menu item requires: (1) a `<action>` element in `mainwindow.ui`, (2) a `<addaction>` entry in `menuImage`, (3) a slot in `mainwindow.h`, and (4) a `connect()` + implementation in `mainwindow.cpp`.

## Project documents

Three planning documents live at the repository root:

- **`Brilliance.md`** — Complete feature inventory of the original Amiga application Brilliance (the reference for this project), extracted from its manual.
- **`Features.md`** — Prioritized subset of Brilliance features selected for reimplementation (animation and Amiga hardware specifics excluded).
- **`TODO.md`** — Features not yet implemented, cross-referenced with GitHub issues at https://github.com/Vesuri/excellence/issues. Each item links to the corresponding issue which contains implementation details.
