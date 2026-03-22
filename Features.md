# Excellence — Prioritized Feature List

A modern reimplementation of Brilliance's painting features, excluding animation, Amiga hardware specifics, and AmigaOS platform dependencies.

---

## 1. Core Drawing Tools

- [ ] **Freehand Draw — Dotted** — Drag to place a trail of dots; dot spacing increases with mouse speed. Keyboard: `d`.
- [ ] **Freehand Draw — Connected** — Drag to paint a continuous solid line following the mouse path. Keyboard: `s`.
- [ ] **Freehand Draw — Filled Shape** — Draw a freehand outline; release to flood-fill the enclosed area. Open endpoints are closed with a straight line. Keyboard: `D`.
- [ ] **Straight Line** — Click-drag with rubber-band preview; commits on release. Keyboard: `v`.
- [ ] **Connected Lines** — Each click continues from the previous endpoint; right-click or double-click to end. Keyboard: `w`.
- [ ] **Filled Polygon** — Like Connected Lines but fills the enclosed area on termination. Keyboard: `W`.
- [ ] **Elliptical Curve** — Single-control-point curve: set two endpoints, then drag a midpoint to adjust curvature, click to commit. Keyboard: `q`.
- [ ] **Bezier Curve** — Four-point Bezier: place two anchors and two control points; all remain draggable until right-click commits. Keyboard: `Q`.
- [ ] **Outline Rectangle** — Click-drag to draw an unfilled rectangle. Hold Ctrl for perfect square. Keyboard: `r`.
- [ ] **Filled Rectangle** — Same as Outline Rectangle but fills with the current color/fill mode. Keyboard: `R`.
- [ ] **Rectangle — Corner to Corner / Center to Corner modes** — Sub-menu option to switch the anchor point convention.
- [ ] **Outline Ellipse** — Click-drag to draw an unfilled ellipse. Hold Ctrl for perfect circle. Keyboard: `c`.
- [ ] **Filled Ellipse** — Same as Outline Ellipse but fills. Keyboard: `C`.
- [ ] **Ellipse — Center to Corner / Corner to Corner / Rotate modes** — Sub-menu options including free rotation before commit.
- [ ] **Airbrush — Fine Spray** — Sprays individual single-pixel droplets. Keyboard: `a`.
- [ ] **Airbrush — Splatter** — Uses the current pen tip or brush shape as the spray droplet.
- [ ] **Airbrush — Shape Airbrush** — Uses the non-background area of the current brush as a mask through which paint gradually builds up.
- [ ] **Airbrush nozzle controls** — Interactive nozzle size adjustment, Flow slider (0–100%), and Focus slider (concentrated vs. diffused spread). Keyboard: `A`.
- [ ] **Fill Tool** — Flood-fill contiguous same-color pixels. Left button fills with foreground; right button fills with background. Fill type determined by active Draw Mode. Keyboard: `f`.
- [ ] **Text Tool** — Opens a text requester; select font, size (bitmap and scalable), and styles (bold, italic, underline); convert text to a brush attached to the cursor for placement. Keyboard: `t`.
- [ ] **Erase / CLR** — Clear entire canvas: left-click clears with color zero, right-click clears with background color. Keyboard: `K` / `Shift-K`.
- [ ] **Undo** — Multi-level undo limited only by available RAM. Keyboard: `u`.
- [ ] **Redo** — Restores previously undone changes. Keyboard: `U`.
- [ ] **Undo All / Redo All** — Jump to the beginning or end of the undo buffer. Keyboard: `Alt-u` / `Alt-U`.
- [ ] **Clear Undo Buffer** — Releases undo history RAM. Keyboard: `Ctrl-u`.

---

## 2. Brush System

