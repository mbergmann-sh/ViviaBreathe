// ViviaBreathe mobil – Bindeglied zwischen QML-Oberfläche und Kern
//
// In QML als Kontext-Eigenschaft „app“ verfügbar. Daten gehen als QVariantMap/QVariantList an die
// Oberfläche; Änderungen erhöhen 'revision', damit Listen neu laden. Fehlertexte stehen in
// lastError. Aufgeteilt auf appcontroller.cpp (Start, Anmeldung, Benutzer, Einstellungen, Abgleich),
// appcontroller_data.cpp (Übungen, Tagebuch, Berichte) und appcontroller_alarms.cpp (Ereignisse,
// Wecker, Einschlafen).
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class MobileReminders;
class Playback;
#include <QQmlApplicationEngine>
class SessionRunner;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool isAndroid READ isAndroid CONSTANT)
    Q_PROPERTY(bool demo READ demo CONSTANT)
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(bool languageChosen READ languageChosen NOTIFY languageChanged)
    Q_PROPERTY(bool disclaimerAccepted READ disclaimerAccepted NOTIFY sessionChanged)
    Q_PROPERTY(bool hasDatabase READ hasDatabase NOTIFY sessionChanged)
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY sessionChanged)
    Q_PROPERTY(int themeMode READ themeMode WRITE setThemeMode NOTIFY themeChanged)     // 0 System, 1 hell, 2 dunkel
    Q_PROPERTY(bool dark READ dark NOTIFY themeChanged)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    Q_PROPERTY(QVariantList users READ users NOTIFY usersChanged)
    Q_PROPERTY(qint64 userId READ userId NOTIFY userChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorChanged)
    Q_PROPERTY(int autoLockMinutes READ autoLockMinutes WRITE setAutoLockMinutes NOTIFY settingsChanged)
    Q_PROPERTY(bool syncBusy READ syncBusy NOTIFY syncStateChanged)
    Q_PROPERTY(QObject *runner READ runner CONSTANT)
    Q_PROPERTY(QObject *playback READ playback CONSTANT)
    Q_PROPERTY(bool multimedia READ multimedia CONSTANT)
    Q_PROPERTY(bool vibration READ vibration CONSTANT)

