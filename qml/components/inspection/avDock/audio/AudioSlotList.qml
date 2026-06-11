import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Lists audio input slots and exposes audio dock actions to the parent container.

ColumnLayout {
    id: root

    property var recordingViewModel
    property var sourceDiscoveryViewModel
    property var audioMeterViewModel

    signal advancedAudioRequested()
    signal refreshAudioRequested()

    spacing: 6

    ScrollView {
        id: audioSlotScroll

        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        ColumnLayout {
            width: audioSlotScroll.availableWidth
            spacing: 6

            Repeater {
                model: root.recordingViewModel ? root.recordingViewModel.audioSlots : []

                delegate: AudioSlotRow {
                    slotData: modelData
                    slotIndex: index
                    recordingViewModel: root.recordingViewModel
                    sourceDiscoveryViewModel: root.sourceDiscoveryViewModel
                    audioMeterViewModel: root.audioMeterViewModel
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 6

        Button {
            Layout.fillWidth: true
            implicitHeight: 24
            text: "Advanced"
            enabled: recordingViewModel
            onClicked: root.advancedAudioRequested()
        }

        Button {
            Layout.fillWidth: true
            implicitHeight: 24
            text: "Refresh"
            enabled: sourceDiscoveryViewModel && !sourceDiscoveryViewModel.loading
            onClicked: root.refreshAudioRequested()
        }
    }

    Label {
        Layout.fillWidth: true
        visible: sourceDiscoveryViewModel && sourceDiscoveryViewModel.lastError.length > 0
        color: "#e6b36a"
        font.pixelSize: 11
        text: sourceDiscoveryViewModel ? sourceDiscoveryViewModel.lastError : ""
        wrapMode: Text.Wrap
    }
}
