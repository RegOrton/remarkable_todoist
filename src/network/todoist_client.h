#ifndef TODOIST_CLIENT_H
#define TODOIST_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVector>
#include <QMap>
#include "../models/task.h"

/**
 * @brief TodoistClient provides async access to the Todoist REST API v2.
 *
 * This client handles fetching tasks and projects from the Todoist API,
 * with proper error handling and memory management.
 *
 * Usage:
 *   TodoistClient* client = new TodoistClient(apiToken, this);
 *   connect(client, &TodoistClient::tasksFetched, this, &MyClass::onTasks);
 *   connect(client, &TodoistClient::errorOccurred, this, &MyClass::onError);
 *   client->fetchProjects();  // Fetch projects first to get names
 *   client->fetchAllTasks();  // Then fetch tasks
 */
class TodoistClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a TodoistClient
     * @param apiToken The Todoist API token for authentication
     * @param parent Parent QObject for memory management
     */
    explicit TodoistClient(const QString& apiToken, QObject* parent = nullptr);

    /**
     * @brief Fetch all tasks from Todoist
     *
     * Emits tasksFetched() on success or errorOccurred() on failure.
     * Tasks will have projectName populated from cached project data.
     */
    void fetchAllTasks();

    /**
     * @brief Fetch all projects from Todoist
     *
     * Emits projectsFetched() on success or errorOccurred() on failure.
     * Project names are cached for use when fetching tasks.
     */
    void fetchProjects();

signals:
    /**
     * @brief Emitted when tasks have been successfully fetched
     * @param tasks Vector of Task objects parsed from API response
     */
    void tasksFetched(const QVector<Task>& tasks);

    /**
     * @brief Emitted when projects have been successfully fetched
     * @param projects Map of project ID to project name
     */
    void projectsFetched(const QMap<QString, QString>& projects);

    /**
     * @brief Emitted when an error occurs during API request
     * @param error User-friendly error message
     */
    void errorOccurred(const QString& error);

private slots:
    void onTasksReplyFinished();
    void onProjectsReplyFinished();

private:
    QString handleNetworkError(QNetworkReply* reply);

    QNetworkAccessManager* m_networkManager;
    QString m_apiToken;
    QMap<QString, QString> m_projectNames;  // projectId -> projectName cache
};

#endif // TODOIST_CLIENT_H
