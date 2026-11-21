// Bebilderte Schritt-für-Schritt-Anleitung einer Übung (z. B. Atemtrainer TriBall)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "guidePage"
    property string moduleId: ""
    property var info: app.moduleInfo(moduleId)
    signal closed()
    background: Rectangle { color: Theme.paper }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader { title: qsTr("Anleitung – %1").arg(page.info.shortName || ""); onBack: page.closed() }
        ScrollPage {
            id: scroll
            maxWidth: 1100
            Label { visible: (page.info.posture || "") !== ""; text: qsTr("Haltung: %1").arg(page.info.posture || "")
                    wrapMode: Text.WordWrap; Layout.fillWidth: true; color: Theme.ink2 }
            GridLayout {
                Layout.fillWidth: true
                columns: scroll.width > 900 ? 3 : scroll.width > 560 ? 2 : 1
                columnSpacing: Theme.gap
                rowSpacing: Theme.gap
                Repeater {
                    model: page.info.guide || []
                    delegate: Card {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 300
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: true
                            Layout.preferredHeight: width * 0.75
                            source: Theme.illustration(modelData.image)
                            sourceSize.width: 600
                            fillMode: Image.PreserveAspectFit
                        }
                        Label { text: modelData.text; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: Theme.ink }
                    }
                }
            }
            Repeater {
                model: page.info.tips || []
                delegate: RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label { text: "•"; color: Theme.brand; Layout.alignment: Qt.AlignTop }
                    Label { text: modelData; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: Theme.ink2 }
                }
            }
        }
    }
}
