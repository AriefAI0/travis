import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Collects one video source configuration before it is applied to the recording viewmodel.

Dialog {
    id: root

    property var recordingViewModel
    property var previewController
    property var sourceDiscoveryViewModel
    property bool manualMode: false

    title: "Add Video Source"
    modal: true
    width: 480

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                color: "#d7e3ed"
                text: sourceDiscoveryViewModel && sourceDiscoveryViewModel.loading
                    ? "Refreshing sources..."
                    : "Video sources"
            }

            Button {
                text: "Refresh"
                enabled: sourceDiscoveryViewModel && !sourceDiscoveryViewModel.loading
                onClicked: sourceDiscoveryViewModel.refreshVideoSources()
            }

            CheckBox {
                text: "Manual"
                checked: root.manualMode
                onToggled: root.manualMode = checked
            }
        }

        ComboBox {
            id: sourceKindCombo

            Layout.fillWidth: true
            visible: root.manualMode
            model: [
                { value: "device-capture", label: "Device Capture" },
                { value: "ndi", label: "NDI" }
            ]
            textRole: "label"
            valueRole: "value"
        }

        ListView {
            id: discoveredVideoList

            Layout.fillWidth: true
            Layout.preferredHeight: 220
            clip: true
            visible: !root.manualMode
            model: sourceDiscoveryViewModel ? sourceDiscoveryViewModel.videoSources : []

            delegate: RadioDelegate {
                width: ListView.view.width
                text: modelData.name
                checked: discoveredVideoList.currentIndex === index
                onClicked: discoveredVideoList.currentIndex = index
            }
        }

        Label {
            Layout.fillWidth: true
            visible: !root.manualMode && discoveredVideoList.count === 0
            color: "#d9b66d"
            text: "No discovered video sources. Use Manual for custom input."
            wrapMode: Text.Wrap
        }

        TextField {
            id: sourceNameField

            Layout.fillWidth: true
            visible: root.manualMode
            placeholderText: "Source name"
        }

        TextField {
            id: sourceLabelField

            Layout.fillWidth: true
            placeholderText: "Display label"
        }

        TextField {
            id: devicePathField

            Layout.fillWidth: true
            visible: root.manualMode && sourceKindCombo.currentValue === "device-capture"
            placeholderText: "Device path"
        }

        TextField {
            id: sourceElementField

            Layout.fillWidth: true
            visible: root.manualMode && sourceKindCombo.currentValue === "device-capture"
            placeholderText: "Source element (optional)"
        }

        TextField {
            id: urlAddressField

            Layout.fillWidth: true
            visible: root.manualMode && sourceKindCombo.currentValue === "ndi"
            placeholderText: "NDI URL address (optional)"
        }
    }

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok

        onAccepted: {
            const selectedSource = !root.manualMode && discoveredVideoList.currentIndex >= 0
                ? discoveredVideoList.model[discoveredVideoList.currentIndex]
                : null

            const sourceKind = selectedSource ? selectedSource.kind : sourceKindCombo.currentValue
            const sourceName = selectedSource ? selectedSource.name : sourceNameField.text
            const devicePath = selectedSource ? selectedSource.devicePath : devicePathField.text
            const sourceElement = selectedSource ? selectedSource.sourceElement : sourceElementField.text
            const urlAddress = selectedSource ? selectedSource.urlAddress : urlAddressField.text

            recordingViewModel.sourceKind = sourceKind
            recordingViewModel.sourceName = sourceName
            recordingViewModel.sourceLabel = sourceLabelField.text
            recordingViewModel.devicePath = devicePath
            recordingViewModel.sourceElement = sourceElement
            recordingViewModel.urlAddress = urlAddress

            if (previewController) {
                if (sourceKind === "device-capture") {
                    previewController.startDeviceCapturePreview(
                        sourceName,
                        devicePath,
                        sourceElement
                    )
                } else {
                    previewController.startNdiPreview(
                        sourceName,
                        urlAddress
                    )
                }
            }

            root.close()
        }

        onRejected: root.close()
    }
}
