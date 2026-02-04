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

void TodoistClient::closeTask(const QString& taskId)
{
    qDebug() << "Closing task" << taskId << "via Todoist API...";

    QString urlString = QString("https://api.todoist.com/rest/v2/tasks/%1/close").arg(taskId);
    QUrl url(urlString);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiToken).toUtf8());
    request.setTransferTimeout(REQUEST_TIMEOUT_MS);

    // POST with empty body (required by Qt for bodyless POST)
    QNetworkReply* reply = m_networkManager->post(request, QByteArray());
    connect(reply, &QNetworkReply::finished, this, &TodoistClient::onCloseTaskReplyFinished);
}

void TodoistClient::createTask(const QString& content)
{
    qDebug() << "Creating task via Todoist API:" << content;

    QUrl url(TASKS_ENDPOINT);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiToken).toUtf8());
    request.setRawHeader("Content-Type", "application/json");
    request.setTransferTimeout(REQUEST_TIMEOUT_MS);

    // Build JSON body
    QJsonObject jsonObj;
    jsonObj["content"] = content;
    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, &TodoistClient::onCreateTaskReplyFinished);
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

void TodoistClient::onCloseTaskReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qWarning() << "onCloseTaskReplyFinished called with invalid sender";
        return;
    }

    // Extract task ID from URL (e.g., https://api.todoist.com/rest/v2/tasks/12345/close)
    QString url = reply->url().toString();
    QString taskId = url.section('/', -2, -2);  // Extract ID from path

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = handleNetworkError(reply);
        qWarning() << "Close task" << taskId << "failed:" << errorMsg;
        emit closeTaskFailed(taskId, errorMsg);
        reply->deleteLater();
        return;
    }

    // Check for 204 No Content status code (success)
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 204) {
        qDebug() << "Successfully closed task" << taskId;
        emit taskClosed(taskId);
    } else {
        QString errorMsg = QString("Unexpected status code: %1").arg(statusCode);
        qWarning() << "Close task" << taskId << "failed:" << errorMsg;
        emit closeTaskFailed(taskId, errorMsg);
    }

    // Critical: prevent memory leak
    reply->deleteLater();
}

void TodoistClient::onCreateTaskReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qWarning() << "onCreateTaskReplyFinished called with invalid sender";
        return;
    }

    // Extract content from request body for signal parameter
    QByteArray requestData = reply->request().attribute(QNetworkRequest::CustomVerbAttribute).toByteArray();
    QJsonDocument requestDoc = QJsonDocument::fromJson(reply->request().attribute(QNetworkRequest::User).toByteArray());
    QString content;

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = handleNetworkError(reply);
        qWarning() << "Create task failed:" << errorMsg;

        // Try to extract content from reply readAll (in case of server response)
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (doc.isObject() && doc.object().contains("content")) {
            content = doc.object()["content"].toString();
        }

        emit createTaskFailed(content, errorMsg);
        reply->deleteLater();
        return;
    }

    // Parse JSON response
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = QString("Failed to parse create task response: %1").arg(parseError.errorString());
        qWarning() << errorMsg;
        emit createTaskFailed(content, errorMsg);
        reply->deleteLater();
        return;
    }

    // Check for HTTP 200 status code (Todoist API returns 200, not 201)
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 200 && doc.isObject()) {
        QJsonObject taskObj = doc.object();
        QString newTaskId = taskObj["id"].toString();
        content = taskObj["content"].toString();

        qDebug() << "Successfully created task:" << content << "id:" << newTaskId;
        emit taskCreated(content, newTaskId);
    } else {
        QString errorMsg = QString("Unexpected status code: %1").arg(statusCode);
        qWarning() << "Create task failed:" << errorMsg;
        if (doc.isObject() && doc.object().contains("content")) {
            content = doc.object()["content"].toString();
        }
        emit createTaskFailed(content, errorMsg);
    }

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
