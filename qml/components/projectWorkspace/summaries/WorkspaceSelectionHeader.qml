import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Compact top context header for the active project workspace selection.

Rectangle {
    id: root

    property var project: ({})
    property string selectedType: "project"
    property var selectedAsset: null
    property var selectedComponent: null
    property var selectedItem: null
    property int assetCount: 0
    property int componentCount: 0
    property int itemCount: 0

    function titleText() {
        if (root.selectedType === "asset" && root.selectedAsset !== null) {
            return root.selectedAsset.name
        }
        if (root.selectedType === "component" && root.selectedComponent !== null) {
            return root.selectedComponent.name
        }
        if (root.selectedType === "item" && root.selectedItem !== null) {
            return root.selectedItem.itemLabel
        }
        return root.project.title || "Project overview"
    }

    function contextText() {
        if (root.selectedType === "asset") {
            return "Project / Asset"
        }
        if (root.selectedType === "component") {
            return (root.selectedAsset ? root.selectedAsset.name : "Asset") + " / Component"
        }
        if (root.selectedType === "item") {
            return (root.selectedComponent ? root.selectedComponent.name : "Component") + " / Item"
        }
        return "Project workspace"
    }

    Layout.fillWidth: true
    implicitHeight: headerColumn.implicitHeight + 22
    radius: 6
    color: "#181c24"
    border.color: "#303642"
    border.width: 1

    ColumnLayout {
        id: headerColumn

        anchors.fill: parent
        anchors.margins: 11
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Label {
                    color: "#78818f"
                    font.bold: true
                    font.pixelSize: 10
                    text: root.contextText().toUpperCase()
                }

                Label {
                    Layout.fillWidth: true
                    color: "#e6e8eb"
                    font.bold: true
                    font.pixelSize: 18
                    text: root.titleText()
                    elide: Text.ElideRight
                }
            }

            Label {
                color: "#9aa3af"
                font.pixelSize: 11
                text: root.selectedType.toUpperCase()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: "#303642"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            HeaderMetric {
                label: "Document"
                value: root.project.documentId || "-"
            }

            HeaderMetric {
                label: "Assets"
                value: String(root.assetCount)
            }

            HeaderMetric {
                label: "Components"
                value: String(root.componentCount)
            }

            HeaderMetric {
                label: "Items"
                value: String(root.itemCount)
            }
        }
    }

    component HeaderMetric: ColumnLayout {
        property string label: ""
        property string value: ""

        Layout.fillWidth: true
        spacing: 2

        Label {
            color: "#78818f"
            font.pixelSize: 10
            text: label
        }

        Label {
            Layout.fillWidth: true
            color: "#d8dde5"
            font.bold: true
            font.pixelSize: 12
            text: value
            elide: Text.ElideRight
        }
    }
}
