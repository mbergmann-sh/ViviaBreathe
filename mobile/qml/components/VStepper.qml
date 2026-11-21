// Zahl mit „−“ und „+“ (große Trefferflächen statt eines kleinen Drehfelds); auch Kommazahlen.
// Der Wert wird nicht selbst gesetzt: onEdited: (v) => meinWert = v
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

ColumnLayout {
    id: s
    property string label: ""
    property string hint: ""
    property real value: 1
    property real from: 1
    property real to: 99
    property real step: 1
    property int decimals: 0
    property string suffix: ""
    property string text: Number(value).toLocaleString(Qt.locale(), "f", decimals) + suffix
    signal edited(real v)          // neuer Wert – der Aufrufer übernimmt ihn (die Bindung von 'value' bleibt)
    spacing: 4
    opacity: enabled ? 1 : 0.5
    function set(v) {
        v = Math.round(v / s.step) * s.step
        v = Math.max(s.from, Math.min(s.to, v))
        if (Math.abs(v - s.value) > 1e-9) s.edited(v)
    }
    Label { visible: s.label !== ""; text: s.label; font.family: Theme.family; font.pixelSize: Theme.fsS; color: Theme.ink2
            Layout.fillWidth: true; wrapMode: Text.WordWrap }
    RowLayout {
        spacing: 0
        VButton { text: "−"; implicitWidth: Theme.touch + 4; enabled: s.value > s.from + 1e-9; onClicked: s.set(s.value - s.step)
                  autoRepeat: true }
        Label {
            text: s.text
            horizontalAlignment: Text.AlignHCenter
            Layout.preferredWidth: Math.max(96, implicitWidth + 20)
            font.family: Theme.family; font.pixelSize: Theme.fsL; font.bold: true; color: Theme.ink
        }
        VButton { text: "+"; implicitWidth: Theme.touch + 4; enabled: s.value < s.to - 1e-9; onClicked: s.set(s.value + s.step)
                  autoRepeat: true }
    }
    Label { visible: s.hint !== ""; text: s.hint; font.family: Theme.family; font.pixelSize: Theme.fsXs; color: Theme.muted
            wrapMode: Text.WordWrap; Layout.fillWidth: true }
}
