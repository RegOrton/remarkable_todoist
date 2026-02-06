import QtQuick 2.15

Item {
    id: drawingCanvas

    // Public API
    signal cleared()

    // Property that updates when strokes change
    property bool hasStrokes: false

    function clear() {
        strokes = []
        currentStroke = []
        hasStrokes = false
        canvas.requestPaint()
        cleared()
    }

    function isEmpty() {
        return !hasStrokes
    }

    function save(filePath) {
        return canvas.save(filePath)
    }

    function grabToImage(callback) {
        return canvas.grabToImage(callback)
    }

    // Stroke data
    property var strokes: []        // Array of completed strokes
    property var currentStroke: []  // Current in-progress stroke

    // Visual border
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "#333333"
        border.width: 2
    }

    // Drawing canvas
    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 2  // Inside the border

        property real lineWidth: 8  // Increased from 3 for better OCR recognition

        onPaint: {
            var ctx = getContext("2d")

            // Clear and fill white background
            ctx.fillStyle = "white"
            ctx.fillRect(0, 0, width, height)

            // Configure stroke style
            ctx.strokeStyle = "black"
            ctx.lineWidth = lineWidth
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            // Draw all completed strokes
            for (var i = 0; i < strokes.length; i++) {
                var stroke = strokes[i]
                if (stroke.length < 2) continue

                ctx.beginPath()
                ctx.moveTo(stroke[0].x, stroke[0].y)

                for (var j = 1; j < stroke.length; j++) {
                    ctx.lineTo(stroke[j].x, stroke[j].y)
                }

                ctx.stroke()
            }

            // Draw current in-progress stroke
            if (currentStroke.length >= 2) {
                ctx.beginPath()
                ctx.moveTo(currentStroke[0].x, currentStroke[0].y)

                for (var k = 1; k < currentStroke.length; k++) {
                    ctx.lineTo(currentStroke[k].x, currentStroke[k].y)
                }

                ctx.stroke()
            }
        }
    }

    // Input handling
    MouseArea {
        anchors.fill: canvas

        onPressed: function(mouse) {
            console.log("Mouse pressed at:", mouse.x, mouse.y)
            // Start new stroke
            currentStroke = [{x: mouse.x, y: mouse.y}]
            canvas.requestPaint()
        }

        onPositionChanged: function(mouse) {
            // Add point to current stroke
            if (currentStroke.length > 0) {
                currentStroke.push({x: mouse.x, y: mouse.y})
                canvas.requestPaint()
            }
        }

        onReleased: function(mouse) {
            console.log("Mouse released, currentStroke length:", currentStroke.length)
            // Finalize stroke
            if (currentStroke.length > 0) {
                // Add completed stroke to strokes array
                var completedStroke = currentStroke
                strokes.push(completedStroke)
                hasStrokes = true  // Update property to trigger binding
                console.log("Stroke added! Total strokes now:", strokes.length, "hasStrokes:", hasStrokes)

                // Reset current stroke
                currentStroke = []

                // Repaint with finalized stroke
                canvas.requestPaint()
            }
        }
    }
}
