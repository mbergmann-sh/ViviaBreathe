// ViviaBreathe mobil – Ereignisse, Wecker und Einschlafen (Teil von AppController)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "appcontroller.h"
#include "mobilereminders.h"
#include "platform.h"
#include "playback.h"

#include "breathmodules.h"
#include "database.h"
#include "schedule.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>
#include <QSettings>

namespace {
QVariantList fileList(const QStringList &files)
{
    QVariantList l;
    for (const QString &f : files)
        l << QVariantMap{ { QStringLiteral("file"), f }, { QStringLiteral("name"), Platform::displayName(f) } };
    return l;
}

QStringList filesFrom(const QVariant &v)
{
    QStringList l;
    for (const QVariant &x : v.toList()) {
        const QString f = x.toMap().isEmpty() ? x.toString() : x.toMap().value(QStringLiteral("file")).toString();
        if (!f.isEmpty()) l << f;
    }
    return l;
}

QTime timeFrom(const QVariantMap &m, const QTime &def)
{
    if (!m.contains(QStringLiteral("hour"))) return def;
    return QTime(qBound(0, m.value(QStringLiteral("hour")).toInt(), 23), qBound(0, m.value(QStringLiteral("minute")).toInt(), 59));
}

QString nextText(const QTime &t, int weekdays, bool enabled)
{
    if (!enabled || !weekdays) return {};
    const QDateTime n = Schedule::next(t, weekdays);
    return n.isValid() ? AppController::tr("nächste: %1").arg(Schedule::relative(n)) : QString();
}
} // namespace

QString AppController::weekdaysText(int mask) const { return Weekdays::text(mask); }

// ---------------------------------------------------------------- Ereignisse

QVariantList AppController::events() const
{
    QVariantList l;
    if (!loggedIn() || !m_userId) return l;
    for (const BreathEvent &e : Database::instance().events(m_userId)) {
        l << QVariantMap{ { QStringLiteral("id"), e.id }, { QStringLiteral("name"), e.name },
                          { QStringLiteral("moduleId"), e.moduleId },
                          { QStringLiteral("moduleName"), BreathModules::displayName(e.moduleId) },
                          { QStringLiteral("time"), e.time.toString(QStringLiteral("HH:mm")) },
                          { QStringLiteral("hour"), e.time.hour() }, { QStringLiteral("minute"), e.time.minute() },
                          { QStringLiteral("weekdays"), e.weekdays }, { QStringLiteral("weekdaysText"), Weekdays::text(e.weekdays) },
                          { QStringLiteral("enabled"), e.enabled }, { QStringLiteral("sound"), e.sound },
                          { QStringLiteral("note"), e.note }, { QStringLiteral("next"), nextText(e.time, e.weekdays, e.enabled) } };
    }
    return l;
}

bool AppController::saveEvent(const QVariantMap &m)
{
    if (!loggedIn() || !m_userId) return false;
    BreathEvent e;
    const qint64 id = m.value(QStringLiteral("id")).toLongLong();
    if (id) {
        bool found = false;
        for (const BreathEvent &x : Database::instance().events())
            if (x.id == id) { e = x; found = true; break; }
        if (!found) { setError(tr("Der Eintrag wurde nicht gefunden.")); return false; }
    } else {
        e.userId = m_userId;
    }
    e.name = m.value(QStringLiteral("name")).toString().trimmed();
    if (e.name.isEmpty()) { setError(tr("Bitte eine Bezeichnung eingeben.")); return false; }
    e.moduleId = m.value(QStringLiteral("moduleId"), e.moduleId).toString();
    if (!BreathModules::find(e.moduleId)) { setError(tr("Bitte eine Übung wählen.")); return false; }
    e.time = timeFrom(m, e.time);
    e.weekdays = m.value(QStringLiteral("weekdays"), e.weekdays).toInt() & Weekdays::kAll;
    if (!e.weekdays) { setError(tr("Bitte mindestens einen Wochentag wählen.")); return false; }
    e.enabled = m.value(QStringLiteral("enabled"), e.enabled).toBool();
    e.sound = m.value(QStringLiteral("sound"), e.sound).toBool();
    e.note = m.value(QStringLiteral("note")).toString().trimmed();
    QString err;
    if (!Database::instance().saveEvent(e, &err)) { setError(err); return false; }
    m_reminders->scheduleSoon();
    bump();
    return true;
}

