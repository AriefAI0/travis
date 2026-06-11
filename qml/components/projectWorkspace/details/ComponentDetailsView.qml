import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../../projects"

// Component detail view with item creation and item table.

ColumnLayout {
    id: root

    property var selectedAsset: null
    property var selectedComponent: null
    property bool loading: false

    signal createItemRequested(int componentId, string label, string position, int status)
    signal itemSelected(var item)

    function clearItemForm() {
        itemLabelField.text = ""
        itemPositionField.text = ""
        itemStatusField.value = 0
    }

    spacing: 12

    DetailCard {
        Layout.fillWidth: true
        title: "Items"
        meta: root.selectedComponent ? root.selectedComponent.items.length + " item(s)" : ""

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            rowSpacing: 8
            columnSpacing: 8

            TextField {
                id: itemLabelField

                Layout.fillWidth: true
                placeholderText: "Item label"
                enabled: !root.loading
            }

            TextField {
                id: itemPositionField

                Layout.fillWidth: true
                placeholderText: "Position"
                enabled: !root.loading
            }

            SpinBox {
                id: itemStatusField

                Layout.preferredWidth: 96
                from: 0
                to: 100
                value: 0
                enabled: !root.loading
            }

            ProjectActionButton {
                text: root.loading ? "Adding..." : "Add item"
                enabled: !root.loading &&
                    root.selectedComponent !== null &&
                    itemLabelField.text.trim().length > 0
                onClicked: root.createItemRequested(
                    root.selectedComponent.componentId,
                    itemLabelField.text,
                    itemPositionField.text,
                    itemStatusField.value
                )
            }
        }

        StructureTable {
            Layout.fillWidth: true
            rows: root.selectedComponent ? root.selectedComponent.items : []
            emptyText: "No items yet."
            firstHeader: "Label"
            secondHeader: "Position"
            thirdHeader: "Status"
            firstValue: function(row) { return row.itemLabel }
            secondValue: function(row) { return row.position || "-" }
            thirdValue: function(row) { return row.status === undefined ? "-" : String(row.status) }
            onRowSelected: function(row) { root.itemSelected(row) }
        }
    }
}
