// ViviaBreathe – Animation einer Atemübung (Desktop-Widget; gezeichnet von BreathRenderer)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include "breathrenderer.h"

#include <QWidget>

class BreathView : public QWidget
{
    Q_OBJECT
public:
    explicit BreathView(QWidget *parent = nullptr);
    void setModule(const ModuleDef *m, const QVector<double> &phaseSec) { m_r.setModule(m, phaseSec); update(); }
    void setEngine(BreathEngine *e);
    void setShowHints(bool on) { m_r.setShowHints(on); update(); }
    void setDeviceTarget(int balls) { m_r.setDeviceTarget(balls); update(); }
    QSize sizeHint() const override { return { 520, 520 }; }
    QSize minimumSizeHint() const override { return { 240, 260 }; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    BreathRenderer m_r;
};
