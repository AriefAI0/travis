import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Reusable card section for project workspace detail content.

Rectangle {
    id: root

    property string title: ""
    property string meta: ""
    default property alias content: cardContent.data

    implicitHeight: cardColumn.implicitHeight + 24
    radius: 8
    color: "#20242d"
    border.color: "#303642"
    border.width: 1

    ColumnLayout {
        id: cardColumn

        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 14
                text: root.title
            }

            Label {
                visible: root.meta.length > 0
                color: "#78818f"
                font.pixelSize: 11
                text: root.meta
            }
        }

        ColumnLayout {
            id: cardContent

            Layout.fillWidth: true
            spacing: 8
        }
    }
}
