// Hinweis, solange Einschlafmusik läuft: Restzeit und „Aus“
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Rectangle {
    id: bar
    readonly property bool shown: app.playback.running && app.playback.mode === "sleep"
    visible: shown
    implicitHeight: row.implicitHeight + 16
    radius: Theme.radius
    color: Theme.brandWash
    border.color: Theme.brand
    function minutes(sec) { return Math.ceil(sec / 60) }
    RowLayout {
        id: row
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8
        Image { source: Theme.icon("moon", Theme.brand); sourceSize.width: 24; sourceSize.height: 24 }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0
            Label { text: app.playback.title || qsTr("Einschlafen"); font.bold: true; color: Theme.ink; elide: Text.ElideRight; Layout.fillWidth: true }
            Label { text: app.playback.remainingSec > 0 ? qsTr("noch %1 min").arg(bar.minutes(app.playback.remainingSec)) : app.playback.track
                    color: Theme.ink2; font.pixelSize: Theme.fsS; elide: Text.ElideRight; Layout.fillWidth: true }
        }
        VButton { text: qsTr("Aus"); iconName: "stop"; onClicked: app.playback.stop() }
    }
}
