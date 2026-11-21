// Hinweis vor der ersten Nutzung: kein Medizinprodukt, sondern ein Tagebuch
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "disclaimerPage"
    signal done()
    background: Rectangle { color: Theme.paper }
    CenterCard {
        cardWidth: 640
        AppTitle {}
        SectionTitle { text: qsTr("Bitte vor der ersten Nutzung lesen") }
        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink; textFormat: Text.StyledText
                text: qsTr("<b>ViviaBreathe ist kein Medizinprodukt.</b> Es ist ein persönliches Übungs-Tagebuch mit geführten Atemübungen. Es stellt keine Diagnosen und ersetzt keine ärztliche Beratung oder Behandlung.") }
        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                text: qsTr("Wird Ihnen bei einer Übung schwindelig oder unwohl, hören Sie auf und atmen Sie normal weiter. Bei Atemnot halten Sie sich an Ihren ärztlichen Notfallplan; im Notfall wählen Sie 112.") }
        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2
                text: qsTr("Ihre Daten bleiben verschlüsselt auf diesem Gerät. Nur wenn Sie den Abgleich einrichten, werden sie – ebenfalls verschlüsselt – auf Ihrem eigenen Webserver gespeichert.") }
        VButton { objectName: "acceptButton"; text: qsTr("Verstanden"); kind: "primary"; Layout.fillWidth: true
                  onClicked: { app.acceptDisclaimer(); page.done() } }
    }
}
