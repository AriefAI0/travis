import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Dashboard card for one project, based on the Electron project card design.

Rectangle {
    id: root

    property var project
    signal openRequested(int projectId)

    width: 320
    implicitHeight: cardLayout.implicitHeight + 40
    radius: 18
    color: mouseArea.containsMouse ? "#20242d" : "#1b1f27"
    border.color: mouseArea.containsMouse ? "#d43a40" : "#303642"
    border.width: 1

    Behavior on color {
        ColorAnimation {
            duration: 120
        }
    }

    ColumnLayout {
        id: cardLayout

        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    color: "#78818f"
                    font.bold: true
                    font.pixelSize: 12
                    text: "Project #" + (root.project.projectId || "")
                }

                Label {
                    Layout.fillWidth: true
                    color: "#e6e8eb"
                    font.bold: true
                    font.pixelSize: 20
                    text: root.project.title
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignTop
                implicitWidth: progressLabel.implicitWidth + 20
                implicitHeight: progressLabel.implicitHeight + 12
                radius: implicitHeight / 2
                color: "#351c22"

                Label {
                    id: progressLabel

                    anchors.centerIn: parent
                    color: "#d43a40"
                    font.bold: true
                    font.pixelSize: 12
                    text: (root.project.overallProgress || 0) + "% complete"
                }
            }
        }

        Label {
            Layout.fillWidth: true
            color: "#a8b0bd"
            font.pixelSize: 13
            text: root.project.description && root.project.description.trim().length > 0
                ? root.project.description
                : "No description provided."
            wrapMode: Text.Wrap
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 12

            ProjectStatTile {
                Layout.fillWidth: true
                label: "Document ID"
                value: root.project.documentId && root.project.documentId.trim().length > 0
                    ? root.project.documentId
                    : "-"
            }

            ProjectStatTile {
                Layout.fillWidth: true
                label: "Assets"
                value: String(root.project.totalAssets || 0)
            }

            ProjectStatTile {
                Layout.fillWidth: true
                label: "Components"
                value: String(root.project.totalComponents || 0)
            }

            ProjectStatTile {
                Layout.fillWidth: true
                label: "Total items"
                value: String(root.project.totalItems || 0)
            }

            ProjectStatTile {
                Layout.fillWidth: true
                label: "Completed"
                value: String(root.project.completedItems || 0)
            }

            ProjectStatTile {
                Layout.fillWidth: true
                label: "Pending"
                value: String(root.project.pendingItems || 0)
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: root.openRequested(root.project.projectId)
    }
}
