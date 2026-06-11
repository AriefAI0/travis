import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Displays generated timeline thumbnail paths for the loaded master video.

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

        RowLayout {
            Layout.fillWidth: true

            Label {
                color: "#f4f7fa"
                font.pixelSize: 16
                text: "Timeline Thumbnails"
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                color: "#9fb1bf"
                font.pixelSize: 12
                text: root.playbackViewModel
                    ? `${root.playbackViewModel.timelineThumbnailPaths.length} item(s)`
                    : "0 item(s)"
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.playbackViewModel ? root.playbackViewModel.timelineThumbnailPaths : []

            delegate: Rectangle {
                width: ListView.view.width
                height: 44
                color: index % 2 === 0 ? "#17212a" : "#111922"

                Label {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    verticalAlignment: Text.AlignVCenter
                    color: "#dce7ef"
                    elide: Text.ElideMiddle
                    text: modelData
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.playbackViewModel && root.playbackViewModel.timelineThumbnailPaths.length === 0
            color: "#9fb1bf"
            font.pixelSize: 12
            text: "No thumbnails have been generated for this master video yet."
            wrapMode: Text.Wrap
        }
    }
}
