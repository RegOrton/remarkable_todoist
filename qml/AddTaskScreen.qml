import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: addTaskScreen

    // Signals for parent integration
    signal taskSubmitted(string taskContent)
    signal cancelled()

    // Properties for parent to control
    property string recognizedText: ""
    property bool recognizing: false

    // Expose canvas for parent to grab image
    property alias canvas: drawingCanvas

    // Helper to get canvas image
    function getCanvasImage() {
        return drawingCanvas
    }

    // E-ink colors (match main.qml)
    readonly property color backgroundColor: "white"
    readonly property color textColor: "black"
    readonly property color borderColor: "#333333"
    readonly property color mutedColor: "#666666"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header (100px)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: backgroundColor
            border.color: borderColor
            border.width: 2

            RowLayout {
                anchors.fill: parent
                anchors.margins: 20

                // Cancel button (left)
                Button {
                    text: "Cancel"
                    onClicked: cancelled()

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 24
                        color: textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitWidth: 140
                        implicitHeight: 60
                        color: parent.pressed ? "#e0e0e0" : backgroundColor
                        border.color: borderColor
                        border.width: 3
                    }
                }

                Item { Layout.fillWidth: true }

                // Title (center)
                Text {
                    text: "Add Task"
                    font.pixelSize: 36
                    font.bold: true
                    color: textColor
                    Layout.alignment: Qt.AlignHCenter
                }

                Item { Layout.fillWidth: true }

                // Clear button (right)
                Button {
                    text: "Clear"
                    onClicked: {
                        drawingCanvas.clear()
                        recognizedText = ""
                    }

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 24
                        color: textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitWidth: 140
                        implicitHeight: 60
                        color: parent.pressed ? "#e0e0e0" : backgroundColor
                        border.color: borderColor
                        border.width: 3
                    }
                }
            }
        }

        // Drawing area (reduced to make room for keyboard)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 500
            color: backgroundColor
            border.color: borderColor
            border.width: 2

            // Placeholder text (shown when canvas is empty)
            Text {
                anchors.centerIn: parent
                text: "Write task name here..."
                font.pixelSize: 28
                color: mutedColor
                visible: drawingCanvas.isEmpty()
            }

            DrawingCanvas {
                id: drawingCanvas
                anchors.fill: parent
                anchors.margins: 10
            }
        }

        // Preview/Input area (~250px)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            color: backgroundColor
            border.color: borderColor
            border.width: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Text {
                    text: "Task name:"
                    font.pixelSize: 20
                    color: mutedColor
                }

                // Text input field for typing
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: backgroundColor
                    border.color: borderColor
                    border.width: 2

                    TextField {
                        id: taskTextField
                        anchors.fill: parent
                        anchors.margins: 10
                        font.pixelSize: 24
                        color: textColor
                        placeholderText: "Type task name or use OCR..."
                        background: Item {}

                        onTextChanged: {
                            recognizedText = text
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: "Recognize Drawing"
                        enabled: !drawingCanvas.isEmpty() && !recognizing
                        Layout.fillWidth: true
                        onClicked: {
                            recognizing = true
                            canvas.save("/tmp/remarkable-todoist-canvas.png")
                            var result = appController.recognizeHandwriting("/tmp/remarkable-todoist-canvas.png")
                            taskTextField.text = result
                            recognizing = false
                        }

                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 20
                            color: parent.enabled ? textColor : mutedColor
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            implicitHeight: 55
                            color: parent.pressed ? "#e0e0e0" : backgroundColor
                            border.color: parent.enabled ? borderColor : mutedColor
                            border.width: 3
                        }
                    }

                    Text {
                        text: recognizing ? "Processing..." : ""
                        font.pixelSize: 18
                        color: mutedColor
                        visible: recognizing
                    }
                }
            }
        }

        // On-screen keyboard (~350px)
        SimpleKeyboard {
            Layout.fillWidth: true
            Layout.preferredHeight: 350
            targetTextField: taskTextField
        }

        // Action area (~100px)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: backgroundColor

            Button {
                anchors.centerIn: parent
                text: "Submit Task"
                enabled: recognizedText.length > 0 && !recognizing
                onClicked: taskSubmitted(recognizedText)

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 28
                    font.bold: true
                    color: parent.enabled ? textColor : mutedColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    implicitWidth: parent.parent.width - 80
                    implicitHeight: 70
                    color: parent.pressed ? "#e0e0e0" : backgroundColor
                    border.color: parent.enabled ? borderColor : mutedColor
                    border.width: 3
                }
            }
        }
    }
}
