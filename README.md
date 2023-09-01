# TRIAD-6C
This is a port of [Bandock/TRIAD-6](https://github.com/Bandock/TRIAD-6) to C.

This repo does not have a dependency on OpenGL for further compatibility. Instead, it lets SDL2 pick the backend renderer.

## Building
Make sure you have SDL2 installed and linkable.

- `git clone https://github.com/scratchminer/TRIAD-6C`
- `cd TRIAD-6C && mkdir build && cd build`
- `cmake ..`
- `make`

The TRIAD-6 binary should be in the `build` directory after that.
