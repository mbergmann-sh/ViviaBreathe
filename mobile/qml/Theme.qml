// ViviaBreathe mobil – Farben, Schrift und Maße (ruhig, kontrastreich, gut lesbar; Farben wie am Desktop)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
pragma Singleton
import QtQuick

QtObject {
    readonly property bool dark: app.dark

    // Farben
    readonly property color paper:     dark ? "#0F171D" : "#F3F6F8"
    readonly property color surface:   dark ? "#16212A" : "#FFFFFF"
    readonly property color surface2:  dark ? "#1C2933" : "#F8FAFB"
    readonly property color ink:       dark ? "#E6EDF2" : "#16222B"
    readonly property color ink2:      dark ? "#B8C4CD" : "#45535E"
    readonly property color muted:     dark ? "#93A1AC" : "#6B7782"
    readonly property color rule:      dark ? "#26333D" : "#D9E0E6"
    readonly property color ruleStrong: dark ? "#3A4955" : "#B9C4CD"
    readonly property color brand:     dark ? "#4A9EE0" : "#1474BC"
    readonly property color brandInk:  dark ? "#0E171D" : "#FFFFFF"
    readonly property color brandWash: dark ? "#183247" : "#E3EFF8"
    readonly property color accent:    dark ? "#F58A3C" : "#E0620D"
    readonly property color accentWash: dark ? "#3A2616" : "#FCEBDD"
    readonly property color alert:     dark ? "#F2867B" : "#B3261E"
    readonly property color alertWash: dark ? "#3A1D1A" : "#FBE9E7"
    readonly property color ok:        dark ? "#6CCB9B" : "#1E7A4C"

    // Symbol aus resources/icons/ui in der passenden Farbe
    function icon(name, color) {
        const c = String(color || ink).replace("#", "")
        return "image://icon/" + name + "?c=" + c.slice(c.length - 6) + "&a=" + String(accent).replace("#", "").slice(-6)
    }
    function illustration(name) { return "image://illus/" + name + "?dark=" + (dark ? 1 : 0) }

    // Schrift (Atkinson Hyperlegible – für gute Lesbarkeit entworfen)
    readonly property string family: "Atkinson Hyperlegible"
    readonly property int fsXs: 14
    readonly property int fsS: 15
    readonly property int fsM: 17
    readonly property int fsL: 20
    readonly property int fsXl: 26
    readonly property int fsXxl: 34
    readonly property int fsHuge: 54

    // Maße
    readonly property int touch: 52          // Mindesthöhe von Bedienelementen
    readonly property int radius: 10
    readonly property int gap: 16
    readonly property int gapS: 8
    readonly property int gapL: 24
    readonly property int railWidth: 260
    readonly property int maxContent: 860    // breite Tablets: Inhalt nicht über die ganze Breite
}
