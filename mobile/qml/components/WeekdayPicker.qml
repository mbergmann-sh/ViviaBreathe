// Wochentage als Bitmaske (Bit 0 = Montag … Bit 6 = Sonntag) mit Schnellwahl
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

ColumnLayout {
    id: w
    property int mask: 0x7F
    property string label: qsTr("Wochentage")
    readonly property var names: [qsTr("Mo"), qsTr("Di"), qsTr("Mi"), qsTr("Do"), qsTr("Fr"), qsTr("Sa"), qsTr("So")]
    spacing: 6
    Label { text: w.label; font.family: Theme.family; font.pixelSize: Theme.fsS; color: Theme.ink2 }
    Flow {
        Layout.fillWidth: true
        Layout.preferredWidth: 200
        Layout.preferredHeight: implicitHeight
        spacing: 6
        Repeater {
            model: 7
            VChip { text: w.names[index]; checkable: false; checked: (w.mask >> index) & 1
                    width: Math.max(implicitWidth, 54)
                    onClicked: w.mask = w.mask ^ (1 << index) }
        }
    }
    Flow {
        Layout.fillWidth: true
        Layout.preferredWidth: 200
        Layout.preferredHeight: implicitHeight
        spacing: 6
        VButton { kind: "flat"; text: qsTr("täglich"); onClicked: w.mask = 0x7F }
        VButton { kind: "flat"; text: qsTr("werktags"); onClicked: w.mask = 0x1F }
        VButton { kind: "flat"; text: qsTr("am Wochenende"); onClicked: w.mask = 0x60 }
    }
}
