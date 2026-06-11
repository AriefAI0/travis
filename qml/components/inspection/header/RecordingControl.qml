import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Provides inspection recording action buttons bound to the recording viewmodel.

RowLayout {
    id: root

    property var recordingViewModel

    spacing: 10

    Button {
        text: "Start"
        enabled: recordingViewModel && !recordingViewModel.recordingActive
        onClicked: recordingViewModel.startRecording()
    }

    Button {
        text: "Pause"
        enabled: recordingViewModel && recordingViewModel.recordingActive && !recordingViewModel.paused
        onClicked: recordingViewModel.pauseRecording()
    }

    Button {
        text: "Resume"
        enabled: recordingViewModel && recordingViewModel.recordingActive && recordingViewModel.paused
        onClicked: recordingViewModel.resumeRecording()
    }

    Button {
        text: "Stop"
        enabled: recordingViewModel && recordingViewModel.recordingActive
        onClicked: recordingViewModel.stopRecording()
    }
}
