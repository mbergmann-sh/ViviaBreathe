// Abgleich über den eigenen Webserver (wie am Computer: Adresse und Abgleich-Schlüssel)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "syncPage"
    property var c: app.syncConfig()
    signal closed()
    background: Rectangle { color: Theme.paper }
    Connections { target: app; function onSyncFinished() { page.c = app.syncConfig() } }
    function save() {
        if (app.saveSyncConfig({ enabled: on.checked, autoSync: auto.checked, url: url.text, key: key.text })) {
            win.toast(qsTr("Gespeichert.")); page.c = app.syncConfig(); return true
        }
        win.toast(app.lastError, true)
        return false
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader { title: qsTr("Abgleich"); onBack: page.closed() }
        ScrollPage {
            maxWidth: 680
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                    text: qsTr("Der Webserver verwahrt nur die verschlüsselte Datenbank; zusammengeführt wird auf den Geräten. Alle Geräte verwenden dasselbe Master-Passwort. Adresse und Schlüssel stehen am Computer unter Extras → Einstellungen → Abgleich.") }
            Card {
                VSwitch { id: on; text: qsTr("Abgleich einschalten"); checked: !!page.c.enabled }
                VSwitch { id: auto; text: qsTr("Automatisch abgleichen"); sub: qsTr("nach Änderungen, beim Öffnen und beim Wechsel in den Hintergrund")
                          checked: page.c.autoSync !== false }
                VField { id: url; label: qsTr("Adresse des Webservers"); text: page.c.url || ""; placeholder: "https://atem.example.de/"
                         inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase; Layout.fillWidth: true }
                VField { id: key; label: qsTr("Abgleich-Schlüssel"); text: page.c.key || ""; Layout.fillWidth: true
                         inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText }
                Label { text: page.c.tls ? qsTr("TLS bereit: %1").arg(page.c.tlsText) : qsTr("TLS fehlt: %1").arg(page.c.tlsText)
                        color: page.c.tls ? Theme.muted : Theme.alert; font.pixelSize: Theme.fsXs; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            }
            Card {
                Label { text: page.c.last ? qsTr("Zuletzt: %1 – %2").arg(page.c.last).arg(page.c.lastResult || "") : qsTr("Noch nicht abgeglichen.")
                        color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                Label { visible: !!page.c.pending; text: qsTr("Es gibt Änderungen, die noch nicht abgeglichen sind."); color: Theme.accent
                        wrapMode: Text.WordWrap; Layout.fillWidth: true }
                BusyIndicator { visible: app.syncBusy; running: visible }
            }
            ButtonRow {
                VButton { kind: "primary"; iconName: "sync"; text: qsTr("Jetzt abgleichen"); enabled: !app.syncBusy
                          onClicked: if (page.save()) app.syncNow() }
                VButton { text: qsTr("Speichern"); onClicked: page.save() }
            }
        }
    }
}
