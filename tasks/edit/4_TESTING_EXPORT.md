# Task 4: Visual Feedback & Export

## Core Requirements
- [ ] **Test Playback:**
    - [ ] Implement a "Test Play" mode (`Ctrl + P`).
    - [ ] Transition from Level Editor to `MainScene` (with the current unsaved map).
    - [ ] Return to Level Editor at the exact time the player was at.
- [ ] **Export & Finalization:**
    - [ ] Add an "Export" button to move a song from `edits/` to `levels/`.
    - [ ] Implement the `ExportWizard`:
        - [ ] Verify metadata.
        - [ ] Package song, map, and background into a final `.pvmap` package if needed.
        - [ ] Update `levels/` list to show the new song in the normal menu.
- [ ] **Polishing:**
    - [ ] Tooltips for UI buttons.
    - [ ] Visual indicators for "Unsaved Changes" (e.g., * in title).
    - [ ] Autosave functionality.
- [ ] **Validation:**
    - [ ] Add unit tests for `BeatmapWriter` and `HistoryManager`.
    - [ ] Ensure `undo/redo` works consistently across different node types.
