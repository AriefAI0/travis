import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../summaries"

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

            ProjectRecordingsSummary {
                Layout.fillWidth: true
                recordings: root.recordings
            }
        },

        SelectionSummaryPanel {
            Layout.fillWidth: true
            visible: root.selectedType !== "project"
            selectedType: root.selectedType
            selectedAsset: root.selectedAsset
            selectedComponent: root.selectedComponent
            selectedItem: root.selectedItem
        }
    ]
}
