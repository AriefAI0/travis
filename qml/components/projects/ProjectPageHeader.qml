import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Header section for the project dashboard, matching the Electron project page layout.

Item {
    id: root

    property bool loading: false
    property bool createFormVisible: false
    signal refreshRequested()
    signal toggleCreateRequested()

    implicitHeight: headerLayout.implicitHeight

    RowLayout {
        id: headerLayout

        anchors.fill: parent
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                color: "#78818f"
                font.bold: true
                font.pixelSize: 11
                text: "PROJECT WORKSPACE"
            }

            Label {
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 32
                text: "Projects"
            }

            Label {
                Layout.maximumWidth: 560
                color: "#a8b0bd"
                font.pixelSize: 13
                text: "View all projects and create a new project from one place."
                wrapMode: Text.Wrap
            }
        }

        Flow {
            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            Layout.preferredWidth: Math.min(260, root.width)
            spacing: 12

            ProjectActionButton {
                text: root.loading ? "Refreshing..." : "Refresh"
                enabled: !root.loading
                onClicked: root.refreshRequested()
            }

            ProjectActionButton {
                primary: true
                text: root.createFormVisible ? "Close form" : "Create project"
                onClicked: root.toggleCreateRequested()
            }
        }
    }
}
