// Über ViviaBreathe: Version, Hinweis, Lizenz (PolyForm Noncommercial 1.0.0) und Fremdbestandteile
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "aboutPage"
    signal closed()
    property bool showLicense: false
    background: Rectangle { color: Theme.paper }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader { title: qsTr("Über ViviaBreathe"); onBack: page.closed() }
        ScrollPage {
            maxWidth: 720
            AppTitle {}
            Label { text: qsTr("Version %1").arg(app.version); color: Theme.ink2 }
            Card {
                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink; textFormat: Text.StyledText
                        text: qsTr("<b>Kein Medizinprodukt.</b> ViviaBreathe ist ein persönliches Tagebuch. Es stellt keine Diagnosen und ersetzt keine ärztliche Beratung. Die Verwendung erfolgt auf eigene Gefahr.") }
            }
            Card {
                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink; textFormat: Text.StyledText
                        onLinkActivated: (link) => Qt.openUrlExternally(link)
                        text: qsTr("© 2026 Michael Bergmann<br/>Quelloffen unter der <b>PolyForm Noncommercial License 1.0.0</b>: kostenlos für jede <b>nicht-kommerzielle</b> Nutzung – privat sowie durch gemeinnützige Organisationen, Schulen und Behörden –, einschließlich Verändern und Weitergeben. <b>Kommerzielle Nutzung und Verkauf</b> nur mit Erlaubnis des Autors.<br/><a href=\"https://polyformproject.org/licenses/noncommercial/1.0.0\">polyformproject.org/licenses/noncommercial/1.0.0</a>") }
                Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink2; font.pixelSize: Theme.fsS
                        text: qsTr("Fremdbestandteile: Qt (LGPLv3), SQLite3 Multiple Ciphers (MIT, Ulrich Telle), SQLite (gemeinfrei), OpenSSL für Android (Apache-2.0), Schrift Atkinson Hyperlegible (SIL Open Font License 1.1, Braille Institute).") }
                VButton { text: page.showLicense ? qsTr("Lizenztext ausblenden") : qsTr("Lizenztext anzeigen"); onClicked: page.showLicense = !page.showLicense }
                Label { visible: page.showLicense; text: app.licenseText(); wrapMode: Text.WordWrap; Layout.fillWidth: true
                        color: Theme.ink2; font.pixelSize: Theme.fsXs; textFormat: Text.PlainText }
            }
        }
    }
}
