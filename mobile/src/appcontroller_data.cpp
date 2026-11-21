// ViviaBreathe mobil – Übungen, Tagebuch, Verlauf und Berichte (Teil von AppController)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "appcontroller.h"
#include "platform.h"
#include "sessionrunner.h"

#include "breathmodules.h"
#include "chartrenderer.h"
#include "database.h"
#include "reportdoc.h"
#include "schedule.h"

#include <QFile>
#include <QLocale>
#include <QSet>
#include <QStandardPaths>

namespace {
QDateTime dayStart(const QString &iso) { return QDateTime(QDate::fromString(iso, Qt::ISODate), QTime(0, 0)); }

QString musicKey(const QString &moduleId) { return QStringLiteral("mobile/music/") + moduleId; }
QString musicOnKey(const QString &moduleId) { return QStringLiteral("mobile/musicOn/") + moduleId; }

// Musik ist gerätebezogen: Ein Pfad vom Computer gilt auf dem Telefon nicht (und umgekehrt).
ModuleSettings withDeviceMusic(const ModuleSettings &s, const QString &moduleId)
{
    ModuleSettings r = s;
    Database &db = Database::instance();
    r.musicFile = db.deviceSetting(musicKey(moduleId));
    r.musicEnabled = db.deviceSetting(musicOnKey(moduleId)) == QLatin1String("1") && !r.musicFile.isEmpty();
    return r;
}

QString measureText(const SessionRecord &s)
{
    QStringList parts;
    auto pair = [&](const QString &name, int a, int b, const QString &unit) {
        if (a < 0 && b < 0) return;
        parts << QStringLiteral("%1 %2 → %3%4").arg(name, a >= 0 ? QString::number(a) : QStringLiteral("–"),
                                                   b >= 0 ? QString::number(b) : QStringLiteral("–"), unit);
    };
    pair(AppController::tr("SpO₂"), s.spo2Before, s.spo2After, QStringLiteral(" %"));
    pair(AppController::tr("PF"), s.pfBefore, s.pfAfter, QString());
    pair(AppController::tr("Puls"), s.pulseBefore, s.pulseAfter, QString());
    pair(AppController::tr("Befinden"), s.feelingBefore, s.feelingAfter, QString());
    return parts.join(QStringLiteral(" · "));
}

QString deviceText(const SessionRecord &s)
{
    QStringList t;
    if (s.deviceBalls >= 0) t << AppController::tr("%n Ball/Bälle", nullptr, s.deviceBalls);
    if (s.deviceHoldSec >= 0) t << AppController::tr("%1 s gehalten").arg(s.deviceHoldSec);
    return t.join(QStringLiteral(", "));
}

QString numText(int v) { return v >= 0 ? QString::number(v) : QString(); }
int numValue(const QVariant &v)
{
    bool ok = false;
    const int i = v.toString().trimmed().toInt(&ok);
    return ok && i >= 0 ? i : kNoValue;
}
} // namespace

// ---------------------------------------------------------------- Übungen

QString AppController::emergencyModuleId() const
{
    for (const ModuleDef &m : BreathModules::all())
        if (m.quickHelp) return m.id;
    return {};
}

