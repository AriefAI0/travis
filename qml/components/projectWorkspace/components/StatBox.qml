import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Compact metric tile used inside project workspace detail cards.

Rectangle {
    property string label: ""
    property string value: ""

    Layout.fillWidth: true
    implicitHeight: 58
    radius: 6
    color: "#252a33"
    border.color: "#303642"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 3

        Label {
            color: "#78818f"
            font.pixelSize: 10
            text: label
        }

        Label {
            Layout.fillWidth: true
            color: "#e6e8eb"
            font.bold: true
            font.pixelSize: 13
            text: value
            elide: Text.ElideRight
        }
    }
}
