# Project State: Remarkable Todoist

**Last Updated:** 2026-02-02
**Phase:** 2 - Sync & Task Completion
**Status:** In Progress

---

## Project Reference

**Core Value:** View and manage Todoist tasks on the reMarkable 2 without needing a phone or computer

**Current Focus:** Enable task completion with sync queue and background sync management

**Active Constraints:**
- E-ink display (slow refresh, monochrome, high contrast required)
- ARM Linux platform with limited resources
- Offline-first design (device often disconnected)
- Stylus input only (no physical keyboard)

---

## Current Position

**Phase:** 2 of 3 (Sync & Task Completion)
**Plan:** 02-01 complete (1 of 4)
**Status:** In Progress
**Last activity:** 2026-02-02 - Completed 02-01-PLAN.md (Task completion building blocks)

**Progress:** [████████████░░░░░░░░] 50% (4/8 plans delivered)

**Phase Goal:** User can complete tasks offline with automatic background sync

**Active Requirements:** SYNC-01, SYNC-02, SYNC-03, COMP-01, COMP-02, COMP-03

**Next Milestone:** Complete 02-03-PLAN.md - SyncManager implementation

---

## Performance Metrics

**Requirements:**
- Total v1: 15
- Completed: 0
- In Progress: 8 (Phase 1)
- Pending: 7

**Phases:**
- Total: 3
- Completed: 1
- In Progress: 1
- Pending: 1

**Plans:**
- Total: 8 (across all phases)
- Completed: 4
- In Progress: 0
- Pending: 4

**Velocity:** 7 min/plan (4 data points: 16 + 11 + 3 + 2 = 32 min / 4 plans)

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
| 30 second request timeout | Handle slow/unreliable WiFi on reMarkable device | 01-02 | 2026-01-31 |
| Project name caching in QMap | Populate task.projectName efficiently before emitting tasksFetched | 01-02 | 2026-01-31 |
| User-friendly error messages | "Invalid API token" instead of "401 Unauthorized" for better UX | 01-02 | 2026-01-31 |
| **Qt6 Quick/QML instead of Qt5 Widgets** | reMarkable 3.x firmware uses Qt6 with Quick/QML, not Qt Widgets. Discovered during 01-04 integration | 01-04 | 2026-01-31 |
| QGuiApplication + QQmlApplicationEngine | Standard pattern for Qt Quick apps - AppController exposes Q_PROPERTY for QML binding | 01-04 | 2026-01-31 |
| Removed C++ widget views | TaskDelegate.qml and main.qml replace C++ TaskListView/TaskDelegate classes | 01-04 | 2026-01-31 |
| 56x56 checkbox touch targets | Increased from 48px for better stylus/touch input on e-ink | 01-04 | 2026-01-31 |
| 110px row height for task list | Increased from 80px to accommodate QML layout with comfortable spacing | 01-04 | 2026-01-31 |
| Priority colors in QML | P1=red (#d1453b), P2=orange (#eb8909), P3=blue (#246fe0), P4=muted | 01-04 | 2026-01-31 |
| Full-screen mode by default | Window.FullScreen for reMarkable; 1404x1872 native resolution | 01-04 | 2026-01-31 |
| Official reMarkable toolchain | Use remarkable.engineering SDK (v3.1.15), not Toltec Docker for Qt6 | 01-04 | 2026-01-31 |
| CMake toolchain file pattern | Source env script first, then use cmake/remarkable.cmake for cross-compile | 01-04 | 2026-01-31 |
| ARM flags: armv7-a + neon + hard float | Required for Cortex-A7 CPU on reMarkable 2 | 01-04 | 2026-01-31 |
| QT_QUICK_BACKEND=epaper | Required env var for e-ink rendering on reMarkable | 01-04 | 2026-01-31 |
| -platform epaper | Qt platform plugin for e-paper display | 01-04 | 2026-01-31 |
| Stop xochitl before running | Must stop main UI: `systemctl stop xochitl` | 01-04 | 2026-01-31 |
| USB network route | `ip route add default via 10.11.99.8` for API access via USB | 01-04 | 2026-01-31 |
| Touch rotation | `QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="rotate=180:invertx"` | 01-04 | 2026-01-31 |
| **On-device building** | Cross-compilation toolchain lacks Qt6 Quick/QML libraries; build on device instead | Deploy | 2026-02-01 |
| **Stock firmware (no Toltec)** | Toltec not supported on current reMarkable OS version | Deploy | 2026-02-01 |
| **Launcher notebook for app launch** | No app launcher on stock firmware; use inotifywait to detect notebook open, switch to app | Deploy | 2026-02-01 |
| Oxide integration as fallback | If user has Toltec/Oxide, can use standard launcher metadata at /opt/etc/draft/ | Deploy | 2026-02-01 |
| POST with empty QByteArray | Qt requires empty body for bodyless POST; Todoist /close returns 204 No Content | 02-01 | 2026-02-02 |
| Linear scan in setTaskCompleted | Task list is small (<100 items), no need for hash map | 02-01 | 2026-02-02 |
| dataChanged with specific role | Emit {CompletedRole} for minimal QML update instead of all roles | 02-01 | 2026-02-02 |

### Open Questions

| Question | Context | Priority | Phase |
|----------|---------|----------|-------|
| ~~Qt Widgets vs Qt Quick?~~ | **RESOLVED:** reMarkable 3.x uses Qt6 Quick/QML. Discovered during 01-04 integration. | ~~High~~ | 1 |
| Handwriting OCR approach? | MyScript vs on-device, quality/cost unknown | High | 3 |
| Optimal e-ink refresh frequency? | Conservative start (full refresh every 5 partials), tune empirically | Medium | 1 |

### Current Blockers

None

### Active TODOs

- [x] Execute 01-01-PLAN.md (Project foundation)
- [x] Execute 01-02-PLAN.md (Todoist API client with error handling)
- [x] Execute 01-03-PLAN.md (Task list UI: TaskListView and TaskDelegate) - **obsoleted by Qt6/QML**
- [x] Execute 01-04-PLAN.md (Integration) - Refactored to Qt6 Quick/QML
- [ ] Commit Qt6/QML changes (uncommitted from crash)
- [x] Verify build and test on desktop - **PASSED: 6 projects, 21 tasks fetched**
- [x] Visual verification checkpoint - **PASSED: Running on reMarkable device**

---

## Session Continuity

**Last Session:** 2026-02-02 - Phase 2 Plan 01 execution
**Stopped at:** Completed 02-01-PLAN.md (Task completion building blocks)
**Resume file:** None

**Quick Context for Next Session:**
- **Phase 1 complete:** App displays Todoist tasks on device
- **Build:** On-device compilation (cross-compilation not working due to Qt6 library mismatch)
- **Deployment:** Two options implemented:
  1. `launcher/` - Notebook-based launcher for stock firmware (inotifywait watches for "Launch Todoist" notebook)
  2. `oxide/` - Standard Oxide integration for Toltec users

**Phase 2 Progress:**
- ✓ 02-01 - Task completion building blocks
  - TodoistClient.closeTask() - POST to /tasks/{id}/close with 204 No Content
  - TaskModel.setTaskCompleted() - State update with dataChanged signal
  - Both methods follow established patterns from Phase 1

**Next Steps:**
- Execute 02-02-PLAN.md - SyncQueue implementation with JSON persistence
- Execute 02-03-PLAN.md - SyncManager orchestration
- Execute 02-04-PLAN.md - UI integration and visual verification
- Test offline task completion on device

---

*This file maintains project memory across sessions. Update after each significant change.*
