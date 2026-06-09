import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../dialogs"

// Hosts the workspace A/V dock with inline audio slots and popout dialogs for video and advanced audio settings.

Rectangle {
    id: root

    property var recordingViewModel
    property var previewController

    radius: 10
    color: "#18212a"
    border.color: "#30404d"
    border.width: 1

    property int activeTabIndex: 0

    SourceSelectionDialog {
        id: sourceSelectionDialog
        recordingViewModel: root.recordingViewModel
        previewController: root.previewController
    }

    AdvancedAudioPropertiesDialog {
        id: advancedAudioPropertiesDialog
        audioSlots: recordingViewModel ? recordingViewModel.audioSlots : []
        onApply: function(nextSlots) {
            recordingViewModel.applyAdvancedAudioSlots(nextSlots)
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
                        model: recordingViewModel ? recordingViewModel.audioSlots : []

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
                                        recordingViewModel.updateAudioSlotBasic(
                                            index,
                                            text,
                                            modelData.devicePath,
                                            modelData.sourceElement
                                        )
                                    }
                                }

                                TextField {
                                    Layout.fillWidth: true
                                    placeholderText: "Device path"
                                    text: modelData.devicePath
                                    onTextChanged: {
                                        recordingViewModel.updateAudioSlotBasic(
                                            index,
                                            modelData.deviceName,
                                            text,
                                            modelData.sourceElement
                                        )
                                    }
                                }

                                TextField {
                                    Layout.fillWidth: true
                                    placeholderText: "Source element"
                                    text: modelData.sourceElement
                                    onTextChanged: {
                                        recordingViewModel.updateAudioSlotBasic(
                                            index,
                                            modelData.deviceName,
                                            modelData.devicePath,
                                            text
                                        )
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
