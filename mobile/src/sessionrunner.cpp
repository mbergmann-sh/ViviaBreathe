// ViviaBreathe mobil – Ablauf einer Atemübung (siehe sessionrunner.h)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "sessionrunner.h"
#include "platform.h"

#include "audio.h"
#include "database.h"

#include <QGuiApplication>
#include <QPainter>
#include <QTimer>
#include <QVariantList>
#include <QtMath>

// ---------------------------------------------------------------- Einstellungen ↔ QML

namespace SettingsMap {

QVariantMap toMap(const ModuleDef &m, const ModuleSettings &s)
{
    QVariantMap r;
    QVariantList phases;
    for (int i = 0; i < m.phases.size(); ++i) phases << s.phaseSec.value(i, m.phases.at(i).defaultSec);
    QVariantList repeats;
    for (int v : s.repeats) repeats << v;
    r.insert(QStringLiteral("phaseSec"), phases);
    r.insert(QStringLiteral("repeats"), repeats);
    r.insert(QStringLiteral("totalSec"), s.totalSec);
    r.insert(QStringLiteral("signal"), s.signal);
    r.insert(QStringLiteral("doneGong"), s.doneGong);
    r.insert(QStringLiteral("distinctTones"), s.distinctTones);
    r.insert(QStringLiteral("signalVolume"), s.signalVolume);
    r.insert(QStringLiteral("vibrate"), s.vibrate);
    r.insert(QStringLiteral("musicEnabled"), s.musicEnabled);
    r.insert(QStringLiteral("musicFile"), s.musicFile);
    r.insert(QStringLiteral("musicName"), Platform::displayName(s.musicFile));
    r.insert(QStringLiteral("musicVolume"), s.musicVolume);
    r.insert(QStringLiteral("trainingMode"), s.trainingMode);
    r.insert(QStringLiteral("askSpo2"), s.askSpo2);
    r.insert(QStringLiteral("askPeakFlow"), s.askPeakFlow);
    r.insert(QStringLiteral("askPulse"), s.askPulse);
    r.insert(QStringLiteral("askFeeling"), s.askFeeling);
    r.insert(QStringLiteral("eventName"), s.eventName);
    r.insert(QStringLiteral("showTips"), s.showTips);
    r.insert(QStringLiteral("deviceTarget"), s.deviceTarget);
    r.insert(QStringLiteral("pattern"), s.pattern());
    r.insert(QStringLiteral("roundSec"), s.roundSec(m));
    return r;
}

ModuleSettings fromMap(const ModuleDef &m, const QVariantMap &map, const ModuleSettings &base)
{
    ModuleSettings s = base;
    const QVariantList ph = map.value(QStringLiteral("phaseSec")).toList();
    if (ph.size() == m.phases.size())
        for (int i = 0; i < ph.size(); ++i)
            s.phaseSec[i] = qBound(m.phases.at(i).minSec, ph.at(i).toDouble(), m.phases.at(i).maxSec);
    const QVariantList rp = map.value(QStringLiteral("repeats")).toList();
    const QList<SegmentDef> segs = m.effectiveSegments();
    if (rp.size() == segs.size())
        for (int i = 0; i < rp.size(); ++i)
            s.repeats[i] = qBound(segs.at(i).minRepeats, rp.at(i).toInt(), segs.at(i).maxRepeats);
    auto b = [&](const char *k, bool &v) { if (map.contains(QLatin1String(k))) v = map.value(QLatin1String(k)).toBool(); };
    auto n = [&](const char *k, int &v) { if (map.contains(QLatin1String(k))) v = map.value(QLatin1String(k)).toInt(); };
    n("totalSec", s.totalSec);
    s.totalSec = qBound(m.singleRound ? 10 : 60, s.totalSec, m.maxTotalSec);
    b("signal", s.signal);
    b("doneGong", s.doneGong);
    b("distinctTones", s.distinctTones);
    n("signalVolume", s.signalVolume);
    b("vibrate", s.vibrate);
    b("musicEnabled", s.musicEnabled);
    if (map.contains(QStringLiteral("musicFile"))) s.musicFile = map.value(QStringLiteral("musicFile")).toString();
    n("musicVolume", s.musicVolume);
    b("trainingMode", s.trainingMode);
    b("askSpo2", s.askSpo2);
    b("askPeakFlow", s.askPeakFlow);
    b("askPulse", s.askPulse);
    b("askFeeling", s.askFeeling);
    if (map.contains(QStringLiteral("eventName"))) s.eventName = map.value(QStringLiteral("eventName")).toString().trimmed();
    b("showTips", s.showTips);
    n("deviceTarget", s.deviceTarget);
    s.deviceTarget = qBound(1, s.deviceTarget, 3);
    s.signalVolume = qBound(0, s.signalVolume, 100);
    s.musicVolume = qBound(0, s.musicVolume, 100);
    return s;
}

} // namespace SettingsMap

