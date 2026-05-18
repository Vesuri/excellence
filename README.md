# Excellence

A pixel art / indexed-color image editor targeting Commodore Amiga-style workflows, inspired by the classic Amiga application [Brilliance](https://en.wikipedia.org/wiki/Brilliance_(paint_program)).

## Features

- Indexed-color canvas (1–8 bit, up to 256 colors)
- Drawing tools: draw, line, curve, rectangle, ellipse, airbrush, fill, text, brush, and more
- Gradient fills (linear, radial, spherical, highlight, horizontal, vertical)
- Mirror draw, grid lock, segment stamping
- Undo/redo
- IFF ILBM file format support (Amiga native format)
- Raw binary pixel data export

## Requirements

- Qt 5 or Qt 6
- qmake
- C++ compiler (clang or GCC)

## Build

```bash
qmake Excellence.pro
make -j4
```

On macOS the application builds as `Excellence.app`. Run it with:

```bash
open Excellence.app
```

On Linux:

```bash
./Excellence
```

## Project Structure

```
src/             Application source
src/plugins/     File format plugins (ilbm, raw)
Brilliance.md    Feature reference from the original Amiga application
Features.md      Prioritized feature list for this project
TODO.md          Outstanding work items
```
