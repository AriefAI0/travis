import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shows selected inspection item details and the current inspection type selector.

ColumnLayout {
    id: root

    property var inspectionContextViewModel
    property alias inspectionTypeText: inspectionTypeField.text

    spacing: 6

    Label {
        Layout.fillWidth: true
        color: "#78818f"
        font.bold: true
        font.pixelSize: 10
        text: "INSPECTION DETAILS"
    }

    Label {
        Layout.fillWidth: true
        color: "#e6e8eb"
        font.bold: true
        font.pixelSize: 14
        text: inspectionContextViewModel && inspectionContextViewModel.selectedItemId > 0
            ? inspectionContextViewModel.selectedItem.itemLabel
            : "Select an inspection item"
        wrapMode: Text.Wrap
    }

    Label {
        Layout.fillWidth: true
        color: "#a8b0bd"
        text: inspectionContextViewModel && inspectionContextViewModel.selectedItemId > 0
            ? `${inspectionContextViewModel.selectedItem.assetName} / ${inspectionContextViewModel.selectedItem.componentName}`
            : "Choose an item from the inspection tree to enable clipping."
        wrapMode: Text.Wrap
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: "#303642"
    }

    Label {
        Layout.fillWidth: true
        color: "#78818f"
        font.bold: true
        font.pixelSize: 10
        text: "INSPECTION TYPE"
    }

    TextField {
        id: inspectionTypeField

        Layout.fillWidth: true
        text: "GVI"
        placeholderText: "Type"
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        columnSpacing: 6
        rowSpacing: 6

        Repeater {
            model: ["GVI", "CVI", "MGI", "FMD"]

            Button {
                Layout.fillWidth: true
                implicitHeight: 24
                text: modelData
                enabled: root.inspectionContextViewModel && root.inspectionContextViewModel.selectedItemId > 0
                onClicked: inspectionTypeField.text = modelData
            }
        }
    }

    Label {
        Layout.fillWidth: true
        color: "#78818f"
        text: "Detailed Electron task forms will be migrated into this panel next."
        wrapMode: Text.Wrap
    }
}
