import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "centerPanel"
import "timeline"
import "rightPanel"
import "evidenceExplorer"
import "../.."

// Composes playback workspace panels while keeping the route page thin.

ColumnLayout {
    id: root

    property var playbackViewModel
    property var playbackSurfaceController

    spacing: 0

    StatusBanner {
        Layout.fillWidth: true
        message: root.playbackViewModel && root.playbackViewModel.lastError.length > 0
            ? root.playbackViewModel.lastError
            : root.playbackSurfaceController ? root.playbackSurfaceController.lastError : ""
        error: true
    }

    StatusBanner {
        Layout.fillWidth: true
        message: root.playbackViewModel ? root.playbackViewModel.statusMessage : ""
        error: false
    }

    GridLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        columns: 3
        rowSpacing: 0
        columnSpacing: 0

        PlaybackVideoPanel {
            Layout.column: 0
            Layout.row: 0
            Layout.columnSpan: 2
            playbackViewModel: root.playbackViewModel
            playbackSurfaceController: root.playbackSurfaceController
        }

        PlaybackMetadataPanel {
            Layout.column: 2
            Layout.row: 0
            Layout.preferredWidth: 260
            playbackViewModel: root.playbackViewModel
        }

        PlaybackTimelinePanel {
            Layout.column: 0
            Layout.row: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            playbackViewModel: root.playbackViewModel
        }

        PlaybackEvidenceExplorer {
            Layout.column: 1
            Layout.row: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            playbackViewModel: root.playbackViewModel
        }

        Item {
            Layout.column: 2
            Layout.row: 1
            Layout.preferredWidth: 260
            Layout.fillHeight: true
        }
    }
}
