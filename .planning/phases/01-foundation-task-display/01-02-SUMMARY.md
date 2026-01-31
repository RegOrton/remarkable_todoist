---
phase: 01-foundation-task-display
plan: 02
subsystem: network
tags: [qt5, qnetworkaccessmanager, todoist-api, rest, json, async]

# Dependency graph
requires:
  - phase: 01-01
    provides: Task struct with fromJson(), AppSettings for API token
provides:
  - TodoistClient class for async Todoist API access
  - Task fetching from /rest/v2/tasks endpoint
  - Project fetching from /rest/v2/projects endpoint
  - Project name caching for task display
  - Comprehensive HTTP error handling with user-friendly messages
affects:
  - 01-03 (TaskListView needs TaskModel populated by client)
  - 01-04 (Integration connects TodoistClient to UI)

# Tech tracking
tech-stack:
  added: []
  patterns: [Qt async networking with signals/slots, QNetworkReply memory management, HTTP error mapping]

key-files:
  created:
    - src/network/todoist_client.h
    - src/network/todoist_client.cpp
  modified:
    - CMakeLists.txt

key-decisions:
  - "30 second request timeout for slow/unreliable network conditions"
  - "Project names cached in QMap for task display enrichment"
  - "Error messages user-friendly, not technical HTTP codes"

patterns-established:
  - "Network pattern: QNetworkReply* with connect to finished slot, deleteLater() on all paths"
  - "Error pattern: handleNetworkError() helper method for consistent error mapping"
  - "Async pattern: emit signals for success (data) and failure (QString error message)"

# Metrics
duration: 6min
completed: 2026-01-31
---

# Phase 1 Plan 02: Todoist API Client Summary

**TodoistClient with async task/project fetching via QNetworkAccessManager, Bearer token auth, comprehensive error handling, and project name caching**

## Performance

- **Duration:** 6 min
- **Started:** 2026-01-31T17:57:25Z
- **Completed:** 2026-01-31T18:03:30Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments
- TodoistClient class with QNetworkAccessManager for async HTTP requests
- fetchAllTasks() retrieves tasks from https://api.todoist.com/rest/v2/tasks
- fetchProjects() retrieves projects from https://api.todoist.com/rest/v2/projects
- Project name caching (m_projectNames map) for populating task.projectName
- Bearer token authentication in Authorization header
- Comprehensive error handling: 401 (invalid token), 403 (forbidden), 404 (not found), 429 (rate limit), 5xx (server errors)
- User-friendly error messages (not raw HTTP codes)
- 30 second transfer timeout to handle slow connections
- Memory leak prevention with reply->deleteLater() on all code paths

## Task Commits

Each task was committed atomically:

1. **Task 1: Implement TodoistClient with task fetching** - `95441c9` (feat)

Note: Task 2 (comprehensive error handling) was fully implemented within Task 1. The error handling helper method handleNetworkError() was created during initial implementation with all error code mappings, logging, and timeout configuration.

## Files Created/Modified
- `src/network/todoist_client.h` - TodoistClient class declaration with signals and slots
- `src/network/todoist_client.cpp` - Full implementation with async fetching and error handling
- `CMakeLists.txt` - Added todoist_client.cpp to SOURCES

## Decisions Made
- **Request timeout:** 30 seconds via setTransferTimeout() to handle slow/unreliable WiFi on reMarkable device
- **Project caching:** Store project id->name mapping in m_projectNames QMap before fetching tasks
- **Error messages:** User-friendly strings like "Invalid API token. Please check your token in settings." instead of raw "401 Unauthorized"
- **Static endpoint constants:** TASKS_ENDPOINT and PROJECTS_ENDPOINT as static const QString for maintainability

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None - all tasks completed successfully on first attempt.

## User Setup Required

None - no external service configuration required. API token configuration documented in later plans.

## Next Phase Readiness
- TodoistClient ready to be connected to TaskModel in integration (01-04)
- TaskListView (01-03) can use TaskModel populated by TodoistClient
- Error signals can be connected to display error messages to user
- Project name caching ensures tasks have human-readable project names

---
*Phase: 01-foundation-task-display*
*Completed: 2026-01-31*