QVariantList AppController::modules() const
{
    QVariantList l;
    if (!loggedIn() || !m_userId) return l;
    const QList<SessionRecord> recent = Database::instance().sessions(m_userId, QDateTime::currentDateTime().addDays(-365));
    QHash<QString, QDateTime> last;
    for (const SessionRecord &s : recent)
        if (!last.contains(s.moduleId) || last.value(s.moduleId) < s.started) last.insert(s.moduleId, s.started);
    for (const ModuleDef &m : BreathModules::all()) {
        const ModuleSettings st = withDeviceMusic(ModuleSettings::load(m_userId, m), m.id);
        QStringList parts;
        if (m.singleRound) {
            parts << tr("%n Wiederholung(en)", nullptr, st.repeats.value(0, 1));
            if (m.deviceTraining) parts << tr("Ziel: %n Ball/Bälle", nullptr, st.deviceTarget);
        } else if (m.hasSegments()) {
            QStringList segs;
            const QList<SegmentDef> sd = m.effectiveSegments();
            for (int g = 0; g < sd.size(); ++g)
                segs << (st.repeats.value(g, 1) > 1 ? QStringLiteral("%1 × %2").arg(st.repeats.value(g)).arg(sd.at(g).title)
                                                     : sd.at(g).title);
            parts << segs.join(QStringLiteral(" → "));
        } else {
            parts << st.pattern() + tr(" s");
        }
        if (!m.singleRound) parts << tr("%1 min").arg(st.totalSec / 60);
        if (st.musicEnabled) parts << tr("mit Musik");
        if (st.trainingMode) parts << tr("Trainingsmodus");
        QVariantMap c;
        c.insert(QStringLiteral("id"), m.id);
        c.insert(QStringLiteral("name"), m.name);
        c.insert(QStringLiteral("shortName"), m.shortName);
        c.insert(QStringLiteral("category"), m.category);
        c.insert(QStringLiteral("description"), m.description);
        c.insert(QStringLiteral("icon"), m.icon);
        c.insert(QStringLiteral("quickHelp"), m.quickHelp);
        c.insert(QStringLiteral("airway"), m.airway);
        c.insert(QStringLiteral("hasGuide"), !m.guide.isEmpty());
        c.insert(QStringLiteral("settingsText"), parts.join(QStringLiteral(" · ")));
        c.insert(QStringLiteral("lastText"), last.contains(m.id) ? Schedule::relative(last.value(m.id)) : QString());
        c.insert(QStringLiteral("color"), ChartRenderer::moduleColor(m.id).name());
        l << c;
    }
    return l;
}

QVariantMap AppController::moduleInfo(const QString &id) const
{
    QVariantMap r;
    const ModuleDef *m = BreathModules::find(id);
    if (!m) return r;
    r.insert(QStringLiteral("id"), m->id);
    r.insert(QStringLiteral("name"), m->name);
    r.insert(QStringLiteral("shortName"), m->shortName);
    r.insert(QStringLiteral("category"), m->category);
    r.insert(QStringLiteral("description"), m->description);
    r.insert(QStringLiteral("icon"), m->icon);
    r.insert(QStringLiteral("tips"), m->tips);
    r.insert(QStringLiteral("generalTips"), m->airway ? BreathModules::airwayTips() : BreathModules::generalTips());
    r.insert(QStringLiteral("posture"), m->posture);
    r.insert(QStringLiteral("emergencyNote"), m->emergencyNote);
    r.insert(QStringLiteral("postureImage"), m->postureImage);
    r.insert(QStringLiteral("quickHelp"), m->quickHelp);
    r.insert(QStringLiteral("airway"), m->airway);
    r.insert(QStringLiteral("singleRound"), m->singleRound);
    r.insert(QStringLiteral("deviceTraining"), m->deviceTraining);
    r.insert(QStringLiteral("hasSegments"), m->hasSegments());
    r.insert(QStringLiteral("defaultTotalSec"), m->defaultTotalSec);
    r.insert(QStringLiteral("maxTotalSec"), m->maxTotalSec);
    r.insert(QStringLiteral("minTotalSec"), m->singleRound ? 10 : 60);
    QVariantList guide;
    for (const GuideStep &g : m->guide)
        guide << QVariantMap{ { QStringLiteral("image"), g.image }, { QStringLiteral("text"), g.text } };
    r.insert(QStringLiteral("guide"), guide);
    QVariantList phases;
    for (const PhaseDef &p : m->phases)
        phases << QVariantMap{ { QStringLiteral("name"), phaseName(p.kind) }, { QStringLiteral("hint"), p.hint },
                               { QStringLiteral("min"), p.minSec }, { QStringLiteral("max"), p.maxSec },
                               { QStringLiteral("def"), p.defaultSec } };
    r.insert(QStringLiteral("phases"), phases);
    QVariantList segs;
    for (const SegmentDef &g : m->effectiveSegments())
        segs << QVariantMap{ { QStringLiteral("title"), g.title }, { QStringLiteral("min"), g.minRepeats },
                             { QStringLiteral("max"), g.maxRepeats }, { QStringLiteral("def"), g.defaultRepeats },
                             { QStringLiteral("first"), g.firstPhase }, { QStringLiteral("count"), g.phaseCount } };
    r.insert(QStringLiteral("segments"), segs);
    return r;
}

