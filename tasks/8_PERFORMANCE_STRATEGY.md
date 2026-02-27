# Task 8: Performance Strategy Implementation

## Core Requirements
- [ ] **Memory Management:**
    - [x] Pre-allocate all entity and component memory during level load. ✅
    - [x] Avoid `new`/`delete` or `std::shared_ptr` in the update loop. ✅
- [ ] **Object Pooling:**
    - [ ] Implement pools for particles and visual effects.
- [ ] **Data Locality:**
    - [ ] Ensure systems iterate linearly over component arrays for cache efficiency.
- [ ] **Batch Rendering:**
    - [x] Minimize draw calls by batching geometry sharing materials into `sf::VertexArray`. ✅
- [ ] **Frame Pacing:**
    - [ ] Implement frame limiting and synchronization with audio hardware.
- [ ] **Profiling:**
    - [ ] Integrate Tracy or another profiler for real-time monitoring.
