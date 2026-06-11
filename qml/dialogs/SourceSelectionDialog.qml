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

    parent: Overlay.overlay
    title: "Video Source"
    modal: true
    width: 480
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    function openForCurrentSource() {
        sourceLabelField.text = recordingViewModel ? recordingViewModel.sourceLabel : ""
        root.manualMode = false

        root.open()

        if (sourceDiscoveryViewModel) {
            sourceDiscoveryViewModel.refreshVideoSources()
        }
    }

    function selectCurrentSource() {
        if (!recordingViewModel || root.manualMode) {
            return
        }

        for (let sourceIndex = 0; sourceIndex < discoveredVideoList.count; ++sourceIndex) {
            const source = discoveredVideoList.model[sourceIndex]
            if (source.kind === recordingViewModel.sourceKind &&
                    source.name === recordingViewModel.sourceName &&
                    source.devicePath === recordingViewModel.devicePath &&
                    source.urlAddress === recordingViewModel.urlAddress) {
                discoveredVideoList.currentIndex = sourceIndex
                return
            }
        }
    }

    function selectedDiscoveredSource() {
        if (root.manualMode || discoveredVideoList.currentIndex < 0) {
            return null
        }

        return discoveredVideoList.model[discoveredVideoList.currentIndex]
    }

    function sourceKindLabel(kind) {
        if (kind === "ndi") {
            return "NDI"
        }

        if (kind === "device-capture") {
            return "Device Capture"
        }

        return kind && kind.length > 0 ? kind : "Unknown"
    }

    function sourceDetail(source) {
        if (!source) {
            return ""
        }

        if (source.kind === "ndi") {
            return source.urlAddress && source.urlAddress.length > 0 ? source.urlAddress : "NDI network source"
        }

        if (source.devicePath && source.devicePath.length > 0) {
            return source.devicePath
        }

        return source.sourceElement && source.sourceElement.length > 0 ? source.sourceElement : "Local capture device"
    }

    function canApply() {
        if (!recordingViewModel) {
            return false
        }

        if (!root.manualMode) {
            return root.selectedDiscoveredSource() !== null
        }

        return sourceNameField.text.trim().length > 0
    }

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
                text: "Custom"
                checked: root.manualMode
                onToggled: root.manualMode = checked
            }
        }

        Label {
            Layout.fillWidth: true
            visible: sourceDiscoveryViewModel && sourceDiscoveryViewModel.lastError.length > 0
            color: "#e6b36a"
            text: sourceDiscoveryViewModel ? sourceDiscoveryViewModel.lastError : ""
            wrapMode: Text.Wrap
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

            delegate: ItemDelegate {
                width: ListView.view.width
                checked: discoveredVideoList.currentIndex === index
                highlighted: checked
                onClicked: discoveredVideoList.currentIndex = index

                contentItem: ColumnLayout {
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        color: discoveredVideoList.currentIndex === index ? "#ffffff" : "#eef3f7"
                        elide: Text.ElideRight
                        text: modelData.name
                    }

                    Label {
                        Layout.fillWidth: true
                        color: discoveredVideoList.currentIndex === index ? "#cfe4f3" : "#9fb0be"
                        elide: Text.ElideMiddle
                        font.pixelSize: 12
                        text: root.sourceKindLabel(modelData.kind) + " - " + root.sourceDetail(modelData)
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: !root.manualMode && discoveredVideoList.count === 0
            color: "#d9b66d"
            text: sourceDiscoveryViewModel && sourceDiscoveryViewModel.loading
                ? "Looking for local and NDI video sources..."
                : "No discovered video sources. Use Custom for manual input."
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
        id: dialogButtons

        standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok

        Component.onCompleted: {
            standardButton(DialogButtonBox.Ok).text = "Add Source"
            standardButton(DialogButtonBox.Ok).enabled = root.canApply()
        }

        Connections {
            target: root

            function onManualModeChanged() {
                dialogButtons.standardButton(DialogButtonBox.Ok).enabled = root.canApply()
            }
        }

        Connections {
            target: discoveredVideoList

            function onCurrentIndexChanged() {
                dialogButtons.standardButton(DialogButtonBox.Ok).enabled = root.canApply()
            }
        }

        Connections {
            target: sourceNameField

            function onTextChanged() {
                dialogButtons.standardButton(DialogButtonBox.Ok).enabled = root.canApply()
            }
        }

        Connections {
            target: root.sourceDiscoveryViewModel || null

            function onVideoSourcesChanged() {
                root.selectCurrentSource()
                dialogButtons.standardButton(DialogButtonBox.Ok).enabled = root.canApply()
            }
        }

        onAccepted: {
            const selectedSource = root.selectedDiscoveredSource()

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
