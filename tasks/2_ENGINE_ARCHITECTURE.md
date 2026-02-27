# Task 2: Engine Architecture Implementation

## Core Requirements
- [x] Set up the **KineticEngine** core infrastructure.
- [x] Integrate **EnTT** or a custom ECS for data-oriented design.
- [x] Implement the Application Layer (SFML window management, OS event polling).
- [x] Develop a decoupled Game Loop:
    - [x] Fixed timestep update (240Hz) for input/logic precision.
    - [x] Variable timestep render with state interpolation.
- [x] Build the Audio Core:
    - [x] Poll exact playback position from audio hardware.
    - [x] Use audio position as the source of truth for all rhythmic logic.
- [x] Ensure strict separation between `engine/` and `game/` codebases.
