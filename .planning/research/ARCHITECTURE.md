# Architecture Patterns: reMarkable 2 Todoist Client

**Domain:** E-ink tablet task management application
**Stack:** C++17, Qt 5.15, Todoist REST API v2, JSON storage, rm2fb
**Researched:** 2026-01-29
**Confidence:** HIGH (reMarkable patterns), HIGH (Qt patterns), HIGH (sync architecture)

## Executive Summary

This application combines three architectural domains: reMarkable e-ink UI constraints, Qt application structure, and offline-first sync. The architecture follows Qt's Model-View pattern with a dedicated sync engine running on a worker thread. Components communicate via Qt signals/slots, with JSON files providing durable storage and an in-memory cache for UI responsiveness.

**Key architectural decisions:**
- **Single-threaded UI, background sync:** UI runs on main thread with rm2fb updates; sync operations run on QThread to prevent blocking
- **Offline-first with command queue:** All modifications queue locally first, sync in background
- **JSON file storage:** Durable, human-readable, debuggable persistence without database overhead
- **Partial framebuffer updates:** Minimize e-ink refresh latency by updating only changed regions

---

## Recommended Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         UI Layer (Main Thread)               │
│  ┌──────────────────┐         ┌──────────────────┐         │
│  │   TaskListView   │         │   AddTaskView    │         │
│  │   (QML/Widget)   │         │   (QML/Widget)   │         │
│  └────────┬─────────┘         └────────┬─────────┘         │
│           │                            │                    │
│           └──────────┬─────────────────┘                    │
│                      │                                      │
│           ┌──────────▼─────────┐                           │
│           │   ViewController   │                           │
│           │  (Controller.h)    │                           │
│           └──────────┬─────────┘                           │
└──────────────────────┼──────────────────────────────────────┘
                       │ Qt Signals/Slots
┌──────────────────────▼──────────────────────────────────────┐
│                    Business Logic Layer                      │
│           ┌────────────────────┐                            │
│           │    TaskManager     │                            │
│           │  (In-memory cache) │                            │
│           └────────┬───────────┘                            │
│                    │                                        │
│        ┌───────────┼───────────┐                           │
│        │                       │                           │
│  ┌─────▼──────┐        ┌──────▼─────┐                     │
│  │ JSONStore  │        │ SyncEngine │                      │
│  │ (persist)  │        │ (worker)   │                      │
│  └─────┬──────┘        └──────┬─────┘                     │
└────────┼──────────────────────┼──────────────────────────────┘
         │                      │
         │ QFile I/O            │ HTTPS
         │                      │
    ┌────▼────┐          ┌─────▼─────────┐
    │  JSON   │          │ Todoist REST  │
    │  Files  │          │   API v2      │
    └─────────┘          └───────────────┘
```

### Component Boundaries

| Component | Responsibility | Communicates With | Thread |
|-----------|---------------|-------------------|--------|
| **TaskListView** | Display tasks with checkboxes, handle taps, trigger partial e-ink updates | ViewController | Main |
| **AddTaskView** | Handwriting input area, convert strokes to text, show preview | ViewController, HandwritingRecognizer | Main |
| **ViewController** | Coordinate views, route user actions to business logic, manage navigation | Views, TaskManager | Main |
| **TaskManager** | In-memory task cache, filter/sort operations, emit data change signals | ViewController, JSONStore, SyncEngine | Main |
| **JSONStore** | Read/write tasks.json, projects.json; atomic file operations | TaskManager | Main (I/O blocking) |
| **SyncEngine** | REST API calls, offline queue processing, conflict resolution | TaskManager, Todoist API | Worker (QThread) |
| **HandwritingRecognizer** | Process pen strokes, call recognition engine, return text | AddTaskView | Worker (QThread) |

---

## Data Flow

### Read Path (App Launch)

```
1. TaskManager.init()
   ├─> JSONStore.loadTasks() → parse tasks.json → return QVector<Task>
   ├─> JSONStore.loadProjects() → parse projects.json → return QVector<Project>
   └─> TaskManager caches data in memory

