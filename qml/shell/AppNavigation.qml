import QtQuick

// Centralizes route-like navigation actions for the Qt StackView shell.

QtObject {
    id: root

    property StackView stackView
    property Component inspectionWorkspacePage

    function goInspectionWorkspace() {
        if (!stackView || !inspectionWorkspacePage) {
            console.warn("[Navigation] Inspection workspace route is not ready")
            return
        }

        stackView.replace(inspectionWorkspacePage)
    }

    function goProjects() {
        console.warn("[Navigation] Project page is not implemented yet")
    }

    function goProject(projectId) {
        console.warn("[Navigation] Project workspace is not implemented yet:", projectId)
    }

    function goPlayback(projectId, masterVideoId) {
        console.warn("[Navigation] Playback workspace is not implemented yet:", projectId, masterVideoId)
    }
}
