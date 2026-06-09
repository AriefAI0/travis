import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
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
        StatusBanner {
            Layout.fillWidth: true
            message: playbackViewModel ? playbackViewModel.lastError : ""
            error: true
        },

        StatusBanner {
            Layout.fillWidth: true
            message: playbackViewModel ? playbackViewModel.statusMessage : ""
            error: false
        },

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 260
            radius: 12
            color: "#101720"
            border.color: "#31404d"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Label {
                    color: "#f4f7fa"
                    font.pixelSize: 22
                    text: playbackViewModel && playbackViewModel.selectedMasterVideoId > 0
                        ? `Master Video #${playbackViewModel.selectedMasterVideoId}`
                        : "No master video loaded"
                }

                Label {
                    Layout.fillWidth: true
                    color: "#9fb1bf"
                    text: playbackViewModel && playbackViewModel.selectedMasterVideoPath.length > 0
                        ? playbackViewModel.selectedMasterVideoPath
                        : "Open playback from a project video list to load persisted media metadata."
                    wrapMode: Text.Wrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 10
                    color: "#06090d"
                    border.color: "#22313e"
                    border.width: 1

                    Label {
                        anchors.centerIn: parent
                        color: "#758796"
                        text: "Native playback surface wiring pending"
                    }
                }
            }
        },

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: "#121922"
            border.color: "#31404d"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        color: "#f4f7fa"
                        font.pixelSize: 18
                        text: "Timeline Thumbnails"
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        color: "#9fb1bf"
                        text: playbackViewModel
                            ? `${playbackViewModel.timelineThumbnailPaths.length} item(s)`
                            : "0 item(s)"
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: playbackViewModel ? playbackViewModel.timelineThumbnailPaths : []

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 44
                        color: index % 2 === 0 ? "#17212a" : "#111922"

                        Label {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            verticalAlignment: Text.AlignVCenter
                            color: "#dce7ef"
                            elide: Text.ElideMiddle
                            text: modelData
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: playbackViewModel && playbackViewModel.timelineThumbnailPaths.length === 0
                    color: "#9fb1bf"
                    text: "No thumbnails have been generated for this master video yet."
                    wrapMode: Text.Wrap
                }
            }
        }
    ]

    onMasterVideoIdChanged: loadPlaybackRoute()

    Component.onCompleted: loadPlaybackRoute()
}
