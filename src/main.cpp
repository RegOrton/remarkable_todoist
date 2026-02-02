#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "controllers/appcontroller.h"
#include "models/taskmodel.h"
#include "network/sync_manager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Set application identity (for QSettings)
    QGuiApplication::setOrganizationName("remarkable-todoist");
    QGuiApplication::setApplicationName("Remarkable Todoist");

    // Use Basic style (simplest, good for e-ink)
    QQuickStyle::setStyle("Basic");

    // Create controller and initialize
    AppController controller;

    // Set up QML engine
    QQmlApplicationEngine engine;

    // Register SyncManager for QML
    qmlRegisterUncreatableType<SyncManager>("RemarkableTodoist", 1, 0, "SyncManager", "Access via appController.syncManager");

    // Expose controller and model to QML
    engine.rootContext()->setContextProperty("appController", &controller);
    engine.rootContext()->setContextProperty("taskModel", controller.taskModel());

    // Load QML
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML";
        return -1;
    }

    // Initialize after QML is loaded (starts fetching data)
    controller.initialize();

    return app.exec();
}
