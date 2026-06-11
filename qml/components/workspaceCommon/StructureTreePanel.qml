import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../projectWorkspace"

// Tree navigation for project assets, components, and inspection items.

WorkspacePanel {
    id: root

    property var assets: []
    property string selectedKey: "project"
    signal projectSelected()
    signal assetSelected(var asset)
    signal componentSelected(var asset, var component)
    signal itemSelected(var asset, var component, var item)

    content: [
        Label {
            color: "#e6e8eb"
            font.bold: true
            font.pixelSize: 14
            text: "Structure"
        },

        Button {
            id: projectButton

            Layout.fillWidth: true
            text: "Project overview"
            onClicked: root.projectSelected()

            contentItem: Label {
                color: "#e6e8eb"
                font.pixelSize: 12
                text: projectButton.text
                elide: Text.ElideRight
            }

            background: Rectangle {
                radius: 4
                color: root.selectedKey === "project" ? "#3b2227" : "transparent"
                border.color: root.selectedKey === "project" ? "#c1272d" : "transparent"
                border.width: 1
            }
        },

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#303642"
        },

        Label {
            Layout.fillWidth: true
            visible: root.assets.length === 0
            color: "#78818f"
            font.pixelSize: 12
            text: "No assets created yet."
            wrapMode: Text.Wrap
        },

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2
            model: root.assets

            delegate: ColumnLayout {
                id: assetDelegate

                property var assetData: modelData

                width: ListView.view.width
                spacing: 2

                TreeNodeButton {
                    Layout.fillWidth: true
                    nodeText: assetDelegate.assetData.name
                    selected: root.selectedKey === "asset:" + assetDelegate.assetData.assetId
                    onClicked: root.assetSelected(assetDelegate.assetData)
                }

                Repeater {
                    model: assetDelegate.assetData.components

                    delegate: ColumnLayout {
                        id: componentDelegate

                        property var componentData: modelData

                        Layout.fillWidth: true
                        spacing: 2

                        TreeNodeButton {
                            Layout.fillWidth: true
                            nodeIndent: 14
                            nodeText: componentDelegate.componentData.name
                            selected: root.selectedKey === "component:" + componentDelegate.componentData.componentId
                            onClicked: root.componentSelected(
                                assetDelegate.assetData,
                                componentDelegate.componentData
                            )
                        }

                        Repeater {
                            model: componentDelegate.componentData.items

                            delegate: TreeNodeButton {
                                Layout.fillWidth: true
                                nodeIndent: 28
                                leaf: true
                                nodeText: modelData.itemLabel
                                selected: root.selectedKey === "item:" + modelData.itemId
                                onClicked: {
                                    root.itemSelected(
                                        assetDelegate.assetData,
                                        componentDelegate.componentData,
                                        modelData
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    ]

    component TreeNodeButton: Button {
        id: nodeButton

        property string nodeText: ""
        property bool selected: false
        property bool leaf: false
        property int nodeIndent: 0

        implicitHeight: 26
        text: nodeText
        hoverEnabled: true

        contentItem: Label {
            leftPadding: nodeButton.nodeIndent + 6
            rightPadding: 6
            color: nodeButton.selected ? "#e6e8eb" : (nodeButton.leaf ? "#a8b0bd" : "#c9ced6")
            font.pixelSize: 12
            text: nodeButton.text
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            radius: 4
            color: nodeButton.selected ? "#3b2227" : (nodeButton.hovered ? "#20242d" : "transparent")
            border.color: nodeButton.selected ? "#c1272d" : "transparent"
            border.width: 1
        }
    }
}
