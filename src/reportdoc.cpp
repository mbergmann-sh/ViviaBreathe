// ViviaBreathe – Übungstagebuch als Dokument (HTML/QTextDocument), PDF und CSV.
// Ohne Qt Print Support: Desktop druckt das Dokument (ReportPrinter), die mobile App schreibt
// es mit QPdfWriter als PDF.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "reportdoc.h"
#include "breathmodules.h"
#include "database.h"

#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QLocale>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QTextDocument>
#include <QUrl>

// Übersetzungskontext "ReportPrinter" (für lupdate erkennbar)
struct ReportTr { Q_DECLARE_TR_FUNCTIONS(ReportPrinter) };

namespace ReportDoc {

namespace {
QString esc(const QString &s) { return s.toHtmlEscaped(); }
} // namespace

void build(QTextDocument &doc, const QSizeF &pagePx, const User &user, const ChartSpec &spec, const Options &o, double imageScale)
{
    const QRectF page(QPointF(0, 0), pagePx);
    doc.setPageSize(page.size());
    QFont base(QStringLiteral("Sans Serif"));
    base.setPointSizeF(9);
    doc.setDefaultFont(base);

    // Kennzahlen
    int total = 0, minutes = 0, complete = 0;
    for (const SessionRecord &s : spec.sessions) {
        ++total;
        minutes += s.actualSec;
        if (s.completed) ++complete;
    }
    QString html = QStringLiteral("<html><body>");
    html += QStringLiteral("<h2 style='color:#1474BC'>%1</h2>").arg(ReportTr::tr("ViviaBreathe – Übungstagebuch"));
    html += QStringLiteral("<p><b>%1</b> &nbsp;·&nbsp; %2 %3 – %4<br/>%5</p>")
                .arg(esc(user.name), ReportTr::tr("Zeitraum:"), QLocale().toString(spec.from, QLocale::ShortFormat),
                     QLocale().toString(spec.to, QLocale::ShortFormat),
                     ReportTr::tr("%1 Übungseinheiten, davon %2 vollständig · zusammen %3 Minuten")
                         .arg(total).arg(complete).arg(minutes / 60));

    if (o.chart) {
        const int dispW = int(page.width() - 2 * doc.documentMargin() - 2);   // Anzeigebreite im Dokument
        const QSize sz(int(dispW * imageScale), int(dispW * imageScale * 0.62));   // gezeichnet in höherer Auflösung
        QImage img(sz, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        {
            QPainter p(&img);
            QFont f = base;
            f.setPixelSize(qMax(10, sz.width() / 58));
            p.setFont(f);
            p.setPen(Qt::black);
            ChartRenderer::paint(p, QRectF(QPointF(0, 0), QSizeF(sz)).adjusted(4, 4, -4, -4), spec, false);
        }
        doc.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("vb://chart")), img);
        html += QStringLiteral("<p><img src='vb://chart' width='%1' height='%2'/></p>").arg(dispW).arg(int(dispW * 0.62));
    }
    if (o.table) {
        html += QStringLiteral("<table border='1' cellspacing='0' cellpadding='3' width='100%' style='border-color:#999'>"
                               "<thead><tr style='background-color:#e8f1f8'>");
        const QStringList heads = { ReportTr::tr("Datum / Uhrzeit"), ReportTr::tr("Übung"), ReportTr::tr("Anlass"), ReportTr::tr("Muster"), ReportTr::tr("Dauer"),
                                    ReportTr::tr("Zyklen"), ReportTr::tr("SpO₂ vor/nach"), ReportTr::tr("Peak Flow vor/nach"), ReportTr::tr("Puls vor/nach"),
                                    ReportTr::tr("Befinden vor/nach"), ReportTr::tr("Atemtrainer") };
        for (const QString &h : heads) html += QStringLiteral("<th>%1</th>").arg(esc(h));
        html += QStringLiteral("</tr></thead>");
        auto device = [](const SessionRecord &r) {
            QString t;
            if (r.deviceBalls >= 0) t = ReportTr::tr("%n Ball/Bälle", nullptr, r.deviceBalls);
            if (r.deviceHoldSec >= 0) t += (t.isEmpty() ? QString() : QStringLiteral(", ")) + QStringLiteral("%1 s").arg(r.deviceHoldSec);
            return t;
        };
        auto pair = [](int a, int b) {
            if (a < 0 && b < 0) return QString();
            return QStringLiteral("%1 / %2").arg(a >= 0 ? QString::number(a) : QStringLiteral("–"),
                                                 b >= 0 ? QString::number(b) : QStringLiteral("–"));
        };
        for (const SessionRecord &s : spec.sessions) {
            html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td align='right'>%5%6</td>"
                                   "<td align='right'>%7</td><td align='center'>%8</td><td align='center'>%9</td>"
                                   "<td align='center'>%10</td><td align='center'>%11</td><td align='center'>%12</td></tr>")
                        .arg(QLocale().toString(s.started, QStringLiteral("ddd dd.MM.yyyy HH:mm")),
                             esc(BreathModules::displayName(s.moduleId)), esc(s.eventName), esc(s.pattern),
                             formatDuration(s.actualSec), s.completed ? QString() : QStringLiteral(" *"),
                             QString::number(s.cycles), pair(s.spo2Before, s.spo2After), pair(s.pfBefore, s.pfAfter))
                        .arg(pair(s.pulseBefore, s.pulseAfter), pair(s.feelingBefore, s.feelingAfter), device(s));
            if (o.notes && !s.note.isEmpty())
                html += QStringLiteral("<tr><td></td><td colspan='10'><i>%1</i></td></tr>").arg(esc(s.note));
        }
        html += QStringLiteral("</table><p style='font-size:7pt'>%1</p>")
                    .arg(ReportTr::tr("* vorzeitig beendet. SpO₂ in %, Peak Flow in l/min, Puls in Schlägen/min, Befinden 1–10."));
    }
    html += QStringLiteral("<p style='font-size:7pt;color:#666'>%1</p></body></html>")
                .arg(ReportTr::tr("Erstellt mit ViviaBreathe %1 am %2. ViviaBreathe ist ein persönliches Übungs-Tagebuch und kein "
                        "Medizinprodukt.").arg(vbVersion(), QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat)));
    doc.setHtml(html);
}

