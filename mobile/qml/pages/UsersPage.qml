// Benutzer anlegen, umbenennen, löschen und auswählen
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "usersPage"
    property var editId: 0
    property var deleteId: 0
    signal closed()
    background: Rectangle { color: Theme.paper }
    Confirm {
        id: delQuestion
        title: qsTr("Benutzer löschen?")
        message: qsTr("Der Benutzer wird mit seinem ganzen Tagebuch, seinen Erinnerungen, Weckern und Einstellungen gelöscht.")
        acceptText: qsTr("Löschen"); danger: true
        onAccepted: { if (!app.deleteUser(page.deleteId)) win.toast(app.lastError, true) }
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader { title: qsTr("Benutzer"); onBack: page.closed() }
        ScrollPage {
            maxWidth: 640
            Repeater {
                model: app.users
                delegate: Card {
                    RowLayout {
                        Layout.fillWidth: true
                        visible: page.editId !== modelData.id
                        Image { source: Theme.icon("user", modelData.id === app.userId ? Theme.brand : Theme.ink2); sourceSize.width: 28; sourceSize.height: 28 }
                        Label { text: modelData.name; font.bold: modelData.id === app.userId; color: Theme.ink; Layout.fillWidth: true
                                elide: Text.ElideRight }
                        VButton { visible: modelData.id !== app.userId; text: qsTr("Wählen"); onClicked: app.selectUser(modelData.id) }
                        VButton { kind: "flat"; iconName: "edit"; onClicked: { page.editId = modelData.id; rename.text = modelData.name } }
                        VButton { kind: "flat"; iconName: "delete"; visible: app.users.length > 1
                                  onClicked: { page.deleteId = modelData.id; delQuestion.message = qsTr("„%1“ wird mit dem ganzen Tagebuch, den Erinnerungen, Weckern und Einstellungen gelöscht.").arg(modelData.name); delQuestion.open() } }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: page.editId === modelData.id
                        VField { id: rename; Layout.fillWidth: true }
                        VButton { kind: "primary"; text: qsTr("OK")
                                  onClicked: { if (app.renameUser(modelData.id, rename.text)) page.editId = 0; else win.toast(app.lastError, true) } }
                        VButton { kind: "flat"; text: "✕"; onClicked: page.editId = 0 }
                    }
                }
            }
            Card {
                SectionTitle { text: qsTr("Neuer Benutzer") }
                RowLayout {
                    Layout.fillWidth: true
                    VField { id: newName; objectName: "newUserName"; placeholder: qsTr("Name"); Layout.fillWidth: true }
                    VButton { kind: "primary"; iconName: "user-add"; text: win.phone ? "" : qsTr("Anlegen"); enabled: newName.text.trim() !== ""
                              onClicked: { if (app.addUser(newName.text)) { newName.text = ""; win.toast(qsTr("Benutzer angelegt.")) } else win.toast(app.lastError, true) } }
                }
            }
        }
    }
}