// ---------------------------------------------------------------- Ablauf

namespace {
Tones::Kind toneFor(PhaseKind k)
{
    switch (k) {
    case PhaseKind::Inhale: return Tones::Kind::Inhale;
    case PhaseKind::Exhale:
    case PhaseKind::Huff:   return Tones::Kind::Exhale;
    case PhaseKind::Calm:   return Tones::Kind::Phase;
    case PhaseKind::Ready:  return Tones::Kind::Reminder;
    default:                return Tones::Kind::Hold;
    }
}
int value(const QVariantMap &m, const char *key)
{
    const QVariant v = m.value(QLatin1String(key));
    if (!v.isValid() || v.isNull() || v.toString().trimmed().isEmpty()) return kNoValue;
    bool ok = false;
    const int i = v.toString().trimmed().toInt(&ok);
    return ok && i >= 0 ? i : kNoValue;
}
}

SessionRunner::SessionRunner(QObject *parent) : QObject(parent)
{
    m_engine = new BreathEngine(this);
    m_music = new MusicPlayer(this);
    m_renderer.setEngine(m_engine);
    m_renderer.setCaption(false);            // Phase und Hinweis zeigt die QML-Seite
    connect(m_engine, &BreathEngine::tick, this, &SessionRunner::tick);
    connect(m_engine, &BreathEngine::phaseChanged, this, &SessionRunner::onPhaseChanged);
    connect(m_engine, &BreathEngine::finished, this, &SessionRunner::onFinished);
    connect(m_engine, &BreathEngine::prerollSecond, this, [this](int) { emit tick(); });
    // App verlässt den Vordergrund (anderer App, Anruf): Übung anhalten statt im Hintergrund weiterzuzählen
    connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState st) {
        if (st != Qt::ApplicationSuspended && st != Qt::ApplicationHidden) return;
        if (m_engine->state() == BreathEngine::State::Running || m_engine->state() == BreathEngine::State::Preroll)
            togglePause();
    });
}

bool SessionRunner::prepare(qint64 userId, const QString &moduleId, const QVariantMap &settings)
{
    m_error.clear();
    const ModuleDef *m = BreathModules::find(moduleId);
    if (!m) { m_error = tr("Unbekannte Übung."); return false; }
    m_engine->stop();
    m_module = m;
    m_userId = userId;
    m_settings = SettingsMap::fromMap(*m, settings, ModuleSettings::load(userId, *m));
    if (m_settings.cycleSec() <= 0) {
        m_error = tr("Bitte mindestens eine Phase mit einer Dauer einstellen.");
        m_module = nullptr;
        return false;
    }
    m_rec = SessionRecord();
    m_rec.userId = userId;
    m_rec.moduleId = m->id;
    m_rec.eventName = m_settings.eventName;
    m_rec.plannedSec = m->singleRound ? int(m_settings.roundSec(*m) + 0.5) : m_settings.totalSec;
    m_rec.pattern = m_settings.pattern();
    m_renderer.setModule(m, m_settings.phaseSec);
    m_renderer.setDeviceTarget(m_settings.deviceTarget);
    m_engine->setup(m_settings.round(*m), m_settings.totalSec, m->quickHelp ? 1 : 3);
    emit changed();
    emit tick();
    return true;
}

void SessionRunner::setBefore(const QVariantMap &v)
{
    m_rec.spo2Before = value(v, "spo2");
    m_rec.pfBefore = value(v, "peakFlow");
    m_rec.pulseBefore = value(v, "pulse");
    m_rec.feelingBefore = value(v, "feeling");
}

void SessionRunner::start()
{
    if (!m_module) return;
    m_rec.started = QDateTime::currentDateTime();
    Platform::keepScreenOn(true);
    if (musicOn()) {
        m_music->setPlaylist({ m_settings.musicFile });
        m_music->setVolume(0);
        m_music->play();
        m_music->fadeTo(m_settings.musicVolume, 4000);
    }
    m_engine->start();
    emit changed();
}

