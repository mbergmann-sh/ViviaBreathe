// ViviaBreathe mobil – automatischer Rundgang für Bildschirmfotos und Prüfungen am Desktop
// (qmake CONFIG+=autopilot; nicht im Android-Paket)
//
//   VB_AUTOPILOT_OUT   Ordner für die Bildschirmfotos (Vorgabe /tmp/vb-autopilot)
//   VB_AUTOPILOT_LANG  de | en
//
// Rundgang in drei Größen (Smartphone 390×844, Tablet quer 1280×800, Tablet hoch 800×1280) und im
// dunklen Design. Je Seite: Bildschirmfoto und Prüfung auf Elemente, die über den rechten Rand
// hinausragen (Smartphone: alles muss ohne seitliches Wischen erreichbar sein).
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "appcontroller.h"
#include "playback.h"
#include "sessionrunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSettings>
#include <QTimer>
#include <cstdio>
#include <functional>

namespace {
QQuickWindow *g_win = nullptr;
QString g_out;
QString g_prefix;
int g_problems = 0;

void wait(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

// Funktionen von Main.qml aufrufen (immer mit der vollen Zahl an Parametern)
QVariant call(const char *fn)
{
    QVariant r;
    const QByteArray f(fn);
    if (f == "openPrep") QMetaObject::invokeMethod(g_win, fn, Q_RETURN_ARG(QVariant, r), Q_ARG(QVariant, QString()), Q_ARG(QVariant, QString()));
    else if (f == "backHome" || f == "openGuide") QMetaObject::invokeMethod(g_win, fn, Q_RETURN_ARG(QVariant, r), Q_ARG(QVariant, QString()));
    else QMetaObject::invokeMethod(g_win, fn, Q_RETURN_ARG(QVariant, r));
    return r;
}
QVariant call(const char *fn, const QVariant &a)
{
    QVariant r;
    if (QByteArray(fn) == "openPrep")
        QMetaObject::invokeMethod(g_win, fn, Q_RETURN_ARG(QVariant, r), Q_ARG(QVariant, a), Q_ARG(QVariant, QString()));
    else
        QMetaObject::invokeMethod(g_win, fn, Q_RETURN_ARG(QVariant, r), Q_ARG(QVariant, a));
    return r;
}

QQuickItem *stackTop()
{
    auto *stack = g_win->findChild<QQuickItem *>(QStringLiteral("stack"));
    return stack ? stack->property("currentItem").value<QQuickItem *>() : nullptr;
}

// Element nach objectName in der Item-Hierarchie suchen (sichtbare zuerst)
QQuickItem *findIn(QQuickItem *root, const std::function<bool(QQuickItem *)> &match)
{
    QList<QQuickItem *> todo{ root };
    QQuickItem *hidden = nullptr;
    while (!todo.isEmpty()) {
        QQuickItem *it = todo.takeFirst();
        if (match(it)) {
            if (it->isVisible()) return it;
            if (!hidden) hidden = it;
        }
        todo << it->childItems();
    }
    return hidden;
}

QQuickItem *find(const QString &name)
{
    return findIn(g_win->contentItem(), [&](QQuickItem *it) { return it->objectName() == name; });
}

QQuickItem *flickableIn(QQuickItem *root)
{
    return root ? findIn(root, [](QQuickItem *it) { return it->inherits("QQuickFlickable"); }) : nullptr;
}

bool effectivelyVisible(QQuickItem *it)
{
    for (QQuickItem *p = it; p; p = p->parentItem())
        if (!p->isVisible() || p->opacity() <= 0.01) return false;
    return true;
}

// Elemente, die rechts über das Fenster hinausragen (nur sichtbare, nicht in einem Popup)
void checkOverflow(const QString &shot)
{
    const qreal W = g_win->width();
    QList<QQuickItem *> todo{ g_win->contentItem() };
    int found = 0;
    while (!todo.isEmpty()) {
        QQuickItem *it = todo.takeFirst();
        for (QQuickItem *c : it->childItems()) todo << c;
        if (it == g_win->contentItem() || !effectivelyVisible(it) || it->width() < 1) continue;
        const QRectF r = it->mapRectToScene(QRectF(0, 0, it->width(), it->height()));
        // Grenze: Fenster bzw. der nächste beschneidende Bereich (was dort hinausragt, ist unsichtbar)
        qreal limit = W;
        for (QQuickItem *p = it->parentItem(); p; p = p->parentItem()) {
            if (!p->clip()) continue;
            const bool hScroll = p->inherits("QQuickFlickable")
                                 && p->property("contentWidth").toDouble() > p->width() + 1;
            if (!hScroll) limit = qMin(limit, p->mapRectToScene(QRectF(0, 0, p->width(), p->height())).right());
            break;
        }
        if (r.right() > limit + 1.5 && r.left() < limit) {
            if (++found <= 8)
                std::printf("  ZU BREIT [%s] %s '%s' rechts bei %.0f (Grenze %.0f)\n", qPrintable(shot),
                            it->metaObject()->className(), qPrintable(it->objectName()), r.right(), limit);
        }
    }
    g_problems += found;
}

void shot(const QString &name)
{
    wait(350);
    const QString file = QStringLiteral("%1/%2%3.png").arg(g_out, g_prefix, name);
    g_win->grabWindow().save(file);
    checkOverflow(g_prefix + name);
    std::printf("Bild: %s\n", qPrintable(file));
}

void setTab(int i)
{
    if (QQuickItem *h = find(QStringLiteral("home"))) h->setProperty("tab", i);
    wait(150);
}

void back() { call("goBack"); wait(250); }

void tour(AppController &app)
{
    call("goStart");
    wait(300);
    setTab(0);
    shot(QStringLiteral("01_uebungen"));
    // Übersicht nach unten rollen: Atemwege
    if (QQuickItem *flick = flickableIn(find(QStringLiteral("exercisesView")))) {
        flick->setProperty("contentY", qMax(0.0, flick->property("contentHeight").toDouble() * 0.55));
        shot(QStringLiteral("02_uebungen_atemwege"));
        flick->setProperty("contentY", 0);
    }
    call("openPrep", QStringLiteral("four_seven_eight"));
    shot(QStringLiteral("03_vorbereitung"));
    back();
    call("openPrep", QStringLiteral("triball"));
    shot(QStringLiteral("04_vorbereitung_triball"));
    back();
    call("openGuide", QStringLiteral("triball"));
    shot(QStringLiteral("05_anleitung_triball"));
    back();

    // Übung: Box-Atmen starten, kurz laufen lassen
    auto *runner = qobject_cast<SessionRunner *>(app.runner());
    QVariantMap s = app.moduleSettings(QStringLiteral("box"));
    s.insert(QStringLiteral("signal"), false);
    s.insert(QStringLiteral("doneGong"), false);
    s.insert(QStringLiteral("trainingMode"), false);
    s.insert(QStringLiteral("totalSec"), 60);
    call("openPrep", QStringLiteral("box"));
    wait(200);
    if (app.startSession(QStringLiteral("box"), s)) call("beginSession");
    wait(5200);            // Vorlauf 3 s, dann mitten im Einatmen
    shot(QStringLiteral("06_uebung_box"));
    runner->stop();
    wait(900);
    shot(QStringLiteral("07_zusammenfassung"));
    if (qEnvironmentVariableIsSet("VB_AUTOPILOT_DEBUG")) {
        QQuickItem *pg = stackTop();
        QList<QQuickItem *> todo{ pg };
        while (!todo.isEmpty()) {
            QQuickItem *it = todo.takeFirst();
            if (it->width() > 391) std::printf("DBG %s '%s' w=%.0f iw=%.0f parent=%s\n", it->metaObject()->className(), qPrintable(it->objectName()),
                                               it->width(), it->implicitWidth(), it->parentItem() ? it->parentItem()->metaObject()->className() : "-");
            todo << it->childItems();
        }
    }
    call("backHome");
    runner->reset();

    // Kreis (4-7-8) mit Trainingsmodus: Messen vorher
    QVariantMap t = app.moduleSettings(QStringLiteral("four_seven_eight"));
    t.insert(QStringLiteral("signal"), false);
    t.insert(QStringLiteral("doneGong"), false);
    t.insert(QStringLiteral("trainingMode"), true);
    call("openPrep", QStringLiteral("four_seven_eight"));
    wait(150);
    if (app.startSession(QStringLiteral("four_seven_eight"), t)) call("beginSession");
    shot(QStringLiteral("08_messen_vorher"));
    if (QQuickItem *p = stackTop()) QMetaObject::invokeMethod(p, "finish", Q_ARG(QVariant, true));
    wait(5800);
    shot(QStringLiteral("09_uebung_478"));
    runner->abort();
    call("backHome");
    t.insert(QStringLiteral("trainingMode"), false);
    app.saveModuleSettings(QStringLiteral("four_seven_eight"), t);

    // TriBall
    QVariantMap tb = app.moduleSettings(QStringLiteral("triball"));
    tb.insert(QStringLiteral("signal"), false);
    tb.insert(QStringLiteral("doneGong"), false);
    tb.insert(QStringLiteral("deviceTarget"), 2);
    call("openPrep", QStringLiteral("triball"));
    wait(150);
    if (app.startSession(QStringLiteral("triball"), tb)) call("beginSession");
    wait(7200);
    shot(QStringLiteral("10_uebung_triball"));
    runner->stop();
    wait(900);
    shot(QStringLiteral("11_zusammenfassung_triball"));
    call("backHome");
    runner->reset();

    // Atemnot-Hilfe
    call("quickHelp");
    wait(3500);
    shot(QStringLiteral("12_atemnot_hilfe"));
    runner->abort();
    call("backHome");

    // Verlauf: Liste und Grafik
    setTab(1);
    shot(QStringLiteral("13_verlauf_liste"));
    if (QQuickItem *v = find(QStringLiteral("historyView"))) {
        v->setProperty("chart", true);
        shot(QStringLiteral("14_verlauf_grafik"));
        v->setProperty("chart", false);
    }
    setTab(2);
    shot(QStringLiteral("15_erinnerungen"));
    setTab(3);
    shot(QStringLiteral("16_wecker_einschlafen"));
    setTab(4);
    shot(QStringLiteral("17_einstellungen"));
    setTab(0);

    // Bearbeiten-Seiten
    auto open = [](const QString &file, const QVariantMap &props, const QString &name) {
        QVariant r;
        QMetaObject::invokeMethod(g_win, "openPageFile", Q_RETURN_ARG(QVariant, r), Q_ARG(QVariant, file), Q_ARG(QVariant, props));
        shot(name);
        back();
    };
    const QVariantList sessions = app.sessions(app.todayIso(-6), app.todayIso(0), QStringLiteral("triball"));
    if (!sessions.isEmpty())
        open(QStringLiteral("SessionEditPage.qml"), { { QStringLiteral("sessionId"), sessions.first().toMap().value(QStringLiteral("id")) } },
             QStringLiteral("18_eintrag_aendern"));
    const QVariantList ev = app.events();
    if (!ev.isEmpty())
        open(QStringLiteral("EventEditPage.qml"), { { QStringLiteral("eventId"), ev.first().toMap().value(QStringLiteral("id")) } },
             QStringLiteral("19_ereignis"));
    const QVariantList al = app.alarms();
    if (!al.isEmpty())
        open(QStringLiteral("AlarmEditPage.qml"), { { QStringLiteral("alarmId"), al.first().toMap().value(QStringLiteral("id")) } },
             QStringLiteral("20_wecker"));
    const QVariantList sp = app.sleepPlans();
    if (!sp.isEmpty()) {
        open(QStringLiteral("SleepEditPage.qml"), { { QStringLiteral("planId"), sp.first().toMap().value(QStringLiteral("id")) } },
             QStringLiteral("21_einschlafen"));
        open(QStringLiteral("SleepRunPage.qml"), { { QStringLiteral("planId"), sp.first().toMap().value(QStringLiteral("id")) } },
             QStringLiteral("22_einschlafen_start"));
    }
    open(QStringLiteral("UsersPage.qml"), {}, QStringLiteral("23_benutzer"));
    open(QStringLiteral("SyncPage.qml"), {}, QStringLiteral("24_abgleich"));
    open(QStringLiteral("PasswordPage.qml"), {}, QStringLiteral("25_passwort"));
    open(QStringLiteral("AboutPage.qml"), {}, QStringLiteral("26_ueber"));
    // Wecker klingelt
    QVariantMap c{ { QStringLiteral("files"), QStringList() }, { QStringLiteral("volume"), 1 }, { QStringLiteral("title"), QStringLiteral("Arbeitstag") },
                   { QStringLiteral("ref"), al.isEmpty() ? 0 : al.first().toMap().value(QStringLiteral("id")) } };
    if (auto *pb = qobject_cast<Playback *>(app.playback())) {
        pb->start(QStringLiteral("alarm"), c);
        shot(QStringLiteral("27_wecker_klingelt"));
        pb->stop();
        wait(200);
        call("backHome");
        QVariantMap sc = c;
        sc.insert(QStringLiteral("title"), QStringLiteral("Wochentags"));
        sc.insert(QStringLiteral("durationSec"), 1800);
        pb->start(QStringLiteral("sleep"), sc);
        setTab(3);
        shot(QStringLiteral("28_einschlafmusik_laeuft"));
        pb->stop();
        setTab(0);
    }
}
} // namespace

int runAutopilot(QQmlApplicationEngine &engine, AppController &app)
{
    g_win = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    if (!g_win) return 2;
    g_out = qEnvironmentVariable("VB_AUTOPILOT_OUT", QStringLiteral("/tmp/vb-autopilot"));
    QDir().mkpath(g_out);
    const QString lang = qEnvironmentVariable("VB_AUTOPILOT_LANG", QStringLiteral("de"));

    // Erste Schritte (vor den Beispieldaten)
    g_prefix = lang + QStringLiteral("_phone_");
    g_win->resize(390, 844);
    wait(300);
    shot(QStringLiteral("00a_sprache"));
    QSettings().setValue(QStringLiteral("mobile/languageChosen"), true);
    call("goStart");
    shot(QStringLiteral("00b_hinweis"));
    app.acceptDisclaimer();
    call("goStart");
    shot(QStringLiteral("00c_einrichtung"));

    if (!app.startDemo()) { std::printf("Demo-Datenbank: %s\n", qPrintable(app.lastError())); return 3; }
    app.setThemeMode(1);
    struct Size { const char *name; int w, h; };
    const Size sizes[] = { { "phone_", 390, 844 }, { "small_", 360, 740 }, { "tablet_", 1280, 800 }, { "tabletp_", 800, 1280 } };
    for (const Size &sz : sizes) {
        g_prefix = lang + QLatin1Char('_') + QLatin1String(sz.name);
        g_win->resize(sz.w, sz.h);
        wait(400);
        tour(app);
    }
    // dunkles Design (Smartphone)
    app.setThemeMode(2);
    g_prefix = lang + QStringLiteral("_phone_dark_");
    g_win->resize(390, 844);
    wait(400);
    tour(app);
    app.setThemeMode(0);

    // Bericht als PDF und CSV (Teilen am Desktop: Datei wird nur erzeugt)
    if (app.sharePdf(app.todayIso(-29), app.todayIso(0), QString(), true, true))
        std::printf("PDF: %s\n", qPrintable(app.lastShared().value(0)));
    else
        { std::printf("PDF FEHLER: %s\n", qPrintable(app.lastError())); ++g_problems; }
    if (app.shareCsv(app.todayIso(-29), app.todayIso(0), QString()))
        std::printf("CSV: %s\n", qPrintable(app.lastShared().value(0)));
    else
        { std::printf("CSV FEHLER: %s\n", qPrintable(app.lastError())); ++g_problems; }

    std::printf("\n%s – %d Element(e) zu breit\n", g_problems ? "PROBLEME" : "OK", g_problems);
    return g_problems ? 1 : 0;
}
