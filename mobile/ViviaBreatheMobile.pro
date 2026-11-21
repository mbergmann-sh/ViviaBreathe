# ---------------------------------------------------------------------------
#  ViviaBreathe mobil – Touch-Oberfläche (Qt Quick) für Android-Smartphones und -Tablets
#  Copyright (c) 2026 Michael Bergmann – PolyForm Noncommercial License 1.0.0 (siehe ../LICENSE)
#
#  Verwendet denselben Kern wie das Desktop-Programm (../core.pri): verschlüsselte Datenbank,
#  Übungsmodule, Taktgeber, Verlaufsgrafik, Bericht (PDF/CSV) und Abgleich über die eigene Website.
#
#  Bauen:  Qt Creator → Projekt öffnen → Kit "Android Qt 6.x arm64-v8a" wählen → Ausführen
#          (Anleitung: mobile/README.md). Am Desktop lässt sich die App zum Testen mit einem
#          normalen Kit bauen und starten (Fenster in Telefon- oder Tablet-Größe).
#  Benötigt Qt >= 6.4 (Android: >= 6.10) mit: quick quickcontrols2 svg network multimedia
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
# ---------------------------------------------------------------------------

TARGET = ViviaBreathe
include(mobile.pri)