QVariantMap AppController::moduleSettings(const QString &id) const
{
    const ModuleDef *m = BreathModules::find(id);
    if (!m || !loggedIn()) return {};
    return SettingsMap::toMap(*m, withDeviceMusic(ModuleSettings::load(m_userId, *m), id));
}

QVariantMap AppController::defaultSettings(const QString &id) const
{
    const ModuleDef *m = BreathModules::find(id);
    if (!m) return {};
    // Zurücksetzen betrifft die Atemwerte; Musik und Töne bleiben, wie sie sind
    const ModuleSettings cur = withDeviceMusic(ModuleSettings::load(m_userId, *m), id);
    ModuleSettings d = ModuleSettings::defaults(*m);
    d.musicEnabled = cur.musicEnabled;
    d.musicFile = cur.musicFile;
    d.musicVolume = cur.musicVolume;
    d.signal = cur.signal;
    d.doneGong = cur.doneGong;
    d.distinctTones = cur.distinctTones;
    d.signalVolume = cur.signalVolume;
    d.vibrate = cur.vibrate;
    d.trainingMode = cur.trainingMode;
    return SettingsMap::toMap(*m, d);
}

bool AppController::saveModuleSettings(const QString &id, const QVariantMap &map)
{
    const ModuleDef *m = BreathModules::find(id);
    if (!m || !loggedIn() || !m_userId) return false;
    const ModuleSettings stored = ModuleSettings::load(m_userId, *m);
    ModuleSettings s = SettingsMap::fromMap(*m, map, stored);
    Database &db = Database::instance();
    db.setDeviceSetting(musicKey(id), s.musicFile);
    db.setDeviceSetting(musicOnKey(id), s.musicEnabled ? QStringLiteral("1") : QStringLiteral("0"));
    s.musicFile = stored.musicFile;           // abgeglichene Werte (anderer Geräte) unverändert lassen
    s.musicEnabled = stored.musicEnabled;
    s.save(m_userId, *m);
    return true;
}

QVariantMap AppController::settingsPreview(const QString &id, const QVariantMap &map) const
{
    QVariantMap r;
    const ModuleDef *m = BreathModules::find(id);
    if (!m) return r;
    const ModuleSettings s = SettingsMap::fromMap(*m, map, ModuleSettings::defaults(*m));
    r.insert(QStringLiteral("pattern"), s.pattern());
    const double round = s.roundSec(*m);
    r.insert(QStringLiteral("roundSec"), round);
    if (m->singleRound) {
        r.insert(QStringLiteral("text"), tr("Dauer der Übung: etwa %1").arg(formatDuration(int(round + 0.5))));
    } else if (round > 0) {
        const int cycles = int(s.totalSec / round);
        r.insert(QStringLiteral("text"), (m->hasSegments() ? tr("Eine Runde dauert %1 s – in %2 etwa %n Runde(n).", nullptr, cycles)
                                                            : tr("Ein Atemzyklus dauert %1 s – in %2 etwa %n Zyklen.", nullptr, cycles))
                                             .arg(formatSeconds(round), formatDuration(s.totalSec)));
    } else {
        r.insert(QStringLiteral("text"), tr("Bitte mindestens eine Phase mit einer Dauer einstellen."));
    }
    return r;
}

QStringList AppController::eventNames() const
{
    if (!loggedIn() || !m_userId) return {};
    return Database::instance().eventNames(m_userId);
}

