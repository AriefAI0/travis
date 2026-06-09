import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Provides the reusable inspection workspace frame around header, status, body, and side dock.

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
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0d1117" }
            GradientStop { position: 1.0; color: "#19222b" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 4

                Label {
                    text: root.title
                    font.pixelSize: 28
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

        ColumnLayout {
            id: controlsSlot

            Layout.fillWidth: true
            spacing: 8
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            ColumnLayout {
                id: mainContentSlot

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 3
                spacing: 12
            }

            ColumnLayout {
                id: sideDockSlot

                Layout.fillHeight: true
                Layout.preferredWidth: 360
            }
        }
    }
}