void SessionRunner::togglePause()
{
    if (m_engine->state() == BreathEngine::State::Paused) {
        m_engine->resume();
        m_music->resume();
    } else if (m_engine->state() == BreathEngine::State::Running || m_engine->state() == BreathEngine::State::Preroll) {
        m_engine->pause();
        m_music->pause();
    }
    emit changed();
    emit tick();
}

void SessionRunner::proceed()
{
    m_engine->proceed();
    emit changed();
}

void SessionRunner::stop()
{
    if (m_engine->state() == BreathEngine::State::Idle || m_engine->state() == BreathEngine::State::Finished) return;
    m_engine->stop();
}

void SessionRunner::abort()
{
    m_engine->stop();
    m_music->stop();
    Platform::keepScreenOn(false);
    reset();
}

void SessionRunner::reset()
{
    m_module = nullptr;
    m_rec = SessionRecord();
    emit changed();
}

void SessionRunner::onPhaseChanged(int, PhaseKind kind)
{
    if (m_settings.signal)
        Tones::play(m_settings.distinctTones ? toneFor(kind) : Tones::Kind::Phase, m_settings.signalVolume);
    if (m_settings.vibrate) Platform::vibrate(kind == PhaseKind::Inhale ? 160 : 90);
    emit changed();
}

int SessionRunner::repetitionsTotal() const
{
    return qMax(1, m_settings.repeats.value(0, 1));
}

void SessionRunner::onFinished(bool completed)
{
    if (!m_module) return;
    m_rec.actualSec = int(m_engine->elapsed() + 0.5);
    m_rec.cycles = m_engine->cycles();
    if (m_module->singleRound) {
        const int total = repetitionsTotal();
        const int perRep = qMax(1, int(m_engine->steps().size()) / total);
        m_rec.cycles = completed ? total : m_engine->phaseIndex() / perRep;
    }
    m_rec.completed = completed;
    m_music->fadeTo(0, completed ? 5000 : 1500, true);
    if (completed && m_settings.doneGong) Tones::play(Tones::Kind::Done, m_settings.signalVolume);
    Platform::keepScreenOn(false);
    emit changed();
    emit tick();
    emit finished(completed);
}

void SessionRunner::setAfter(const QVariantMap &v)
{
    m_rec.spo2After = value(v, "spo2");
    m_rec.pfAfter = value(v, "peakFlow");
    m_rec.pulseAfter = value(v, "pulse");
    m_rec.feelingAfter = value(v, "feeling");
}

QVariantMap SessionRunner::summary() const
{
    QVariantMap r;
    if (!m_module) return r;
    r.insert(QStringLiteral("module"), m_module->name);
    r.insert(QStringLiteral("event"), m_rec.eventName);
    r.insert(QStringLiteral("pattern"), m_rec.pattern);
    r.insert(QStringLiteral("duration"), tr("%1 von geplant %2").arg(formatDuration(m_rec.actualSec), formatDuration(m_rec.plannedSec)));
    r.insert(QStringLiteral("cyclesLabel"), m_module->singleRound ? tr("Wiederholungen") : tr("Atemzyklen"));
    r.insert(QStringLiteral("cycles"), m_rec.cycles);
    r.insert(QStringLiteral("completed"), m_rec.completed);
    QVariantList rows;
    auto row = [&](const QString &name, int a, int b, const QString &unit) {
        if (a < 0 && b < 0) return;
        rows << QVariantMap{ { QStringLiteral("name"), name },
                             { QStringLiteral("before"), a >= 0 ? QString::number(a) + unit : QStringLiteral("–") },
                             { QStringLiteral("after"), b >= 0 ? QString::number(b) + unit : QStringLiteral("–") } };
    };
    row(tr("SpO₂"), m_rec.spo2Before, m_rec.spo2After, QStringLiteral(" %"));
    row(tr("Peak Flow"), m_rec.pfBefore, m_rec.pfAfter, QStringLiteral(" l/min"));
    row(tr("Puls"), m_rec.pulseBefore, m_rec.pulseAfter, QStringLiteral(" /min"));
    row(tr("Befinden"), m_rec.feelingBefore, m_rec.feelingAfter, QStringLiteral("/10"));
    r.insert(QStringLiteral("measurements"), rows);
    r.insert(QStringLiteral("deviceTraining"), m_module->deviceTraining);
    r.insert(QStringLiteral("deviceTarget"), m_settings.deviceTarget);
    r.insert(QStringLiteral("deviceHoldSuggestion"), qRound(m_settings.phaseSec.value(2, 3)));
    return r;
}

