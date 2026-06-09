import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Provides a reusable desktop workspace layout with line-divided panes.

Item {
    id: root

    property string title: ""
    property string subtitle: ""
    property alias headerActions: headerActionsSlot.data
    property alias body: bodySlot.data

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
                    color: "#f6f8fb"
                    font.pixelSize: 22
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

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#2b3844"
        }

        ColumnLayout {
            id: bodySlot

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
        }
    }
}
