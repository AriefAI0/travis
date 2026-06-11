import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../../projects"

// Displays one finalized master recording available for review playback.

Rectangle {
    id: root

    property var recording: ({})
    signal openPlaybackRequested(int masterVideoId)

    Layout.fillWidth: true
    implicitHeight: recordingRow.implicitHeight + 16
    radius: 6
    color: "#20242d"
    border.color: "#303642"

    RowLayout {
        id: recordingRow

        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                Layout.fillWidth: true
                color: "#e6e8eb"
                font.pixelSize: 12
                text: root.recording.sourceName || ("Master Video #" + root.recording.masterVideoId)
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.pixelSize: 11
                text: root.recording.fileUrl
                elide: Text.ElideMiddle
            }
        }

        ProjectActionButton {
            text: "Open review"
            enabled: root.recording.masterVideoId > 0 && root.recording.status === "finalized"
            onClicked: root.openPlaybackRequested(root.recording.masterVideoId)
        }
    }
}
