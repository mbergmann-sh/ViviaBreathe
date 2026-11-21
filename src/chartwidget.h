// ViviaBreathe – Verlaufsgrafik als Widget (Desktop; gezeichnet von ChartRenderer)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include "chartrenderer.h"

#include <QWidget>

class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChartWidget(QWidget *parent = nullptr);
    void setSpec(const ChartSpec &s);
    const ChartSpec &spec() const { return m_spec; }
    // Mindesthöhe wächst mit der Zahl der Bereiche (in der Verlaufsseite scrollbar)
    QSize minimumSizeHint() const override { return { 360, qMax(260, 60 + 85 * m_panels) }; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    ChartSpec m_spec;
    int m_panels = 0;
};
