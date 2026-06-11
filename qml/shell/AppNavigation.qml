import QtQuick
import QtQuick.Controls

// Centralizes route-like navigation actions for the Qt StackView shell.

QtObject {
    id: root

    property StackView stackView
    property Component projectPage
    property Component projectWorkspacePage
    property Component inspectionWorkspacePage
    property Component playbackWorkspacePage

    function replaceWith(component, properties) {
        if (!stackView || !component) {
            console.warn("[Navigation] Route is not ready")
            return
        }

        stackView.replace(component, properties || {}, StackView.Immediate)
    }

    function goProjects() {
        replaceWith(projectPage)
    }

    function goProject(projectId) {
        replaceWith(projectWorkspacePage, { projectId: Number(projectId) || 0 })
    }

    function goInspectionWorkspace(projectId) {
        replaceWith(inspectionWorkspacePage, { projectId: Number(projectId) || 0 })
    }

    function goPlayback(projectId, masterVideoId) {
        replaceWith(playbackWorkspacePage, {
            projectId: Number(projectId) || 0,
            masterVideoId: Number(masterVideoId) || 0
        })
    }
}