bool AppController::startSession(const QString &moduleId, const QVariantMap &settings)
{
    if (!loggedIn() || !m_userId) return false;
    saveModuleSettings(moduleId, settings);
    if (!m_runner->prepare(m_userId, moduleId, settings)) {
        setError(m_runner->lastError());
        return false;
    }
    return true;
}

QString AppController::rememberFile(const QString &url)
{
    Platform::persistUri(url);
    return Platform::displayName(url);
}

QString AppController::fileName(const QString &f) const { return Platform::displayName(f); }

// ---------------------------------------------------------------- Tagebuch

QVariantList AppController::sessions(const QString &fromIso, const QString &toIso, const QString &moduleId) const
{
    QVariantList l;
    if (!loggedIn() || !m_userId) return l;
    const QList<SessionRecord> all = Database::instance().sessions(m_userId, dayStart(fromIso), dayStart(toIso).addDays(1));
    const QLocale loc;
    for (const SessionRecord &s : all) {                    // die Datenbank liefert die neuesten zuerst
        if (!moduleId.isEmpty() && s.moduleId != moduleId) continue;
        QVariantMap r;
        r.insert(QStringLiteral("id"), s.id);
        r.insert(QStringLiteral("day"), s.started.date().toString(Qt::ISODate));
        r.insert(QStringLiteral("dayText"), loc.toString(s.started.date(), QStringLiteral("dddd, d. MMMM yyyy")));
        r.insert(QStringLiteral("time"), s.started.time().toString(QStringLiteral("HH:mm")));
        r.insert(QStringLiteral("module"), BreathModules::displayName(s.moduleId));
        r.insert(QStringLiteral("moduleId"), s.moduleId);
        r.insert(QStringLiteral("color"), ChartRenderer::moduleColor(s.moduleId).name());
        r.insert(QStringLiteral("event"), s.eventName);
        r.insert(QStringLiteral("pattern"), s.pattern);
        r.insert(QStringLiteral("duration"), formatDuration(s.actualSec));
        r.insert(QStringLiteral("completed"), s.completed);
        r.insert(QStringLiteral("measurements"), measureText(s));
        r.insert(QStringLiteral("device"), deviceText(s));
        r.insert(QStringLiteral("note"), s.note);
        l << r;
    }
    return l;
}

QVariantMap AppController::sessionStats(const QString &fromIso, const QString &toIso, const QString &moduleId) const
{
    int n = 0, sec = 0, complete = 0;
    QSet<QDate> days;
    if (loggedIn() && m_userId) {
        for (const SessionRecord &s : Database::instance().sessions(m_userId, dayStart(fromIso), dayStart(toIso).addDays(1))) {
            if (!moduleId.isEmpty() && s.moduleId != moduleId) continue;
            ++n;
            sec += s.actualSec;
            if (s.completed) ++complete;
            days.insert(s.started.date());
        }
    }
    return { { QStringLiteral("count"), n }, { QStringLiteral("minutes"), sec / 60 }, { QStringLiteral("complete"), complete },
             { QStringLiteral("days"), int(days.size()) },
             { QStringLiteral("text"), tr("%1 Übungseinheiten an %2 Tagen · zusammen %3 Minuten").arg(n).arg(days.size()).arg(sec / 60) } };
}

