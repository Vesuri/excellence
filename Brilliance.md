# Brilliance (Amiga Paint & Animation) — Complete Feature Inventory

Based on the Brilliance Version 1.0 User's Guide (Digital Creations, 1993).

---

## 1. Program Variants

- **Brilliance (Register mode)** — Standard bitmap paint/animation supporting 2, 4, 8, 16, 32, 64, 128, and 256 colors. Supports all Amiga register-based graphic modes.
- **TrueBrilliance (HAM mode)** — HAM/True Color version supporting millions of colors via HAM6 (4,096 colors) and HAM8 (262,144 colors on AGA). Can save 15-bit or 24-bit True Color files.
- Both programs share a nearly identical interface; differences are noted per feature where they exist.

---

## 2. User Interface & Menu System

### 2.1 Main Menu Panel
- Stacking panel-based menu system (not pull-down menus). Menus stack vertically on screen.
- **Expand Panel gadget** — Expands the palette area to show all colors when using EHB or AGA modes with more colors than fit in the default palette display.
- **Palette area** — Displays all colors of the current palette as clickable color wells.
- **Scroll Palette gadgets** — Up/down arrows scroll the displayed palette rows (active when palette has more than 32 colors).
- **Current Colors area** — Shows the two currently active colors: foreground (top, used with left mouse button) and background (bottom, used with right mouse button).
- **Squash Menu gadget** — Collapses all sub-menus except the main menu. Toggle with F9.
- **Screen Depth gadget** — Cycles Brilliance between front and back of the Amiga display stack. Also appears on all requesters.
- **User Feedback area** — Real-time display panel showing: current animation frame number, current menu item or tool name, current draw mode, RGB values (when over palette), X/Y cursor coordinates, line angle/length (when drawing lines), brush rotation degrees, and operation countdown.
- **Help area (blue rectangle)** — Displays a description of whatever menu item the pointer is hovering over.
- **CLR tool** — Clears the current image buffer. Left-click clears with color zero; right-click clears with background color. In animation mode, right-click brings up a Clear Frames requester (all frames, specific frames, or current frame). Keyboard shortcut: `K` / `Shift-K`.

### 2.2 Sub-Menus
- Each drawing tool has its own sub-menu panel accessed by right-clicking the tool.
- Sub-menus stack below the main menu.
- **Close sub-menu** — Click the icon area at the far left of the sub-menu panel.
- **Move sub-menu to top of stack** — Right-click the icon area at the far left.
- Unavailable options are shown "ghosted" in dark grey.

### 2.3 Menu Hotkeys (Custom Configurations)
- Up to 9 custom menu stack configurations can be saved and recalled.
- **Save configuration** — Hold Shift and press a numeric keypad key (1–9).
- **Recall configuration** — Press numeric keypad key (1–9).
- Saved in Preferences. Not available on Amiga A600 (no numeric keypad).
- **9 pre-configured default hotkeys:**
  1. Color Setup (Gradient Menu + Palette Menu)
  2. Morph Setup (Anim-Brush Menu + Brush Menu)
  3. Path of Motion Setup (Anti Alias Menu + Animation Menu + Tweening Menu)
  4. Painting Setup (Pen Tips Menu + Transparency Menu + Draw Modes Menu)
  5. Drawing Setup (Pen Tips + Transparency + Grid + Segment + Mirror menus)
  6. Airbrush Setup (Airbrush Menu + Transparency Menu + Draw Modes Menu)
  7. Brush Setup (Brush Distort Menu + Anti Alias Menu + Brush Menu)
  8. Stencil Setup (Stencil Menu + Draw Modes Menu)
  9. Geometric Draw Setup (Draw Modes + Rectangle + Ellipse + Grid menus)

### 2.4 Menu Toggle
- **Spacebar / F10** — Toggles all menus (including main menu) on/off.
- **Middle mouse button** (three-button mouse) — Also toggles menus on/off.
- **F9 / Squash Menu gadget** — Toggles between showing all open menus vs. main menu only.

---

## 3. Drawing Tools

### 3.1 Freehand Draw Tool
Three modes (cycle with left-click on tool):
- **Dotted Freehand Draw** (`d`) — Default mode. Places a trail of dots as you drag; dot spacing increases with mouse speed.
- **Connected Freehand Draw** (`s`) — Draws a continuous solid line following the mouse path.
- **Filled Freehand Shape** (`D`) — Draws a freehand outline and fills the enclosed area with the current fill pattern when the mouse button is released. If endpoints are not connected, a straight line closes the shape.

### 3.2 Line Draw Tool
Three modes (cycle with left-click):
- **Straight Line** (`v`) — Click and drag to draw a single line with rubber-band preview. Line is drawn on release.
- **Connected Lines** (`w`) — Each click starts a new segment from the previous endpoint. Right-click or double-click to end the series.
- **Filled Lines / Filled Polygon** (`W`) — Like Connected Lines but fills the enclosed area on termination (right-click or double-click).
- **Constraint** — Hold Ctrl to constrain to specific angles (not explicitly stated but standard behavior implied).

### 3.3 Curved Line Tool
Two modes (cycle with left-click):
- **Elliptical Curve** (`q`) — Single-control-point curve. Click-drag to set endpoints; release, then drag the attached midpoint to adjust curvature; click to commit.
- **Bezier Curve** (`Q`) — Four-point Bezier curve. Click to set: (1) first anchor, (2) first control point, (3) second control point, (4) second anchor. All points remain "live" and draggable until right-click commits the curve.

### 3.4 Rectangle Tool
Two modes (cycle with left-click):
- **Outline Rectangle** (`r`) — Click-drag to define bounding corners; draws an unfilled rectangle outline.
- **Filled Rectangle** (`R`) — Same operation but fills the rectangle with the current color/fill pattern.
- **Constraint** — Hold Ctrl to draw a perfect square.
- **Rectangle Menu** (right-click): 
  - *Corner to Corner* (default) — Initial click sets one corner; drag to opposite corner.
  - *Center to Corner* — Initial click sets center; drag defines the extent.

### 3.5 Ellipse Tool
Two modes (cycle with left-click):
- **Outline Ellipse** (`c`) — Click-drag to draw an unfilled ellipse.
- **Filled Ellipse** (`C`) — Same but fills the ellipse.
- **Constraint** — Hold Ctrl to draw a perfect circle.
- **Ellipse Menu** (right-click):
  - *Center to Corner* (default) — Click sets center; drag sets outermost extent.
  - *Corner to Corner* — Click sets one bounding-box corner; drag sets the opposite.
  - *Rotate* — After drawing the ellipse shape, hold a mouse button to rotate it freely about its center before releasing to commit.

### 3.6 Airbrush Tool (`a`)
Three modes (cycle with left-click):
- **Fine Spray** — Default mode. Sprays individual single-pixel droplets.
- **Splatter** — Uses the current pen tip or brush shape to form the individual droplets of spray.
- **Shape Airbrush** — Uses the non-background areas of the current brush as a mask/stencil through which paint is sprayed; paint gradually builds up in the brush's shape.
- **Airbrush Menu** (right-click):
  - *Size Nozzle* (`A`) — Interactive gadget: drag a circle toward/away from center to decrease/increase nozzle diameter.
  - *Flow slider* — Controls spray rate (0%–100% of maximum).
  - *Focus slider* — Controls spray diffusion (0% = concentrated center; 100% = fully diffused).
