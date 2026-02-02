# Remarkable Todoist

## What This Is

A Todoist client for the reMarkable 2 e-ink tablet. Lets you view all your tasks, check them off, and add new tasks using handwriting recognition — all from your reMarkable.

## Core Value

View and manage Todoist tasks on the reMarkable 2 without needing a phone or computer.

## Requirements

### Validated

- [x] View all tasks in a list with name, due date, project, and priority (Phase 1 complete)

### Active

- [ ] Check off tasks to mark them complete
- [ ] Add new tasks via handwriting recognition (task name only)
- [ ] Auto-sync when WiFi is available
- [ ] Manual refresh button
- [ ] Offline support: queue changes and sync when reconnected

### Out of Scope

- Setting project/due date/priority when adding tasks — keep add flow simple
- Toltec package distribution — goal is working on personal device
- Editing existing tasks — view and complete only
- Subtasks — flat task list only
- Filters/views — shows all tasks, no filtering UI

## Context

- **Target device:** reMarkable 2 e-ink tablet
- **Firmware:** Stock reMarkable OS (Toltec not supported on current firmware version)
- **Build environment:** On-device compilation (cross-compilation not working due to Qt6 library mismatch)
- **Launch method:** Launcher notebook (opens special notebook to launch app) or manual via SSH
- **Input method:** Stylus for handwriting recognition, touch for tapping checkboxes and buttons
- **Display:** E-ink (slow refresh, monochrome, high contrast)
- **Connectivity:** WiFi, often disconnected
- **API:** Todoist REST API (user needs guidance on obtaining API token)

## Constraints

- **Platform:** Stock reMarkable 2 firmware — ARM Linux, limited resources, no Toltec/Oxide
- **Build:** Must compile on-device; cross-compilation toolchain doesn't have matching Qt6 Quick/QML libraries
- **E-ink:** UI must work with slow refresh rates — no animations, minimize full-screen redraws
- **Input:** No physical keyboard — text input via handwriting recognition only
- **Network:** Device is often offline — must queue changes locally
- **Launch:** No app launcher on stock firmware — using launcher notebook workaround

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Qt6 Quick/QML (not Qt5 Widgets) | reMarkable 3.x firmware uses Qt6 with QML, discovered during Phase 1 | Confirmed |
| On-device building | Cross-compilation toolchain missing Qt6 Quick libraries | Working |
| Launcher notebook | Stock firmware has no app launcher; notebook trigger switches from Xochitl to app | Implemented |
| Two-screen UI (list + add) | Simple, matches e-ink constraints | Pending |
| Handwriting for text input | Native to reMarkable experience | Pending |
| All tasks (no filtering) | Keep v1 simple, add filters later if needed | Pending |
| Task name only on add | Reduce complexity, most tasks just need a name | Pending |

---
*Last updated: 2026-02-01 — Phase 1 complete, stock firmware deployment learned*
