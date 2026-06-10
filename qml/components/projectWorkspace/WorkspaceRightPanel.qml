import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Context summary panel for the active project workspace selection.

WorkspacePanel {
    id: root

    property string selectedType: "project"
    property var selectedAsset: null
    property var selectedComponent: null
    property var selectedItem: null
    property var recordings: []
    property int assetCount: 0
    property int componentCount: 0
    property int itemCount: 0

    content: [
        Label {
            color: "#e6e8eb"
            font.bold: true
            font.pixelSize: 14
            text: root.selectedType === "project" ? "Recordings" : "Selection summary"
        },

        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selectedType === "project"
            spacing: 8

            Label {
                Layout.fillWidth: true
                visible: root.recordings.length === 0
                color: "#78818f"
                font.pixelSize: 12
                text: "No recordings for this project yet."
                wrapMode: Text.Wrap
            }

            Repeater {
                model: root.recordings

                delegate: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    Label {
                        Layout.fillWidth: true
                        color: "#c9ced6"
                        font.pixelSize: 12
                        text: modelData.fileUrl
                        elide: Text.ElideMiddle
                    }

                    Label {
                        Layout.fillWidth: true
                        color: modelData.status === "finalized" ? "#78818f" : "#d9a441"
                        font.pixelSize: 11
                        text: modelData.status || "unknown"
                        elide: Text.ElideRight
                    }
                }
            }
        },

        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selectedType !== "project"
            spacing: 8

            SummaryMetric {
                label: "Asset"
                value: root.selectedAsset ? root.selectedAsset.name : "-"
            }

            SummaryMetric {
                visible: root.selectedType === "asset"
                label: "Components"
                value: root.selectedAsset ? String(root.selectedAsset.components.length) : "0"
            }

            SummaryMetric {
                visible: root.selectedType === "component" || root.selectedType === "item"
                label: "Component"
                value: root.selectedComponent ? root.selectedComponent.name : "-"
            }

            SummaryMetric {
                visible: root.selectedType === "component"
                label: "Items"
                value: root.selectedComponent ? String(root.selectedComponent.items.length) : "0"
            }

            SummaryMetric {
                visible: root.selectedType === "item"
                label: "Item"
                value: root.selectedItem ? root.selectedItem.itemLabel : "-"
            }
        }
    ]

    component SummaryMetric: Rectangle {
        property string label: ""
        property string value: ""

        Layout.fillWidth: true
        implicitHeight: 54
        radius: 6
        color: "#20242d"
        border.color: "#303642"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 3

            Label {
                color: "#78818f"
                font.pixelSize: 10
                text: label
            }

            Label {
                Layout.fillWidth: true
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 13
                text: value
                elide: Text.ElideRight
            }
        }
    }
}
