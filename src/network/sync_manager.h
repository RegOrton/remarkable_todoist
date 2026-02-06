#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <QObject>
#include <QTimer>
#include "../models/sync_queue.h"

class TodoistClient;
class QNetworkInformation;

class SyncManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingCountChanged)
    Q_PROPERTY(bool isSyncing READ isSyncing NOTIFY isSyncingChanged)

public:
    explicit SyncManager(TodoistClient* client, QObject* parent = nullptr);

    // Properties
    bool isOnline() const { return m_isOnline; }
    int pendingCount() const { return m_queue.count(); }
    bool isSyncing() const { return m_isSyncing; }

    // Queue a task completion operation
    void queueTaskCompletion(const QString& taskId);

    // Queue a task creation operation
    void queueTaskCreation(const QString& content, const QString& tempId);

    // Process pending operations (call when you think we're online)
    void processQueue();

signals:
    void isOnlineChanged();
    void pendingCountChanged();
    void isSyncingChanged();
    void syncSucceeded(const QString& taskId);
    void syncFailed(const QString& taskId, const QString& error);
    void taskCreateSynced(const QString& tempId, const QString& serverTaskId);
    void taskCreateSyncFailed(const QString& tempId, const QString& error);

private slots:
    void onTaskClosed(const QString& taskId);
    void onCloseTaskFailed(const QString& taskId, const QString& error);
    void onTaskCreated(const QString& content, const QString& newTaskId);
    void onCreateTaskFailed(const QString& content, const QString& error);
    void onReachabilityChanged();
    void processNextOperation();

private:
    void initializeNetworkMonitoring();
    void setOnline(bool online);
    void setIsSyncing(bool syncing);

    TodoistClient* m_client;
    SyncQueue m_queue;
    QTimer m_retryTimer;

    bool m_isOnline;
    bool m_isSyncing;
    bool m_useNetworkInfo;        // Whether QNetworkInformation is available
    QString m_currentSyncTaskId;  // Task currently being synced
    QString m_currentSyncContent; // Content of current sync operation (for create_task)
};

#endif // SYNC_MANAGER_H
