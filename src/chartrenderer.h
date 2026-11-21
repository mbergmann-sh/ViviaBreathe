// ViviaBreathe – Verlaufsgrafik der Übungseinheiten (eigene QPainter-Zeichnung wie in VitaParms,
// ohne Qt Charts – das Modul gibt es in der Open-Source-Variante nur unter GPLv3)
//
// Bereiche: Übungsminuten je Tag (Balken, nach Übung gefärbt), SpO2, Peak Flow und Puls jeweils
// vor (offene Marker, gestrichelt) und nach der Übung (gefüllte Marker). Dieselbe Zeichenroutine
// dient Bildschirm, Druck und PDF.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include "datatypes.h"

#include <QColor>
#include <QDate>
#include <QRectF>

class QPainter;

struct ChartSpec {
    QDate from, to;                    // inklusive
    QList<SessionRecord> sessions;
    bool showMinutes = true, showSpo2 = true, showPeakFlow = true, showPulse = true, showDevice = true;
};

namespace ChartRenderer {
void paint(QPainter &p, const QRectF &rect, const ChartSpec &spec, bool screen = true);
QColor moduleColor(const QString &moduleId);
int panelCount(const ChartSpec &spec);     // Zahl der Bereiche mit Daten (für die Mindesthöhe)
}
