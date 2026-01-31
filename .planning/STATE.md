# Project State: Remarkable Todoist

**Last Updated:** 2026-01-31
**Phase:** 1 - Foundation & Task Display
**Status:** In Progress

---

## Project Reference

**Core Value:** View and manage Todoist tasks on the reMarkable 2 without needing a phone or computer

**Current Focus:** Establish development environment and build read-only task display with proper e-ink rendering and auth foundation

**Active Constraints:**
- E-ink display (slow refresh, monochrome, high contrast required)
- ARM Linux platform with limited resources
- Offline-first design (device often disconnected)
- Stylus input only (no physical keyboard)

---

## Current Position

**Phase:** 1 of 3 (Foundation & Task Display)
**Plan:** 3 of 4 complete
**Status:** In progress
**Last activity:** 2026-01-31 - Completed 01-03-PLAN.md

**Progress:** [██████████░░░░░░░░░░] 20% (3/15 plans delivered)

**Phase Goal:** User can view all their Todoist tasks on the reMarkable 2

**Active Requirements:** AUTH-01, AUTH-02, DISP-01, DISP-02, DISP-03, DISP-04, DISP-05, DISP-06

**Next Milestone:** Complete 01-04-PLAN.md (Integration and visual verification)

---

## Performance Metrics

**Requirements:**
- Total v1: 15
- Completed: 0
- In Progress: 8 (Phase 1)
- Pending: 7

**Phases:**
- Total: 3
- Completed: 0
- In Progress: 1
- Pending: 2

**Plans:**
- Total: 4 (Phase 1)
- Completed: 3
- In Progress: 0
- Pending: 1

**Velocity:** 10 min/plan (3 data points: 16 + 11 + 3 = 30 min / 3 plans)

---

## Accumulated Context

### Key Decisions

| Decision | Rationale | Phase | Date |
|----------|-----------|-------|------|
| Three-phase structure for quick depth | Requirements naturally group into display -> sync -> creation, follows dependencies | Roadmap | 2026-01-29 |
| Include handwriting in v1 | TASK-02 is v1 requirement despite complexity | Roadmap | 2026-01-29 |
| Combine foundation with display in Phase 1 | Delivers visible progress early, validates e-ink constraints before adding complexity | Roadmap | 2026-01-29 |
| Priority display inverts API values | API 4 (highest) -> P1, API 1 (lowest) -> P4 for user-friendly display | 01-01 | 2026-01-31 |
| QSettings static class pattern | No instances needed, simpler API for settings access | 01-01 | 2026-01-31 |
| Config at ~/.config/remarkable-todoist/ | Standard QSettings UserScope with 0600 permissions for security | 01-01 | 2026-01-31 |
| 48x48 checkbox touch targets | Meets WCAG guidelines for stylus/touch input | 01-03 | 2026-01-31 |
| Full e-ink refresh every 5 partials | Prevents ghosting while maintaining responsiveness | 01-03 | 2026-01-31 |
| 80px row height for task list | Accommodates two-line layout (title + metadata) with spacing | 01-03 | 2026-01-31 |

### Open Questions

| Question | Context | Priority | Phase |
|----------|---------|----------|-------|
| Qt Widgets vs Qt Quick? | Research shows conflicting info, needs validation in Phase 1 | High | 1 |
| Handwriting OCR approach? | MyScript vs on-device, quality/cost unknown | High | 3 |
| Optimal e-ink refresh frequency? | Conservative start (full refresh every 5 partials), tune empirically | Medium | 1 |

### Current Blockers

None

### Active TODOs

- [x] Execute 01-01-PLAN.md (Project foundation)
- [x] Execute 01-02-PLAN.md (Todoist API client with error handling)
- [x] Execute 01-03-PLAN.md (Task list UI: TaskListView and TaskDelegate)
- [ ] Execute 01-04-PLAN.md (Integration and visual verification)

---

## Session Continuity

**Last Session:** 2026-01-31 - Completed 01-03-PLAN.md
**Stopped at:** Completed 01-03-PLAN.md (Task list UI)
**Resume file:** None

**Quick Context for Next Session:**
- Plan 01-01 complete: Qt 5.15 project, Task model, TaskModel, AppSettings
- Plan 01-02 complete: TodoistClient with async HTTP, error handling, task fetching
- Plan 01-03 complete: TaskDelegate (48x48 checkbox, 80px rows), TaskListView (e-ink refresh tracking)
- All components ready for integration in 01-04-PLAN.md
- Build system includes all source files

**Files Modified This Session:**
- CMakeLists.txt (updated with view sources)
- src/views/taskdelegate.h, taskdelegate.cpp (created)
- src/views/tasklistview.h, tasklistview.cpp (created)
- .planning/phases/01-foundation-task-display/01-03-SUMMARY.md (created)
- .planning/STATE.md (updated)

---

*This file maintains project memory across sessions. Update after each significant change.*
