import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../components/projects"

// Project dashboard page using the Electron project layout and card visual language.

Item {
    id: root

    property var navigation
    property bool createFormVisible: false

    Rectangle {
        anchors.fill: parent
        color: "#0f1117"
    }

    Flickable {
        id: pageScroll

        anchors.fill: parent
        contentWidth: width
        contentHeight: pageContent.implicitHeight + 64
        clip: true

        ColumnLayout {
            id: pageContent

            width: Math.min(Math.max(pageScroll.width - 40, 0), 1200)
            x: Math.max(20, (pageScroll.width - width) / 2)
            y: 32
            spacing: 24

            ProjectPageHeader {
                Layout.fillWidth: true
                loading: projectViewModel.loading
                createFormVisible: root.createFormVisible
                onRefreshRequested: projectViewModel.refreshProjects()
                onToggleCreateRequested: root.createFormVisible = !root.createFormVisible
            }

            ProjectCreatePanel {
                id: createPanel

                Layout.fillWidth: true
                visible: root.createFormVisible
                loading: projectViewModel.loading
                errorMessage: projectViewModel.lastError
                onCancelRequested: root.createFormVisible = false
                onCreateRequested: function(title, description, documentId) {
                    if (projectViewModel.createProject(title, description, documentId)) {
                        createPanel.clearFields()
                        root.createFormVisible = false
                    }
                }
            }

            StatusBanner {
                Layout.fillWidth: true
                message: !root.createFormVisible ? projectViewModel.lastError : ""
                error: true
            }

            StatusBanner {
                Layout.fillWidth: true
                message: projectViewModel.statusMessage
                error: false
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 108
                visible: projectViewModel.loading
                radius: 18
                color: "#1b1f27"
                border.color: "#303642"
                border.width: 1

                Label {
                    anchors.fill: parent
                    anchors.margins: 24
                    color: "#a8b0bd"
                    font.pixelSize: 13
                    text: "Loading projects..."
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: emptyLayout.implicitHeight + 48
                visible: !projectViewModel.loading &&
                    projectViewModel.lastError.length === 0 &&
                    projectViewModel.projects.length === 0
                radius: 18
                color: "#1b1f27"
                border.color: "#303642"
                border.width: 1

                ColumnLayout {
                    id: emptyLayout

                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 12

                    Label {
                        color: "#e6e8eb"
                        font.bold: true
                        font.pixelSize: 20
                        text: "No projects yet"
                    }

                    Label {
                        color: "#a8b0bd"
                        font.pixelSize: 13
                        text: "Create your first project to start building the inspection workflow."
                        wrapMode: Text.Wrap
                    }

                    ProjectActionButton {
                        primary: true
                        visible: !root.createFormVisible
                        text: "Create project"
                        onClicked: root.createFormVisible = true
                    }
                }
            }

            Flow {
                id: projectGrid

                Layout.fillWidth: true
                visible: !projectViewModel.loading &&
                    projectViewModel.lastError.length === 0 &&
                    projectViewModel.projects.length > 0
                spacing: 20

                Repeater {
                    model: projectViewModel.projects

                    ProjectCard {
                        project: modelData
                        width: Math.min(320, projectGrid.width)
                        onOpenRequested: function(projectId) {
                            if (root.navigation && projectId > 0) {
                                root.navigation.goProject(projectId)
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: projectViewModel.refreshProjects()
}
