import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../projects"

// Project workspace header matching the Electron workspace route.

RowLayout {
    id: root

    property string projectTitle: "Project Workspace"
    property bool loading: false
    property bool canInspect: false
    signal backRequested()
    signal refreshRequested()
    signal inspectionRequested()
    signal projectSelected()

    spacing: 16

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 4

        Button {
            id: backButton

            text: "< Back to project dashboard"
            flat: true
            hoverEnabled: true
            padding: 0
            onClicked: root.backRequested()

            contentItem: Label {
                color: backButton.hovered ? "#d43a40" : "#a8b0bd"
                font.pixelSize: 12
                text: backButton.text
            }
        }

        Label {
            color: "#78818f"
            font.bold: true
            font.pixelSize: 11
            text: "PROJECT WORKSPACE"
        }

        Button {
            id: titleButton

            Layout.fillWidth: true
            flat: true
            padding: 0
            hoverEnabled: true
            text: root.projectTitle
            onClicked: root.projectSelected()

            contentItem: Label {
                Layout.fillWidth: true
                color: titleButton.hovered ? "#d43a40" : "#e6e8eb"
                font.bold: true
                font.pixelSize: 18
                text: titleButton.text
                elide: Text.ElideRight
            }
        }
    }

    RowLayout {
        spacing: 8

        ProjectActionButton {
            text: "Refresh"
            enabled: !root.loading
            onClicked: root.refreshRequested()
        }

        ProjectActionButton {
            primary: true
            text: "START INSPECTION"
            enabled: root.canInspect && !root.loading
            onClicked: root.inspectionRequested()
        }
    }
}
