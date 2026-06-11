import QtQuick
import QtQuick.Controls

import "../components/inspection"

// Lightweight inspection route; heavy workspace content is loaded asynchronously.

Item {
    id: root

    property var navigation
    property int projectId: 0

    Rectangle {
        anchors.fill: parent
        color: "#101820"
    }

    Label {
        anchors.centerIn: parent
        visible: inspectionContentLoader.status !== Loader.Ready
        color: "#9fb1bf"
        font.pixelSize: 13
        text: "Loading inspection workspace..."
    }

    Loader {
        id: inspectionContentLoader

        anchors.fill: parent
        asynchronous: true
        sourceComponent: inspectionContentComponent
    }

    Component {
        id: inspectionContentComponent

        InspectionWorkspaceContent {
            navigation: root.navigation
            projectId: root.projectId
        }
    }
}
