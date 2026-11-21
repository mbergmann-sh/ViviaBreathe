// ViviaBreathe mobil – Erinnerungen, Wecker und Einschlafen über Android (siehe mobilereminders.h)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "mobilereminders.h"
#include "playback.h"

#include "audio.h"
#include "breathmodules.h"
#include "database.h"
#include "schedule.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <algorithm>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QtCore/qcoreapplication_platform.h>
namespace {
const char *kReminders = "de/mbergmann/viviabreathe/Reminders";
QJniObject context() { return QJniObject(QNativeInterface::QAndroidApplication::context()); }
QJniObject jstr(const QString &s) { return QJniObject::fromString(s); }
}
#endif

namespace {
int makeId(int kind, qint64 ref, int n) { return (kind << 27) | (int(ref & 0xFFFF) << 8) | (n & 0xFF); }

QJsonArray toArray(const QStringList &l)
{
    QJsonArray a;
    for (const QString &s : l) a << s;
    return a;
}
} // namespace

MobileReminders::MobileReminders(QObject *parent) : QObject(parent)
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(1500);
    connect(&m_debounce, &QTimer::timeout, this, &MobileReminders::scheduleNow);
}

QStringList MobileReminders::playlist(const QString &kind, qint64 id, const QStringList &synced)
{
    const QString key = QStringLiteral("mobile/playlist/%1/%2").arg(kind).arg(id);
    const QString stored = Database::instance().deviceSetting(key, QStringLiteral("-"));
    if (stored != QLatin1String("-")) {
        QStringList l;
        for (const QJsonValue &v : QJsonDocument::fromJson(stored.toUtf8()).array()) l << v.toString();
        return l;
    }
    QStringList l;
    for (const QString &f : synced)
        if (f.startsWith(QLatin1String("content:")) || QFileInfo::exists(f)) l << f;
    return l;
}

void MobileReminders::setPlaylist(const QString &kind, qint64 id, const QStringList &files)
{
    const QString key = QStringLiteral("mobile/playlist/%1/%2").arg(kind).arg(id);
    Database::instance().setDeviceSetting(key, QString::fromUtf8(QJsonDocument(toArray(files)).toJson(QJsonDocument::Compact)));
}

QJsonObject MobileReminders::alarmConfig(qint64 alarmId)
{
    Database &db = Database::instance();
    for (const Alarm &a : db.alarms()) {
        if (a.id != alarmId) continue;
        QJsonObject o;
        o.insert(QStringLiteral("files"), toArray(playlist(QStringLiteral("alarm"), a.id, a.playlist)));
        o.insert(QStringLiteral("shuffle"), a.shuffle);
        o.insert(QStringLiteral("volume"), a.volume);
        o.insert(QStringLiteral("fadeInSec"), a.fadeInMin * 60);
        o.insert(QStringLiteral("durationSec"), 0);
        o.insert(QStringLiteral("fadeOutSec"), 0);
        o.insert(QStringLiteral("snoozeMin"), a.snoozeMin);
        o.insert(QStringLiteral("title"), a.label.isEmpty() ? QCoreApplication::translate("MobileReminders", "Wecker") : a.label);
        o.insert(QStringLiteral("text"), QCoreApplication::translate("MobileReminders", "Guten Morgen! %1").arg(a.time.toString(QStringLiteral("HH:mm"))));
        o.insert(QStringLiteral("ref"), double(a.id));
        o.insert(QStringLiteral("user"), double(a.userId));
        o.insert(QStringLiteral("fallback"), Tones::builtinAlarmFile());
        return o;
    }
    return {};
}

QJsonObject MobileReminders::sleepConfig(qint64 planId, int minutesOverride)
{
    Database &db = Database::instance();
    for (const SleepPlan &p : db.sleepPlans()) {
        if (p.id != planId) continue;
        const int minutes = minutesOverride > 0 ? minutesOverride : p.minutes;
        QJsonObject o;
        o.insert(QStringLiteral("files"), toArray(playlist(QStringLiteral("sleep"), p.id, p.playlist)));
        o.insert(QStringLiteral("shuffle"), p.shuffle);
        o.insert(QStringLiteral("volume"), p.volume);
        o.insert(QStringLiteral("fadeInSec"), 5);
        o.insert(QStringLiteral("durationSec"), minutes * 60);
        o.insert(QStringLiteral("fadeOutSec"), qMin(minutes, p.fadeOutMin) * 60);
        o.insert(QStringLiteral("title"), p.label.isEmpty() ? QCoreApplication::translate("MobileReminders", "Einschlafen") : p.label);
        o.insert(QStringLiteral("text"), QCoreApplication::translate("MobileReminders", "Gute Nacht"));
        o.insert(QStringLiteral("ref"), double(p.id));
        o.insert(QStringLiteral("user"), double(p.userId));
        return o;
    }
    return {};
}

