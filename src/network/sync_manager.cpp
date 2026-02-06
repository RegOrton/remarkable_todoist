#include "sync_manager.h"
#include "todoist_client.h"
#include <QNetworkInformation>
#include <QUuid>
#include <QDebug>

SyncManager::SyncManager(TodoistClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_isOnline(true)  // Optimistic - assume online until proven otherwise
    , m_isSyncing(false)
    , m_useNetworkInfo(false)
{
    // Connect TodoistClient signals
    connect(m_client, &TodoistClient::taskClosed, this, &SyncManager::onTaskClosed);
    connect(m_client, &TodoistClient::closeTaskFailed, this, &SyncManager::onCloseTaskFailed);
    connect(m_client, &TodoistClient::taskCreated, this, &SyncManager::onTaskCreated);
    connect(m_client, &TodoistClient::createTaskFailed, this, &SyncManager::onCreateTaskFailed);

    // Initialize network monitoring
    initializeNetworkMonitoring();

    // Set up retry timer (2 second delay for connectivity restoration)
    m_retryTimer.setSingleShot(true);
    m_retryTimer.setInterval(2000);
    connect(&m_retryTimer, &QTimer::timeout, this, &SyncManager::processNextOperation);

    // Connect queue signals
    connect(&m_queue, &SyncQueue::countChanged, this, &SyncManager::pendingCountChanged);

    // Load persisted queue
    m_queue.loadFromFile();

    qDebug() << "SyncManager initialized with" << m_queue.count() << "pending operations";

    // Try to process queue immediately if we have operations
    if (!m_queue.isEmpty()) {
        processQueue();
    }
}

void SyncManager::initializeNetworkMonitoring()
{
    // Try to load QNetworkInformation backend
    if (QNetworkInformation::loadDefaultBackend()) {
        QNetworkInformation* netInfo = QNetworkInformation::instance();
        if (netInfo) {
            connect(netInfo, &QNetworkInformation::reachabilityChanged,
                    this, &SyncManager::onReachabilityChanged);
            m_useNetworkInfo = true;
            qDebug() << "SyncManager: QNetworkInformation monitoring enabled";

            // Get initial state
            bool online = (netInfo->reachability() == QNetworkInformation::Reachability::Online);
            setOnline(online);
        } else {
            m_useNetworkInfo = false;
            qWarning() << "SyncManager: QNetworkInformation instance not available";
        }
    } else {
        m_useNetworkInfo = false;
        qWarning() << "SyncManager: QNetworkInformation not available - will detect offline via failed requests";
    }
}

void SyncManager::queueTaskCompletion(const QString& taskId)
{
    // Check for duplicate operations
    if (m_queue.hasOperationForTask(taskId, "close_task")) {
        qDebug() << "Task" << taskId << "already in queue, skipping duplicate";
        return;
    }

    // Create operation
    SyncOperation op;
    op.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    op.type = "close_task";
    op.taskId = taskId;
    op.queuedAt = QDateTime::currentDateTime();
    op.retryCount = 0;

    // Add to queue
    m_queue.enqueue(op);
    qDebug() << "Queued task completion:" << taskId << "uuid:" << op.uuid;

    // Save to disk
    m_queue.saveToFile();

    // Attempt immediate sync
    processQueue();
}

void SyncManager::queueTaskCreation(const QString& content, const QString& tempId)
{
    // Create operation
    SyncOperation op;
    op.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    op.type = "create_task";
    op.content = content;
    op.tempId = tempId;
    op.queuedAt = QDateTime::currentDateTime();
    op.retryCount = 0;

    // Add to queue
    m_queue.enqueue(op);
    qDebug() << "Queued task creation:" << content << "tempId:" << tempId << "uuid:" << op.uuid;

    // Save to disk
    m_queue.saveToFile();

    // Attempt immediate sync
    processQueue();
}

void SyncManager::processQueue()
{
    if (m_queue.isEmpty()) {
        qDebug() << "Queue is empty, nothing to process";
        return;
    }

    if (m_isSyncing) {
        qDebug() << "Already syncing, will process queue after current operation completes";
        return;
    }

    processNextOperation();
}

void SyncManager::processNextOperation()
{
    if (m_queue.isEmpty()) {
        setIsSyncing(false);
        qDebug() << "Queue empty, sync complete";
        return;
    }

    setIsSyncing(true);

    // Peek at next operation (don't remove until confirmed)
    SyncOperation op = m_queue.peek();
    m_currentSyncTaskId = op.taskId;

    qDebug() << "Processing operation:" << op.type << "taskId:" << op.taskId << "attempt:" << (op.retryCount + 1);

    // Process based on operation type
    if (op.type == "close_task") {
        m_client->closeTask(op.taskId);
    } else if (op.type == "create_task") {
        m_currentSyncContent = op.content;
        m_currentSyncTaskId = op.tempId;  // Store tempId for matching responses
        m_client->createTask(op.content);
    } else {
        qWarning() << "Unknown operation type:" << op.type;
        // Remove invalid operation
        m_queue.dequeue();
        m_queue.saveToFile();
        processNextOperation();
    }
}

void SyncManager::onTaskClosed(const QString& taskId)
{
    if (taskId != m_currentSyncTaskId) {
        qWarning() << "Received taskClosed for unexpected task:" << taskId << "expected:" << m_currentSyncTaskId;
        return;
    }

    qDebug() << "Task closed successfully:" << taskId;

    // Remove from queue (operation confirmed successful)
    m_queue.dequeue();
    m_queue.saveToFile();

    // Confirmed online
    setOnline(true);

    // Notify success
    emit syncSucceeded(taskId);

    // Process next operation
    processNextOperation();
}

void SyncManager::onCloseTaskFailed(const QString& taskId, const QString& error)
{
    if (taskId != m_currentSyncTaskId) {
        qWarning() << "Received closeTaskFailed for unexpected task:" << taskId << "expected:" << m_currentSyncTaskId;
        return;
    }

    qWarning() << "Task close failed:" << taskId << "error:" << error;

    setIsSyncing(false);

    // Emit failure signal
    emit syncFailed(taskId, error);

    // Detect offline based on error message
    QString lowerError = error.toLower();
    if (lowerError.contains("timeout") || lowerError.contains("connection") ||
        lowerError.contains("network") || lowerError.contains("unreachable")) {
        qDebug() << "Detected offline state from error";
        setOnline(false);
        // Don't retry immediately when offline - wait for connectivity change
        return;
    }

    // For other errors (e.g., server errors), increment retry count and try again later
    if (!m_queue.isEmpty()) {
        SyncOperation op = m_queue.peek();
        op.retryCount++;

        // Update the operation in the queue
        m_queue.dequeue();
        m_queue.enqueue(op);
        m_queue.saveToFile();

        if (op.retryCount < 5) {
            qDebug() << "Retrying operation after 2 seconds, attempt:" << (op.retryCount + 1);
            m_retryTimer.start();
        } else {
            qWarning() << "Operation failed after" << op.retryCount << "attempts, giving up";
            // Remove failed operation
            m_queue.dequeue();
            m_queue.saveToFile();
            // Try next operation
            processNextOperation();
        }
    }
}

void SyncManager::onTaskCreated(const QString& content, const QString& newTaskId)
{
    if (content != m_currentSyncContent) {
        qWarning() << "Received taskCreated for unexpected content:" << content << "expected:" << m_currentSyncContent;
        return;
    }

    qDebug() << "Task created successfully:" << content << "serverTaskId:" << newTaskId;

    // Get tempId before dequeue
    QString tempId = m_currentSyncTaskId;

    // Remove from queue (operation confirmed successful)
    m_queue.dequeue();
    m_queue.saveToFile();

    // Confirmed online
    setOnline(true);

    // Notify success with tempId and server-assigned ID
    emit taskCreateSynced(tempId, newTaskId);

    // Process next operation
    processNextOperation();
}

void SyncManager::onCreateTaskFailed(const QString& content, const QString& error)
{
    if (content != m_currentSyncContent) {
        qWarning() << "Received createTaskFailed for unexpected content:" << content << "expected:" << m_currentSyncContent;
        return;
    }

    qWarning() << "Task create failed:" << content << "error:" << error;

    setIsSyncing(false);

    // Get tempId for failure signal
    QString tempId = m_currentSyncTaskId;

    // Emit failure signal
    emit taskCreateSyncFailed(tempId, error);

    // Detect offline based on error message
    QString lowerError = error.toLower();
    if (lowerError.contains("timeout") || lowerError.contains("connection") ||
        lowerError.contains("network") || lowerError.contains("unreachable")) {
        qDebug() << "Detected offline state from error";
        setOnline(false);
        // Don't retry immediately when offline - wait for connectivity change
        return;
    }

    // For other errors (e.g., server errors), increment retry count and try again later
    if (!m_queue.isEmpty()) {
        SyncOperation op = m_queue.peek();
        op.retryCount++;

        // Update the operation in the queue
        m_queue.dequeue();
        m_queue.enqueue(op);
        m_queue.saveToFile();

        if (op.retryCount < 5) {
            qDebug() << "Retrying operation after 2 seconds, attempt:" << (op.retryCount + 1);
            m_retryTimer.start();
        } else {
            qWarning() << "Operation failed after" << op.retryCount << "attempts, giving up";
            // Remove failed operation
            m_queue.dequeue();
            m_queue.saveToFile();
            // Try next operation
            processNextOperation();
        }
    }
}

void SyncManager::onReachabilityChanged()
{
    if (!m_useNetworkInfo) {
        return;
    }

    QNetworkInformation* netInfo = QNetworkInformation::instance();
    if (!netInfo) {
        return;
    }

    bool online = (netInfo->reachability() == QNetworkInformation::Reachability::Online);
    qDebug() << "Network reachability changed, online:" << online;

    bool wasOnline = m_isOnline;
    setOnline(online);

    // If we just came online and have pending operations, start retry timer
    if (online && !wasOnline && !m_queue.isEmpty()) {
        qDebug() << "Connectivity restored, scheduling sync in 2 seconds";
        m_retryTimer.start();
    }
}

void SyncManager::setOnline(bool online)
{
    if (m_isOnline != online) {
        m_isOnline = online;
        emit isOnlineChanged();
        qDebug() << "Online state changed:" << online;
    }
}

void SyncManager::setIsSyncing(bool syncing)
{
    if (m_isSyncing != syncing) {
        m_isSyncing = syncing;
        emit isSyncingChanged();
    }
}
