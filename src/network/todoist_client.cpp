#include "todoist_client.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

// API endpoints
static const QString TASKS_ENDPOINT = "https://api.todoist.com/rest/v2/tasks";
static const QString PROJECTS_ENDPOINT = "https://api.todoist.com/rest/v2/projects";

// Request timeout in milliseconds (30 seconds)
static const int REQUEST_TIMEOUT_MS = 30000;

TodoistClient::TodoistClient(const QString& apiToken, QObject* parent)
    : QObject(parent)
    , m_apiToken(apiToken)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void TodoistClient::fetchAllTasks()
{
    qDebug() << "Fetching tasks from Todoist API...";

    QUrl url(TASKS_ENDPOINT);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiToken).toUtf8());
    request.setTransferTimeout(REQUEST_TIMEOUT_MS);

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &TodoistClient::onTasksReplyFinished);
}

void TodoistClient::fetchProjects()
{
    qDebug() << "Fetching projects from Todoist API...";

    QUrl url(PROJECTS_ENDPOINT);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiToken).toUtf8());
    request.setTransferTimeout(REQUEST_TIMEOUT_MS);

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &TodoistClient::onProjectsReplyFinished);
}

void TodoistClient::onTasksReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qWarning() << "onTasksReplyFinished called with invalid sender";
        return;
    }

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = handleNetworkError(reply);
        qWarning() << "Tasks fetch failed:" << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }

    // Parse JSON response
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = QString("Failed to parse tasks response: %1").arg(parseError.errorString());
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }

    // Parse tasks array
    QVector<Task> tasks;
    QJsonArray tasksArray = doc.array();

    for (const QJsonValue& value : tasksArray) {
        if (value.isObject()) {
            Task task = Task::fromJson(value.toObject());

            // Look up project name from cache
            if (!task.projectId.isEmpty() && m_projectNames.contains(task.projectId)) {
                task.projectName = m_projectNames.value(task.projectId);
            }

            tasks.append(task);
        }
    }

    qDebug() << "Successfully fetched" << tasks.size() << "tasks";
    emit tasksFetched(tasks);

    // Critical: prevent memory leak
    reply->deleteLater();
}

void TodoistClient::onProjectsReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qWarning() << "onProjectsReplyFinished called with invalid sender";
        return;
    }

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = handleNetworkError(reply);
        qWarning() << "Projects fetch failed:" << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }

    // Parse JSON response
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = QString("Failed to parse projects response: %1").arg(parseError.errorString());
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }

    // Parse projects array and update cache
    m_projectNames.clear();
    QJsonArray projectsArray = doc.array();

    for (const QJsonValue& value : projectsArray) {
        if (value.isObject()) {
            QJsonObject projectObj = value.toObject();
            QString id = projectObj["id"].toString();
            QString name = projectObj["name"].toString();

            if (!id.isEmpty()) {
                m_projectNames.insert(id, name);
            }
        }
    }

    qDebug() << "Successfully fetched" << m_projectNames.size() << "projects";
    emit projectsFetched(m_projectNames);

    // Critical: prevent memory leak
    reply->deleteLater();
}

QString TodoistClient::handleNetworkError(QNetworkReply* reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
        case 401:
            return "Invalid API token. Please check your token in settings.";
        case 403:
            return "Access forbidden. Your token may not have the required permissions.";
        case 404:
            return "API endpoint not found. The Todoist API may have changed.";
        case 429:
            return "Too many requests. Please wait a moment and try again.";
        case 500:
        case 502:
        case 503:
        case 504:
            return "Todoist server error. Please try again later.";
        default:
            if (reply->error() == QNetworkReply::NoError) {
                return QString();  // No error
            }
            // Network-level errors (no connection, timeout, etc.)
            return QString("Network error: %1").arg(reply->errorString());
    }
}
