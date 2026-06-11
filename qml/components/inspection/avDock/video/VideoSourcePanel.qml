import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shows the selected video source and exposes source/preview actions to the dock container.

ColumnLayout {
    id: root

    property var recordingViewModel
    property var previewController
    property var sourceDiscoveryViewModel

    signal openSourceRequested()
    signal removeSourceRequested()
    signal stopPreviewRequested()

    spacing: 10

    function hasVideoSource() {
        return recordingViewModel && recordingViewModel.sourceName.length > 0
    }

    function videoSourceKindLabel() {
        if (!recordingViewModel || recordingViewModel.sourceKind.length === 0) {
            return "Not configured"
        }

        if (recordingViewModel.sourceKind === "ndi") {
            return "NDI"
        }

        if (recordingViewModel.sourceKind === "device-capture") {
            return "Device Capture"
        }

        return recordingViewModel.sourceKind
    }

    function videoSourceDetail() {
        if (!recordingViewModel) {
            return ""
        }

        if (recordingViewModel.sourceKind === "ndi") {
            return recordingViewModel.urlAddress.length > 0 ? recordingViewModel.urlAddress : "NDI network source"
        }

        if (recordingViewModel.devicePath.length > 0) {
            return recordingViewModel.devicePath
        }

        return recordingViewModel.sourceElement.length > 0 ? recordingViewModel.sourceElement : "Local capture device"
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 72

        ColumnLayout {
            anchors.fill: parent
            spacing: 3

            Label {
                Layout.fillWidth: true
                color: "#eef3f7"
                font.pixelSize: 12
                text: root.hasVideoSource()
                    ? recordingViewModel.sourceName
                    : "No video source selected"
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                color: "#9fb0be"
                font.pixelSize: 11
                text: root.hasVideoSource()
                    ? root.videoSourceKindLabel() + " - " + root.videoSourceDetail()
                    : "Open the source dialog to configure one"
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                visible: recordingViewModel && recordingViewModel.sourceLabel.length > 0
                color: "#9fb0be"
                font.pixelSize: 11
                text: recordingViewModel ? "Label: " + recordingViewModel.sourceLabel : ""
                wrapMode: Text.Wrap
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Button {
            implicitHeight: 24
            Layout.fillWidth: true
            text: "+ Source"
            enabled: recordingViewModel && sourceDiscoveryViewModel
            onClicked: root.openSourceRequested()
        }

        Button {
            implicitHeight: 24
            Layout.fillWidth: true
            text: "- Source"
            enabled: recordingViewModel && root.hasVideoSource()
            onClicked: root.removeSourceRequested()
        }

        Button {
            implicitHeight: 24
            Layout.fillWidth: true
            text: "Stop Preview"
            enabled: previewController && root.hasVideoSource()
            onClicked: root.stopPreviewRequested()
        }
    }
}
