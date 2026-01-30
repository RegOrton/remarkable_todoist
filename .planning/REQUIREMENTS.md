# Requirements: Remarkable Todoist

**Defined:** 2026-01-29
**Core Value:** View and manage Todoist tasks on the reMarkable 2 without needing a phone or computer.

## v1 Requirements

### Authentication

- [ ] **AUTH-01**: Store API token in plaintext config file on device
- [ ] **AUTH-02**: Load token on app startup

### Task Display

- [ ] **DISP-01**: Show all tasks in scrollable list
- [ ] **DISP-02**: Display task name for each task
- [ ] **DISP-03**: Display due date for each task
- [ ] **DISP-04**: Display project name for each task
- [ ] **DISP-05**: Display priority indicator (P1-P4) for each task
- [ ] **DISP-06**: Checkbox next to each task to mark complete

### Sync

- [ ] **SYNC-01**: Manual refresh button pulls latest tasks from Todoist
- [ ] **SYNC-02**: Offline queue stores pending changes when no WiFi
- [ ] **SYNC-03**: Sync queued changes when connection restored
- [ ] **SYNC-04**: Show sync status indicator in UI

### Task Creation

- [ ] **TASK-01**: Add task screen with handwriting input area
- [ ] **TASK-02**: Convert handwriting to text
- [ ] **TASK-03**: Submit new task to Todoist (or queue if offline)

## v2 Requirements

### Enhanced Input

- **INPUT-01**: Natural language date parsing ("tomorrow", "next Friday")
- **INPUT-02**: Set project when creating task
- **INPUT-03**: Set priority when creating task

### Sync Improvements

- **SYNC-05**: Auto-sync when WiFi becomes available
- **SYNC-06**: Conflict resolution for concurrent edits

### Display Optimization

- **DISP-07**: Partial e-ink refresh for smoother scrolling
- **DISP-08**: Gesture shortcuts (swipe to complete)

## Out of Scope

| Feature | Reason |
|---------|--------|
| Secure/encrypted token storage | User accepts plaintext for simplicity |
| Auto-sync on WiFi | Manual refresh preferred for battery/control |
| Editing existing tasks | View and complete only for v1 |
| Subtasks | Flat task list only |
| Filters/custom views | Shows all tasks, no filtering UI |
| Labels/tags display | Keep list simple |
| Multiple accounts | Single account for v1 |
| Toltec package distribution | Goal is working on personal device |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| AUTH-01 | Phase 1 | Pending |
| AUTH-02 | Phase 1 | Pending |
| DISP-01 | Phase 1 | Pending |
| DISP-02 | Phase 1 | Pending |
| DISP-03 | Phase 1 | Pending |
| DISP-04 | Phase 1 | Pending |
| DISP-05 | Phase 1 | Pending |
| DISP-06 | Phase 1 | Pending |
| SYNC-01 | Phase 2 | Pending |
| SYNC-02 | Phase 2 | Pending |
| SYNC-03 | Phase 2 | Pending |
| SYNC-04 | Phase 2 | Pending |
| TASK-01 | Phase 3 | Pending |
| TASK-02 | Phase 3 | Pending |
| TASK-03 | Phase 3 | Pending |

**Coverage:**
- v1 requirements: 15 total
- Mapped to phases: 15
- Unmapped: 0

---
*Requirements defined: 2026-01-29*
*Last updated: 2026-01-29 after roadmap creation*
