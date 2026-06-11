import QtQuick
import QtQuick.Layouts

// Provides fixed desktop pane placement so the preview owns the center area.

Item {
    id: root

    property string title: "Inspection Workspace"
    property string subtitle: ""
    property int headerHeight: 56
    property int leftPaneWidth: 240
    property int rightPaneWidth: 260
    property int bottomPaneHeight: 260
    property int separatorSize: 1
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

    Rectangle {
        id: headerPane

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.headerHeight
        color: "#141820"

        Item {
            anchors.fill: parent
            anchors.margins: 8

            ColumnLayout {
                id: headerNavigationSlot

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: root.leftPaneWidth - 16
                spacing: 2
            }

            ColumnLayout {
                anchors.left: headerNavigationSlot.right
                anchors.leftMargin: 8
                anchors.right: headerStatusSlot.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
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

            RowLayout {
                id: headerStatusSlot

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(520, Math.max(320, root.width * 0.34))
                spacing: 0
            }
        }
    }

    SeparatorLine {
        id: headerSeparator

        anchors.top: headerPane.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    PaneSlot {
        id: leftTopPane

        anchors.top: headerSeparator.bottom
        anchors.left: parent.left
        anchors.bottom: middleSeparator.top
        width: root.leftPaneWidth

        ColumnLayout {
            id: leftSidebarSlot

            anchors.fill: parent
            anchors.margins: 8
            spacing: 6
        }
    }

    VerticalSeparator {
        id: leftSeparator

        anchors.top: headerSeparator.bottom
        anchors.bottom: parent.bottom
        anchors.left: leftTopPane.right
    }

    Item {
        id: centerPane

        anchors.top: headerSeparator.bottom
        anchors.left: leftSeparator.right
        anchors.right: rightSeparator.left
        anchors.bottom: middleSeparator.top

        ColumnLayout {
            id: centerContentSlot

            anchors.fill: parent
            spacing: 0
        }
    }

    VerticalSeparator {
        id: rightSeparator

        anchors.top: headerSeparator.bottom
        anchors.bottom: middleSeparator.top
        anchors.right: rightPane.left
    }

    PaneSlot {
        id: rightPane

        anchors.top: headerSeparator.bottom
        anchors.right: parent.right
        anchors.bottom: middleSeparator.top
        width: root.rightPaneWidth

        ColumnLayout {
            id: rightSidebarSlot

            anchors.fill: parent
            anchors.margins: 8
            spacing: 6
        }
    }

    SeparatorLine {
        id: middleSeparator

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomPane.top
    }

    PaneSlot {
        id: leftBottomPane

        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: root.leftPaneWidth
        height: root.bottomPaneHeight

        ColumnLayout {
            id: leftBottomSlot

            anchors.fill: parent
            anchors.margins: 8
            spacing: 6
        }
    }

    PaneSlot {
        id: bottomPane

        anchors.left: leftSeparator.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: root.bottomPaneHeight

        ColumnLayout {
            id: bottomContentSlot

            anchors.fill: parent
            anchors.margins: 8
            spacing: 6
        }
    }

    component PaneSlot: Rectangle {
        radius: 0
        color: "#141820"
        clip: true
    }

    component SeparatorLine: Rectangle {
        height: root.separatorSize
        color: "#303642"
    }

    component VerticalSeparator: Rectangle {
        width: root.separatorSize
        color: "#303642"
    }
}
