import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shows project recording paths and status in the right workspace panel.

ColumnLayout {
    id: root

    property var recordings: []

    spacing: 8

    Label {
        Layout.fillWidth: true
        visible: root.recordings.length === 0
        color: "#78818f"
        font.pixelSize: 12
        text: "No recordings for this project yet."
        wrapMode: Text.Wrap
    }

    Repeater {
        model: root.recordings

        delegate: ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            Label {
                Layout.fillWidth: true
                color: "#c9ced6"
                font.pixelSize: 12
                text: modelData.fileUrl
                elide: Text.ElideMiddle
            }

            Label {
                Layout.fillWidth: true
                color: modelData.status === "finalized" ? "#78818f" : "#d9a441"
                font.pixelSize: 11
                text: modelData.status || "unknown"
                elide: Text.ElideRight
            }
        }
    }
}
