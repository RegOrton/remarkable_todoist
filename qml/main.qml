import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    visibility: Window.FullScreen
    width: 1404
    height: 1872
    title: "Remarkable Todoist"

    // High contrast colors for e-ink
    readonly property color backgroundColor: "white"
    readonly property color textColor: "black"
    readonly property color borderColor: "#333333"
    readonly property color mutedColor: "#666666"

    color: backgroundColor

    // StackView for navigation (no animations for e-ink)
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: taskListPage

        // Disable animations for e-ink
        pushEnter: null
        pushExit: null
        popEnter: null
        popExit: null

        // Task list page (main screen)
        Component {
            id: taskListPage

            ColumnLayout {
                spacing: 0

                // Header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    color: backgroundColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 24

                        Text {
                            text: "Todoist"
                            font.pixelSize: 36
                            font.bold: true
                            color: textColor
                        }

                        // Sync status indicator
                        RowLayout {
                            spacing: 12

                            Rectangle {
                                width: 16
                                height: 16
                                radius: 8
                                color: appController.syncManager ? (appController.syncManager.isOnline ? "#4CAF50" : "#F44336") : "#888888"
                            }

                            Text {
                                text: {
                                    if (!appController.syncManager) {
                                        return "..."
                                    } else if (!appController.syncManager.isOnline) {
                                        return "Offline"
                                    } else if (appController.syncManager.pendingCount > 0) {
                                        return "Syncing " + appController.syncManager.pendingCount + "..."
                                    } else if (appController.syncManager.isSyncing) {
                                        return "Syncing..."
                                    } else {
                                        return "Synced"
                                    }
                                }
                                font.pixelSize: 20
                                color: mutedColor
                            }
                        }

                        Item { Layout.fillWidth: true }

                        // Add Task button
                        Button {
                            text: "Add"
                            onClicked: stackView.push(addTaskPage)

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 24
                                color: textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 70
                                color: parent.pressed ? "#e0e0e0" : backgroundColor
                                border.color: borderColor
                                border.width: 3
                            }
                        }

                        Button {
                            text: "Refresh"
                            enabled: !appController.loading
                            onClicked: appController.refresh()

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 24
                                color: textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 160
                                implicitHeight: 70
                                color: parent.pressed ? "#e0e0e0" : backgroundColor
                                border.color: borderColor
                                border.width: 3
                            }
                        }

                        Button {
                            text: "Exit"
                            onClicked: appController.quit()

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 24
                                color: textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 70
                                color: parent.pressed ? "#e0e0e0" : backgroundColor
                                border.color: borderColor
                                border.width: 3
                            }
                        }
                    }

                    // Bottom border
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: borderColor
                    }
                }

                // Content area
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // Loading indicator
                    Text {
                        anchors.centerIn: parent
                        text: "Loading tasks..."
                        font.pixelSize: 36
                        color: mutedColor
                        visible: appController.loading
                    }

                    // Error message
                    Text {
                        anchors.centerIn: parent
                        anchors.margins: 40
                        width: parent.width - 80
                        text: appController.errorMessage
                        font.pixelSize: 24
                        color: textColor
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        visible: appController.errorMessage !== "" && !appController.loading
                    }

                    // Task list
                    ListView {
                        id: taskList
                        anchors.fill: parent
                        model: taskModel
                        visible: !appController.loading && appController.errorMessage === ""
                        clip: true

                        // Touch-friendly scrolling
                        flickDeceleration: 1500
                        maximumFlickVelocity: 2000

                        delegate: TaskDelegate {
                            width: taskList.width
                        }

                        // Scrollbar
                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                            width: 20
                        }
                    }
                }
            }
        }

        // Add task page
        Component {
            id: addTaskPage

            AddTaskScreen {
                // Wire up task submission
                onTaskSubmitted: function(taskContent) {
                    appController.createTask(taskContent)
                    stackView.pop()
                }

                // Wire up cancel
                onCancelled: {
                    stackView.pop()
                }
            }
        }
    }
}