QList<MobileReminders::Item> MobileReminders::plan(const QDateTime &now, int days, int max)
{
    QList<Item> l;
    Database &db = Database::instance();
    if (!db.isOpen()) return l;
    QHash<qint64, QString> names;
    const QList<User> users = db.users();
    for (const User &u : users) names.insert(u.id, u.name);
    const bool many = users.size() > 1;
    const QDateTime end = now.addDays(days);
    auto occurrences = [&](const QTime &t, int weekdays, int kind, qint64 ref, auto make) {
        QDateTime at = Schedule::next(t, weekdays, now.addSecs(1));
        int n = 0;
        while (at.isValid() && at <= end && n < 60) {
            Item it = make();
            it.id = makeId(kind, ref, n);
            it.at = at;
            l << it;
            ++n;
            at = Schedule::next(t, weekdays, at.addSecs(60));
        }
    };
    for (const BreathEvent &e : db.events()) {
        if (!e.enabled || !e.weekdays) continue;
        occurrences(e.time, e.weekdays, 1, e.id, [&] {
            Item it;
            it.kind = QStringLiteral("event");
            it.ref = e.id;
            it.user = e.userId;
            it.title = QCoreApplication::translate("MobileReminders", "Zeit für: %1").arg(e.name);
            QStringList parts{ BreathModules::displayName(e.moduleId) };
            if (many && names.contains(e.userId)) parts << names.value(e.userId);
            if (!e.note.isEmpty()) parts << e.note;
            it.text = parts.join(QStringLiteral(" · "));
            it.sound = e.sound;
            return it;
        });
    }
    for (const Alarm &a : db.alarms()) {
        if (!a.enabled || !a.weekdays) continue;
        const QJsonObject cfg = alarmConfig(a.id);
        occurrences(a.time, a.weekdays, 2, a.id, [&] {
            Item it;
            it.kind = QStringLiteral("alarm");
            it.ref = a.id;
            it.user = a.userId;
            it.title = cfg.value(QStringLiteral("title")).toString();
            it.text = cfg.value(QStringLiteral("text")).toString();
            it.play = cfg;
            return it;
        });
    }
    for (const SleepPlan &p : db.sleepPlans()) {
        if (!p.scheduled || !p.weekdays) continue;
        // automatisch nur ohne vorherige Übung (die braucht die Oberfläche) – sonst Benachrichtigung
        const bool autoPlay = p.autoStart && p.moduleId.isEmpty();
        const QJsonObject cfg = autoPlay ? sleepConfig(p.id) : QJsonObject();
        occurrences(p.time, p.weekdays, 3, p.id, [&] {
            Item it;
            it.kind = QStringLiteral("sleep");
            it.ref = p.id;
            it.user = p.userId;
            it.title = QCoreApplication::translate("MobileReminders", "Zeit zum Einschlafen");
            QStringList parts;
            if (!p.label.isEmpty()) parts << p.label;
            if (!p.moduleId.isEmpty()) parts << BreathModules::displayName(p.moduleId);
            if (many && names.contains(p.userId)) parts << names.value(p.userId);
            it.text = parts.join(QStringLiteral(" · "));
            it.sound = false;
            it.play = cfg;
            return it;
        });
    }
    std::sort(l.begin(), l.end(), [](const Item &x, const Item &y) { return x.at < y.at; });
    if (l.size() > max) l = l.mid(0, max);
    return l;
}

void MobileReminders::scheduleSoon()
{
    m_debounce.start();
}

void MobileReminders::scheduleNow()
{
    const QList<Item> items = plan(QDateTime::currentDateTime());
    m_count = int(items.size());
    m_next = items.isEmpty() ? QDateTime() : items.first().at;
    QJsonArray arr;
    for (const Item &it : items) {
        QJsonObject o{ { QStringLiteral("id"), it.id }, { QStringLiteral("at"), double(it.at.toMSecsSinceEpoch()) },
                       { QStringLiteral("kind"), it.kind }, { QStringLiteral("ref"), double(it.ref) },
                       { QStringLiteral("user"), double(it.user) }, { QStringLiteral("title"), it.title },
                       { QStringLiteral("text"), it.text }, { QStringLiteral("sound"), it.sound } };
        if (!it.play.isEmpty()) {
            QJsonObject play = it.play;
            const QVariantMap labels = Playback::labels();
            for (auto l = labels.cbegin(); l != labels.cend(); ++l) play.insert(l.key(), l.value().toString());
            play.insert(QStringLiteral("mode"), it.kind);
            o.insert(QStringLiteral("play"), play);
        }
        arr << o;
    }
    const QByteArray planJson = QJsonDocument(arr).toJson(QJsonDocument::Compact);
    if (planJson == m_lastPlan) return;
    m_lastPlan = planJson;
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(kReminders, "ensureChannels",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", context().object(),
        jstr(tr("Übungs-Erinnerungen")).object<jstring>(), jstr(tr("Erinnerungen (ohne Ton)")).object<jstring>(),
        jstr(tr("Erinnerungen an geplante Atemübungen und ans Einschlafen")).object<jstring>());
    QJniObject::callStaticMethod<void>(kReminders, "replaceAll", "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
        context().object(), jstr(QString::fromUtf8(planJson)).object<jstring>(),
        jstr(tr("Erinnerung")).object<jstring>());
#endif
}

void MobileReminders::clear()
{
    m_debounce.stop();
}

void MobileReminders::requestPermission()
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<jboolean>(kReminders, "ensurePermission", "(Landroid/content/Context;)Z", context().object());
#endif
}

bool MobileReminders::permissionGranted() const
{
#ifdef Q_OS_ANDROID
    return QJniObject::callStaticMethod<jboolean>(kReminders, "permissionGranted", "(Landroid/content/Context;)Z", context().object());
#else
    return true;
#endif
}

bool MobileReminders::exactAllowed() const
{
#ifdef Q_OS_ANDROID
    return QJniObject::callStaticMethod<jboolean>(kReminders, "exactAllowed", "(Landroid/content/Context;)Z", context().object());
#else
    return true;
#endif
}

void MobileReminders::openExactSettings()
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(kReminders, "openExactSettings", "(Landroid/content/Context;)V", context().object());
#endif
}
