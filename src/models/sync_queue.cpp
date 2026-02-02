#include "sync_queue.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QUuid>

SyncQueue::SyncQueue(QObject* parent)
    : QObject(parent)
{
    loadFromFile();
}

void SyncQueue::enqueue(const SyncOperation& op)
{
    m_operations.append(op);
    saveToFile();
    emit countChanged();
}

SyncOperation SyncQueue::dequeue()
{
    if (m_operations.isEmpty()) {
        qWarning() << "SyncQueue::dequeue() called on empty queue";
        return SyncOperation();
    }

    SyncOperation op = m_operations.takeFirst();
    saveToFile();
    emit countChanged();
    return op;
}

SyncOperation SyncQueue::peek() const
{
    if (m_operations.isEmpty()) {
        qWarning() << "SyncQueue::peek() called on empty queue";
        return SyncOperation();
    }

    return m_operations.first();
}

bool SyncQueue::isEmpty() const
{
    return m_operations.isEmpty();
}

int SyncQueue::count() const
{
    return m_operations.size();
}

void SyncQueue::clear()
{
    m_operations.clear();
    saveToFile();
    emit countChanged();
}

bool SyncQueue::hasOperationForTask(const QString& taskId, const QString& type) const
{
    for (const SyncOperation& op : m_operations) {
        if (op.taskId == taskId && op.type == type) {
            return true;
        }
    }
    return false;
}

QString SyncQueue::queueFilePath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/sync_queue.json";
}

void SyncQueue::saveToFile()
{
    QString filePath = queueFilePath();

    // Create parent directory if needed
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create directory:" << dir.path();
            return;
        }
    }

    // Serialize operations to JSON
    QJsonArray jsonArray;
    for (const SyncOperation& op : m_operations) {
        QJsonObject jsonObj;
        jsonObj["uuid"] = op.uuid;
        jsonObj["type"] = op.type;
        jsonObj["taskId"] = op.taskId;
        jsonObj["queuedAt"] = op.queuedAt.toString(Qt::ISODate);
        jsonObj["retryCount"] = op.retryCount;
        jsonArray.append(jsonObj);
    }

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath << file.errorString();
        return;
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();

    qDebug() << "Saved" << m_operations.size() << "operations to" << filePath;
}

void SyncQueue::loadFromFile()
{
    QString filePath = queueFilePath();

    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "Sync queue file does not exist, starting with empty queue";
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath << file.errorString();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << parseError.errorString();
        return;
    }

    if (!doc.isArray()) {
        qWarning() << "Expected JSON array in sync queue file";
        return;
    }

    // Deserialize operations from JSON
    QJsonArray jsonArray = doc.array();
    m_operations.clear();

    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            qWarning() << "Skipping non-object value in sync queue";
            continue;
        }

        QJsonObject jsonObj = value.toObject();
        SyncOperation op;
        op.uuid = jsonObj["uuid"].toString();
        op.type = jsonObj["type"].toString();
        op.taskId = jsonObj["taskId"].toString();
        op.queuedAt = QDateTime::fromString(jsonObj["queuedAt"].toString(), Qt::ISODate);
        op.retryCount = jsonObj["retryCount"].toInt();

        m_operations.append(op);
    }

    qDebug() << "Loaded" << m_operations.size() << "operations from" << filePath;
}