2. ViewController requests data
   ├─> TaskManager.getAllTasks() → filter/sort cached tasks
   └─> emit tasksChanged() signal

3. TaskListView receives signal
   └─> Update UI with partial framebuffer refresh
```

**Key point:** All reads come from in-memory cache. No I/O blocking during scrolling/filtering.

### Write Path (User Checks Task)

```
1. User taps checkbox in TaskListView
   └─> ViewController.onTaskCompleted(taskId)

2. TaskManager.completeTask(taskId)
   ├─> Update in-memory cache (mark completed)
   ├─> JSONStore.saveTasks() → write tasks.json atomically
   ├─> SyncEngine.queueCommand({type: "item_close", id: taskId, uuid: generateUUID()})
   └─> emit taskCompleted(taskId) signal

3. TaskListView receives signal
   └─> Strike through task, partial e-ink update

4. SyncEngine (background thread)
   ├─> Process queue when online
   ├─> POST /rest/v2/tasks/{taskId}/close
   ├─> On success: remove command from queue, persist queue
   └─> On failure: retry with exponential backoff
```

**Key point:** UI updates immediately from cache. Network sync happens asynchronously.

### Sync Path (Background Refresh)

```
1. SyncEngine.syncTasks() (triggered by timer or manual refresh)
   ├─> GET /rest/v2/tasks → receive all active tasks
   ├─> GET /rest/v2/projects → receive all projects
   └─> Return results to main thread via signal

2. TaskManager receives syncCompleted(tasks, projects)
   ├─> Merge with local cache (resolve conflicts)
   ├─> JSONStore.saveTasks() → persist merged state
   └─> emit tasksChanged() signal

3. TaskListView refreshes
   └─> Full or partial e-ink update depending on change magnitude
```

**Conflict resolution:** Last-write-wins based on timestamp. Local pending commands always take precedence over server state.

---

## Patterns to Follow

### Pattern 1: Qt Model-View with Controller

**What:** Separate data (TaskManager) from presentation (Views) with a controller mediating.

**When:** Always for Qt applications. Enables testability and keeps QML/Widget code thin.

**Example:**
```cpp
// controller.h
class Controller : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void completeTask(int taskId);
    Q_INVOKABLE QVariantList getTasks();

signals:
    void tasksChanged();

private:
    TaskManager* m_taskManager;
};

// TaskListView.qml
ListView {
    model: controller.getTasks()
    delegate: TaskItem {
        onChecked: controller.completeTask(taskId)
    }
}
```

### Pattern 2: Single QNetworkAccessManager Instance

**What:** One QNetworkAccessManager shared across all HTTP requests.

**When:** Always for REST API clients. Enables connection reuse and request queuing.

**Example:**
```cpp
// syncengine.h
class SyncEngine : public QObject {
private:
    QNetworkAccessManager* m_networkManager; // Single instance

    QNetworkReply* getTasks() {
        QNetworkRequest req(QUrl("https://api.todoist.com/rest/v2/tasks"));
        req.setRawHeader("Authorization", "Bearer " + m_token);
        return m_networkManager->get(req); // Reuses connections
    }
};
```

**Remember:** Always call `reply->deleteLater()` to avoid leaks. Use reply signals, not manager signals.

### Pattern 3: Worker Object for Background Sync

**What:** Move SyncEngine to QThread using `moveToThread()`, not subclass QThread.

**When:** Always for long-running or blocking operations (network I/O, handwriting recognition).

**Example:**
```cpp
// main.cpp
SyncEngine* syncEngine = new SyncEngine();
QThread* workerThread = new QThread();

syncEngine->moveToThread(workerThread);
connect(workerThread, &QThread::started, syncEngine, &SyncEngine::init);
connect(syncEngine, &SyncEngine::syncCompleted, taskManager, &TaskManager::mergeTasks);

