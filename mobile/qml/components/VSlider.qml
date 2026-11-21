// Schieberegler mit Beschriftung und Wert (z. B. Lautstärke)
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

ColumnLayout {
    id: s
    property string label: ""
    property alias value: sl.value
    property alias from: sl.from
    property alias to: sl.to
    property string suffix: " %"
    signal moved()
    spacing: 2
    opacity: enabled ? 1 : 0.5
    RowLayout {
        Layout.fillWidth: true
        Label { text: s.label; font.family: Theme.family; font.pixelSize: Theme.fsS; color: Theme.ink2; Layout.fillWidth: true; wrapMode: Text.WordWrap }
        Label { text: Math.round(sl.value) + s.suffix; font.family: Theme.family; font.pixelSize: Theme.fsS; color: Theme.ink }
    }
    Slider {
        id: sl
        Layout.fillWidth: true
        from: 0; to: 100; stepSize: 1
        implicitHeight: 40
        onMoved: s.moved()
        background: Rectangle {
            x: sl.leftPadding; y: sl.topPadding + sl.availableHeight / 2 - height / 2
            width: sl.availableWidth; height: 6; radius: 3; color: Theme.rule
            Rectangle { width: sl.visualPosition * parent.width; height: parent.height; radius: 3; color: Theme.brand }
        }
        handle: Rectangle {
            x: sl.leftPadding + sl.visualPosition * (sl.availableWidth - width)
            y: sl.topPadding + sl.availableHeight / 2 - height / 2
            width: 28; height: 28; radius: 14
            color: sl.pressed ? Theme.brandWash : Theme.surface
            border.color: Theme.brand; border.width: 2
        }
    }
}
