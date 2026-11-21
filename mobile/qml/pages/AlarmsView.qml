// Bereich „Wecker & Einschlafen“: Wecker mit Musik und gespeicherte (auch geplante) Einschlaf-Einträge
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Item {
    id: view
    objectName: "alarmsView"
    property var alarms: []
    property var plans: []
    function reload() { alarms = app.alarms(); plans = app.sleepPlans() }
    Component.onCompleted: reload()
    Connections { target: app; function onRevisionChanged() { view.reload() } function onUserChanged() { view.reload() } }
    Component { id: alarmComp; AlarmEditPage { onClosed: (msg) => { stack.pop(); if (msg) win.toast(msg) } } }
    Component { id: sleepComp; SleepEditPage { onClosed: (msg) => { stack.pop(); if (msg) win.toast(msg) } } }
    Component { id: sleepRunComp; SleepRunPage { onClosed: stack.pop() } }

    ScrollPage {
        anchors.fill: parent
        ReminderHint { needExact: view.alarms.length > 0 }

        // ---- Wecker
        RowLayout {
            Layout.fillWidth: true
            Image { source: Theme.icon("alarm", Theme.ink); sourceSize.width: 30; sourceSize.height: 30 }
            Label { text: qsTr("Wecker"); font.pixelSize: Theme.fsXl; font.bold: true; color: Theme.ink; Layout.fillWidth: true }
            VButton { objectName: "addAlarm"; kind: "primary"; iconName: "add"; text: win.phone ? "" : qsTr("Neu …")
                      onClicked: stack.push(alarmComp, { alarmId: 0 }) }
        }
        Label { visible: view.alarms.length === 0; text: qsTr("Noch kein Wecker. Er weckt mit Ihrer Musik, sanft lauter werdend – auch wenn die App geschlossen ist.")
                color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Repeater {
            model: view.alarms
            delegate: Card {
                clickable: true
                onClicked: stack.push(alarmComp, { alarmId: modelData.id })
                opacity: modelData.enabled ? 1 : 0.6
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.gap
                    Label { text: modelData.time; font.pixelSize: Theme.fsXxl; font.bold: true; color: Theme.brand }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Label { text: modelData.label || qsTr("Wecker"); font.bold: true; color: Theme.ink; Layout.fillWidth: true; elide: Text.ElideRight }
                        Label { text: modelData.weekdaysText + " · " + modelData.playlistText; color: Theme.ink2; font.pixelSize: Theme.fsS
                                Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        Label { text: modelData.enabled ? modelData.next : qsTr("ausgeschaltet"); color: Theme.muted; font.pixelSize: Theme.fsXs }
                    }
                }
            }
        }

        // ---- Einschlafen
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.gap
            Image { source: Theme.icon("moon", Theme.ink); sourceSize.width: 30; sourceSize.height: 30 }
            Label { text: qsTr("Einschlafen"); font.pixelSize: Theme.fsXl; font.bold: true; color: Theme.ink; Layout.fillWidth: true }
            VButton { objectName: "addSleep"; kind: "primary"; iconName: "add"; text: win.phone ? "" : qsTr("Neu …")
                      onClicked: stack.push(sleepComp, { planId: 0 }) }
        }
        Label { visible: view.plans.length === 0; text: qsTr("Noch kein Einschlaf-Eintrag. Er spielt Ihre Musik eine gewählte Zeit lang und wird zum Ende leiser.")
                color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Repeater {
            model: view.plans
            delegate: Card {
                clickable: true
                onClicked: stack.push(sleepComp, { planId: modelData.id })
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.gap
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Label { text: modelData.label; font.bold: true; color: Theme.ink; Layout.fillWidth: true; elide: Text.ElideRight }
                        Label { text: qsTr("%1 min · %2").arg(modelData.minutes).arg(modelData.playlistText)
                                      + (modelData.moduleName ? " · " + qsTr("vorher %1").arg(modelData.moduleName) : "")
                                color: Theme.ink2; font.pixelSize: Theme.fsS; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        Label { visible: modelData.scheduled
                                text: qsTr("geplant: %1 · %2").arg(modelData.time).arg(modelData.weekdaysText)
                                      + (modelData.autoStart ? " · " + qsTr("startet automatisch") : "")
                                color: Theme.muted; font.pixelSize: Theme.fsXs; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                    }
                    VButton { objectName: "sleepNow_" + index; kind: "primary"; iconName: "moon"; text: win.phone ? "" : qsTr("Jetzt")
                              onClicked: stack.push(sleepRunComp, { planId: modelData.id }) }
                }
            }
        }
    }
}
