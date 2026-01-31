#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMap>
#include <QVector>
#include "../models/task.h"

class TodoistClient;
class TaskModel;
class TaskListView;

/**
 * AppController - Orchestrates the application components
 *
 * Mediates between TodoistClient, TaskModel, and TaskListView.
 * Handles the startup flow:
 * 1. Check for API token
 * 2. Fetch projects (for name lookup)
 * 3. Fetch tasks
 * 4. Populate model and display list
 *
 * Also handles error display and refresh requests.
 */
class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    /**
     * Initialize the application
     * Call after construction to start the fetch flow
     */
    void initialize();

    /**
     * Get the main window widget
     * The returned widget should be shown and managed by the caller
     */
    QWidget* mainWidget();

    /**
     * Refresh the task list
     * Fetches projects then tasks from Todoist API
     */
    void refresh();

private slots:
    void onProjectsFetched(const QMap<QString, QString>& projects);
    void onTasksFetched(const QVector<Task>& tasks);
    void onError(const QString& error);

private:
    void showLoading();
    void showError(const QString& message);
    void showTaskList();
    void createNoTokenMessage();

    // Main window container
    QWidget* m_mainWidget;
    QVBoxLayout* m_layout;

    // UI components
    TaskListView* m_taskListView;
    QLabel* m_loadingLabel;
    QLabel* m_errorLabel;

    // Data layer
    TaskModel* m_taskModel;
    TodoistClient* m_todoistClient;
};

#endif // APPCONTROLLER_H
