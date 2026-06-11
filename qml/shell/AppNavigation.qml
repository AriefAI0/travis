import QtQuick

// Centralizes route-like navigation actions for the cached QML shell.

QtObject {
    id: root

    property var shell

    function goProjects() {
        if (!shell) {
            console.warn("[Navigation] Shell is not ready")
            return
        }

        shell.showProjects()
    }

    function goProject(projectId) {
        if (!shell) {
            console.warn("[Navigation] Shell is not ready")
            return
        }

        shell.showProject(projectId)
    }

    function goInspectionWorkspace(projectId) {
        if (!shell) {
            console.warn("[Navigation] Shell is not ready")
            return
        }

        shell.showInspection(projectId)
    }

    function goPlayback(projectId, masterVideoId) {
        if (!shell) {
            console.warn("[Navigation] Shell is not ready")
            return
        }

        shell.showPlayback(projectId, masterVideoId)
    }
}
