// ViviaBreathe – Übungstagebuch als Dokument, PDF (ohne Qt Print Support) und CSV
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include "chartrenderer.h"
#include "datatypes.h"

#include <QByteArray>
#include <QSizeF>

class QTextDocument;

namespace ReportDoc {
struct Options {
    bool chart = true;
    bool table = true;
    bool notes = true;
};
// Dokument für eine Seitengröße in Pixeln aufbauen (Grafik, Tabelle, Hinweise)
// imageScale: Grafik in höherer Auflösung zeichnen (Anzeige in Seitenbreite)
void build(QTextDocument &doc, const QSizeF &pagePx, const User &user, const ChartSpec &spec, const Options &o,
           double imageScale = 1.0);
bool writePdf(const QString &file, const User &user, const ChartSpec &spec, const Options &o);
QByteArray csv(const QList<SessionRecord> &sessions);     // UTF-8 mit BOM, Semikolon
}
