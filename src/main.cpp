#include <QApplication>
#include <QLabel>
#include <QDebug>

#include "models/taskmodel.h"

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

    // Create placeholder window
    QLabel *label = new QLabel("Hello reMarkable\n\nTaskModel: OK");
    label->setWindowTitle("Remarkable Todoist");
    label->setAlignment(Qt::AlignCenter);
    label->resize(400, 300);
    label->show();

    return app.exec();
}
