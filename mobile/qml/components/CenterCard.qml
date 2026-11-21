// Mittige Karte (Anmeldung, Einrichtung); rollbar, wenn die Tastatur oder ein kleiner Bildschirm drängt
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Flickable {
    id: f
    default property alias content: col.data
    property int cardWidth: 560
    anchors.fill: parent
    clip: true
    contentWidth: width
    contentHeight: Math.max(height, card.height + 48)
    boundsBehavior: Flickable.StopAtBounds
    Rectangle {
        id: card
        width: Math.min(f.cardWidth, f.width - 24)
        height: col.implicitHeight + 2 * pad
        readonly property int pad: f.width < 600 ? 20 : 32
        x: (f.width - width) / 2
        y: Math.max(24, (f.height - height) / 2)
        radius: Theme.radius + 4
        color: Theme.surface
        border.color: Theme.rule
        ColumnLayout {
            id: col
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: card.pad }
            spacing: Theme.gap
        }
    }
}
