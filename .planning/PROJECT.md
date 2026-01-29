# Remarkable Todoist

## What This Is

A Todoist client for the reMarkable 2 e-ink tablet, distributed as a Toltec/Oxide app. Lets you view all your tasks, check them off, and add new tasks using handwriting recognition — all from your reMarkable.

## Core Value

View and manage Todoist tasks on the reMarkable 2 without needing a phone or computer.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] View all tasks in a list with name, due date, project, and priority
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
- **Distribution:** Toltec package manager with Oxide launcher
- **Input method:** Stylus for handwriting recognition, touch for tapping checkboxes and buttons
- **Display:** E-ink (slow refresh, monochrome, high contrast)
- **Connectivity:** WiFi, often disconnected
- **API:** Todoist REST API (user needs guidance on obtaining API token)

## Constraints

- **Platform:** Must run on reMarkable 2 with Toltec/Oxide — ARM Linux, limited resources
- **E-ink:** UI must work with slow refresh rates — no animations, minimize full-screen redraws
- **Input:** No physical keyboard — text input via handwriting recognition only
- **Network:** Device is often offline — must queue changes locally

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Two-screen UI (list + add) | Simple, matches e-ink constraints | — Pending |
| Handwriting for text input | Native to reMarkable experience | — Pending |
| All tasks (no filtering) | Keep v1 simple, add filters later if needed | — Pending |
| Task name only on add | Reduce complexity, most tasks just need a name | — Pending |

---
*Last updated: 2026-01-29 after initialization*
