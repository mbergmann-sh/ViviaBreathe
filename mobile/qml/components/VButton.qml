// Schaltfläche: "primary" (gefüllt), "ghost" (umrandet), "danger", "accent", "flat" (nur Text)
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Button {
    id: b
    property string kind: "ghost"
    property string iconName: ""            // Symbol aus resources/icons/ui
    property string iconText: ""
    implicitHeight: Theme.touch
    // in Zeilen darf die Schaltfläche schmaler werden (Text wird gekürzt) – nie breiter als der Bildschirm
    Layout.minimumWidth: Math.min(implicitWidth, Theme.touch + 40)
    leftPadding: text === "" ? 12 : 18; rightPadding: text === "" ? 12 : 18
    font.family: Theme.family
    font.pixelSize: Theme.fsM
    font.bold: kind === "primary" || kind === "danger" || kind === "accent"
    readonly property color fg: !b.enabled ? Theme.muted
                              : b.kind === "primary" ? Theme.brandInk
                              : (b.kind === "danger" || b.kind === "accent") ? "#FFFFFF"
                              : b.kind === "flat" ? Theme.brand : Theme.ink
    contentItem: Item {
        // natürliche Breite aus den Bestandteilen (nicht aus der gekürzten Beschriftung – sonst Kreisbezug)
        implicitWidth: (b.iconName !== "" ? 24 : 0) + (b.iconText !== "" ? iconLabel.implicitWidth : 0)
                       + (b.text !== "" ? label.implicitWidth : 0)
                       + 8 * (((b.iconName !== "") ? 1 : 0) + ((b.iconText !== "") ? 1 : 0) + ((b.text !== "") ? 1 : 0) - 1)
        implicitHeight: row.implicitHeight
        Row {
            id: row
            spacing: 8
            anchors.centerIn: parent
            width: Math.min(implicitWidth, parent.width)
            Image {
                visible: b.iconName !== ""
                source: b.iconName !== "" ? Theme.icon(b.iconName, b.fg) : ""
                sourceSize.width: 24; sourceSize.height: 24
                width: 24; height: 24
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: iconLabel
                visible: b.iconText !== ""
                text: b.iconText
                font.pixelSize: b.font.pixelSize + 2
                color: b.fg
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: label
                visible: b.text !== ""
                text: b.text
                font: b.font
                color: b.fg
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideRight
                width: Math.min(implicitWidth, Math.max(20, b.availableWidth - (b.iconName !== "" ? 32 : 0) - (b.iconText !== "" ? 30 : 0)))
            }
        }
    }
    background: Rectangle {
        radius: Theme.radius
        color: b.kind === "primary" ? (b.enabled ? (b.down ? Qt.darker(Theme.brand, 1.15) : Theme.brand) : Theme.rule)
             : b.kind === "danger" ? (b.down ? Qt.darker(Theme.alert, 1.15) : Theme.alert)
             : b.kind === "accent" ? (b.down ? Qt.darker(Theme.accent, 1.15) : Theme.accent)
             : b.kind === "flat" ? (b.down ? Theme.brandWash : "transparent")
             : (b.down || b.checked ? Theme.brandWash : Theme.surface)
        border.width: b.kind === "ghost" ? 1 : 0
        border.color: b.checked ? Theme.brand : Theme.ruleStrong
    }
}
