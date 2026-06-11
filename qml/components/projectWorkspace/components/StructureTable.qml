import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Three-column structure table used for assets, components, and items.

ColumnLayout {
    id: root

    property var rows: []
    property string emptyText: ""
    property string firstHeader: ""
    property string secondHeader: ""
    property string thirdHeader: ""
    property var firstValue
    property var secondValue
    property var thirdValue

    signal rowSelected(var row)

    spacing: 0

    RowLayout {
        Layout.fillWidth: true
        spacing: 0

        TableCell { text: root.firstHeader; header: true }
        TableCell { text: root.secondHeader; header: true }
        TableCell { text: root.thirdHeader; header: true }
    }

    Label {
        Layout.fillWidth: true
        visible: root.rows.length === 0
        topPadding: 10
        color: "#78818f"
        font.pixelSize: 12
        text: root.emptyText
    }

    Repeater {
        model: root.rows

        delegate: Button {
            Layout.fillWidth: true
            implicitHeight: 32
            flat: true
            onClicked: root.rowSelected(modelData)

            contentItem: RowLayout {
                spacing: 0

                TableCell { text: root.firstValue(modelData) }
                TableCell { text: root.secondValue(modelData) }
                TableCell { text: root.thirdValue(modelData) }
            }

            background: Rectangle {
                color: parent.hovered ? "#252a33" : "transparent"
                border.color: "#303642"
                border.width: 1
            }
        }
    }

    component TableCell: Label {
        property bool header: false

        Layout.fillWidth: true
        leftPadding: 8
        rightPadding: 8
        color: header ? "#78818f" : "#c9ced6"
        font.bold: header
        font.pixelSize: header ? 10 : 12
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
