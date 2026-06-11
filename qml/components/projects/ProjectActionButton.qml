import QtQuick
import QtQuick.Controls

// Rounded project-page action button matching the Electron dashboard button styles.

Button {
    id: root

    property bool primary: false

    implicitWidth: Math.max(112, labelText.implicitWidth + 32)
    implicitHeight: 38
    hoverEnabled: true

    contentItem: Label {
        id: labelText

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: root.enabled ? "#e6e8eb" : "#78818f"
        font.bold: true
        font.pixelSize: 13
        text: root.text
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: 10
        color: {
            if (!root.enabled) {
                return root.primary ? "#7a2428" : "#252a33"
            }
            if (root.primary) {
                return root.hovered ? "#d43a40" : "#c1272d"
            }
            return root.hovered ? "#252a33" : "#20242d"
        }
        border.color: root.primary
            ? (root.hovered ? "#d43a40" : "#c1272d")
            : (root.hovered ? "#c1272d" : "#3b424f")
        border.width: 1
    }
}
