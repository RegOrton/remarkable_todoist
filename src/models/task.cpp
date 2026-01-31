#include "task.h"
#include <QJsonValue>

Task Task::fromJson(const QJsonObject& obj)
{
    Task task;

    task.id = obj["id"].toString();
    task.title = obj["content"].toString();
    task.projectId = obj["project_id"].toString();
    task.priority = obj["priority"].toInt(1);  // Default to 1 (lowest) if not present
    task.completed = obj["is_completed"].toBool(false);

    // Parse due date if present
    QJsonValue dueValue = obj["due"];
    if (dueValue.isObject()) {
        QJsonObject dueObj = dueValue.toObject();
        QString dateStr = dueObj["date"].toString();
        if (!dateStr.isEmpty()) {
            task.dueDate = QDate::fromString(dateStr, Qt::ISODate);
        }
    }

    // Note: projectName will be filled in by the caller after matching with project data

    return task;
}

QString Task::priorityLabel() const
{
    // Todoist API: 1=lowest, 4=highest
    // UI display: P1=highest, P4=lowest
    switch (priority) {
        case 4: return "P1";
        case 3: return "P2";
        case 2: return "P3";
        case 1: return "P4";
        default: return "";
    }
}

QString Task::dueDateDisplay() const
{
    if (!dueDate.isValid()) {
        return "";
    }

    // Format as "Mon, Jan 30" or similar readable format
    return dueDate.toString("ddd, MMM d");
}