bool SessionRunner::save(const QString &note, int deviceBalls, int deviceHoldSec)
{
    if (!m_module) return false;
    m_rec.note = note.trimmed();
    if (m_module->deviceTraining) {
        m_rec.deviceBalls = deviceBalls >= 0 ? qBound(0, deviceBalls, 3) : kNoValue;
        m_rec.deviceHoldSec = deviceHoldSec >= 0 ? deviceHoldSec : kNoValue;
    }
    QString err;
    if (!Database::instance().saveSession(m_rec, &err)) {
        m_error = err;
        return false;
    }
    Database::instance().setUserSetting(m_userId, QStringLiteral("ui/lastModule"), m_module->id);
    emit saved();
    reset();
    return true;
}

void SessionRunner::paint(QPainter *p, const QRectF &rect, const QColor &fg, bool dark, const QFont &font)
{
    if (!m_module) return;
    m_renderer.paint(*p, rect, fg, dark, font);
}

QString SessionRunner::state() const
{
    switch (m_engine->state()) {
    case BreathEngine::State::Idle:     return QStringLiteral("idle");
    case BreathEngine::State::Preroll:  return QStringLiteral("preroll");
    case BreathEngine::State::Running:  return QStringLiteral("running");
    case BreathEngine::State::Paused:   return QStringLiteral("paused");
    case BreathEngine::State::Waiting:  return QStringLiteral("waiting");
    case BreathEngine::State::Finished: return QStringLiteral("finished");
    }
    return QString();
}

QString SessionRunner::title() const
{
    QString t, s;
    m_renderer.captionTexts(t, s);
    if (m_engine->state() == BreathEngine::State::Preroll)
        return QString::number(qCeil(m_engine->prerollRemaining()));
    return t;
}

QString SessionRunner::sub() const
{
    QString t, s;
    m_renderer.captionTexts(t, s);
    if (m_engine->state() == BreathEngine::State::Preroll) return t;   // „Gleich geht's los …“
    return s;
}

QString SessionRunner::info() const
{
    if (!m_module) return {};
    const auto st = m_engine->state();
    if (st == BreathEngine::State::Preroll || st == BreathEngine::State::Idle) return {};
    const int el = int(m_engine->elapsed());
    const int rem = int(qCeil(m_engine->remaining()));
    if (m_module->singleRound) {
        const int total = repetitionsTotal();
        const int perRep = qMax(1, int(m_engine->steps().size()) / total);
        const int rep = st == BreathEngine::State::Finished ? total : qMin(total, m_engine->phaseIndex() / perRep + 1);
        return tr("Wiederholung %1 von %2 · vergangen %3").arg(rep).arg(total).arg(formatDuration(el));
    }
    return (m_module->hasSegments() ? tr("Runde %1 · vergangen %2 · noch %3") : tr("Zyklus %1 · vergangen %2 · noch %3"))
        .arg(m_engine->cycles() + (st == BreathEngine::State::Finished ? 0 : 1))
        .arg(formatDuration(el), formatDuration(rem));
}

int SessionRunner::phaseRemaining() const
{
    const auto st = m_engine->state();
    if (st != BreathEngine::State::Running && st != BreathEngine::State::Paused) return 0;
    return int(qCeil(m_engine->phaseRemaining()));
}

double SessionRunner::totalProgress() const
{
    if (!m_module) return 0;
    if (m_engine->state() == BreathEngine::State::Finished) return 1;
    const double total = m_module->singleRound ? m_settings.roundSec(*m_module) : m_settings.totalSec;
    return total > 0 ? qBound(0.0, m_engine->elapsed() / total, 1.0) : 0;
}

void SessionRunner::setSoundOn(bool on)
{
    if (on == soundOn()) return;
    m_settings.signal = on;
    m_settings.doneGong = on;
    if (m_module) {
        // auch für das nächste Mal merken
        ModuleSettings stored = ModuleSettings::load(m_userId, *m_module);
        stored.signal = on;
        stored.doneGong = on;
        stored.save(m_userId, *m_module);
    }
    emit changed();
}

void SessionRunner::setMusicVolume(int v)
{
    v = qBound(0, v, 100);
    if (v == m_settings.musicVolume) return;
    m_settings.musicVolume = v;
    m_music->setVolume(v);
    emit changed();
}

QVariantMap SessionRunner::askFields() const
{
    return { { QStringLiteral("spo2"), m_settings.askSpo2 }, { QStringLiteral("peakFlow"), m_settings.askPeakFlow },
             { QStringLiteral("pulse"), m_settings.askPulse }, { QStringLiteral("feeling"), m_settings.askFeeling } };
}
