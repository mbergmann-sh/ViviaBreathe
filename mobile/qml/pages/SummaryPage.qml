// Zusammenfassung nach der Übung: Werte, Notiz, Ergebnis mit dem Atemtrainer; speichern oder verwerfen
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "summaryPage"
    property var sum: app.runner.summary()
    property int balls: sum.deviceTarget || 1
    property int hold: sum.deviceHoldSuggestion || 3
    signal closed(string msg)
    background: Rectangle { color: Theme.paper }
    function goBack() { discardQuestion.open(); return true }

    Confirm {
        id: discardQuestion
        title: qsTr("Eintrag verwerfen?")
        message: qsTr("Die Übung wird nicht ins Tagebuch übernommen.")
        acceptText: qsTr("Verwerfen"); danger: true
        onAccepted: { app.runner.reset(); page.closed("") }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader { backVisible: false; title: page.sum.completed ? qsTr("Geschafft!") : qsTr("Übung beendet"); subtitle: page.sum.module || "" }
        ScrollPage {
            maxWidth: 680
            Card {
                Repeater {
                    model: [
                        [qsTr("Anlass"), page.sum.event || ""],
                        [qsTr("Muster"), (page.sum.pattern || "") + qsTr(" Sekunden")],
                        [qsTr("Dauer"), page.sum.duration || ""],
                        [page.sum.cyclesLabel || "", String(page.sum.cycles)],
                        [qsTr("Status"), page.sum.completed ? qsTr("vollständig") : qsTr("vorzeitig beendet")]
                    ].filter(r => r[1] !== "")
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        Label { text: modelData[0]; color: Theme.muted; Layout.preferredWidth: win.phone ? 120 : 180 }
                        Label { text: modelData[1]; color: Theme.ink; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.bold: true }
                    }
                }
            }
            Card {
                visible: !!page.sum.measurements && page.sum.measurements.length > 0
                SectionTitle { text: qsTr("Messwerte") }
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.preferredWidth: win.phone ? 110 : 160 }
                    Label { text: qsTr("vorher"); color: Theme.muted; Layout.preferredWidth: 100 }
                    Label { text: " "; color: Theme.muted }
                    Label { text: qsTr("nachher"); color: Theme.muted; Layout.fillWidth: true }
                }
                Repeater {
                    model: page.sum.measurements || []
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        Label { text: modelData.name; color: Theme.ink2; Layout.preferredWidth: win.phone ? 110 : 160 }
                        Label { text: modelData.before; color: Theme.ink; Layout.preferredWidth: 100 }
                        Label { text: "→"; color: Theme.muted }
                        Label { text: modelData.after; color: Theme.ink; font.bold: true; Layout.fillWidth: true }
                    }
                }
            }
            Card {
                visible: !!page.sum.deviceTraining
                SectionTitle { text: qsTr("Ergebnis mit dem Atemtrainer") }
                VCombo { label: qsTr("Meist erreicht"); Layout.fillWidth: true
                         model: [qsTr("kein Ball"), qsTr("1 Ball"), qsTr("2 Bälle"), qsTr("3 Bälle")]
                         currentIndex: page.balls; onActivated: (i) => page.balls = i }
                VStepper { label: qsTr("Längste Haltezeit"); from: 0; to: 60; step: 1; suffix: " s"; value: page.hold
                           onEdited: (v) => page.hold = v }
            }
            Card {
                VArea { id: note; label: qsTr("Notiz (optional)"); placeholder: qsTr("Wie ging es Ihnen bei der Übung?"); Layout.fillWidth: true }
            }
            ButtonRow {
                VButton { objectName: "saveButton"; kind: "primary"; iconName: "add"; text: qsTr("Im Tagebuch speichern")
                          onClicked: {
                              if (app.runner.save(note.text, page.sum.deviceTraining ? page.balls : -1, page.sum.deviceTraining ? page.hold : -1))
                                  page.closed(qsTr("Im Tagebuch gespeichert."))
                              else win.toast(qsTr("Speichern fehlgeschlagen."), true)
                          } }
                VButton { text: qsTr("Verwerfen"); onClicked: discardQuestion.open() }
            }
        }
    }
}
