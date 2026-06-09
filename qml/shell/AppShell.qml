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
        id: projectRoute

        ProjectPage {
            navigation: appNavigation
        }
    }

    Component {
        id: projectWorkspaceRoute

        ProjectWorkspacePage {
            navigation: appNavigation
        }
    }

    Component {
        id: inspectionWorkspaceRoute

        InspectionWorkspacePage {
            navigation: appNavigation
        }
    }

    Component {
        id: playbackWorkspaceRoute

        PlaybackWorkspacePage {
            navigation: appNavigation
        }
    }

    AppNavigation {
        id: appNavigation

        stackView: appStack
        projectPage: projectRoute
        projectWorkspacePage: projectWorkspaceRoute
        inspectionWorkspacePage: inspectionWorkspaceRoute
        playbackWorkspacePage: playbackWorkspaceRoute
    }

    StackView {
        id: appStack

        anchors.fill: parent
        initialItem: projectRoute
    }
}
