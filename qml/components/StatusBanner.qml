import QtQuick
import QtQuick.Controls

// Shows backend status or error messages in a compact reusable banner.

Rectangle {
    id: root

    property string message: ""
    property bool error: false

    implicitHeight: visible ? messageText.implicitHeight + 14 : 0
    radius: 0
    color: error ? "#3d1f1f" : "#1d2c37"
    border.color: error ? "#d96c6c" : "#5e8fb3"
    border.width: 1
    visible: message.length > 0

    Label {
        id: messageText

        anchors.fill: parent
        anchors.margins: 7
        color: error ? "#ffd2d2" : "#d8ebfb"
        text: root.message
        wrapMode: Text.Wrap
    }
}