- [ ] **Cut Brush (Rectangular)** — Crosshair guides; drag a rectangle to capture a region as a brush (non-destructive copy). Keyboard: `b`.
- [ ] **Carve Brush (Freehand)** — Hold and draw any outline; irregular region is extracted as a brush.
- [ ] **Load Brush** — File requester to load a saved brush image.
- [ ] **Save Brush** — Save the current brush to disk.
- [ ] **Copy Brush to Clipboard / Paste Brush from Clipboard** — System clipboard integration for brush transfer.
- [ ] **Delete Brush** — Remove the current brush from memory.
- [ ] **8 Brush Wells** — Eight temporary storage slots; click to store, click again to retrieve.
- [ ] **Brush Handle — Corner / Center / Custom** — Position the brush's anchor point at any of the four corners, the center, or an arbitrary location (even outside the brush bounds).
- [ ] **Brush Remap** — Remap brush colors to the closest matches in the current working palette.
- [ ] **BG → FG swap** — Replace background-colored pixels in the brush with the foreground color; also swap BG↔FG.
- [ ] **Auto Background detection** — Sample four brush corners; if all are the same color treat it as transparent background automatically.
- [ ] **Tile Cut** — Cut brush one pixel shorter on right and bottom so tiled stamps fit without overlap (for use with grid).
- [ ] **Brush Align** — Visually reposition a brush relative to its original canvas location; useful for pattern fills needing a specific start point.
- [ ] **Brush Dimensions display** — Real-time readout of current brush width and height in pixels.
- [ ] **Brush resize — Adjust (interactive)** — Free drag to resize proportionally or per-axis. Keyboard: `Z`.
- [ ] **Brush resize — Double / Halve** — Double or halve both dimensions, width only, or height only. Keyboard: `H` / `h` / `X` / `Y`.
- [ ] **Brush resize — 1px / 10px increments** — Keyboard nudge keys: `-` / `=` / `Shift--` / `Shift-=`.
- [ ] **Brush Flip Horizontal / Vertical** — Mirror the brush on either axis. Keyboard: `x` / `y`.
- [ ] **Brush Rotate — Numeric** — Enter rotation in degrees; aspect-corrected.
- [ ] **Brush Rotate — Interactive** — Free drag clockwise/counter-clockwise.
- [ ] **Brush Rotate — Fast 90°** — Instant 90-degree non-aspect-corrected rotation. Keyboard: `z`.
- [ ] **Brush Restore** — Return to the last undistorted brush state. Keyboard: `Shift-B`.
- [ ] **Brush Bend X / Y** — Curve the brush horizontally or vertically with interactive drag.
- [ ] **Brush Shear X / Y** — Slant the brush horizontally or vertically with interactive drag.
- [ ] **Brush Outline** — Add a 1-pixel border in the current foreground color at background-to-color boundaries. Keyboard: `o`.
- [ ] **Brush Trim** — Remove a 1-pixel boundary from the brush wherever background meets another color. Keyboard: `O`.
- [ ] **Pen Tip Tool** — System-defined single-color brushes of various sizes and shapes, including user-sizable circular and rectangular tips. Keyboard: `.` (single pixel).
- [ ] **Anti-Aliasing for brush transforms** — Three quality levels (Low, Medium, High) applied during resize, rotation, and distortion.

---

## 3. Draw Modes

