#include "appcontroller.h"

#include <QDebug>

#include "../models/task.h"
#include "../models/taskmodel.h"
#include "../network/todoist_client.h"
#include "../config/settings.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_loading(false)
    , m_errorMessage("")
    , m_taskModel(nullptr)
    , m_todoistClient(nullptr)
{
    // Create task model
    m_taskModel = new TaskModel(this);
}

AppController::~AppController()
{
}

void AppController::initialize()
{
    // Check for API token
    if (!AppSettings::hasApiToken()) {
        qWarning() << "No API token configured";
        setErrorMessage(
            "API Token Not Configured\n\n"
            "To use Remarkable Todoist, configure your API token.\n\n"
            "1. Get token from:\n"
            "   todoist.com/app/settings/integrations/developer\n\n"
            "2. Create config file:\n"
            "   mkdir -p ~/.config/remarkable-todoist\n\n"
            "3. Add your token:\n"
            "   echo \"[auth]\" > ~/.config/remarkable-todoist/config.ini\n"
            "   echo \"api_token=YOUR_TOKEN\" >> ~/.config/remarkable-todoist/config.ini\n\n"
            "4. Restart the application"
        );
        return;
    }

    // Create Todoist client with the token
    QString token = AppSettings::getApiToken();
    m_todoistClient = new TodoistClient(token, this);

    // Connect signals
    connect(m_todoistClient, &TodoistClient::projectsFetched,
            this, &AppController::onProjectsFetched);
    connect(m_todoistClient, &TodoistClient::tasksFetched,
            this, &AppController::onTasksFetched);
    connect(m_todoistClient, &TodoistClient::errorOccurred,
            this, &AppController::onError);

    // Start fetch flow: projects first, then tasks
    refresh();
}

void AppController::refresh()
{
    if (!m_todoistClient) {
        qWarning() << "Cannot refresh: No API client (token not configured?)";
        return;
    }

    setLoading(true);
    setErrorMessage("");
    qDebug() << "Fetching projects from Todoist API...";
    m_todoistClient->fetchProjects();
}

void AppController::onProjectsFetched(const QMap<QString, QString>& projects)
{
    Q_UNUSED(projects);
    qDebug() << "Projects fetched:" << projects.count() << "projects";

    // Now fetch tasks (TodoistClient will use cached project names)
    m_todoistClient->fetchAllTasks();
}

void AppController::onTasksFetched(const QVector<Task>& tasks)
{
    qDebug() << "Tasks fetched:" << tasks.count() << "tasks";

    // Populate the model
    m_taskModel->setTasks(tasks);

    // Done loading
    setLoading(false);
}

void AppController::onError(const QString& error)
{
    qWarning() << "Todoist API error:" << error;
    setLoading(false);
    setErrorMessage(error);
}

void AppController::setLoading(bool loading)
{
    if (m_loading != loading) {
        m_loading = loading;
        emit loadingChanged();
    }
}

void AppController::setErrorMessage(const QString& message)
{
    if (m_errorMessage != message) {
        m_errorMessage = message;
        emit errorMessageChanged();
    }
}
