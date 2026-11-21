// Arbeitsbereich: Benutzer, Bereiche (Übungen, Verlauf, Erinnerungen, Wecker & Einschlafen, Einstellungen)
// Tablet quer: Seitenleiste links. Smartphone/Tablet hoch: Kopfzeile oben, Leiste unten.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: home
    property int tab: 0
    function goBack() { if (tab === 0) return false; tab = 0; return true }   // „Zurück“: erst zu „Übungen“
    function showTab(i) { tab = i }
    background: Rectangle { color: Theme.paper }

    readonly property var sections: [
        { title: qsTr("Übungen"), short: qsTr("Üben"), icon: "lungs" },
        { title: qsTr("Verlauf"), short: qsTr("Verlauf"), icon: "history" },
        { title: qsTr("Erinnerungen"), short: qsTr("Erinnern"), icon: "bell" },
        { title: qsTr("Wecker & Einschlafen"), short: qsTr("Wecker"), icon: "alarm" },
        { title: qsTr("Einstellungen"), short: qsTr("Mehr"), icon: "settings" }
    ]

    // Benutzerwahl (Smartphone: als Liste in einem Fenster)
    Popup {
        id: userPopup
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(420, home.width - 32)
        padding: 16
        background: Rectangle { radius: Theme.radius + 4; color: Theme.surface; border.color: Theme.rule }
        Overlay.modal: Rectangle { color: "#80000000" }
        contentItem: ColumnLayout {
            spacing: 4
            SectionTitle { text: qsTr("Benutzer wählen") }
            Repeater {
                model: app.users
                delegate: VButton {
                    Layout.fillWidth: true
                    kind: modelData.id === app.userId ? "primary" : "ghost"
                    iconName: "user"
                    text: modelData.name
                    onClicked: { app.selectUser(modelData.id); userPopup.close() }
                }
            }
            VButton { text: qsTr("Benutzer verwalten …"); kind: "flat"; Layout.fillWidth: true
                      onClicked: { userPopup.close(); stack.push(usersComp) } }
        }
    }
    Component { id: usersComp; UsersPage { onClosed: stack.pop() } }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---------------- Seitenleiste (Tablet quer)
        Rectangle {
            visible: win.wide
            Layout.preferredWidth: Theme.railWidth
            Layout.fillHeight: true
            color: Theme.surface
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.rule }
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                RowLayout {
                    Layout.margins: 16
                    spacing: 10
                    Image { source: "qrc:/icons/viviabreathe.png"; sourceSize.width: 40; sourceSize.height: 40 }
                    Label { text: app.appName; font.pixelSize: Theme.fsL; font.bold: true; color: Theme.ink; Layout.fillWidth: true }
                }
                VButton {
                    objectName: "userButton"
                    Layout.leftMargin: 12; Layout.rightMargin: 12; Layout.fillWidth: true
                    iconName: "user"; text: app.userName; onClicked: userPopup.open()
                }
                Item { height: 12 }
                Repeater {
                    model: home.sections
                    delegate: AbstractButton {
                        Layout.fillWidth: true
                        implicitHeight: 56
                        readonly property bool current: index === home.tab
                        onClicked: home.tab = index
                        background: Rectangle {
                            color: current ? Theme.brandWash : parent.down ? Theme.surface2 : "transparent"
                            Rectangle { visible: current; width: 4; height: parent.height; color: Theme.brand }
                        }
                        contentItem: RowLayout {
                            spacing: 12
                            Item { width: 8 }
                            Image { source: Theme.icon(modelData.icon, current ? Theme.brand : Theme.ink2); sourceSize.width: 26; sourceSize.height: 26 }
                            Label { text: modelData.title; color: current ? Theme.ink : Theme.ink2; font.bold: current; Layout.fillWidth: true
                                    elide: Text.ElideRight }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
                SleepBar { Layout.fillWidth: true; Layout.margins: 12 }
                VButton {
                    objectName: "quickHelpRail"
                    Layout.margins: 12; Layout.fillWidth: true
                    kind: "danger"; iconName: "lungs"; text: qsTr("Atemnot-Hilfe")
                    onClicked: win.quickHelp()
                }
            }
        }

        // ---------------- Inhalt
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Kopfzeile (Smartphone/Tablet hoch)
            Rectangle {
                visible: !win.wide
                Layout.fillWidth: true
                implicitHeight: 60
                color: Theme.surface
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.rule }
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12; anchors.rightMargin: 8
                    spacing: 8
                    Image { source: "qrc:/icons/viviabreathe.png"; sourceSize.width: 34; sourceSize.height: 34 }
                    VButton {
                        objectName: "userButtonTop"
                        kind: "flat"; iconName: "user"; text: app.userName
                        Layout.fillWidth: true; Layout.maximumWidth: implicitWidth
                        onClicked: userPopup.open()
                    }
                    Item { Layout.fillWidth: true }
                    VButton { objectName: "quickHelpTop"; kind: "danger"; iconName: "lungs"; text: win.phone ? "" : qsTr("Atemnot-Hilfe")
                              onClicked: win.quickHelp() }
                }
            }
            SleepBar { visible: !win.wide && shown; Layout.fillWidth: true; Layout.margins: 8 }
            Rectangle {
                visible: app.demo
                Layout.fillWidth: true
                implicitHeight: demoLabel.implicitHeight + 12
                color: Theme.accentWash
                Label { id: demoLabel; anchors.centerIn: parent; width: parent.width - 24; horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap; color: Theme.ink; font.pixelSize: Theme.fsS
                        text: qsTr("Beispieldaten – Änderungen werden beim nächsten Ausprobieren verworfen.") }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: home.tab
                ExercisesView {}
                HistoryView {}
                EventsView {}
                AlarmsView {}
                SettingsView {}
            }

            // Leiste unten (Smartphone/Tablet hoch)
            Rectangle {
                visible: !win.wide
                Layout.fillWidth: true
                implicitHeight: 66
                color: Theme.surface
                Rectangle { width: parent.width; height: 1; color: Theme.rule }
                RowLayout {
                    anchors.fill: parent
                    spacing: 0
                    Repeater {
                        model: home.sections
                        delegate: AbstractButton {
                            objectName: "tab" + index
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            readonly property bool current: index === home.tab
                            onClicked: home.tab = index
                            background: Rectangle { color: parent.down ? Theme.surface2 : "transparent" }
                            contentItem: ColumnLayout {
                                spacing: 2
                                Item { Layout.fillHeight: true }
                                Rectangle {
                                    Layout.alignment: Qt.AlignHCenter
                                    implicitWidth: 56; implicitHeight: 30; radius: 15
                                    color: current ? Theme.brandWash : "transparent"
                                    Image { anchors.centerIn: parent; source: Theme.icon(modelData.icon, current ? Theme.brand : Theme.ink2)
                                            sourceSize.width: 24; sourceSize.height: 24 }
                                }
                                Label { text: modelData.short; Layout.alignment: Qt.AlignHCenter; font.pixelSize: Theme.fsXs
                                        color: current ? Theme.ink : Theme.muted; font.bold: current }
                                Item { Layout.fillHeight: true }
                            }
                        }
                    }
                }
            }
        }
    }
}
