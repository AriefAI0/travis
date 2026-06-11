import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components/projectWorkspace"
import "../components/projectWorkspace/panels"
import "../components/workspaceCommon"

// Project workspace page following the Electron layout with native QML panels.

Item {
    id: root

    property var navigation
    property int projectId: 0
    property string selectedType: "project"
    property string selectedKey: "project"
    property var selectedAsset: null
    property var selectedComponent: null
    property var selectedItem: null
    property bool completed: false

    function selectProject() {
        selectedType = "project"
        selectedKey = "project"
        selectedAsset = null
        selectedComponent = null
        selectedItem = null
    }

    function selectAsset(asset) {
        selectedType = "asset"
        selectedKey = "asset:" + asset.assetId
        selectedAsset = asset
        selectedComponent = null
        selectedItem = null
    }

    function selectComponent(asset, component) {
        selectedType = "component"
        selectedKey = "component:" + component.componentId
        selectedAsset = asset
        selectedComponent = component
        selectedItem = null
    }

    function selectItem(asset, component, item) {
        selectedType = "item"
        selectedKey = "item:" + item.itemId
        selectedAsset = asset
        selectedComponent = component
        selectedItem = item
    }

    function reloadProject() {
        if (projectId > 0) {
            projectWorkspaceViewModel.loadProject(projectId)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#0f1117"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        WorkspaceHeader {
            Layout.fillWidth: true
            projectTitle: projectWorkspaceViewModel.project.title || "Project Workspace"
            loading: projectWorkspaceViewModel.loading
            canInspect: root.projectId > 0
            onBackRequested: {
                if (root.navigation) {
                    root.navigation.goProjects()
                }
            }
            onRefreshRequested: root.reloadProject()
            onInspectionRequested: {
                if (root.navigation && root.projectId > 0) {
                    root.navigation.goInspectionWorkspace(root.projectId)
                }
            }
            onProjectSelected: root.selectProject()
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#303642"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            visible: projectWorkspaceViewModel.loading
            radius: 8
            color: "#1b1f27"
            border.color: "#303642"

            Label {
                anchors.fill: parent
                anchors.margins: 12
                color: "#a8b0bd"
                font.pixelSize: 12
                text: "Loading project workspace..."
                verticalAlignment: Text.AlignVCenter
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            StructureTreePanel {
                Layout.preferredWidth: 250
                Layout.fillHeight: true
                assets: projectWorkspaceViewModel.structureTree
                selectedKey: root.selectedKey
                onProjectSelected: root.selectProject()
                onAssetSelected: function(asset) { root.selectAsset(asset) }
                onComponentSelected: function(asset, component) {
                    root.selectComponent(asset, component)
                }
                onItemSelected: function(asset, component, item) {
                    root.selectItem(asset, component, item)
                }
            }

            WorkspaceCenterPanel {
                id: centerPanel

                Layout.fillWidth: true
                Layout.fillHeight: true
                project: projectWorkspaceViewModel.project
                assets: projectWorkspaceViewModel.structureTree
                selectedType: root.selectedType
                selectedAsset: root.selectedAsset
                selectedComponent: root.selectedComponent
                selectedItem: root.selectedItem
                assetCount: projectWorkspaceViewModel.assetCount
                componentCount: projectWorkspaceViewModel.componentCount
                itemCount: projectWorkspaceViewModel.itemCount
                loading: projectWorkspaceViewModel.loading
                errorMessage: projectWorkspaceViewModel.lastError
                recordings: projectWorkspaceViewModel.masterVideos
                onAssetSelected: function(asset) { root.selectAsset(asset) }
                onComponentSelected: function(asset, component) {
                    root.selectComponent(asset, component)
                }
                onItemSelected: function(asset, component, item) {
                    root.selectItem(asset, component, item)
                }
                onCreateAssetRequested: function(name) {
                    if (projectWorkspaceViewModel.createAsset(name)) {
                        centerPanel.clearAssetForm()
                        root.selectProject()
                    }
                }
                onCreateComponentRequested: function(assetId, name) {
                    if (projectWorkspaceViewModel.createComponent(assetId, name)) {
                        centerPanel.clearComponentForm()
                        root.selectProject()
                    }
                }
                onCreateItemRequested: function(componentId, label, position, status) {
                    if (projectWorkspaceViewModel.createItem(componentId, label, position, status)) {
                        centerPanel.clearItemForm()
                        root.selectProject()
                    }
                }
                onOpenPlaybackRequested: function(masterVideoId) {
                    if (root.navigation && root.projectId > 0 && masterVideoId > 0) {
                        root.navigation.goPlayback(root.projectId, masterVideoId)
                    }
                }
            }

            WorkspaceRightPanel {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                selectedType: root.selectedType
                selectedAsset: root.selectedAsset
                selectedComponent: root.selectedComponent
                selectedItem: root.selectedItem
                recordings: projectWorkspaceViewModel.masterVideos
                assetCount: projectWorkspaceViewModel.assetCount
                componentCount: projectWorkspaceViewModel.componentCount
                itemCount: projectWorkspaceViewModel.itemCount
            }
        }
    }

    onProjectIdChanged: {
        root.selectProject()
        if (root.completed) {
            root.reloadProject()
        }
    }

    Component.onCompleted: {
        root.completed = true
        root.reloadProject()
    }
}
