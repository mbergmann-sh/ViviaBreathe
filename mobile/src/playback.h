// ViviaBreathe mobil – Musik für Wecker und Einschlafen, auch bei ausgeschaltetem Bildschirm
//
// Unter Android spielt ein Vordergrunddienst (PlaybackService.java) die Wiedergabeliste: Er läuft
// weiter, wenn der Bildschirm aus ist oder die App im Hintergrund, und wird beim Wecken direkt
// vom Wecker des Systems gestartet (auch wenn die App nicht läuft). Die Benachrichtigung des
// Dienstes hat „Aus“ bzw. „Schlummern“.
// Am Desktop (Testbetrieb) übernimmt MusicPlayer aus dem Kern dieselbe Aufgabe.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include <QDateTime>
#include <QObject>
#include <QVariantMap>

class MusicPlayer;
class QTimer;

class Playback : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY changed)
    Q_PROPERTY(QString mode READ mode NOTIFY changed)              // "alarm" | "sleep" | ""
    Q_PROPERTY(QString title READ title NOTIFY changed)
    Q_PROPERTY(int remainingSec READ remainingSec NOTIFY changed)  // Einschlafen: Restzeit
    Q_PROPERTY(QString track READ track NOTIFY changed)
    Q_PROPERTY(qint64 refId READ refId NOTIFY changed)
public:
    explicit Playback(QObject *parent = nullptr);

    // Konfiguration (wie für den Android-Dienst):
    //   files (Liste), shuffle, volume 0–100, fadeInSec, durationSec (0 = bis „Aus“), fadeOutSec,
    //   snoozeMin, title, text, ref, fallback (eingebauter Weckton), Beschriftungen
    void start(const QString &mode, const QVariantMap &config);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void snooze();
    Q_INVOKABLE void refresh();       // Zustand des Dienstes abfragen

    bool running() const { return m_running; }
    QString mode() const { return m_mode; }
    QString title() const { return m_title; }
    int remainingSec() const { return m_remaining; }
    QString track() const { return m_track; }
    qint64 refId() const { return m_ref; }

    static QVariantMap labels();      // Texte für die Benachrichtigung des Dienstes

signals:
    void changed();
    void ended(const QString &mode, qint64 refId);   // Wiedergabe vorbei (Zeit um oder „Aus“)

private:
    void setState(bool running, const QString &mode, const QString &title, int remaining, const QString &track, qint64 ref);

    QTimer *m_poll = nullptr;
    bool m_running = false;
    QString m_mode, m_title, m_track;
    int m_remaining = 0;
    qint64 m_ref = 0;
    // Desktop-Ersatz
    MusicPlayer *m_player = nullptr;
    QVariantMap m_config;
    QDateTime m_started;
    bool m_fadingOut = false;
};
