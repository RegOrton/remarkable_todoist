#include "appcontroller.h"

#include <QDebug>

#include "../models/task.h"
#include "../models/taskmodel.h"
#include "../network/todoist_client.h"
#include "../views/tasklistview.h"
#include "../config/settings.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_mainWidget(nullptr)
    , m_layout(nullptr)
    , m_taskListView(nullptr)
    , m_loadingLabel(nullptr)
    , m_errorLabel(nullptr)
    , m_taskModel(nullptr)
    , m_todoistClient(nullptr)
{
    // Create main window widget
    m_mainWidget = new QWidget();
    m_mainWidget->setStyleSheet("background-color: white;");

    // Create layout
    m_layout = new QVBoxLayout(m_mainWidget);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // Create task model
    m_taskModel = new TaskModel(this);

    // Create task list view
    m_taskListView = new TaskListView(m_mainWidget);
    m_taskListView->setModel(m_taskModel);
    m_taskListView->hide(); // Hidden until tasks loaded

    // Create loading label
    m_loadingLabel = new QLabel("Loading tasks...", m_mainWidget);
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 18pt;"
        "  color: #333333;"
        "  background-color: white;"
        "}"
    );
    m_loadingLabel->hide();

    // Create error label
    m_errorLabel = new QLabel(m_mainWidget);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 14pt;"
        "  color: #333333;"
        "  background-color: white;"
        "  padding: 20px;"
        "}"
    );
    m_errorLabel->hide();

    // Add widgets to layout
    m_layout->addWidget(m_taskListView);
    m_layout->addWidget(m_loadingLabel);
    m_layout->addWidget(m_errorLabel);
}

AppController::~AppController()
{
    // m_mainWidget and its children are not parented to this QObject,
    // so they need explicit deletion if not shown in a window
    // However, if the widget is shown, Qt will handle deletion
    // We don't delete here because mainWidget() ownership is transferred to caller
}

void AppController::initialize()
{
    // Check for API token
    if (!AppSettings::hasApiToken()) {
        qWarning() << "No API token configured";
        createNoTokenMessage();
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
    showLoading();
    m_todoistClient->fetchProjects();
}

QWidget* AppController::mainWidget()
{
    return m_mainWidget;
}

void AppController::refresh()
{
    if (!m_todoistClient) {
        qWarning() << "Cannot refresh: No API client (token not configured?)";
        return;
    }

    showLoading();
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

    // Show the task list
    showTaskList();
}

void AppController::onError(const QString& error)
{
    qWarning() << "Todoist API error:" << error;
    showError(error);
}

void AppController::showLoading()
{
    m_taskListView->hide();
    m_errorLabel->hide();
    m_loadingLabel->show();
}

void AppController::showError(const QString& message)
{
    m_loadingLabel->hide();
    m_taskListView->hide();
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

void AppController::showTaskList()
{
    m_loadingLabel->hide();
    m_errorLabel->hide();
    m_taskListView->show();
}

void AppController::createNoTokenMessage()
{
    QString message =
        "API Token Not Configured\n\n"
        "To use Remarkable Todoist, you need to configure your Todoist API token.\n\n"
        "Steps:\n"
        "1. Get your token from:\n"
        "   https://todoist.com/app/settings/integrations/developer\n\n"
        "2. Create the config file:\n"
        "   mkdir -p ~/.config/remarkable-todoist\n\n"
        "3. Add your token:\n"
        "   echo \"[auth]\" > ~/.config/remarkable-todoist/config.ini\n"
        "   echo \"api_token=YOUR_TOKEN_HERE\" >> ~/.config/remarkable-todoist/config.ini\n\n"
        "4. Restart the application";

    showError(message);
}
