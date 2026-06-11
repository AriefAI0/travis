import QtQuick
import QtQuick.Layouts

// Provides the compact inspection workspace frame and slot geometry.

Item {
    id: root

    property string title: "Inspection Workspace"
    property string subtitle: ""
    property alias headerNavigation: headerNavigationSlot.data
    property alias headerStatus: headerStatusSlot.data
    property alias leftSidebar: leftSidebarSlot.data
    property alias leftBottom: leftBottomSlot.data
    property alias centerContent: centerContentSlot.data
    property alias rightSidebar: rightSidebarSlot.data
    property alias bottomContent: bottomContentSlot.data

    Rectangle {
        anchors.fill: parent
        color: "#0f1117"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            radius: 0
            color: "#141820"

            GridLayout {
                anchors.fill: parent
                anchors.margins: 8
                columns: 3
                columnSpacing: 8
                rowSpacing: 0

                ColumnLayout {
                    id: headerNavigationSlot

                    Layout.minimumWidth: 180
                    Layout.preferredWidth: 220
                    Layout.maximumWidth: 260
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 2
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 2

                    Text {
                        Layout.fillWidth: true
                        text: root.title
                        font.bold: true
                        font.pixelSize: 14
                        color: "#e6e8eb"
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.subtitle
                        font.pixelSize: 10
                        color: "#78818f"
                        elide: Text.ElideRight
                    }
                }

                ColumnLayout {
                    id: headerStatusSlot

                    Layout.minimumWidth: 300
                    Layout.preferredWidth: 420
                    Layout.maximumWidth: 520
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 0
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#303642"
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 5
            columnSpacing: 0
            rowSpacing: 0

            PaneSlot {
                Layout.fillHeight: true
                Layout.minimumWidth: 200
                Layout.preferredWidth: 240
                Layout.maximumWidth: 280
                Layout.rowSpan: 1

                ColumnLayout {
                    id: leftSidebarSlot

                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                }
            }

            VerticalSeparator {
                Layout.fillHeight: true
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    id: centerContentSlot

                    anchors.fill: parent
                    spacing: 0
                }
            }

            VerticalSeparator {
                Layout.fillHeight: true
            }

            PaneSlot {
                Layout.fillHeight: true
                Layout.minimumWidth: 220
                Layout.preferredWidth: 260
                Layout.maximumWidth: 300

                ColumnLayout {
                    id: rightSidebarSlot

                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                }
            }

            HorizontalSeparator {
                Layout.columnSpan: 5
                Layout.fillWidth: true
            }

            PaneSlot {
                Layout.minimumWidth: 200
                Layout.preferredWidth: 240
                Layout.maximumWidth: 280
                Layout.preferredHeight: 190
                Layout.minimumHeight: 160
                Layout.maximumHeight: 210

                ColumnLayout {
                    id: leftBottomSlot

                    anchors.fill: parent
                    spacing: 0
                }
            }

            VerticalSeparator {
                Layout.fillHeight: true
            }

            PaneSlot {
                Layout.fillWidth: true
                Layout.columnSpan: 3
                Layout.preferredHeight: 190
                Layout.minimumHeight: 160
                Layout.maximumHeight: 210

                ColumnLayout {
                    id: bottomContentSlot

                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                }
            }
        }
    }

    component PaneSlot: Rectangle {
        radius: 0
        color: "#141820"
        clip: true
    }

    component VerticalSeparator: Rectangle {
        Layout.preferredWidth: 1
        color: "#303642"
    }

    component HorizontalSeparator: Rectangle {
        Layout.preferredHeight: 1
        color: "#303642"
    }
}
