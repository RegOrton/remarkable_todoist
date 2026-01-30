# Phase 1: Foundation & Task Display - Research

**Researched:** 2026-01-30
**Domain:** Qt 5.15 Widgets application for reMarkable 2 e-ink tablet
**Confidence:** HIGH

## Summary

Phase 1 builds the foundation for displaying Todoist tasks on the reMarkable 2. This involves three technical domains: Qt 5.15 Widgets for UI, Todoist REST API v2 for data fetching, and rm2fb for e-ink display compatibility. The standard approach is Qt Model-View architecture with QAbstractListModel providing data to QListView, with custom delegates for rendering task rows.

The reMarkable 2 platform imposes specific constraints: 1GB RAM, e-ink display requiring refresh management, stylus-first input requiring 48x48dp minimum touch targets, and rm2fb shim required for framebuffer access. Configuration storage uses Qt's QSettings for INI-based API token storage. JSON parsing uses Qt's built-in QJsonDocument classes.

Authentication is intentionally simple for Phase 1: plaintext API token stored in ~/.config with restrictive file permissions. This is adequate for the device's single-user, physical-security model.

**Primary recommendation:** Use Qt Model-View with QAbstractListModel + QListView + QStyledItemDelegate. Design for high contrast (black on white), large touch targets (48x48dp minimum), and static layouts. Store API token in QSettings with 600 permissions. Use partial e-ink refreshes for individual task updates, full refresh every 5 partials or on screen transitions.

## Standard Stack

The established libraries/tools for this domain:

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| Qt Widgets | 5.15 | UI framework | Pre-installed on reMarkable 2, provides Model-View architecture, built-in networking and JSON |
| QNetworkAccessManager | Qt 5.15 | HTTP client | Built-in async HTTP client with SSL support, connection pooling, perfect for REST APIs |
| QJsonDocument | Qt 5.15 | JSON parsing | Built-in, fast, integrates with Qt types (QString, QVariant) |
| QSettings | Qt 5.15 | Config storage | Simple INI file management, handles paths/permissions automatically |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| rm2fb | Latest via Toltec | Framebuffer shim | REQUIRED on reMarkable 2 - without it, app cannot display to screen |
| QAbstractListModel | Qt 5.15 | Data model | For task list - provides virtual scrolling, efficient updates |
| QStyledItemDelegate | Qt 5.15 | Custom rendering | For task row layout with checkbox, text, metadata |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Qt Widgets | Qt Quick (QML) | Official reMarkable docs recommend Qt Quick, but community successfully uses Widgets; Widgets more stable on Qt 5.15 |
| QSettings plaintext | Encrypted storage | Device security model is physical-access-based; encryption adds complexity for minimal gain |
| QJsonDocument | Third-party JSON lib | QJsonDocument is built-in, fast enough for <1000 tasks, reduces dependencies |
| REST API v2 | Sync API v9 | REST API v2 is simpler for read-heavy workload; Sync API is complex and deprecated |

**Note on Qt Widgets vs Qt Quick:** Official reMarkable developer documentation states "only pure Qt Quick applications (no Qt Widgets) are supported," but community evidence shows Qt Widgets works fine with rm2fb. The project decision to use Qt Widgets (from STACK.md) is based on community validation and Qt 5.15 maturity.

**Installation:**
```bash
# On device (via Toltec)
opkg update
opkg install display  # rm2fb compatibility layer

# Development machine (cross-compile)
# Qt 5.15 is included in Toltec toolchain Docker image
docker pull ghcr.io/toltec-dev/toolchain:latest
```

## Architecture Patterns

### Recommended Project Structure
```
src/
├── main.cpp                # Entry point, QApplication setup
├── models/
│   ├── task.h/.cpp         # Task data structure with JSON serialization
│   ├── taskmodel.h/.cpp    # QAbstractListModel implementation
│   └── project.h/.cpp      # Project metadata structure
├── views/
│   ├── tasklistview.h/.cpp # QListView subclass with rm2fb refresh
│   └── taskdelegate.h/.cpp # QStyledItemDelegate for task rows
├── controllers/
│   └── appcontroller.h/.cpp # Mediates between view and model
├── network/
│   └── todoist_client.h/.cpp # REST API wrapper
└── config/
    └── settings.h/.cpp      # QSettings wrapper for token storage
```

