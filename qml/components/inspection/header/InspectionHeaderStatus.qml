import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shows recording/session state and compact master recording controls in the header.

RowLayout {
    id: root

    property var recordingViewModel
    property var inspectionContextViewModel

    spacing: 8

    Rectangle {
        Layout.preferredWidth: 8
        Layout.preferredHeight: 8
        radius: 4
        color: recordingViewModel && recordingViewModel.recordingActive
            ? (recordingViewModel.paused ? "#d8a536" : "#c1272d")
            : "#78818f"
    }

    Label {
        color: "#a8b0bd"
        font.pixelSize: 11
        text: recordingViewModel && recordingViewModel.recordingActive
            ? (recordingViewModel.paused ? "Paused" : "Recording")
            : "Idle"
    }

    Label {
        color: "#78818f"
        font.pixelSize: 11
        text: inspectionContextViewModel && inspectionContextViewModel.sessionId > 0
            ? `Session ${inspectionContextViewModel.sessionId}`
            : "No session"
    }

    Item {
        Layout.fillWidth: true
    }

    RecordingControl {
        recordingViewModel: root.recordingViewModel
    }
}
