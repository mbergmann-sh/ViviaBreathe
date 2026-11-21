// ViviaBreathe – Zeichnen der Atemanimation, unabhängig von Widgets bzw. Qt Quick
//
//  * Box: Ein farbiger Punkt wandert im Uhrzeigersinn, beginnend links oben, von Ecke zu Ecke
//    eines andersfarbigen Quadrats – jede Seite ist eine Phase. Unter dem Quadrat steht die Phase.
//  * Kreis: Die Grafik dehnt sich beim Einatmen aus, bleibt beim Halten stehen und zieht sich
//    beim Ausatmen zusammen. Die Geschwindigkeit folgt den eingestellten Phasendauern.
//  * TriBall: Atemtrainer mit drei Bällen.
// Verwendet von BreathView (Desktop, QWidget) und BreathItem (mobile App, Qt Quick).
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include "breathengine.h"
#include "breathmodules.h"

#include <QColor>
#include <QCoreApplication>
#include <QFont>
#include <QPointer>
#include <QVector>

class QPainter;
class QRectF;

class BreathRenderer
{
    Q_DECLARE_TR_FUNCTIONS(BreathView)    // Übersetzungskontext wie bisher
public:
    void setModule(const ModuleDef *m, const QVector<double> &phaseSec) { m_module = m; m_sec = phaseSec; }
    const ModuleDef *module() const { return m_module; }
    void setEngine(BreathEngine *e) { m_engine = e; }
    BreathEngine *engine() const { return m_engine; }
    void setShowHints(bool on) { m_hints = on; }
    void setDeviceTarget(int balls) { m_target = qBound(1, balls, 3); }   // TriBall: Ziel 1–3 Bälle
    void setCaption(bool on) { m_caption = on; }                         // Phase/Hinweis unter der Grafik
    void setWaitHint(const QString &h) { m_waitHint = h; }               // z. B. „(Leertaste = weiter)“

    void paint(QPainter &p, const QRectF &rect, const QColor &fg, bool dark, const QFont &font);
    void captionTexts(QString &title, QString &sub) const;
    bool wantsFrame() const { return m_wantsFrame; }                     // gleich erneut zeichnen (Pulsieren)

private:
    void paintBox(QPainter &p, const QRectF &area);
    void paintCircle(QPainter &p, const QRectF &area);
    void paintTriBall(QPainter &p, const QRectF &area);
    void paintCaption(QPainter &p, const QRectF &r, const QString &title, const QString &sub);

    const ModuleDef        *m_module = nullptr;
    QVector<double>         m_sec;
    QPointer<BreathEngine>  m_engine;
    bool                    m_hints = true;
    bool                    m_caption = true;
    int                     m_target = 1;
    QString                 m_waitHint;
    // beim Zeichnen gesetzt
    QColor                  m_fg;
    bool                    m_dark = false;
    QFont                   m_font;
    bool                    m_wantsFrame = false;
};
