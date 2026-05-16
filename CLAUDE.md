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

Current tools: `DrawTool`, `LineTool`, `RectangleTool`, `EllipseTool`, `BrushTool`, `CurveTool`, `AirTool`, `FillTool`, `TextTool`, `PaletteTool`, `ZoomTool`, `UndoTool`, `ClearTool`, `PickColorTool`, `GradientTool`, `GridLockTool`, `MirrorTool`, `SegmentTool`, `PenTipTool`, `DrawModeTool`.

When adding a new tool, follow the pattern in `rectangletool.{h,cpp}`. Tools may draw a temporary preview during mouse movement (before the mouse button is released) using the temporary-result drawing mechanism: `hover()` returns the rect to save/restore, and `move()` with `mouseButton_ == Qt::NoButton` draws the preview.

#### Tool paradigms

**Mode cycling on repeated activation** — Tools with multiple sub-modes (e.g. outline vs. filled, quadratic vs. Bezier curve, line vs. connected lines vs. filled polygon) share a single toolbar button. Clicking the button when the tool is already active cycles to the next mode. Implemented in `activate()` by checking `buffer_->tool() == this`. See `RectangleTool`, `CurveTool`, `LineTool`.

**Right mouse button draws with background/erase color** — Most drawing tools treat the right mouse button as "draw with background color" (i.e., erase). The tool records the button at the start of the operation (`mouseButton_` from the base class, or an `erasing_` flag set in `press()`) and uses `pen()->erase()` instead of `pen()->paint()` throughout. Do not re-check the button mid-operation; record the intent at press time to keep all phases consistent. See `LineTool`, `CurveTool`.

