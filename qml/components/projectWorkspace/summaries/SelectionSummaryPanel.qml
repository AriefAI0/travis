import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shows metrics for the active project workspace structure selection.

ColumnLayout {
    id: root

    property string selectedType: "project"
    property var selectedAsset: null
    property var selectedComponent: null
    property var selectedItem: null

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
