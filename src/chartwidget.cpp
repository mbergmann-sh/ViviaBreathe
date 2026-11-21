// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "chartwidget.h"

#include <QPainter>

void ChartWidget::setSpec(const ChartSpec &s)
{
    m_spec = s;
    m_panels = ChartRenderer::panelCount(s);
    updateGeometry();
    update();
}

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), palette().color(QPalette::Base));
    p.setPen(palette().color(QPalette::Text));
    ChartRenderer::paint(p, QRectF(rect()).adjusted(8, 8, -8, -8), m_spec, true);
}
