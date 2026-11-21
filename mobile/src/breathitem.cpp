// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "breathitem.h"
#include "sessionrunner.h"

#include <QGuiApplication>
#include <QPainter>
#include <QTimer>

BreathItem::BreathItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
}

QObject *BreathItem::runner() const { return m_runner; }

void BreathItem::setRunner(QObject *r)
{
    auto *sr = qobject_cast<SessionRunner *>(r);
    if (sr == m_runner) return;
    if (m_runner) disconnect(m_runner, nullptr, this, nullptr);
    m_runner = sr;
    if (sr) {
        connect(sr, &SessionRunner::tick, this, [this] { update(); });
        connect(sr, &SessionRunner::changed, this, [this] { update(); });
    }
    update();
    emit runnerChanged();
}

void BreathItem::paint(QPainter *p)
{
    if (!m_runner) return;
    QFont f = QGuiApplication::font();
    f.setPointSizeF(qMax(11.0, f.pointSizeF()));
    m_runner->paint(p, QRectF(0, 0, width(), height()), m_fg, m_dark, f);
    if (m_runner->wantsFrame()) QTimer::singleShot(40, this, [this] { update(); });
}
