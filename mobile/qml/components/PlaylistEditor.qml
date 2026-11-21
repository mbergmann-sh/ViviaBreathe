// Wiedergabeliste: MP3-Dateien hinzufügen (Android-Dateiauswahl), entfernen, Reihenfolge
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ".."

ColumnLayout {
    id: p
    property var files: []          // [{file, name}]
    property string emptyText: qsTr("Keine Musik gewählt")
    spacing: 6
    function add(urls) {
        const l = p.files.slice()
        for (let i = 0; i < urls.length; ++i) {
            const u = String(urls[i])
            const name = app.rememberFile(u)
            if (!l.some(x => x.file === u)) l.push({ file: u, name: name })
        }
        p.files = l
    }
    function remove(i) { const l = p.files.slice(); l.splice(i, 1); p.files = l }
    function move(i, d) {
        const j = i + d
        if (j < 0 || j >= p.files.length) return
        const l = p.files.slice(); const x = l[i]; l[i] = l[j]; l[j] = x; p.files = l
    }
    Label { visible: p.files.length === 0; text: p.emptyText; color: Theme.muted; font.family: Theme.family; wrapMode: Text.WordWrap
            Layout.fillWidth: true }
    Repeater {
        model: p.files
        delegate: RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Image { source: Theme.icon("music", Theme.ink2); sourceSize.width: 22; sourceSize.height: 22 }
            Label { text: modelData.name; color: Theme.ink; elide: Text.ElideMiddle; Layout.fillWidth: true; font.family: Theme.family }
            VButton { kind: "flat"; text: "↑"; visible: p.files.length > 1; enabled: index > 0; onClicked: p.move(index, -1); implicitWidth: 44 }
            VButton { kind: "flat"; iconName: "delete"; onClicked: p.remove(index); implicitWidth: 48 }
        }
    }
    VButton { text: qsTr("Musik hinzufügen …"); iconName: "add"; onClicked: dlg.open() }
    FileDialog {
        id: dlg
        title: qsTr("Musik auswählen")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("Musik (*.mp3 *.m4a *.ogg *.wav *.flac)"), qsTr("Alle Dateien (*)")]
        onAccepted: p.add(selectedFiles)
    }
}
