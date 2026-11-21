// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "breathview.h"
#include "breathengine.h"

#include <QPainter>
#include <QTimer>

BreathView::BreathView(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_r.setWaitHint(tr("(Leertaste = weiter)"));
}

void BreathView::setEngine(BreathEngine *e)
{
    if (m_r.engine()) disconnect(m_r.engine(), nullptr, this, nullptr);
    m_r.setEngine(e);
    if (e) connect(e, &BreathEngine::tick, this, QOverload<>::of(&QWidget::update));
    update();
}

void BreathView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    m_r.paint(p, QRectF(rect()), palette().color(QPalette::WindowText),
              palette().color(QPalette::Window).lightness() < 110, font());
    if (m_r.wantsFrame() && m_r.engine()) QTimer::singleShot(40, this, QOverload<>::of(&QWidget::update));
}
