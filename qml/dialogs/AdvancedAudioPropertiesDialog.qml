import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Edits advanced audio slot properties while leaving basic device selection in the dock.

Dialog {
    id: root

    property var audioSlots: []
    property var onApply: null

    title: "Advanced Audio Properties"
    modal: true
    width: 760
    height: 420

    ListModel {
        id: advancedAudioModel
    }

    function reloadModel() {
        advancedAudioModel.clear()

        for (let index = 0; index < root.audioSlots.length; ++index) {
            const slot = root.audioSlots[index]
            advancedAudioModel.append({
                slotId: slot.slotId,
                displayName: slot.displayName,
                deviceName: slot.deviceName,
                devicePath: slot.devicePath,
                sourceElement: slot.sourceElement,
                volume: slot.volume,
                mono: slot.mono,
                balance: slot.balance,
                syncOffsetMs: slot.syncOffsetMs,
                monitoringMode: slot.monitoringMode
            })
        }
    }

    onOpened: reloadModel()

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#121922"
            radius: 8
            border.color: "#31404d"
            border.width: 1

            ListView {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                clip: true
                model: advancedAudioModel

                delegate: Rectangle {
                    width: ListView.view.width
                    color: "#182029"
                    radius: 8
                    border.color: "#33424f"
                    border.width: 1
                    implicitHeight: content.implicitHeight + 18

                    ColumnLayout {
                        id: content

                        anchors.fill: parent
                        anchors.margins: 9
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            color: "#eef3f7"
                            text: model.displayName
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "Volume"
                                color: "#b9c7d3"
                            }

                            Slider {
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: model.volume
                                onMoved: advancedAudioModel.setProperty(index, "volume", Math.round(value))
                            }

                            Label {
                                text: `${model.volume}%`
                                color: "#d7e3ed"
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            CheckBox {
                                text: "Mono"
                                checked: model.mono
                                onToggled: advancedAudioModel.setProperty(index, "mono", checked)
                            }

                            Label {
                                text: "Balance"
                                color: "#b9c7d3"
                            }

                            Slider {
                                Layout.fillWidth: true
                                from: -100
                                to: 100
                                value: model.balance
                                onMoved: advancedAudioModel.setProperty(index, "balance", Math.round(value))
                            }

                            Label {
                                text: `${model.balance}`
                                color: "#d7e3ed"
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "Sync Offset"
                                color: "#b9c7d3"
                            }

                            TextField {
                                Layout.preferredWidth: 100
                                text: `${model.syncOffsetMs}`
                                onTextChanged: advancedAudioModel.setProperty(
                                    index,
                                    "syncOffsetMs",
                                    text.length > 0 ? Number(text) : 0
                                )
                            }

                            ComboBox {
                                Layout.fillWidth: true
                                model: [
                                    { value: "off", label: "Monitor Off" },
                                    { value: "monitor-only", label: "Monitor Only" },
                                    { value: "monitor-and-output", label: "Monitor and Output" }
                                ]
                                textRole: "label"
                                valueRole: "value"
                                Component.onCompleted: currentIndex = Math.max(
                                    0,
                                    indexOfValue(model.monitoringMode)
                                )
                                onActivated: advancedAudioModel.setProperty(
                                    index,
                                    "monitoringMode",
                                    currentValue
                                )
                            }
                        }
                    }
                }
            }
        }
    }

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok

        onAccepted: {
            const nextSlots = []
            for (let index = 0; index < advancedAudioModel.count; ++index) {
                nextSlots.push(advancedAudioModel.get(index))
            }

            if (root.onApply) {
                root.onApply(nextSlots)
            }

            root.close()
        }

        onRejected: root.close()
    }
}
