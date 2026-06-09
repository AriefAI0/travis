import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../layouts"

// Placeholder for the project dashboard route while project UI migration is pending.

WorkspaceShellLayout {
    id: root

    property var navigation

    title: "Projects"
    subtitle: "Project dashboard route placeholder. Project listing will be wired through the native service/viewmodel layer."

    body: [
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: "#121922"
            border.color: "#31404d"
            border.width: 1

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12

                Label {
                    color: "#eef3f7"
                    font.pixelSize: 20
                    text: "Project dashboard migration pending"
                }

                Button {
                    text: "Open Inspection Workspace"
                    onClicked: root.navigation.goInspectionWorkspace(0)
                }
            }
        }
    ]
}
