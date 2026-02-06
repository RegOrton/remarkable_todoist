---
phase: 03-task-creation
plan: 01
title: Task Creation Backend - API and Sync Support
completed: 2026-02-06
duration: 8 min
status: complete
subsystem: network/sync
tags: [todoist-api, sync-queue, task-creation, rest-api]

dependency-graph:
  requires:
    - 02-01 (TodoistClient close_task pattern)
    - 02-02 (SyncQueue persistence)
    - 02-03 (SyncManager orchestration)
  provides:
    - TodoistClient.createTask() API method
    - SyncManager.queueTaskCreation() for offline-first creation
    - taskCreateSynced/taskCreateSyncFailed signals for UI binding
  affects:
    - 03-02 (handwriting input will use queueTaskCreation)
    - 03-04 (UI integration will bind to createTask signals)

tech-stack:
  added: []
  patterns:
    - POST with JSON body for task creation
    - tempId for optimistic UI tracking
    - content/tempId fields in SyncOperation

key-files:
  created: []
  modified:
    - src/network/todoist_client.h
    - src/network/todoist_client.cpp
    - src/models/sync_queue.h
    - src/models/sync_queue.cpp
    - src/network/sync_manager.h
    - src/network/sync_manager.cpp

decisions:
  - id: create-task-http-200
    description: "Todoist REST API v2 POST /tasks returns HTTP 200 (not 201)"
    rationale: "API returns full task JSON with 200 status on success"
  - id: tempid-for-optimistic-ui
    description: "Use tempId field to track optimistic UI entries"
    rationale: "Allows UI to show task immediately, update with real ID when server confirms"
  - id: content-field-for-create-ops
    description: "SyncOperation.content stores task name for create_task"
    rationale: "Different from close_task which only needs taskId"

metrics:
  tasks-completed: 2
  tasks-total: 2
  commits: 2
  lines-added: ~120
---

# Phase 03 Plan 01: Task Creation Backend Summary

**One-liner:** POST /rest/v2/tasks with JSON body via TodoistClient.createTask(), SyncManager orchestration with tempId tracking for offline-first creation.

## What Was Built

### Task 1: Add createTask to TodoistClient and extend SyncOperation (1a9e127)

Extended the Todoist API client with task creation capability:

- **TodoistClient.createTask(content)**: POSTs to `/rest/v2/tasks` with JSON body `{"content": "..."}`
- **taskCreated signal**: Emitted on success with content and server-assigned taskId
- **createTaskFailed signal**: Emitted on failure with content and error message
- **onCreateTaskReplyFinished handler**: Parses HTTP 200 response with full task JSON

Extended SyncOperation struct for create_task operations:

- **content field**: Stores task name for create operations
- **tempId field**: Tracks optimistic UI entries before server confirmation
- Updated sync_queue.cpp to serialize/deserialize new fields

### Task 2: Extend SyncManager to queue and process task creation (f64fe91)

Added task creation orchestration to SyncManager:

- **queueTaskCreation(content, tempId)**: Creates and enqueues create_task operation
- **taskCreateSynced signal**: Emitted with tempId + serverTaskId for UI update
- **taskCreateSyncFailed signal**: Emitted with tempId + error for retry/rollback
- **processNextOperation()**: Now handles both close_task and create_task types
- **onTaskCreated/onCreateTaskFailed**: Response handlers with same retry logic as close_task

## Architecture Notes

The task creation flow follows the established sync patterns:

```
UI calls queueTaskCreation(content, tempId)
    -> SyncOperation{type: "create_task", content, tempId} enqueued
    -> processQueue() attempts immediate sync
    -> TodoistClient.createTask(content) POSTs to API
    -> onTaskCreated extracts serverTaskId from response
    -> emit taskCreateSynced(tempId, serverTaskId)
    -> UI replaces tempId with real ID
```

Offline behavior:

- Operations persist to sync_queue.json
- Same retry logic (max 5 attempts, 2s delay)
- Same offline detection from network errors
- Auto-sync when connectivity restored

## Verification Results

```
1. Project compiles cleanly: PASS
2. TodoistClient has createTask(), taskCreated, createTaskFailed: PASS
3. SyncManager has queueTaskCreation(), taskCreateSynced, taskCreateSyncFailed: PASS
4. SyncOperation has content and tempId fields: PASS
5. SyncQueue serializes/deserializes content and tempId: PASS
6. processNextOperation() handles both close_task and create_task: PASS
```

## Deviations from Plan

None - plan executed exactly as written.

## Commits

| Hash | Type | Description |
|------|------|-------------|
| 1a9e127 | feat | Add createTask to TodoistClient and extend SyncOperation |
| f64fe91 | feat | Extend SyncManager to queue and process task creation |

## Next Steps

- Plan 03-02: Handwriting canvas for stylus input
- Plan 03-03: OCR integration (MyScript or on-device)
- Plan 03-04: Wire handwriting to task creation API
