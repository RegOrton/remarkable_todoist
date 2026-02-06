#include "appcontroller.h"

#include <QCoreApplication>
#include <QDebug>
#include <QUuid>
#include <QFileInfo>

#include "../models/task.h"
#include "../models/taskmodel.h"
#include "../network/todoist_client.h"
#include "../network/sync_manager.h"
#include "../config/settings.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_loading(false)
    , m_errorMessage("")
    , m_taskModel(nullptr)
    , m_todoistClient(nullptr)
    , m_syncManager(nullptr)
#ifdef ENABLE_OCR
    , m_recognizer(nullptr)
#endif
{
    // Create task model
    m_taskModel = new TaskModel(this);

#ifdef ENABLE_OCR
    // Create handwriting recognizer
    m_recognizer = new HandwritingRecognizer(this);
#endif
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

    // Create SyncManager after TodoistClient
    m_syncManager = new SyncManager(m_todoistClient, this);

#ifdef ENABLE_OCR
    // Initialize handwriting recognizer
    if (!m_recognizer->initialize()) {
        qWarning() << "Handwriting recognizer initialization failed (OCR may not work)";
    }
#else
    qDebug() << "OCR support not compiled - handwriting recognition unavailable";
#endif

    // Connect signals
    connect(m_todoistClient, &TodoistClient::projectsFetched,
            this, &AppController::onProjectsFetched);
    connect(m_todoistClient, &TodoistClient::tasksFetched,
            this, &AppController::onTasksFetched);
    connect(m_todoistClient, &TodoistClient::errorOccurred,
            this, &AppController::onError);

    // Connect SyncManager signals for task creation feedback
    connect(m_syncManager, &SyncManager::taskCreateSynced,
            this, [this](const QString& tempId, const QString& serverTaskId) {
        qDebug() << "Task synced, tempId:" << tempId << "-> serverId:" << serverTaskId;
    });

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

void AppController::quit()
{
    QCoreApplication::quit();
}

void AppController::completeTask(const QString& taskId)
{
    // Optimistic UI update
    m_taskModel->setTaskCompleted(taskId, true);

    // Queue for sync
    m_syncManager->queueTaskCompletion(taskId);
}

void AppController::createTask(const QString& content)
{
    if (content.trimmed().isEmpty()) {
        qWarning() << "Cannot create task with empty content";
        return;
    }

    // Generate temp ID for optimistic UI tracking
    QString tempId = "temp_" + QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Optimistic UI: add task to model immediately
    Task newTask;
    newTask.id = tempId;
    newTask.title = content.trimmed();
    newTask.priority = 1;  // Default priority
    newTask.completed = false;
    m_taskModel->addTask(newTask);

    // Queue for sync
    m_syncManager->queueTaskCreation(content.trimmed(), tempId);

    emit taskCreated();
    qDebug() << "Task created (optimistic):" << content << "tempId:" << tempId;
}

QString AppController::recognizeHandwriting(const QString& imagePath)
{
    qDebug() << "recognizeHandwriting called with path:" << imagePath;

#ifdef ENABLE_OCR
    if (!m_recognizer || !m_recognizer->isReady()) {
        qWarning() << "Handwriting recognizer not ready";
        return QString("ERROR: Recognizer not ready");
    }

    // Check if file exists and is readable
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) {
        qWarning() << "Image file does not exist:" << imagePath;
        return QString("ERROR: Image file not found");
    }
    if (!fileInfo.isReadable()) {
        qWarning() << "Image file not readable:" << imagePath;
        return QString("ERROR: Image file not readable");
    }

    qDebug() << "Image file found, size:" << fileInfo.size() << "bytes";

    QString result = m_recognizer->recognizeFile(imagePath);
    qDebug() << "Recognized text:" << result;

    if (result.isEmpty()) {
        return QString("No text detected - try writing larger and clearer");
    }

    return result;
#else
    Q_UNUSED(imagePath);
    qWarning() << "OCR not available - app was built without Tesseract support";
    return QString("ERROR: OCR not available - app built without handwriting recognition support");
#endif
}
