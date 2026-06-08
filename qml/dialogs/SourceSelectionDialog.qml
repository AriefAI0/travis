import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Collects one video source configuration before it is applied to the recording viewmodel.

Dialog {
    id: root

    property var recordingViewModel
    property var previewController

    title: "Add Video Source"
    modal: true
    width: 420

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        ComboBox {
            id: sourceKindCombo

            Layout.fillWidth: true
            model: [
                { value: "device-capture", label: "Device Capture" },
                { value: "ndi", label: "NDI" }
            ]
            textRole: "label"
            valueRole: "value"
        }

        TextField {
            id: sourceNameField

            Layout.fillWidth: true
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
            visible: sourceKindCombo.currentValue === "device-capture"
            placeholderText: "Device path"
        }

        TextField {
            id: sourceElementField

            Layout.fillWidth: true
            visible: sourceKindCombo.currentValue === "device-capture"
            placeholderText: "Source element (optional)"
        }

        TextField {
            id: urlAddressField

            Layout.fillWidth: true
            visible: sourceKindCombo.currentValue === "ndi"
            placeholderText: "NDI URL address (optional)"
        }
    }

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok

        onAccepted: {
            recordingViewModel.sourceKind = sourceKindCombo.currentValue
            recordingViewModel.sourceName = sourceNameField.text
            recordingViewModel.sourceLabel = sourceLabelField.text
            recordingViewModel.devicePath = devicePathField.text
            recordingViewModel.sourceElement = sourceElementField.text
            recordingViewModel.urlAddress = urlAddressField.text

            if (previewController) {
                if (sourceKindCombo.currentValue === "device-capture") {
                    previewController.startDeviceCapturePreview(
                        sourceNameField.text,
                        devicePathField.text,
                        sourceElementField.text
                    )
                } else {
                    previewController.startNdiPreview(
                        sourceNameField.text,
                        urlAddressField.text
                    )
                }
            }

            root.close()
        }

        onRejected: root.close()
    }
}
