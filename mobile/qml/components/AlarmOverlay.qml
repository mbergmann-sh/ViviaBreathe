// Wecker klingelt: Uhrzeit, Bezeichnung, große Knöpfe „Aus“ und „Schlummern“
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Rectangle {
    id: o
    signal wakeExercise(string moduleId)
    visible: app.playback.running && app.playback.mode === "alarm"
    color: Theme.paper
    property string now: ""
    property var alarmInfo: ({})
    onVisibleChanged: if (visible) { o.alarmInfo = app.loggedIn ? app.alarm(app.playback.refId) : ({}); tick() }
    function tick() { o.now = Qt.formatTime(new Date(), "HH:mm") }
    Timer { interval: 5000; repeat: true; running: o.visible; onTriggered: o.tick() }
    MouseArea { anchors.fill: parent }          // nichts darunter antippen
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(520, parent.width - 40)
        spacing: Theme.gapL
        Image { Layout.alignment: Qt.AlignHCenter; source: Theme.icon("alarm", Theme.accent); sourceSize.width: 96; sourceSize.height: 96 }
        Label { Layout.alignment: Qt.AlignHCenter; text: o.now; font.pixelSize: 72; font.bold: true; color: Theme.ink }
        Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap
                text: app.playback.title; font.pixelSize: Theme.fsXl; color: Theme.ink }
        Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; elide: Text.ElideMiddle; visible: text !== ""
                text: app.playback.track; color: Theme.muted }
        VButton { objectName: "alarmOff"; Layout.fillWidth: true; implicitHeight: 72; kind: "primary"; text: qsTr("Aus"); font.pixelSize: Theme.fsXl
                  onClicked: {
                      const m = o.alarmInfo.moduleId || ""
                      app.playback.stop()
                      if (m) o.wakeExercise(m)
                  } }
        VButton { objectName: "alarmSnooze"; Layout.fillWidth: true; implicitHeight: 64; font.pixelSize: Theme.fsL
                  text: qsTr("Schlummern (%1 min)").arg(o.alarmInfo.snoozeMin || 9)
                  onClicked: app.playback.snooze() }
    }
}
