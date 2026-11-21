// Zeile mit Schaltflächen am Seitenende: rechtsbündig, bricht auf schmalen Bildschirmen um
import QtQuick
import QtQuick.Layouts
import ".."

Flow {
    Layout.fillWidth: true
    Layout.preferredWidth: 200            // Breite vom Layout, nicht aus den Knöpfen (Layout-Schleife)
    Layout.preferredHeight: implicitHeight
    spacing: Theme.gapS
    layoutDirection: Qt.RightToLeft
}
