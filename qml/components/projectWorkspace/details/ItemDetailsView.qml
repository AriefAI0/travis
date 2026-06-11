import QtQuick
import QtQuick.Layouts

import "../components"

// Read-only details for the selected inspection item.

DetailCard {
    id: root

    property var selectedAsset: null
    property var selectedComponent: null
    property var selectedItem: null

    Layout.fillWidth: true
    title: "Item details"

    GridLayout {
        Layout.fillWidth: true
        columns: 3
        rowSpacing: 8
        columnSpacing: 8

        StatBox { label: "Asset"; value: root.selectedAsset ? root.selectedAsset.name : "-" }
        StatBox { label: "Component"; value: root.selectedComponent ? root.selectedComponent.name : "-" }
        StatBox { label: "Position"; value: root.selectedItem ? (root.selectedItem.position || "-") : "-" }
        StatBox {
            label: "Status"
            value: root.selectedItem !== null && root.selectedItem.status !== undefined
                ? String(root.selectedItem.status)
                : "-"
        }
    }
}
