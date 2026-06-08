import QtQuick
import QtQuick.Controls

import "pages"

// Loads the first native inspection workspace shell on top of the new Qt/C++ ui bridge.

ApplicationWindow {
    id: root

    width: 1280
    height: 720
    visible: true
    title: "Travis Inspection Workspace"

    InspectionWorkspacePage {
        anchors.fill: parent
    }
}
