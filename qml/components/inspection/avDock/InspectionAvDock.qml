import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../../../dialogs"
import "audio"
import "video"

// Hosts inspection A/V dock tabs while dialogs and lifecycle stay owned by the container.

Rectangle {
    id: root

    property var recordingViewModel
    property var previewController
    property var sourceDiscoveryViewModel
    property var audioMeterViewModel
    property int activeTabIndex: 0

    radius: 8
    color: "#1b1f27"
    border.color: "#303642"
    border.width: 1
    clip: true

    function syncAudioMeters() {
        if (!audioMeterViewModel || !recordingViewModel) {
            return
        }

        audioMeterViewModel.syncAudioSlots(recordingViewModel.audioSlots, root.activeTabIndex === 1)
    }

    function removeVideoSource() {
        if (root.previewController) {
            root.previewController.stopPreview()
        }

        if (!root.recordingViewModel) {
            return
        }

        root.recordingViewModel.sourceKind = ""
        root.recordingViewModel.sourceName = ""
        root.recordingViewModel.sourceLabel = ""
        root.recordingViewModel.devicePath = ""
        root.recordingViewModel.sourceElement = ""
        root.recordingViewModel.urlAddress = ""
    }

    onActiveTabIndexChanged: syncAudioMeters()

    SourceSelectionDialog {
        id: sourceSelectionDialog
        recordingViewModel: root.recordingViewModel
        previewController: root.previewController
        sourceDiscoveryViewModel: root.sourceDiscoveryViewModel
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
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 26
            spacing: 6

            Label {
                color: "#f4f7fa"
                font.bold: true
                font.pixelSize: 12
                text: "A/V Dock"
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                implicitHeight: 24
                implicitWidth: 58
                text: "Video"
                highlighted: root.activeTabIndex === 0
                onClicked: root.activeTabIndex = 0
            }

            Button {
                implicitHeight: 24
                implicitWidth: 58
                text: "Audio"
                highlighted: root.activeTabIndex === 1
                onClicked: root.activeTabIndex = 1
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#20242d"
            radius: 6
            border.color: "#303642"
            border.width: 1
            clip: true

            StackLayout {
                anchors.fill: parent
                anchors.margins: 8
                currentIndex: root.activeTabIndex

                VideoSourcePanel {
                    recordingViewModel: root.recordingViewModel
                    previewController: root.previewController
                    sourceDiscoveryViewModel: root.sourceDiscoveryViewModel
                    onOpenSourceRequested: sourceSelectionDialog.openForCurrentSource()
                    onRemoveSourceRequested: root.removeVideoSource()
                    onStopPreviewRequested: root.previewController.stopPreview()
                }

                AudioSlotList {
                    recordingViewModel: root.recordingViewModel
                    sourceDiscoveryViewModel: root.sourceDiscoveryViewModel
                    audioMeterViewModel: root.audioMeterViewModel
                    onAdvancedAudioRequested: advancedAudioPropertiesDialog.open()
                    onRefreshAudioRequested: root.sourceDiscoveryViewModel.refreshAudioSources()
                }
            }
        }
    }

    Connections {
        target: root.recordingViewModel || null

        function onAudioSlotsChanged() {
            root.syncAudioMeters()
        }
    }

    Component.onDestruction: {
        if (audioMeterViewModel) {
            audioMeterViewModel.stopAll()
        }
    }
}
