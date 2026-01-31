#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "task.h"

class TaskModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum TaskRoles {
        TitleRole = Qt::UserRole + 1,
        DueDateRole,
        ProjectRole,
        PriorityRole,
        CompletedRole
    };

    explicit TaskModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Custom methods
    void setTasks(const QVector<Task> &tasks);
    const Task& taskAt(int row) const;
    int taskCount() const;

private:
    QVector<Task> m_tasks;
};

#endif // TASKMODEL_H