- Paint direction: left mouse button uses foreground color; right mouse button uses background color.

### 3.7 Fill Tool (`f`)
- Flood-fill: fills all contiguous pixels of the same color within a bounded area.
- "Hot spot" is the tip of the paint drop graphic.
- Left-click fills with foreground color / current fill mode.
- Right-click fills with background color.
- Fill type (solid color, gradient, brush pattern, etc.) is determined by the Draw Mode menu.
- A gap of even one pixel allows fill to leak through a boundary.

### 3.8 Text Tool (`t`)
- Opens a **Text Requester**:
  - *Text string gadget* — Type the text to be placed.
  - *Font Requester* — Browse and select fonts from the Fonts directory. Displays available sizes.
  - *Font size selector* — Choose from listed bitmap sizes; Compugraphic (outline) fonts support any size.
  - *Text styles* — Bold, Italic, Underline (combinable).
  - *Show button* — Preview the text on-screen in the selected font/size.
  - *Reload button* — Re-reads the Fonts directory (supports reassigning or adding Font directories via shell).
  - *Kerning support* — Special < and > characters in Kara Fonts provide positive/negative kerning.
- Text is converted to a brush and attached to the cursor for placement.

---

## 4. Brush System

### 4.1 Brush Tool (Cut/Carve)
- **Cut Brush** (`b`) — Displays cross-hair guides. First click sets one corner; drag cross-hair to opposite corner; release to capture the rectangular region as a brush (copies, does not remove).
- **Carve Brush** — Freehand-outline mode. Hold left mouse button and draw any shape. An irregular brush is extracted from the enclosed area. If not fully closed, a straight line closes the shape.
- After cutting, the brush follows the cursor as the active drawing tool; Brilliance automatically activates Freehand Draw mode.

### 4.2 Brush Menu (right-click on Brush tool)
- **Load Brush** (`Right-Amiga-I`) — Opens file requester to load a saved brush file.
- **Save Brush** (`Right-Amiga-E`) — Saves the current brush to disk.
- **Copy to Clipboard** (`Right-Amiga-C`) — Copies the current brush to the Amiga system clipboard.
- **Copy from Clipboard** (`Right-Amiga-V`) — Pastes a brush from the Amiga clipboard onto the pointer.
- **Delete Brush** — Removes the current brush from memory.
- **Brush Wells (×8)** — Eight parking slots for temporary brush storage. Click a well to store the current brush there (displacing any existing brush); click again to retrieve. Brushes with a bent corner indicator are anim-brushes. Wells accept both standard brushes and anim-brushes.
- **Handle positioning:**
  - *Corner gadgets* (4) — Moves the handle to one of the four corners.
  - *Center* — Centers the handle.
  - *Adjust* (`Alt-z`) — Press right mouse button and drag to place handle anywhere (even outside the brush).
  - Keyboard shortcuts: `Alt-s` (center), `Alt-x` (flip handle H), `Alt-y` (flip handle V).
- **Palette button** — Copies the brush's palette into the working palette and opens the Palette menu. Not available in TrueBrilliance.
- **Remap button** — Remaps brush colors to the closest matches in the current palette. In HAM mode, remaps to base colors.
- **BG→FG** — Replaces background-colored pixels in the brush with the foreground color.
- **BG↔FG** — Swaps background-colored and foreground-colored pixels in the brush.
- **BG→TC** — Reassigns the background color in the brush as the transparent color.
- **Brush Dimensions display** — Shows current brush width and height in pixels. Also displays pen tip size.
- **Auto BG (Auto Background)** — When enabled, samples the four corner pixels when cutting; if all four are the same color, that color is treated as background (transparent). Useful for cutting brushes with irregular or non-black backgrounds.
- **Tile Cut** — When Grid Mode is active, cuts the brush one pixel shorter on right and bottom so tiled stamps fit without overlap.
- **Align** — Allows visual repositioning of a brush relative to its original canvas position; useful for pattern fills needing a specific start point.

### 4.3 Brush Distort Menu
Accessed via the Brush Distort gadget. The **Restore** button returns to the last undistorted state (`Shift-B`).

**Size:**
- *Adjust* (`Z`) — Free interactive resize by dragging; proportional when dragging diagonally.
- *Double* (`H`) — Doubles both width and height.
- *Double X* (`X`) — Doubles width only.
- *Double Y* (`Y`) — Doubles height only.
- *Halve* (`h`) — Halves both width and height.
- *Halve X* — Halves width only.
- *Halve Y* — Halves height only.
- Keyboard increments: `-` (reduce 1px), `=` (increase 1px), `Shift--` (reduce 10px), `Shift-=` (increase 10px).
- Size operations respect Anti-Aliasing if enabled.

**Flip:**
- *Flip X* (`x`) — Mirrors brush left-to-right.
- *Flip Y* (`y`) — Flips brush top-to-bottom.

**Bend and Shear:**
- *Bend X* — Curves the brush horizontally (top/bottom fixed; sides curve). Interactive drag.
- *Bend Y* — Curves the brush vertically (left/right fixed; top/bottom curve). Interactive drag.
- *Shear X* — Slants the brush horizontally; bottom follows cursor while top stays anchored.
- *Shear Y* — Slants the brush vertically; right side follows cursor while left stays anchored.
- Bend operations use "magnetic" cursor behavior (attracts nearest edge or corner).

**Rotation:**
- *Rotate* — Numeric input (degrees, clockwise only, 0–360+). Aspect-corrected.
- *Adjust* — Free interactive rotation by dragging clockwise/counter-clockwise.
- *Fast90* (`z`) — Quick 90-degree rotation, non-aspect-corrected.

**Edge:**
- *Outline* (`o`) — Adds a 1-pixel border in the current foreground color wherever the background color touches another color.
- *Trim* (`O`) — Removes a 1-pixel boundary from the brush wherever background color touches any other color.

---

## 5. Draw Modes

Accessed via the **Draw Mode Menu** (`F`, right-click on Draw Mode tool). The Draw Mode tool (`left-click`) toggles between the selected draw mode and standard (Color/Brush) mode. Three classes of tools each maintain their own last-used draw mode independently.

### 5.1 Brush and Brush Fill Modes
- **Brush** (`F1`) — Paints with the current brush's colors and shape. Can stamp or drag.
- **Stretch** — Stretches the brush to fit the fill area along the Y axis; horizontal lines remain horizontal but verticals/diagonals are warped.
- **Pattern** — Tiles the brush repeatedly without modification to fill an area; truncated if brush is larger than area.
- **Shape** — Like Stretch, but warps the brush both horizontally and vertically to conform to the fill area.
- **Perspective** — Tiles the brush conforming to the current perspective settings (lines of perspective).
- **Replace** (`F3`) — All colors in the current brush are treated as foreground (no transparent areas).