bool AppController::deleteEvent(qint64 id)
{
    QString err;
    if (!Database::instance().deleteEvent(id, &err)) { setError(err); return false; }
    m_reminders->scheduleSoon();
    bump();
    return true;
}

// ---------------------------------------------------------------- Wecker

QVariantMap AppController::alarm(qint64 id) const
{
    for (const Alarm &a : Database::instance().alarms()) {
        if (a.id != id) continue;
        const QStringList files = MobileReminders::playlist(QStringLiteral("alarm"), a.id, a.playlist);
        return { { QStringLiteral("id"), a.id }, { QStringLiteral("label"), a.label },
                 { QStringLiteral("time"), a.time.toString(QStringLiteral("HH:mm")) },
                 { QStringLiteral("hour"), a.time.hour() }, { QStringLiteral("minute"), a.time.minute() },
                 { QStringLiteral("weekdays"), a.weekdays }, { QStringLiteral("weekdaysText"), Weekdays::text(a.weekdays) },
                 { QStringLiteral("enabled"), a.enabled }, { QStringLiteral("playlist"), fileList(files) },
                 { QStringLiteral("playlistText"), files.isEmpty() ? tr("eingebauter Weckton") : tr("%n Titel", nullptr, int(files.size())) },
                 { QStringLiteral("shuffle"), a.shuffle }, { QStringLiteral("volume"), a.volume },
                 { QStringLiteral("fadeInMin"), a.fadeInMin }, { QStringLiteral("snoozeMin"), a.snoozeMin },
                 { QStringLiteral("moduleId"), a.moduleId },
                 { QStringLiteral("moduleName"), a.moduleId.isEmpty() ? QString() : BreathModules::displayName(a.moduleId) },
                 { QStringLiteral("next"), nextText(a.time, a.weekdays, a.enabled) } };
    }
    return {};
}

QVariantList AppController::alarms() const
{
    QVariantList l;
    if (!loggedIn() || !m_userId) return l;
    for (const Alarm &a : Database::instance().alarms(m_userId)) l << alarm(a.id);
    return l;
}

bool AppController::saveAlarm(const QVariantMap &m)
{
    if (!loggedIn() || !m_userId) return false;
    Alarm a;
    const qint64 id = m.value(QStringLiteral("id")).toLongLong();
    if (id) {
        bool found = false;
        for (const Alarm &x : Database::instance().alarms())
            if (x.id == id) { a = x; found = true; break; }
        if (!found) { setError(tr("Der Eintrag wurde nicht gefunden.")); return false; }
    } else {
        a.userId = m_userId;
    }
    a.label = m.value(QStringLiteral("label")).toString().trimmed();
    a.time = timeFrom(m, a.time);
    a.weekdays = m.value(QStringLiteral("weekdays"), a.weekdays).toInt() & Weekdays::kAll;
    if (!a.weekdays) { setError(tr("Bitte mindestens einen Wochentag wählen.")); return false; }
    a.enabled = m.value(QStringLiteral("enabled"), a.enabled).toBool();
    a.shuffle = m.value(QStringLiteral("shuffle"), a.shuffle).toBool();
    a.volume = qBound(5, m.value(QStringLiteral("volume"), a.volume).toInt(), 100);
    a.fadeInMin = qBound(0, m.value(QStringLiteral("fadeInMin"), a.fadeInMin).toInt(), 30);
    a.snoozeMin = qBound(1, m.value(QStringLiteral("snoozeMin"), a.snoozeMin).toInt(), 60);
    a.moduleId = m.value(QStringLiteral("moduleId")).toString();
    QString err;
    if (!Database::instance().saveAlarm(a, &err)) { setError(err); return false; }
    MobileReminders::setPlaylist(QStringLiteral("alarm"), a.id, filesFrom(m.value(QStringLiteral("playlist"))));
    m_reminders->scheduleSoon();
    bump();
    return true;
}

