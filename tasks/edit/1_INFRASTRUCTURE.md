# Task 1: Level Editor Infrastructure & Project Management

## Core Requirements
- [ ] **Menu Integration:**
    - [ ] Add a "Level Editor" button to the `MenuScene` (or as a separate entry in the slanted menu).
    - [ ] Create `LevelEditorMenuScene` to manage custom levels in the `edits/` folder.
- [ ] **Project Management:**
    - [ ] Ensure the `edits/` directory exists (create it on startup if missing).
    - [ ] Implement "New Song" wizard:
        - [ ] Input fields for Song Name, Artist, BPM.
        - [ ] File picker (or simple string path) for MP3 and Background Image.
        - [ ] Generate a skeleton `.pvmap` file.
- [ ] **History System:**
    - [ ] Implement a `Command` pattern for editor actions.
    - [ ] Create a `HistoryManager` with `undo()` and `redo()` capabilities.
    - [ ] Support at least 50 levels of history.
- [ ] **File I/O:**
    - [ ] Implement a `BeatmapWriter` to save `Beatmap` structures back to `.pvmap` (JSON or custom format).
