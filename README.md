# Simple 2D Verlet Integration Engine

A 2D physics engine implemented in C utilizing Verlet integration and basic tilemap collision.

---

## Physics Architecture & Features

- **Verlet Integration:** Position integration using implicit velocity and spring constraints.
- **Constraint Solver:** Distance-based constraint relaxation for sticks, links, and structural meshes.
- **Tilemap Collision:** Grid-aligned collision detection and response against level terrain.
- **Tiled Map Integration:** Loads level maps created with Tiled (`/Tiled/` directory).

---

## Repository Structure

- `src/` — C source files (Main, physics, tilemap loader)
- `include/` — Header files
- `Tiled/` — Tiled map files (`.tmx`/`.json`) and tileset assets
- `Makefile` — Build automation script

---

## Getting Started

### Prerequisites

Ensure you have a C compiler (`gcc` or `clang`); currently only Windows is supported.

### Building & Running

1. Clone the repository and navigate to the project directory:
    ```
    git clone https://github.com/itdish0/simple-verlet-raylib.git
    cd simple-verlet-raylib
    ```
2. Clean, compile, and run the project:
    ```
    make clean && make run
    ```
---

## Tiled Map Workflow

Level maps and tile collision flags are located in the `/Tiled/` directory.

- Map data files (`.tmx` / `.json`) define tile placement, layer ordering, and static collision geometry.
- The C engine loads a given map into memory to for collision between the tilemap and Verlet particles.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
