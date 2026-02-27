# Task 3: System Breakdown Implementation

## Core Requirements
- [x] **ECS Manager:**
    - [x] Efficient entity lifecycle management.
    - [x] Pre-allocated component pools for data locality.
- [x] **Event Bus:**
    - [x] Lock-free ring buffer for inter-system communication.
    - [x] Define standard events: `HitEvent`, `MissEvent`, `AudioPeakEvent`.
- [x] **Input System:**
    - [x] Raw SFML event buffering.
    - [x] World-space coordinate mapping for mouse input.
    - [x] High-precision keyboard timestamping.
- [ ] **Rhythm Director:**
    - [x] Ahead-of-time entity spawning based on Approach Rate (AR).
    - [x] Hit window calculation (Perfect, Great, Miss).
- [x] **Scoring & Combo System:**
    - [x] Subscribing to hit events and calculating multipliers.
- [x] **Asset Manager:**
    - [x] Centralized loading for Textures, Shaders, Audio, and Fonts.
