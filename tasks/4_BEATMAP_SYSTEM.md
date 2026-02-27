# Task 4: Beatmap System (.pvmap) Implementation

## Core Requirements
- [x] Define the `.pvmap` file specification (human-readable, versioned).
- [x] Implement a robust parser for `.pvmap` files.
- [x] **File Sections:**
    - [x] `[Version]`: Format versioning.
    - [x] `[Metadata]`: Title, Artist, BPM, Offset.
    - [x] `[Timing]`: BPM changes and timing points.
    - [x] `[Events]`: Visual/Shader triggers linked to timestamps.
    - [x] `[Nodes]`: Hit objects and Sliders (including spline path data).
- [x] Integrate parsing with the ECS (loading beatmap into memory at start).
- [x] Synchronize parser timestamps with the Audio Core's sample clock.
