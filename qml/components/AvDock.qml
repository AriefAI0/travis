import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../dialogs"

// Hosts the workspace A/V dock and launches the source/audio configuration dialogs.

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

    AudioInputDialog {
        id: audioInputDialog
        recordingViewModel: root.recordingViewModel
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

                    Label {
                        Layout.fillWidth: true
                        color: "#9fb0be"
                        text: "Configure audio input slots through the dialog."
                        wrapMode: Text.Wrap
                    }

                    Button {
                        text: "Open Audio Dialog"
                        onClicked: audioInputDialog.open()
                    }
                }
            }
        }
    }
}
