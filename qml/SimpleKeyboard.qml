import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: keyboard
    color: "#f0f0f0"
    border.color: "#333333"
    border.width: 2

    property var targetTextField: null
    property bool isShifted: false

    signal keyPressed(string key)

    function insertText(text) {
        if (targetTextField) {
            var textToInsert = isShifted ? text.toUpperCase() : text.toLowerCase()
            targetTextField.insert(targetTextField.cursorPosition, textToInsert)
        }
    }

    // Simple on-screen keyboard layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // Row 1: Numbers
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Repeater {
                model: ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0"]
                delegate: Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    text: modelData
                    onClicked: {
                        if (targetTextField) {
                            targetTextField.insert(targetTextField.cursorPosition, modelData)
                        }
                        keyPressed(modelData)
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.pressed ? "#d0d0d0" : "white"
                        border.color: "#333333"
                        border.width: 2
                    }
                }
            }
        }

        // Row 2: QWERTYUIOP
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Repeater {
                model: ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p"]
                delegate: Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    text: isShifted ? modelData.toUpperCase() : modelData
                    onClicked: {
                        insertText(modelData)
                        keyPressed(modelData)
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.pressed ? "#d0d0d0" : "white"
                        border.color: "#333333"
                        border.width: 2
                    }
                }
            }
        }

        // Row 3: ASDFGHJKL
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Item { Layout.preferredWidth: 20 }
            Repeater {
                model: ["a", "s", "d", "f", "g", "h", "j", "k", "l"]
                delegate: Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    text: isShifted ? modelData.toUpperCase() : modelData
                    onClicked: {
                        insertText(modelData)
                        keyPressed(modelData)
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.pressed ? "#d0d0d0" : "white"
                        border.color: "#333333"
                        border.width: 2
                    }
                }
            }
            Item { Layout.preferredWidth: 20 }
        }

        // Row 4: Shift + ZXCVBNM + Backspace
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            // Shift key
            Button {
                Layout.preferredWidth: 120
                Layout.preferredHeight: 60
                text: "⇧"
                onClicked: {
                    isShifted = !isShifted
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    font.bold: isShifted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: isShifted ? "#c0e0ff" : (parent.pressed ? "#d0d0d0" : "white")
                    border.color: "#333333"
                    border.width: isShifted ? 3 : 2
                }
            }

            Repeater {
                model: ["z", "x", "c", "v", "b", "n", "m"]
                delegate: Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    text: isShifted ? modelData.toUpperCase() : modelData
                    onClicked: {
                        insertText(modelData)
                        keyPressed(modelData)
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.pressed ? "#d0d0d0" : "white"
                        border.color: "#333333"
                        border.width: 2
                    }
                }
            }

            // Backspace
            Button {
                Layout.preferredWidth: 120
                Layout.preferredHeight: 60
                text: "⌫"
                onClicked: {
                    if (targetTextField && targetTextField.cursorPosition > 0) {
                        targetTextField.remove(targetTextField.cursorPosition - 1, targetTextField.cursorPosition)
                    }
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#d0d0d0" : "#ffdddd"
                    border.color: "#333333"
                    border.width: 2
                }
            }
        }

        // Row 5: Space bar and punctuation
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Button {
                Layout.preferredWidth: 100
                Layout.preferredHeight: 60
                text: ","
                onClicked: {
                    if (targetTextField) {
                        targetTextField.insert(targetTextField.cursorPosition, ",")
                    }
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#d0d0d0" : "white"
                    border.color: "#333333"
                    border.width: 2
                }
            }

            // Space bar
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                text: "SPACE"
                onClicked: {
                    if (targetTextField) {
                        targetTextField.insert(targetTextField.cursorPosition, " ")
                    }
                    keyPressed(" ")
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#d0d0d0" : "white"
                    border.color: "#333333"
                    border.width: 2
                }
            }

            Button {
                Layout.preferredWidth: 100
                Layout.preferredHeight: 60
                text: "."
                onClicked: {
                    if (targetTextField) {
                        targetTextField.insert(targetTextField.cursorPosition, ".")
                    }
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.pressed ? "#d0d0d0" : "white"
                    border.color: "#333333"
                    border.width: 2
                }
            }
        }
    }
}