### Pattern 1: Qt Model-View Architecture
**What:** Separate data (QAbstractListModel) from presentation (QListView) with controller mediation.

**When to use:** Always for list-based UIs in Qt. Enables efficient scrolling (only visible items rendered), clean separation of concerns, and testability.

**Example:**
```cpp
// taskmodel.h
class TaskModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum TaskRoles {
        TitleRole = Qt::UserRole + 1,
        DueDateRole,
        ProjectRole,
        PriorityRole,
        CompletedRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTasks(const QVector<Task>& tasks);

private:
    QVector<Task> m_tasks;
};

// main.cpp
TaskModel* model = new TaskModel();
QListView* view = new QListView();
view->setModel(model);

// Model automatically notifies view when data changes
model->setTasks(fetchedTasks); // View updates automatically
```

**Why this pattern:** QListView virtualizes rendering - only visible rows are created. With 100+ tasks, this saves memory and makes scrolling instant. Manual widget creation for each task would consume excessive RAM.

### Pattern 2: Custom Delegate for Task Rows
**What:** Subclass QStyledItemDelegate to control how each task row is drawn and handle checkbox interactions.

**When to use:** When list items need custom layouts beyond simple text - checkboxes, icons, multiple lines, custom spacing.

**Example:**
```cpp
// taskdelegate.h
class TaskDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
signals:
    void taskCheckboxToggled(int row);
};

// Implementation paint() method
void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const {
    // Draw checkbox (48x48 minimum for stylus)
    QRect checkboxRect(option.rect.left() + 10,
                       option.rect.top() + (option.rect.height() - 48) / 2,
                       48, 48);

    bool completed = index.data(TaskModel::CompletedRole).toBool();
    painter->drawRect(checkboxRect);
    if (completed) {
        painter->drawLine(checkboxRect.topLeft(), checkboxRect.bottomRight());
        painter->drawLine(checkboxRect.topRight(), checkboxRect.bottomLeft());
    }

    // Draw task title (bold, 16pt for readability)
    QRect textRect = option.rect.adjusted(70, 5, -10, -5);
    painter->setFont(QFont("Sans", 16, QFont::Bold));
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                      index.data(TaskModel::TitleRole).toString());

    // Draw metadata (project, due date, priority)
    // ... additional drawing code
}

// sizeHint ensures adequate touch target
QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    return QSize(option.rect.width(), 80); // 80px row height
}
```

**Why this pattern:** Delegates are reused as list scrolls (only ~10 instances for visible rows), unlike creating 100+ custom widgets. Delegates have direct access to paint events for e-ink optimization.

### Pattern 3: Todoist REST API v2 Integration
**What:** Use QNetworkAccessManager for async HTTP requests to Todoist REST API v2 with Bearer token authentication.

**When to use:** Always for REST API communication in Qt. Provides async/non-blocking requests, connection reuse, SSL validation.

**Example:**
```cpp
// todoist_client.h
class TodoistClient : public QObject {
    Q_OBJECT
public:
    TodoistClient(const QString& apiToken, QObject* parent = nullptr);
    void fetchAllTasks();

signals:
    void tasksFetched(const QVector<Task>& tasks);
    void errorOccurred(const QString& error);

private slots:
    void onTasksReplyFinished();

private:
    QNetworkAccessManager* m_networkManager;
    QString m_apiToken;
};

// todoist_client.cpp
TodoistClient::TodoistClient(const QString& apiToken, QObject* parent)
    : QObject(parent), m_apiToken(apiToken) {
    m_networkManager = new QNetworkAccessManager(this);
}

void TodoistClient::fetchAllTasks() {
    QUrl url("https://api.todoist.com/rest/v2/tasks");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_apiToken).toUtf8());

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished,
            this, &TodoistClient::onTasksReplyFinished);
}

void TodoistClient::onTasksReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray tasksArray = doc.array();

        QVector<Task> tasks;
        for (const QJsonValue& value : tasksArray) {
            tasks.append(Task::fromJson(value.toObject()));
        }
        emit tasksFetched(tasks);
    } else {
        emit errorOccurred(reply->errorString());
    }

    reply->deleteLater(); // Critical: prevent memory leak
}
```

**Why this pattern:** Single QNetworkAccessManager instance enables HTTP connection reuse (faster requests). Async signals prevent UI blocking. Bearer token in header is Todoist's standard auth method.

