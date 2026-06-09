import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Provides the reusable inspection workspace frame with desktop-style pane separators.

Item {
    id: root

    property string title: "Inspection Workspace"
    property string subtitle: ""
    property alias headerActions: headerActionsSlot.data
    property alias controls: controlsSlot.data
    property alias mainContent: mainContentSlot.data
    property alias sideDock: sideDockSlot.data

    Rectangle {
        anchors.fill: parent
        color: "#101820"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 8
            Layout.bottomMargin: 8

            ColumnLayout {
                spacing: 4

                Label {
                    text: root.title
                    font.pixelSize: 22
                    color: "#f6f8fb"
                }

                Label {
                    color: "#9fb1bf"
                    text: root.subtitle
                }
            }

            Item {
                Layout.fillWidth: true
            }

            RowLayout {
                id: headerActionsSlot
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#2b3844"
        }

        ColumnLayout {
            id: controlsSlot

            Layout.fillWidth: true
            spacing: 0
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            ColumnLayout {
                id: mainContentSlot

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 3
                spacing: 0
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: "#2b3844"
            }

            ColumnLayout {
                id: sideDockSlot

                Layout.fillHeight: true
                Layout.preferredWidth: 360
            }
        }
    }
}
