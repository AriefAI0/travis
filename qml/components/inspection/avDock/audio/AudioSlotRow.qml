import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Edits one audio input slot and displays its current meter levels.

Rectangle {
    id: root

    property var slotData
    property int slotIndex: -1
    property var recordingViewModel
    property var sourceDiscoveryViewModel
    property var audioMeterViewModel

    Layout.fillWidth: true
    color: "transparent"
    implicitHeight: slotContent.implicitHeight + 12

    ColumnLayout {
        id: slotContent

        anchors.fill: parent
        anchors.margins: 6
        spacing: 5

        Label {
            color: "#eef3f7"
            text: root.slotData ? root.slotData.displayName : "Audio slot"
        }

        ComboBox {
            id: audioSourceCombo

            Layout.fillWidth: true
            model: sourceDiscoveryViewModel ? sourceDiscoveryViewModel.audioSources : []
            textRole: "name"
            valueRole: "id"
            displayText: currentIndex >= 0 ? currentText : "Select audio device"

            function syncCurrentSource() {
                if (!root.slotData) {
                    currentIndex = -1
                    return
                }

                for (let sourceIndex = 0; sourceIndex < count; ++sourceIndex) {
                    if (model[sourceIndex].devicePath === root.slotData.devicePath &&
                            model[sourceIndex].name === root.slotData.deviceName) {
                        currentIndex = sourceIndex
                        return
                    }
                }

                currentIndex = -1
            }

            Component.onCompleted: syncCurrentSource()
            onModelChanged: syncCurrentSource()

            onActivated: {
                if (!recordingViewModel || currentIndex < 0) {
                    return
                }

                const source = model[currentIndex]
                recordingViewModel.updateAudioSlotBasic(
                    root.slotIndex,
                    source.name,
                    source.devicePath,
                    source.sourceElement
                )
            }
        }

        TextField {
            Layout.fillWidth: true
            placeholderText: "Device path"
            text: root.slotData ? root.slotData.devicePath : ""
            onTextChanged: {
                if (recordingViewModel && root.slotData) {
                    recordingViewModel.updateAudioSlotBasic(
                        root.slotIndex,
                        root.slotData.deviceName,
                        text,
                        root.slotData.sourceElement
                    )
                }
            }
        }

        TextField {
            Layout.fillWidth: true
            placeholderText: "Source element"
            text: root.slotData ? root.slotData.sourceElement : ""
            onTextChanged: {
                if (recordingViewModel && root.slotData) {
                    recordingViewModel.updateAudioSlotBasic(
                        root.slotIndex,
                        root.slotData.deviceName,
                        root.slotData.devicePath,
                        text
                    )
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            RowLayout {
                Layout.fillWidth: true

                Label {
                    color: "#aebbc6"
                    text: "L"
                }

                ProgressBar {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: audioMeterViewModel && root.slotData
                        ? (audioMeterViewModel.revision, audioMeterViewModel.leftLevel(root.slotData.slotId))
                        : 0
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    color: "#aebbc6"
                    text: "R"
                }

                ProgressBar {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: audioMeterViewModel && root.slotData
                        ? (audioMeterViewModel.revision, audioMeterViewModel.rightLevel(root.slotData.slotId))
                        : 0
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                color: "#aebbc6"
                text: root.slotData ? `Volume ${root.slotData.volume}%` : "Volume"
            }

            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                value: root.slotData ? root.slotData.volume : 0
                enabled: false
            }
        }
    }
}
