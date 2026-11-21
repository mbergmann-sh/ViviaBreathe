// ViviaBreathe mobil – Erinnerungen, Wecker und geplantes Einschlafen über Android
//
// Die Datenbank ist verschlüsselt; Android kann sie nicht lesen. Deshalb plant die App nach der
// Anmeldung, nach Änderungen und beim Wechsel in den Hintergrund die Termine der nächsten 14 Tage
// (alle Benutzer dieses Geräts) und übergibt sie Android (Java: Reminders → AlarmManager):
//   * Ereignis       → Benachrichtigung „Zeit für: Einschlafübung“; Antippen öffnet die Übung
//   * Wecker         → startet zur Weckzeit die Musik (PlaybackService), mit „Aus“/„Schlummern“
//   * Einschlafen    → geplant mit „automatisch starten“: Musik startet von selbst; sonst
//                      Benachrichtigung „Zeit zum Einschlafen“, Antippen öffnet die Einschlafseite
// Wer die App länger als 14 Tage nicht öffnet, bekommt danach keine Erinnerungen mehr.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QTimer>

class MobileReminders : public QObject
{
    Q_OBJECT
public:
    struct Item {
        int id = 0;
        QDateTime at;
        QString kind;           // event | alarm | sleep
        qint64 ref = 0;         // ID des Ereignisses, Weckers bzw. Einschlaf-Eintrags
        qint64 user = 0;
        QString title, text;
        bool sound = true;
        QJsonObject play;       // Wecker / automatisches Einschlafen: Konfiguration der Wiedergabe
    };
    explicit MobileReminders(QObject *parent = nullptr);

    static QList<Item> plan(const QDateTime &now, int days = 14, int max = 150);
    static QJsonObject alarmConfig(qint64 alarmId);   // auch für „Wecker testen“
    static QJsonObject sleepConfig(qint64 planId, int minutesOverride = 0);
    // Wiedergabelisten sind gerätebezogen (Pfade bzw. content://-Adressen gelten nur hier):
    // kind = "alarm" | "sleep"; ohne eigene Liste die abgeglichene, soweit hier lesbar
    static QStringList playlist(const QString &kind, qint64 id, const QStringList &synced);
    static void setPlaylist(const QString &kind, qint64 id, const QStringList &files);

    void scheduleSoon();
    void scheduleNow();
    void clear();                        // ausstehende gebündelte Planung verwerfen (Plan bei Android bleibt)
    void requestPermission();            // Android 13+: Benachrichtigungen erlauben
    bool permissionGranted() const;
    bool exactAllowed() const;           // Android 12+: pünktliche Wecker erlaubt
    void openExactSettings();
    int lastCount() const { return m_count; }
    QDateTime nextAt() const { return m_next; }

private:
    QTimer m_debounce;
    int m_count = 0;
    QDateTime m_next;
    QByteArray m_lastPlan;
};
