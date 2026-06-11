import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Provides inspection recording action buttons bound to the recording viewmodel.

RowLayout {
    id: root

    property var recordingViewModel

    spacing: 4

    Button {
        implicitHeight: 24
        implicitWidth: 54
        text: "Start"
        enabled: recordingViewModel && !recordingViewModel.recordingActive
        onClicked: recordingViewModel.startRecording()
    }

    Button {
        implicitHeight: 24
        implicitWidth: 54
        text: "Pause"
        enabled: recordingViewModel && recordingViewModel.recordingActive && !recordingViewModel.paused
        onClicked: recordingViewModel.pauseRecording()
    }

    Button {
        implicitHeight: 24
        implicitWidth: 62
        text: "Resume"
        enabled: recordingViewModel && recordingViewModel.recordingActive && recordingViewModel.paused
        onClicked: recordingViewModel.resumeRecording()
    }

    Button {
        implicitHeight: 24
        implicitWidth: 54
        text: "Stop"
        enabled: recordingViewModel && recordingViewModel.recordingActive
        onClicked: recordingViewModel.stopRecording()
    }
}
