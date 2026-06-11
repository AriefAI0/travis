import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shows selected inspection item details and the current inspection type selector.

Rectangle {
    id: root

    property var inspectionContextViewModel
    property var recordingViewModel
    property alias inspectionTypeText: inspectionTypeField.text
    signal startClipRequested(string inspectionTypeName)
    signal stopClipRequested(int clipId)

    radius: 8
    color: "#1b1f27"
    border.color: "#303642"
    border.width: 1
    clip: true

    ScrollView {
        id: detailsScroll

        anchors.fill: parent
        anchors.margins: 10
        clip: true

        ColumnLayout {
            width: detailsScroll.availableWidth
            spacing: 8

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.bold: true
                font.pixelSize: 10
                text: "INSPECTION DETAILS"
            }

            Label {
                Layout.fillWidth: true
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 14
                text: inspectionContextViewModel && inspectionContextViewModel.selectedItemId > 0
                    ? inspectionContextViewModel.selectedItem.itemLabel
                    : "Select an inspection item"
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                color: "#a8b0bd"
                font.pixelSize: 12
                text: inspectionContextViewModel && inspectionContextViewModel.selectedItemId > 0
                    ? `${inspectionContextViewModel.selectedItem.assetName} / ${inspectionContextViewModel.selectedItem.componentName}`
                    : "Choose an item from the inspection tree to enable clipping."
                wrapMode: Text.Wrap
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#303642"
            }

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.bold: true
                font.pixelSize: 10
                text: "INSPECTION TYPE"
            }

            TextField {
                id: inspectionTypeField

                Layout.fillWidth: true
                text: "GVI"
                placeholderText: "Type"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 6
                rowSpacing: 6

                Repeater {
                    model: ["GVI", "CVI", "MGI", "FMD"]

                    Button {
                        Layout.fillWidth: true
                        implicitHeight: 24
                        text: modelData
                        enabled: root.inspectionContextViewModel && root.inspectionContextViewModel.selectedItemId > 0
                        onClicked: inspectionTypeField.text = modelData
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.pixelSize: 11
                text: "Select an inspection type, then confirm to start clipping while the main recording is running."
                wrapMode: Text.Wrap
            }

            Button {
                Layout.fillWidth: true
                implicitHeight: 26
                text: "Start Clip"
                enabled: root.recordingViewModel &&
                    root.recordingViewModel.recordingActive &&
                    !root.recordingViewModel.paused &&
                    root.recordingViewModel.activeClipId === 0 &&
                    root.inspectionContextViewModel &&
                    root.inspectionContextViewModel.selectedItemId > 0 &&
                    inspectionTypeField.text.trim().length > 0
                onClicked: root.startClipRequested(inspectionTypeField.text)
            }

            Button {
                Layout.fillWidth: true
                implicitHeight: 26
                text: "Done"
                enabled: root.recordingViewModel && root.recordingViewModel.activeClipId > 0
                onClicked: root.stopClipRequested(root.recordingViewModel.activeClipId)
            }

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.pixelSize: 11
                text: root.recordingViewModel && root.recordingViewModel.activeClipId > 0
                    ? `Active clip ${root.recordingViewModel.activeClipId}`
                    : "Clip controls enable after main recording starts and an item is selected."
                wrapMode: Text.Wrap
            }
        }
    }
}