- [ ] **Color mode** — Default: applies the current color at full strength using the brush shape as a mask. Keyboard: `F2`.
- [ ] **Brush mode** — Paints with the current brush's own colors and shape; can stamp or drag. Keyboard: `F1`.
- [ ] **Replace mode** — All colors in the brush are treated as foreground (no transparency). Keyboard: `F3`.
- [ ] **Smear mode** — Moves pixels in the direction of brush travel, mixing them with underlying pixels; amount proportional to brush size. Keyboard: `F4`.
- [ ] **Smooth mode** — Softens color transitions by finding intermediate colors between adjacent pixels. Keyboard: `F8`.
- [ ] **Range mode** — Increments (left button) or decrements (right button) each pixel under the brush one step along the current gradient range. Keyboard: `F5`.
- [ ] **Average Smear mode** — Like Smear but works only with colors in the current gradient range; averages picked-up pixels with underlying gradient colors. Keyboard: `F6`.
- [ ] **Cycle mode** — Paints with colors cycling sequentially through the current gradient; left button cycles forward, right button backward. Keyboard: `F7`.
- [ ] **Random mode** — Like Cycle but color order is randomized; applies to Fine Spray and Shape Airbrush.
- [ ] **Tint mode** — Alters the chroma of painted areas to the current color; effective for colorizing greyscale images.
- [ ] **Colorize mode** — Changes the hue of areas that already contain chroma information; no effect on neutrals.
- [ ] **Brighten mode** — Advances each underlying pixel to the next brighter shade in its hue range.
- [ ] **Darken mode** — Advances each underlying pixel to the next darker shade in its hue range.
- [ ] **Mix mode** — Mixes on-screen pixel colors with the current foreground color.
- [ ] **Negative mode** — Inverts the colors being painted over (CMY complement).
- [ ] **Dither1 mode** — Paints a dithered pattern of the current color and transparent space; Amount slider controls ratio.
- [ ] **Dither2 mode** — Paints a dithered pattern of the current color and background color; Amount slider controls ratio.
- [ ] **Brush fill modes — Stretch / Pattern / Shape / Perspective** — Tile or warp the brush to fill areas: Stretch warps along Y axis, Pattern tiles unmodified, Shape warps both axes, Perspective tiles with perspective lines.
- [ ] **Stencil draw mode** — Paint the stencil mask directly with any drawing tool; left button adds to mask, right button removes. Keyboard: `~`.
- [ ] **Fill controls — Conform / Center** — Conform scales fills to the cross-section of the filled area; Center forces radial fills to originate from the area's center.
- [ ] **Draw Mode Amount slider** — Controls dither percentage for Dither1, Dither2, Brighten, and Darken modes.
- [ ] **Real-time Preview Mode (RPM)** — Live preview before committing for Tint, Colorize, Brighten, Darken, Dither1, Dither2, and Negative modes.
- [ ] **Transparent painting mode** — Paint at adjustable opacity (0–100%); within a single mouse-down no accumulation occurs; layering adds on re-press. RGB and HSV mixing options. Keyboard: `Alt-I` / `Ctrl-t`.

---

## 4. Palette Management

- [ ] **Foreground / Background color selection** — Left-click a color well to set foreground; right-click to set background.
- [ ] **Eyedropper / Pick Color** — Pick any on-canvas or palette color; left-click sets foreground, right-click sets background. Keyboard: `,`.
- [ ] **Palette navigation shortcuts** — Cycle through palette colors: `[` / `]` (foreground), `{` / `}` (background).
- [ ] **Current Colors area** — Always-visible display showing the active foreground and background colors.
- [ ] **Palette display area** — Scrollable grid of all palette color wells; scroll arrows when palette has more than 32 colors.
- [ ] **Load Palette** — Load a saved palette file, replacing the current one.
- [ ] **Save Palette** — Save the current palette to disk.
- [ ] **Default Palette** — Reset palette to application default colors.
- [ ] **Restore Palette** — Undo palette changes back to the state when the menu was opened.
- [ ] **Copy palette entry** — Copy one color well to another within the palette.
- [ ] **Swap palette entries** — Exchange the contents of two color wells.
- [ ] **Spread** — Generate smooth interpolated colors between two selected end colors, filling the wells in between.
- [ ] **Palette Undo** — Single-level toggle undo for palette operations.
- [ ] **Remap — Page** — Remap all on-screen pixels to the closest match in the modified palette.
- [ ] **Remap — Copy** — Globally remap all instances of one on-screen color to a different palette color.
- [ ] **Remap — Swap** — Exchange two palette colors and remap the image to reflect the change.
- [ ] **Color Cube** — Interactive 2-D spectrum display; click to select a color; supports RGB and CMY color model axes.
- [ ] **RGB sliders** — Per-channel Red, Green, Blue numeric sliders for precise color editing.
- [ ] **HSV sliders** — Hue (0–360°), Saturation, and Value sliders for intuitive color editing.
- [ ] **CMY sliders** — Cyan, Magenta, Yellow subtractive sliders.
- [ ] **Color Bar preview** — Strip showing the color currently being composed before applying it to a well.

---

## 5. Fill System and Gradients