QVariantMap AppController::session(qint64 id) const
{
    QVariantMap r;
    if (!loggedIn()) return r;
    for (const SessionRecord &s : Database::instance().sessions(m_userId)) {
        if (s.id != id) continue;
        const ModuleDef *m = BreathModules::find(s.moduleId);
        r.insert(QStringLiteral("id"), s.id);
        r.insert(QStringLiteral("module"), BreathModules::displayName(s.moduleId));
        r.insert(QStringLiteral("when"), QLocale().toString(s.started, QStringLiteral("ddd d.M.yyyy, HH:mm")));
        r.insert(QStringLiteral("summary"), tr("%1 · %2 · %3").arg(s.pattern, formatDuration(s.actualSec),
                                                                   s.completed ? tr("vollständig") : tr("vorzeitig beendet")));
        r.insert(QStringLiteral("eventName"), s.eventName);
        r.insert(QStringLiteral("note"), s.note);
        r.insert(QStringLiteral("spo2Before"), numText(s.spo2Before));
        r.insert(QStringLiteral("spo2After"), numText(s.spo2After));
        r.insert(QStringLiteral("pfBefore"), numText(s.pfBefore));
        r.insert(QStringLiteral("pfAfter"), numText(s.pfAfter));
        r.insert(QStringLiteral("pulseBefore"), numText(s.pulseBefore));
        r.insert(QStringLiteral("pulseAfter"), numText(s.pulseAfter));
        r.insert(QStringLiteral("feelingBefore"), s.feelingBefore);
        r.insert(QStringLiteral("feelingAfter"), s.feelingAfter);
        r.insert(QStringLiteral("deviceTraining"), m && m->deviceTraining);
        r.insert(QStringLiteral("deviceBalls"), s.deviceBalls);
        r.insert(QStringLiteral("deviceHold"), numText(s.deviceHoldSec));
        break;
    }
    return r;
}

bool AppController::updateSession(const QVariantMap &m)
{
    if (!loggedIn()) return false;
    const qint64 id = m.value(QStringLiteral("id")).toLongLong();
    for (SessionRecord s : Database::instance().sessions(m_userId)) {
        if (s.id != id) continue;
        s.eventName = m.value(QStringLiteral("eventName")).toString().trimmed();
        s.note = m.value(QStringLiteral("note")).toString().trimmed();
        s.spo2Before = numValue(m.value(QStringLiteral("spo2Before")));
        s.spo2After = numValue(m.value(QStringLiteral("spo2After")));
        s.pfBefore = numValue(m.value(QStringLiteral("pfBefore")));
        s.pfAfter = numValue(m.value(QStringLiteral("pfAfter")));
        s.pulseBefore = numValue(m.value(QStringLiteral("pulseBefore")));
        s.pulseAfter = numValue(m.value(QStringLiteral("pulseAfter")));
        s.feelingBefore = m.value(QStringLiteral("feelingBefore"), -1).toInt();
        s.feelingAfter = m.value(QStringLiteral("feelingAfter"), -1).toInt();
        if (s.feelingBefore < 1 || s.feelingBefore > 10) s.feelingBefore = kNoValue;
        if (s.feelingAfter < 1 || s.feelingAfter > 10) s.feelingAfter = kNoValue;
        if (m.contains(QStringLiteral("deviceBalls"))) {
            const int b = m.value(QStringLiteral("deviceBalls")).toInt();
            s.deviceBalls = b >= 0 && b <= 3 ? b : kNoValue;
            s.deviceHoldSec = numValue(m.value(QStringLiteral("deviceHold")));
        }
        QString err;
        if (!Database::instance().saveSession(s, &err)) { setError(err); return false; }
        bump();
        return true;
    }
    setError(tr("Der Eintrag wurde nicht gefunden."));
    return false;
}

bool AppController::deleteSession(qint64 id)
{
    QString err;
    if (!Database::instance().deleteSession(id, &err)) { setError(err); return false; }
    bump();
    return true;
}

