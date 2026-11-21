// Bereich „Einstellungen“: Sprache, Darstellung, Benutzer, Sicherheit, Sicherung, Abgleich, Erinnerungen, Über
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Item {
    id: view
    objectName: "settingsView"
    property var rs: app.reminderStatus()
    Component { id: usersComp; UsersPage { onClosed: stack.pop() } }
    Component { id: syncComp; SyncPage { onClosed: stack.pop() } }
    Component { id: pwComp; PasswordPage { onClosed: (msg) => { stack.pop(); if (msg) win.toast(msg) } } }
    Component { id: aboutComp; AboutPage { onClosed: stack.pop() } }
    Connections { target: app; function onRevisionChanged() { view.rs = app.reminderStatus() } }

    ScrollPage {
        anchors.fill: parent
        Card {
            SectionTitle { text: qsTr("Sprache und Darstellung") }
            VCombo { label: qsTr("Sprache / Language"); Layout.fillWidth: true; model: ["Deutsch", "English"]
                     currentIndex: app.language === "en" ? 1 : 0; onActivated: (i) => app.setLanguage(i === 1 ? "en" : "de") }
            Label { text: qsTr("Design"); color: Theme.ink2; font.pixelSize: Theme.fsS }
            Flow {
                Layout.fillWidth: true
                Layout.preferredWidth: 200
                Layout.preferredHeight: implicitHeight
                spacing: 6
                Repeater {
                    model: [qsTr("wie das System"), qsTr("hell"), qsTr("dunkel")]
                    VChip { text: modelData; checkable: false; checked: app.themeMode === index; onClicked: app.themeMode = index }
                }
            }
        }
        Card {
            SectionTitle { text: qsTr("Benutzer") }
            Label { text: qsTr("Jeder Benutzer hat eigene Einstellungen, Erinnerungen, Wecker und ein eigenes Tagebuch.")
                    color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            VButton { objectName: "manageUsers"; iconName: "users"; text: qsTr("Benutzer verwalten …"); onClicked: stack.push(usersComp) }
        }
        Card {
            SectionTitle { text: qsTr("Sicherheit") }
            VButton { visible: !app.demo; iconName: "lock"; text: qsTr("Master-Passwort ändern …"); onClicked: stack.push(pwComp) }
            VCombo { label: qsTr("Im Hintergrund sperren nach"); Layout.fillWidth: true
                     readonly property var mins: [0, 5, 15, 60]
                     model: [qsTr("nie"), qsTr("5 Minuten"), qsTr("15 Minuten"), qsTr("1 Stunde")]
                     currentIndex: Math.max(0, mins.indexOf(app.autoLockMinutes)); onActivated: (i) => app.autoLockMinutes = mins[i] }
            VButton { iconName: "lock"; text: app.demo ? qsTr("Beispieldaten schließen") : qsTr("Jetzt sperren")
                      onClicked: { app.lock(); win.goStart() } }
        }
        Card {
            visible: !app.demo
            SectionTitle { text: qsTr("Daten") }
            Label { text: qsTr("Die Sicherung ist mit dem Master-Passwort verschlüsselt. Sie lässt sich am Computer (Datei → Datenbank wiederherstellen) oder bei der Einrichtung eines anderen Geräts einspielen.")
                    color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            VButton { iconName: "database"; text: qsTr("Sicherung teilen …"); onClicked: if (!app.shareBackup()) win.toast(app.lastError, true) }
            VButton { objectName: "syncSettings"; iconName: "sync"; text: qsTr("Abgleich über den eigenen Webserver …"); onClicked: stack.push(syncComp) }
        }
        Card {
            visible: app.isAndroid
            SectionTitle { text: qsTr("Erinnerungen und Wecker") }
            Label { text: view.rs.count > 0 ? qsTr("%1 Termine in den nächsten 14 Tagen bei Android angemeldet, nächster: %2").arg(view.rs.count).arg(view.rs.next)
                                            : qsTr("Zurzeit sind keine Erinnerungen angemeldet.")
                    color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: qsTr("Öffnen Sie ViviaBreathe mindestens alle 14 Tage einmal – sonst laufen die angemeldeten Termine aus.")
                    color: Theme.muted; font.pixelSize: Theme.fsS; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            VButton { text: view.rs.permission ? qsTr("Benachrichtigungen: erlaubt") : qsTr("Benachrichtigungen erlauben …")
                      onClicked: app.requestNotificationPermission() }
            VButton { text: view.rs.exact ? qsTr("Pünktliche Wecker: erlaubt") : qsTr("Pünktliche Wecker erlauben …")
                      onClicked: app.openExactAlarmSettings() }
        }
        Card {
            SectionTitle { text: qsTr("Über ViviaBreathe") }
            Label { text: qsTr("Version %1 · kein Medizinprodukt, sondern ein persönliches Übungs-Tagebuch").arg(app.version)
                    color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            VButton { iconName: "info"; text: qsTr("Über und Lizenz …"); onClicked: stack.push(aboutComp) }
        }
    }
}
