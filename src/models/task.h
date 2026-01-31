#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDate>
#include <QJsonObject>

struct Task {
    QString id;
    QString title;
    QString projectId;
    QString projectName;
    QDate dueDate;
    int priority;  // 1-4, where 4 is highest in Todoist API
    bool completed;

    // Default constructor
    Task() : priority(0), completed(false) {}

    // Parse from Todoist API JSON response
    static Task fromJson(const QJsonObject& obj);

    // Return priority label for display (P1-P4, where P1 is highest)
    QString priorityLabel() const;

    // Return formatted due date or empty string
    QString dueDateDisplay() const;
};

#endif // TASK_H
