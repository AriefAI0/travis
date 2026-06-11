import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components/playback"
import "../layouts"

// Shows persisted playback state for the selected master video route.

WorkspaceShellLayout {
    id: root

    property var navigation
    property int projectId: 0
    property int masterVideoId: 0

    function loadPlaybackRoute() {
        if (!playbackViewModel) {
            return
        }

        if (playbackSurfaceController) {
            playbackSurfaceController.stopPlayback()
        }

        playbackViewModel.clearSelection()

        if (root.masterVideoId > 0) {
            playbackViewModel.openMasterVideo(root.masterVideoId)
        }
    }

    title: "Playback Workspace"
    subtitle: masterVideoId > 0
        ? `Reviewing master video ${masterVideoId}`
        : "Select a master video from a project before opening playback"

    headerActions: [
        Button {
            text: "Project"
            enabled: root.navigation && root.projectId > 0
            onClicked: root.navigation.goProject(root.projectId)
        },

        Button {
            text: "Inspection"
            enabled: root.navigation && root.projectId > 0
            onClicked: root.navigation.goInspectionWorkspace(root.projectId)
        }
    ]

    body: [
        PlaybackWorkspaceContent {
            Layout.fillWidth: true
            Layout.fillHeight: true
            playbackViewModel: playbackViewModel
            playbackSurfaceController: playbackSurfaceController
        }
    ]

    onMasterVideoIdChanged: loadPlaybackRoute()

    Component.onCompleted: loadPlaybackRoute()

    Component.onDestruction: {
        if (playbackSurfaceController) {
            playbackSurfaceController.stopPlayback()
        }
    }
}
