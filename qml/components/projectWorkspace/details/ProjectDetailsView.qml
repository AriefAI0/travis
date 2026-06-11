import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../../projects"

// Project overview, asset creation, and review recording list.

ColumnLayout {
    id: root

    property var project: ({})
    property var assets: []
    property var recordings: []
    property int assetCount: 0
    property int componentCount: 0
    property int itemCount: 0
    property bool loading: false

    signal createAssetRequested(string name)
    signal assetSelected(var asset)
    signal openPlaybackRequested(int masterVideoId)

    function clearAssetForm() {
        assetNameField.text = ""
    }

    spacing: 12

    DetailCard {
        Layout.fillWidth: true
        title: "Project details"

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            rowSpacing: 8
            columnSpacing: 8

            StatBox { label: "Document ID"; value: root.project.documentId || "-" }
            StatBox { label: "Assets"; value: String(root.assetCount) }
            StatBox { label: "Components"; value: String(root.componentCount) }
            StatBox { label: "Items"; value: String(root.itemCount) }
        }
    }

    DetailCard {
        Layout.fillWidth: true
        title: "Assets"

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: assetNameField

                Layout.fillWidth: true
                placeholderText: "Asset name"
                enabled: !root.loading
                onAccepted: addAssetButton.clicked()
            }

            ProjectActionButton {
                id: addAssetButton

                text: root.loading ? "Adding..." : "Add asset"
                enabled: !root.loading && assetNameField.text.trim().length > 0
                onClicked: root.createAssetRequested(assetNameField.text)
            }
        }

        StructureTable {
            Layout.fillWidth: true
            rows: root.assets
            emptyText: "No assets yet."
            firstHeader: "Name"
            secondHeader: "Components"
            thirdHeader: "Items"
            firstValue: function(row) { return row.name }
            secondValue: function(row) { return String(row.components.length) }
            thirdValue: function(row) {
                let total = 0
                for (let i = 0; i < row.components.length; i++) {
                    total += row.components[i].items.length
                }
                return String(total)
            }
            onRowSelected: function(row) { root.assetSelected(row) }
        }
    }

    DetailCard {
        Layout.fillWidth: true
        title: "Review workspace"
        meta: root.recordings.length + " recording(s)"

        Label {
            Layout.fillWidth: true
            visible: root.recordings.length === 0
            color: "#78818f"
            font.pixelSize: 12
            text: "No finalized master videos are available for review yet."
            wrapMode: Text.Wrap
        }

        Repeater {
            model: root.recordings

            delegate: ReviewRecordingCard {
                recording: modelData
                onOpenPlaybackRequested: function(masterVideoId) {
                    root.openPlaybackRequested(masterVideoId)
                }
            }
        }
    }
}
