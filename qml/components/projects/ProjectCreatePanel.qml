import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"

// Create-project form panel used by the project dashboard.

Rectangle {
    id: root

    property bool loading: false
    property string errorMessage: ""
    signal cancelRequested()
    signal createRequested(string title, string description, string documentId)

    implicitHeight: createLayout.implicitHeight + 48
    radius: 18
    color: "#1b1f27"
    border.color: "#303642"
    border.width: 1

    ColumnLayout {
        id: createLayout

        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        ColumnLayout {
            spacing: 6

            Label {
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 20
                text: "Create project"
            }

            Label {
                color: "#a8b0bd"
                font.pixelSize: 13
                text: "Add a new project to start organizing assets and inspection work."
                wrapMode: Text.Wrap
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.width < 760 ? 1 : 2
            columnSpacing: 16
            rowSpacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    color: "#e6e8eb"
                    font.bold: true
                    text: "Title"
                }

                TextField {
                    id: titleField

                    Layout.fillWidth: true
                    enabled: !root.loading
                    color: "#e6e8eb"
                    placeholderTextColor: "#78818f"
                    background: Rectangle {
                        radius: 10
                        color: "#141821"
                        border.color: titleField.activeFocus ? "#c1272d" : "#3b424f"
                        border.width: 1
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    color: "#e6e8eb"
                    font.bold: true
                    text: "Document ID"
                }

                TextField {
                    id: documentIdField

                    Layout.fillWidth: true
                    enabled: !root.loading
                    color: "#e6e8eb"
                    placeholderTextColor: "#78818f"
                    background: Rectangle {
                        radius: 10
                        color: "#141821"
                        border.color: documentIdField.activeFocus ? "#c1272d" : "#3b424f"
                        border.width: 1
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                color: "#e6e8eb"
                font.bold: true
                text: "Description"
            }

            TextArea {
                id: descriptionField

                Layout.fillWidth: true
                Layout.preferredHeight: 96
                enabled: !root.loading
                wrapMode: TextArea.Wrap
                color: "#e6e8eb"
                placeholderTextColor: "#78818f"
                background: Rectangle {
                    radius: 10
                    color: "#141821"
                    border.color: descriptionField.activeFocus ? "#c1272d" : "#3b424f"
                    border.width: 1
                }
            }
        }

        StatusBanner {
            Layout.fillWidth: true
            message: root.errorMessage
            error: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 12

            ProjectActionButton {
                text: "Cancel"
                onClicked: root.cancelRequested()
            }

            ProjectActionButton {
                primary: true
                text: root.loading ? "Creating project..." : "Save project"
                enabled: !root.loading && titleField.text.trim().length > 0
                onClicked: root.createRequested(
                    titleField.text,
                    descriptionField.text,
                    documentIdField.text
                )
            }
        }
    }

    function clearFields() {
        titleField.clear()
        descriptionField.clear()
        documentIdField.clear()
    }
}
