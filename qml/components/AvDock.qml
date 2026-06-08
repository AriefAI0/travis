import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../dialogs"

// Hosts the workspace A/V dock with inline audio slots and popout dialogs for video and advanced audio settings.

Rectangle {
    id: root

    property var recordingViewModel
    property var previewController
    property var audioSlots: [
        {
            slotId: "audio-input-1",
            displayName: "Audio Input 1",
            deviceName: "",
            devicePath: "",
            sourceElement: "wasapi2src",
            volume: 100,
            mono: false,
            balance: 0,
            syncOffsetMs: 0,
            monitoringMode: "off"
        },
        {
            slotId: "audio-input-2",
            displayName: "Audio Input 2",
            deviceName: "",
            devicePath: "",
            sourceElement: "wasapi2src",
            volume: 100,
            mono: false,
            balance: 0,
            syncOffsetMs: 0,
            monitoringMode: "off"
        }
    ]

    radius: 10
    color: "#18212a"
    border.color: "#30404d"
    border.width: 1

    property int activeTabIndex: 0

    function updateAudioConfiguration() {
        if (!recordingViewModel) {
            return
        }

        const configuredAudioInputs = []
        for (let index = 0; index < audioSlots.length; ++index) {
            const slot = audioSlots[index]
            if (slot.deviceName.length === 0 && slot.devicePath.length === 0) {
                continue
            }

            configuredAudioInputs.push({
                deviceName: slot.deviceName,
                devicePath: slot.devicePath,
                sourceElement: slot.sourceElement
            })
        }

        recordingViewModel.configureAudioInputs(configuredAudioInputs)
    }

    SourceSelectionDialog {
        id: sourceSelectionDialog
        recordingViewModel: root.recordingViewModel
        previewController: root.previewController
    }

    AdvancedAudioPropertiesDialog {
        id: advancedAudioPropertiesDialog
        audioSlots: root.audioSlots
        onApply: function(nextSlots) {
            root.audioSlots = nextSlots
            root.updateAudioConfiguration()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "A/V Dock"
                font.pixelSize: 18
                color: "#f4f7fa"
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Video"
                highlighted: root.activeTabIndex === 0
                onClicked: root.activeTabIndex = 0
            }

            Button {
                text: "Audio"
                highlighted: root.activeTabIndex === 1
                onClicked: root.activeTabIndex = 1
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.activeTabIndex

            Rectangle {
                color: "transparent"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: "#10171d"
                        border.color: "#2f3a44"
                        border.width: 1
                        implicitHeight: 140

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8

                            Label {
                                color: "#eef3f7"
                                text: recordingViewModel && recordingViewModel.sourceName.length > 0
                                    ? recordingViewModel.sourceName
                                    : "No video source selected"
                            }

                            Label {
                                color: "#9fb0be"
                                text: recordingViewModel && recordingViewModel.sourceKind.length > 0
                                    ? "Kind: " + recordingViewModel.sourceKind
                                    : "Open the source dialog to configure one"
                                wrapMode: Text.Wrap
                            }

                            Label {
                                visible: recordingViewModel && recordingViewModel.sourceLabel.length > 0
                                color: "#9fb0be"
                                text: recordingViewModel ? "Label: " + recordingViewModel.sourceLabel : ""
                                wrapMode: Text.Wrap
                            }
                        }
                    }

                    RowLayout {
                        Button {
                            text: "Add Source"
                            onClicked: sourceSelectionDialog.open()
                        }

                        Button {
                            text: "Stop Preview"
                            onClicked: previewController.stopPreview()
                        }

                        Button {
                            text: "Popout"
                            enabled: false
                        }
                    }
                }
            }

            Rectangle {
                color: "transparent"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    Repeater {
                        model: root.audioSlots

                        delegate: Rectangle {
                            Layout.fillWidth: true
                            color: "#10171d"
                            radius: 8
                            border.color: "#2f3a44"
                            border.width: 1
                            implicitHeight: slotContent.implicitHeight + 18

                            ColumnLayout {
                                id: slotContent

                                anchors.fill: parent
                                anchors.margins: 9
                                spacing: 8

                                Label {
                                    color: "#eef3f7"
                                    text: modelData.displayName
                                }

                                TextField {
                                    Layout.fillWidth: true
                                    placeholderText: "Device name"
                                    text: modelData.deviceName
                                    onTextChanged: {
                                        const nextSlots = root.audioSlots.slice()
                                        nextSlots[index] = {
                                            slotId: nextSlots[index].slotId,
                                            displayName: nextSlots[index].displayName,
                                            deviceName: text,
                                            devicePath: nextSlots[index].devicePath,
                                            sourceElement: nextSlots[index].sourceElement,
                                            volume: nextSlots[index].volume,
                                            mono: nextSlots[index].mono,
                                            balance: nextSlots[index].balance,
                                            syncOffsetMs: nextSlots[index].syncOffsetMs,
                                            monitoringMode: nextSlots[index].monitoringMode
                                        }
                                        root.audioSlots = nextSlots
                                        root.updateAudioConfiguration()
                                    }
                                }

                                TextField {
                                    Layout.fillWidth: true
                                    placeholderText: "Device path"
                                    text: modelData.devicePath
                                    onTextChanged: {
                                        const nextSlots = root.audioSlots.slice()
                                        nextSlots[index] = {
                                            slotId: nextSlots[index].slotId,
                                            displayName: nextSlots[index].displayName,
                                            deviceName: nextSlots[index].deviceName,
                                            devicePath: text,
                                            sourceElement: nextSlots[index].sourceElement,
                                            volume: nextSlots[index].volume,
                                            mono: nextSlots[index].mono,
                                            balance: nextSlots[index].balance,
                                            syncOffsetMs: nextSlots[index].syncOffsetMs,
                                            monitoringMode: nextSlots[index].monitoringMode
                                        }
                                        root.audioSlots = nextSlots
                                        root.updateAudioConfiguration()
                                    }
                                }

                                TextField {
                                    Layout.fillWidth: true
                                    placeholderText: "Source element"
                                    text: modelData.sourceElement
                                    onTextChanged: {
                                        const nextSlots = root.audioSlots.slice()
                                        nextSlots[index] = {
                                            slotId: nextSlots[index].slotId,
                                            displayName: nextSlots[index].displayName,
                                            deviceName: nextSlots[index].deviceName,
                                            devicePath: nextSlots[index].devicePath,
                                            sourceElement: text,
                                            volume: nextSlots[index].volume,
                                            mono: nextSlots[index].mono,
                                            balance: nextSlots[index].balance,
                                            syncOffsetMs: nextSlots[index].syncOffsetMs,
                                            monitoringMode: nextSlots[index].monitoringMode
                                        }
                                        root.audioSlots = nextSlots
                                        root.updateAudioConfiguration()
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true

                                    Label {
                                        color: "#aebbc6"
                                        text: `Volume ${modelData.volume}%`
                                    }

                                    Slider {
                                        Layout.fillWidth: true
                                        from: 0
                                        to: 100
                                        value: modelData.volume
                                        enabled: false
                                    }
                                }
                            }
                        }
                    }

                    Button {
                        text: "Advanced Audio Properties"
                        onClicked: advancedAudioPropertiesDialog.open()
                    }
                }
            }
        }
    }
}
