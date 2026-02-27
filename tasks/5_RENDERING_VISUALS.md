# Task 5: Rendering & Visual Impact Implementation

## Core Requirements
- [x] Implement a layered rendering pipeline using `sf::VertexArray` batching.
- [x] **Rendering Layers:**
    - [x] Layer 1: Background (Fragment shader driven). ✅
    - [x] Layer 2: Playfield (Spline paths/grid).
    - [x] Layer 3: Entities (Rhythm nodes).
    - [x] Layer 4: Particles (High-density geometry).
- [x] **Post-Processing Stack:**
    - [x] Multi-pass `sf::RenderTexture` ping-ponging.
    - [x] Bloom (Brightness extraction + Blur + Additive blend).
    - [x] Chromatic Aberration (Tension-driven).
- [x] **Visual Feedback:**
    - [x] Screen shake via texture coordinate offset.
    - [x] Audio-reactive background shaders using FFT data. ✅
    - [x] Hot-reload system for GLSL shaders. ✅
