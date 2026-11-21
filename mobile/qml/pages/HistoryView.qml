// Bereich „Verlauf“: Tagebuch als Liste oder Grafik, Zeitraum, Übung, Ausgabe als PDF/CSV
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ViviaBreathe
import "../components"
import ".."

Item {
    id: view
    objectName: "historyView"
    property int range: 1            // 0: 7 Tage, 1: 30 Tage, 2: 90 Tage, 3: 1 Jahr
    property bool chart: false
    property string moduleId: ""
    property var list: []
    property var stats: ({})
    readonly property var days: [7, 30, 90, 365]
    readonly property string fromIso: app.todayIso(-(days[range] - 1))
    readonly property string toIso: app.todayIso(0)
    property var mods: []
    function reload() {
        list = app.sessions(fromIso, toIso, moduleId)
        stats = app.sessionStats(fromIso, toIso, moduleId)
        mods = [{ id: "", name: qsTr("Alle Übungen") }].concat(app.modules().map(m => ({ id: m.id, name: m.shortName })))
    }
    onRangeChanged: reload()
    onModuleIdChanged: reload()
    Component.onCompleted: reload()
    Connections { target: app; function onRevisionChanged() { view.reload() } function onUserChanged() { view.reload() } }
    Component { id: editComp; SessionEditPage { onClosed: (msg) => { stack.pop(); if (msg) win.toast(msg) } } }

    Popup {
        id: sharePopup
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(480, view.width - 32)
        padding: 20
        background: Rectangle { radius: Theme.radius + 4; color: Theme.surface; border.color: Theme.rule }
        Overlay.modal: Rectangle { color: "#80000000" }
        contentItem: ColumnLayout {
            spacing: Theme.gapS
            SectionTitle { text: qsTr("Tagebuch weitergeben") }
            Label { text: qsTr("%1 bis %2").arg(app.dateText(view.fromIso)).arg(app.dateText(view.toIso)); color: Theme.ink2 }
            VSwitch { id: withChart; text: qsTr("mit Verlaufsgrafik"); checked: true }
            VSwitch { id: withNotes; text: qsTr("mit Notizen"); checked: true }
            VButton { kind: "primary"; iconName: "pdf"; text: qsTr("Als PDF teilen …"); Layout.fillWidth: true
                      onClicked: { sharePopup.close(); if (!app.sharePdf(view.fromIso, view.toIso, view.moduleId, withChart.checked, withNotes.checked)) win.toast(app.lastError, true) } }
            VButton { iconName: "history"; text: qsTr("Als Tabelle (CSV) teilen …"); Layout.fillWidth: true
                      onClicked: { sharePopup.close(); if (!app.shareCsv(view.fromIso, view.toIso, view.moduleId)) win.toast(app.lastError, true) } }
            VButton { kind: "flat"; text: qsTr("Abbrechen"); Layout.alignment: Qt.AlignRight; onClicked: sharePopup.close() }
        }
    }

    ScrollPage {
        anchors.fill: parent
        maxWidth: 1000
        // Auswahl
        Flow {
            Layout.fillWidth: true
            Layout.preferredWidth: 200
            Layout.preferredHeight: implicitHeight
            spacing: 6
            Repeater {
                model: [qsTr("7 Tage"), qsTr("30 Tage"), qsTr("90 Tage"), qsTr("1 Jahr")]
                VChip { text: modelData; checkable: false; checked: view.range === index; onClicked: view.range = index }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.gapS
            VCombo { Layout.fillWidth: true; model: view.mods; textRole: "name"
                     currentIndex: Math.max(0, view.mods.findIndex(m => m.id === view.moduleId))
                     onActivated: (i) => view.moduleId = view.mods[i].id }
            VButton { objectName: "chartToggle"; iconName: view.chart ? "history" : "chart"; text: win.phone ? "" : (view.chart ? qsTr("Liste") : qsTr("Grafik"))
                      onClicked: view.chart = !view.chart }
            VButton { objectName: "shareButton"; iconName: "pdf"; text: win.phone ? "" : qsTr("Teilen …"); onClicked: sharePopup.open() }
        }
        Label { text: view.stats.text || ""; color: Theme.ink2; wrapMode: Text.WordWrap; Layout.fillWidth: true }

        // Grafik
        Rectangle {
            visible: view.chart
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(320, 90 + 150 * chartItem.panelCount)
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.rule
            Chart {
                id: chartItem
                objectName: "chart"
                anchors.fill: parent
                anchors.margins: 6
                userId: app.userId
                fromDate: view.fromIso
                toDate: view.toIso
                moduleId: view.moduleId
                revision: app.revision
                background: Theme.surface
                foreground: Theme.ink
            }
        }

        // Liste
        Label { visible: !view.chart && view.list.length === 0; text: qsTr("Im gewählten Zeitraum gibt es keine Einträge.")
                color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        Repeater {
            model: view.chart ? [] : view.list
            delegate: ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Label { visible: index === 0 || view.list[index - 1].day !== modelData.day; text: modelData.dayText
                        font.bold: true; color: Theme.ink2; Layout.topMargin: index === 0 ? 0 : 8 }
                Card {
                    tint: modelData.color
                    pad: 12
                    clickable: true
                    onClicked: stack.push(editComp, { sessionId: modelData.id })
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: modelData.time; color: Theme.muted }
                        Label { text: modelData.module; font.bold: true; color: Theme.ink; elide: Text.ElideRight; Layout.fillWidth: true }
                        Label { text: modelData.duration + (modelData.completed ? "" : " *"); color: Theme.ink2 }
                    }
                    Label { visible: modelData.event !== ""; text: modelData.event; color: Theme.ink2; font.pixelSize: Theme.fsS
                            Layout.fillWidth: true; elide: Text.ElideRight }
                    Label { visible: modelData.measurements !== ""; text: modelData.measurements; color: Theme.ink2; font.pixelSize: Theme.fsS
                            wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Label { visible: modelData.device !== ""; text: qsTr("Atemtrainer: %1").arg(modelData.device); color: Theme.ink2
                            font.pixelSize: Theme.fsS; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                    Label { visible: modelData.note !== ""; text: modelData.note; color: Theme.muted; font.italic: true; font.pixelSize: Theme.fsS
                            wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
            }
        }
        Label { visible: !view.chart && view.list.length > 0; text: qsTr("* vorzeitig beendet"); color: Theme.muted; font.pixelSize: Theme.fsXs }
    }
}
