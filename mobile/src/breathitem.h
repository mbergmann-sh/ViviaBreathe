// ViviaBreathe mobil – Atemanimation als QML-Element (gleiche Zeichenroutine wie am Desktop)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include <QPointer>
#include <QQuickPaintedItem>

class SessionRunner;

class BreathItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QObject *runner READ runner WRITE setRunner NOTIFY runnerChanged)
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground NOTIFY styleChanged)
    Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY styleChanged)
public:
    explicit BreathItem(QQuickItem *parent = nullptr);
    void paint(QPainter *p) override;

    QObject *runner() const;
    void setRunner(QObject *r);
    QColor foreground() const { return m_fg; }
    void setForeground(const QColor &c) { if (c != m_fg) { m_fg = c; update(); emit styleChanged(); } }
    bool dark() const { return m_dark; }
    void setDark(bool d) { if (d != m_dark) { m_dark = d; update(); emit styleChanged(); } }

signals:
    void runnerChanged();
    void styleChanged();

private:
    QPointer<SessionRunner> m_runner;
    QColor m_fg = Qt::black;
    bool m_dark = false;
};
