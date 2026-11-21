// Allererster Start: Sprache wählen (zweisprachig, vor dem Hinweis)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "languagePage"
    signal done()
    background: Rectangle { color: Theme.paper }
    CenterCard {
        AppTitle {}
        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink; font.pixelSize: Theme.fsL
                text: "Willkommen! Bitte wählen Sie die Sprache.\nWelcome! Please choose your language." }
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.gap
            VButton { objectName: "langDe"; text: "Deutsch"; kind: "primary"; Layout.fillWidth: true
                      onClicked: { app.setLanguage("de"); page.done() } }
            VButton { objectName: "langEn"; text: "English"; kind: "primary"; Layout.fillWidth: true
                      onClicked: { app.setLanguage("en"); page.done() } }
        }
        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.muted; font.pixelSize: Theme.fsS
                text: "Später änderbar unter Einstellungen.\nCan be changed later under Settings." }
    }
}
