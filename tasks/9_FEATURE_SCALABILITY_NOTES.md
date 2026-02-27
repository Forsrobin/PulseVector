# Task 9: Feature Scalability Notes

## Core Requirements
- [ ] **Plugin/Modding Architecture:**
    - [ ] Design an interface for loading external `.so` libraries for custom gameplay modes.
- [ ] **GPU Acceleration:**
    - [ ] Investigate OpenGL interop for moving particle physics to Compute Shaders.
- [ ] **Multi-threading:**
    - [ ] Parallelize rendering preparation (filling `sf::VertexArray`).
    - [ ] Offload Audio FFT analysis to a dedicated background thread.
- [ ] **Asset Streaming:**
    - [ ] Implement background loading for assets during long transitions or between songs.
- [ ] **Networking:**
    - [ ] Ensure ECS state is deterministic to allow for future rollback-based multiplayer.