bool AppController::sharePdf(const QString &fromIso, const QString &toIso, const QString &moduleId, bool chart, bool notes)
{
    if (!loggedIn() || !m_userId) return false;
    const auto u = Database::instance().user(m_userId);
    if (!u) return false;
    ChartSpec spec;
    spec.from = QDate::fromString(fromIso, Qt::ISODate);
    spec.to = QDate::fromString(toIso, Qt::ISODate);
    for (const SessionRecord &s : Database::instance().sessions(m_userId, dayStart(fromIso), dayStart(toIso).addDays(1)))
        if (moduleId.isEmpty() || s.moduleId == moduleId) spec.sessions << s;
    if (spec.sessions.isEmpty()) { setError(tr("Im gewählten Zeitraum gibt es keine Einträge.")); return false; }
    ReportDoc::Options o;
    o.chart = chart;
    o.notes = notes;
    QString name = u->name;
    name.replace(QLatin1Char(' '), QLatin1Char('_'));
    const QString f = workDir(QStringLiteral("share")) + QStringLiteral("/ViviaBreathe_%1_%2.pdf").arg(name, toIso);
    QFile::remove(f);
    if (!ReportDoc::writePdf(f, *u, spec, o)) { setError(tr("Die PDF-Datei konnte nicht geschrieben werden.")); return false; }
    return shareFiles({ f }, QStringLiteral("application/pdf"), tr("ViviaBreathe – Übungstagebuch"));
}

bool AppController::shareCsv(const QString &fromIso, const QString &toIso, const QString &moduleId)
{
    if (!loggedIn() || !m_userId) return false;
    QList<SessionRecord> list;
    for (const SessionRecord &s : Database::instance().sessions(m_userId, dayStart(fromIso), dayStart(toIso).addDays(1)))
        if (moduleId.isEmpty() || s.moduleId == moduleId) list << s;
    if (list.isEmpty()) { setError(tr("Im gewählten Zeitraum gibt es keine Einträge.")); return false; }
    const QString f = workDir(QStringLiteral("share")) + QStringLiteral("/ViviaBreathe_%1.csv").arg(toIso);
    QFile file(f);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) { setError(file.errorString()); return false; }
    file.write(ReportDoc::csv(list));
    file.close();
    return shareFiles({ f }, QStringLiteral("text/csv"), tr("ViviaBreathe – Übungstagebuch (CSV)"));
}

QString AppController::todayIso(int addDays) const { return QDate::currentDate().addDays(addDays).toString(Qt::ISODate); }

QString AppController::dateText(const QString &iso) const
{
    const QDate d = QDate::fromString(iso, Qt::ISODate);
    return d.isValid() ? QLocale().toString(d, QStringLiteral("d. MMM yyyy")) : QString();
}

QVariantMap AppController::overview() const
{
    QVariantMap r;
    if (!loggedIn() || !m_userId) return r;
    Database &db = Database::instance();
    const QDate today = QDate::currentDate();
    const QList<SessionRecord> list = db.sessions(m_userId, QDateTime(today.addDays(-400), QTime(0, 0)));
    int todayN = 0, todaySec = 0, weekSec = 0;
    QSet<QDate> days;
    for (const SessionRecord &s : list) {
        const QDate d = s.started.date();
        days.insert(d);
        if (d == today) { ++todayN; todaySec += s.actualSec; }
        if (d > today.addDays(-7)) weekSec += s.actualSec;
    }
    int streak = 0;
    QDate d = days.contains(today) ? today : today.addDays(-1);
    while (days.contains(d)) { ++streak; d = d.addDays(-1); }
    r.insert(QStringLiteral("todayCount"), todayN);
    r.insert(QStringLiteral("todayMinutes"), todaySec / 60);
    r.insert(QStringLiteral("weekMinutes"), weekSec / 60);
    r.insert(QStringLiteral("streak"), streak);
    // nächste Erinnerung dieses Benutzers
    QDateTime next;
    QString what;
    for (const BreathEvent &e : db.events(m_userId)) {
        if (!e.enabled) continue;
        const QDateTime t = Schedule::next(e.time, e.weekdays);
        if (t.isValid() && (!next.isValid() || t < next)) { next = t; what = e.name; }
    }
    for (const Alarm &a : db.alarms(m_userId)) {
        if (!a.enabled) continue;
        const QDateTime t = Schedule::next(a.time, a.weekdays);
        if (t.isValid() && (!next.isValid() || t < next)) { next = t; what = tr("Wecker"); }
    }
    r.insert(QStringLiteral("next"), next.isValid() ? tr("%1 %2").arg(what, Schedule::relative(next)) : QString());
    return r;
}
