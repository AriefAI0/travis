import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Lists result image evidence associated with the selected playback clip.

Rectangle {
    id: root

    property var playbackViewModel

    Layout.fillWidth: true
    Layout.fillHeight: true
    radius: 0
    color: "#121922"
    border.color: "#31404d"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Label {
            color: "#f4f7fa"
            font.bold: true
            font.pixelSize: 14
            text: "Evidence"
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.playbackViewModel ? root.playbackViewModel.resultImagePaths : []

            delegate: Rectangle {
                width: ListView.view.width
                height: 38
                color: index % 2 === 0 ? "#17212a" : "#111922"

                Label {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    verticalAlignment: Text.AlignVCenter
                    color: "#dce7ef"
                    elide: Text.ElideMiddle
                    text: modelData
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.playbackViewModel && root.playbackViewModel.resultImagePaths.length === 0
            color: "#9fb1bf"
            font.pixelSize: 12
            text: "No evidence images are loaded for the current clip."
            wrapMode: Text.Wrap
        }
    }
}
