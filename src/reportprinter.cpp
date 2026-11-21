// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "reportprinter.h"
#include "database.h"

#include <QCoreApplication>
#include <QFile>
#include <QPageLayout>
#include <QPageSize>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QTextDocument>

// Übersetzungskontext "ReportPrinter" (für lupdate erkennbar)
struct ReportPrintTr { Q_DECLARE_TR_FUNCTIONS(ReportPrinter) };

namespace ReportPrinter {

namespace {
void setupPage(QPrinter *printer)
{
    printer->setPageSize(QPageSize(QPageSize::A4));
    printer->setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
}
} // namespace

void render(QPrinter *printer, const User &user, const ChartSpec &spec, const Options &o)
{
    QTextDocument doc;
    ReportDoc::build(doc, printer->pageLayout().paintRectPixels(printer->resolution()).size(), user, spec, o);
    doc.print(printer);
}

void preview(QWidget *parent, const User &user, const ChartSpec &spec, const Options &o)
{
    QPrinter printer(QPrinter::HighResolution);
    setupPage(&printer);
    QPrintPreviewDialog dlg(&printer, parent);
    dlg.setWindowTitle(ReportPrintTr::tr("Druckvorschau – Übungstagebuch"));
    dlg.resize(900, 1000);
    QObject::connect(&dlg, &QPrintPreviewDialog::paintRequested, &dlg,
                     [&](QPrinter *pr) { render(pr, user, spec, o); });
    dlg.exec();
}

bool savePdf(QWidget *, const User &user, const ChartSpec &spec, const Options &o, const QString &file)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);
    printer.setDocName(ReportPrintTr::tr("ViviaBreathe – Übungstagebuch"));
    printer.setCreator(QStringLiteral("ViviaBreathe %1").arg(vbVersion()));
    setupPage(&printer);
    render(&printer, user, spec, o);
    return QFile::exists(file);
}

} // namespace ReportPrinter
