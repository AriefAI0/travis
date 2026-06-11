import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../../projects"

// Asset detail view with component creation and component table.

ColumnLayout {
    id: root

    property var selectedAsset: null
    property bool loading: false

    signal createComponentRequested(int assetId, string name)
    signal componentSelected(var component)

    function clearComponentForm() {
        componentNameField.text = ""
    }

    spacing: 12

    DetailCard {
        Layout.fillWidth: true
        title: "Components"
        meta: root.selectedAsset ? root.selectedAsset.components.length + " component(s)" : ""

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: componentNameField

                Layout.fillWidth: true
                placeholderText: "Component name"
                enabled: !root.loading
                onAccepted: addComponentButton.clicked()
            }

            ProjectActionButton {
                id: addComponentButton

                text: root.loading ? "Adding..." : "Add component"
                enabled: !root.loading &&
                    root.selectedAsset !== null &&
                    componentNameField.text.trim().length > 0
                onClicked: root.createComponentRequested(
                    root.selectedAsset.assetId,
                    componentNameField.text
                )
            }
        }

        StructureTable {
            Layout.fillWidth: true
            rows: root.selectedAsset ? root.selectedAsset.components : []
            emptyText: "No components yet."
            firstHeader: "Name"
            secondHeader: "Items"
            thirdHeader: "Asset"
            firstValue: function(row) { return row.name }
            secondValue: function(row) { return String(row.items.length) }
            thirdValue: function() { return root.selectedAsset ? root.selectedAsset.name : "-" }
            onRowSelected: function(row) { root.componentSelected(row) }
        }
    }
}