**Tool-managed undo buffer for drag preview** — When a tool needs to show a live preview while a mouse button is held down (the Buffer's built-in hover/move undo only runs with `mouseButton_ == Qt::NoButton`), the tool holds its own `UndoBuffer *undoBuffer_` member. Pattern: save area in `press()`, then in each `move()` call: `undoBuffer_->apply(buffer_)`, `delete undoBuffer_`, recompute the save rect, allocate a new `UndoBuffer`, draw the preview. On `release()`: `undoBuffer_->apply(buffer_)`, `delete undoBuffer_`, draw the permanent result. See `LineTool`, `CurveTool`.

**Tool cancel** — `Tool` has a virtual `cancel()` (no-op default). Multi-step tools override it to restore the canvas from their `undoBuffer_` and reset state. Called on `Escape` key in `BufferView::keyPressEvent`. Tools that implement `cancel()`: `CurveTool`, `LineTool` (both Line and connected-line modes), `EllipseTool`, `BrushTool`.

**setBuffer and tool-change connection** — Tools that show a checked/active state must connect to `buffer_->toolChanged(Tool*)` to keep their button in sync. Use the base-class helpers `connectToolChecked()` and `disconnectToolChecked()` in `setBuffer()` — call `disconnectToolChecked()` before `Tool::setBuffer(buffer)`, then `connectToolChecked()` after. Tools that also need state reset on buffer switch (e.g. `CurveTool`, `LineTool`) call their reset method between the two. See any drawing tool's `setBuffer()` for the pattern.

**Tool option windows** — `createOptionsWidget()` creates a floating `Qt::Tool` window opened by right-clicking the toolbar button. Conventions: outer layout uses `setSpacing(8)` and `setContentsMargins(6, 6, 6, 6)`; `QFormLayout` sub-layouts use `setSpacing(4)`; `QHBoxLayout` sub-layouts use `setSpacing(6)`; compact button grids (`QGridLayout`) use `setSpacing(2)`.

**UndoBuffer bounds safety** — `UndoBuffer::apply()` uses `pos_` with no bounds check. Any rect saved into an `UndoBuffer` (including `hover()` return values) must be clipped to `buffer_->image().rect()` before use, otherwise `setPixel` will be called with out-of-bounds coordinates.

### Palette System

Images are indexed-color. The palette is part of `Buffer`. `PaletteButton` shows individual palette entries; `PaletteQuantizer` / `spatial_color_quant.h` handle color reduction. Palette operations (copy, swap, remap) live in `mainwindow.cpp` under the Palette menu. The default buffer is 320×256 with 32 colors (bit depth 5). `buffer.cpp` contains Brilliance-style default palettes for each bit depth (1–8); `defaultPaletteForColors(n)` selects the right one.

### File Format Plugins

Image format support is plugin-based (Qt plugin API). Plugins live under `src/plugins/`:
- **ilbm** — Commodore Amiga IFF ILBM format (the primary target format)
- **raw** — Raw binary pixel data

Each plugin has its own `.pro` file and a `.json` metadata file. The main app loads plugins at startup (see `main.cpp`).

### Tool Grid Layout

The toolbar is a two-row `QGridLayout` (`ui->toolsLayout`). Each tool declares its own position in `addButtonToGridLayout()`. The layout follows the Brilliance toolbox order — see the memory file `project_brilliance_toolbox_layout.md` for the full reference. Current positions:

**Row 0:** Clear(0), Palette(1), Draw(2), Line(3), Curve(4), Rectangle(5), Ellipse(6), Airbrush(7), Fill(8), Text(9), Brush(10), Undo(11)

Note: Line(3) cycles between Line, Connected Lines, and Filled Polygon modes on repeated activation — ConnectedLinesTool no longer exists as a separate tool. Brush(10) cycles between Rectangle and Freehand (carve) modes on repeated activation — CarveBrushTool no longer exists as a separate tool.

**Row 1:** GradientTool(1), GridLockTool(4), MirrorTool(5), SegmentTool(6), PenTipTool(9), DrawModeTool(10), Zoom(11) — Zoom is under Undo. GradientTool, GridLockTool, MirrorTool, SegmentTool, PenTipTool, and DrawModeTool do not replace the active drawing tool when clicked. DrawModeTool is a checkable toggle: unchecked = Normal (Color) mode, checked = uses the selected mode. Right-click opens the options panel. MirrorTool is a toggle: activates X-mirror if none active; deactivates if any active. `/` keyboard shortcut. GridLockTool (`G` key) snaps draw points to a configurable grid. SegmentTool is a checkable toggle that stamps the pen/brush at evenly-spaced intervals; Distance mode fires every N pixels, # of Points mode places N evenly-spaced stamps retrospectively at release. Right-click opens the options panel. Remaining row-1 slots are reserved for unimplemented Brilliance tools (Animation, Anim-Brush, etc.).

**PickColorTool** has no toolbar button. It is activated by clicking the foreground (top half) or background (bottom half) color rectangles in the palette area (`CurrentColorsButton`). Clicking foreground forces the next pick to set the foreground color; clicking background forces it to set the background color. After picking, the previous tool is automatically restored (one-shot mode). The `,` keyboard shortcut and Alt temporary-activation still work as persistent activation.

### Mirror Draw

`Buffer` stores `mirrorX_`, `mirrorY_` (bool) and `mirrorCenterX_/Y_` (int, defaults to image center). When either mirror flag is set, `PenTip::paint/erase()` and `Brush::paint/erase()` stamp at the reflected point(s) in addition to the primary point. Reflected X: `(2*cx - px, py)`; reflected Y: `(px, 2*cy - py)`; both: `(2*cx - px, 2*cy - py)`. Only the draw *point* is reflected — the brush/pen stamp is not flipped. `Buffer::move()` expands the hover save rect to cover all mirror positions before creating `moveUndoBuffer`, preventing preview artifacts.

### Brush Handle

`Brush` has a `handleOffset_` (`QPoint`) that is subtracted from the stamp point so a chosen anchor pixel aligns with the cursor. Defaults to the image center. `paint()`, `erase()`, and `rect()` all apply the offset via `point - handleOffset_`.

### Drawing Algorithms

`Algorithms` (`algorithms.{h,cpp}`) contains:
- Bresenham-style `line()`, `rectangle()`, `fillRectangle()`, `ellipse()`, `fillEllipse()` — used by multiple tools
- `floodFill(QImage&, seed, targetColor, fillColor)` — scanline flood fill; defined but used internally only by `FillTool` (via its own flood-fill logic)

`GradientRenderer` (`gradientrenderer.{h,cpp}`) contains:
- `computeT(px, py, mode, from, to, conformRect = QRect())` — maps a pixel coordinate to a gradient position t ∈ [0, 1]. `conformRect`, when valid, scales the gradient relative to the bounding rect rather than using absolute canvas coordinates (see Gradient Fill Controls below).
- `polygonFillScanline(image, polygon, fillColor, useGradient, range, fillMode, gradFrom, gradTo, conformRect = QRect())` — scanline polygon fill with optional gradient support; shared by `DrawTool` (FilledShape mode) and `LineTool` (FilledPolygon mode). When `conformRect` is valid and mode is Horizontal/Vertical, the function performs a per-row or per-column extent pass internally so each scanline's gradient spans its actual pixel width/height.

**`Algorithms::fillEllipse` row-order guarantee** — pixels are delivered strictly top-to-bottom, left-to-right within each row (the outer loop is `for dy in -yBound..yBound`, inner is a left-to-right `line()` call). Code that accumulates per-row state inside the callback (e.g. `EllipseTool`'s conform logic that re-solves the quadratic once per new y) can rely on this.

### Gradient Fill Controls

`gradientrange.h` exports two global flags alongside `activeGradientFillMode`:

- **`conformFill`** — when true, gradient fills scale to the cross-sections of the filled area rather than using absolute canvas coordinates. For Horizontal/Vertical modes this means per-row or per-column normalization (each row/column gets a gradient that spans exactly its own pixel extent). For Linear/Radial/Spherical/Highlight the gradient is bounded by the fill's bounding rect. Implemented in `GradientRenderer::computeT()` via the `conformRect` parameter, and via pre-scan loops in `polygonFillScanline`, `FillTool::applyGradientFill`, and `EllipseTool::drawEllipseShape`.
- **`centerFill`** — when true, Radial/Spherical/Highlight fills originate from the geometric center of the filled area (fill rect center, polygon bbox center, `(cx_, cy_)` for ellipses) rather than the drag start point.
- **`gradientFillIsRadial(mode)`** — inline helper in `gradientrange.h` returning true for `FillRadial | FillSpherical | FillHighlight`; use this instead of repeating the three-way check. Note: `FillTool::applyGradientFill` intentionally excludes `FillHighlight` from its center check because that mode uses a separate BFS distance-transform path that doesn't accept a gradient origin point.

### Palette Quantization

`MainWindow::convertToIndexed(QImage)` converts any RGB image to the current buffer palette using nearest-color (squared RGB distance) matching. Use this whenever an external image (clipboard, file) needs to be mapped to the indexed canvas. The higher-quality `PaletteQuantizer` / `spatial_color_quant.h` path is used when reducing a full image to a new palette (e.g. on file open).

### Dirty Tracking

`Buffer::dirty_` is set when the canvas has unsaved changes (`release()` after a non-null modified area, or `notifyModified()`). `clearDirty()` clears it (called in `MainWindow::saveFile()` after a successful write). `dirtyChanged(bool)` signal is emitted on transitions. `MainWindow` connects to this signal to update the window title (`*Excellence` when dirty) and shows a Save/Discard/Cancel dialog in `closeEvent()` if dirty.

### Undo

`UndoBuffer` (`undobuffer.{h,cpp}`) manages undo history. Tools snapshot state through `Buffer` before committing changes.

### Image Menu

`mainwindow.cpp` implements image-level operations under the Image menu. Actions are declared in `mainwindow.ui` and connected in `MainWindow::MainWindow()`. Adding a new Image menu item requires: (1) a `<action>` element in `mainwindow.ui`, (2) a `<addaction>` entry in `menuImage`, (3) a slot in `mainwindow.h`, and (4) a `connect()` + implementation in `mainwindow.cpp`.

### Keyboard Shortcuts

All canvas-level keyboard shortcuts are handled in `BufferView::handleKey()` (`bufferview.cpp`). When adding a new shortcut, add a `case Qt::Key_X:` there. Check for modifier keys with `event->modifiers() & Qt::ShiftModifier` etc.

Currently wired tool-activation keys (plain key activates; repeated press cycles mode where applicable):

| Key | Tool / Action |
|-----|---------------|
| `D` | DrawTool (cycles Dotted → Connected → FilledShape) |
| `W` | LineTool (cycles Line → Connected Lines → Filled Polygon) |
| `Q` | CurveTool (cycles Quadratic ↔ Bézier) |
| `R` / `Shift+R` | RectangleTool (Outline / Filled) |
| `C` / `Shift+C` | EllipseTool (Outline / Filled) |
| `F` | FillTool |
| `A` / `Shift+A` | AirTool / open options |
| `T` | TextTool |
| `B` | BrushTool (cycles Rectangle ↔ Freehand) |
| `,` | PickColorTool (persistent) |
| `Alt` (held) | PickColorTool (temporary; restores on release) |

Other canvas shortcuts: `S` show page, `N`/`Shift+N` center/pan, `M`/`Shift+M` magnifier, `P` pixel grid, `G` grid lock, `/` mirror, `Tab` gradient cycle, `K`/`Shift+K` clear, `[`/`]` cycle palette color, `U`/`Shift+U`/`Alt+U` undo/redo, `Alt+R` flip gradient range, `Alt+J`/`Alt+Shift+J` next/prev gradient range, `J` swap buffers, arrow keys scroll, `+`/`-` zoom.

Brush transform shortcuts are menu actions in `mainwindow.ui`: `x` flip H, `y` flip V, `z` rotate 90° CW, `H`/`h` double/halve, `o` outline, `Shift+O` trim, `Shift+B` restore.

## Project documents

Three planning documents live at the repository root:

- **`Brilliance.md`** — Complete feature inventory of the original Amiga application Brilliance (the reference for this project), extracted from its manual.
- **`Features.md`** — Prioritized subset of Brilliance features selected for reimplementation (animation and Amiga hardware specifics excluded).
- **`TODO.md`** — Features not yet implemented, cross-referenced with GitHub issues at https://github.com/Vesuri/excellence/issues. Each item links to the corresponding issue which contains implementation details.
