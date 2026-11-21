// Bereich „Übungen“: Überblick (heute, 7 Tage, Serie) und die Übungen als Karten, nach Gruppen
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Item {
    id: view
    objectName: "exercisesView"
    property var mods: []
    property var ov: ({})
    function reload() { mods = app.modules(); ov = app.overview() }
    Component.onCompleted: reload()
    Connections { target: app; function onRevisionChanged() { view.reload() } function onUserChanged() { view.reload() }
                  function onLanguageChanged() { view.reload() } }

    ScrollPage {
        anchors.fill: parent
        // Überblick
        Card {
            objectName: "overviewCard"
            Label { text: qsTr("Hallo %1!").arg(app.userName); font.pixelSize: Theme.fsXl; font.bold: true; color: Theme.ink
                    Layout.fillWidth: true; wrapMode: Text.WordWrap }
            Flow {
                Layout.fillWidth: true
                Layout.preferredWidth: 200
                Layout.preferredHeight: implicitHeight
                spacing: Theme.gap
                Repeater {
                    model: [
                        { v: String(view.ov.todayCount || 0), t: qsTr("Übungen heute") },
                        { v: String(view.ov.weekMinutes || 0), t: qsTr("Minuten in 7 Tagen") },
                        { v: String(view.ov.streak || 0), t: qsTr("Tage in Folge") }
                    ]
                    delegate: ColumnLayout {
                        spacing: 0
                        Label { text: modelData.v; font.pixelSize: Theme.fsXxl; font.bold: true; color: Theme.brand }
                        Label { text: modelData.t; font.pixelSize: Theme.fsS; color: Theme.ink2 }
                    }
                }
            }
            Label { visible: !!view.ov.next; text: qsTr("Nächste Erinnerung: %1").arg(view.ov.next || ""); color: Theme.ink2
                    wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }

        Repeater {
            model: view.mods
            delegate: ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.gapS
                readonly property bool newGroup: index === 0 || view.mods[index - 1].category !== modelData.category
                SectionTitle { visible: parent.newGroup; text: modelData.category; Layout.topMargin: index === 0 ? 0 : Theme.gapS }
                Card {
                    objectName: "card_" + modelData.id
                    tint: modelData.quickHelp ? Theme.alert : modelData.color
                    color: modelData.quickHelp ? Theme.alertWash : Theme.surface
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gap
                        Image { source: Theme.icon(modelData.icon, Theme.ink); sourceSize.width: win.phone ? 44 : 56; sourceSize.height: win.phone ? 44 : 56
                                Layout.alignment: Qt.AlignTop }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Label { text: modelData.name; font.pixelSize: Theme.fsL; font.bold: true; color: Theme.ink
                                    wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Label { text: modelData.description; color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true
                                    maximumLineCount: win.phone ? 3 : 4; elide: Text.ElideRight; font.pixelSize: Theme.fsS }
                            Label { text: qsTr("Ihre Einstellung: %1").arg(modelData.settingsText); color: Theme.muted; font.pixelSize: Theme.fsXs
                                    wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Label { visible: modelData.lastText !== ""; text: qsTr("Zuletzt geübt: %1").arg(modelData.lastText)
                                    color: Theme.muted; font.pixelSize: Theme.fsXs; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        }
                    }
                    Flow {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 200
                        Layout.preferredHeight: implicitHeight
                        spacing: Theme.gapS
                        layoutDirection: Qt.RightToLeft
                        VButton {
                            objectName: "start_" + modelData.id
                            kind: modelData.quickHelp ? "danger" : "primary"
                            iconName: "play"
                            text: modelData.quickHelp ? qsTr("Sofort starten") : qsTr("Üben …")
                            onClicked: modelData.quickHelp ? win.quickHelp() : win.openPrep(modelData.id)
                        }
                        VButton { visible: modelData.hasGuide; iconName: "help"; text: qsTr("Anleitung")
                                  onClicked: win.openGuide(modelData.id) }
                        VButton { visible: modelData.quickHelp; iconName: "settings"; text: qsTr("Einstellen …")
                                  onClicked: win.openPrep(modelData.id) }
                    }
                }
            }
        }
    }
}
