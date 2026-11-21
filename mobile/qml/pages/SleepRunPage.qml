// Einschlafen starten: ggf. erst die Übung, dann läuft die Musik (auch bei ausgeschaltetem Bildschirm)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "sleepRunPage"
    property var planId: 0
    property var p: app.sleepPlan(planId)
    property int minutes: p.minutes || 30
    readonly property bool playing: app.playback.running && app.playback.mode === "sleep"
    signal closed()
    background: Rectangle { color: Theme.paper }

    function startMusic() {
        if (app.startSleep(page.planId, page.minutes)) win.toast(qsTr("Gute Nacht!"))
        else win.toast(app.lastError, true)
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader { title: page.p.label || qsTr("Einschlafen"); onBack: page.closed() }
        ScrollPage {
            maxWidth: 560
            Image { Layout.alignment: Qt.AlignHCenter; source: Theme.icon("moon", Theme.brand); sourceSize.width: 96; sourceSize.height: 96 }
            Card {
                visible: !page.playing
                Label { text: page.p.playlistText || ""; color: Theme.ink2; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                VStepper { label: qsTr("Spieldauer"); from: 5; to: 240; step: 5; suffix: qsTr(" min"); value: page.minutes
                           onEdited: (v) => page.minutes = v }
                VButton { visible: !!page.p.moduleId; kind: "primary"; iconName: "play"; Layout.fillWidth: true
                          text: qsTr("Erst %1, dann Musik").arg(page.p.moduleName || "")
                          onClicked: { win.pendingSleep = { planId: page.planId, minutes: page.minutes }; win.openPrep(page.p.moduleId, qsTr("Einschlafübung")) } }
                VButton { objectName: "sleepStart"; kind: page.p.moduleId ? "ghost" : "primary"; iconName: "moon"; Layout.fillWidth: true
                          text: qsTr("Musik starten"); onClicked: page.startMusic() }
            }
            Card {
                visible: page.playing
                Label { text: qsTr("Die Musik läuft und wird zum Ende leiser. Sie können den Bildschirm ausschalten."); color: Theme.ink
                        wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { text: qsTr("noch %1 min").arg(Math.ceil(app.playback.remainingSec / 60)); font.pixelSize: Theme.fsXxl; font.bold: true
                        color: Theme.brand }
                Label { text: app.playback.track; color: Theme.muted; elide: Text.ElideMiddle; Layout.fillWidth: true }
                VButton { kind: "primary"; iconName: "stop"; text: qsTr("Aus"); Layout.fillWidth: true; onClicked: app.playback.stop() }
            }
            Label { text: qsTr("Tipp: Mit dem Lautstärkeregler des Geräts lässt sich die Musik jederzeit anpassen."); color: Theme.muted
                    font.pixelSize: Theme.fsS; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
    }
}
