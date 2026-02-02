import QtQuick
import QtQuick.Layouts

Item {
    id: delegate
    height: 110

    // High contrast colors for e-ink
    readonly property color textColor: "black"
    readonly property color borderColor: "#333333"
    readonly property color mutedColor: "#666666"
    readonly property color completedColor: "#999999"

    Rectangle {
        anchors.fill: parent
        color: "white"

        // Bottom border
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 2
            color: "#cccccc"
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 24

            // Checkbox (56x56 for touch)
            Rectangle {
                Layout.preferredWidth: 56
                Layout.preferredHeight: 56
                Layout.alignment: Qt.AlignVCenter
                color: "white"
                border.color: borderColor
                border.width: 2

                // Checkmark for completed tasks
                Text {
                    anchors.centerIn: parent
                    text: model.completed ? "✓" : ""
                    font.pixelSize: 36
                    font.bold: true
                    color: textColor
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // Only allow completing non-completed tasks
                        if (!model.completed) {
                            appController.completeTask(model.id)
                        }
                    }
                }
            }

            // Task content
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                // Task title
                Text {
                    Layout.fillWidth: true
                    text: model.title
                    font.pixelSize: 26
                    font.bold: true
                    font.strikeout: model.completed
                    color: model.completed ? completedColor : textColor
                    elide: Text.ElideRight
                    maximumLineCount: 2
                    wrapMode: Text.WordWrap
                }

                // Metadata row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    // Project name
                    Text {
                        text: model.projectName
                        font.pixelSize: 20
                        color: mutedColor
                        visible: model.projectName !== ""
                    }

                    Text {
                        text: "|"
                        font.pixelSize: 20
                        color: mutedColor
                        visible: model.projectName !== "" && model.dueDate !== ""
                    }

                    // Due date
                    Text {
                        text: model.dueDate
                        font.pixelSize: 20
                        color: mutedColor
                        visible: model.dueDate !== ""
                    }

                    Text {
                        text: "|"
                        font.pixelSize: 20
                        color: mutedColor
                        visible: (model.projectName !== "" || model.dueDate !== "") && model.priority > 1
                    }

                    // Priority
                    Text {
                        text: "P" + model.priority
                        font.pixelSize: 20
                        font.bold: true
                        color: {
                            switch(model.priority) {
                                case 4: return "#d1453b"  // P1 - red
                                case 3: return "#eb8909"  // P2 - orange
                                case 2: return "#246fe0"  // P3 - blue
                                default: return mutedColor
                            }
                        }
                        visible: model.priority > 1
                    }

                    Item { Layout.fillWidth: true }
                }
            }
        }
    }
}
