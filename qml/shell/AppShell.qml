import QtQuick
import QtQuick.Controls

import "../pages"

// Owns the application window and Qt-native StackView navigation.

ApplicationWindow {
    id: root

    width: 1280
    height: 720
    visible: true
    title: "Travis Inspection Workspace"

    Component {
        id: inspectionWorkspaceRoute

        InspectionWorkspacePage {
            navigation: appNavigation
        }
    }

    AppNavigation {
        id: appNavigation

        stackView: appStack
        inspectionWorkspacePage: inspectionWorkspaceRoute
    }

    StackView {
        id: appStack

        anchors.fill: parent
        initialItem: inspectionWorkspaceRoute
    }
}
