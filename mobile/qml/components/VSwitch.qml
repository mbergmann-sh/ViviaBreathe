// Ein/Aus mit Text (und optionaler Erklärung), ganze Zeile antippbar; Text bricht um
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

AbstractButton {
    id: s
    checkable: true
    property string sub: ""
    Layout.fillWidth: true
    Layout.minimumWidth: 0
    implicitHeight: Math.max(Theme.touch, col.implicitHeight + 12)
    font.family: Theme.family
    font.pixelSize: Theme.fsM
    opacity: enabled ? 1 : 0.5
    contentItem: RowLayout {
        spacing: Theme.gap
        ColumnLayout {
            id: col
            Layout.fillWidth: true
            spacing: 2
            Label { text: s.text; font: s.font; color: Theme.ink; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { visible: s.sub !== ""; text: s.sub; font.family: Theme.family; font.pixelSize: Theme.fsXs; color: Theme.muted
                    wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 52; implicitHeight: 30; radius: 15
            color: s.checked ? Theme.brand : Theme.rule
            border.color: s.checked ? Theme.brand : Theme.ruleStrong
            Rectangle {
                width: 24; height: 24; radius: 12
                y: 3; x: s.checked ? parent.width - width - 3 : 3
                color: s.checked ? Theme.brandInk : Theme.surface
                border.color: Theme.ruleStrong
                Behavior on x { NumberAnimation { duration: 120 } }
            }
        }
    }
    background: Item {}
}
