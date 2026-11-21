// Trainingsmodus: Messwerte vor bzw. nach der Übung (leere Felder werden nicht gespeichert)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "measurePage"
    property bool before: true
    property int feeling: -1
    readonly property var ask: app.runner.askFields
    signal done()
    background: Rectangle { color: Theme.paper }
    function goBack() { return true }        // mitten im Ablauf nicht einfach zurück

    function finish(skip) {
        const v = skip ? ({}) : { spo2: spo2.text, peakFlow: pf.text, pulse: pulse.text, feeling: page.feeling }
        if (page.before) app.runner.setBefore(v); else app.runner.setAfter(v)
        page.done()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader {
            backVisible: false
            title: page.before ? qsTr("Vor der Übung messen") : qsTr("Nach der Übung messen")
            subtitle: app.runner.moduleName
        }
        ScrollPage {
            maxWidth: 640
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                    text: page.before ? qsTr("Messen Sie in Ruhe, bevor Sie beginnen. Felder, die Sie nicht messen möchten, lassen Sie einfach leer.")
                                      : qsTr("Bleiben Sie noch einen Moment ruhig sitzen und messen Sie dann erneut – so lässt sich die Wirkung der Übung im Verlauf vergleichen.") }
            Card {
                VField { id: spo2; objectName: "spo2Field"; visible: !!page.ask.spo2; label: qsTr("Sauerstoffsättigung SpO₂ (%)")
                         inputMethodHints: Qt.ImhDigitsOnly; Layout.fillWidth: true; validator: IntValidator { bottom: 50; top: 100 } }
                VField { id: pf; visible: !!page.ask.peakFlow; label: qsTr("Peak Flow (l/min)")
                         inputMethodHints: Qt.ImhDigitsOnly; Layout.fillWidth: true; validator: IntValidator { bottom: 30; top: 900 } }
                VField { id: pulse; visible: !!page.ask.pulse; label: qsTr("Puls (Schläge/min)")
                         inputMethodHints: Qt.ImhDigitsOnly; Layout.fillWidth: true; validator: IntValidator { bottom: 30; top: 250 } }
                ColumnLayout {
                    visible: !!page.ask.feeling
                    Layout.fillWidth: true
                    spacing: 4
                    Label { text: qsTr("Befinden (1 = schlecht … 10 = sehr gut)"); color: Theme.ink2; font.pixelSize: Theme.fsS
                            wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Flow {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 200
                        Layout.preferredHeight: implicitHeight
                        spacing: 6
                        Repeater {
                            model: 10
                            VChip { text: String(index + 1); checkable: false; checked: page.feeling === index + 1
                                    implicitWidth: Theme.touch; onClicked: page.feeling = (page.feeling === index + 1 ? -1 : index + 1) }
                        }
                    }
                }
            }
            ButtonRow {
                VButton { objectName: "measureNext"; kind: "primary"; text: page.before ? qsTr("Weiter zur Übung") : qsTr("Weiter")
                          onClicked: page.finish(false) }
                VButton { text: qsTr("Überspringen"); onClicked: page.finish(true) }
            }
        }
    }
}
