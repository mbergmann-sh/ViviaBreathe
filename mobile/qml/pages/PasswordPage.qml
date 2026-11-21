// Master-Passwort ändern (verschlüsselt die Datenbank neu)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "passwordPage"
    signal closed(string msg)
    background: Rectangle { color: Theme.paper }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader { title: qsTr("Master-Passwort ändern"); onBack: page.closed("") }
        ScrollPage {
            maxWidth: 560
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                    text: qsTr("Das neue Passwort gilt für alle Geräte, die abgleichen. Ändern Sie es danach auch dort – andere Geräte fragen beim nächsten Abgleich nach dem neuen Passwort.") }
            Card {
                VField { id: oldPw; label: qsTr("Bisheriges Master-Passwort"); echoMode: TextInput.Password; Layout.fillWidth: true }
                VField { id: pw1; label: qsTr("Neues Master-Passwort"); echoMode: TextInput.Password; Layout.fillWidth: true
                         hint: qsTr("Mindestens 8 Zeichen, Buchstaben mit Ziffern oder Sonderzeichen gemischt – oder ein langer Satz.") }
                VField { id: pw2; label: qsTr("Neues Master-Passwort wiederholen"); echoMode: TextInput.Password; Layout.fillWidth: true }
                Label { id: msg; color: Theme.alert; wrapMode: Text.WordWrap; Layout.fillWidth: true; visible: text !== "" }
            }
            VButton { kind: "primary"; text: qsTr("Ändern"); Layout.alignment: Qt.AlignRight
                      onClicked: { if (app.changePassword(oldPw.text, pw1.text, pw2.text)) page.closed(qsTr("Das Master-Passwort wurde geändert.")); else msg.text = app.lastError } }
        }
    }
}
