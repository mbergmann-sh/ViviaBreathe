// Wecker anlegen oder ändern: Uhrzeit, Wochentage, Musik, Lautstärke, sanftes Lauterwerden, Schlummern
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "alarmEditPage"
    property var alarmId: 0
    property var a: alarmId ? app.alarm(alarmId) : ({ enabled: true, hour: 7, minute: 0, weekdays: 31, volume: 70, fadeInMin: 2, snoozeMin: 9, playlist: [] })
    property var mods: [{ id: "", name: qsTr("keine") }].concat(app.modules())
    property int weekdays: a.weekdays || 31
    property int fadeIn: a.fadeInMin === undefined ? 2 : a.fadeInMin
    property int snooze: a.snoozeMin || 9
    signal closed(string msg)
    background: Rectangle { color: Theme.paper }

    function values() {
        return { id: page.alarmId, label: label.text, hour: time.hour, minute: time.minute, weekdays: page.weekdays,
                 enabled: on.checked, playlist: list.files, shuffle: shuffle.checked, volume: Math.round(vol.value),
                 fadeInMin: page.fadeIn, snoozeMin: page.snooze, moduleId: page.mods[mod.currentIndex].id }
    }
    function save() { if (app.saveAlarm(values())) page.closed(qsTr("Wecker gespeichert.")); else win.toast(app.lastError, true) }
    Confirm { id: delQuestion; title: qsTr("Wecker löschen?"); acceptText: qsTr("Löschen"); danger: true
              onAccepted: { if (app.deleteAlarm(page.alarmId)) page.closed(qsTr("Gelöscht.")) } }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader {
            title: page.alarmId ? qsTr("Wecker ändern") : qsTr("Neuer Wecker")
            onBack: page.closed("")
            VButton { visible: !!page.alarmId; kind: "flat"; iconName: "delete"; text: win.phone ? "" : qsTr("Löschen"); onClicked: delQuestion.open() }
        }
        ScrollPage {
            maxWidth: 680
            Card {
                VSwitch { id: on; text: qsTr("Wecker eingeschaltet"); checked: page.a.enabled !== false }
                VField { id: label; label: qsTr("Bezeichnung"); text: page.a.label || ""; placeholder: qsTr("z. B. Arbeitstag"); Layout.fillWidth: true }
                TimePicker { id: time; label: qsTr("Weckzeit"); hour: page.a.hour; minute: page.a.minute }
                WeekdayPicker { mask: page.weekdays; onMaskChanged: page.weekdays = mask }
            }
            Card {
                SectionTitle { text: qsTr("Musik") }
                PlaylistEditor { id: list; Layout.fillWidth: true; files: page.a.playlist || []
                                 emptyText: qsTr("Ohne eigene Musik weckt der eingebaute Weckton.") }
                VSwitch { id: shuffle; text: qsTr("Zufällige Reihenfolge"); checked: !!page.a.shuffle }
                VSlider { id: vol; label: qsTr("Lautstärke"); Layout.fillWidth: true; from: 5; value: page.a.volume || 70 }
                VStepper { label: qsTr("Sanft lauter werden über"); from: 0; to: 30; suffix: qsTr(" min"); value: page.fadeIn
                           onEdited: (v) => page.fadeIn = v }
                VStepper { label: qsTr("Schlummern"); from: 1; to: 60; suffix: qsTr(" min"); value: page.snooze
                           onEdited: (v) => page.snooze = v }
                VButton { iconName: "play"; text: qsTr("Probehören (20 s)"); onClicked: app.testAlarm(page.values()) }
            }
            Card {
                VCombo { id: mod; label: qsTr("Nach dem Aufwachen eine Übung anbieten"); Layout.fillWidth: true
                         model: page.mods; textRole: "name"
                         currentIndex: Math.max(0, page.mods.findIndex(m => m.id === (page.a.moduleId || ""))) }
            }
            VButton { objectName: "saveAlarm"; kind: "primary"; text: qsTr("Speichern"); Layout.alignment: Qt.AlignRight; onClicked: page.save() }
        }
    }
}