bool AppController::deleteAlarm(qint64 id)
{
    QString err;
    if (!Database::instance().deleteAlarm(id, &err)) { setError(err); return false; }
    m_reminders->scheduleSoon();
    bump();
    return true;
}

void AppController::testAlarm(const QVariantMap &m)
{
    QVariantMap c;
    c.insert(QStringLiteral("files"), filesFrom(m.value(QStringLiteral("playlist"))));
    c.insert(QStringLiteral("shuffle"), m.value(QStringLiteral("shuffle")).toBool());
    c.insert(QStringLiteral("volume"), m.value(QStringLiteral("volume"), 70).toInt());
    c.insert(QStringLiteral("fadeInSec"), 3);
    c.insert(QStringLiteral("durationSec"), 20);       // Probehören: 20 Sekunden
    c.insert(QStringLiteral("fadeOutSec"), 3);
    c.insert(QStringLiteral("snoozeMin"), 0);
    c.insert(QStringLiteral("title"), tr("Wecker – Probe"));
    c.insert(QStringLiteral("text"), tr("Probehören (20 Sekunden)"));
    c.insert(QStringLiteral("ref"), 0);
    m_playback->start(QStringLiteral("sleep"), c);
}

// ---------------------------------------------------------------- Einschlafen

QVariantMap AppController::sleepPlan(qint64 id) const
{
    for (const SleepPlan &p : Database::instance().sleepPlans()) {
        if (p.id != id) continue;
        const QStringList files = MobileReminders::playlist(QStringLiteral("sleep"), p.id, p.playlist);
        return { { QStringLiteral("id"), p.id }, { QStringLiteral("label"), p.label },
                 { QStringLiteral("time"), p.time.toString(QStringLiteral("HH:mm")) },
                 { QStringLiteral("hour"), p.time.hour() }, { QStringLiteral("minute"), p.time.minute() },
                 { QStringLiteral("weekdays"), p.weekdays }, { QStringLiteral("weekdaysText"), Weekdays::text(p.weekdays) },
                 { QStringLiteral("scheduled"), p.scheduled }, { QStringLiteral("autoStart"), p.autoStart },
                 { QStringLiteral("playlist"), fileList(files) },
                 { QStringLiteral("playlistText"), files.isEmpty() ? tr("ohne Musik") : tr("%n Titel", nullptr, int(files.size())) },
                 { QStringLiteral("shuffle"), p.shuffle }, { QStringLiteral("volume"), p.volume },
                 { QStringLiteral("minutes"), p.minutes }, { QStringLiteral("fadeOutMin"), p.fadeOutMin },
                 { QStringLiteral("moduleId"), p.moduleId },
                 { QStringLiteral("moduleName"), p.moduleId.isEmpty() ? QString() : BreathModules::displayName(p.moduleId) },
                 { QStringLiteral("next"), p.scheduled ? nextText(p.time, p.weekdays, true) : QString() } };
    }
    return {};
}

QVariantList AppController::sleepPlans() const
{
    QVariantList l;
    if (!loggedIn() || !m_userId) return l;
    for (const SleepPlan &p : Database::instance().sleepPlans(m_userId)) l << sleepPlan(p.id);
    return l;
}

