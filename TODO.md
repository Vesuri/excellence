# TODO.md

Features remaining to implement, compared against Features.md. Items already fully implemented are omitted. Partially implemented items note what is missing.

---

## 1. Core Drawing Tools

- [ ] **Freehand Draw — Dotted** — Dot spacing should increase with mouse speed (currently Draw mode paints continuously). [#1](https://github.com/Vesuri/excellence/issues/1)
- [ ] **Connected Lines** — Multi-click polyline; each click continues from the previous endpoint; right-click or double-click ends. [#2](https://github.com/Vesuri/excellence/issues/2)
- [ ] **Filled Polygon** — Like Connected Lines but fills the enclosed area on termination. [#3](https://github.com/Vesuri/excellence/issues/3)
- [ ] **Elliptical Curve** — Single-control-point curve: two endpoints then drag midpoint to adjust curvature, click to commit. [#4](https://github.com/Vesuri/excellence/issues/4)
- [ ] **Bezier Curve** — Four-point Bezier with two anchors and two control points; all draggable until right-click commits. [#5](https://github.com/Vesuri/excellence/issues/5)
- [ ] **Rectangle — Corner to Corner / Center to Corner modes** — Sub-option to switch anchor-point convention (currently only corner-to-corner). [#6](https://github.com/Vesuri/excellence/issues/6)
- [ ] **Outline Ellipse** — Click-drag unfilled ellipse; Ctrl for perfect circle. [#7](https://github.com/Vesuri/excellence/issues/7)
- [ ] **Filled Ellipse** — Like Outline Ellipse but fills. [#7](https://github.com/Vesuri/excellence/issues/7)
- [ ] **Ellipse — Center to Corner / Corner to Corner / Rotate modes** — Sub-options including free rotation before commit. [#7](https://github.com/Vesuri/excellence/issues/7)
- [ ] **Airbrush — Fine Spray** — Sprays individual single-pixel droplets; dot density builds with dwell time. [#8](https://github.com/Vesuri/excellence/issues/8)
- [ ] **Airbrush — Splatter** — Uses the current pen tip or brush shape as the spray droplet. [#8](https://github.com/Vesuri/excellence/issues/8)
- [ ] **Airbrush — Shape Airbrush** — Current brush shape used as mask; paint builds through it gradually. [#8](https://github.com/Vesuri/excellence/issues/8)
- [ ] **Airbrush nozzle controls** — Nozzle size, Flow (0–100%), and Focus sliders. [#8](https://github.com/Vesuri/excellence/issues/8)
- [ ] **Fill Tool** — Flood-fill contiguous same-color pixels; fill type determined by active Draw Mode. [#9](https://github.com/Vesuri/excellence/issues/9)
- [ ] **Text Tool** — Font/size/style selector; renders text as a movable brush for placement. [#10](https://github.com/Vesuri/excellence/issues/10)
- [ ] **Erase / CLR** — Right-click variant should clear with background color (currently always clears to color 0). Keyboard shortcuts `K` / `Shift-K` missing. [#11](https://github.com/Vesuri/excellence/issues/11)
- [ ] **Undo** — Currently single-level; needs multi-level undo (limited only by RAM). Keyboard shortcut `u` missing. [#12](https://github.com/Vesuri/excellence/issues/12)
- [ ] **Redo** — Restore previously undone changes. Keyboard: `U`. [#12](https://github.com/Vesuri/excellence/issues/12)
- [ ] **Undo All / Redo All** — Jump to start or end of undo history. Keyboard: `Alt-u` / `Alt-U`. [#12](https://github.com/Vesuri/excellence/issues/12)
- [ ] **Clear Undo Buffer** — Release undo history RAM. Keyboard: `Ctrl-u`. [#12](https://github.com/Vesuri/excellence/issues/12)

---

## 2. Brush System

- [ ] **Carve Brush (Freehand)** — Freehand outline selection to extract an irregularly shaped brush. [#13](https://github.com/Vesuri/excellence/issues/13)
- [ ] **Load Brush** — Load a saved brush image from disk. [#14](https://github.com/Vesuri/excellence/issues/14)
- [ ] **Save Brush** — Save current brush to disk. [#14](https://github.com/Vesuri/excellence/issues/14)
- [ ] **Copy / Paste Brush via Clipboard** — System clipboard integration for brush transfer. [#14](https://github.com/Vesuri/excellence/issues/14)
- [ ] **Delete Brush** — Remove the current brush from memory, reverting to the default pen tip. [#14](https://github.com/Vesuri/excellence/issues/14)
- [ ] **8 Brush Wells** — Eight storage slots; click to store, click again to retrieve. [#15](https://github.com/Vesuri/excellence/issues/15)
- [ ] **Brush Handle — Corner / Center / Custom** — Set anchor point of the brush at any corner, center, or arbitrary location. [#16](https://github.com/Vesuri/excellence/issues/16)
- [ ] **Brush Remap** — Remap brush colors to closest matches in the current working palette. [#17](https://github.com/Vesuri/excellence/issues/17)
- [ ] **BG → FG swap** — Replace background-colored pixels in the brush with the foreground color; also swap BG↔FG colors. [#17](https://github.com/Vesuri/excellence/issues/17)
- [ ] **Auto Background detection** — Sample four brush corners; if all identical, treat that color as transparent background automatically. [#17](https://github.com/Vesuri/excellence/issues/17)
- [ ] **Tile Cut** — Trim brush by 1 pixel on right and bottom so tiled stamps fit without overlap. [#19](https://github.com/Vesuri/excellence/issues/19)
- [ ] **Brush Align** — Visually reposition a brush relative to its original canvas location. [#19](https://github.com/Vesuri/excellence/issues/19)
- [ ] **Brush Dimensions display** — Real-time readout of brush width and height in pixels. [#19](https://github.com/Vesuri/excellence/issues/19)
- [ ] **Brush resize — Adjust (interactive)** — Free drag to resize proportionally or per-axis. Keyboard: `Z`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush resize — Double / Halve** — Double or halve both dimensions, width only, or height only. Keyboard: `H` / `h` / `X` / `Y`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush resize — 1px / 10px increments** — Keyboard nudge: `-` / `=` / `Shift--` / `Shift-=`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Flip Horizontal / Vertical** — Mirror the brush on either axis. Keyboard: `x` / `y`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Rotate — Numeric** — Enter rotation in degrees; aspect-corrected. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Rotate — Interactive** — Free drag clockwise/counter-clockwise. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Rotate — Fast 90°** — Instant 90° non-aspect-corrected rotation. Keyboard: `z`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Restore** — Return to last undistorted brush state. Keyboard: `Shift-B`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Bend X / Y** — Curve the brush horizontally or vertically with interactive drag. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Shear X / Y** — Slant the brush horizontally or vertically with interactive drag. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Outline** — Add a 1-pixel border in the foreground color at background-to-color boundaries. Keyboard: `o`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Brush Trim** — Remove a 1-pixel boundary where background meets another color. Keyboard: `O`. [#18](https://github.com/Vesuri/excellence/issues/18)
- [ ] **Pen Tip Tool** — Selectable pen tip sizes and shapes (multi-pixel circle, square, etc.); currently only 1×1 pixel is used. Keyboard: `.` for single pixel. [#20](https://github.com/Vesuri/excellence/issues/20)
- [ ] **Anti-Aliasing for brush transforms** — Low / Medium / High quality levels for resize, rotation, and distortion. [#18](https://github.com/Vesuri/excellence/issues/18)

---

## 3. Draw Modes

Currently only default Color mode (single foreground color) and Brush mode (stamp brush's own colors) are effectively supported. All the following are missing:

- [ ] **Replace mode** — All brush colors treated as foreground; no transparency. [#21](https://github.com/Vesuri/excellence/issues/21)
- [ ] **Smear mode** — Drags and mixes pixels in the direction of travel. [#21](https://github.com/Vesuri/excellence/issues/21)
- [ ] **Smooth mode** — Softens color transitions between adjacent pixels. [#21](https://github.com/Vesuri/excellence/issues/21)
- [ ] **Range mode** — Increments/decrements each pixel one step along the active gradient range. [#22](https://github.com/Vesuri/excellence/issues/22)
- [ ] **Average Smear mode** — Smear restricted to gradient range colors; averages with underlying colors. [#22](https://github.com/Vesuri/excellence/issues/22)
- [ ] **Cycle mode** — Paints cycling through gradient colors sequentially; direction depends on mouse button. [#22](https://github.com/Vesuri/excellence/issues/22)
- [ ] **Random mode** — Like Cycle but color order is randomized. [#22](https://github.com/Vesuri/excellence/issues/22)
- [ ] **Tint mode** — Alters chroma of painted areas toward the current color. [#23](https://github.com/Vesuri/excellence/issues/23)
- [ ] **Colorize mode** — Changes hue of areas containing chroma; no effect on neutrals. [#23](https://github.com/Vesuri/excellence/issues/23)
- [ ] **Brighten mode** — Advances each pixel to the next brighter shade in its hue range. [#23](https://github.com/Vesuri/excellence/issues/23)
- [ ] **Darken mode** — Advances each pixel to the next darker shade in its hue range. [#23](https://github.com/Vesuri/excellence/issues/23)
- [ ] **Mix mode** — Mixes on-screen pixel colors with the current foreground color. [#23](https://github.com/Vesuri/excellence/issues/23)
- [ ] **Negative mode** — Inverts colors being painted over. [#23](https://github.com/Vesuri/excellence/issues/23)
- [ ] **Dither1 mode** — Dithered pattern of foreground color and transparency; Amount slider controls ratio. [#24](https://github.com/Vesuri/excellence/issues/24)
- [ ] **Dither2 mode** — Dithered pattern of foreground and background color; Amount slider controls ratio. [#24](https://github.com/Vesuri/excellence/issues/24)
- [ ] **Brush fill modes — Stretch / Pattern / Shape / Perspective** — Tile or warp the brush to fill areas. [#21](https://github.com/Vesuri/excellence/issues/21)
- [ ] **Stencil draw mode** — Paint the stencil mask directly with any drawing tool. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Fill controls — Conform / Center** — Conform scales fills to the cross-section of the area; Center forces radial fills to originate from its center. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Draw Mode Amount slider** — Controls dither percentage for Dither1, Dither2, Brighten, and Darken. [#24](https://github.com/Vesuri/excellence/issues/24)
- [ ] **Real-time Preview Mode (RPM)** — Live preview before committing for Tint, Colorize, Brighten, Darken, Dither1, Dither2, and Negative. [#23](https://github.com/Vesuri/excellence/issues/23)
- [ ] **Transparent painting mode** — Adjustable opacity (0–100%); no accumulation within a single mouse-down; RGB and HSV mixing options. [#24](https://github.com/Vesuri/excellence/issues/24)

---

## 4. Palette Management

Foreground/background selection, current colors display, copy/swap palette entries, and color remapping (copy-remap and swap-remap) are implemented. Missing:

- [ ] **Eyedropper / Pick Color** — Click anywhere on canvas to pick a color; left sets foreground, right sets background. Keyboard: `,`. [#25](https://github.com/Vesuri/excellence/issues/25)
- [ ] **Palette navigation shortcuts** — `[` / `]` cycle foreground color; `{` / `}` cycle background color. [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Palette scroll** — Scroll arrows when palette has more than 32 colors (currently all colors shown but no scrolling UI). [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Save Palette** — Save current palette to a file. [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Default Palette** — Reset palette to application default. [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Restore Palette** — Undo palette changes back to state when the palette editor was opened. [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Spread** — Interpolate colors between two selected palette wells, filling wells in between. [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Palette Undo** — Single-level toggle undo for palette-only operations. [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Remap — Page** — Remap all canvas pixels to the closest match in the current (modified) palette. [#27](https://github.com/Vesuri/excellence/issues/27)
- [ ] **Color Cube** — Interactive 2-D color spectrum picker; click to select; RGB and CMY axis modes. [#26](https://github.com/Vesuri/excellence/issues/26)
- [ ] **RGB sliders** — Dedicated per-channel R/G/B sliders in the palette editor (currently delegated to QColorDialog). [#26](https://github.com/Vesuri/excellence/issues/26)
- [ ] **HSV sliders** — Dedicated H/S/V sliders in the palette editor. [#26](https://github.com/Vesuri/excellence/issues/26)
- [ ] **CMY sliders** — Cyan, Magenta, Yellow subtractive sliders. [#26](https://github.com/Vesuri/excellence/issues/26)
- [ ] **Color Bar preview** — Strip showing the color being composed before committing it to a palette well. [#26](https://github.com/Vesuri/excellence/issues/26)

---

## 5. Fill System and Gradients

Entirely unimplemented:

- [ ] **Gradient Menu** — Up to 8 independent gradient ranges, each with its own color set. Keyboard: `P`. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Gradient Marker Box** — 128 slots for color markers; supports abrupt (double-tab) transitions. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Gradient Flip** — Reverse color order of the current gradient range. Keyboard: `Alt-r`. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Gradient Clear** — Remove all markers from the current gradient range. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Gradient Undo / Restore** — Single-level undo and restore for gradient edits. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Gradient Spread slider** — Number of interpolated colors per gap between markers. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Gradient Dither — Random / Ordered / Hard Edge** — How color transitions are dithered. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Gradient Dither Amount slider** — Dither spread distance (0 = none, 100% = maximum). [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Color Cycling** — Per-gradient speed slider (0–60 cycles/sec) with up to 8 ranges cycling simultaneously. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Horizontal gradient fill** — Gradient progresses horizontally across the filled area. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Vertical gradient fill** — Gradient progresses vertically through the filled area. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Linear gradient fill** — Gradient follows a user-specified rubber-band direction. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Radial gradient fill** — Concentric rings from a user-placed center. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Spherical gradient fill** — Like Radial but progression accelerates toward edges to simulate a sphere. [#28](https://github.com/Vesuri/excellence/issues/28)
- [ ] **Highlight gradient fill** — Simulates directional light; gradient radiates from a center conforming to object radii. [#28](https://github.com/Vesuri/excellence/issues/28)

---

## 6. View and Navigation

Zoom in/out (2× / 0.5×) is implemented. Missing:

- [ ] **Magnify mode (split-pane)** — Normal view alongside a magnified view; all drawing tools remain active in both panes. Keyboard: `m`. [#29](https://github.com/Vesuri/excellence/issues/29)
- [ ] **Place Magnifier** — Show the zoom selection square for repositioning before opening. Keyboard: `M`. [#29](https://github.com/Vesuri/excellence/issues/29)
- [ ] **31 zoom levels** — Continuous zoom from 2× to 32× with `>` / `<` keys (currently only 2× / 0.5× steps). [#29](https://github.com/Vesuri/excellence/issues/29)
- [ ] **Pixel grid** — Optional black outlines around individual pixels in the magnified view. [#29](https://github.com/Vesuri/excellence/issues/29)
- [ ] **Magnified view scroll** — Arrow keys scroll the magnified region. [#29](https://github.com/Vesuri/excellence/issues/29)
- [ ] **Show Page** — Scale entire canvas to fit the screen preserving aspect ratio; click to reposition. Keyboard: `s`. [#30](https://github.com/Vesuri/excellence/issues/30)
- [ ] **Pan canvas** — Interactive pan tool. Keyboard: `N`. [#30](https://github.com/Vesuri/excellence/issues/30)
- [ ] **Center view on cursor** — Recenters the canvas at the current cursor position. Keyboard: `n`. [#30](https://github.com/Vesuri/excellence/issues/30)
- [ ] **Canvas scroll** — Arrow keys scroll the canvas when the page is larger than the view. [#30](https://github.com/Vesuri/excellence/issues/30)
- [ ] **Toggle menus** — Hide/show all tool panels to maximize canvas area. Keyboard: `Spacebar` / `F10`. [#30](https://github.com/Vesuri/excellence/issues/30)
- [ ] **Squash menus** — Collapse all open sub-panels to the main menu only. Keyboard: `F9`. [#30](https://github.com/Vesuri/excellence/issues/30)
- [ ] **Status / feedback area** — Persistent display showing tool name, draw mode, foreground/background color indices, X/Y coordinates, line angle and length while drawing, and brush rotation degrees. Currently only coordinates appear in the window title. [#31](https://github.com/Vesuri/excellence/issues/31)
- [ ] **Coordinates display mode** — Toggle relative (angle/length) vs. absolute (X/Y) during line drawing. [#31](https://github.com/Vesuri/excellence/issues/31)
- [ ] **Canvas size** — Set canvas width and height independent of window size for a scrollable page larger than the view. [#30](https://github.com/Vesuri/excellence/issues/30)

---

## 7. Stencil System

Entirely unimplemented:

- [ ] **Stencil toggle** — Activate/deactivate stencil mask; protected areas block all painting and CLR. Keyboard: `'`. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Load / Save Stencil** — Persist stencil masks to disk. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Invert Stencil** — Flip protected and paintable areas. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Delete Stencil** — Remove mask from memory. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Make Stencil from Foreground** — Automatically protect all non-background pixels on the canvas. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Make Stencil from Selected Colors — Replace / Add / Subtract** — Build or modify the mask from chosen palette colors. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Stencil color selection — Eyedropper** — Left-click adds a color to the protected set; right-click removes it. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Stencil color selection — Lasso** — Freehand outline to add colors found within to the stencil set. [#32](https://github.com/Vesuri/excellence/issues/32)
- [ ] **Stencil color Invert / Clear / Restore / Undo** — Manipulate the set of protected colors. [#32](https://github.com/Vesuri/excellence/issues/32)

---

## 8. Spare Page / Second Canvas

Entirely unimplemented:

- [ ] **Spare buffer** — A second full-resolution canvas always in memory alongside the working canvas. [#33](https://github.com/Vesuri/excellence/issues/33)
- [ ] **Toggle Work / Spare** — Instantly switch display between working and spare buffers. Keyboard: `J`. [#33](https://github.com/Vesuri/excellence/issues/33)
- [ ] **Copy Spare to Work** — Copy the spare buffer's content into the working canvas. [#33](https://github.com/Vesuri/excellence/issues/33)
- [ ] **Merge — Front** — Layer the spare buffer's foreground over the working canvas foreground. [#33](https://github.com/Vesuri/excellence/issues/33)
- [ ] **Merge — Back** — Layer the spare buffer's foreground beneath the working canvas foreground. [#33](https://github.com/Vesuri/excellence/issues/33)
- [ ] **Multiple buffers** — Create and navigate additional image buffers (keypad `(` / `)` / `+`). [#33](https://github.com/Vesuri/excellence/issues/33)
- [ ] **Fix Background** — Lock the current canvas state as a protected base layer; survives CLR until unlocked. [#33](https://github.com/Vesuri/excellence/issues/33)

---

## 9. File I/O

Load and save are implemented. Missing:

- [ ] **File info** — Display image dimensions and color depth for a highlighted file before loading. [#34](https://github.com/Vesuri/excellence/issues/34)
- [ ] **Export with transparency** — Save PNG with alpha channel where the background color is treated as transparent. [#34](https://github.com/Vesuri/excellence/issues/34)
- [ ] **Print** — Send canvas to printer with density, color correction, shade mode, dither method, layout, and scaling options. [#34](https://github.com/Vesuri/excellence/issues/34)

---

## 10. Preferences and Misc UI

- [ ] **Grid Lock** — Snap all drawing operations to a configurable grid. Keyboard: `g`. [#35](https://github.com/Vesuri/excellence/issues/35)
- [ ] **Grid settings** — Grid spacing (W/H) and offset (X/Y); fit-to-brush shortcut. [#35](https://github.com/Vesuri/excellence/issues/35)
- [ ] **Mirror Draw** — Reflect all brush actions across X axis, Y axis, or both; adjustable midpoint. Keyboard: `/`. [#36](https://github.com/Vesuri/excellence/issues/36)
- [ ] **Segmented Draw** — Place brush stamps at evenly-spaced intervals along lines, curves, or shapes. [#37](https://github.com/Vesuri/excellence/issues/37)
- [ ] **Help area** — On-hover description of any menu item or tool displayed in a dedicated UI region. [#38](https://github.com/Vesuri/excellence/issues/38)
- [ ] **Escape to abort** — Cancel any in-progress operation. Keyboard: `Esc`. [#38](https://github.com/Vesuri/excellence/issues/38)
- [ ] **Quit with confirmation** — Prompt before exiting when there are unsaved changes. Keyboard: `Ctrl-Q`. [#38](https://github.com/Vesuri/excellence/issues/38)
- [ ] **About screen** — Display version and copyright information. [#38](https://github.com/Vesuri/excellence/issues/38)