workerThread->start();
```

**Communication:** Use queued signals/slots across threads. Qt handles thread-safe delivery automatically.

### Pattern 4: Atomic JSON File Writes

**What:** Write to temporary file, then atomic rename to prevent corruption on crash.

**When:** Always when persisting critical data.

**Example:**
```cpp
bool JSONStore::saveTasks(const QVector<Task>& tasks) {
    QJsonArray array;
    for (const Task& task : tasks) {
        array.append(task.toJson());
    }

    QJsonDocument doc(array);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    // Write to temp file
    QFile tempFile(m_dataPath + "/tasks.json.tmp");
    if (!tempFile.open(QIODevice::WriteOnly)) return false;
    tempFile.write(data);
    tempFile.close();

    // Atomic rename
    QFile::remove(m_dataPath + "/tasks.json");
    return tempFile.rename(m_dataPath + "/tasks.json");
}
```

### Pattern 5: Command Queue with UUIDs for Idempotency

**What:** Each command gets a unique UUID. Server ignores duplicate UUIDs.

**When:** Always for offline-first sync to enable safe retries.

**Example:**
```cpp
struct SyncCommand {
    QString uuid;        // QUuid::createUuid().toString()
    QString type;        // "item_add", "item_close", etc.
    QJsonObject args;    // Command parameters
    QDateTime createdAt;
};

void SyncEngine::queueCommand(const SyncCommand& cmd) {
    m_commandQueue.append(cmd);
    saveCommandQueue(); // Persist immediately
}

