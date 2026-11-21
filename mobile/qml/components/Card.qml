// Fläche mit Rand für zusammengehörige Inhalte
import QtQuick
import QtQuick.Layouts
import ".."

Rectangle {
    id: c
    default property alias content: col.data
    property alias spacing: col.spacing
    property int pad: Theme.gap
    property color tint: "transparent"       // farbiger Rand links (z. B. Übung)
    property bool clickable: false
    signal clicked()
    Layout.fillWidth: true
    implicitHeight: col.implicitHeight + 2 * pad
    radius: Theme.radius + 2
    color: Theme.surface
    border.color: Theme.rule
    MouseArea { id: ma; anchors.fill: parent; enabled: c.clickable; onClicked: c.clicked() }
    Rectangle { anchors.fill: parent; radius: c.radius; color: Theme.ink; opacity: ma.pressed ? 0.06 : 0 }
    Rectangle { visible: c.tint.a > 0; width: 5; radius: 2; color: c.tint; anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 1 } }
    ColumnLayout {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: c.pad; leftMargin: c.pad + (c.tint.a > 0 ? 6 : 0) }
        spacing: Theme.gapS
    }
}
