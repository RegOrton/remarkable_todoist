#include <QApplication>
#include "controllers/appcontroller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application identity (for QSettings)
    QApplication::setOrganizationName("remarkable-todoist");
    QApplication::setApplicationName("Remarkable Todoist");

    // Create and initialize controller
    AppController controller;
    controller.initialize();

    // Get main widget and configure window
    QWidget* mainWindow = controller.mainWidget();
    mainWindow->setWindowTitle("Remarkable Todoist");

    // Set window size appropriate for reMarkable 2 (1404x1872 portrait)
    // For development, use smaller window; on device will be fullscreen
    mainWindow->resize(700, 900);

    mainWindow->show();

    return app.exec();
}
