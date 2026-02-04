#ifndef SYNC_QUEUE_H
#define SYNC_QUEUE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QDateTime>

struct SyncOperation {
    QString uuid;           // Unique ID for idempotency (QUuid::createUuid())
    QString type;           // "close_task" or "create_task"
    QString taskId;         // Target task ID (empty for create_task until server responds)
    QString content;        // Task name for create_task operations
    QString tempId;         // Temporary ID for tracking optimistic UI entries
    QDateTime queuedAt;     // When queued
    int retryCount;         // Number of sync attempts

    SyncOperation() : retryCount(0) {}
};

class SyncQueue : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit SyncQueue(QObject* parent = nullptr);

    // Queue operations
    void enqueue(const SyncOperation& op);
    SyncOperation dequeue();           // Remove and return front
    SyncOperation peek() const;        // View front without removing
    bool isEmpty() const;
    int count() const;
    void clear();

    // Find operation by task ID (for deduplication)
    bool hasOperationForTask(const QString& taskId, const QString& type) const;

    // Persistence
    void saveToFile();
    void loadFromFile();

signals:
    void countChanged();

private:
    QString queueFilePath() const;

    QVector<SyncOperation> m_operations;
};

#endif // SYNC_QUEUE_H
