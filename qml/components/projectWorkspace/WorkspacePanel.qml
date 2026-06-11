import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shared bordered panel for the project workspace desktop layout.

Rectangle {
    id: root

    default property alias content: contentSlot.data

    radius: 8
    color: "#1b1f27"
    border.color: "#303642"
    border.width: 1
    clip: true

    ColumnLayout {
        id: contentSlot

        anchors.fill: parent
        anchors.margins: 12
        spacing: 12
    }
}