**IMPORTANT:** Todoist REST API v2 is deprecated and will be shut down in February 2026. However, the new unified API v1 has the same authentication and similar endpoints. For Phase 1, REST API v2 is adequate for testing, but migration to v1 must happen before production deployment.

### Pattern 4: QSettings for Configuration Storage
**What:** Store API token in INI file using QSettings with restrictive file permissions.

**When to use:** Always for simple key-value configuration in Qt applications.

**Example:**
```cpp
// settings.h
class AppSettings {
public:
    static QString getApiToken();
    static void setApiToken(const QString& token);
    static bool hasApiToken();

private:
    static QSettings* instance();
};

// settings.cpp
QSettings* AppSettings::instance() {
    static QSettings settings(
        QSettings::IniFormat,
        QSettings::UserScope,
        "remarkable-todoist",
        "config"
    );
    return &settings;
}

QString AppSettings::getApiToken() {
    return instance()->value("auth/api_token").toString();
}

void AppSettings::setApiToken(const QString& token) {
    instance()->setValue("auth/api_token", token);
    instance()->sync(); // Force write to disk

    // Set restrictive permissions (600 = owner read/write only)
    QFile configFile(instance()->fileName());
    configFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}

bool AppSettings::hasApiToken() {
    return !getApiToken().isEmpty();
}
```

**File location:** QSettings with UserScope stores in `~/.config/remarkable-todoist/config.ini` on Linux.

**Why this pattern:** QSettings abstracts platform-specific config paths. INI format is human-readable for debugging. File permissions prevent other users from reading token (though reMarkable is single-user).

### Pattern 5: E-ink Refresh Management
**What:** Use partial refreshes for individual task updates, full refresh periodically or on screen changes.

**When to use:** Always on e-ink displays to balance responsiveness vs ghosting.

**Example:**
```cpp
// tasklistview.h
class TaskListView : public QListView {
    Q_OBJECT
public:
    TaskListView(QWidget* parent = nullptr);

protected:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;

private:
    void triggerPartialRefresh(const QRect& region);
    void triggerFullRefresh();

    int m_partialRefreshCount = 0;
    static const int MAX_PARTIAL_REFRESHES = 5;
};

// tasklistview.cpp
void TaskListView::dataChanged(const QModelIndex &topLeft,
                                const QModelIndex &bottomRight,
                                const QVector<int> &roles) {
    QListView::dataChanged(topLeft, bottomRight, roles);

    // Determine refresh type based on counter
    if (m_partialRefreshCount >= MAX_PARTIAL_REFRESHES) {
        triggerFullRefresh();
        m_partialRefreshCount = 0;
    } else {
        // Calculate affected rectangle
        QRect updateRect = visualRect(topLeft);
        if (topLeft != bottomRight) {
            updateRect = updateRect.united(visualRect(bottomRight));
        }
        triggerPartialRefresh(updateRect);
        m_partialRefreshCount++;
    }
}

void TaskListView::triggerPartialRefresh(const QRect& region) {
    // rm2fb handles partial refresh via Qt's update() mechanism
    // Qt EPaper platform plugin translates to appropriate waveform
    update(region);
}

void TaskListView::triggerFullRefresh() {
    // Force full screen refresh
    // Implementation depends on rm2fb integration method
    repaint(); // Full widget repaint
}
```

**Why this pattern:** Partial refreshes are fast (~300ms) but accumulate ghosting. Full refreshes eliminate ghosting but cause black flash and take 1-2 seconds. Ratio of 5:1 partial:full balances responsiveness and clarity.

**Note:** The exact rm2fb API for controlling refresh modes may require additional integration. Qt's EPaper platform plugin provides some automatic optimization, but manual control may be needed for optimal results.

