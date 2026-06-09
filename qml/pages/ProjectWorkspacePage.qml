import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../layouts"

// Placeholder for a selected project workspace route.

WorkspaceShellLayout {
    id: root

    property var navigation
    property int projectId: 0

    title: "Project Workspace"
    subtitle: projectId > 0
        ? `Project ${projectId} workspace route placeholder`
        : "Project workspace route placeholder"

    headerActions: [
        Button {
            text: "Projects"
            onClicked: root.navigation.goProjects()
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
                text: "Project workspace migration pending"
            }
        }
    ]
}
