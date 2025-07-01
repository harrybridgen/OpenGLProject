[View it in action here](https://youtu.be/gl-psFSN4wM)


# RunEscape: Real-Time Terrain Exploration

By Harry Bridgen

## Overview

This is a C++ OpenGL terrain simulation and exploration project developed for university coursework. It showcases a dynamic 3D world featuring procedurally generated terrain, lighting with shadows, post-processing effects, and a controllable player character.

All core systems, including terrain rendering, environment effects, camera, player movement, tree placement, water simulation, and post-processing fog, were implemented by me.

## Features

- Procedurally generated terrain using multi-scale noise
- Dynamic lighting with day-night cycle driven by a moving sun
- Real-time shadow mapping
- Fog post-processing using screen-space depth and color buffers
- Third-person camera with smooth tracking and mouse control
- Click-to-move player character rendered with a loaded 3D model
- Water rendering with animated surface
- Tree placement with per-instance transforms and material-based shading
- Modular design with clean OpenGL buffer and shader management

## Project Structure

- `Coursework2.cpp` — Main application loop and OpenGL setup
- `camera.cpp` — Orbiting camera system with mouse controls
- `player.cpp` — Click-to-move player logic, animation, and rendering
- `terrain.cpp` — Terrain mesh generation, noise-based elevation, and material blending
- `tree.cpp` — Tree placement and rendering with support for multiple meshes and textures
- `sun.cpp` — Simulates sun movement, light color, and direction over time
- `water.cpp` — Renders animated water plane with time-driven shader
- `shader.cpp` — GLSL shader compilation helper
- `/shaders` — Folder containing multiple shaders

## Dependencies

- OpenGL 3.3+ with `GLFW` and `gl3w`
- `GLM` for math operations
- `stb_image` for texture loading
- Custom OBJ loader and utility functions

## Controls

- **Right Mouse Drag** — Rotate the camera
- **Scroll Wheel** — Zoom in/out
- **Left Mouse Click** — Set player destination

## Author

Harry Bridgen  
[github.com/harrybridgen](https://github.com/harrybridgen)

## Credits

- University of Nottingham — Coursework specification
- OpenGL, GLFW, GLM — Core libraries used
