# Phase 2: Sync & Task Completion - Research

**Researched:** 2026-02-02
**Domain:** Offline-first sync architecture with Qt6/QML
**Confidence:** HIGH

## Summary

Phase 2 implements task completion with offline queuing for the reMarkable tablet. The core challenge is handling the device's intermittent WiFi connectivity gracefully - users should be able to complete tasks anytime, with changes syncing when connectivity is available.

The architecture follows the offline-first pattern: optimistic UI updates provide immediate feedback, a persistent queue stores pending operations, and a sync manager handles network state detection and queue processing. The Todoist REST API v2 provides a simple POST endpoint for task completion that returns 204 No Content on success.

**Primary recommendation:** Implement a SyncManager class that owns connectivity detection, queue persistence, and sync orchestration. Keep TodoistClient focused on API calls, and let AppController coordinate between UI state and sync operations.

## Standard Stack

The established libraries/tools for this domain:

### Core (Already in Project)
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| Qt6::Network | 6.x | QNetworkAccessManager, QNetworkInformation | Already used for API calls |
| Qt6::Core | 6.x | QFile, QJsonDocument, QStandardPaths | Standard Qt data persistence |

### Supporting (New for Phase 2)
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| QNetworkInformation | Qt 6.1+ | Connectivity monitoring | Detect online/offline state |
| QJsonDocument | Qt 6.x | Queue persistence | Store pending operations as JSON |
| QStandardPaths | Qt 6.x | Writable paths | Find app data directory |
| QTimer | Qt 6.x | Sync scheduling | Retry queue on reconnect |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| QNetworkInformation | Ping-based connectivity check | QNetworkInformation may not work on reMarkable (no NetworkManager); fallback to API error detection |
| JSON file queue | SQLite | JSON simpler for small queue, no additional dependency |
| QSettings for queue | QFile + JSON | QSettings better for simple values; JSON better for structured queue |

**Installation:**
No new dependencies - all functionality available in existing Qt6::Network and Qt6::Core modules. CMakeLists.txt already includes required components.

## Architecture Patterns

### Recommended Project Structure
```
src/
├── network/
│   ├── todoist_client.h/cpp      # Existing - add closeTask()
│   └── sync_manager.h/cpp        # NEW - queue management, connectivity
├── models/
│   ├── task.h/cpp               # Existing - no changes
│   ├── taskmodel.h/cpp          # Existing - add setTaskCompleted()
│   └── sync_queue.h/cpp         # NEW - queue data structure
├── controllers/
│   └── appcontroller.h/cpp      # Existing - add sync status, toggle
qml/
├── main.qml                     # Add sync status indicator
└── TaskDelegate.qml             # Wire checkbox to completeTask()
```

### Pattern 1: Optimistic UI Updates
**What:** Update UI immediately when user taps checkbox, before server confirms
**When to use:** All user-initiated state changes
**Example:**
```cpp
// In AppController
Q_INVOKABLE void completeTask(const QString& taskId) {
    // 1. Optimistic update - immediate visual feedback
    m_taskModel->setTaskCompleted(taskId, true);

    // 2. Queue operation for sync
    m_syncManager->queueTaskCompletion(taskId);

    // 3. Attempt immediate sync if online
    m_syncManager->processQueue();
}
```

### Pattern 2: Queue-Based Sync
**What:** Store operations in persistent FIFO queue, process when connected
**When to use:** Any operation that modifies server state
**Example:**
```cpp
// SyncOperation stored in queue
struct SyncOperation {
    QString uuid;           // Unique ID for idempotency
    QString type;           // "close_task"
    QString taskId;         // Target task
    QDateTime queuedAt;     // When queued
    int retryCount;         // Number of sync attempts
};
```

### Pattern 3: Connectivity-Aware Sync Manager
**What:** Centralize network state detection and queue processing
**When to use:** All sync operations
**Example:**
```cpp
// SyncManager responsibilities
class SyncManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingCountChanged)

public:
    void queueTaskCompletion(const QString& taskId);
    void processQueue();          // Try to sync all pending
    void refreshTasks();          // Pull latest from server

signals:
    void isOnlineChanged();
    void pendingCountChanged();
    void syncFailed(const QString& taskId, const QString& error);
    void syncSucceeded(const QString& taskId);
};
```

### Anti-Patterns to Avoid
- **Direct API calls from QML:** Always go through AppController for state management
- **Blocking UI on sync:** Use async operations, never block main thread
- **Losing queued operations:** Persist queue to disk before attempting sync
- **Polling for connectivity:** Use QNetworkInformation signals or detect via failed requests

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| JSON serialization | Custom parser | QJsonDocument | Handles edge cases, errors |
| App data directory | Hardcoded paths | QStandardPaths::AppDataLocation | Cross-platform, follows conventions |
| UUID generation | Custom IDs | QUuid::createUuid() | RFC-compliant, guaranteed unique |
| Network state | Polling ping | QNetworkInformation (with fallback) | OS-level integration |
| Model updates | Manual view refresh | dataChanged signal | Qt's reactive binding system |