void SyncEngine::processQueue() {
    for (const SyncCommand& cmd : m_commandQueue) {
        QJsonObject payload;
        payload["commands"] = QJsonArray{cmd.toJson()};

        // Server checks UUID; duplicates are no-ops
        QNetworkReply* reply = postSync(payload);
        // ... handle response, remove on success
    }
}
```

### Pattern 6: Partial Framebuffer Updates for E-ink

**What:** Tell rm2fb to refresh only the changed rectangle, not full screen.

**When:** For checkbox toggles, single task updates. Full refresh on screen change or after ~10 partials.

**Example:**
```cpp
// Pseudo-code (rm2fb uses ioctl, not Qt API)
void TaskListView::updateTask(int index) {
    QRect taskRect = getTaskRect(index); // e.g., (0, 80*index, 1404, 80)

    // Draw to framebuffer
    drawTask(taskRect, m_tasks[index]);

    // Partial refresh (fast, ~300ms)
    mxcfb_update_data update;
    update.update_region = {taskRect.x(), taskRect.y(), taskRect.width(), taskRect.height()};
    update.waveform_mode = WAVEFORM_MODE_DU; // Fast partial
    ioctl(m_fbFd, MXCFB_SEND_UPDATE, &update);
}
```

**Note:** rm2fb abstracts this via `/dev/shm/swtfb*` and message queue. See ddvk/remarkable2-framebuffer docs.

---

## Anti-Patterns to Avoid

### Anti-Pattern 1: Synchronous Network Calls on Main Thread

**What:** Calling `QNetworkAccessManager` methods and waiting for response before updating UI.

**Why bad:** Blocks UI for 100ms-5s, freezes e-ink refresh, terrible UX.

**Instead:** Always use async signals/slots. Update UI from cache immediately, sync in background.

```cpp
// BAD
QNetworkReply* reply = manager->get(request);
while (!reply->isFinished()) {
    QCoreApplication::processEvents(); // Blocks!
}
QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
```

```cpp
// GOOD
QNetworkReply* reply = manager->get(request);
connect(reply, &QNetworkReply::finished, this, [reply]() {
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    emit syncCompleted(doc);
    reply->deleteLater();
});
```

### Anti-Pattern 2: Full Screen Refresh on Every Change

**What:** Triggering full e-ink refresh (1-2 seconds, black flash) for small updates like checkbox toggle.

**Why bad:** Sluggish UX, accelerates e-ink ghosting, wastes battery.

**Instead:** Use partial updates (DU waveform) for small changes, full refresh (GC16) only on screen transitions or after 10-20 partials.

### Anti-Pattern 3: Multiple QNetworkAccessManager Instances

**What:** Creating new `QNetworkAccessManager` for each request or component.

**Why bad:** Prevents HTTP connection reuse, increases latency, wastes memory.

**Instead:** One global instance, shared via dependency injection or singleton.

### Anti-Pattern 4: Directly Subclassing QThread

**What:** Inheriting from `QThread` and overriding `run()` with business logic.

**Why bad:** No event loop in thread means no signals/slots. Hard to extend.

**Instead:** Use worker object pattern (`moveToThread()`).

```cpp
// BAD
class SyncThread : public QThread {
    void run() override {
        // Logic here; can't use signals/slots easily
    }
};
```

```cpp
// GOOD
class SyncEngine : public QObject { /* ... */ };
SyncEngine* engine = new SyncEngine();
QThread* thread = new QThread();
engine->moveToThread(thread);
thread->start();
```

### Anti-Pattern 5: Ignoring rm2fb Client Library

**What:** Trying to write directly to `/dev/fb0` on reMarkable 2.

**Why bad:** reMarkable 2 doesn't expose `/dev/fb0`. The framebuffer is managed by proprietary `SWTCON` driver.

**Instead:** Use rm2fb shim (`/dev/shm/swtfb*`) which translates rM1-style framebuffer calls to rM2's SWTCON API. Link against `librm2fb_client.so` or use message queue IPC.

---

## Scalability Considerations

| Concern | At 100 tasks | At 1,000 tasks | At 10,000 tasks |
|---------|--------------|----------------|-----------------|
| **Memory** | 50KB in-memory cache | 500KB cache | 5MB cache (acceptable for 512MB RAM device) |
| **Sync time** | 1-2s full sync | 5-10s full sync | 30-60s full sync; consider incremental sync |
| **UI rendering** | Instant list load | Instant with Qt model/view virtualization | Must use QAbstractItemModel with lazy loading |
| **JSON I/O** | 10ms read/write | 50ms read/write | 300ms read/write; consider switching to SQLite |
| **Offline queue** | 10 commands typical | 50-100 commands possible | May hit Todoist batch limit (100 commands/request) |

### Recommended Thresholds

- **100-500 tasks:** Simple `QVector<Task>` with `QListView` is fine
- **500-2,000 tasks:** Use `QAbstractItemModel` for virtualized rendering
- **2,000+ tasks:** Switch to SQLite for storage, incremental sync (requires Sync API v9, not REST API v2)

**For MVP:** Target 100-500 tasks. Most Todoist users have <200 active tasks.

---

## Component Build Order

Build order based on dependencies, enabling incremental testing.

### Phase 1: Data Foundation
1. **Task/Project models** (`task.h`, `project.h`)
   - Plain structs with `toJson()/fromJson()` serializers
   - No dependencies
   - Testable: Unit tests for serialization

2. **JSONStore** (`jsonstore.h`)
   - Depends on: Task/Project models
   - Testable: File I/O with temp directory

3. **TaskManager** (`taskmanager.h`)
   - Depends on: Models, JSONStore
   - Testable: Mock JSONStore, test caching logic

### Phase 2: Network Layer
4. **SyncEngine** (`syncengine.h`)
   - Depends on: Models, TaskManager
   - Testable: Mock QNetworkAccessManager with QSignalSpy

5. **Offline Queue** (`commandqueue.h`)
   - Depends on: SyncEngine
   - Testable: Persist/load queue, test retry logic

### Phase 3: UI Foundation
6. **rm2fb Integration** (`framebuffer.h`)
   - No dependencies (just platform layer)
   - Testable: On-device only; check for artifacts

7. **Controller** (`controller.h`)
   - Depends on: TaskManager, SyncEngine
   - Exposes Q_INVOKABLE methods for QML
   - Testable: Unit tests for signal emission

### Phase 4: UI Implementation
8. **TaskListView** (QML or Qt Widget)
   - Depends on: Controller, rm2fb
   - Testable: Visual inspection on-device

9. **AddTaskView** (QML or Qt Widget)
   - Depends on: Controller, HandwritingRecognizer
   - Testable: Visual inspection on-device

### Phase 5: Polish
10. **Handwriting Recognition**
    - Integrate Qt Virtual Keyboard or custom recognizer
    - Testable: Accuracy tests with sample strokes

11. **Oxide Integration** (`myapp.oxide`)
    - Register app with Oxide launcher
    - Add icon, metadata
    - Testable: Launch from Oxide

### Testing Strategy per Phase

- **Phases 1-2:** Desktop unit tests with Qt Test framework
- **Phase 3:** On-device smoke test (does app launch?)
- **Phase 4:** On-device integration test (can user complete task?)
- **Phase 5:** Full manual QA pass

**Critical path:** Phases 1-2 must be solid before moving to UI. Network bugs discovered during UI implementation are expensive.

---

## Application Deployment

### Stock Firmware (No Toltec)

On newer reMarkable OS versions where Toltec isn't supported, use one of these approaches:

#### Option 1: Launcher Notebook (Recommended)

A background service watches for a specific notebook to be opened, then switches from Xochitl to the app.

**How it works:**
1. Systemd service runs `todoist-launcher.sh` in background
2. Service uses `inotifywait` to monitor for "Launch Todoist" notebook access
3. When notebook opened: stop Xochitl → run app → restart Xochitl on exit

**Files:**
```
/opt/bin/remarkable-todoist           # Main executable
/opt/bin/todoist-launcher.sh          # Watcher script
/etc/systemd/system/todoist-launcher.service  # Systemd unit
```

**Requires:** `inotifywait` (from inotify-tools) on device

#### Option 2: Manual SSH Launch

```bash
systemctl stop xochitl
export QT_QPA_PLATFORM=epaper
export QT_QUICK_BACKEND=epaper
./remarkable-todoist
systemctl start xochitl
```

#### Option 3: Autostart Service

Create a systemd service that runs the app on boot, replacing Xochitl.

---

### Oxide/Toltec (If Supported)

For devices with Toltec/Oxide installed:

#### Required Files

```
/opt/bin/remarkable-todoist          # Main executable
/opt/etc/draft/remarkable-todoist.json  # Oxide app metadata
/opt/usr/share/applications/remarkable-todoist.oxide  # Desktop entry
```

#### remarkable-todoist.json Example

```json
{
  "name": "Todoist",
  "description": "Task management with Todoist sync",
  "bin": "/opt/bin/remarkable-todoist",
  "icon": "todo",
  "term": false,
  "type": "foreground",
  "displayName": "Todoist",
  "permissions": ["wifi", "power"]
}
```

#### Toltec Package Structure

```
package/
├── usr/
│   └── bin/
│       └── remarkable-todoist        # Binary
├── etc/
│   └── draft/
│       └── remarkable-todoist.json   # Config
└── usr/
    └── share/
        └── applications/
            └── remarkable-todoist.oxide
