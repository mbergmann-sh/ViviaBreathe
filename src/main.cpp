// ViviaBreathe – Programmstart
//
//   ViviaBreathe                 normaler Start (Master-Passwort-Abfrage)
//   ViviaBreathe --demo          Demo mit erfundenen Daten in einer eigenen, temporären Datenbank
//                               (Master-Passwort "demo"), ohne die eigenen Daten zu berühren
//   ViviaBreathe --screenshots <ordner>   (mit --demo) Bildschirmfotos für Handbuch/README erzeugen
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "alarmpage.h"
#include "audio.h"
#include "breathengine.h"
#include "breathmodules.h"
#include "database.h"
#include "demodata.h"
#include "dbsync.h"
#include "i18n.h"
#include "legacyimport.h"
#include "icons.h"
#include "logindialog.h"
#include "mainwindow.h"
#include "screenfit.h"
#include "sessiondialog.h"
#include "themes.h"
#include "windowstate.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QGroupBox>
#include <QCommandLineParser>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSettings>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTimer>

#include "qtcompat.h"

namespace {

bool openDatabase(QString path, bool demo)
{
    Database &db = Database::instance();
    if (demo) {
        QFile::remove(path);
        if (db.open(path, QStringLiteral("demo")) != Database::OpenResult::Ok) return false;
        DemoData::create();
        return true;
    }
    QString error;
    for (;;) {
        LoginDialog dlg(path);
        if (!error.isEmpty()) dlg.setError(error);
        if (dlg.exec() != QDialog::Accepted) return false;
        path = dlg.databasePath();
        const bool isNew = dlg.isNewDatabase();
        QString err;
        const auto r = db.open(path, dlg.password(), &err);
        if (r == Database::OpenResult::Ok) {
            if (isNew && db.users().isEmpty()) db.addUser(dlg.firstUserName());
            DbSync::adoptInitialState(path);
            return true;
        }
        error = r == Database::OpenResult::WrongPassword ? QObject::tr("Das Master-Passwort ist falsch.") : err;
    }
}

// Bildschirmfotos für Dokumentation (nur mit --demo)
void takeScreenshots(MainWindow *w, const QString &dir)
{
    QDir().mkpath(dir);
    auto shot = [dir](QWidget *wid, const QString &name) {
        QApplication::processEvents();
        wid->grab().save(dir + QLatin1Char('/') + name + QStringLiteral(".png"));
    };
    for (int i = 0; i < w->tabs()->count(); ++i) {
        w->tabs()->setCurrentIndex(i);
        shot(w, QStringLiteral("main_%1").arg(i));
    }
    w->tabs()->setCurrentIndex(0);
    QTimer::singleShot(300, [shot]() {   // Über-Fenster (modal) aufnehmen und schließen
        if (QWidget *about = QApplication::activeModalWidget()) {
            shot(about, QStringLiteral("about"));
            about->close();
        }
    });
    w->showAbout();
    if (!Database::instance().sleepPlans(w->currentUser()).isEmpty()) {
        SleepPlanDialog sd(Database::instance().sleepPlans(w->currentUser()).first(), w);
        sd.show();
        shot(&sd, QStringLiteral("sleep_plan"));
        sd.reject();
    }
    for (const ModuleDef &m : BreathModules::all()) {
        SessionDialog dlg(w->currentUser(), m.id, w);
        dlg.resize(720, 760);
        dlg.show();
        shot(&dlg, QStringLiteral("prep_%1").arg(m.id));
        // Übung starten (wie ein Klick auf „Übung starten“) und Momentaufnahmen machen
        auto *engine = dlg.findChild<BreathEngine *>();
        auto wait = [](int ms) {
            QEventLoop loop;
            QTimer::singleShot(ms, &loop, &QEventLoop::quit);
            loop.exec();
        };
        auto click = [&dlg](const QString &text) {
            for (QAbstractButton *b : dlg.findChildren<QAbstractButton *>())
                if (b->isVisible() && b->text() == text) { b->click(); return true; }
            return false;
        };
        const bool training = m.id == QLatin1String("four_seven_eight");
        if (training)
            for (QGroupBox *g : dlg.findChildren<QGroupBox *>())
                if (g->isCheckable() && g->title() == QCoreApplication::translate("SessionDialog", "Trainingsmodus: Messwerte vor und nach der Übung erfassen")) g->setChecked(true);
        click(QCoreApplication::translate("SessionDialog", "Übung starten"));
        if (training) {
            wait(200);
            shot(&dlg, QStringLiteral("measure_before"));
            click(QCoreApplication::translate("SessionDialog", "Weiter zur Übung"));
        }
        const int waits[] = { 5200, 3300 };
        for (int k = 0; k < 2; ++k) {
            wait(waits[k]);
            shot(&dlg, QStringLiteral("run_%1_%2").arg(m.id).arg(k));
        }
        engine->stop();
        wait(700);
        if (training) {
            shot(&dlg, QStringLiteral("measure_after"));
            click(QCoreApplication::translate("SessionDialog", "Weiter"));
            wait(200);
        }
        shot(&dlg, QStringLiteral("summary_%1").arg(m.id));
        click(QCoreApplication::translate("SessionDialog", "Im Tagebuch speichern"));
        dlg.hide();
    }
}

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("mbergmann-sh"));
    QApplication::setApplicationName(QStringLiteral("ViviaBreathe"));
    QApplication::setApplicationVersion(vbVersion());
    QApplication::setWindowIcon(Icons::app());
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("ViviaBreathe – geführte Atemübungen mit Tagebuch"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption demo(QStringLiteral("demo"), QStringLiteral("Demo mit erfundenen Daten (Master-Passwort: demo)"));
    QCommandLineOption shots(QStringLiteral("screenshots"), QStringLiteral("Bildschirmfotos erzeugen (mit --demo)"),
                             QStringLiteral("ordner"));
    QCommandLineOption dbOpt(QStringLiteral("database"), QStringLiteral("Datenbankdatei"), QStringLiteral("datei"));
    parser.addOptions({ demo, shots, dbOpt });
    parser.process(app);
    const bool isDemo = parser.isSet(demo);
    if (isDemo) QApplication::setApplicationName(QStringLiteral("ViviaBreathe-Demo"));   // eigene Einstellungen

    // Bis 0.4.0 hieß das Programm „VitaBreathe“: Einstellungen und Datenbank einmalig übernehmen
    if (!isDemo) LegacyImport::run();

    // Sprache (Vorgabe Deutsch, alternativ Englisch) – vor dem ersten Fenster
    I18n::install(app);

    Themes::init();
    ScreenFit::install(app);
    WindowState::install(app);   // nach ScreenFit: erst wiederherstellen, dann an den Bildschirm anpassen

    QString path = parser.value(dbOpt);
    if (path.isEmpty()) path = QSettings().value(QStringLiteral("db/path"), Database::defaultPath()).toString();
    if (isDemo) path = QDir::temp().filePath(QStringLiteral("viviabreathe-demo.sqlite"));
    if (!openDatabase(path, isDemo)) return 0;
    if (!isDemo) QSettings().setValue(QStringLiteral("db/path"), Database::instance().path());

    MainWindow w;
    w.show();

    if (isDemo && parser.isSet(shots)) {
        QTimer::singleShot(300, &w, [&w, &parser, &shots] {
            takeScreenshots(&w, parser.value(shots));
            w.setQuitting();
            w.close();
        });
        return app.exec();
    }

    if (!isDemo && QSettings().value(QStringLiteral("ui/startNotice"), true).toBool()) {
        QTimer::singleShot(400, &w, [&w] {
            QMessageBox box(&w);
            box.setWindowTitle(QObject::tr("Hinweis"));
            box.setIcon(QMessageBox::Information);
            box.setText(QObject::tr("<b>ViviaBreathe ist kein Medizinprodukt.</b><br/>Es ist ein persönliches Übungs-Tagebuch "
                                    "und ersetzt keine ärztliche Beratung. Brechen Sie eine Übung bei Schwindel, Unwohlsein "
                                    "oder Atemnot sofort ab."));
            auto *dont = new QCheckBox(QObject::tr("Nicht mehr anzeigen"));
            box.setCheckBox(dont);
            box.exec();
            if (dont->isChecked()) QSettings().setValue(QStringLiteral("ui/startNotice"), false);
        });
    }

    // Abgleich beim Start
    const DbSync::Config c = DbSync::config();
    if (!isDemo && c.enabled && c.autoSync && DbSync::isReady(c)) {
        QTimer::singleShot(1500, &DbSync::instance(), [] { DbSync::instance().sync(); });
        DbSync::instance().watchDatabase();
    } else if (!isDemo) {
        DbSync::instance().watchDatabase();
    }
    return app.exec();
}
