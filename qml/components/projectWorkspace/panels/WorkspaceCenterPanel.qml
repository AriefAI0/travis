import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../details"
import "../summaries"

// Switches between project workspace detail views for the active structure selection.

WorkspacePanel {
    id: root

    property var project: ({})
    property var assets: []
    property var selectedAsset: null
    property var selectedComponent: null
    property var selectedItem: null
    property string selectedType: "project"
    property int assetCount: 0
    property int componentCount: 0
    property int itemCount: 0
    property bool loading: false
    property string errorMessage: ""
    property var recordings: []

    signal createAssetRequested(string name)
    signal createComponentRequested(int assetId, string name)
    signal createItemRequested(int componentId, string label, string position, int status)
    signal assetSelected(var asset)
    signal componentSelected(var asset, var component)
    signal itemSelected(var asset, var component, var item)
    signal openPlaybackRequested(int masterVideoId)

    function clearAssetForm() {
        projectDetailsView.clearAssetForm()
    }

    function clearComponentForm() {
        assetDetailsView.clearComponentForm()
    }

    function clearItemForm() {
        componentDetailsView.clearItemForm()
    }

    content: [
        WorkspaceSelectionHeader {
            Layout.fillWidth: true
            project: root.project
            selectedType: root.selectedType
            selectedAsset: root.selectedAsset
            selectedComponent: root.selectedComponent
            selectedItem: root.selectedItem
            assetCount: root.assetCount
            componentCount: root.componentCount
            itemCount: root.itemCount
        },

        StatusBanner {
            Layout.fillWidth: true
            message: root.errorMessage
            error: true
        },

        ProjectDetailsView {
            id: projectDetailsView

            Layout.fillWidth: true
            visible: root.selectedType === "project"
            project: root.project
            assets: root.assets
            recordings: root.recordings
            assetCount: root.assetCount
            componentCount: root.componentCount
            itemCount: root.itemCount
            loading: root.loading
            onCreateAssetRequested: function(name) { root.createAssetRequested(name) }
            onAssetSelected: function(asset) { root.assetSelected(asset) }
            onOpenPlaybackRequested: function(masterVideoId) {
                root.openPlaybackRequested(masterVideoId)
            }
        },

        AssetDetailsView {
            id: assetDetailsView

            Layout.fillWidth: true
            visible: root.selectedType === "asset" && root.selectedAsset !== null
            selectedAsset: root.selectedAsset
            loading: root.loading
            onCreateComponentRequested: function(assetId, name) {
                root.createComponentRequested(assetId, name)
            }
            onComponentSelected: function(component) {
                root.componentSelected(root.selectedAsset, component)
            }
        },

        ComponentDetailsView {
            id: componentDetailsView

            Layout.fillWidth: true
            visible: root.selectedType === "component" && root.selectedComponent !== null
            selectedAsset: root.selectedAsset
            selectedComponent: root.selectedComponent
            loading: root.loading
            onCreateItemRequested: function(componentId, label, position, status) {
                root.createItemRequested(componentId, label, position, status)
            }
            onItemSelected: function(item) {
                root.itemSelected(root.selectedAsset, root.selectedComponent, item)
            }
        },

        ItemDetailsView {
            Layout.fillWidth: true
            visible: root.selectedType === "item" && root.selectedItem !== null
            selectedAsset: root.selectedAsset
            selectedComponent: root.selectedComponent
            selectedItem: root.selectedItem
        }
    ]
}