```

---

## Build Environment

### On-Device Building (Current Approach)

Cross-compilation from desktop is problematic because:
- Official reMarkable toolchain (v3.1.15) lacks Qt6 Quick/QML libraries
- Toltec Docker images have Qt5, not Qt6
- Library version mismatches cause linker failures

**Solution:** Build directly on the reMarkable device where Qt6 is already installed.

```bash
# On device
cd ~/Remarkable_Todoist
make
```

### Cross-Compilation (Not Currently Working)

If cross-compilation is needed in the future, the toolchain setup is:

```bash
# Download official toolchain
source ~/remarkable-toolchain/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi

# Build with CMake
cmake -B build-rm -DCMAKE_TOOLCHAIN_FILE=cmake/remarkable.cmake
cmake --build build-rm
```

The issue is that the sysroot must contain matching Qt6 Quick/QML libraries, which aren't currently available in the official or Toltec toolchains.

---

## Sources

### reMarkable Development
- [reMarkable Qt Framework Guide](https://remarkable.guide/devel/language/c++/qt.html)
- [Writing a Simple reMarkable Application](https://eeems.website/writing-a-simple-oxide-application/)
- [reMarkable Qt Template App](https://github.com/Eeems-Org/remarkable-template-qt-app)
- [Oxide Desktop Environment](https://github.com/Eeems-Org/oxide)
- [rm2fb Framebuffer Architecture](https://github.com/ddvk/remarkable2-framebuffer)

### Qt Architecture Patterns
- [Model/View Programming | Qt Widgets](https://doc.qt.io/qt-6/model-view-programming.html)
- [Separation of Model, View and Logic Code | Felgo](https://felgo.com/doc/apps-howto-model-view-separation/)
- [RESTful Client Applications in Qt 6.7](https://www.qt.io/blog/restful-client-applications-in-qt-6.7-and-forward)
- [QNetworkAccessManager Best Practices](https://www.volkerkrause.eu/2022/11/19/qt-qnetworkaccessmanager-best-practices.html)
- [Qt Multithreading in C++ | Toptal](https://www.toptal.com/qt/qt-multithreading-c-plus-plus)
- [Threading Basics | Qt 5.15](https://doc.qt.io/qt-5/thread-basics.html)

### JSON Storage
- [JSON Support in Qt | Qt Core](https://doc.qt.io/qt-6/json.html)
- [QJsonDocument Class | Qt Core 5.15](https://doc.qt.io/qt-5/qjsondocument.html)
- [Qt/C++ - Work with QJsonObject, QJsonArray, QJsonDocument](https://evileg.com/en/post/419/)

### Offline-First Sync
- [State Management for Offline-First Web Applications](https://blog.pixelfreestudio.com/state-management-for-offline-first-web-applications/)
- [Build an offline-first app | Android Developers](https://developer.android.com/topic/architecture/data-layer/offline-first)
- [Offline-First Architecture: Designing for Reality](https://medium.com/@jusuftopic/offline-first-architecture-designing-for-reality-not-just-the-cloud-e5fd18e50a79)

### Todoist API
- [REST API Reference | Todoist Developer](https://developer.todoist.com/rest/v2/)
- [Sync API Reference | Todoist Developer](https://developer.todoist.com/sync/v9/)

### E-ink Display Optimization
- [Qt Framebuffer Partial Update](https://doc.qt.io/archives/QtForMCUs-2.4/platform-porting-guide-partial-framebuffer.html)
- [E-paper Display Using Partial Updates](https://www.hackster.io/galoebn/e-paper-display-using-partial-updates-a8af20)

### Handwriting Recognition
- [Handwriting Recognition | Qt Virtual Keyboard](https://doc.qt.io/qt-6/handwriting.html)
- [Qt Virtual Keyboard Updated with Handwriting Recognition](https://www.qt.io/blog/2016/03/02/qt-virtual-keyboard-updated-with-handwriting-recognition)

---

## Confidence Assessment

| Category | Level | Rationale |
|----------|-------|-----------|
| **reMarkable Patterns** | HIGH | Official docs + template apps + rm2fb documentation |
| **Qt Architecture** | HIGH | Official Qt docs + established patterns + multiple sources |
| **Offline Sync** | HIGH | Industry standard patterns + Todoist API docs |
| **E-ink Optimization** | MEDIUM | rm2fb documented, but partial update tuning requires on-device testing |
| **Handwriting Integration** | MEDIUM | Qt Virtual Keyboard documented, but accuracy/performance TBD |

**Overall confidence:** HIGH for core architecture, MEDIUM for device-specific optimizations requiring empirical testing.

---

## Notes for Roadmap Creation

### Recommended Phase Structure

1. **Foundation Phase:** Build TaskManager + JSONStore + Models. Desktop unit tests only. No UI, no network.
2. **Sync Phase:** Add SyncEngine + offline queue. Desktop integration tests with mock API.
3. **UI Phase:** Build TaskListView with rm2fb. On-device testing starts here.
4. **Input Phase:** Add AddTaskView + handwriting recognition. High risk; may need fallback to keyboard input.
5. **Polish Phase:** Oxide integration, icon, error handling, offline UX.

### Phases Likely to Need Deeper Research

- **Phase 4 (Input):** Handwriting recognition accuracy/latency unknown until tested on-device. May need custom recognizer or compromise on feature.
- **Phase 3 (UI):** rm2fb partial update performance tuning is iterative; may require experimentation with waveform modes.

### Phases with Standard Patterns (Low Research Risk)

- **Phase 1 (Foundation):** Pure C++, well-documented Qt patterns.
- **Phase 2 (Sync):** Standard offline-first architecture, well-documented API.
