// ViviaBreathe mobil – Musik für Wecker und Einschlafen (siehe playback.h)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "playback.h"
#include "audio.h"
#include "platform.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniObject>
#include <QtCore/qcoreapplication_platform.h>

namespace {
const char *kService = "de/mbergmann/viviabreathe/PlaybackService";
QJniObject context() { return QJniObject(QNativeInterface::QAndroidApplication::context()); }
}
#endif

Playback::Playback(QObject *parent) : QObject(parent)
{
    m_poll = new QTimer(this);
    m_poll->setInterval(1000);
    connect(m_poll, &QTimer::timeout, this, &Playback::refresh);
    m_poll->start();
#ifndef Q_OS_ANDROID
    m_player = new MusicPlayer(this);
    connect(m_player, &MusicPlayer::trackChanged, this, [this](const QString &f) {
        m_track = Platform::displayName(f);
        emit changed();
    });
#endif
}

QVariantMap Playback::labels()
{
    return { { QStringLiteral("stopLabel"), tr("Aus") },
             { QStringLiteral("snoozeLabel"), tr("Schlummern") },
             { QStringLiteral("channelName"), tr("Wecker und Einschlafmusik") },
             { QStringLiteral("remainLabel"), tr("noch %1 min") } };
}

void Playback::start(const QString &mode, const QVariantMap &config)
{
    QVariantMap c = config;
    const QVariantMap l = labels();
    for (auto it = l.cbegin(); it != l.cend(); ++it)
        if (!c.contains(it.key())) c.insert(it.key(), it.value());
    c.insert(QStringLiteral("mode"), mode);
    if (!c.contains(QStringLiteral("fallback"))) c.insert(QStringLiteral("fallback"), Tones::builtinAlarmFile());
#ifdef Q_OS_ANDROID
    const QByteArray json = QJsonDocument(QJsonObject::fromVariantMap(c)).toJson(QJsonDocument::Compact);
    QJniObject::callStaticMethod<void>(kService, "start", "(Landroid/content/Context;Ljava/lang/String;)V",
                                       context().object(), QJniObject::fromString(QString::fromUtf8(json)).object<jstring>());
    QTimer::singleShot(400, this, &Playback::refresh);
#else
    m_config = c;
    m_started = QDateTime::currentDateTime();
    m_fadingOut = false;
    QStringList files = c.value(QStringLiteral("files")).toStringList();
    if (files.isEmpty()) files << c.value(QStringLiteral("fallback")).toString();
    m_player->setPlaylist(files, c.value(QStringLiteral("shuffle")).toBool());
    m_player->setLoop(true);
    m_player->setVolume(0);
    m_player->play();
    const int fadeIn = c.value(QStringLiteral("fadeInSec")).toInt();
    m_player->fadeTo(c.value(QStringLiteral("volume"), 50).toInt(), qMax(1500, fadeIn * 1000));
    setState(true, mode, c.value(QStringLiteral("title")).toString(), c.value(QStringLiteral("durationSec")).toInt(),
             m_track, c.value(QStringLiteral("ref")).toLongLong());
#endif
}

void Playback::stop()
{
    const QString mode = m_mode;
    const qint64 ref = m_ref;
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(kService, "stopNow", "(Landroid/content/Context;)V", context().object());
#else
    m_player->stop();
#endif
    if (m_running) {
        setState(false, QString(), QString(), 0, QString(), 0);
        emit ended(mode, ref);
    }
}

void Playback::snooze()
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(kService, "snooze", "(Landroid/content/Context;)V", context().object());
    setState(false, QString(), QString(), 0, QString(), 0);
#else
    // Desktop: nach der Schlummerzeit erneut starten
    const QVariantMap c = m_config;
    const int min = qMax(1, c.value(QStringLiteral("snoozeMin"), 9).toInt());
    m_player->stop();
    setState(false, QString(), QString(), 0, QString(), 0);
    QTimer::singleShot(min * 60000, this, [this, c] { start(QStringLiteral("alarm"), c); });
#endif
}

void Playback::refresh()
{
#ifdef Q_OS_ANDROID
    const QString json = QJniObject::callStaticObjectMethod(kService, "state", "()Ljava/lang/String;").toString();
    const QJsonObject o = QJsonDocument::fromJson(json.toUtf8()).object();
    const bool wasRunning = m_running;
    const QString mode = m_mode;
    const qint64 ref = m_ref;
    setState(o.value(QStringLiteral("running")).toBool(), o.value(QStringLiteral("mode")).toString(),
             o.value(QStringLiteral("title")).toString(), o.value(QStringLiteral("remainingSec")).toInt(),
             o.value(QStringLiteral("track")).toString(), qint64(o.value(QStringLiteral("ref")).toDouble()));
    if (wasRunning && !m_running) emit ended(mode, ref);
#else
    if (!m_running) return;
    const int duration = m_config.value(QStringLiteral("durationSec")).toInt();
    if (duration <= 0) return;
    const int el = int(m_started.secsTo(QDateTime::currentDateTime()));
    const int rem = qMax(0, duration - el);
    const int fadeOut = m_config.value(QStringLiteral("fadeOutSec")).toInt();
    if (!m_fadingOut && rem <= fadeOut) {
        m_fadingOut = true;
        m_player->fadeTo(0, qMax(1, rem) * 1000, false);
    }
    if (rem <= 0) {
        stop();
        return;
    }
    setState(true, m_mode, m_title, rem, m_track, m_ref);
#endif
}

void Playback::setState(bool running, const QString &mode, const QString &title, int remaining, const QString &track, qint64 ref)
{
    if (running == m_running && mode == m_mode && title == m_title && remaining == m_remaining && track == m_track && ref == m_ref)
        return;
    m_running = running;
    m_mode = mode;
    m_title = title;
    m_remaining = remaining;
    m_track = track;
    m_ref = ref;
    emit changed();
}
