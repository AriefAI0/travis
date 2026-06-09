import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../layouts"

// Shows the selected project overview, structure tree, and session list.

WorkspaceShellLayout {
    id: root

    property var navigation
    property int projectId: 0
    property int selectedAssetId: 0
    property int selectedComponentId: 0
    property string selectedAssetName: ""
    property string selectedComponentName: ""

    title: projectWorkspaceViewModel.project.title || "Project Workspace"
    subtitle: projectWorkspaceViewModel.loading
        ? "Loading project workspace..."
        : (projectWorkspaceViewModel.project.description || "Project workspace overview")

    headerActions: [
        Button {
            text: "Projects"
            enabled: root.navigation
            onClicked: root.navigation.goProjects()
        },

        Button {
            text: "Inspection"
            enabled: root.navigation && root.projectId > 0
            onClicked: root.navigation.goInspectionWorkspace(root.projectId)
        },

        Button {
            text: "Refresh"
            enabled: root.projectId > 0 && !projectWorkspaceViewModel.loading
            onClicked: projectWorkspaceViewModel.loadProject(root.projectId)
        }
    ]

    body: [
        StatusBanner {
            Layout.fillWidth: true
            message: projectWorkspaceViewModel.lastError
            error: true
        },

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                radius: 12
                color: "#101820"
                border.color: "#31404d"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Label {
                        color: "#f4f7fa"
                        font.pixelSize: 18
                        text: "Structure"
                    }

                    Label {
                        color: "#9fb1bf"
                        text: `${projectWorkspaceViewModel.assetCount} assets | ${projectWorkspaceViewModel.componentCount} components | ${projectWorkspaceViewModel.itemCount} items`
                    }

                    TextField {
                        id: assetNameField

                        Layout.fillWidth: true
                        placeholderText: "New asset name"
                        onAccepted: addAssetButton.clicked()
                    }

                    Button {
                        id: addAssetButton

                        Layout.fillWidth: true
                        text: "Add Asset"
                        enabled: root.projectId > 0 &&
                            !projectWorkspaceViewModel.loading &&
                            assetNameField.text.trim().length > 0
                        onClicked: {
                            if (projectWorkspaceViewModel.createAsset(assetNameField.text)) {
                                assetNameField.clear()
                            }
                        }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8
                        model: projectWorkspaceViewModel.structureTree

                        delegate: Rectangle {
                            width: ListView.view.width
                            implicitHeight: assetContent.implicitHeight + 18
                            radius: 8
                            color: "#18222c"
                            border.color: "#334657"
                            border.width: 1

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.selectedAssetId = modelData.assetId
                                    root.selectedAssetName = modelData.name
                                    root.selectedComponentId = 0
                                    root.selectedComponentName = ""
                                }
                            }

                            ColumnLayout {
                                id: assetContent

                                anchors.fill: parent
                                anchors.margins: 9
                                spacing: 5

                                Label {
                                    Layout.fillWidth: true
                                    color: root.selectedAssetId === modelData.assetId ? "#9fe0b0" : "#eef3f7"
                                    font.bold: true
                                    text: modelData.name
                                    elide: Text.ElideRight
                                }

                                Repeater {
                                    model: modelData.components

                                    delegate: ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Label {
                                            Layout.fillWidth: true
                                            leftPadding: 8
                                            color: root.selectedComponentId === modelData.componentId ? "#9fe0b0" : "#bfd0dd"
                                            text: modelData.name
                                            elide: Text.ElideRight

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    root.selectedAssetId = modelData.assetId
                                                    root.selectedAssetName = ""
                                                    root.selectedComponentId = modelData.componentId
                                                    root.selectedComponentName = modelData.name
                                                }
                                            }
                                        }

                                        Repeater {
                                            model: modelData.items

                                            delegate: Label {
                                                Layout.fillWidth: true
                                                leftPadding: 18
                                                color: "#91a6b7"
                                                text: modelData.itemLabel
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 12
                color: "#121922"
                border.color: "#31404d"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Label {
                        color: "#f4f7fa"
                        font.pixelSize: 20
                        text: "Project Details"
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        rowSpacing: 8
                        columnSpacing: 12

                        Label { color: "#9fb1bf"; text: "Project ID" }
                        Label { color: "#eef3f7"; text: `${projectWorkspaceViewModel.project.projectId || ""}` }

                        Label { color: "#9fb1bf"; text: "Title" }
                        Label {
                            Layout.fillWidth: true
                            color: "#eef3f7"
                            text: projectWorkspaceViewModel.project.title || ""
                            wrapMode: Text.Wrap
                        }

                        Label { color: "#9fb1bf"; text: "Document" }
                        Label {
                            Layout.fillWidth: true
                            color: "#eef3f7"
                            text: projectWorkspaceViewModel.project.documentId || "No document ID"
                            wrapMode: Text.Wrap
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#31404d"
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            color: "#f4f7fa"
                            font.pixelSize: 18
                            text: "Recordings"
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            color: "#9fb1bf"
                            text: `${projectWorkspaceViewModel.masterVideos.length} video(s)`
                        }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 180
                        clip: true
                        spacing: 8
                        model: projectWorkspaceViewModel.masterVideos

                        delegate: Rectangle {
                            width: ListView.view.width
                            implicitHeight: videoContent.implicitHeight + 18
                            radius: 8
                            color: "#18222c"
                            border.color: "#334657"
                            border.width: 1

                            RowLayout {
                                id: videoContent

                                anchors.fill: parent
                                anchors.margins: 9
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 3

                                    Label {
                                        Layout.fillWidth: true
                                        color: "#eef3f7"
                                        text: modelData.sourceName.length > 0
                                            ? modelData.sourceName
                                            : `Master Video #${modelData.masterVideoId}`
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        color: "#91a6b7"
                                        text: modelData.fileUrl
                                        elide: Text.ElideMiddle
                                    }

                                    Label {
                                        color: "#91a6b7"
                                        text: `Session ${modelData.sessionId} | ${modelData.status}`
                                    }
                                }

                                Button {
                                    text: "Open Playback"
                                    enabled: root.navigation && modelData.masterVideoId > 0
                                    onClicked: root.navigation.goPlayback(
                                        root.projectId,
                                        modelData.masterVideoId
                                    )
                                }
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: projectWorkspaceViewModel.masterVideos.length === 0
                        color: "#9fb1bf"
                        text: "No recordings have been created for this project yet."
                        wrapMode: Text.Wrap
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#31404d"
                    }

                    Label {
                        color: "#f4f7fa"
                        font.pixelSize: 18
                        text: "Edit Structure"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            color: "#9fb1bf"
                            text: root.selectedAssetId > 0
                                ? `Selected asset: ${root.selectedAssetName.length > 0 ? root.selectedAssetName : root.selectedAssetId}`
                                : "Select an asset to add components"
                            wrapMode: Text.Wrap
                        }

                        TextField {
                            id: componentNameField

                            Layout.fillWidth: true
                            placeholderText: "New component name"
                            enabled: root.selectedAssetId > 0
                            onAccepted: addComponentButton.clicked()
                        }

                        Button {
                            id: addComponentButton

                            Layout.fillWidth: true
                            text: "Add Component"
                            enabled: root.selectedAssetId > 0 &&
                                !projectWorkspaceViewModel.loading &&
                                componentNameField.text.trim().length > 0
                            onClicked: {
                                if (projectWorkspaceViewModel.createComponent(
                                        root.selectedAssetId,
                                        componentNameField.text
                                    )) {
                                    componentNameField.clear()
                                }
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            color: "#9fb1bf"
                            text: root.selectedComponentId > 0
                                ? `Selected component: ${root.selectedComponentName.length > 0 ? root.selectedComponentName : root.selectedComponentId}`
                                : "Select a component to add items"
                            wrapMode: Text.Wrap
                        }

                        TextField {
                            id: itemLabelField

                            Layout.fillWidth: true
                            placeholderText: "New item label"
                            enabled: root.selectedComponentId > 0
                        }

                        TextField {
                            id: itemPositionField

                            Layout.fillWidth: true
                            placeholderText: "Position"
                            enabled: root.selectedComponentId > 0
                        }

                        SpinBox {
                            id: itemStatusField

                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 0
                            enabled: root.selectedComponentId > 0
                        }

                        Button {
                            Layout.fillWidth: true
                            text: "Add Item"
                            enabled: root.selectedComponentId > 0 &&
                                !projectWorkspaceViewModel.loading &&
                                itemLabelField.text.trim().length > 0
                            onClicked: {
                                if (projectWorkspaceViewModel.createItem(
                                        root.selectedComponentId,
                                        itemLabelField.text,
                                        itemPositionField.text,
                                        itemStatusField.value
                                    )) {
                                    itemLabelField.clear()
                                    itemPositionField.clear()
                                    itemStatusField.value = 0
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                radius: 12
                color: "#101820"
                border.color: "#31404d"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Label {
                        color: "#f4f7fa"
                        font.pixelSize: 18
                        text: "Sessions"
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8
                        model: projectWorkspaceViewModel.sessions

                        delegate: Rectangle {
                            width: ListView.view.width
                            implicitHeight: sessionContent.implicitHeight + 18
                            radius: 8
                            color: "#18222c"
                            border.color: "#334657"
                            border.width: 1

                            ColumnLayout {
                                id: sessionContent

                                anchors.fill: parent
                                anchors.margins: 9
                                spacing: 4

                                Label {
                                    Layout.fillWidth: true
                                    color: "#eef3f7"
                                    text: modelData.name
                                    elide: Text.ElideRight
                                }

                                Label {
                                    color: "#91a6b7"
                                    text: `Session ID: ${modelData.sessionId}`
                                }
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: projectWorkspaceViewModel.sessions.length === 0
                        color: "#9fb1bf"
                        text: "No sessions yet. Start an inspection to create one."
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    ]

    onProjectIdChanged: {
        root.selectedAssetId = 0
        root.selectedComponentId = 0
        root.selectedAssetName = ""
        root.selectedComponentName = ""
        if (projectId > 0) {
            projectWorkspaceViewModel.loadProject(projectId)
        }
    }

    Component.onCompleted: {
        if (projectId > 0) {
            projectWorkspaceViewModel.loadProject(projectId)
        }
    }
}
