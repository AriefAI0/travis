import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Provides a reusable framed workspace layout for project, playback, and utility pages.

Item {
    id: root

    property string title: ""
    property string subtitle: ""
    property alias headerActions: headerActionsSlot.data
    property alias body: bodySlot.data

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
                    color: "#f6f8fb"
                    font.pixelSize: 28
                    text: root.title
                }

                Label {
                    color: "#9fb1bf"
                    text: root.subtitle
                    wrapMode: Text.Wrap
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
            id: bodySlot

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
        }
    }
}
