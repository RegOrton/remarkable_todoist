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
        case TitleRole:
            return task.title;
        case DueDateRole:
            return task.dueDateDisplay();
        case ProjectRole:
            return task.projectName;
        case PriorityRole:
            return task.priorityLabel();
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
    roles[TitleRole] = "title";
    roles[DueDateRole] = "dueDate";
    roles[ProjectRole] = "project";
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
