# Project State: Remarkable Todoist

**Last Updated:** 2026-02-01
**Phase:** 1 - Foundation & Task Display
**Status:** Complete

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
**Plan:** 01-04 complete (4 of 4)
**Status:** **PHASE 1 COMPLETE** - verified on device
**Last activity:** 2026-01-31 - App running on reMarkable, displaying Todoist tasks

**Progress:** [██████████░░░░░░░░░░] 20% (3/15 plans delivered)

**Phase Goal:** User can view all their Todoist tasks on the reMarkable 2

**Active Requirements:** AUTH-01, AUTH-02, DISP-01, DISP-02, DISP-03, DISP-04, DISP-05, DISP-06

**Next Milestone:** Complete 01-04-PLAN.md - commit Qt6/QML changes, verify on device

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

**Last Session:** 2026-02-01 - Deployment options for stock firmware
**Stopped at:** Committed launcher notebook and Oxide integration
**Resume file:** None

**Quick Context for Next Session:**
- **Phase 1 complete:** App displays Todoist tasks on device
- **Build:** On-device compilation (cross-compilation not working due to Qt6 library mismatch)
- **Deployment:** Two options implemented:
  1. `launcher/` - Notebook-based launcher for stock firmware (inotifywait watches for "Launch Todoist" notebook)
  2. `oxide/` - Standard Oxide integration for Toltec users
- **Key learnings:**
  - Toltec not supported on current reMarkable OS version
  - Cross-compilation toolchain lacks Qt6 Quick/QML libraries
  - Stock firmware has no app launcher - must stop xochitl to run custom apps
  - Launcher notebook pattern: systemd service + inotifywait + notebook trigger

**Deployment Files (committed):**
- launcher/todoist-launcher.sh - Watches for notebook, switches apps
- launcher/todoist-launcher.service - Systemd unit
- launcher/install.sh - Installation script
- oxide/remarkable-todoist.json - Oxide metadata
- oxide/install.sh - Oxide installation script

**Next Steps:**
- Test launcher notebook on device (requires inotify-tools)
- Begin Phase 2: Sync & Task Completion

---

*This file maintains project memory across sessions. Update after each significant change.*
