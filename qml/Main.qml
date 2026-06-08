import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Hosts the first native preview surface so the media-engine render path can be validated end to end.

ApplicationWindow {
    id: root

    width: 1280
    height: 720
    visible: true
    title: "Travis Preview Shell"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Label {
            text: "Native preview shell"
            font.pixelSize: 24
        }

        Rectangle {
            id: previewSurface
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#101418"
            border.color: "#3a4652"
            border.width: 1
            radius: 8
        }

        Label {
            Layout.fillWidth: true
            visible: previewSurfaceController.lastError.length > 0
            color: "#d96c6c"
            text: previewSurfaceController.lastError
            wrapMode: Text.Wrap
        }
    }

    Component.onCompleted: {
        previewSurfaceController.previewItem = previewSurface
    }
}
