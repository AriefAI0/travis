import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Compact statistic tile used inside project cards.

Rectangle {
    id: root

    property string label: ""
    property string value: ""

    implicitHeight: statLayout.implicitHeight + 24
    radius: 12
    color: "#20242d"

    ColumnLayout {
        id: statLayout

        anchors.fill: parent
        anchors.margins: 12
        spacing: 4

        Label {
            Layout.fillWidth: true
            color: "#78818f"
            font.pixelSize: 12
            text: root.label
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            color: "#e6e8eb"
            font.bold: true
            font.pixelSize: 18
            text: root.value
            elide: Text.ElideRight
        }
    }
}
