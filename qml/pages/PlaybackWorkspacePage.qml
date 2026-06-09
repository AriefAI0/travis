import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../layouts"

// Placeholder for the playback workspace route.

WorkspaceShellLayout {
    id: root

    property var navigation
    property int projectId: 0
    property int masterVideoId: 0

    title: "Playback Workspace"
    subtitle: masterVideoId > 0
        ? `Playback route placeholder for video ${masterVideoId}`
        : "Playback workspace route placeholder"

    headerActions: [
        Button {
            text: "Project"
            onClicked: root.navigation.goProject(root.projectId)
        },

        Button {
            text: "Inspection"
            onClicked: root.navigation.goInspectionWorkspace(root.projectId)
        }
    ]

    body: [
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: "#121922"
            border.color: "#31404d"
            border.width: 1

            Label {
                anchors.centerIn: parent
                color: "#eef3f7"
                font.pixelSize: 20
                text: "Playback page migration pending"
            }
        }
    ]
}
