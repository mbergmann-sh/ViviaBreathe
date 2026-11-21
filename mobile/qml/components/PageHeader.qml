// Kopfzeile einer geöffneten Seite: „Zurück“, Titel, rechts Aktionen
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Rectangle {
    id: h
    property string title: ""
    property string subtitle: ""
    property bool backVisible: true
    property string backText: win.phone ? "" : qsTr("Zurück")
    default property alias actions: actionRow.data
    signal back()
    Layout.fillWidth: true
    implicitHeight: Math.max(64, col.implicitHeight + 16)
    color: Theme.surface
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.rule }
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8; anchors.rightMargin: 12
        spacing: 6
        VButton { visible: h.backVisible; kind: "flat"; iconText: "‹"; text: h.backText; objectName: "backButton"; Layout.minimumWidth: implicitWidth
                  font.pixelSize: Theme.fsL; onClicked: h.back() }
        ColumnLayout {
            id: col
            Layout.fillWidth: true
            Layout.leftMargin: h.backVisible ? 0 : 8
            spacing: 0
            Label { text: h.title; font.family: Theme.family; font.pixelSize: win.phone ? Theme.fsL : Theme.fsXl; font.bold: true
                    color: Theme.ink; elide: Text.ElideRight; Layout.fillWidth: true }
            Label { visible: h.subtitle !== ""; text: h.subtitle; font.family: Theme.family; font.pixelSize: Theme.fsXs; color: Theme.muted
                    elide: Text.ElideRight; Layout.fillWidth: true }
        }
        Row { id: actionRow; spacing: 6; Layout.alignment: Qt.AlignVCenter }
    }
}
