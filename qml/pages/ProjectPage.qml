import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../layouts"

// Lists projects from the native project service and routes into a selected project workspace.

WorkspaceShellLayout {
    id: root

    property var navigation

    title: "Projects"
    subtitle: projectViewModel.loading
        ? "Loading projects..."
        : "Select an existing inspection project or create a new one."

    headerActions: [
        Button {
            text: "Refresh"
            enabled: !projectViewModel.loading
            onClicked: projectViewModel.refreshProjects()
        }
    ]

    body: [
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 2
                radius: 0
                color: "#121922"
                border.color: "#31404d"
                border.width: 1

                ListView {
                    id: projectList

                    anchors.fill: parent
                    anchors.margins: 0
                    clip: true
                    spacing: 0
                    model: projectViewModel.projects

                    delegate: Rectangle {
                        width: ListView.view.width
                        implicitHeight: projectCardContent.implicitHeight + 16
                        radius: 0
                        color: index % 2 === 0 ? "#16202a" : "#121a22"
                        border.color: "#263542"
                        border.width: 0

                        ColumnLayout {
                            id: projectCardContent

                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            Label {
                                Layout.fillWidth: true
                                color: "#f4f7fa"
                                font.pixelSize: 18
                                font.bold: true
                                text: modelData.title
                                elide: Text.ElideRight
                            }

                            Label {
                                Layout.fillWidth: true
                                color: "#9fb1bf"
                                text: modelData.description.length > 0
                                    ? modelData.description
                                    : "No description"
                                wrapMode: Text.Wrap
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    Layout.fillWidth: true
                                    color: "#7f94a6"
                                    text: modelData.documentId.length > 0
                                        ? `Document: ${modelData.documentId}`
                                        : "No document ID"
                                }

                                Button {
                                    text: "Open"
                                    enabled: root.navigation && modelData.projectId > 0
                                    onClicked: root.navigation.goProject(modelData.projectId)
                                }

                                Button {
                                    text: "Inspect"
                                    enabled: root.navigation && modelData.projectId > 0
                                    onClicked: root.navigation.goInspectionWorkspace(modelData.projectId)
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                radius: 0
                color: "#101820"
                border.color: "#31404d"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Label {
                        color: "#f4f7fa"
                        font.pixelSize: 18
                        text: "Create Project"
                    }

                    StatusBanner {
                        Layout.fillWidth: true
                        message: projectViewModel.lastError
                        error: true
                    }

                    StatusBanner {
                        Layout.fillWidth: true
                        message: projectViewModel.statusMessage
                        error: false
                    }

                    TextField {
                        id: titleField

                        Layout.fillWidth: true
                        placeholderText: "Project title"
                    }

                    TextArea {
                        id: descriptionField

                        Layout.fillWidth: true
                        Layout.preferredHeight: 90
                        placeholderText: "Description"
                        wrapMode: TextArea.Wrap
                    }

                    TextField {
                        id: documentIdField

                        Layout.fillWidth: true
                        placeholderText: "Document ID"
                    }

                    Button {
                        Layout.fillWidth: true
                        text: "Create"
                        enabled: !projectViewModel.loading && titleField.text.trim().length > 0
                        onClicked: {
                            if (projectViewModel.createProject(
                                    titleField.text,
                                    descriptionField.text,
                                    documentIdField.text
                                )) {
                                titleField.clear()
                                descriptionField.clear()
                                documentIdField.clear()
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
    ]

    Component.onCompleted: projectViewModel.refreshProjects()
}
