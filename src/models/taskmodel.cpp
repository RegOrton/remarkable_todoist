#include "taskmodel.h"

TaskModel::TaskModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_tasks.size();
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tasks.size()) {
        return QVariant();
    }

    const Task &task = m_tasks[index.row()];

    switch (role) {
        case IdRole:
            return task.id;
        case TitleRole:
            return task.title;
        case DueDateRole:
            return task.dueDateDisplay();
        case ProjectNameRole:
            return task.projectName;
        case PriorityRole:
            return task.priority;  // Return numeric priority for QML
        case CompletedRole:
            return task.completed;
        case Qt::DisplayRole:
            return task.title;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[DueDateRole] = "dueDate";
    roles[ProjectNameRole] = "projectName";
    roles[PriorityRole] = "priority";
    roles[CompletedRole] = "completed";
    return roles;
}

void TaskModel::setTasks(const QVector<Task> &tasks)
{
    beginResetModel();
    m_tasks = tasks;
    endResetModel();
}

const Task& TaskModel::taskAt(int row) const
{
    return m_tasks[row];
}

int TaskModel::taskCount() const
{
    return m_tasks.size();
}

void TaskModel::setTaskCompleted(const QString& taskId, bool completed)
{
    // Find task by ID in m_tasks vector
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId) {
            // Check if state actually changed
            if (m_tasks[i].completed == completed) {
                return;  // No change needed
            }

            // Update task state
            m_tasks[i].completed = completed;

            // Notify QML that this specific row's CompletedRole changed
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {CompletedRole});
            return;
        }
    }

    // Task not found - this is OK (might have been deleted elsewhere)
    qDebug() << "setTaskCompleted: Task" << taskId << "not found in model";
}