### 5.2 Color/Drawing Operators
- **Color** (`F2`) — Default mode. Applies the current color at full strength (subject to opacity) using the brush shape as a mask.
- **Tint** — Alters the chroma (color) of non-black areas being painted with the current color. Best with large palettes. Used for colorizing greyscale images.
- **Colorize** — Changes the color content of any on-screen area containing chroma information. No effect on black, greys, or whites.
- **Brighten** — Advances each underlying color to the next brighter shade in its hue range.
- **Darken** — Advances each underlying color to the next darker shade in its hue range.
- **Stencil** (`~` tilde) — Paint applied creates a stencil mask (visible while active). Left mouse button adds to mask; right mouse button removes from mask. Switching away from Stencil mode hides but retains the mask.
- **Mix** — Mixes on-screen pixel colors with the current color.
- **Smooth** (`F8`) — Softens color transitions by finding intermediate colors between adjacent pixels.
- **Smear** (`F4`) — Moves pixels in the direction of brush travel, mixing them with underlying pixels. Amount proportional to brush size.
- **Average Smear** (`F6`) — Like Smear but works only with colors in the current gradient range. Averages "picked up" pixels with underlying ones from the gradient range.
- **Range** (`F5`) — Increments (left button) or decrements (right button) each pixel under the brush one step along the current gradient range. Unaffected if color is not in the gradient.
- **Cycle** (`F7`) — Paints with colors cycling sequentially through the current gradient. Left button cycles forward; right button cycles backward. Applies to non-filled drawing tools and Splatter Airbrush.
- **Random** — Like Cycle but color order is randomized. Applies only to Fine Spray and Shape Airbrush.
- **Dither1** — Paints in a dithered pattern of the current color and transparent spaces. The Amount slider controls the ratio (0–100%).
- **Dither2** — Paints in a dithered pattern of the current color and the background color. Amount slider controls ratio.
- **Negative** — Inverts the colors being painted over (opposite in CMY terms).
- **Halfbrite** — (EHB mode only) Converts normal register colors to their halfbrite counterparts (left button) or vice versa (right button).
- **Not** — Performs a bitwise NOT on the register number of each pixel under the brush. (Register mode only; not available in HAM.)

### 5.3 Gradient Fill Modes
All gradient fills use the colors defined in the current gradient range:
- **Horizontal** — Gradient progresses horizontally (vertical color bands).
- **Vertical** — Gradient progresses vertically (horizontal color bands).
- **Linear** — Gradient follows a user-specified direction. After drawing the shape, a rubber-band line from center to cursor defines the angle; right-click to commit.
- **Highlight** — Simulates light hitting the object; gradient radiates from a user-placed center point conforming to the object's radii. Always conforms to shape.
- **Spherical** — Like Radial but progression accelerates toward the outer edges to simulate a 3-D sphere.
- **Radial** — Concentric rings of gradient color radiating from a user-placed center point.

### 5.4 Fill Controls
- **Conform** — Scales pattern and gradient fills to conform to cross-sections of the filled area (horizontal, vertical, diagonal, or area cross-sections depending on fill type).
- **Center** — Forces radial/spherical/highlight fills to always originate from the center of the filled area.
- **Amount slider** — Controls the dither percentage for Dither1, Dither2, Brighten, and Darken modes.

### 5.5 Real-time Preview Mode (RPM)
- Available for Tint, Colorize, Brighten, Darken, Dither1, Dither2, and Negative.
- Moving the brush over an area previews the result before committing.

---

## 6. Pen Tip Tool

- **Pen Tip Tool** (`.` period — selects single-pixel pen tip) — System-defined brushes. Paint in a single color only (unlike user brushes). When a pen tip is active, the tool icon reflects the chosen tip.
- **Pen Tip Menu** (right-click):
  - Multiple predefined pen tips of varying shapes and sizes.
  - *User-sizable circular pen tip* — Select then click-drag to define the diameter.
  - *User-sizable rectangular pen tip* — Select then click-drag to define the size.
  - Double-clicking on any pen tip in the menu selects it and closes the menu.
- De-selecting the pen tip re-activates the most recent user-defined brush.

---

## 7. Color and Palette

### 7.1 Color Selection
- **Foreground color** — Left-click on a color well in the palette area.
- **Background color** — Right-click on a color well.
- **Eyedropper / Pick Color** (`,` comma) — Click on the Current Colors area to activate the eyedropper. Pick any color from the canvas, palette area, or Palette menu. Left-click picks foreground; right-click picks background.
- **Palette navigation shortcuts:** `[` / `]` (previous/next foreground color), `{` / `}` (previous/next background color).

### 7.2 Palette Menu (`p`)
The Palette menu is divided into: Operations, Color Screen (Color Cube), and Color Sliders.

**Operations:**
- **Load Palette** — Loads a previously saved palette, displacing the current one.
- **Save Palette** — Saves the current palette (and gradients for HAM/True Color images) to disk.
- **Default** — Resets the palette to default colors.
- **Restore** — Undoes palette changes back to the state when the image was loaded or when the menu was opened.
- **Copy** — Copies the color from one palette well to another. In EHB mode, limited to base colors.
- **Swap** — Exchanges the contents of two palette wells. In EHB mode, limited to base colors.
- **Spread** — Generates smooth intermediate colors between two selected end colors, filling the wells in between. In EHB mode, limited to base colors.
- **Undo** — Single-level toggle undo for palette operations.
- **Remap (Brilliance / register mode only):**
  - *Page* — Remaps all on-screen pixels to the closest matching color in the modified palette.
  - *Copy* — Globally remaps all instances of one on-screen color to another palette color.
  - *Swap* — Exchanges two palette colors and remaps the image to reflect the change.
- **Base Colors (TrueBrilliance only):**
  - *Get* — Copies the current screen colors to the Palette menu base colors.
  - *Set* — Copies Palette menu base colors to the screen.
  - *Scan* — Analyzes the True Color Buffer and selects the best base colors automatically.

### 7.3 Color Cube
- Displays a spectrum of up to 4,096 colors (full 16.7 million on AGA via sliders).
- Controlled by 6 component buttons (R, G, B for RGB system; C, M, Y for CMY system). Selecting a button also selects the color model.
- Each button makes two other components vary along the X (left→right) and Y (bottom→top) axes of the cube face.
- **Intensity Slider** — Controls the intensity of the selected component.
- Click anywhere in the Color Cube to select that color. Hold right mouse button and drag for dynamic selection.
- Color Component Sliders update to reflect the selected color.

### 7.4 Color Sliders
Three color model modes (can exceed 4,096 colors; full AGA palette available):
- **HSV sliders** — H (Hue, 0–360° color wheel), S (Saturation, grey→fully saturated), V (Value, black→full brightness).
- **RGB sliders** — R (Red), G (Green), B (Blue) additive components.
- **CMY sliders** — C (Cyan), M (Magenta), Y (Yellow) subtractive components.
- **Color Bar** — Preview strip showing the result of the current slider/cube selection.

---

## 8. Gradient System

### 8.1 Gradient/Cycle Tool
- **Left-click** — Activates Color Cycling (if at least one gradient range has cycling enabled with speed > 0). Keyboard: `Tab`.
- **Right-click** — Opens the Gradient Menu. Keyboard: `P` (also opens Palette Menu — context dependent).
- Color Cycling not available in HAM/TrueBrilliance.

