// Symbol und Name der App
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

RowLayout {
    property int size: 56
    spacing: 14
    Image { source: "qrc:/icons/viviabreathe.png"; sourceSize.width: parent.size; sourceSize.height: parent.size }
    ColumnLayout {
        spacing: 0
        Label { text: app.appName; font.family: Theme.family; font.pixelSize: Theme.fsXxl; font.bold: true; color: Theme.ink }
        Label { text: qsTr("Geführte Atemübungen · Übungstagebuch"); font.family: Theme.family; font.pixelSize: Theme.fsS
                color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    }
}
