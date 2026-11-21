// Anmeldung mit dem Master-Passwort
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "loginPage"
    signal done()
    signal setupRequested()
    background: Rectangle { color: Theme.paper }

    function tryLogin() {
        if (app.login(pw.text)) { pw.text = ""; done() }
        else msg.text = app.lastError
    }

    CenterCard {
        cardWidth: 520
        AppTitle {}
        VField { id: pw; objectName: "passwordField"; label: qsTr("Master-Passwort"); echoMode: TextInput.Password; Layout.fillWidth: true
                 inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                 onAccepted: page.tryLogin(); Component.onCompleted: field.forceActiveFocus() }
        Label { id: msg; color: Theme.alert; Layout.fillWidth: true; wrapMode: Text.WordWrap; visible: text !== "" }
        VButton { objectName: "loginButton"; text: qsTr("Öffnen"); kind: "primary"; Layout.fillWidth: true; enabled: pw.text !== ""
                  onClicked: page.tryLogin() }
        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.muted; font.pixelSize: Theme.fsS
                text: qsTr("Das Master-Passwort verschlüsselt alle Daten. Es lässt sich nicht wiederherstellen – ohne es bleiben die Daten unlesbar.") }
        VButton { text: qsTr("Mit Beispieldaten ausprobieren"); kind: "flat"; Layout.alignment: Qt.AlignHCenter
                  onClicked: { if (app.startDemo()) page.done(); else msg.text = app.lastError } }
    }
    Label {
        anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottomMargin: 16
        width: Math.min(implicitWidth, parent.width - 24); horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap
        text: qsTr("%1 %2 – persönliches Übungs-Tagebuch, kein Medizinprodukt").arg(app.appName).arg(app.version)
        color: Theme.muted; font.pixelSize: Theme.fsXs
    }
}