**Key insight:** Qt6 provides robust primitives for offline-first apps. The complexity is in orchestrating them correctly, not in reimplementing them.

## Common Pitfalls

### Pitfall 1: QNetworkInformation Not Available
**What goes wrong:** QNetworkInformation requires a backend plugin (NetworkManager on Linux). reMarkable may not have NetworkManager.
**Why it happens:** QNetworkInformation::loadDefaultBackend() returns false if no suitable plugin found.
**How to avoid:**
1. Try QNetworkInformation first
2. Fallback to "optimistic online" - assume online, detect offline via failed requests
3. Network errors (timeout, connection refused) indicate offline state
**Warning signs:** loadDefaultBackend() returns false, or reachability stays "Unknown"

### Pitfall 2: Lost Queue on App Crash
**What goes wrong:** In-memory queue lost if app crashes before sync
**Why it happens:** Queue only persisted after successful sync
**How to avoid:** Write queue to disk BEFORE attempting any sync operation
**Warning signs:** Users report completed tasks reappearing after restart

### Pitfall 3: Duplicate Task Completions
**What goes wrong:** Same task completed multiple times on server (rate limit issues, confusion)
**Why it happens:** Network timeout but server actually processed request
**How to avoid:** Use UUID for each operation; check if task already completed before re-queuing
**Warning signs:** 409 Conflict errors, rate limit warnings

### Pitfall 4: UI Not Updating After Model Change
**What goes wrong:** Checkbox state doesn't visually update in QML ListView
**Why it happens:** dataChanged signal not emitted, or wrong roles specified
**How to avoid:** Always emit dataChanged with correct index and roles
```cpp
void TaskModel::setTaskCompleted(const QString& taskId, bool completed) {
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId) {
            m_tasks[i].completed = completed;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {CompletedRole});
            return;
        }
    }
}
```
**Warning signs:** Console shows setTaskCompleted called but UI checkbox unchanged

### Pitfall 5: POST Request Body Issues
**What goes wrong:** Todoist API returns error despite correct endpoint
**Why it happens:** Qt's post() needs explicit empty QByteArray for no-body requests
**How to avoid:**
```cpp
// For POST with no body (like /tasks/{id}/close)
QNetworkReply* reply = m_networkManager->post(request, QByteArray());
```
**Warning signs:** 400 Bad Request or content-type errors

### Pitfall 6: Blocking on Network in QML Thread
**What goes wrong:** UI freezes during sync operations
**Why it happens:** Sync logic running synchronously in main thread
**How to avoid:** All network operations async via signals/slots
**Warning signs:** App unresponsive when tapping refresh or completing tasks

## Code Examples

Verified patterns from official sources:

### Todoist Close Task API Call
```cpp
// Source: https://developer.todoist.com/rest/v2/#close-a-task
// POST https://api.todoist.com/rest/v2/tasks/{task_id}/close
// Returns: 204 No Content on success

void TodoistClient::closeTask(const QString& taskId) {
    QString url = QString("https://api.todoist.com/rest/v2/tasks/%1/close").arg(taskId);
    QNetworkRequest request(QUrl(url));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiToken).toUtf8());
    request.setTransferTimeout(REQUEST_TIMEOUT_MS);

    QNetworkReply* reply = m_networkManager->post(request, QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId]() {
        if (reply->error() == QNetworkReply::NoError) {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (statusCode == 204) {
                emit taskClosed(taskId);
            }
        } else {
            emit closeTaskFailed(taskId, handleNetworkError(reply));
        }
        reply->deleteLater();
    });
}
```

### QNetworkInformation Connectivity Detection
```cpp
// Source: https://doc.qt.io/qt-6/qnetworkinformation.html

bool SyncManager::initializeNetworkMonitoring() {
    // Try to load platform-appropriate backend
    if (!QNetworkInformation::loadDefaultBackend()) {
        qWarning() << "QNetworkInformation backend not available, using fallback";
        m_useNetworkInfo = false;
        return false;
    }

    QNetworkInformation* netInfo = QNetworkInformation::instance();
    connect(netInfo, &QNetworkInformation::reachabilityChanged,
            this, &SyncManager::onReachabilityChanged);

    m_useNetworkInfo = true;
    return true;
}

void SyncManager::onReachabilityChanged(QNetworkInformation::Reachability reachability) {
    bool wasOnline = m_isOnline;
    m_isOnline = (reachability == QNetworkInformation::Reachability::Online);

    if (m_isOnline != wasOnline) {
        emit isOnlineChanged();
        if (m_isOnline && m_pendingQueue.size() > 0) {
            processQueue();  // Auto-sync when coming online
        }
    }
}
```