bool AppController::saveSleepPlan(const QVariantMap &m)
{
    if (!loggedIn() || !m_userId) return false;
    SleepPlan p;
    const qint64 id = m.value(QStringLiteral("id")).toLongLong();
    if (id) {
        bool found = false;
        for (const SleepPlan &x : Database::instance().sleepPlans())
            if (x.id == id) { p = x; found = true; break; }
        if (!found) { setError(tr("Der Eintrag wurde nicht gefunden.")); return false; }
    } else {
        p.userId = m_userId;
    }
    p.label = m.value(QStringLiteral("label")).toString().trimmed();
    if (p.label.isEmpty()) { setError(tr("Bitte eine Bezeichnung eingeben.")); return false; }
    p.time = timeFrom(m, p.time);
    p.weekdays = m.value(QStringLiteral("weekdays"), p.weekdays).toInt() & Weekdays::kAll;
    p.scheduled = m.value(QStringLiteral("scheduled"), p.scheduled).toBool();
    if (p.scheduled && !p.weekdays) { setError(tr("Bitte mindestens einen Wochentag wählen.")); return false; }
    p.autoStart = m.value(QStringLiteral("autoStart"), p.autoStart).toBool();
    p.shuffle = m.value(QStringLiteral("shuffle"), p.shuffle).toBool();
    p.volume = qBound(5, m.value(QStringLiteral("volume"), p.volume).toInt(), 100);
    p.minutes = qBound(5, m.value(QStringLiteral("minutes"), p.minutes).toInt(), 240);
    p.fadeOutMin = qBound(0, m.value(QStringLiteral("fadeOutMin"), p.fadeOutMin).toInt(), p.minutes);
    p.moduleId = m.value(QStringLiteral("moduleId")).toString();
    QString err;
    if (!Database::instance().saveSleepPlan(p, &err)) { setError(err); return false; }
    MobileReminders::setPlaylist(QStringLiteral("sleep"), p.id, filesFrom(m.value(QStringLiteral("playlist"))));
    m_reminders->scheduleSoon();
    bump();
    return true;
}

bool AppController::deleteSleepPlan(qint64 id)
{
    QString err;
    if (!Database::instance().deleteSleepPlan(id, &err)) { setError(err); return false; }
    m_reminders->scheduleSoon();
    bump();
    return true;
}

bool AppController::startSleep(qint64 planId, int minutes)
{
    const QJsonObject c = MobileReminders::sleepConfig(planId, minutes);
    if (c.isEmpty()) { setError(tr("Der Eintrag wurde nicht gefunden.")); return false; }
    if (c.value(QStringLiteral("files")).toArray().isEmpty()) {
        setError(tr("Für diesen Eintrag ist auf diesem Gerät keine Musik gewählt."));
        return false;
    }
    m_playback->start(QStringLiteral("sleep"), c.toVariantMap());
    return true;
}

// ---------------------------------------------------------------- Android: Erinnerungen

QVariantMap AppController::reminderStatus() const
{
    const QDateTime next = m_reminders->nextAt();
    return { { QStringLiteral("permission"), m_reminders->permissionGranted() },
             { QStringLiteral("exact"), m_reminders->exactAllowed() },
             { QStringLiteral("count"), m_reminders->lastCount() },
             { QStringLiteral("next"), next.isValid() ? Schedule::relative(next) : QString() } };
}

void AppController::requestNotificationPermission()
{
    // Android fragt nur, solange der Anwender nicht dauerhaft abgelehnt hat – danach die Einstellungen öffnen
    QSettings s;
    if (!s.value(QStringLiteral("mobile/notificationAsked"), false).toBool()) {
        s.setValue(QStringLiteral("mobile/notificationAsked"), true);
        m_reminders->requestPermission();
    } else {
        Platform::openAppNotificationSettings();
    }
}

void AppController::openExactAlarmSettings() { m_reminders->openExactSettings(); }

QVariantMap AppController::takeLaunchAction()
{
    QVariantMap a = Platform::takeLaunchAction();
    if (a.isEmpty()) return a;
    // Benutzer der Erinnerung auswählen
    const qint64 user = a.value(QStringLiteral("user")).toLongLong();
    if (loggedIn() && user) selectUser(user);
    const QString kind = a.value(QStringLiteral("kind")).toString();
    const qint64 ref = a.value(QStringLiteral("ref")).toLongLong();
    if (kind == QLatin1String("event") && loggedIn()) {
        for (const BreathEvent &e : Database::instance().events())
            if (e.id == ref) {
                a.insert(QStringLiteral("moduleId"), e.moduleId);
                a.insert(QStringLiteral("eventName"), e.name);
            }
    }
    return a;
}