### 8.2 Gradient Menu
- Up to **8 independent gradient ranges** (numbered 1–8), each with its own color set and cycle settings.
- **Show button** — Temporarily displays a color bar of the current gradient above the menu.
- **Flip** (`Alt-r`) — Reverses the order of colors in the current gradient.
- **Clear** — Removes all color markers from the current gradient range.
- **Undo** — Single-level toggle undo for gradient operations.
- **Restore** — Returns the gradient to the state it was in when the menu was opened.
- **Gradient Area Marker Box** — 128 slots for color markers. Displays 21 slots at a time; scrollable.
  - Left-click a palette color then click a slot to place a marker.
  - Left-click an existing marker to pick it up.
  - Shift + left-click shifts markers left or right.
  - Shift + right-click removes a marker or pulls right-side markers left.
  - Right-click an existing marker to create a double-tab (abrupt transition).
  - Drag from first color across empty slots to place sequential palette colors.
  - "Real" colors (from painting palette) and "Dynamic" colors (system palette only, for cycling) are both supported.
- **Dither options:**
  - *Random* — Dither transition by randomly mixing adjacent color pixels.
  - *Ordered* (default when Random off) — Dither in a predetermined progressive pattern.
  - *Hard Edge* — Suppresses dithering on double-tab boundaries (only affects Random mode).
  - *Show* — Displays the gradient strip whenever the dither slider is moved.
  - *Dither Amount slider* — Controls dither spread distance (0 = no dither, 100% = maximum).
- **Color Readout** — Displays the total number of colors in the current gradient (counting duplicates).
- **Spread slider** — Sets the number of interpolated in-between colors per gap between markers. At max (254), a gradient can contain over 32,000 colors.
- **Cycle toggle** — Enables color cycling for the current gradient range.
- **Speed slider** — Sets the cycling rate (0–60 cycles/sec, logarithmic scale). Per-gradient setting.

---

## 9. Stencil System

### 9.1 Stencil Tool (`'` apostrophe)
- **Left-click** — Toggles the active stencil mask on/off (if one exists).
- **Right-click** — Opens the Stencil Menu.
- Active stencil mask: selected areas cannot be painted over, and CLR does not affect them.

### 9.2 Stencil Menu — General Operations (both modes)
- **Load Stencil** — Loads a saved stencil mask file; restores its position and protected areas.
- **Save Stencil** — Saves the current stencil mask to disk.
- **Invert** — Flips the stencil mask: previously protected areas become unprotected and vice versa.
- **Delete** — Removes the current stencil mask from memory.

### 9.3 Stencil Menu — Brilliance (Register Mode)
**Make section:**
- **Foregnd** — Automatically creates a stencil from all foreground colors present in the image (protects all non-background areas).
- **Colors** — Creates/modifies the stencil mask using currently selected stencil colors, in one of three operation modes:
  - *Replace* — Replaces the stencil with areas occupied by selected colors.
  - *Add* — Adds selected-color areas to the existing stencil.
  - *Subtract* — Removes selected-color areas from the existing stencil.
- **AnimRemake** — When selected, rebuilds the stencil mask frame-by-frame throughout the animation.

**Colors section:**
- **Clear** — De-selects all stencil colors.
- **Restore** — Restores the first group of stencil colors set while this menu was open.
- **Undo** — Single-level toggle undo for stencil color selection.
- **Select** — Activates eyedropper. Left-click adds a color as a stencil color; right-click removes it. Selected colors shown with an oval in the palette.
- **Lasso** — Draw a freehand outline around an area; colors within are added to the current stencil color set.
- **Invert** — Swaps selected and unselected stencil colors.
- **Show** — Displays selected stencil colors in the palette with oval markers.

### 9.4 Stencil Options — TrueBrilliance (HAM)
Since HAM has millions of colors, stencil creation uses a color range approach:
- **Color selector** — HSV-based picker (H, S, V sliders) to choose the central color.
- **Select button** — Activates eyedropper to pick a color from the screen.
- **Minus Range display** (9 squares) — Shows the "below" portion of the tolerance range.
- **Selected Color display** (center square) — Shows the currently selected central color.
- **Plus Range display** (9 squares) — Shows the "above" portion of the tolerance range.
- **Variance Requester** — Three sliders (H, S, V) setting the allowed deviation from the central color; colors within the range are incorporated into the stencil mask.

### 9.5 Stencil via Draw Mode
- **Stencil draw mode** (from Draw Mode menu, `~` tilde) — Paint the stencil mask directly using any drawing tool. Left mouse button adds; right mouse button removes.

---

## 10. Special Drawing Modes / Modifiers

### 10.1 Fix Background
- **Left-click** — "Locks" everything currently on the canvas as a background layer. Subsequent painting can cover the locked background but it remains intact. Any paint applied after locking can be cleared without affecting the locked layer.
- Toggle: a second click unlocks (frees) the background.
- Functionally opposite to Stencil Foreground.

### 10.2 Grid Lock (`g`)
- Constrains all drawing operations to snap to grid points.
- **Left-click** — Toggles grid lock on/off.
- **Right-click** — Opens Grid Lock Menu:
  - *Adjust button* — Interactive: drag to set grid spacing (horizontal = X width, vertical = Y height; diagonal adjusts both).
  - *Brush button* — Sizes the grid to exactly fit the current brush (one brush per grid square).
  - *X* — Pixel offset of the grid from the left edge of the canvas.
  - *Y* — Pixel offset of the grid from the top edge of the canvas.
  - *W (Width)* — Horizontal pixel distance between grid points.
  - *H (Height)* — Vertical pixel distance between grid points.
  - All values can be entered numerically or adjusted with scroll arrows.

### 10.3 Mirror (`/`)
- **Left-click** — Activates Mirror Draw mode; brush actions are reflected across one or both axes.
- **Right-click** — Opens Mirror Menu:
  - *Adjust* — Click to set the mirror midpoint (crosshair cursor; left-click to place).
  - *X Axis switch* — Reflects actions across the horizontal axis.
  - *Y Axis switch* — Reflects actions across the vertical axis.
  - Both axes can be selected simultaneously for 4-quadrant symmetry.
- Default midpoint: center of canvas.
- Also supports segment-based circular symmetry (evenly divided segments of a circle).

### 10.4 Segmented Draw
- **Left-click on Segment Tool** — Activates Segmented Draw mode. Lines/curves/ellipses/rectangles are drawn using evenly-spaced points rather than continuous pixels. Works with pen tips or user brushes.
- **Right-click** — Opens Segment Menu:
  - *Distance* (default) — Points are spaced by a specified pixel distance (1–999).
  - *# of Points* — Line consists of a fixed count of points regardless of length; spacing increases as line gets longer. Setting to 1 centers a brush between two points.

### 10.5 Fast Feedback
- Toggle on/off by clicking the Fast Feedback tool.
- When on, computation-intensive operations (e.g., pen tip / brush + Line/Curve/Ellipse/Rectangle tools in Smooth/Smear/Cycle mode) display a 1-pixel path + brush shape outline instead of doing the full render, for speed.
- Also reduces brush distortion operations (Size, Bend, Shear) to a single-color brush representation for speed.

### 10.6 Anti-Alias
- Toggle on/off by clicking the Anti Alias button.
- Smooths the appearance of diagonal lines and curved edges by inserting intermediate transition colors.
- Works with brush size, rotation, and distortion operations.
- **Anti Alias Menu** (right-click): Three levels:
  - *Low* — Least pronounced effect; least CPU usage.
  - *Medium* — More pronounced effect; moderate CPU usage.
  - *High* — Best quality; most CPU intensive.

