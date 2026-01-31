#include <QApplication>
#include <QLabel>
#include <QDebug>

#include "models/taskmodel.h"
#include "config/settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    QCoreApplication::setApplicationName("remarkable-todoist");
    QCoreApplication::setOrganizationName("remarkable-todoist");

    // Verify TaskModel works
    TaskModel model;
    Q_ASSERT(model.rowCount() == 0);
    QVector<Task> emptyTasks;
    model.setTasks(emptyTasks);
    Q_ASSERT(model.rowCount() == 0);
    qDebug() << "TaskModel verification passed: rowCount =" << model.rowCount();

    // Verify Settings works
    qDebug() << "Config file path:" << AppSettings::configFilePath();
    qDebug() << "Has API token:" << AppSettings::hasApiToken();

    // Create placeholder window showing verification status
    QString status = QString("Hello reMarkable\n\n"
                            "TaskModel: OK\n"
                            "Settings: OK\n"
                            "Config: %1\n"
                            "Token configured: %2")
                        .arg(AppSettings::configFilePath())
                        .arg(AppSettings::hasApiToken() ? "Yes" : "No");

    QLabel *label = new QLabel(status);
    label->setWindowTitle("Remarkable Todoist");
    label->setAlignment(Qt::AlignCenter);
    label->resize(500, 350);
    label->show();

    return app.exec();
}
