# Task 6: Project Folder Structure Implementation

## Core Requirements
- [x] Establish the root directory structure:
    - [x] `src/engine/`: Core infrastructure.
    - [x] `src/game/`: Rhythm-specific gameplay logic.
    - [x] `assets/`: Textures, Audio, Fonts.
    - [x] `shaders/`: GLSL shader code.
    - [x] `levels/`: `.pvmap` files.
    - [x] `external/`: Third-party headers/libs not managed by Conan.
    - [x] `tests/`: GTest or Catch2 unit/integration tests.
- [x] Ensure `CMakeLists.txt` correctly targets these directories.
- [x] Implement strict `include` path rules to maintain decoupling.