### 10.7 Transparent Mode (`Alt-I`)
- **Left-click** — Activates Transparent painting mode.
- Paint applied is transparent (blended with underlying colors) based on the Opacity setting.
- Only works with the left mouse button / foreground color.
- Layering: within a single mouse-button hold, no accumulation occurs; releasing and re-pressing adds another layer.
- **Transparent Menu** (`Ctrl-t`, right-click):
  - *Amount slider* (0%–100%) — 0% = fully transparent (invisible); 100% = fully opaque.
  - *RGB mode* — Mixes colors based on RGB values.
  - *HSV mode* — Mixes colors based on HSV values.

---

## 11. Undo / Redo

- **UNDO** (`u`) — Removes the most recent change. Brilliance automatically stores each edit in the Undo Buffer.
- **REDO** (`U`) — Restores a previously undone change.
- Number of undo/redo levels limited only by available RAM (user-configurable in Preferences).
- Default: reserves memory for one complete screen in the current resolution.
- **Alt-u** — Undo everything back to the first action in the Undo buffer.
- **Alt-U** — Redo everything forward to the last action in the Undo buffer.
- **Ctrl-u** — Clears the Undo buffer entirely (irreversible).
- In Magnify mode, Undo functions identically to the main menu Undo.

---

## 12. Magnify Mode

- **`m`** — Opens the Magnify window centered at the current cursor position.
- **`M`** — Displays the magnification square for repositioning before committing.
- **`<`** — Decreases magnification level.
- **`>`** — Increases magnification level.
- 31 levels of magnification from 2× to 32×; default is 4×.
- **Magnify Window layout:** Left side = normal view; Right side = magnified view; Center = Drag Bar.
- **Drag Bar controls:**
  - Resize the window by dragging the checkerboard portion.
  - *Magnify off* button — Closes the window.
  - *Increase magnification* button — Increments zoom level.
  - *Decrease magnification* button — Decrements zoom level.
  - *Segmentation toggle* — Turns pixel grid borders (black outlines per pixel) on/off.
- Cursor keys scroll the magnified view within the Magnify window.
- All drawing tools are available while in Magnify mode.

---

## 13. View / Navigation

- **Show Page** (`s`) — Scales the entire canvas to fit one screen while preserving aspect ratio. ESC exits; clicking repositions the view.
- **Cursor keys** — Scroll the canvas (when page is larger than screen).
- **`n`** — Centers the screen on the mouse cursor position.
- **`N`** — Pan the page interactively.
- **Spacebar / F10** — Toggle all menus on/off.
- **Screen Depth gadget** — Moves Brilliance to front or back of Amiga screen stack.

---

## 14. Animation System

### 14.1 Animation Menu (opens via Animation tool)
Accessed with left or right mouse button on the Animation tool.

**Playback Controls:**
- **Frame Selector bar** — Drag to scrub through frames. Left-click advances/retreats one frame; right-click jumps to proportional position.
- **First Frame** — Go to frame 1.
- **Previous Frame** — Step back one frame (wraps from frame 1 to last frame).
- **Next Frame** — Step forward one frame (wraps from last to first).
- **Last Frame** — Go to the last frame.
- **Play Loop Forward** — Continuously plays forward.
- **Play Once Forward** — Plays forward one time.
- **Play Ping-Pong** — Continuously plays forward then backward.
- **Play Once Backward** — Plays backward one time.
- **Play Loop Backward** — Continuously plays backward.
- **Animation Speed** — Set playback rate in Frames Per Second (1–99 FPS).
- **Play Range** — Designate a start and end frame to play only a portion of the animation.
- **Show button** — Must be active for frame changes to be displayed on screen.

**Frame Manipulation:**
- **FRAMES: display** — Shows the current total frame count.
- **Set # (Set Number of Frames)** — Opens the Set Number of Frames requester:
  - Set *Total Frames* (adding or deleting frames as needed).
  - *Add frames*: specify count and insertion point (after frame N).
  - *Delete All*: removes all frames.
  - *Delete range*: specify start and end frame to delete.
- **ADD** — Adds one blank frame at the current position.
- **DEL** — Deletes the currently displayed frame (with warning; cannot be undone).
- **Copy...** — Opens the Copy Frames requester:
  - *Copy current frame to All*: copies the current frame to every frame.
  - *Copy current frame to Frames N through M*: copies to a specified range.
  - *Copy Multiple*: copies a range of frames to a new position within the animation.

**Animation File I/O:**
- **Load Animation** — Opens file requester to load an animation file.
  - *Append option*: adds the loaded animation to the end of the current buffer.
  - *Show option*: displays frames as they load.
- **Save Animation** — Opens file requester to save the animation.
  - Save formats: *Op-5 Byte* (older, compatible with Deluxe Paint), *Op-8 Word* (newer, smaller), *Op-8 Long* (fastest playback on 68020+).
  - *Frame Range* (From: / To:): save only a portion of the animation.

