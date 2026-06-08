import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Collects a small audio-input slot list and pushes it into the recording viewmodel.

Dialog {
    id: root

    property var recordingViewModel

    title: "Audio Inputs"
    modal: true
    width: 460

    ListModel {
        id: audioInputModel
    }

    function appendEmptyRow() {
        audioInputModel.append({
            deviceName: "",
            devicePath: "",
            sourceElement: "wasapi2src"
        })
    }

    Component.onCompleted: {
        if (audioInputModel.count === 0) {
            appendEmptyRow()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        Repeater {
            model: audioInputModel

            delegate: Rectangle {
                Layout.fillWidth: true
                color: "#131b22"
                radius: 8
                border.color: "#34424f"
                border.width: 1
                implicitHeight: rowLayout.implicitHeight + 18

                ColumnLayout {
                    id: rowLayout

                    anchors.fill: parent
                    anchors.margins: 9
                    spacing: 8

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Device name"
                        text: model.deviceName
                        onTextChanged: audioInputModel.setProperty(index, "deviceName", text)
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Device path"
                        text: model.devicePath
                        onTextChanged: audioInputModel.setProperty(index, "devicePath", text)
                    }

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "Source element"
                        text: model.sourceElement
                        onTextChanged: audioInputModel.setProperty(index, "sourceElement", text)
                    }

                    Button {
                        text: "Remove"
                        enabled: audioInputModel.count > 1
                        onClicked: audioInputModel.remove(index)
                    }
                }
            }
        }

        Button {
            text: "Add Audio Input"
            onClicked: appendEmptyRow()
        }
    }

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok

        onAccepted: {
            const audioInputs = []
            for (let index = 0; index < audioInputModel.count; ++index) {
                audioInputs.push(audioInputModel.get(index))
            }

            recordingViewModel.configureAudioInputs(audioInputs)
            root.close()
        }

        onRejected: root.close()
    }
}
