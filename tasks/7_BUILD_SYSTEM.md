# Task 7: Build System & Target Architecture Implementation

## Core Requirements
- [x] **Conan Integration:**
    - [x] Configure `conanfile.py` with `sfml/3.0.2`, `fmt`, and `entt`.
    - [x] Set up `build_policy="missing"`.
- [x] **CMake Configuration:**
    - [x] Create a modern target-based `CMakeLists.txt`.
    - [x] `audio-ride-engine` as a static library.
    - [x] `audio-ride-game` as a static library.
    - [x] `audio-ride-exe` as the executable linking both libraries.
- [x] **Presets (`CMakePresets.json`):**
    - [x] `linux-debug`: `-O0 -g`, Sanitizers (ASan, UBSan).
    - [x] `linux-release`: `-O3 -flto -march=native`.
- [x] **Orchestration (`build.sh`):**
    - [x] Automated `conan install`.
    - [x] Automated `cmake --preset`.
    - [x] Build and Run flags.