**Keyboard shortcuts for animation:**
- `,` Previous frame; `4` Play loop forward; `5` Play once forward; `6` Ping-pong; `3` Next frame; `@` First frame; `"` Last frame; `$` Play loop backward; `%` Play once backward.
- `Alt--` Delete current frame; `Alt-=` Add frame; `Alt-\` Add blank frame.

### 14.2 Manual Frame-by-Frame Animation (ALT key technique)
- Hold `Alt` + left-click: stamps the current brush on the current frame and advances to the next frame.
- Hold `Alt` + hold left mouse button: continuously draws the brush while advancing frames as the mouse moves.
- `X` key: flips the current brush horizontally (useful for reversing character direction).

---

## 15. Anim-Brush System

### 15.1 Cutting Anim-Brushes
- **Left-click on Anim-Brush tool** — Activates "Cut Anim-Brush" mode. Full-screen crosshairs appear. Drag a box around the object to cut; the tool automatically cuts that region from the current frame and each succeeding frame of the animation.
- Keyboard: `Alt-H`.

### 15.2 Anim-Brush Menu (right-click on Anim-Brush tool)
**Load / Save / Delete:**
- **Load Anim-Brush** — Opens file requester to load a saved anim-brush.
- **Save Anim-Brush** — Opens file requester to save the current anim-brush (only available when an anim-brush is active).
- **Delete** — Removes the current anim-brush.

**Cut section:**
- *FRAMES: counter* — Sets the number of frames to include in the anim-brush.
- *Cut Forward / Cut Backward* — Direction to cut through the animation frames.

**Morph section:**
- **Morph** — Creates an anim-brush that morphs (transforms shape and color) from one stored brush to the current brush over a specified number of frames.
- Requires: one or more brushes in the Brush Wells, and a current active brush.
- *Brush well indicators* (1–8): light grey = brush stored; dark grey = empty or contains an anim-brush (cannot morph from an anim-brush).
- *Frames counter* — Sets the number of morph frames.
- Select the source well number to initiate the morph.

**Anim-Brush Painting Tools:**
- *Actual / Fit display* — Shows actual frame number and the fitted (remapped) frame number. The Fit text entry allows stretching or compressing the anim-brush across a different number of frames.
- **VCR controls for anim-brush frames:**
  - First Frame, Previous Frame, Next Frame, Last Frame.
  - *Anim-Paint Forward* — Cycles frames first→last as you paint.
  - *Anim-Paint Backwards* — Cycles frames last→first as you paint.
  - *Anim-Paint Ping-Pong* — Cycles forward then backward as you paint.
  - *Anim-Paint Pause* — Freezes on the current frame (no advancement while painting).

---

## 16. Tweening (In-Betweening) System

The Tweening menu generates intermediate animation frames automatically by moving and/or transforming a brush between defined start and end states in 3-D perspective space.

### 16.1 Tweening Menu Overview
- Operates in 3-D space (X=horizontal, Y=vertical, Z=depth) projected onto 2-D screen with perspective.
- The **Origin** is the vanishing point where perspective lines converge.
- The **Lens** setting controls the focal length / angle of perspective lines.

### 16.2 Copy/Swap Buttons
- **S→E** — Copies Start settings (Position, Rotation, Opacity) to End.
- **E→S** — Copies End settings to Start.
- **Swap** — Swaps Start and End settings.
- **BrushXY** — Inserts the brush's X/Y cut coordinates (or Align coordinates) into Start or End values.
- **Reset** — Resets all values in the currently active (Start or End) section to defaults (position = 0, opacity = 100).

### 16.3 Settings
**START / END selection** — Toggle between defining starting or ending state.

**POSITION** (X, Y, Z):
- X: horizontal position in pixels from screen center (negative = left).
- Y: vertical position in pixels from screen center (negative = up).
- Z: virtual depth (negative = closer to viewer; positive = further away).
- Left-click arrows: ±1 pixel; Right-click arrows: ±50 pixels.

**ROTATION** (X, Y, Z, in degrees):
- X-axis rotation: brush tumbles top-toward/away from viewer.
- Y-axis rotation: brush spins like a door (left/right edge toward viewer).
- Z-axis rotation: brush spins like a propeller.
- Values can exceed 360° (multiple full rotations). Positive = clockwise; negative = counter-clockwise.

**OPACITY** (0–100%):
- 0% = invisible; 100% = fully opaque.
- Used to fade brush in or out over the tween.

**DECAY** — Applies opacity settings to Trail mode; creates comet-tail fading effects.

### 16.4 Adjust Screen (interactive positioning)
- **Start / End** — Select which state is being adjusted.
- **Brush / Screen** coordinate reference selector.
- **Pos: XY** — Drag brush position on the 2-D plane.
- **Pos: Z** — Drag depth (right = further, left = closer).
- **Rot: X / Y / Z** — Interactive drag to rotate around each axis.
- **Set Area: Lens** — Drag to adjust focal length (right = longer, left = shorter). Default: 512.
- **Set Area: Origin** — Drag the vanishing point to a new position.
- Use / Undo / Reset / Cancel buttons.

### 16.5 Brush Relative Switches
- **Position** — When active, End position values are treated as relative offsets rather than absolute coordinates.
- **Rotation** — When active, End rotation values are treated as relative angular changes.

### 16.6 Ease (Acceleration / Deceleration)
- **Pos: X, Y, Z buttons** — Enable ease for position on individual axes.
- **Rot: X, Y, Z buttons** — Enable ease for rotation on individual axes.
- **Opacity button** — Enable ease for opacity change.
- **Edit Ease sliders:**
  - *Start* — Controls acceleration/deceleration at the beginning of the segment (left = decelerate, right = accelerate).
  - *End* — Controls acceleration/deceleration at the end of the segment.
  - *Xover (crossover)* — Percentage split point between Start and End sections (0–100%).

### 16.7 Count and Draw Settings
- **Count** — Total number of frames the tween spans (cannot exceed animation total frames).
- **Draw S: / E:** — Starting and ending frame numbers within the tween count range to actually draw upon.
- **Trails T:** — Number of "echo" stamps left behind the moving brush.
- Scroll arrows change by 1 (left-click) or 10 (right-click).

### 16.8 Animation Method Buttons
- **Go Back** — Returns to the starting frame after tween is drawn (if not set, stays at end+1).
- **Cyclic** — Adjusts intermediate frames so that a looping tween completes on the first frame rather than the last (avoids "hiccup" when start = end).
- **Aspect** — Preserves the original aspect ratio of the brush during all rotations.
- **Fill** — Treats the entire screen as a single brush for rotation; fills screen with tiled brush pattern, applying tween transformations to the whole screen.
- **Trails** — Stamps the brush multiple times leaving a visible trail; number set in T: gadget.
- **Preview** — Shows a wire-frame rectangle preview of the brush's tween path without drawing.
- **Draw** — Generates all animation frames of the tween sequence.

---

## 17. Buffer System

### 17.1 Buffer Menu
Accessed by clicking the Buffer tool. Used for file I/O, image buffers, preferences, and quitting.

- **Load Picture** — Opens file requester to load any standard Amiga IFF image into the current buffer. Info button shows file dimensions and color depth. Loading a different mode/resolution warns and clears all buffers.
- **Save Picture** — Saves the current image to disk.
  - In TrueBrilliance: can save as HAM (HAM6/HAM8) or True Color (15-bit/24-bit).
- **Preferences** — Opens the Preferences requester to customize and save the working environment (including Undo buffer size and other settings).
- **Print** — Opens the Print requester (see Printing section below).
- **Quit** — Exits Brilliance with a confirmation requester. Also accessible via `Amiga-q`.

### 17.2 Image Buffers
- Brilliance opens with two buffers: one working buffer and one "scratch" buffer.
- All buffers must share the same color mode and resolution.
- Maximum buffer count limited by available RAM and resolution (higher resolution / more colors = fewer possible buffers).
- Each buffer can hold a single image (with its own filename, gradient, and palette) or a single animation frame.
- **Buffer Selector slider** — Drag or click to navigate between buffers.
- **Buffer Indicator** — Displays the current buffer's filename. Unnamed buffers use "UNTITLED", "UNTITLED-001", etc. Current working buffer shown as "WORK"; spare shown as "SPARE".
- **ADD** — Creates a new empty working buffer.
- **DELETE** — Removes the current buffer from memory (cannot delete the last work or spare buffer).
- **Show Page** (`s`) — Scales the full canvas to fit the screen.
- **Copy** — Copies the SPARE buffer's contents to the current WORK buffer.
- **Mark** — Designates the currently displayed buffer as the SPARE buffer.
- **GOTO** — Switches to the SPARE buffer.
- **Merge — Front** — Layers the foreground of the SPARE buffer over the foreground of the WORK buffer.
- **Merge — Back** — Layers the foreground of the SPARE buffer under the foreground of the WORK buffer.
- **About** — Displays version and copyright information.
- **J key** — Toggles between the current work buffer and the spare buffer (mentioned in tutorials).
- Numeric keypad `(` / `)` — Go to previous/next buffer. Keypad `+` — Add new buffer.

---

## 18. Screen Settings

Accessed via the Screen Settings tool on the Main menu.

### 18.1 Screen Mode
**Pre-AmigaDOS 2.1 (Brilliance / register mode):**
- *Low Res* — Max screen width 368px (standard 320×200).
- *High Res* — Max screen width 736px (standard 640×200).
- *Super-High Res* — Max screen width 1440px (standard 1280×200; requires enhanced Denise chip).
- *Extra Halfbrite* — Low Res mode with doubled palette (up to 64 colors). Available on most Amigas (not A1000).
- *Interlace* — Doubles vertical resolution (e.g., Low Res interlaced = 320×400; High Res interlaced = 640×400).

**Pre-AmigaDOS 2.1 (TrueBrilliance / HAM):**
- Non-interlaced or Interlaced (standard Amiga HAM, low res only on non-AGA).

**Post-AmigaDOS 2.1 / AGA:**
- *Select RGB Mode* — Full AGA resolution picker (depends on installed monitor drivers).
- *Select EHB Mode* — Enhanced Half Brite on AGA (64 colors from 16 million; not restricted to Low Res on AGA).
- TrueBrilliance: *Select HAM Mode* — Lists all available HAM screen modes.

### 18.2 Screen Size
- **Width / Height gadgets** — Manual pixel entry with up/down arrows.
- Overscan presets: *Regular*, *Text* (Workbench text overscan), *Standard* (standard overscan), *Max* (maximum viewable overscan), *Video* (absolute maximum; allows areas invisible on monitor but visible on video output; auto-scroll disabled).
- **Retain Image** — When resizing, attempts to retain and scale the existing image content.

### 18.3 Colors (Brilliance / register mode)
- *Colors* slider — Sets the number of palette colors (minimum 2; maximum depends on mode and chipset; up to 256 on AGA).
- *Out of:* slider — Sets the total color space size from which colors are drawn (4K on standard Amiga; 16.7 million on AGA).

### 18.4 Colors (TrueBrilliance / HAM)
- *HAM6* — Up to 4,096 colors.
- *HAM8* — Up to 262,144 colors (AGA only).

### 18.5 Page Size
- **Custom Page button** — Enables a canvas larger than the screen. Cursor keys scroll to off-screen areas.
- **Width / Height gadgets** — Manual entry for page dimensions. Page must be ≤ screen size for animations.

### 18.6 True Color Buffer (TrueBrilliance only)
- *15-bit* — 32,768 color internal buffer (saves memory).
- *24-bit* — Full 16.7 million color internal buffer (higher quality; requires more RAM). Used when saving as True Color 24-bit.

---

## 19. Printing

Accessed via the **Print gadget** on the Buffer menu.

- **Printer** — Shows the currently selected Amiga printer driver.
- **Density slider** — Adjusts DPI (dots-per-inch) of the printout; higher = better quality and slower.
- **Threshold slider** — (Black & White printing only) Determines which on-screen colors print as black.
- **Color Correction** — (Color printer only) Attempts to match on-screen colors to printed output. Selectable per channel (R, G, B). Reduces the maximum printable colors.
- **Copies** — Sets the number of print copies.
- **Shade options:**
  - *Black & White* — Colors printed as black or white based on threshold.
  - *Grey 1* — Varying shades of grey.
  - *Grey 2* — Maximum four shades of grey.
  - *Color* — Full color (requires color printer).
- **Aspect Correction** — Maintains width/height ratio when adjusting print size.
- **Smoothing** — Attempts to reduce "jaggies" on diagonal lines (increases print time).
- **Size units** — *Inches*, *Pixels*, or *Percent* for specifying print dimensions.
- **Width / Height** — Set the size of the printed image.
- **Visual Feedback diagram** — Shows orientation (landscape/portrait) and center/mirror/negative settings graphically.
- **Layout options:**
  - *Landscape / Portrait* — Print orientation.
  - *Center* — Centers image on the page.
  - *Mirror* — Prints a mirror image.
  - *Negative* — Prints a photographic negative.
  - *No Form Feed* — Disables the automatic form feed after printing.
- **Dither (for printing):**
  - *Ordered* — Standard ordered dither (checkerboard-like pattern).
  - *Halftone* — Variable-density dot pattern (best at ≥150 DPI).
  - *Floyd-Steinberg* — Error-diffusion dither; highest detail but slowest (best at ≥150 DPI; auto-disables Smoothing).
- **Scaling:**
  - *Fraction* — Random pixel enlargement/reduction; best for shaded images.
  - *Integer* — Even pixel-to-dot ratio; best for images with thin vertical/horizontal lines.
- **Print / Cancel** buttons.

---

## 20. File I/O System

### 20.1 File Requester (Load & Save)
All requesters share a common layout:
- **Title Bar** — Identifies file type (Picture, Brush, Anim-Brush, Animation, Palette, Stencil) and action (Load = blue bar; Save = red bar).
- **Icon** — Matches the tool that opened the requester; also the Workbench icon saved with the file.
- **Info button** — Retrieves and displays file information (type, dimensions, color depth) for the highlighted file. Also activated by Shift + left-click on a filename.
- **File Display Area** — Shows available disks, assigns, volumes, directories, and files. Directories in white; files in black.
- **Drawer: gadget** — Shows current directory path; can be typed into directly.
- **File: gadget** — Shows/enters the filename. Typing a letter jumps to the first matching filename.
- **Pattern: gadget** — Supports AmigaDOS wildcard/pattern matching.
- **Volumes gadget** — Lists all available volumes and assigns.
- **Parent gadget** — Navigates up one directory level.
- **Load / Save / Cancel** action buttons. Double-clicking a filename also loads it.

### 20.2 Supported File Formats

**Loading (Brilliance / register):**
- Register-based IFF images (2, 4, 8, 16, 32, 64, 128, 256 colors)
- Greyscale images
- DCTV Display images (display only, via DCTV hardware)

**Loading (TrueBrilliance / HAM):**
- All of the above, plus:
- HAM6 and HAM8 images
- True Color (15-bit and 24-bit)
- DCTV Display images (full paint capability via RGB monitor)

**Saving:**
- Standard Amiga IFF picture
- HAM or True Color (TrueBrilliance; 15-bit or 24-bit)
- Animation: Op-5 Byte, Op-8 Word, Op-8 Long formats

---

## 21. Preferences

Accessed from the Buffer menu. Settings include:
- **Undo Buffer size** — Amount of RAM dedicated to undo history (default: one screen's worth).
- **Relative Coordinates** — When active, line tool feedback area shows angle and length instead of absolute coordinates.
- Custom working environment settings saved and recalled via Preferences.

---

## 22. Color Cycling

- Defined per gradient range in the Gradient menu.
- Up to 8 independent cycling ranges active simultaneously, each with its own speed.
- Colors in the Gradient Marker Box cycle through the defined sequence.
- Multiple ranges using the same color register will all cycle through that register simultaneously.
- **Tab key** — Toggles color cycling on/off globally.
- Not available in HAM/TrueBrilliance.

---

## 23. Keyboard Shortcuts Reference

### Brush Control
| Key | Action |
|-----|--------|
| `-` | Reduce brush size by 1px |
| `_` (Shift-hyphen) | Reduce brush size by 10px |
| `=` | Increase brush size by 1px |
| `Shift-=` | Increase brush size by 10px |
| `b` | Cut brush |
| `B` | Restore brush (Shift-B) |
| `h` | Halve brush |
| `H` | Double brush |
| `o` | Outline edge of brush |
| `O` | Trim edge of brush |
| `x` | Flip horizontal |
| `X` | Double horizontal size |
| `y` | Flip vertical |
| `Y` | Double vertical size |
| `z` | 90-degree rotate (fast) |
| `Z` | Stretch/size brush (interactive) |
| `Alt-s` | Center brush handle |
| `Alt-x` | Flip brush handle horizontally |
| `Alt-y` | Flip brush handle vertically |
| `Alt-z` | Place brush handle interactively |
| `Amiga-C` | Copy brush to clipboard |
| `Amiga-E` | Save (Export) brush |
| `Amiga-I` | Load (Import) brush |
| `Amiga-V` | Get brush from clipboard |

### Tool Control
| Key | Action |
|-----|--------|
| `a` | Airbrush select |
| `A` | Size airbrush nozzle |
| `d` | Freehand draw with dots |
| `D` | Freehand filled draw |
| `c` | Unfilled ellipse |
| `C` | Filled ellipse |
| `f` | Fill draw tool |
| `g` | Grid on/off |
| `m` | Magnify mode on/off |
| `M` | Place magnifier |
| `q` | Elliptical curve |
| `Q` | Bezier curve |
| `J` | Toggle buffers (work/spare) |
| `r` | Unfilled rectangle |
| `R` | Filled rectangle |
| `s` | Freehand draw with lines (Connected) |
| `t` | Text requester |
| `u` | Undo |
| `U` | Redo |
| `Alt-u` | Undo all |
| `Alt-U` | Redo all |
| `Ctrl-u` | Reset undo buffer |
| `v` | Line draw |
| `w` | Unfilled polygon (Connected Lines) |
| `W` | Filled polygon |
| `>` | Increase magnification |
| `<` | Decrease magnification |
| `,` | Pick foreground color (eyedropper) |
| `[` | Select previous color for foreground |
| `{` | Select previous color for background |
| `]` | Select next color for foreground |
| `}` | Select next color for background |
| `.` | One-pixel pen tip |
| `/` | Toggle symmetry (Mirror) on/off |

### Gradient & Color Shortcuts
| Key | Action |
|-----|--------|
| `Alt-r` | Reverse gradient |
| `Alt-j` | Use next gradient |
| `Alt-J` | Use previous gradient |
| `Alt-t` | Toggle transparency |

### Menus and Modes
| Key | Action |
|-----|--------|
| `Alt-a` | Animation Menu |
| `F` | Draw Mode Menu |
| `p` | Palette Menu |
| `P` | Gradient Menu |
| `Alt-p` | Tint draw mode |
| `Ctrl-t` | Transparency Menu |
| `Amiga-P` | Print Menu |

### Function Keys (Draw Modes)
| Key | Action |
|-----|--------|
| `F1` | Brush Mode |
| `F2` | Color Mode |
| `F3` | Replace Mode |
| `F4` | Smear Draw Mode |
| `F5` | Range Draw Mode |
| `F6` | Average Smear Draw Mode |
| `F7` | Cycle Draw Mode |
| `F8` | Smooth Draw Mode |
| `F9` | Squash Menus |
| `F10` | Toggle Menus on/off |

### Animation Control
| Key | Action |
|-----|--------|
| `,` | Previous frame |
| `3` | Next frame |
| `@` | Go to first animation frame |
| `"` | Go to last animation frame |
| `4` | Play animation forward continuously |
| `5` | Play animation forward one time |
| `6` | Play animation ping-pong |
| `$` | Play animation backward continuously |
| `%` | Play animation backward one time |
| `Alt--` | Delete current frame |
| `Alt-=` | Add frame |
| `Alt-\` | Add blank frame |

### Anim-Brush Control
| Key | Action |
|-----|--------|
| `7` | Previous anim-brush frame |
| `8` | Next anim-brush frame |
| `Alt-H` | Cut anim-brush |

### Program Operation
| Key | Action |
|-----|--------|
| `K` | Clear current page |
| `n` | Center screen on mouse cursor |
| `N` | Pan page |
| `s` | Show page |
| `Spacebar` | Toggle menus on/off |
| `Tab` | Toggle color cycling on/off |
| `Esc` | Abort current operation |
| `Del` | Toggle mouse cursor on/off |
| Arrow keys | Scroll canvas up/down/left/right |
| `Amiga-q` | Quit Brilliance |

### Buffer Control
| Key | Action |
|-----|--------|
| `Amiga-s` | Save image |
| `Amiga-l` | Load image |
| `(` (keypad) | Go to previous buffer |
| `)` (keypad) | Go to next buffer |
| `+` (keypad) | Add new buffer |
| Numeric keys 1–9 (keypad) | Recall Menu Hotkey configuration |
| Shift + numeric keys 1–9 (keypad) | Save current menus as Menu Hotkey |

---

## 24. AGA (Advanced Graphics Architecture) Support

- Any AGA Amiga screen resolution and mode is supported.
- AGA register mode: up to 256 colors from a 16.7 million color palette.
- AGA HAM8: up to 262,144 simultaneous colors.
- AGA EHB: up to 64 colors (not restricted to Low Res as on OCS/ECS).
- AGA color sliders: 256 steps per RGB channel (vs. 16 steps on OCS/ECS).
- Screen mode selector uses ASL (Application Support Library) requester on AmigaDOS 2.1+.

---

## 25. Special Features Summary

- **Multiple Undo/Redo levels** — RAM-limited history of all edit operations.
- **Up to 8 brush wells** — Store standard brushes and anim-brushes simultaneously.
- **Clipboard integration** — Copy/paste brushes to/from the Amiga system clipboard (enables transfer between Brilliance and TrueBrilliance).
- **Real-time Preview Mode (RPM)** — Live preview for Tint, Colorize, Brighten, Darken, Dither1, Dither2, Negative before committing.
- **Tweening with 3-D perspective** — Full X/Y/Z position, rotation, opacity, ease, trails, cyclic, fill, and aspect modes.
- **Morphing** — Transforms one brush into another over a specified number of anim-brush frames.
- **Color Cycling** — Up to 8 simultaneous independent cycling ranges with individual speeds.
- **Gradient system** — Up to 8 ranges with 128 marker slots, dithering, and spread controls; up to 32,000+ colors in a single gradient.
- **Stencil system** — Color-based or draw-based mask creation; invertible; load/save; per-frame animation rebuild.
- **Fix Background** — Locks the canvas state as a protected base layer.
- **Mirror draw** — X-axis, Y-axis, or both-axis symmetry with adjustable midpoint.
- **Segmented draw** — Evenly-spaced point-based drawing along any line/curve/shape.
- **Anti-aliasing** — Three quality levels for smooth edge rendering; applied to brush rotation, resizing, and all draw operations.
- **Transparent painting** — Adjustable opacity with RGB or HSV color mixing.
- **Fast Feedback** — Performance mode for slower CPUs that previews without rendering.
- **Dual-program architecture** — Separate optimized executables for register and HAM modes.
- **Configurable menu hotkeys** — 9 saveable menu stack presets.

---

*End of feature inventory. Compiled from the Brilliance Version 1.0 User's Guide, Digital Creations, Inc., 1993.*

---

This is the complete feature inventory. It covers every tool, mode, option, menu item, keyboard shortcut, and capability described in the manual. The document is organized into 25 sections with subcategories for easy navigation during reimplementation.

