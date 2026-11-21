// Ereignis mit Erinnerung anlegen oder ändern
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "eventEditPage"
    property var eventId: 0
    property var e: eventId ? (app.events().find(x => x.id === eventId) || ({})) : ({ enabled: true, sound: true, hour: 21, minute: 30, weekdays: 127 })
    property var mods: app.modules()
    property int weekdays: e.weekdays || 127
    signal closed(string msg)
    background: Rectangle { color: Theme.paper }

    function save() {
        const m = { id: page.eventId, name: name.text, moduleId: page.mods[mod.currentIndex].id, hour: time.hour, minute: time.minute,
                    weekdays: page.weekdays, enabled: enabled_.checked, sound: sound.checked, note: note.text }
        if (app.saveEvent(m)) page.closed(qsTr("Gespeichert.")); else win.toast(app.lastError, true)
    }
    Confirm { id: delQuestion; title: qsTr("Ereignis löschen?"); acceptText: qsTr("Löschen"); danger: true
              onAccepted: { if (app.deleteEvent(page.eventId)) page.closed(qsTr("Gelöscht.")) } }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader {
            title: page.eventId ? qsTr("Ereignis ändern") : qsTr("Neues Ereignis")
            onBack: page.closed("")
            VButton { visible: !!page.eventId; kind: "flat"; iconName: "delete"; text: win.phone ? "" : qsTr("Löschen"); onClicked: delQuestion.open() }
        }
        ScrollPage {
            maxWidth: 680
            Card {
                VField { id: name; objectName: "eventName"; label: qsTr("Bezeichnung"); text: page.e.name || ""
                         placeholder: qsTr("z. B. Einschlafübung"); Layout.fillWidth: true }
                Flow {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: implicitHeight
                    spacing: 6
                    Repeater {
                        model: [qsTr("Einschlafübung"), qsTr("Stressbewältigung"), qsTr("Atemtraining"), qsTr("Konzentration")]
                        VChip { text: modelData; checkable: false; font.pixelSize: Theme.fsS; onClicked: name.text = modelData }
                    }
                }
                VCombo { id: mod; label: qsTr("Übung"); Layout.fillWidth: true; model: page.mods; textRole: "name"
                         currentIndex: Math.max(0, page.mods.findIndex(m => m.id === page.e.moduleId)) }
            }
            Card {
                TimePicker { id: time; hour: page.e.hour; minute: page.e.minute }
                WeekdayPicker { mask: page.weekdays; onMaskChanged: page.weekdays = mask }
            }
            Card {
                VSwitch { id: enabled_; text: qsTr("Erinnerung eingeschaltet"); checked: page.e.enabled !== false }
                VSwitch { id: sound; text: qsTr("mit Ton"); checked: page.e.sound !== false }
                VField { id: note; label: qsTr("Notiz"); text: page.e.note || ""; placeholder: qsTr("z. B. im Bett, Licht aus"); Layout.fillWidth: true }
            }
            VButton { objectName: "saveEvent"; kind: "primary"; text: qsTr("Speichern"); Layout.alignment: Qt.AlignRight; onClicked: page.save() }
        }
    }
}