### Anti-Patterns to Avoid
- **Manual widget creation per task:** Creating a QWidget for each task (e.g., 100 QWidget instances) wastes memory and slows scrolling. Use Model-View with virtual scrolling.
- **Synchronous HTTP requests on main thread:** Using `QEventLoop` to wait for QNetworkReply blocks UI, freezes display updates. Always use async signals/slots.
- **Hardcoding API token in source:** Never embed tokens in code or commit to git. Use QSettings with runtime configuration.
- **Touch targets <48dp:** Small checkboxes (<48x48dp) are frustrating with stylus input. reMarkable stylus tip is ~2mm; 48dp provides adequate tolerance.
- **Color-dependent UI:** Using color alone to indicate priority (red=high, green=low) fails on monochrome e-ink. Use icons, bold text, or patterns.

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| HTTP client with SSL | Custom socket + OpenSSL wrapper | QNetworkAccessManager | Handles connection pooling, redirects, SSL cert validation, proxy support, cookies - reimplementing misses edge cases |
| JSON parsing | String splitting with regex | QJsonDocument | Handles escape sequences, unicode, nested objects, validation - string parsing breaks on edge cases |
| List virtualization | Manual scroll + visible item calculation | QAbstractListModel + QListView | Qt handles viewport calculation, item reuse, keyboard navigation, selection - manual approach is bug-prone |
| Configuration storage | Manual INI parsing or JSON config file | QSettings | Handles platform-specific paths, atomic writes, type conversion, defaults - custom parsing misses atomic write protection |
| Checkbox rendering | Manual QPainter rect + line drawing | QStyle::drawPrimitive with PE_IndicatorCheckBox | Platform-consistent styling, automatic hover states, accessibility support |

**Key insight:** Qt provides production-ready solutions for all common UI and networking tasks. Custom implementations introduce bugs (especially around error handling, edge cases, and platform differences) without performance benefits. The only place custom code is required is domain-specific logic (Task model, Todoist API wrapper).

## Common Pitfalls

### Pitfall 1: Touch Targets Too Small for Stylus
**What goes wrong:** Checkboxes or buttons smaller than 48x48dp are difficult to tap with reMarkable's stylus, causing missed taps and user frustration.

**Why it happens:** Copying desktop UI sizes (24x24px checkboxes) to tablet without considering stylus tip diameter (2mm) and tap accuracy. Not testing on actual hardware.

**How to avoid:**
- Minimum 48x48dp for all interactive elements (checkboxes, buttons)
- Make entire task row tappable, not just checkbox
- Add visual padding around checkbox (8dp minimum spacing from text)
- Test with actual reMarkable stylus, not mouse/trackpad

**Warning signs:**
- User taps multiple times without effect
- Accidental taps on wrong task
- User feedback: "hard to click" or "unresponsive"

**Phase impact:** Must be designed correctly in Phase 1 UI layout. Retrofitting larger targets requires full UI redesign.

### Pitfall 2: E-ink Ghosting from Only Partial Refreshes
**What goes wrong:** Using only partial/fast refreshes causes ghosting - faint residue of previous content remains visible, making task list increasingly illegible.

**Why it happens:** Partial updates don't fully realign e-ink microcapsules. Without periodic full refreshes (which flash black-white-target), residue accumulates.

