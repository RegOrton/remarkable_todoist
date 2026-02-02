---
phase: 02-sync-task-completion
plan: 01
subsystem: api
tags: [todoist-api, task-completion, qt-signals, qml-binding]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: TodoistClient with fetch methods and TaskModel with display roles
provides:
  - TodoistClient.closeTask() method for task completion API
  - TaskModel.setTaskCompleted() method for UI state updates
  - Signal infrastructure for task completion (taskClosed, closeTaskFailed)
affects: [02-02, 02-03, 02-04]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "POST with empty body pattern for Todoist /close endpoint"
    - "dataChanged with specific role for granular QML updates"

key-files:
  created: []
  modified:
    - src/network/todoist_client.h
    - src/network/todoist_client.cpp
    - src/models/taskmodel.h
    - src/models/taskmodel.cpp

key-decisions:
  - "POST with empty QByteArray for bodyless POST requests"
  - "204 No Content as success indicator for closeTask"
  - "Linear scan in setTaskCompleted (task list is small)"
  - "dataChanged with {CompletedRole} for minimal QML update"

patterns-established:
  - "Error handling via handleNetworkError() for all API methods"
  - "reply->deleteLater() on all code paths to prevent leaks"
  - "dataChanged with specific roles for QML ListView updates"

# Metrics
duration: 2min
completed: 2026-02-02
---

# Phase 02 Plan 01: Task Completion Building Blocks Summary

**TodoistClient gains closeTask() API method; TaskModel gains setTaskCompleted() state management with proper QML signal emission**

## Performance

- **Duration:** 2 min 27 sec
- **Started:** 2026-02-02T17:41:48Z
- **Completed:** 2026-02-02T17:44:15Z
- **Tasks:** 2
- **Files modified:** 4

## Accomplishments
- TodoistClient can now close tasks via Todoist API with proper 204 No Content handling
- TaskModel can mark tasks completed with dataChanged signal for QML binding
- Both methods follow established patterns from Phase 1

## Task Commits

Each task was committed atomically:

1. **Task 1: Add closeTask() to TodoistClient** - `3659055` (feat)
2. **Task 2: Add setTaskCompleted() to TaskModel** - `8ced1ed` (feat)

## Files Created/Modified
- `src/network/todoist_client.h` - Added closeTask() method declaration and signals
- `src/network/todoist_client.cpp` - Implemented closeTask() with POST to /tasks/{id}/close, onCloseTaskReplyFinished handler with 204 check
- `src/models/taskmodel.h` - Added setTaskCompleted() method declaration
- `src/models/taskmodel.cpp` - Implemented setTaskCompleted() with task lookup, state update, and dataChanged emission

## Decisions Made

**POST with empty body pattern**
- Qt requires QByteArray() for bodyless POST requests
- Todoist /close endpoint expects no body, returns 204 No Content

**Linear scan for task lookup**
- setTaskCompleted() uses simple loop through m_tasks vector
- Task list is small (typically < 100 items), no need for hash map
- Follows existing pattern from Phase 1

**Specific role in dataChanged**
- Emit dataChanged(idx, idx, {CompletedRole}) instead of all roles
- More efficient - QML only updates completed checkbox, not entire row
- Follows research from RESEARCH.md

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

**Ready for next plan (02-02):**
- TodoistClient has API method for task completion
- TaskModel has state management for task completion
- Both building blocks tested via compilation

**Next steps:**
- 02-02: Add checkbox to QML UI and wire click handling
- 02-03: Implement optimistic UI updates with sync queue
- 02-04: Visual verification of task completion flow

---
*Phase: 02-sync-task-completion*
*Completed: 2026-02-02*
