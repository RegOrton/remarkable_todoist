# Roadmap: Remarkable Todoist

**Project:** Todoist client for reMarkable 2 e-ink tablet
**Core Value:** View and manage Todoist tasks on the reMarkable 2 without needing a phone or computer
**Depth:** Quick (3-5 phases)
**Created:** 2026-01-29

## Overview

This roadmap delivers a working Todoist client for the reMarkable 2 in three phases. Phase 1 establishes the foundation and read-only task display. Phase 2 adds network sync and task completion capability. Phase 3 enables task creation via handwriting recognition. The structure follows natural dependencies: display before sync, read before write, simple operations before complex ones.

## Phases

### Phase 1: Foundation & Task Display

**Goal:** User can view all their Todoist tasks on the reMarkable 2

**Status:** Planning Complete

**Dependencies:** None (foundation phase)

**Requirements:**
- AUTH-01: Store API token in plaintext config file on device
- AUTH-02: Load token on app startup
- DISP-01: Show all tasks in scrollable list
- DISP-02: Display task name for each task
- DISP-03: Display due date for each task
- DISP-04: Display project name for each task
- DISP-05: Display priority indicator (P1-P4) for each task
- DISP-06: Checkbox next to each task to mark complete

**Success Criteria:**
1. User can launch app from Oxide launcher on reMarkable 2
2. User sees all their tasks displayed in a scrollable list showing name, due date, project, and priority
3. App loads API token from config file on startup without user intervention
4. Task list displays correctly on e-ink screen with high contrast and no ghosting
5. User can scroll through task list using stylus with proper touch targets

**Plans:** 4 plans

Plans:
- [ ] 01-01-PLAN.md — Project foundation: CMake build, Task model, Settings module
- [ ] 01-02-PLAN.md — Todoist API client with error handling
- [ ] 01-03-PLAN.md — Task list UI: TaskListView and TaskDelegate
- [ ] 01-04-PLAN.md — Integration and visual verification

---

### Phase 2: Sync & Task Completion

**Goal:** User can refresh task list and check off completed tasks

**Status:** Pending

**Dependencies:** Phase 1 (requires task display and auth foundation)

**Requirements:**
- SYNC-01: Manual refresh button pulls latest tasks from Todoist
- SYNC-02: Offline queue stores pending changes when no WiFi
- SYNC-03: Sync queued changes when connection restored
- SYNC-04: Show sync status indicator in UI

**Success Criteria:**
1. User can tap refresh button to pull latest tasks from Todoist when WiFi is available
2. User can tap checkbox next to task to mark it complete, with immediate visual feedback
3. User can complete tasks while offline, changes are queued automatically
4. User sees sync status indicator showing online/offline state and pending changes
5. Queued changes sync automatically when WiFi reconnects, without data loss

**Plans:** 0 planned, 0 completed

---

### Phase 3: Task Creation

**Goal:** User can add new tasks via handwriting input

**Status:** Pending

**Dependencies:** Phase 2 (requires sync infrastructure for submitting new tasks)

**Requirements:**
- TASK-01: Add task screen with handwriting input area
- TASK-02: Convert handwriting to text
- TASK-03: Submit new task to Todoist (or queue if offline)

**Success Criteria:**
1. User can navigate to "Add Task" screen from task list
2. User can write task name using stylus in handwriting input area
3. App converts handwriting to text with editable preview before submission
4. User can submit new task, which appears in task list immediately
5. New tasks created offline are queued and submitted when WiFi reconnects

**Plans:** 0 planned, 0 completed

---

## Progress

| Phase | Status | Plans | Requirements | Completion |
|-------|--------|-------|--------------|------------|
| 1 - Foundation & Task Display | Planning Complete | 0/4 | 8 | 0% |
| 2 - Sync & Task Completion | Pending | 0/0 | 4 | 0% |
| 3 - Task Creation | Pending | 0/0 | 3 | 0% |

**Overall:** 0% complete (0/15 requirements delivered)

---

## Version History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-29 | 1.0 | Initial roadmap created with 3 phases covering all 15 v1 requirements |
| 2026-01-30 | 1.1 | Phase 1 planning complete: 4 plans in 3 waves |

---

*Roadmap tracks phase-level goals and success criteria. For detailed implementation plans, see individual phase plan files.*