**How to avoid:**
- Full refresh every 5 consecutive partial updates
- Full refresh on screen transitions (task list → add task)
- Full refresh when user manually triggers sync
- Provide manual refresh option in settings
- Test on actual e-ink display (emulator doesn't show ghosting)

**Warning signs:**
- Faint previous text visible behind current content
- UI looks "dirty" or "blurry"
- User reports text is hard to read

**Phase impact:** Refresh strategy must be built into display layer from start. Adding later requires refactoring all update code paths.

### Pitfall 3: Not Handling Todoist API Errors Gracefully
**What goes wrong:** Network failures, 401 authentication errors, or rate limits (429) cause app to crash or freeze without user feedback.

**Why it happens:** Only implementing happy path (200 OK). Not testing with network disconnected, invalid tokens, or rate limit conditions.

**How to avoid:**
- Handle all HTTP error codes: 401 (bad token), 429 (rate limit), 500+ (server error)
- Display user-friendly error messages ("Check your API token" not "401 Unauthorized")
- Implement retry with exponential backoff for 429 and 5xx
- Show offline indicator when network unavailable
- Provide way to refresh/retry after error
- Test with airplane mode, invalid token, rate limit simulation

**Warning signs:**
- App crashes on network disconnect
- Silent failures (no error message)
- Logs show unhandled exceptions from QNetworkReply
- User reports "app just stopped working"

**Phase impact:** Error handling must be designed with networking code in Phase 1. Adding comprehensive error handling later requires touching every API call site.

### Pitfall 4: Memory Leaks from QNetworkReply
**What goes wrong:** Forgetting `reply->deleteLater()` after handling QNetworkReply causes memory leaks that accumulate with each API request.

**Why it happens:** QNetworkAccessManager creates QNetworkReply objects that aren't automatically deleted. Documentation requires manual deletion but it's easy to forget.

**How to avoid:**
- Always call `reply->deleteLater()` in finished() slot
- Use RAII wrapper or smart pointer (QPointer<QNetworkReply>)
- Create helper method that guarantees cleanup
- Enable Qt warnings: `QLoggingCategory::setFilterRules("qt.network.reply=true")`
- Run with valgrind or Qt's leak checker during development

**Warning signs:**
- Memory usage grows with each sync operation
- Valgrind reports "definitely lost" blocks from QNetworkReply
- App slows down after many syncs
- `top` shows increasing RSS after repeated API calls

**Phase impact:** Establish correct pattern in Phase 1 networking code. Hunting leaks later requires auditing all network code.

### Pitfall 5: Hardcoding API Token or Committing to Git
**What goes wrong:** Developer embeds API token in source code or commits config file with token to version control, exposing user credentials.

**Why it happens:** Convenience during development. Not realizing git history is permanent. Testing with personal token and forgetting to remove.

**How to avoid:**
- Never hardcode tokens - always use QSettings or environment variables
- Add config files to .gitignore before first commit
- Use placeholder in example config: `api_token=YOUR_TOKEN_HERE`
- Document in README how to obtain and configure token
- Run `git log --all --full-history -- *config*` to check for leaked tokens
- Use pre-commit hooks to prevent config file commits

**Warning signs:**
- `grep -r "Bearer" src/` finds hardcoded tokens
- Config files with real tokens in git history
- Token visible in binary with `strings` command

**Phase impact:** Security pattern must be established in Phase 1 setup. Leaked tokens in git history are permanent (even after deletion).

## Code Examples

Verified patterns from official sources:

### Minimal QAbstractListModel Implementation
```cpp
// Source: https://doc.qt.io/qt-6/qabstractlistmodel.html
// Adapted for task list use case

class TaskModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum TaskRoles {
        TitleRole = Qt::UserRole + 1,
        DueDateRole,
        ProjectRole,
        PriorityRole,
        CompletedRole
    };

    explicit TaskModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {}

    // Required: Return number of tasks
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_tasks.count();
    }

    // Required: Return data for specific role
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_tasks.count())
            return QVariant();

        const Task &task = m_tasks.at(index.row());

        switch (role) {
            case TitleRole:
                return task.title;
            case DueDateRole:
                return task.dueDate;
            case ProjectRole:
                return task.projectName;
            case PriorityRole:
                return task.priority;
            case CompletedRole:
                return task.completed;
            default:
                return QVariant();
        }
    }

    // Optional but recommended: Role names for debugging/QML
    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[TitleRole] = "title";
        roles[DueDateRole] = "dueDate";
        roles[ProjectRole] = "project";
        roles[PriorityRole] = "priority";
        roles[CompletedRole] = "completed";
        return roles;
    }

    // Custom method to update data
    void setTasks(const QVector<Task> &tasks) {
        beginResetModel(); // Notify views that data is changing
        m_tasks = tasks;
        endResetModel();   // Notify views to refresh
    }

private:
    QVector<Task> m_tasks;
};
```

### QJsonDocument Parsing for Todoist API
```cpp
// Source: https://doc.qt.io/qt-5/qjsondocument.html
// Example of parsing Todoist REST API v2 response

void TodoistClient::onTasksReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("JSON parse error: " + parseError.errorString());
        reply->deleteLater();
        return;
    }

    QVector<Task> tasks;
    QJsonArray tasksArray = doc.array();

    for (const QJsonValue &value : tasksArray) {
        QJsonObject taskObj = value.toObject();

        Task task;
        task.id = taskObj["id"].toString();
        task.title = taskObj["content"].toString();
        task.projectId = taskObj["project_id"].toString();
        task.priority = taskObj["priority"].toInt();
        task.completed = taskObj["is_completed"].toBool();

        // Parse due date (may be null)
        if (taskObj.contains("due") && !taskObj["due"].isNull()) {
            QJsonObject dueObj = taskObj["due"].toObject();
            task.dueDate = QDate::fromString(dueObj["date"].toString(), Qt::ISODate);
        }

        tasks.append(task);
    }

    emit tasksFetched(tasks);
    reply->deleteLater(); // Critical: prevent memory leak
}
```

### QSettings for Token Storage
```cpp
// Source: https://doc.qt.io/archives/qt-5.15/qsettings.html
// Example of secure token storage with permissions

class AppSettings {
public:
    static QString getApiToken() {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "remarkable-todoist", "config");
        return settings.value("auth/api_token").toString();
    }

    static void setApiToken(const QString& token) {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "remarkable-todoist", "config");
        settings.setValue("auth/api_token", token);
        settings.sync(); // Force immediate write

        // Set restrictive file permissions (owner read/write only)
        QFile file(settings.fileName());
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }

    static bool hasApiToken() {
        return !getApiToken().isEmpty();
    }

    static QString configFilePath() {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "remarkable-todoist", "config");
        return settings.fileName();
    }
};

// Usage in main.cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    if (!AppSettings::hasApiToken()) {
        // Show first-run dialog to get token
        QString token = QInputDialog::getText(
            nullptr,
            "API Token Required",
            "Enter your Todoist API token:\n"
            "(Get it from https://todoist.com/app/settings/integrations/developer)",
            QLineEdit::Normal
        );

        if (!token.isEmpty()) {
            AppSettings::setApiToken(token);
        } else {
            return 1; // Exit if no token provided
        }
    }

    // ... continue with app initialization
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Qt Widgets manual layout | Qt Model-View architecture | Qt 4.0 (2005) | Enables virtual scrolling, memory efficient for large lists |
| Custom JSON parsing | QJsonDocument built-in | Qt 5.0 (2012) | Eliminates third-party dependency, better Qt type integration |
| QHttp (deprecated) | QNetworkAccessManager | Qt 4.4 (2008) | Async by default, connection reuse, better SSL support |
| Todoist Sync API v9 | REST API v2 (now v1) | 2023 | Simpler for read-heavy apps; Sync API deprecated 2026 |
| Manual framebuffer access | rm2fb shim | reMarkable 2 release (2020) | Required for rM2 compatibility; abstracts SWTCON driver |

**Deprecated/outdated:**
- **Todoist REST API v2:** Deprecated as of 2026, replaced by unified API v1. Migration required before production deployment but v2 adequate for Phase 1 prototyping.
- **QHttp:** Removed in Qt 6. Use QNetworkAccessManager instead.
- **Direct `/dev/fb0` access on rM2:** Doesn't work on reMarkable 2. Must use rm2fb shim.

**Important note on Todoist API:** The search results indicated REST API v2 is deprecated with shutdown planned for February 2026. However, the project STACK.md specifies REST API v2. For Phase 1 (foundation), v2 is acceptable for initial development and testing. Migration to REST API v1 must be planned for a later phase before production release. The endpoints are similar (`/rest/v1/tasks` vs `/rest/v2/tasks`) so migration should be straightforward.

## Open Questions

Things that couldn't be fully resolved:

1. **Exact rm2fb integration method for refresh control**
   - What we know: rm2fb provides framebuffer shim via `/dev/shm/swtfb*` and message queue IPC. Qt EPaper platform plugin provides some automatic optimization.
   - What's unclear: Whether manual waveform control (DU vs GC16) is accessible from Qt Widgets or handled automatically by platform plugin. May require ioctl calls or rm2fb client library.
   - Recommendation: Start with Qt's built-in update()/repaint() methods. Test on device to determine if manual waveform control is needed. Community reports suggest Qt handles this reasonably well by default.

2. **Qt Widgets support status on reMarkable 2**
   - What we know: Official docs say "Qt Quick only" but community successfully uses Qt Widgets with rm2fb. Project STACK.md specifies Qt Widgets based on community validation.
   - What's unclear: Whether there are edge cases or performance issues with Widgets vs Quick on Qt 5.15.
   - Recommendation: Proceed with Qt Widgets as planned. Early prototype (hello world + simple list) will validate this choice. Fallback to Qt Quick is possible but would require architecture changes.

3. **Optimal partial:full refresh ratio**
   - What we know: E-ink requires periodic full refreshes to prevent ghosting. Common ratios are 5:1 to 10:1 partial:full.
   - What's unclear: Optimal ratio for reMarkable 2's specific e-ink panel and task list use case.
   - Recommendation: Start with 5:1 (5 partials, then 1 full). Make configurable in settings. User testing will determine best default.

4. **QSettings file permissions on reMarkable device**
   - What we know: QSettings creates files with default system permissions. setPermissions() method available to restrict access.
   - What's unclear: Whether reMarkable's default umask already restricts config files, or if explicit permission setting is necessary.
   - Recommendation: Explicitly call setPermissions(0600) to ensure security regardless of system defaults. Validate with `ls -la` on device.

## Sources

### Primary (HIGH confidence)
- [Qt 5.15 QAbstractListModel Documentation](https://doc.qt.io/qt-5/qabstractlistmodel.html) - Model implementation patterns
- [Qt 5.15 Model/View Programming](https://doc.qt.io/qt-5/model-view-programming.html) - Architecture patterns
- [Qt 5.15 QListView Documentation](https://doc.qt.io/qt-5/qlistview.html) - View configuration
- [Qt 5.15 QStyledItemDelegate Documentation](https://doc.qt.io/qt-5/qstyleditemdelegate.html) - Custom rendering
- [Qt 5.15 QNetworkAccessManager](https://doc.qt.io/qt-5/qnetworkaccessmanager.html) - HTTP client usage
- [Qt 5.15 QJsonDocument](https://doc.qt.io/qt-5/qjsondocument.html) - JSON parsing API
- [Qt 5.15 QSettings Documentation](https://doc.qt.io/archives/qt-5.15/qsettings.html) - Configuration storage
- [Todoist REST API v2 Documentation](https://developer.todoist.com/rest/v2/) - API endpoints and authentication
- [reMarkable Developer - Qt EPaper](https://developer.remarkable.com/documentation/qt_epaper) - Official platform guidance
- [rm2fb GitHub Repository](https://github.com/ddvk/remarkable2-framebuffer) - Framebuffer compatibility layer

### Secondary (MEDIUM confidence)
- [WCAG 2.1 Success Criterion 2.5.5: Target Size](https://www.w3.org/WAI/WCAG21/Understanding/target-size.html) - 44x44px minimum touch targets
- [Accessible Target Sizes Cheatsheet - Smashing Magazine](https://www.smashingmagazine.com/2023/04/accessible-tap-target-sizes-rage-taps-clicks/) - 48dp recommendation for touchscreens
- [Touch Targets on Touchscreens - Nielsen Norman Group](https://www.nngroup.com/articles/touch-target-size/) - Usability research on touch target sizing
- [E-paper basics 1: Update modes - Hackaday](https://hackaday.io/project/21551-paperino/log/59392-e-paper-basics-1-update-modes) - E-ink refresh strategies
- [E Ink Ghosting Decoded - Viwoods](https://viwoods.com/blogs/paper-tablet/e-ink-ghosting-explained) - Ghosting causes and prevention
- [How to Use QAbstractListModel - Qt Wiki](https://wiki.qt.io/How_to_use_QAbstractListModel) - Community implementation guide
- [How to Use QSettings - Qt Wiki](https://wiki.qt.io/How_to_Use_QSettings) - Configuration best practices

### Tertiary (LOW confidence - needs validation)
- WebSearch results on Qt Widgets vs Qt Quick on reMarkable 2 - community reports suggest Widgets work but official docs recommend Quick
- WebSearch results on rm2fb usage patterns - limited recent documentation, mostly from 2020-2022
- Project STACK.md assertions about Qt Widgets compatibility - based on community validation, not official support

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Qt 5.15 Widgets, QNetworkAccessManager, QJsonDocument, QSettings are all officially documented and verified
- Architecture patterns: HIGH - Qt Model-View is established pattern with extensive official documentation and examples
- E-ink refresh strategy: MEDIUM - General e-ink principles are well-documented, but reMarkable 2 specific optimization requires on-device testing
- rm2fb integration: MEDIUM - Repository documentation available but specific Qt Widgets integration patterns less documented
- Todoist API: HIGH - Official REST API v2 documentation is comprehensive, but note API v2 is deprecated
- Touch targets: HIGH - WCAG and usability research provides clear 44-48dp minimum guideline
- Pitfalls: MEDIUM-HIGH - Based on verified sources for Qt/HTTP/e-ink, community reports for reMarkable specific issues

**Research date:** 2026-01-30
**Valid until:** 30 days (Qt/Todoist stable, but rm2fb may update with OS versions)

**Critical uncertainties requiring early validation:**
1. Qt Widgets rendering performance on reMarkable 2 with rm2fb
2. Exact method for controlling e-ink refresh modes from Qt Widgets
3. Todoist REST API v2 deprecation timeline (migrate to v1 before production)
