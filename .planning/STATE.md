# Project State: Remarkable Todoist

**Last Updated:** 2026-02-02
**Phase:** 2 - Sync & Task Completion
**Status:** Complete

---

## Project Reference

**Core Value:** View and manage Todoist tasks on the reMarkable 2 without needing a phone or computer

**Current Focus:** Phase 2 complete - task completion with sync queue and background sync

**Active Constraints:**
- E-ink display (slow refresh, monochrome, high contrast required)
- ARM Linux platform with limited resources
- Offline-first design (device often disconnected)
- Stylus input only (no physical keyboard)

---

## Current Position

**Phase:** 2 of 3 (Sync & Task Completion)
**Plan:** 02-04 complete (4 of 4)
**Status:** Phase Complete
**Last activity:** 2026-02-02 - Completed 02-04-PLAN.md (UI integration and visual verification)

**Progress:** [████████████████████] 75% (6/8 plans delivered)

**Phase Goal:** User can complete tasks offline with automatic background sync

**Active Requirements:** SYNC-01, SYNC-02, SYNC-03, COMP-01, COMP-02, COMP-03 - ALL COMPLETE

**Next Milestone:** Phase 3 - Task Creation with Handwriting Input

---

## Performance Metrics

**Requirements:**
- Total v1: 15
- Completed: 6 (SYNC-01, SYNC-02, SYNC-03, COMP-01, COMP-02, COMP-03)
- In Progress: 0
- Pending: 9 (Phase 3)

**Phases:**
- Total: 3
- Completed: 2
- In Progress: 0
- Pending: 1

**Plans:**
- Total: 8 (across all phases)
- Completed: 6
- In Progress: 0
- Pending: 2

**Velocity:** 7 min/plan (6 data points: 16 + 11 + 3 + 2 + 2 + 8 = 42 min / 6 plans)

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
| **Cross-compilation via sysroot** | Pull device Qt6 .so files to /tmp/rm-sysroot, cross-compile with arm-linux-gnueabihf-g++ using build-rm.sh | Deploy | 2026-02-03 |
| **Stock firmware (no Toltec)** | Toltec not supported on current reMarkable OS version | Deploy | 2026-02-01 |
| **Launcher notebook for app launch** | No app launcher on stock firmware; use inotifywait to detect notebook open, switch to app | Deploy | 2026-02-01 |
| Oxide integration as fallback | If user has Toltec/Oxide, can use standard launcher metadata at /opt/etc/draft/ | Deploy | 2026-02-01 |
| POST with empty QByteArray | Qt requires empty body for bodyless POST; Todoist /close returns 204 No Content | 02-01 | 2026-02-02 |
| Linear scan in setTaskCompleted | Task list is small (<100 items), no need for hash map | 02-01 | 2026-02-02 |
| dataChanged with specific role | Emit {CompletedRole} for minimal QML update instead of all roles | 02-01 | 2026-02-02 |
| Optimistic online state | Assume online until proven otherwise - better UX for immediate sync attempts | 02-03 | 2026-02-02 |
| QNetworkInformation with error-based fallback | May not work on reMarkable; fallback detects offline via timeout/connection errors | 02-03 | 2026-02-02 |
| 2 second delay after connectivity restoration | Avoid race conditions when network just came up; gives WiFi/DNS time to stabilize | 02-03 | 2026-02-02 |
| Dequeue only after server confirms success | Never remove operation on failure; only dequeue after 204 No Content to prevent data loss | 02-03 | 2026-02-02 |
| Max 5 retry attempts | Balance persistence vs avoiding infinite loops on permanent failures | 02-03 | 2026-02-02 |
| Full SyncManager include in appcontroller.h | Qt6 metaobject requires complete type for Q_PROPERTY, forward declaration insufficient | 02-04 | 2026-02-02 |
| Null checks for syncManager in QML | SyncManager created in initialize() after QML loads, bindings need null-safe access | 02-04 | 2026-02-02 |
| Complete-only, no toggle | Checkbox only completes tasks, doesn't reopen (separate feature) | 02-04 | 2026-02-02 |
| Minimal static inotifywait | Full inotify-tools needs autotools; wrote minimal C replacement (tools/inotifywait.c), cross-compiled static | Deploy | 2026-02-03 |
| 15s watcher delay after xochitl boot | Xochitl reads all .content files on startup; delay avoids false launcher triggers | Deploy | 2026-02-03 |
| HOME=/home/root in launcher env | Systemd services have minimal env; QSettings needs HOME to find config | Deploy | 2026-02-03 |
| appController.quit() not Qt.quit() | Qt.quit() doesn't work on embedded; QCoreApplication::quit() via controller works | Deploy | 2026-02-03 |
| BusyBox compat in launcher scripts | Device uses BusyBox; head -n 1 not head -1, /bin/sh not /bin/bash | Deploy | 2026-02-03 |

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
- [x] Verify build and test on desktop - **PASSED: 6 projects, 21 tasks fetched**
- [x] Visual verification checkpoint - **PASSED: Running on reMarkable device**
- [x] Execute 02-01-PLAN.md (Task completion building blocks)
- [x] Execute 02-02-PLAN.md (SyncQueue with JSON persistence)
- [x] Execute 02-03-PLAN.md (SyncManager orchestration)
- [x] Execute 02-04-PLAN.md (UI integration) - **PASSED: 18 tasks, completion synced to API**

---

## Session Continuity

**Last Session:** 2026-02-02 - Phase 2 Plan 04 completion
**Stopped at:** Completed 02-04-PLAN.md (UI integration and visual verification)
**Resume file:** None

**Quick Context for Next Session:**
- **Phase 1 complete:** App displays Todoist tasks on device
- **Phase 2 complete:** Task completion with optimistic UI and background sync
- **Phase 2 deployed and verified on device** (2026-02-03)
- **Build:** Cross-compilation on arm64 host using `build-rm.sh` with device sysroot at `/tmp/rm-sysroot`
- **Deploy:** `scp build-rm/remarkable-todoist root@10.11.99.1:/opt/bin/`
- **Launcher:** Notebook-based launcher on stock firmware (minimal static inotifywait, 15s boot delay)

**Phase 2 Complete:**
- 02-01 - Task completion building blocks (Wave 1)
  - TodoistClient.closeTask() - POST to /tasks/{id}/close with 204 No Content
  - TaskModel.setTaskCompleted() - State update with dataChanged signal
- 02-02 - SyncQueue with JSON persistence (Wave 1)
  - Queue operations: enqueue, dequeue, peek, clear
  - Persists to ~/.config/remarkable-todoist/sync_queue.json
- 02-03 - SyncManager orchestration (Wave 2)
  - Coordinates TodoistClient, SyncQueue, and connectivity state
  - QNetworkInformation with error-based fallback
  - Auto-sync on reconnect with 2 second delay
  - Retry logic (max 5 attempts)
- 02-04 - UI integration (Wave 3)
  - AppController.completeTask() with optimistic update
  - Sync status indicator in QML header
  - TaskDelegate checkbox wired to completeTask()
  - Verified: 18 tasks fetched, completion synced to Todoist API

**Next Steps:**
- Phase 3: Task Creation with Handwriting Input
- Handwriting canvas for stylus input
- OCR integration (MyScript or on-device)
- Task creation API integration

---

*This file maintains project memory across sessions. Update after each significant change.*
