import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../../../dialogs"

// Hosts the inspection A/V dock with video source dialogs and inline audio slots.

Rectangle {
    id: root

    property var recordingViewModel
    property var previewController
    property var sourceDiscoveryViewModel
    property var audioMeterViewModel

    radius: 0
    color: "#141820"

    property int activeTabIndex: 0

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

    function syncAudioMeters() {
        if (!audioMeterViewModel || !recordingViewModel) {
            return
        }

        audioMeterViewModel.syncAudioSlots(recordingViewModel.audioSlots, root.activeTabIndex === 1)
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
        anchors.margins: 8
        spacing: 6

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "A/V Dock"
                font.pixelSize: 12
                font.bold: true
                color: "#f4f7fa"
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                implicitHeight: 24
                text: "Video"
                highlighted: root.activeTabIndex === 0
                onClicked: root.activeTabIndex = 0
            }

            Button {
                implicitHeight: 24
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
                        radius: 0
                        color: "transparent"
                        implicitHeight: 72

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 3

                            Label {
                                color: "#eef3f7"
                                font.pixelSize: 12
                                text: root.hasVideoSource()
                                    ? recordingViewModel.sourceName
                                    : "No video source selected"
                            }

                            Label {
                                color: "#9fb0be"
                                text: root.hasVideoSource()
                                    ? root.videoSourceKindLabel() + " - " + root.videoSourceDetail()
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
                            implicitHeight: 24
                            text: root.hasVideoSource() ? "Change Source" : "Add Source"
                            enabled: recordingViewModel && sourceDiscoveryViewModel
                            onClicked: sourceSelectionDialog.openForCurrentSource()
                        }

                        Button {
                            implicitHeight: 24
                            text: "Stop Preview"
                            enabled: previewController && root.hasVideoSource()
                            onClicked: previewController.stopPreview()
                        }

                        Button {
                            implicitHeight: 24
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
                            radius: 0
                            border.color: "#2f3a44"
                            border.width: 1
                            implicitHeight: slotContent.implicitHeight + 18

                            ColumnLayout {
                                id: slotContent

                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 6

                                Label {
                                    color: "#eef3f7"
                                    text: modelData.displayName
                                }

                                ComboBox {
                                    id: audioSourceCombo

                                    Layout.fillWidth: true
                                    model: sourceDiscoveryViewModel ? sourceDiscoveryViewModel.audioSources : []
                                    textRole: "name"
                                    valueRole: "id"
                                    displayText: currentIndex >= 0 ? currentText : "Select audio device"

                                    function syncCurrentSource() {
                                        for (let sourceIndex = 0; sourceIndex < count; ++sourceIndex) {
                                            if (model[sourceIndex].devicePath === modelData.devicePath &&
                                                    model[sourceIndex].name === modelData.deviceName) {
                                                currentIndex = sourceIndex
                                                return
                                            }
                                        }

                                        currentIndex = -1
                                    }

                                    Component.onCompleted: syncCurrentSource()
                                    onModelChanged: syncCurrentSource()

                                    onActivated: {
                                        const source = model[currentIndex]
                                        recordingViewModel.updateAudioSlotBasic(
                                            index,
                                            source.name,
                                            source.devicePath,
                                            source.sourceElement
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
                                            value: audioMeterViewModel
                                                ? (audioMeterViewModel.revision, audioMeterViewModel.leftLevel(modelData.slotId))
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
                                            value: audioMeterViewModel
                                                ? (audioMeterViewModel.revision, audioMeterViewModel.rightLevel(modelData.slotId))
                                                : 0
                                        }
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
                        enabled: recordingViewModel
                        onClicked: advancedAudioPropertiesDialog.open()
                    }

                    Button {
                        text: "Refresh Audio Devices"
                        enabled: sourceDiscoveryViewModel && !sourceDiscoveryViewModel.loading
                        onClicked: sourceDiscoveryViewModel.refreshAudioSources()
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: sourceDiscoveryViewModel && sourceDiscoveryViewModel.lastError.length > 0
                        color: "#e6b36a"
                        text: sourceDiscoveryViewModel ? sourceDiscoveryViewModel.lastError : ""
                        wrapMode: Text.Wrap
                    }
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