### JSON Queue Persistence
```cpp
// Source: https://doc.qt.io/qt-6/qjsondocument.html

void SyncQueue::saveToFile() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);  // Ensure directory exists

    QJsonArray queueArray;
    for (const SyncOperation& op : m_operations) {
        QJsonObject obj;
        obj["uuid"] = op.uuid;
        obj["type"] = op.type;
        obj["taskId"] = op.taskId;
        obj["queuedAt"] = op.queuedAt.toString(Qt::ISODate);
        obj["retryCount"] = op.retryCount;
        queueArray.append(obj);
    }

    QJsonDocument doc(queueArray);
    QFile file(path + "/sync_queue.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Compact));
    }
}

void SyncQueue::loadFromFile() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile file(path + "/sync_queue.json");

    if (!file.open(QIODevice::ReadOnly)) {
        return;  // No queue file yet
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse queue:" << error.errorString();
        return;
    }

    // Parse operations from JSON...
}
```

### TaskModel Single-Row Update
```cpp
// Source: https://doc.qt.io/qt-6/qabstractitemmodel.html#dataChanged

void TaskModel::setTaskCompleted(const QString& taskId, bool completed) {
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId) {
            if (m_tasks[i].completed != completed) {
                m_tasks[i].completed = completed;
                QModelIndex idx = index(i, 0);
                // Emit dataChanged for just this row and role
                emit dataChanged(idx, idx, {CompletedRole});
            }
            return;
        }
    }
}
```

### QML Sync Status Indicator
```qml
// Sync status bar in header
RowLayout {
    spacing: 16

    // Online/offline indicator
    Rectangle {
        width: 16
        height: 16
        radius: 8
        color: syncManager.isOnline ? "#4CAF50" : "#F44336"
    }

    Text {
        text: {
            if (!syncManager.isOnline) {
                return "Offline"
            } else if (syncManager.pendingCount > 0) {
                return "Syncing " + syncManager.pendingCount + "..."
            } else {
                return "Synced"
            }
        }
        font.pixelSize: 18
        color: mutedColor
    }
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| QNetworkConfigurationManager | QNetworkInformation | Qt 6.1 (2021) | New API for network state |
| QNetworkAccessManager::networkAccessible() | QNetworkInformation::reachability() | Qt 6.1 | More detailed state info |
| Custom JSON parsing | QJsonDocument | Stable since Qt 5 | Prefer over manual parsing |

**Deprecated/outdated:**
- QNetworkConfigurationManager: Removed in Qt 6, replaced by QNetworkInformation
- networkAccessibleChanged signal: No longer available, use QNetworkInformation::reachabilityChanged

**API Migration Note:**
The Todoist REST API v2 (currently used) has a deprecation notice for February 2026. However, the /tasks/{id}/close endpoint works identically in both v2 and the new API v1. Monitor https://developer.todoist.com for migration guidance if issues arise.

## Open Questions

Things that couldn't be fully resolved:

1. **QNetworkInformation on reMarkable**
   - What we know: Requires NetworkManager backend on Linux; reMarkable runs minimal Linux
   - What's unclear: Whether reMarkable's Linux has NetworkManager or compatible backend
   - Recommendation: Implement with fallback - try QNetworkInformation, fall back to error-based detection

2. **Auto-sync on reconnect timing**
   - What we know: SYNC-03 requires syncing when connection restored
   - What's unclear: How quickly reachabilityChanged fires after WiFi reconnect on reMarkable
   - Recommendation: Add short delay (1-2 seconds) after online signal before processing queue

3. **Queue size limits**
   - What we know: Users can complete many tasks offline
   - What's unclear: Practical limit before sync becomes problematic
   - Recommendation: Start with no limit, monitor performance, add limit if needed

## Sources

### Primary (HIGH confidence)
- [Qt6 QNetworkInformation](https://doc.qt.io/qt-6/qnetworkinformation.html) - Connectivity monitoring API
- [Qt6 QJsonDocument](https://doc.qt.io/qt-6/qjsondocument.html) - JSON parsing/serialization
- [Qt6 QStandardPaths](https://doc.qt.io/qt-6/qstandardpaths.html) - App data directories
- [Qt6 QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) - Model update signals
- [Todoist REST API v2](https://developer.todoist.com/rest/v2/) - Close task endpoint

### Secondary (MEDIUM confidence)
- [Qt Forum: Network connectivity checking](https://forum.qt.io/topic/98676/how-to-check-whether-the-computer-connects-the-internet) - Fallback strategies
- [Qt Forum: dataChanged signal](https://forum.qt.io/topic/76048/qabstractlistmodel-datachanged) - Model update patterns
- [Offline-First Architecture](https://medium.com/@jusuftopic/offline-first-architecture-designing-for-reality-not-just-the-cloud-e5fd18e50a79) - Queue and sync patterns

### Tertiary (LOW confidence)
- [reMarkable network info](https://remarkable.jms1.info/info/network.html) - Device network capabilities (community source)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Using Qt6 modules already in project
- Architecture: HIGH - Well-established offline-first patterns
- Pitfalls: MEDIUM - QNetworkInformation on reMarkable untested
- Code examples: HIGH - Based on official Qt documentation

**Research date:** 2026-02-02
**Valid until:** 2026-03-02 (30 days - stable Qt6 APIs)
