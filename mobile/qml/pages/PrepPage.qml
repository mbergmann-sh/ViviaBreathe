// Vorbereitung einer Übung: Tipps, Haltung, Atemphasen, Dauer, Töne, Musik, Trainingsmodus
// Alle Werte werden für den Benutzer gespeichert; „Zurücksetzen“ stellt die Standardwerte her.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../components"
import ".."

Page {
    id: page
    objectName: "prepPage"
    property string moduleId: ""
    property string presetEvent: ""
    property var info: app.moduleInfo(moduleId)
    property var s: ({})
    property var preview: ({})
    signal closed()
    signal started()
    background: Rectangle { color: Theme.paper }

    function set(k, v) { const x = Object.assign({}, s); x[k] = v; s = x; preview = app.settingsPreview(moduleId, s) }
    function setPhase(i, v) { const a = s.phaseSec.slice(); a[i] = v; set("phaseSec", a) }
    function setRepeat(i, v) { const a = s.repeats.slice(); a[i] = v; set("repeats", a) }
    function load(map) { const x = map; if (presetEvent !== "") x.eventName = presetEvent; s = x; preview = app.settingsPreview(moduleId, s) }
    Component.onCompleted: load(app.moduleSettings(moduleId))
    function save() { app.saveModuleSettings(moduleId, s) }
    function start() {
        if (app.startSession(moduleId, s)) page.started()
        else win.toast(app.lastError, true)
    }
    function goBack() { save(); return false }

    Confirm {
        id: resetQuestion
        title: qsTr("Standardwerte herstellen?")
        message: qsTr("Atemphasen, Dauer und Anlass werden auf die Standardwerte der Übung zurückgesetzt. Töne, Musik und Trainingsmodus bleiben.")
        acceptText: qsTr("Zurücksetzen")
        onAccepted: { page.load(app.defaultSettings(page.moduleId)); page.save(); win.toast(qsTr("Standardwerte hergestellt.")) }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        PageHeader {
            title: page.info.name || ""
            subtitle: page.info.category || ""
            onBack: { page.save(); page.closed() }
        }
        ScrollPage {
            id: scroll
            // ---- Beschreibung, Hinweise
            Card {
                Label { text: page.info.description || ""; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: Theme.ink }
                Rectangle {
                    visible: (page.info.emergencyNote || "") !== ""
                    Layout.fillWidth: true
                    implicitHeight: em.implicitHeight + 20
                    radius: Theme.radius; color: Theme.alertWash; border.color: Theme.alert
                    Label { id: em; anchors.fill: parent; anchors.margins: 10; wrapMode: Text.WordWrap; color: Theme.ink
                            text: page.info.emergencyNote || "" }
                }
                RowLayout {
                    visible: (page.info.posture || "") !== "" || (page.info.postureImage || "") !== ""
                    Layout.fillWidth: true
                    spacing: Theme.gap
                    Image { visible: (page.info.postureImage || "") !== "" && page.info.guide.length === 0
                            source: visible ? Theme.illustration(page.info.postureImage) : ""
                            sourceSize.width: win.phone ? 120 : 170; fillMode: Image.PreserveAspectFit }
                    Label { text: qsTr("Haltung: %1").arg(page.info.posture || ""); visible: (page.info.posture || "") !== ""
                            wrapMode: Text.WordWrap; Layout.fillWidth: true; color: Theme.ink2 }
                }
                VButton { visible: page.info.guide && page.info.guide.length > 0; iconName: "help"; text: qsTr("Bebilderte Anleitung")
                          onClicked: win.openGuide(page.moduleId) }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    VSwitch { text: qsTr("Tipps anzeigen"); checked: !!page.s.showTips; onToggled: page.set("showTips", checked) }
                    Repeater {
                        model: page.s.showTips ? (page.info.tips || []).concat(page.info.generalTips || []) : []
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Label { text: "•"; color: Theme.brand; Layout.alignment: Qt.AlignTop }
                            Label { text: modelData; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: Theme.ink2; font.pixelSize: Theme.fsS }
                        }
                    }
                }
            }

            // ---- Atemphasen und Dauer
            Card {
                objectName: "phasesCard"
                SectionTitle { text: page.info.singleRound ? qsTr("Ablauf") : qsTr("Atemphasen") }
                Flow {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: implicitHeight
                    spacing: Theme.gap
                    Repeater {
                        model: page.info.phases || []
                        delegate: VStepper {
                            width: win.phone ? scroll.width - 60 : 250
                            label: modelData.name
                            hint: modelData.hint
                            from: modelData.min; to: modelData.max; step: 0.5; decimals: 1
                            suffix: " s"
                            value: page.s.phaseSec ? page.s.phaseSec[index] : modelData.def
                            onEdited: (v) => page.setPhase(index, v)
                        }
                    }
                }
                Flow {
                    visible: page.info.hasSegments || page.info.singleRound
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: implicitHeight
                    spacing: Theme.gap
                    Repeater {
                        model: page.info.segments || []
                        delegate: VStepper {
                            visible: modelData.max > modelData.min
                            width: win.phone ? scroll.width - 60 : 250
                            label: page.info.singleRound ? qsTr("Wiederholungen") : qsTr("%1 – Wiederholungen").arg(modelData.title)
                            from: modelData.min; to: modelData.max; step: 1
                            value: page.s.repeats ? page.s.repeats[index] : modelData.def
                            onEdited: (v) => page.setRepeat(index, v)
                        }
                    }
                }
                VStepper {
                    visible: !page.info.singleRound
                    label: qsTr("Gesamtdauer")
                    from: Math.round((page.info.minTotalSec || 60) / 60); to: Math.round((page.info.maxTotalSec || 3600) / 60); step: 1
                    suffix: qsTr(" min")
                    value: Math.round((page.s.totalSec || 300) / 60)
                    onEdited: (v) => page.set("totalSec", v * 60)
                }
                VCombo {
                    visible: !!page.info.deviceTraining
                    label: qsTr("Ziel mit dem Atemtrainer")
                    Layout.fillWidth: true
                    model: [qsTr("1 Ball (600 ml/s)"), qsTr("2 Bälle (900 ml/s)"), qsTr("3 Bälle (1200 ml/s)")]
                    currentIndex: (page.s.deviceTarget || 1) - 1
                    onActivated: (i) => page.set("deviceTarget", i + 1)
                }
                Label { text: page.preview.text || ""; color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                VButton { text: qsTr("Standardwerte"); iconName: "reset"; kind: "flat"; onClicked: resetQuestion.open() }
            }

            // ---- Anlass
            Card {
                SectionTitle { text: qsTr("Anlass") }
                VField { id: eventField; text: page.s.eventName || ""; placeholder: qsTr("z. B. Einschlafübung"); Layout.fillWidth: true
                         onTextChanged: if (text !== (page.s.eventName || "")) page.set("eventName", text) }
                Flow {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: implicitHeight
                    spacing: 6
                    Repeater {
                        model: app.eventNames().slice(0, 8)
                        VChip { text: modelData; checkable: false; checked: modelData === page.s.eventName; font.pixelSize: Theme.fsS
                                onClicked: eventField.text = modelData }
                    }
                }
            }

            // ---- Töne und Musik
            Card {
                objectName: "soundCard"
                SectionTitle { text: qsTr("Töne und Musik") }
                VSwitch { text: qsTr("Signalton beim Phasenwechsel"); sub: qsTr("So können Sie die Augen schließen.")
                          checked: !!page.s.signal; onToggled: page.set("signal", checked) }
                VSwitch { text: qsTr("Unterschiedliche Töne je Phase"); enabled: !!page.s.signal
                          checked: !!page.s.distinctTones; onToggled: page.set("distinctTones", checked) }
                VSwitch { text: qsTr("Gong am Ende der Übung"); checked: !!page.s.doneGong; onToggled: page.set("doneGong", checked) }
                VSlider { label: qsTr("Lautstärke der Signaltöne"); Layout.fillWidth: true; enabled: !!page.s.signal || !!page.s.doneGong
                          value: page.s.signalVolume || 70; onMoved: page.set("signalVolume", Math.round(value)) }
                VSwitch { visible: app.vibration; text: qsTr("Vibration beim Phasenwechsel"); checked: !!page.s.vibrate
                          onToggled: page.set("vibrate", checked) }
                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.rule }
                VSwitch { text: qsTr("Hintergrundmusik"); sub: page.s.musicName ? page.s.musicName : qsTr("MP3-Datei auf diesem Gerät")
                          enabled: app.multimedia
                          checked: !!page.s.musicEnabled
                          onToggled: { page.set("musicEnabled", checked); if (checked && !page.s.musicFile) musicDlg.open() } }
                RowLayout {
                    visible: !!page.s.musicEnabled
                    Layout.fillWidth: true
                    VButton { text: page.s.musicFile ? qsTr("Andere Musik …") : qsTr("Musik wählen …"); iconName: "music"; onClicked: musicDlg.open() }
                }
                VSlider { visible: !!page.s.musicEnabled; label: qsTr("Lautstärke der Musik"); Layout.fillWidth: true
                          value: page.s.musicVolume || 40; onMoved: page.set("musicVolume", Math.round(value)) }
            }

            // ---- Trainingsmodus
            Card {
                SectionTitle { text: qsTr("Trainingsmodus") }
                VSwitch { text: qsTr("Vor und nach der Übung messen"); sub: qsTr("SpO₂, Peak Flow, Puls und Befinden – für den Vergleich im Verlauf.")
                          checked: !!page.s.trainingMode; onToggled: page.set("trainingMode", checked) }
                Flow {
                    visible: !!page.s.trainingMode
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: implicitHeight
                    spacing: 6
                    VChip { text: qsTr("SpO₂"); checkable: false; checked: !!page.s.askSpo2; onClicked: page.set("askSpo2", !page.s.askSpo2) }
                    VChip { text: qsTr("Peak Flow"); checkable: false; checked: !!page.s.askPeakFlow; onClicked: page.set("askPeakFlow", !page.s.askPeakFlow) }
                    VChip { text: qsTr("Puls"); checkable: false; checked: !!page.s.askPulse; onClicked: page.set("askPulse", !page.s.askPulse) }
                    VChip { text: qsTr("Befinden"); checkable: false; checked: !!page.s.askFeeling; onClicked: page.set("askFeeling", !page.s.askFeeling) }
                }
            }
            Item { height: 8 }
        }
        // ---- Starten (immer sichtbar)
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 76
            color: Theme.surface
            Rectangle { width: parent.width; height: 1; color: Theme.rule }
            VButton {
                objectName: "startButton"
                anchors.centerIn: parent
                width: Math.min(parent.width - 32, 420)
                kind: "primary"; iconName: "play"; text: qsTr("Übung starten")
                onClicked: page.start()
            }
        }
    }
    FileDialog {
        id: musicDlg
        title: qsTr("Hintergrundmusik auswählen")
        nameFilters: [qsTr("Musik (*.mp3 *.m4a *.ogg *.wav *.flac)"), qsTr("Alle Dateien (*)")]
        onAccepted: {
            const u = String(selectedFile)
            const n = app.rememberFile(u)
            const x = Object.assign({}, page.s); x.musicFile = u; x.musicName = n; x.musicEnabled = true; page.s = x
        }
        onRejected: if (!page.s.musicFile) page.set("musicEnabled", false)
    }
}
