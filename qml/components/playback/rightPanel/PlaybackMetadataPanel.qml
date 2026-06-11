import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Secondary metadata rail for the loaded playback selection.

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
            text: "Metadata"
        }

        MetadataRow {
            label: "Master video"
            value: root.playbackViewModel && root.playbackViewModel.selectedMasterVideoId > 0
                ? String(root.playbackViewModel.selectedMasterVideoId)
                : "-"
        }

        MetadataRow {
            label: "Master path"
            value: root.playbackViewModel && root.playbackViewModel.selectedMasterVideoPath.length > 0
                ? root.playbackViewModel.selectedMasterVideoPath
                : "-"
        }

        MetadataRow {
            label: "Selected clip"
            value: root.playbackViewModel && root.playbackViewModel.selectedClipId > 0
                ? String(root.playbackViewModel.selectedClipId)
                : "-"
        }

        Item {
            Layout.fillHeight: true
        }
    }

    component MetadataRow: ColumnLayout {
        property string label: ""
        property string value: ""

        Layout.fillWidth: true
        spacing: 3

        Label {
            color: "#7f8b98"
            font.pixelSize: 10
            text: label
        }

        Label {
            Layout.fillWidth: true
            color: "#dce7ef"
            font.pixelSize: 12
            text: value
            elide: Text.ElideMiddle
            wrapMode: Text.Wrap
        }
    }
}
