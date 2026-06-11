import QtQuick
import QtQuick.Controls

import "../pages"

// Owns the application window and cached route navigation.

ApplicationWindow {
    id: root

    width: 1280
    height: 720
    visible: true
    title: "Travis Inspection Workspace"

    property string currentRoute: "projects"

    function stopRouteSideEffects(nextRoute) {
        if (currentRoute === "playback" && nextRoute !== "playback" && playbackLoader.item) {
            playbackLoader.item.stopPlaybackRoute()
        }
    }

    function showProjects() {
        stopRouteSideEffects("projects")
        currentRoute = "projects"
        projectLoader.active = true
    }

    function showProject(projectId) {
        stopRouteSideEffects("project")
        currentRoute = "project"
        projectWorkspaceLoader.active = true
        projectWorkspaceLoader.pendingProjectId = Number(projectId) || 0
        if (projectWorkspaceLoader.item) {
            projectWorkspaceLoader.item.projectId = projectWorkspaceLoader.pendingProjectId
        }
    }

    function showInspection(projectId) {
        stopRouteSideEffects("inspection")
        currentRoute = "inspection"
        inspectionLoader.active = true
        inspectionLoader.pendingProjectId = Number(projectId) || 0
        if (inspectionLoader.item) {
            inspectionLoader.item.projectId = inspectionLoader.pendingProjectId
        }
    }

    function showPlayback(projectId, masterVideoId) {
        currentRoute = "playback"
        playbackLoader.active = true
        playbackLoader.pendingProjectId = Number(projectId) || 0
        playbackLoader.pendingMasterVideoId = Number(masterVideoId) || 0
        if (playbackLoader.item) {
            playbackLoader.item.projectId = playbackLoader.pendingProjectId
            playbackLoader.item.masterVideoId = playbackLoader.pendingMasterVideoId
        }
    }

    AppNavigation {
        id: appNavigation

        shell: root
    }

    Loader {
        id: projectLoader

        anchors.fill: parent
        active: true
        visible: root.currentRoute === "projects"
        sourceComponent: ProjectPage {
            navigation: appNavigation
        }
    }

    Loader {
        id: projectWorkspaceLoader

        property int pendingProjectId: 0

        anchors.fill: parent
        active: false
        visible: root.currentRoute === "project"
        sourceComponent: ProjectWorkspacePage {
            navigation: appNavigation
        }
        onLoaded: item.projectId = pendingProjectId
    }

    Loader {
        id: inspectionLoader

        property int pendingProjectId: 0

        anchors.fill: parent
        active: false
        visible: root.currentRoute === "inspection"
        sourceComponent: InspectionWorkspacePage {
            navigation: appNavigation
        }
        onLoaded: item.projectId = pendingProjectId
    }

    Loader {
        id: playbackLoader

        property int pendingProjectId: 0
        property int pendingMasterVideoId: 0

        anchors.fill: parent
        active: false
        visible: root.currentRoute === "playback"
        sourceComponent: PlaybackWorkspacePage {
            navigation: appNavigation
        }
        onLoaded: {
            item.projectId = pendingProjectId
            item.masterVideoId = pendingMasterVideoId
        }
    }
}