bool writePdf(const QString &file, const User &user, const ChartSpec &spec, const Options &o)
{
    QPdfWriter pdf(file);
    pdf.setResolution(300);
    pdf.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(15, 15, 15, 15),
                                  QPageLayout::Millimeter));
    pdf.setTitle(ReportTr::tr("ViviaBreathe – Übungstagebuch"));
    pdf.setCreator(QStringLiteral("ViviaBreathe %1").arg(vbVersion()));
    // Aufbau in Bildschirm-Einheiten (96 dpi), gezeichnet mit dem Maßstab der PDF-Auflösung –
    // wie QTextDocument::print, aber ohne QPrinter. Die Grafik entsteht in dreifacher Auflösung.
    QTextDocument doc;
    const QRectF page = pdf.pageLayout().paintRectPixels(96);
    build(doc, page.size(), user, spec, o, 3.0);
    QPainter p(&pdf);
    if (!p.isActive()) return false;
    const double scale = pdf.resolution() / 96.0;
    p.scale(scale, scale);
    const int pages = doc.pageCount();
    for (int i = 0; i < pages; ++i) {
        if (i > 0) pdf.newPage();
        p.save();
        p.translate(0, -page.height() * i);
        doc.drawContents(&p, QRectF(0, page.height() * i, page.width(), page.height()));
        p.restore();
    }
    p.end();
    return QFile::exists(file);
}

QByteArray csv(const QList<SessionRecord> &sessions)
{
    QString out;
    auto q = [](QString s) { return QLatin1Char('"') + s.replace(QLatin1Char('"'), QStringLiteral("\"\"")) + QLatin1Char('"'); };
    auto n = [](int v) { return v >= 0 ? QString::number(v) : QString(); };
    out += QStringLiteral("Beginn;Uebung;Anlass;Muster;Geplant_s;Dauer_s;Zyklen;Vollstaendig;SpO2_vor;SpO2_nach;PeakFlow_vor;PeakFlow_nach;"
                          "Puls_vor;Puls_nach;Befinden_vor;Befinden_nach;Atemtrainer_Baelle;Atemtrainer_Haltezeit_s;Notiz\n");
    const QChar sc = QLatin1Char(';');
    for (const SessionRecord &s : sessions)
        out += s.started.toString(Qt::ISODate) + sc + q(BreathModules::displayName(s.moduleId)) + sc + q(s.eventName)
             + sc + q(s.pattern) + sc + QString::number(s.plannedSec) + sc + QString::number(s.actualSec) + sc
             + QString::number(s.cycles) + sc + QString::number(s.completed ? 1 : 0) + sc + n(s.spo2Before) + sc
             + n(s.spo2After) + sc + n(s.pfBefore) + sc + n(s.pfAfter) + sc + n(s.pulseBefore) + sc + n(s.pulseAfter)
             + sc + n(s.feelingBefore) + sc + n(s.feelingAfter) + sc + n(s.deviceBalls) + sc + n(s.deviceHoldSec) + sc
             + q(s.note) + QLatin1Char('\n');
    return QByteArray("\xEF\xBB\xBF") + out.toUtf8();   // BOM für Excel
}

} // namespace ReportDoc
