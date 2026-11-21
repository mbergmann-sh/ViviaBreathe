// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "chartitem.h"
#include "database.h"

#include <QGuiApplication>
#include <QPainter>

ChartItem::ChartItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setOpaquePainting(true);
    m_spec.to = QDate::currentDate();
    m_spec.from = m_spec.to.addDays(-29);
}

void ChartItem::setFromDate(const QString &d)
{
    const QDate x = QDate::fromString(d, Qt::ISODate);
    if (!x.isValid() || x == m_spec.from) return;
    m_spec.from = x;
    reload();
}

void ChartItem::setToDate(const QString &d)
{
    const QDate x = QDate::fromString(d, Qt::ISODate);
    if (!x.isValid() || x == m_spec.to) return;
    m_spec.to = x;
    reload();
}

void ChartItem::setShowMeasurements(bool on)
{
    if (on == m_spec.showSpo2) return;
    m_spec.showSpo2 = m_spec.showPeakFlow = m_spec.showPulse = m_spec.showDevice = on;
    update();
    emit specChanged();
}

void ChartItem::reload()
{
    m_spec.sessions.clear();
    Database &db = Database::instance();
    if (db.isOpen() && m_user && m_spec.from.isValid() && m_spec.to.isValid()) {
        const QList<SessionRecord> all = db.sessions(m_user, QDateTime(m_spec.from, QTime(0, 0)),
                                                     QDateTime(m_spec.to.addDays(1), QTime(0, 0)));
        for (const SessionRecord &s : all)
            if (m_module.isEmpty() || s.moduleId == m_module) m_spec.sessions << s;
    }
    update();
    emit specChanged();
}

void ChartItem::paint(QPainter *p)
{
    p->fillRect(QRectF(0, 0, width(), height()), m_bg);
    QFont f = QGuiApplication::font();
    f.setPixelSize(13);
    p->setFont(f);
    p->setPen(m_fg);
    ChartRenderer::paint(*p, QRectF(0, 0, width(), height()).adjusted(4, 6, -6, -4), m_spec, true);
}
