// Einschlaf-Eintrag anlegen oder ändern: Musik, Dauer, Ausblenden, Übung vorher, Planung wie beim Wecker
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "sleepEditPage"
    property var planId: 0
    property var p: planId ? app.sleepPlan(planId) : ({ hour: 22, minute: 30, weekdays: 127, volume: 50, minutes: 30, fadeOutMin: 5, playlist: [] })
    property var mods: [{ id: "", name: qsTr("keine") }].concat(app.modules())
    property int weekdays: p.weekdays || 127
    property int minutes: p.minutes || 30
    property int fadeOut: p.fadeOutMin === undefined ? 5 : p.fadeOutMin
    signal closed(string msg)
    background: Rectangle { color: Theme.paper }

    function save() {
        const m = { id: page.planId, label: label.text, hour: time.hour, minute: time.minute, weekdays: page.weekdays,
                    scheduled: sched.checked, autoStart: auto.checked, playlist: list.files, shuffle: shuffle.checked,
                    volume: Math.round(vol.value), minutes: page.minutes, fadeOutMin: Math.min(page.fadeOut, page.minutes),
                    moduleId: page.mods[mod.currentIndex].id }
        if (app.saveSleepPlan(m)) page.closed(qsTr("Gespeichert.")); else win.toast(app.lastError, true)
    }
    Confirm { id: delQuestion; title: qsTr("Einschlaf-Eintrag löschen?"); acceptText: qsTr("Löschen"); danger: true
              onAccepted: { if (app.deleteSleepPlan(page.planId)) page.closed(qsTr("Gelöscht.")) } }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader {
            title: page.planId ? qsTr("Einschlafen ändern") : qsTr("Neuer Einschlaf-Eintrag")
            onBack: page.closed("")
            VButton { visible: !!page.planId; kind: "flat"; iconName: "delete"; text: win.phone ? "" : qsTr("Löschen"); onClicked: delQuestion.open() }
        }
        ScrollPage {
            maxWidth: 680
            Card {
                VField { id: label; objectName: "sleepLabel"; label: qsTr("Bezeichnung"); text: page.p.label || ""
                         placeholder: qsTr("z. B. Wochentags"); Layout.fillWidth: true }
                VCombo { id: mod; label: qsTr("Vorher eine Übung"); Layout.fillWidth: true; model: page.mods; textRole: "name"
                         currentIndex: Math.max(0, page.mods.findIndex(m => m.id === (page.p.moduleId || ""))) }
            }
            Card {
                SectionTitle { text: qsTr("Musik") }
                PlaylistEditor { id: list; Layout.fillWidth: true; files: page.p.playlist || [] }
                VSwitch { id: shuffle; text: qsTr("Zufällige Reihenfolge"); checked: !!page.p.shuffle }
                VSlider { id: vol; label: qsTr("Lautstärke"); Layout.fillWidth: true; from: 5; value: page.p.volume || 50 }
                VStepper { label: qsTr("Spieldauer"); from: 5; to: 240; step: 5; suffix: qsTr(" min"); value: page.minutes
                           onEdited: (v) => page.minutes = v }
                VStepper { label: qsTr("Zum Ende leiser werden über"); from: 0; to: Math.min(60, page.minutes); suffix: qsTr(" min"); value: page.fadeOut
                           onEdited: (v) => page.fadeOut = v }
            }
            Card {
                SectionTitle { text: qsTr("Planen") }
                VSwitch { id: sched; text: qsTr("Zu einer Uhrzeit anbieten"); sub: qsTr("Wie beim Wecker: an den gewählten Tagen erinnert eine Benachrichtigung ans Einschlafen.")
                          checked: !!page.p.scheduled }
                TimePicker { id: time; visible: sched.checked; hour: page.p.hour; minute: page.p.minute }
                WeekdayPicker { visible: sched.checked; mask: page.weekdays; onMaskChanged: page.weekdays = mask }
                VSwitch { id: auto; visible: sched.checked; text: qsTr("Musik automatisch starten")
                          sub: qsTr("Ohne Rückfrage, auch wenn die App geschlossen ist (nur ohne Übung vorher).")
                          checked: !!page.p.autoStart }
            }
            VButton { objectName: "saveSleep"; kind: "primary"; text: qsTr("Speichern"); Layout.alignment: Qt.AlignRight; onClicked: page.save() }
        }
    }
}
