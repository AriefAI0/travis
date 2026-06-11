import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../projectWorkspace"

// Tree navigation for project assets, components, and inspection items.

WorkspacePanel {
    id: root

    property var assets: []
    property var treeModel: null
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

        TreeView {
            id: structureTree

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.treeModel
            selectionModel: ItemSelectionModel {}

            delegate: TreeViewDelegate {
                id: treeDelegate

                required property string nodeLabel
                required property string type
                required property string key
                required property var asset
                required property var component
                required property var item

                implicitWidth: structureTree.width
                implicitHeight: 26
                indentation: 14

                contentItem: Label {
                    leftPadding: treeDelegate.depth * treeDelegate.indentation + 22
                    rightPadding: 6
                    color: treeDelegate.key === root.selectedKey
                        ? "#e6e8eb"
                        : treeDelegate.type === "item" ? "#a8b0bd" : "#c9ced6"
                    font.pixelSize: 12
                    text: treeDelegate.nodeLabel
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                indicator: Label {
                    x: treeDelegate.depth * treeDelegate.indentation + 6
                    width: 12
                    height: treeDelegate.height
                    visible: treeDelegate.hasChildren
                    color: "#9fb0be"
                    text: treeDelegate.expanded ? "v" : ">"
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: 4
                    color: treeDelegate.key === root.selectedKey
                        ? "#3b2227"
                        : treeDelegate.hovered ? "#20242d" : "transparent"
                    border.color: treeDelegate.key === root.selectedKey ? "#c1272d" : "transparent"
                    border.width: 1
                }

                onClicked: {
                    if (treeDelegate.type === "asset") {
                        root.assetSelected(treeDelegate.asset)
                    } else if (treeDelegate.type === "component") {
                        root.componentSelected(treeDelegate.asset, treeDelegate.component)
                    } else if (treeDelegate.type === "item") {
                        root.itemSelected(treeDelegate.asset, treeDelegate.component, treeDelegate.item)
                    }
                }
            }
        }
    ]
}
