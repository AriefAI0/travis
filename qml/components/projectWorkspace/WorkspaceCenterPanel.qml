import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../projects"

// Center workspace details and create forms for the active structure selection.

WorkspacePanel {
    id: root

    property var project: ({})
    property var assets: []
    property var selectedAsset: null
    property var selectedComponent: null
    property var selectedItem: null
    property string selectedType: "project"
    property int assetCount: 0
    property int componentCount: 0
    property int itemCount: 0
    property bool loading: false
    property string errorMessage: ""
    property var recordings: []

    signal createAssetRequested(string name)
    signal createComponentRequested(int assetId, string name)
    signal createItemRequested(int componentId, string label, string position, int status)
    signal assetSelected(var asset)
    signal componentSelected(var asset, var component)
    signal itemSelected(var asset, var component, var item)
    signal openPlaybackRequested(int masterVideoId)

    function clearAssetForm() {
        assetNameField.text = ""
    }

    function clearComponentForm() {
        componentNameField.text = ""
    }

    function clearItemForm() {
        itemLabelField.text = ""
        itemPositionField.text = ""
        itemStatusField.value = 0
    }

    content: [
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Label {
                color: "#78818f"
                font.bold: true
                font.pixelSize: 11
                text: root.selectedType.toUpperCase() + " VIEW"
            }

            Label {
                Layout.fillWidth: true
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 18
                text: {
                    if (root.selectedType === "asset" && root.selectedAsset !== null) {
                        return root.selectedAsset.name
                    }
                    if (root.selectedType === "component" && root.selectedComponent !== null) {
                        return root.selectedComponent.name
                    }
                    if (root.selectedType === "item" && root.selectedItem !== null) {
                        return root.selectedItem.itemLabel
                    }
                    return root.project.title || "Project overview"
                }
                elide: Text.ElideRight
            }
        },

        StatusBanner {
            Layout.fillWidth: true
            message: root.errorMessage
            error: true
        },

        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selectedType === "project"
            spacing: 12

            DetailCard {
                Layout.fillWidth: true
                title: "Project details"

                GridLayout {
                    Layout.fillWidth: true
                    columns: 4
                    rowSpacing: 8
                    columnSpacing: 8

                    StatBox { label: "Document ID"; value: root.project.documentId || "-" }
                    StatBox { label: "Assets"; value: String(root.assetCount) }
                    StatBox { label: "Components"; value: String(root.componentCount) }
                    StatBox { label: "Items"; value: String(root.itemCount) }
                }
            }

            DetailCard {
                Layout.fillWidth: true
                title: "Assets"

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: assetNameField

                        Layout.fillWidth: true
                        placeholderText: "Asset name"
                        enabled: !root.loading
                        onAccepted: addAssetButton.clicked()
                    }

                    ProjectActionButton {
                        id: addAssetButton

                        text: root.loading ? "Adding..." : "Add asset"
                        enabled: !root.loading && assetNameField.text.trim().length > 0
                        onClicked: root.createAssetRequested(assetNameField.text)
                    }
                }

                StructureTable {
                    Layout.fillWidth: true
                    rows: root.assets
                    emptyText: "No assets yet."
                    firstHeader: "Name"
                    secondHeader: "Components"
                    thirdHeader: "Items"
                    firstValue: function(row) { return row.name }
                    secondValue: function(row) { return String(row.components.length) }
                    thirdValue: function(row) {
                        let total = 0
                        for (let i = 0; i < row.components.length; i++) {
                            total += row.components[i].items.length
                        }
                        return String(total)
                    }
                    onRowSelected: function(row) { root.assetSelected(row) }
                }
            }

            DetailCard {
                Layout.fillWidth: true
                title: "Review workspace"
                meta: root.recordings.length + " recording(s)"

                Label {
                    Layout.fillWidth: true
                    visible: root.recordings.length === 0
                    color: "#78818f"
                    font.pixelSize: 12
                    text: "No finalized master videos are available for review yet."
                    wrapMode: Text.Wrap
                }

                Repeater {
                    model: root.recordings

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: recordingRow.implicitHeight + 16
                        radius: 6
                        color: "#20242d"
                        border.color: "#303642"

                        RowLayout {
                            id: recordingRow

                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    Layout.fillWidth: true
                                    color: "#e6e8eb"
                                    font.pixelSize: 12
                                    text: modelData.sourceName || ("Master Video #" + modelData.masterVideoId)
                                    elide: Text.ElideRight
                                }

                                Label {
                                    Layout.fillWidth: true
                                    color: "#78818f"
                                    font.pixelSize: 11
                                    text: modelData.fileUrl
                                    elide: Text.ElideMiddle
                                }
                            }

                            ProjectActionButton {
                                text: "Open review"
                                enabled: modelData.masterVideoId > 0 && modelData.status === "finalized"
                                onClicked: root.openPlaybackRequested(modelData.masterVideoId)
                            }
                        }
                    }
                }
            }
        },

        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selectedType === "asset" && root.selectedAsset !== null
            spacing: 12

            DetailCard {
                Layout.fillWidth: true
                title: "Components"
                meta: root.selectedAsset ? root.selectedAsset.components.length + " component(s)" : ""

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: componentNameField

                        Layout.fillWidth: true
                        placeholderText: "Component name"
                        enabled: !root.loading
                        onAccepted: addComponentButton.clicked()
                    }

                    ProjectActionButton {
                        id: addComponentButton

                        text: root.loading ? "Adding..." : "Add component"
                        enabled: !root.loading &&
                            root.selectedAsset !== null &&
                            componentNameField.text.trim().length > 0
                        onClicked: root.createComponentRequested(
                            root.selectedAsset.assetId,
                            componentNameField.text
                        )
                    }
                }

                StructureTable {
                    Layout.fillWidth: true
                    rows: root.selectedAsset ? root.selectedAsset.components : []
                    emptyText: "No components yet."
                    firstHeader: "Name"
                    secondHeader: "Items"
                    thirdHeader: "Asset"
                    firstValue: function(row) { return row.name }
                    secondValue: function(row) { return String(row.items.length) }
                    thirdValue: function() { return root.selectedAsset ? root.selectedAsset.name : "-" }
                    onRowSelected: function(row) { root.componentSelected(root.selectedAsset, row) }
                }
            }
        },

        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selectedType === "component" && root.selectedComponent !== null
            spacing: 12

            DetailCard {
                Layout.fillWidth: true
                title: "Items"
                meta: root.selectedComponent ? root.selectedComponent.items.length + " item(s)" : ""

                GridLayout {
                    Layout.fillWidth: true
                    columns: 4
                    rowSpacing: 8
                    columnSpacing: 8

                    TextField {
                        id: itemLabelField

                        Layout.fillWidth: true
                        placeholderText: "Item label"
                        enabled: !root.loading
                    }

                    TextField {
                        id: itemPositionField

                        Layout.fillWidth: true
                        placeholderText: "Position"
                        enabled: !root.loading
                    }

                    SpinBox {
                        id: itemStatusField

                        Layout.preferredWidth: 96
                        from: 0
                        to: 100
                        value: 0
                        enabled: !root.loading
                    }

                    ProjectActionButton {
                        text: root.loading ? "Adding..." : "Add item"
                        enabled: !root.loading &&
                            root.selectedComponent !== null &&
                            itemLabelField.text.trim().length > 0
                        onClicked: root.createItemRequested(
                            root.selectedComponent.componentId,
                            itemLabelField.text,
                            itemPositionField.text,
                            itemStatusField.value
                        )
                    }
                }

                StructureTable {
                    Layout.fillWidth: true
                    rows: root.selectedComponent ? root.selectedComponent.items : []
                    emptyText: "No items yet."
                    firstHeader: "Label"
                    secondHeader: "Position"
                    thirdHeader: "Status"
                    firstValue: function(row) { return row.itemLabel }
                    secondValue: function(row) { return row.position || "-" }
                    thirdValue: function(row) { return row.status === undefined ? "-" : String(row.status) }
                    onRowSelected: function(row) {
                        root.itemSelected(root.selectedAsset, root.selectedComponent, row)
                    }
                }
            }
        },

        DetailCard {
            Layout.fillWidth: true
            visible: root.selectedType === "item" && root.selectedItem !== null
            title: "Item details"

            GridLayout {
                Layout.fillWidth: true
                columns: 3
                rowSpacing: 8
                columnSpacing: 8

                StatBox { label: "Asset"; value: root.selectedAsset ? root.selectedAsset.name : "-" }
                StatBox { label: "Component"; value: root.selectedComponent ? root.selectedComponent.name : "-" }
                StatBox { label: "Position"; value: root.selectedItem ? (root.selectedItem.position || "-") : "-" }
                StatBox { label: "Status"; value: root.selectedItem !== null && root.selectedItem.status !== undefined ? String(root.selectedItem.status) : "-" }
            }
        }
    ]

    component DetailCard: Rectangle {
        id: card

        property string title: ""
        property string meta: ""
        default property alias content: cardContent.data

        implicitHeight: cardColumn.implicitHeight + 24
        radius: 8
        color: "#20242d"
        border.color: "#303642"
        border.width: 1

        ColumnLayout {
            id: cardColumn

            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    color: "#e6e8eb"
                    font.bold: true
                    font.pixelSize: 14
                    text: card.title
                }

                Label {
                    visible: card.meta.length > 0
                    color: "#78818f"
                    font.pixelSize: 11
                    text: card.meta
                }
            }

            ColumnLayout {
                id: cardContent

                Layout.fillWidth: true
                spacing: 8
            }
        }
    }

    component StatBox: Rectangle {
        property string label: ""
        property string value: ""

        Layout.fillWidth: true
        implicitHeight: 58
        radius: 6
        color: "#252a33"
        border.color: "#303642"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 3

            Label {
                color: "#78818f"
                font.pixelSize: 10
                text: label
            }

            Label {
                Layout.fillWidth: true
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 13
                text: value
                elide: Text.ElideRight
            }
        }
    }

    component StructureTable: ColumnLayout {
        id: table

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

            TableCell { text: table.firstHeader; header: true }
            TableCell { text: table.secondHeader; header: true }
            TableCell { text: table.thirdHeader; header: true }
        }

        Label {
            Layout.fillWidth: true
            visible: table.rows.length === 0
            topPadding: 10
            color: "#78818f"
            font.pixelSize: 12
            text: table.emptyText
        }

        Repeater {
            model: table.rows

            delegate: Button {
                Layout.fillWidth: true
                implicitHeight: 32
                flat: true
                onClicked: table.rowSelected(modelData)

                contentItem: RowLayout {
                    spacing: 0
                    TableCell { text: table.firstValue(modelData) }
                    TableCell { text: table.secondValue(modelData) }
                    TableCell { text: table.thirdValue(modelData) }
                }

                background: Rectangle {
                    color: parent.hovered ? "#252a33" : "transparent"
                    border.color: "#303642"
                    border.width: 1
                }
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
