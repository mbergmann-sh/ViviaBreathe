// Bereich „Erinnerungen“: wiederkehrende Übungsereignisse (z. B. Einschlafübung um 21:30)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Item {
    id: view
    objectName: "eventsView"
    property var list: []
    function reload() { list = app.events() }
    Component.onCompleted: reload()
    Connections { target: app; function onRevisionChanged() { view.reload() } function onUserChanged() { view.reload() } }
    Component { id: editComp; EventEditPage { onClosed: (msg) => { stack.pop(); if (msg) win.toast(msg) } } }

    ScrollPage {
        anchors.fill: parent
        ReminderHint {}
        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Ereignisse mit Erinnerung"); font.pixelSize: Theme.fsXl; font.bold: true; color: Theme.ink
                    Layout.fillWidth: true; wrapMode: Text.WordWrap }
            VButton { objectName: "addEvent"; kind: "primary"; iconName: "add"; text: win.phone ? "" : qsTr("Neu …")
                      onClicked: stack.push(editComp, { eventId: 0 }) }
        }
        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2; font.pixelSize: Theme.fsS
                text: qsTr("Zur gewählten Zeit erinnert eine Benachrichtigung an die Übung; antippen öffnet sie gleich mit dem richtigen Anlass.") }
        Label { visible: view.list.length === 0; text: qsTr("Noch keine Ereignisse – z. B. „Einschlafübung“ täglich um 21:30.")
                color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Repeater {
            model: view.list
            delegate: Card {
                clickable: true
                onClicked: stack.push(editComp, { eventId: modelData.id })
                opacity: modelData.enabled ? 1 : 0.6
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.gap
                    Label { text: modelData.time; font.pixelSize: Theme.fsXl; font.bold: true; color: Theme.brand }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Label { text: modelData.name; font.bold: true; color: Theme.ink; Layout.fillWidth: true; elide: Text.ElideRight }
                        Label { text: modelData.moduleName + " · " + modelData.weekdaysText; color: Theme.ink2; font.pixelSize: Theme.fsS
                                Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        Label { text: modelData.enabled ? modelData.next : qsTr("ausgeschaltet"); color: Theme.muted; font.pixelSize: Theme.fsXs }
                    }
                    VButton { iconName: "play"; text: win.phone ? "" : qsTr("Jetzt üben")
                              onClicked: win.openPrep(modelData.moduleId, modelData.name) }
                }
            }
        }
    }
}
