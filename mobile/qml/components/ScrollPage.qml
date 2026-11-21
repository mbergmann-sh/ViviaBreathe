// Rollbarer Seiteninhalt; auf breiten Bildschirmen mittig mit begrenzter Breite
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Flickable {
    id: f
    default property alias content: col.data
    property alias spacing: col.spacing
    property int maxWidth: Theme.maxContent
    Layout.fillWidth: true
    Layout.fillHeight: true
    clip: true
    contentWidth: width
    contentHeight: col.implicitHeight + 2 * Theme.gap + 40
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar { policy: f.contentHeight > f.height ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff }
    ColumnLayout {
        id: col
        width: Math.min(f.width - 2 * (win.phone ? 14 : Theme.gap), f.maxWidth)
        x: (f.width - width) / 2
        y: Theme.gap
        spacing: Theme.gap
    }
}
