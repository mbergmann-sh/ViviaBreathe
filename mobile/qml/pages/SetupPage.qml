// Ersteinrichtung: neue Datenbank, von einem anderen Gerät (Webserver) übernehmen, Sicherung
// einspielen oder mit Beispieldaten ausprobieren
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "setupPage"
    signal done()
    property bool fromLogin: false
    property int mode: 0          // 0 Auswahl, 1 neu, 2 Webserver, 3 Sicherung
    property string backupUrl: ""
    background: Rectangle { color: Theme.paper }
    function goBack() { if (mode !== 0) { mode = 0; msg.text = ""; return true } return false }

    CenterCard {
        cardWidth: 600
        AppTitle {}
        // ---- Auswahl
        ColumnLayout {
            visible: page.mode === 0
            Layout.fillWidth: true
            spacing: Theme.gapS
            Label { text: qsTr("Wie möchten Sie beginnen?"); font.pixelSize: Theme.fsL; font.bold: true; color: Theme.ink
                    Layout.fillWidth: true; wrapMode: Text.WordWrap }
            VButton { objectName: "newButton"; text: qsTr("Neu beginnen"); kind: "primary"; Layout.fillWidth: true; onClicked: page.mode = 1 }
            VButton { objectName: "serverButton"; text: qsTr("Vom Webserver übernehmen"); Layout.fillWidth: true
                      onClicked: page.mode = 2 }
            VButton { text: qsTr("Sicherung einspielen (.vbdb)"); Layout.fillWidth: true; onClicked: page.mode = 3 }
            VButton { text: qsTr("Mit Beispieldaten ausprobieren"); kind: "flat"; Layout.fillWidth: true
                      onClicked: { if (app.startDemo()) page.done(); else msg.text = app.lastError } }
        }
        // ---- neu
        ColumnLayout {
            visible: page.mode === 1
            Layout.fillWidth: true
            spacing: Theme.gapS
            SectionTitle { text: qsTr("Neu beginnen") }
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                    text: qsTr("Wählen Sie ein Master-Passwort. Es verschlüsselt alle Daten und wird bei jedem Öffnen abgefragt. Bitte gut merken: Ohne das Passwort lassen sich die Daten nicht wiederherstellen.") }
            VField { id: firstUser; objectName: "firstUserField"; label: qsTr("Ihr Name (erster Benutzer)"); Layout.fillWidth: true }
            VField { id: pw1; objectName: "newPassword"; label: qsTr("Master-Passwort"); echoMode: TextInput.Password; Layout.fillWidth: true
                     hint: qsTr("Mindestens 8 Zeichen, Buchstaben mit Ziffern oder Sonderzeichen gemischt – oder ein langer Satz.") }
            VField { id: pw2; objectName: "newPassword2"; label: qsTr("Master-Passwort wiederholen"); echoMode: TextInput.Password; Layout.fillWidth: true }
            VButton { text: qsTr("Einrichten"); kind: "primary"; Layout.fillWidth: true
                      onClicked: { if (app.createDatabase(pw1.text, pw2.text, firstUser.text)) page.done(); else msg.text = app.lastError } }
        }
        // ---- Webserver
        ColumnLayout {
            visible: page.mode === 2
            Layout.fillWidth: true
            spacing: Theme.gapS
            SectionTitle { text: qsTr("Von einem anderen Gerät übernehmen") }
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                    text: qsTr("Die Angaben stehen am Computer unter Extras → Einstellungen → Abgleich. Das Master-Passwort ist auf allen Geräten dasselbe.") }
            VField { id: url; label: qsTr("Adresse des Webservers"); placeholder: "https://atem.example.de/"; Layout.fillWidth: true
                     inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase }
            VField { id: key; label: qsTr("Abgleich-Schlüssel"); Layout.fillWidth: true
                     inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText }
            VField { id: spw; label: qsTr("Master-Passwort"); echoMode: TextInput.Password; Layout.fillWidth: true }
            VButton { text: qsTr("Übernehmen"); kind: "primary"; Layout.fillWidth: true
                      onClicked: { if (app.setupFromServer(url.text, key.text, spw.text)) page.done(); else msg.text = app.lastError } }
        }
        // ---- Sicherung
        ColumnLayout {
            visible: page.mode === 3
            Layout.fillWidth: true
            spacing: Theme.gapS
            SectionTitle { text: qsTr("Sicherung einspielen") }
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                    text: qsTr("Eine Sicherung (.vbdb) vom Computer oder von einem anderen Gerät übernehmen. Sie ist mit dem Master-Passwort verschlüsselt, das beim Sichern galt.") }
            VButton { text: page.backupUrl === "" ? qsTr("Sicherung auswählen …") : app.fileName(page.backupUrl); Layout.fillWidth: true
                      onClicked: backupDlg.open() }
            VField { id: bpw; label: qsTr("Master-Passwort"); echoMode: TextInput.Password; Layout.fillWidth: true }
            VButton { text: qsTr("Einspielen"); kind: "primary"; Layout.fillWidth: true; enabled: page.backupUrl !== ""
                      onClicked: { if (app.restoreBackup(page.backupUrl, bpw.text)) page.done(); else msg.text = app.lastError } }
        }
        Label { id: msg; color: Theme.alert; Layout.fillWidth: true; wrapMode: Text.WordWrap; visible: text !== "" }
        VButton { visible: page.mode !== 0; text: qsTr("Zurück"); kind: "flat"; Layout.alignment: Qt.AlignHCenter; onClicked: page.goBack() }
        VButton { visible: page.mode === 0 && page.fromLogin; text: qsTr("Abbrechen"); kind: "flat"; Layout.alignment: Qt.AlignHCenter
                  onClicked: stack.pop() }
    }
    FileDialog {
        id: backupDlg
        title: qsTr("Sicherung auswählen")
        nameFilters: [qsTr("ViviaBreathe-Sicherung (*.vbdb)"), qsTr("Alle Dateien (*)")]
        onAccepted: page.backupUrl = String(selectedFile)
    }
}