- [ ] **Gradient Menu** — Up to 8 independent gradient ranges, each with its own color set. Keyboard: `P`.
- [ ] **Gradient Marker Box** — 128 slots for color markers; place palette colors at positions to define the gradient sequence; supports abrupt (double-tab) transitions.
- [ ] **Gradient Flip** — Reverse the color order of the current gradient range. Keyboard: `Alt-r`.
- [ ] **Gradient Clear** — Remove all markers from the current gradient range.
- [ ] **Gradient Undo / Restore** — Single-level undo and restore-to-opened-state for gradient edits.
- [ ] **Gradient Spread slider** — Set the number of interpolated in-between colors per gap between markers (up to 254, enabling gradients of 32,000+ colors).
- [ ] **Gradient Dither — Random / Ordered / Hard Edge** — Control how color transitions are dithered across the gradient.
- [ ] **Gradient Dither Amount slider** — Set the dither spread distance (0 = none, 100% = maximum).
- [ ] **Color Cycling** — Per-gradient toggle and speed slider (0–60 cycles/sec); up to 8 independent ranges cycling simultaneously. Toggle all: `Tab`. Next/previous gradient: `Alt-j` / `Alt-J`.
- [ ] **Horizontal gradient fill** — Gradient progresses horizontally across the filled area.
- [ ] **Vertical gradient fill** — Gradient progresses vertically through the filled area.
- [ ] **Linear gradient fill** — Gradient follows a user-specified direction defined by a rubber-band line.
- [ ] **Radial gradient fill** — Concentric rings radiating from a user-placed center point.
- [ ] **Spherical gradient fill** — Like Radial but progression accelerates toward the outer edges to simulate a 3-D sphere.
- [ ] **Highlight gradient fill** — Simulates directional light; gradient radiates from a placed center conforming to the object's radii.

---

## 6. View and Navigation

- [ ] **Magnify mode** — Opens a split-pane zoom window (normal view left, magnified view right); all drawing tools remain active. Keyboard: `m`.
- [ ] **Place Magnifier** — Show the zoom selection square for repositioning before opening. Keyboard: `M`.
- [ ] **31 zoom levels** — From 2× to 32×; increase/decrease with `>` / `<`.
- [ ] **Pixel grid** — Optional black outlines around individual pixels in the magnified view; toggle in the drag bar.
- [ ] **Magnified view scroll** — Arrow keys scroll the magnified region.
- [ ] **Show Page** — Scale the entire canvas to fit one screen while preserving aspect ratio; click to reposition view. Keyboard: `s`.
- [ ] **Pan canvas** — Interactive pan of the canvas page. Keyboard: `N`.
- [ ] **Center view on cursor** — Recenters the screen at the current mouse position. Keyboard: `n`.
- [ ] **Canvas scroll** — Arrow keys scroll the canvas when the page is larger than the visible area.
- [ ] **Toggle menus** — Hide/show all tool panels to maximize the canvas area. Keyboard: `Spacebar` / `F10`.
- [ ] **Squash menus** — Collapse all open sub-panels to the main menu only. Keyboard: `F9`.
- [ ] **Status / feedback area** — Persistent display showing current tool name, draw mode, foreground/background colors, X/Y cursor coordinates, line angle and length while drawing, brush rotation degrees, and RGB values when hovering over palette.
- [ ] **Coordinates display mode** — Preference to show relative (angle/length) vs. absolute (X/Y) coordinates during line drawing.
- [ ] **Canvas size** — Set custom canvas width and height independent of the window/screen size for a scrollable page larger than the view.

---

## 7. Stencil System

- [ ] **Stencil toggle** — Activate/deactivate the current stencil mask; protected areas block all painting and CLR. Keyboard: `'`.
- [ ] **Load Stencil** — Load a saved stencil mask file.
- [ ] **Save Stencil** — Save the current stencil mask to disk.
- [ ] **Invert Stencil** — Flip the mask so previously protected areas become paintable and vice versa.
- [ ] **Delete Stencil** — Remove the stencil mask from memory.
- [ ] **Make Stencil from Foreground** — Automatically protect all non-background pixels currently on the canvas.
- [ ] **Make Stencil from Selected Colors — Replace / Add / Subtract** — Build or modify the mask using colors chosen from the palette, in Replace, Add, or Subtract mode.
- [ ] **Stencil color selection — Eyedropper** — Left-click adds a color to the protected set; right-click removes it.
- [ ] **Stencil color selection — Lasso** — Draw a freehand outline; colors found within are added to the stencil color set.
- [ ] **Stencil color Invert / Clear / Restore / Undo** — Swap selected/unselected color sets, clear all selections, restore to initial state, or undo the last stencil color change.
- [ ] **Stencil color range (HSV tolerance)** — For true-color workflows: define a central color and H/S/V variance sliders to incorporate a range of colors into the mask.

