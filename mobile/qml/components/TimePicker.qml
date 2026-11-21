// Uhrzeit mit großen „−/+“-Knöpfen für Stunde und Minute (Minute in 5er-Schritten, lange drücken wiederholt)
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

ColumnLayout {
    id: t
    property int hour: 7
    property int minute: 0
    property string label: qsTr("Uhrzeit")
    spacing: 4
    readonly property int btn: win.phone ? 44 : Theme.touch
    readonly property int num: win.phone ? 50 : 56
    function pad(n) { return (n < 10 ? "0" : "") + n }
    Label { text: t.label; font.family: Theme.family; font.pixelSize: Theme.fsS; color: Theme.ink2 }
    RowLayout {
        spacing: 4
        VButton { text: "−"; implicitWidth: t.btn; autoRepeat: true; onClicked: t.hour = (t.hour + 23) % 24 }
        Label { text: t.pad(t.hour); font.family: Theme.family; font.pixelSize: Theme.fsXxl; font.bold: true; color: Theme.ink
                horizontalAlignment: Text.AlignHCenter; Layout.preferredWidth: t.num }
        VButton { text: "+"; implicitWidth: t.btn; autoRepeat: true; onClicked: t.hour = (t.hour + 1) % 24 }
        Label { text: ":"; font.pixelSize: Theme.fsXxl; font.bold: true; color: Theme.ink }
        VButton { text: "−"; implicitWidth: t.btn; autoRepeat: true; onClicked: t.minute = (Math.ceil(t.minute / 5) * 5 + 55) % 60 }
        Label { text: t.pad(t.minute); font.family: Theme.family; font.pixelSize: Theme.fsXxl; font.bold: true; color: Theme.ink
                horizontalAlignment: Text.AlignHCenter; Layout.preferredWidth: t.num }
        VButton { text: "+"; implicitWidth: t.btn; autoRepeat: true; onClicked: t.minute = (Math.floor(t.minute / 5) * 5 + 5) % 60 }
    }
}
