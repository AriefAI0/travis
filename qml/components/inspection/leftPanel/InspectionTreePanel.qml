import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shows the inspection item tree used to select the active clipping target.

ColumnLayout {
    id: root

    property var inspectionContextViewModel

    spacing: 6

    Label {
        Layout.fillWidth: true
        color: "#78818f"
        font.bold: true
        font.pixelSize: 10
        text: "INSPECTION TREE"
    }

    Label {
        Layout.fillWidth: true
        visible: inspectionContextViewModel && inspectionContextViewModel.inspectionItems.length === 0
        color: "#78818f"
        text: inspectionContextViewModel && inspectionContextViewModel.loading
            ? "Loading structure..."
            : "No inspection items."
        wrapMode: Text.Wrap
    }

    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        spacing: 4
        model: inspectionContextViewModel ? inspectionContextViewModel.inspectionItems : []

        delegate: Button {
            width: ListView.view.width
            height: 30
            text: modelData.itemLabel
            onClicked: root.inspectionContextViewModel.selectItem(modelData.itemId)

            contentItem: ColumnLayout {
                spacing: 1

                Label {
                    Layout.fillWidth: true
                    color: modelData.itemId === root.inspectionContextViewModel.selectedItemId ? "#e6e8eb" : "#c9ced6"
                    font.pixelSize: 12
                    text: modelData.itemLabel
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    color: "#78818f"
                    font.pixelSize: 10
                    text: `${modelData.assetName} / ${modelData.componentName}`
                    elide: Text.ElideRight
                }
            }

            background: Rectangle {
                radius: 4
                color: modelData.itemId === root.inspectionContextViewModel.selectedItemId
                    ? "#3b2227"
                    : (parent.hovered ? "#20242d" : "transparent")
                border.color: modelData.itemId === root.inspectionContextViewModel.selectedItemId ? "#c1272d" : "transparent"
                border.width: 1
            }
        }
    }
}
