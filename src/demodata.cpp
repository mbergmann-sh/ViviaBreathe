// ViviaBreathe – erfundene Beispieldaten (Demo-Modus, Bildschirmfotos, Tests der mobilen App)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "demodata.h"
#include "database.h"

#include <QObject>
#include <QRandomGenerator>

namespace DemoData {

void create()
{
    Database &db = Database::instance();
    const qint64 erika = db.addUser(QObject::tr("Erika Mustermann"));
    const qint64 max = db.addUser(QObject::tr("Max Mustermann"));
    auto *rng = QRandomGenerator::global();
    const QDate today = QDate::currentDate();
    for (int d = 59; d >= 0; --d) {
        const QDate day = today.addDays(-d);
        // abends 4-6 bzw. 4-7-8, gelegentlich morgens Box-Atmen
        if (rng->bounded(10) < 8) {
            SessionRecord s;
            s.userId = erika;
            const bool sleep = rng->bounded(2) == 0;
            s.moduleId = sleep ? QStringLiteral("four_six") : QStringLiteral("four_seven_eight");
            s.eventName = sleep ? QObject::tr("Einschlafübung") : QObject::tr("Entspannung");
            s.pattern = sleep ? QStringLiteral("4-6") : QStringLiteral("4-7-8");
            s.started = QDateTime(day, QTime(21, 10 + rng->bounded(40)));
            s.plannedSec = sleep ? 600 : 300;
            s.actualSec = s.plannedSec + rng->bounded(15);
            s.cycles = s.actualSec / (sleep ? 10 : 19);
            s.completed = rng->bounded(12) != 0;
            if (!s.completed) s.actualSec /= 2;
            if (d % 3 == 0) {     // Trainingsmodus
                s.spo2Before = 93 + rng->bounded(3);
                s.spo2After = qMin(99, s.spo2Before + rng->bounded(3));
                s.pfBefore = 330 + rng->bounded(40) + (59 - d);
                s.pfAfter = s.pfBefore + 10 + rng->bounded(30);
                s.pulseBefore = 74 + rng->bounded(12);
                s.pulseAfter = s.pulseBefore - 4 - rng->bounded(6);
                s.feelingBefore = 4 + rng->bounded(3);
                s.feelingAfter = s.feelingBefore + 1 + rng->bounded(3);
            }
            if (d == 2) s.note = QObject::tr("Sehr ruhig geworden, schnell eingeschlafen.");
            db.saveSession(s);
        }
        if (rng->bounded(10) < 4) {
            SessionRecord s;
            s.userId = erika;
            s.moduleId = QStringLiteral("box");
            s.eventName = QObject::tr("Stressbewältigung");
            s.pattern = QStringLiteral("4-4-4-4");
            s.started = QDateTime(day, QTime(12 + rng->bounded(5), rng->bounded(60)));
            s.plannedSec = 300;
            s.actualSec = 300 + rng->bounded(12);
            s.cycles = s.actualSec / 16;
            s.completed = true;
            db.saveSession(s);
        }
        if (d % 4 == 1) {     // Lippenbremse im Trainingsmodus
            SessionRecord s;
            s.userId = erika;
            s.moduleId = QStringLiteral("pursed_lip");
            s.eventName = QObject::tr("Atemtraining");
            s.pattern = QStringLiteral("2-4");
            s.started = QDateTime(day, QTime(10, rng->bounded(60)));
            s.plannedSec = 300;
            s.actualSec = 300 + rng->bounded(6);
            s.cycles = 50;
            s.completed = true;
            s.spo2Before = 92 + rng->bounded(3);
            s.spo2After = qMin(99, s.spo2Before + 1 + rng->bounded(2));
            s.pfBefore = 320 + rng->bounded(40);
            s.pfAfter = s.pfBefore + rng->bounded(25);
            db.saveSession(s);
        }
        if (d % 2 == 0) {     // TriBall-Training, langsam gesteigert
            SessionRecord s;
            s.userId = erika;
            s.moduleId = QStringLiteral("triball");
            s.eventName = QObject::tr("Atemtrainer");
            s.pattern = QStringLiteral("3-3-3-4-10");
            s.started = QDateTime(day, QTime(16, rng->bounded(60)));
            s.plannedSec = s.actualSec = 230;
            s.cycles = 10;
            s.completed = true;
            s.deviceBalls = d > 40 ? 1 : d > 15 ? 2 : (rng->bounded(3) ? 2 : 3);
            s.deviceHoldSec = 2 + (59 - d) / 12 + rng->bounded(2);
            db.saveSession(s);
        }
        if (rng->bounded(10) < 3) {
            SessionRecord s;
            s.userId = max;
            s.moduleId = QStringLiteral("box");
            s.eventName = QObject::tr("Konzentration");
            s.pattern = QStringLiteral("4-4-4-4");
            s.started = QDateTime(day, QTime(8, rng->bounded(60)));
            s.plannedSec = s.actualSec = 300;
            s.cycles = 19;
            s.completed = true;
            db.saveSession(s);
        }
    }
    BreathEvent e1{ 0, erika, QObject::tr("Einschlafübung"), QStringLiteral("four_six"), QTime(21, 30), Weekdays::kAll, true, true,
                    QObject::tr("Im Bett, Licht aus") };
    BreathEvent e2{ 0, erika, QObject::tr("Stressbewältigung"), QStringLiteral("box"), QTime(13, 0), Weekdays::kWorkdays, true, true, {} };
    BreathEvent e3{ 0, max, QObject::tr("Konzentration"), QStringLiteral("box"), QTime(8, 0), Weekdays::kWorkdays, true, true, {} };
    db.saveEvent(e1);
    db.saveEvent(e2);
    db.saveEvent(e3);
    Alarm a;
    a.userId = erika;
    a.label = QObject::tr("Arbeitstag");
    a.time = QTime(6, 45);
    a.moduleId = QStringLiteral("box");
    db.saveAlarm(a);
    SleepPlan sp;
    sp.userId = erika;
    sp.label = QObject::tr("Wochentags");
    sp.scheduled = true;
    sp.time = QTime(22, 30);
    sp.weekdays = Weekdays::kWorkdays;
    sp.minutes = 30;
    sp.moduleId = QStringLiteral("four_six");
    db.saveSleepPlan(sp);
    SleepPlan we;
    we.userId = erika;
    we.label = QObject::tr("Wochenende – länger");
    we.minutes = 60;
    we.fadeOutMin = 10;
    db.saveSleepPlan(we);
    db.setDeviceSetting(QStringLiteral("ui/lastUser"), QString::number(erika));
}

} // namespace DemoData
