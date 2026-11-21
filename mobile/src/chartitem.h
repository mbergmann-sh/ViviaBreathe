// ViviaBreathe mobil – Verlaufsgrafik als QML-Element (gleiche Zeichenroutine wie am Desktop)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include "chartrenderer.h"

#include <QQuickPaintedItem>

class ChartItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(qint64 userId READ userId WRITE setUserId NOTIFY specChanged)
    Q_PROPERTY(QString fromDate READ fromDate WRITE setFromDate NOTIFY specChanged)   // ISO, einschließlich
    Q_PROPERTY(QString toDate READ toDate WRITE setToDate NOTIFY specChanged)
    Q_PROPERTY(QString moduleId READ moduleId WRITE setModuleId NOTIFY specChanged)   // leer = alle Übungen
    Q_PROPERTY(int revision READ revision WRITE setRevision NOTIFY specChanged)       // Datenstand → neu laden
    Q_PROPERTY(bool showMeasurements READ showMeasurements WRITE setShowMeasurements NOTIFY specChanged)
    Q_PROPERTY(QColor background READ background WRITE setBackground NOTIFY specChanged)
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground NOTIFY specChanged)
    Q_PROPERTY(int panelCount READ panelCount NOTIFY specChanged)
    Q_PROPERTY(int sessionCount READ sessionCount NOTIFY specChanged)
public:
    explicit ChartItem(QQuickItem *parent = nullptr);
    void paint(QPainter *p) override;

    qint64 userId() const { return m_user; }
    void setUserId(qint64 id) { if (id != m_user) { m_user = id; reload(); } }
    QString fromDate() const { return m_spec.from.toString(Qt::ISODate); }
    void setFromDate(const QString &d);
    QString toDate() const { return m_spec.to.toString(Qt::ISODate); }
    void setToDate(const QString &d);
    QString moduleId() const { return m_module; }
    void setModuleId(const QString &m) { if (m != m_module) { m_module = m; reload(); } }
    int revision() const { return m_revision; }
    void setRevision(int r) { if (r != m_revision) { m_revision = r; reload(); } }
    bool showMeasurements() const { return m_spec.showSpo2; }
    void setShowMeasurements(bool on);
    QColor background() const { return m_bg; }
    void setBackground(const QColor &c) { if (c != m_bg) { m_bg = c; update(); emit specChanged(); } }
    QColor foreground() const { return m_fg; }
    void setForeground(const QColor &c) { if (c != m_fg) { m_fg = c; update(); emit specChanged(); } }
    int panelCount() const { return ChartRenderer::panelCount(m_spec); }
    int sessionCount() const { return int(m_spec.sessions.size()); }

signals:
    void specChanged();

private:
    void reload();
    qint64 m_user = 0;
    QString m_module;
    int m_revision = 0;
    ChartSpec m_spec;
    QColor m_bg = Qt::white, m_fg = Qt::black;
};
