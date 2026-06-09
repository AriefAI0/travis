import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../layouts"

// Shows the selected project overview, structure tree, and session list.

WorkspaceShellLayout {
    id: root

    property var navigation
    property int projectId: 0

    title: projectWorkspaceViewModel.project.title || "Project Workspace"
    subtitle: projectWorkspaceViewModel.loading
        ? "Loading project workspace..."
        : (projectWorkspaceViewModel.project.description || "Project workspace overview")

    headerActions: [
        Button {
            text: "Projects"
            onClicked: root.navigation.goProjects()
        },

        Button {
            text: "Inspection"
            onClicked: root.navigation.goInspectionWorkspace(root.projectId)
        },

        Button {
            text: "Refresh"
            enabled: root.projectId > 0 && !projectWorkspaceViewModel.loading
            onClicked: projectWorkspaceViewModel.loadProject(root.projectId)
        }
    ]

    body: [
        Label {
            Layout.fillWidth: true
            visible: projectWorkspaceViewModel.lastError.length > 0
            color: "#ff9f9f"
            text: projectWorkspaceViewModel.lastError
            wrapMode: Text.Wrap
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

                            ColumnLayout {
                                id: assetContent

                                anchors.fill: parent
                                anchors.margins: 9
                                spacing: 5

                                Label {
                                    Layout.fillWidth: true
                                    color: "#eef3f7"
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
                                            color: "#bfd0dd"
                                            text: modelData.name
                                            elide: Text.ElideRight
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

                    Label {
                        color: "#f4f7fa"
                        font.pixelSize: 18
                        text: "Next actions"
                    }

                    RowLayout {
                        Button {
                            text: "Start Inspection"
                            enabled: root.projectId > 0
                            onClicked: root.navigation.goInspectionWorkspace(root.projectId)
                        }

                        Button {
                            text: "Back to Projects"
                            onClicked: root.navigation.goProjects()
                        }
                    }

                    Item {
                        Layout.fillHeight: true
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