---

## 8. Spare Page / Second Canvas

- [ ] **Spare buffer** — A second full-resolution canvas always kept in memory alongside the working canvas.
- [ ] **Toggle Work / Spare** — Instantly switch the display between the working buffer and the spare buffer. Keyboard: `J`.
- [ ] **Copy Spare to Work** — Copy the spare buffer's content into the working canvas.
- [ ] **Mark current as Spare** — Designate the currently displayed buffer as the spare.
- [ ] **Go to Spare** — Switch display to the spare buffer.
- [ ] **Merge — Front** — Layer the foreground of the spare buffer over the foreground of the working buffer.
- [ ] **Merge — Back** — Layer the foreground of the spare buffer beneath the foreground of the working buffer.
- [ ] **Multiple buffers** — Create and navigate additional image buffers (limited only by available RAM); previous/next navigation via keyboard. Keyboard: keypad `(` / `)` / `+`.
- [ ] **Fix Background** — Lock the current canvas state as a protected base layer; subsequent painting covers the lock but it survives CLR; click again to unlock.

---

## 9. File I/O

- [ ] **Load Image** — File requester to load images in common formats (PNG, BMP, TIFF, JPEG, IFF/LBM). Keyboard: `Ctrl-L`.
- [ ] **Save Image** — Save the current canvas in a chosen format. Keyboard: `Ctrl-S`.
- [ ] **File info** — Display image dimensions and color depth for a highlighted file before loading.
- [ ] **Save with custom canvas/page size** — Preserve the canvas dimensions as set in Screen/Canvas settings.
- [ ] **Export with transparency** — Save with alpha channel where the background color is treated as transparent (PNG).
- [ ] **Print** — Send the canvas to a printer with density, color correction, shade mode (B&W / greyscale / color), dither method (Ordered, Halftone, Floyd-Steinberg), layout (landscape/portrait, center, mirror, negative), and scaling options.

---

## 10. Preferences and Misc UI

- [ ] **Undo Buffer size preference** — Configure the maximum amount of RAM dedicated to undo history.
- [ ] **Grid Lock** — Snap all drawing operations to a configurable grid. Keyboard: `g`.
- [ ] **Grid settings** — Interactive or numeric entry for grid spacing (W/H) and offset (X/Y); fit-to-brush shortcut.
- [ ] **Mirror Draw** — Reflect all brush actions across the X axis, Y axis, or both simultaneously; adjustable midpoint. Keyboard: `/`.
- [ ] **Segmented Draw** — Place brush stamps at evenly-spaced intervals along any line, curve, or shape; configurable by pixel distance or fixed point count.
- [ ] **Fast Feedback mode** — Performance preview: render computation-intensive operations as a 1-pixel path + outline instead of full quality; also simplifies brush distortion previews for low-resource conditions.
- [ ] **Help area** — On-hover description of any menu item or tool displayed in a dedicated UI region.
- [ ] **Sub-menu panels** — Context-specific option panels opened by right-clicking a tool; stack below the main menu; closeable and reorderable.
- [ ] **Menu stack hotkeys** — Save and recall up to 9 custom panel configurations via numeric keypad keys.
- [ ] **Escape to abort** — Cancel any in-progress operation. Keyboard: `Esc`.
- [ ] **Toggle mouse cursor** — Hide/show the mouse cursor for clean screenshots. Keyboard: `Del`.
- [ ] **About screen** — Display version and copyright information.
- [ ] **Quit with confirmation** — Prompt before exiting to prevent accidental data loss. Keyboard: `Ctrl-Q`.
