// Die Übung läuft: Animation, Phase, Countdown, Pause/Weiter/Beenden, Töne und Musik
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ViviaBreathe
import "../components"
import ".."

Page {
    id: page
    objectName: "runPage"
    property bool quick: false
    readonly property var r: app.runner
    signal finished()
    signal aborted()
    background: Rectangle { color: Theme.paper }
    Component.onCompleted: r.start()
    function goBack() { stopQuestion.open(); return true }

    Connections { target: page.r; function onFinished(completed) { endTimer.interval = completed ? 2500 : 400; endTimer.start() } }
    Timer { id: endTimer; onTriggered: page.finished() }

    Confirm {
        id: stopQuestion
        title: qsTr("Übung beenden?")
        message: qsTr("Die Übung endet vorzeitig. Sie können den Eintrag danach speichern oder verwerfen.")
        acceptText: qsTr("Beenden")
        rejectText: qsTr("Weiter üben")
        onAccepted: { if (page.r.state === "preroll") { page.r.abort(); page.aborted() } else page.r.stop() }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        // Kopf: Name, Töne, Beenden
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 60
            color: Theme.surface
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.rule }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14; anchors.rightMargin: 8
                Label { text: page.r.moduleName; font.bold: true; font.pixelSize: Theme.fsL; color: Theme.ink; elide: Text.ElideRight
                        Layout.fillWidth: true }
                VButton { objectName: "soundToggle"; kind: "flat"; iconName: page.r.soundOn ? "volume" : "mute"
                          text: win.phone ? "" : (page.r.soundOn ? qsTr("Töne an") : qsTr("Töne aus"))
                          onClicked: page.r.soundOn = !page.r.soundOn }
            }
        }
        // Hinweis (Atemnot-Hilfe)
        Rectangle {
            visible: page.r.emergencyNote !== ""
            Layout.fillWidth: true
            Layout.margins: 10
            implicitHeight: note.implicitHeight + 16
            radius: Theme.radius; color: Theme.alertWash; border.color: Theme.alert
            Label { id: note; anchors.fill: parent; anchors.margins: 8; wrapMode: Text.WordWrap; color: Theme.ink; font.pixelSize: Theme.fsS
                    text: page.r.emergencyNote }
        }
        // Animation und Texte: hochkant untereinander, quer nebeneinander
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 12
            columns: page.width > page.height * 1.15 ? 2 : 1
            columnSpacing: Theme.gapL
            rowSpacing: Theme.gap
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 200
                Layout.preferredWidth: 600
                BreathView {
                    id: view
                    objectName: "breathView"
                    anchors.centerIn: parent
                    width: Math.min(parent.width, parent.height)
                    height: width
                    runner: page.r
                    foreground: Theme.ink
                    dark: Theme.dark
                    MouseArea { anchors.fill: parent; onClicked: if (page.r.state === "waiting") page.r.proceed() }
                }
                Image {
                    visible: page.r.postureImage !== "" && page.r.emergencyNote !== ""
                    anchors.left: parent.left; anchors.bottom: parent.bottom
                    source: visible ? Theme.illustration(page.r.postureImage) : ""
                    sourceSize.width: Math.min(160, parent.width * 0.3)
                    opacity: 0.9
                }
            }
            ColumnLayout {
                readonly property bool side: parent.columns === 2
                Layout.fillWidth: !side
                Layout.preferredWidth: side ? 380 : -1
                Layout.maximumWidth: side ? 420 : 100000
                Layout.alignment: Qt.AlignVCenter
                spacing: 6
                Label { objectName: "phaseTitle"; text: page.r.title; font.pixelSize: page.r.state === "preroll" ? Theme.fsHuge : Theme.fsXxl
                        font.bold: true; color: Theme.ink; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
                Label { text: page.r.phaseRemaining > 0 ? String(page.r.phaseRemaining) : " "; font.pixelSize: Theme.fsXl
                        color: Theme.accent; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                Label { text: page.r.sub; color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter; Layout.minimumHeight: 44 }
                Rectangle {
                    Layout.fillWidth: true; Layout.topMargin: 6
                    height: 6; radius: 3; color: Theme.rule
                    Rectangle { width: parent.width * page.r.totalProgress; height: parent.height; radius: 3; color: Theme.brand }
                }
                Label { text: page.r.info; color: Theme.muted; font.pixelSize: Theme.fsS; Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap }
                VSlider { visible: page.r.musicOn; label: qsTr("Musik"); Layout.fillWidth: true
                          value: page.r.musicVolume; onMoved: page.r.musicVolume = Math.round(value) }
            }
        }
        // Bedienung
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 80
            color: Theme.surface
            Rectangle { width: parent.width; height: 1; color: Theme.rule }
            RowLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width - 24, 560)
                spacing: Theme.gap
                VButton { objectName: "stopButton"; iconName: "stop"; text: qsTr("Beenden"); Layout.fillWidth: true
                          enabled: page.r.state !== "finished"; onClicked: stopQuestion.open() }
                VButton { objectName: "nextButton"; visible: page.r.state === "waiting"; kind: "accent"; iconName: "play"; text: qsTr("Weiter")
                          Layout.fillWidth: true; onClicked: page.r.proceed() }
                VButton { objectName: "pauseButton"; visible: page.r.state !== "waiting"; kind: "primary"
                          enabled: page.r.state !== "finished"
                          iconName: page.r.state === "paused" ? "play" : "pause"
                          text: page.r.state === "paused" ? qsTr("Fortsetzen") : qsTr("Pause")
                          Layout.fillWidth: true; onClicked: page.r.togglePause() }
            }
        }
    }
}
