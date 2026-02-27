# Gemini CLI: Project Implementation Rules

## 🤖 Operation Philosophy
- **Research -> Strategy -> Execution:** Always validate assumptions before writing code.
- **Surgical Updates:** Use `replace` for precise modifications. Avoid rewriting entire files unless necessary.
- **No Comments:** Do not write code comments (//, /* */) unless explicitly requested.
- **No Git:** The user handles all git operations manually.

## 📋 Task & Progress Management
- **Source of Truth:** `PROGRESS.md` tracks high-level phases. `tasks/*.md` tracks atomic requirements.
- **Workflow:**
    1. Check `PROGRESS.md` for current phase.
    2. Read the corresponding `tasks/X_TASK_NAME.md`.
    3. Execute implementation.
    4. Mark tasks as complete (`[x]`) in `tasks/*.md`.
    5. Update `PROGRESS.md` status.

## 🏗️ Architectural Mandates
- **Separation of Concerns:** `src/engine/` must remain generic and decoupled from `src/game/`.
- **ECS-First:** Use **EnTT** for all game objects. Systems should be data-oriented.
- **Performance:** 
    - Zero allocations in the hot gameplay loop.
    - Linear memory access for component iteration.
    - Batch rendering via `sf::VertexArray`.
- **Timing:** Use audio-sample-based timing where possible for rhythm synchronization.
- **Modern C++:** Use C++23 features (std::expected, std::optional, etc.) where appropriate.

## ✅ Validation Protocol
- **Empirical Proof:** Run the build and verify functionality after every significant change.
- **Tests First:** ALWAYS add or update unit tests in the `tests/` directory when implementing new features or editing existing logic. A feature is not complete until it is tested.
- **Run Tests:** Use `./build.sh` to ensure all tests pass before concluding a task.
