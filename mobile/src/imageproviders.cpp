// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "imageproviders.h"

#include <QFile>
#include <QPainter>
#include <QSvgRenderer>
#include <QUrlQuery>

namespace {
QImage renderSvg(const QByteArray &svg, const QSize &requested, const QSize &fallback, QSize *size)
{
    QSvgRenderer r(svg);
    QSize target = r.defaultSize().isValid() ? r.defaultSize() : fallback;
    if (requested.width() > 0 || requested.height() > 0) {
        const QSize want(requested.width() > 0 ? requested.width() : 100000,
                         requested.height() > 0 ? requested.height() : 100000);
        target.scale(want, Qt::KeepAspectRatio);
    } else {
        target = fallback.isValid() ? fallback : target;
    }
    if (target.width() < 1 || target.height() < 1 || target.width() > 4096 || target.height() > 4096) {
        QImage img(1, 1, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        if (size) *size = img.size();
        return img;
    }
    QImage img(target, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    r.render(&p);
    p.end();
    if (size) *size = img.size();
    return img;
}

QImage empty(QSize *size)
{
    QImage img(1, 1, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    if (size) *size = img.size();
    return img;
}
}

QImage IconProvider::requestImage(const QString &id, QSize *size, const QSize &requested)
{
    const QString name = id.section(QLatin1Char('?'), 0, 0);
    const QUrlQuery q(id.section(QLatin1Char('?'), 1));
    QFile f(QStringLiteral(":/icons/ui/%1.svg").arg(name));
    if (!f.open(QIODevice::ReadOnly)) return empty(size);
    QByteArray svg = f.readAll();
    const QString c = q.queryItemValue(QStringLiteral("c"));
    const QString a = q.queryItemValue(QStringLiteral("a"));
    svg.replace("#COLOR", (c.isEmpty() ? QStringLiteral("#16222b") : QLatin1Char('#') + c).toLatin1());
    svg.replace("#ACCENT", (a.isEmpty() ? QStringLiteral("#e0620d") : QLatin1Char('#') + a).toLatin1());
    return renderSvg(svg, requested, QSize(96, 96), size);
}

QImage IllustrationProvider::requestImage(const QString &id, QSize *size, const QSize &requested)
{
    const QString name = id.section(QLatin1Char('?'), 0, 0);
    const bool dark = QUrlQuery(id.section(QLatin1Char('?'), 1)).queryItemValue(QStringLiteral("dark")) == QLatin1String("1");
    QFile f(QStringLiteral(":/illustrations/%1.svg").arg(name));
    if (!f.open(QIODevice::ReadOnly)) return empty(size);
    QByteArray svg = f.readAll();
    svg.replace("#COLOR", dark ? QByteArray("#e6edf2") : QByteArray("#16222b"));
    svg.replace("#ACCENT", dark ? QByteArray("#f58a3c") : QByteArray("#e0620d"));
    svg.replace("#BLUE", dark ? QByteArray("#4a9ee0") : QByteArray("#1474bc"));
    svg.replace("#GREEN", dark ? QByteArray("#4cc28a") : QByteArray("#2e9e6b"));
    return renderSvg(svg, requested, QSize(480, 360), size);
}