public:
    explicit AppController(bool demo = false, QObject *parent = nullptr);
    ~AppController() override;
    void setEngine(QQmlApplicationEngine *e) { m_engine = e; }

    QString appName() const;
    QString version() const;
    bool isAndroid() const;
    bool demo() const { return m_demo; }
    QString language() const;
    bool languageChosen() const;
    bool disclaimerAccepted() const;
    bool hasDatabase() const;
    bool loggedIn() const;
    int themeMode() const;
    void setThemeMode(int m);
    bool dark() const;
    int revision() const { return m_revision; }
    QVariantList users() const;
    qint64 userId() const { return m_userId; }
    QString userName() const;
    QString lastError() const { return m_error; }
    int autoLockMinutes() const;
    void setAutoLockMinutes(int m);
    bool syncBusy() const;
    QObject *runner() const;
    QObject *playback() const;
    bool multimedia() const;
    bool vibration() const;

    // ---- Start, Anmeldung
    Q_INVOKABLE void setLanguage(const QString &lang);      // wechselt sofort (Oberfläche neu übersetzt)
    Q_INVOKABLE void acceptDisclaimer();
    Q_INVOKABLE bool createDatabase(const QString &password, const QString &repeat, const QString &firstUser);
    Q_INVOKABLE bool login(const QString &password);
    Q_INVOKABLE void lock();
    Q_INVOKABLE bool restoreBackup(const QString &url, const QString &password);   // Sicherung (.vbdb) übernehmen
    Q_INVOKABLE bool setupFromServer(const QString &url, const QString &key, const QString &password);
    Q_INVOKABLE bool changePassword(const QString &oldPw, const QString &newPw, const QString &repeat);
    Q_INVOKABLE bool shareBackup();
    Q_INVOKABLE bool startDemo();                            // Beispieldaten ausprobieren (eigene Datenbank)
    void setDemoMode();                                      // Autopilot

    // ---- Benutzer
    Q_INVOKABLE void selectUser(qint64 id);
    Q_INVOKABLE bool addUser(const QString &name);
    Q_INVOKABLE bool renameUser(qint64 id, const QString &name);
    Q_INVOKABLE bool deleteUser(qint64 id);

    // ---- Übungen
    Q_INVOKABLE QVariantList modules() const;                // Karten, nach Kategorie sortiert
    Q_INVOKABLE QVariantMap moduleInfo(const QString &id) const;
    Q_INVOKABLE QVariantMap moduleSettings(const QString &id) const;
    Q_INVOKABLE QVariantMap defaultSettings(const QString &id) const;
    Q_INVOKABLE bool saveModuleSettings(const QString &id, const QVariantMap &s);
    Q_INVOKABLE QVariantMap settingsPreview(const QString &id, const QVariantMap &s) const;   // Muster, Runde, Dauer
    Q_INVOKABLE QStringList eventNames() const;
    Q_INVOKABLE bool startSession(const QString &moduleId, const QVariantMap &settings);   // → runner
    Q_INVOKABLE QString rememberFile(const QString &url);    // Leserecht behalten; liefert den Namen
    Q_INVOKABLE QString fileName(const QString &fileOrUrl) const;
    Q_INVOKABLE QString emergencyModuleId() const;

    // ---- Tagebuch / Verlauf
    Q_INVOKABLE QVariantList sessions(const QString &fromIso, const QString &toIso, const QString &moduleId) const;
    Q_INVOKABLE QVariantMap sessionStats(const QString &fromIso, const QString &toIso, const QString &moduleId) const;
    Q_INVOKABLE QVariantMap session(qint64 id) const;
    Q_INVOKABLE bool updateSession(const QVariantMap &m);
    Q_INVOKABLE bool deleteSession(qint64 id);
    Q_INVOKABLE bool sharePdf(const QString &fromIso, const QString &toIso, const QString &moduleId, bool chart, bool notes);
    Q_INVOKABLE bool shareCsv(const QString &fromIso, const QString &toIso, const QString &moduleId);
    Q_INVOKABLE QStringList lastShared() const { return m_lastShared; }
    Q_INVOKABLE QString todayIso(int addDays = 0) const;
    Q_INVOKABLE QString dateText(const QString &iso) const;
    Q_INVOKABLE QVariantMap overview() const;                // heute, 7 Tage, Serie, nächste Erinnerung

    // ---- Ereignisse, Wecker, Einschlafen
    Q_INVOKABLE QVariantList events() const;
    Q_INVOKABLE bool saveEvent(const QVariantMap &m);
    Q_INVOKABLE bool deleteEvent(qint64 id);
    Q_INVOKABLE QVariantList alarms() const;
    Q_INVOKABLE bool saveAlarm(const QVariantMap &m);
    Q_INVOKABLE bool deleteAlarm(qint64 id);
    Q_INVOKABLE void testAlarm(const QVariantMap &m);       // Weckmusik jetzt probehören
    Q_INVOKABLE QVariantList sleepPlans() const;
    Q_INVOKABLE bool saveSleepPlan(const QVariantMap &m);
    Q_INVOKABLE bool deleteSleepPlan(qint64 id);
    Q_INVOKABLE QVariantMap sleepPlan(qint64 id) const;
    Q_INVOKABLE bool startSleep(qint64 planId, int minutes);
    Q_INVOKABLE QVariantMap alarm(qint64 id) const;
    Q_INVOKABLE QString weekdaysText(int mask) const;
    Q_INVOKABLE QVariantMap reminderStatus() const;          // Erlaubnis, pünktlich, Anzahl, nächster Termin
    Q_INVOKABLE void requestNotificationPermission();
    Q_INVOKABLE void openExactAlarmSettings();
    Q_INVOKABLE QVariantMap takeLaunchAction();              // angetippte Benachrichtigung

    // ---- Abgleich
    Q_INVOKABLE QVariantMap syncConfig() const;
    Q_INVOKABLE bool saveSyncConfig(const QVariantMap &m);
    Q_INVOKABLE void syncNow();
    Q_INVOKABLE QString generateSyncKey() const;
    void syncIfDue();

    // ---- Verschiedenes
    Q_INVOKABLE QString licenseText() const;
    Q_INVOKABLE void clearError() { setError(QString()); }
    void appHidden();                                        // in den Hintergrund: Erinnerungen planen
    void appActive();

signals:
    void languageChanged();
    void sessionChanged();
    void themeChanged();
    void revisionChanged();
    void usersChanged();
    void userChanged();
    void errorChanged();
    void settingsChanged();
    void syncStateChanged();
    void syncFinished(bool ok, const QString &message);
    void locked();
    void launchActionPending();          // Benachrichtigung angetippt, während die App lief

private:
    void setError(const QString &e);
    void bump();
    void afterLogin();
    bool openDb(const QString &path, const QString &password);
    bool shareFiles(const QStringList &files, const QString &mime, const QString &subject);
    QString workDir(const QString &sub) const;

    QPointer<QQmlApplicationEngine> m_engine;
    SessionRunner *m_runner = nullptr;
    Playback *m_playback = nullptr;
    MobileReminders *m_reminders = nullptr;
    bool m_demo = false;
    int m_revision = 0;
    qint64 m_userId = 0;
    QString m_error;
    QStringList m_lastShared;
};
