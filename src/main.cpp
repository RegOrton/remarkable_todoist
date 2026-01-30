#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    QCoreApplication::setApplicationName("remarkable-todoist");
    QCoreApplication::setOrganizationName("remarkable-todoist");

    // Create placeholder window
    QLabel *label = new QLabel("Hello reMarkable");
    label->setWindowTitle("Remarkable Todoist");
    label->setAlignment(Qt::AlignCenter);
    label->resize(400, 300);
    label->show();

    return app.exec();
}
