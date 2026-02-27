# Pulse Vector: Project Architecture & Design Specification

## 1. High-Level Game Concept
**Core Identity:** A 360-degree spatial-kinetic rhythm game where auditory frequencies manifest as physical entities. 
**Input Paradigm:** Assymetric dual-input. The **Mouse** provides continuous spatial tracking (aiming/tracing), while the **Keyboard** provides discrete temporal execution (rhythm/frequency matching).

**Gameplay Loop:**
*   **Nodes** spawn in a 360-degree field and travel along complex, curved spline paths towards a central "Event Horizon" or away from it.
*   Nodes have specific "Frequencies" represented by colors/shapes, mapped to keyboard keys (e.g., A, S, D, F).
*   **Execution:** The player must continuously track the node with their mouse cursor. At the exact rhythmic moment the node pulses, the player must strike the corresponding frequency key.
*   **Sliders/Tethers:** Sustained sounds require the player to hold the frequency key while physically tracing the exact path of the sound wave with the mouse. 
*   **Feedback & "Thunk":** A "Perfect" hit instantly freezes the node's micro-animation, triggers a localized radial blur shader, emits a heavy burst of vector particles, and applies a micro-pause (hitstop) to the visual layer.

## 2. Engine Architecture Overview
The engine, **KineticEngine**, is built on a strict separation of concerns, utilizing a Data-Oriented Design (DOD) approach via an Entity Component System (ECS).

*   **Application Layer:** Handles OS windowing, input polling (SFML window/events), and the core application lifecycle.
*   **Game Loop:** Decoupled logic and rendering.
    *   *Update:* Fixed timestep (e.g., 240Hz) tied to the audio hardware clock.
    *   *Render:* Variable timestep with state interpolation to support high-refresh-rate monitors.
*   **Audio Core:** The source of truth. The game timer polls the exact playback position of the audio track.
*   **ECS Backbone:** All game objects are entities. Systems iterate over tight, cache-friendly contiguous arrays of components.

## 3. System Breakdown
### Core Systems
*   **ECS Manager:** Manages entity lifecycles and component pools. Optimized for sequential memory access.
*   **Event Bus:** A lock-free ring buffer for decoupled inter-system communication (e.g., `HitEvent`, `MissEvent`).

### Gameplay Systems
*   **Input System:** Buffers raw SFML input events. Maps mouse coordinates to world space and queues keyboard timestamps.
*   **Rhythm Director System:** Reads the beatmap. Spawns entities ahead of time based on approach rate (AR). Calculates hit windows.
*   **Scoring & Combo System:** Subscribes to hit events. Drives the dynamic tension multiplier (combo).

### Rendering & Visual Systems
*   **Render System:** Sorts renderable components by Z-index and material. Batches geometry into `sf::VertexArray`.
*   **Particle System:** Operates on pre-allocated object pools. Pushes vertex data directly to batched buffers.
*   **Post-Process Manager:** Manages `sf::RenderTexture` ping-ponging for multi-pass shader effects.

## 4. Beatmap Format Specification (.pvmap)
The `.pvmap` is a human-readable, versioned, plain-text format.

**Structure Example:**
```ini
[Version]
1.0
[Metadata]
Title: Neural Link
BPM: 175
[Nodes]
1200,A,100,200,Hit
2000,D,100,500,Slider,2500,Spline(150:400|200:300)
```
**Integration:**
*   Parsed on level load into a `Beatmap` data structure.
*   Timing sync relies entirely on the audio track's current sample position.

## 5. Rendering Pipeline & Visual Impact
**Render Stack:**
1.  **Background Layer:** Time-based fragment shader.
2.  **Playfield Layer:** Batched grid lines or spline paths.
3.  **Entity Layer:** Active rhythm nodes, batched by texture atlas.
4.  **Particle Layer:** High-density geometric bursts (single draw call).
5.  **Composite Layer (Post-Processing):** Bloom, Chromatic Aberration, and Screen Shake.

**Creating the "Thunk":**
*   **Hitstop:** 2-3 frame pause on perfect hits.
*   **Screen Shake:** Violent texture coordinate offset using decaying sine waves.
*   **Audio Reactivity:** FFT data driving shader parameters.

## 6. Project Folder Structure
```text
audio-ride/
├── src/
│   ├── engine/ (Core infrastructure)
│   └── game/ (Specific gameplay logic)
├── assets/ (Audio, textures, fonts)
├── shaders/ (GLSL code)
├── levels/ (.pvmap files)
└── tests/ (Unit and integration tests)
```

## 7. Build System & Target Architecture
*   **Conan:** Manages `sfml/3.0.2`, `fmt`, and `entt`.
*   **CMake:** Modern target-based setup.
    *   `audio-ride-engine` (STATIC)
    *   `audio-ride-game` (STATIC)
    *   `audio-ride-exe` (EXECUTABLE)
*   **Presets:** `linux-debug` (Sanitizers, -O0) and `linux-release` (LTO, -O3).

## 8. Performance Strategy
*   **Zero-Allocation Gameplay Loop:** All memory for entities and particles is pre-allocated.
*   **Data Locality:** Linear memory iteration via ECS.
*   **Batching:** Geometry sharing the same texture/shader is pushed into a single `sf::VertexArray`.

## 9. Future Scalability Notes
*   **Plugin Architecture:** Support for loading `.so` files for modding.
*   **Compute Shaders:** Offloading particle physics to the GPU via OpenGL interop.
*   **Multi-threading:** Offloading rendering prep and Audio FFT analysis to worker threads.
