#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include "../models/task.h"
#include "../network/sync_manager.h"

class TodoistClient;
class TaskModel;

/**
 * AppController - Orchestrates the application components for QML
 *
 * Exposes properties and methods for QML to bind to:
 * - loading: whether tasks are being fetched
 * - errorMessage: error to display (empty if no error)
 * - refresh(): trigger a task refresh
 *
 * Also provides access to TaskModel for the QML ListView.
 */
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(SyncManager* syncManager READ syncManager CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    /**
     * Initialize the application
     * Call after construction to start the fetch flow
     */
    void initialize();

    /**
     * Get the TaskModel for QML binding
     */
    TaskModel* taskModel() const { return m_taskModel; }

    // Property accessors
    bool loading() const { return m_loading; }
    QString errorMessage() const { return m_errorMessage; }
    SyncManager* syncManager() const { return m_syncManager; }

public slots:
    /**
     * Refresh the task list
     * Fetches projects then tasks from Todoist API
     */
    void refresh();

    /**
     * Complete a task (optimistic update + queue for sync)
     */
    Q_INVOKABLE void completeTask(const QString& taskId);

signals:
    void loadingChanged();
    void errorMessageChanged();

private slots:
    void onProjectsFetched(const QMap<QString, QString>& projects);
    void onTasksFetched(const QVector<Task>& tasks);
    void onError(const QString& error);

private:
    void setLoading(bool loading);
    void setErrorMessage(const QString& message);

    // State
    bool m_loading;
    QString m_errorMessage;

    // Data layer
    TaskModel* m_taskModel;
    TodoistClient* m_todoistClient;
    SyncManager* m_syncManager;
};

#endif // APPCONTROLLER_H
