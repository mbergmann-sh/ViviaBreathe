// Tagebuch-Eintrag ansehen und ändern (Anlass, Messwerte, Atemtrainer, Notiz) oder löschen
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "sessionEditPage"
    property var sessionId: 0
    property var e: app.session(sessionId)
    property int feelingBefore: e.feelingBefore === undefined ? -1 : e.feelingBefore
    property int feelingAfter: e.feelingAfter === undefined ? -1 : e.feelingAfter
    property int balls: e.deviceBalls === undefined ? -1 : e.deviceBalls
    signal closed(string msg)
    background: Rectangle { color: Theme.paper }

    function save() {
        const m = { id: page.sessionId, eventName: ev.text, note: note.text,
                    spo2Before: s1.text, spo2After: s2.text, pfBefore: p1.text, pfAfter: p2.text,
                    pulseBefore: u1.text, pulseAfter: u2.text, feelingBefore: page.feelingBefore, feelingAfter: page.feelingAfter }
        if (page.e.deviceTraining) { m.deviceBalls = page.balls; m.deviceHold = hold.text }
        if (app.updateSession(m)) page.closed(qsTr("Gespeichert.")); else win.toast(app.lastError, true)
    }
    Confirm {
        id: delQuestion
        title: qsTr("Eintrag löschen?")
        message: qsTr("Der Eintrag wird aus dem Tagebuch entfernt (auch auf abgeglichenen Geräten).")
        acceptText: qsTr("Löschen"); danger: true
        onAccepted: { if (app.deleteSession(page.sessionId)) page.closed(qsTr("Gelöscht.")); else win.toast(app.lastError, true) }
    }

    component Pair: RowLayout {
        property alias label: lbl.text
        property alias a: fa
        property alias b: fb
        Layout.fillWidth: true
        spacing: Theme.gapS
        Label { id: lbl; color: Theme.ink2; Layout.preferredWidth: win.phone ? 90 : 160; wrapMode: Text.WordWrap }
        VField { id: fa; placeholder: qsTr("vorher"); inputMethodHints: Qt.ImhDigitsOnly; Layout.fillWidth: true }
        VField { id: fb; placeholder: qsTr("nachher"); inputMethodHints: Qt.ImhDigitsOnly; Layout.fillWidth: true }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader {
            title: page.e.module || ""
            subtitle: page.e.when || ""
            onBack: page.closed("")
            VButton { kind: "flat"; iconName: "delete"; text: win.phone ? "" : qsTr("Löschen"); onClicked: delQuestion.open() }
        }
        ScrollPage {
            maxWidth: 680
            Label { text: page.e.summary || ""; color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Card {
                VField { id: ev; label: qsTr("Anlass"); text: page.e.eventName || ""; Layout.fillWidth: true }
            }
            Card {
                SectionTitle { text: qsTr("Messwerte") }
                Pair { id: spo; label: qsTr("SpO₂ (%)"); a.text: page.e.spo2Before || ""; b.text: page.e.spo2After || "" }
                Pair { id: pfp; label: qsTr("Peak Flow"); a.text: page.e.pfBefore || ""; b.text: page.e.pfAfter || "" }
                Pair { id: pul; label: qsTr("Puls"); a.text: page.e.pulseBefore || ""; b.text: page.e.pulseAfter || "" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.gapS
                    Label { text: qsTr("Befinden"); color: Theme.ink2; Layout.preferredWidth: win.phone ? 90 : 160 }
                    VCombo { Layout.fillWidth: true; model: ["–", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"]
                             currentIndex: Math.max(0, page.feelingBefore); onActivated: (i) => page.feelingBefore = i === 0 ? -1 : i }
                    VCombo { Layout.fillWidth: true; model: ["–", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"]
                             currentIndex: Math.max(0, page.feelingAfter); onActivated: (i) => page.feelingAfter = i === 0 ? -1 : i }
                }
            }
            Card {
                visible: !!page.e.deviceTraining
                SectionTitle { text: qsTr("Atemtrainer") }
                VCombo { label: qsTr("Meist erreicht"); Layout.fillWidth: true
                         model: [qsTr("nicht erfasst"), qsTr("kein Ball"), qsTr("1 Ball"), qsTr("2 Bälle"), qsTr("3 Bälle")]
                         currentIndex: page.balls + 1; onActivated: (i) => page.balls = i - 1 }
                VField { id: hold; label: qsTr("Längste Haltezeit (s)"); text: page.e.deviceHold || ""; inputMethodHints: Qt.ImhDigitsOnly
                         Layout.fillWidth: true }
            }
            Card {
                VArea { id: note; label: qsTr("Notiz"); text: page.e.note || ""; Layout.fillWidth: true }
            }
            VButton { objectName: "saveSession"; kind: "primary"; text: qsTr("Speichern"); Layout.alignment: Qt.AlignRight; onClicked: page.save() }
        }
    }
    // Aliase für die Felder der Messwerte (Pair-Komponenten)
    readonly property alias s1: spo.a
    readonly property alias s2: spo.b
    readonly property alias p1: pfp.a
    readonly property alias p2: pfp.b
    readonly property alias u1: pul.a
    readonly property alias u2: pul.b
}
