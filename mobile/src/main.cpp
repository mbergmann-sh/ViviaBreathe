// ViviaBreathe mobil – Startpunkt
//
//   ViviaBreathe                         normaler Start
//   (Desktop-Test) --demo                mit Beispieldaten in einer eigenen Datenbank
//   (Desktop-Test) --phone / --tablet    Fenster in Telefon- bzw. Tablet-Größe
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "appcontroller.h"
#include "breathitem.h"
#include "chartitem.h"
#include "imageproviders.h"
#include "systemui.h"

#include "database.h"
#include "i18n.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>

#ifdef VB_AUTOPILOT
int runAutopilot(QQmlApplicationEngine &engine, AppController &app);
#endif

int main(int argc, char *argv[])
{
#ifdef VB_AUTOPILOT
    QStandardPaths::setTestModeEnabled(true);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
#endif
    // QApplication (statt QGuiApplication): der gemeinsame Kern verwendet einzelne Widgets-Klassen
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("mbergmann-sh"));
    QApplication::setApplicationName(QStringLiteral("ViviaBreathe"));
    QApplication::setApplicationDisplayName(QStringLiteral("ViviaBreathe"));
    QApplication::setApplicationVersion(QStringLiteral(VIVIABREATHE_VERSION));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/viviabreathe.png")));
    const QStringList args = QCoreApplication::arguments();
    const bool demo = args.contains(QStringLiteral("--demo"));
#ifdef VB_AUTOPILOT
    I18n::setLanguage(qEnvironmentVariable("VB_AUTOPILOT_LANG", QStringLiteral("de")));
#endif
    I18n::install(app);

    for (const char *f : { ":/fonts/AtkinsonHyperlegible_400Regular.ttf", ":/fonts/AtkinsonHyperlegible_700Bold.ttf" })
        QFontDatabase::addApplicationFont(QString::fromLatin1(f));
    QFont font(QStringLiteral("Atkinson Hyperlegible"));
    font.setPixelSize(17);
    QGuiApplication::setFont(font);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    qmlRegisterType<BreathItem>("ViviaBreathe", 1, 0, "BreathView");
    qmlRegisterType<ChartItem>("ViviaBreathe", 1, 0, "Chart");

    AppController controller(demo);
    if (demo) controller.startDemo();
    SystemUi systemUi;
    QQmlApplicationEngine engine;
    controller.setEngine(&engine);
    engine.addImageProvider(QStringLiteral("icon"), new IconProvider);
    engine.addImageProvider(QStringLiteral("illus"), new IllustrationProvider);
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("systemUi"), &systemUi);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    if (window && !controller.isAndroid()) {
        if (args.contains(QStringLiteral("--phone"))) window->resize(390, 844);
        else if (args.contains(QStringLiteral("--tablet"))) window->resize(1280, 800);
    }
    systemUi.attach(window);
    systemUi.setLightBars(!controller.dark());
    QObject::connect(&controller, &AppController::themeChanged, &systemUi, [&] { systemUi.setLightBars(!controller.dark()); });
#ifdef VB_AUTOPILOT
    QTimer::singleShot(800, &app, [&] { QCoreApplication::exit(runAutopilot(engine, controller)); });
#endif

    // Hintergrund/Vordergrund: Erinnerungen planen, abgleichen, nach längerer Pause sperren
    QElapsedTimer away;
    QObject::connect(&app, &QGuiApplication::applicationStateChanged, &controller, [&](Qt::ApplicationState s) {
        if (s == Qt::ApplicationSuspended || s == Qt::ApplicationHidden || s == Qt::ApplicationInactive) {
            if (!away.isValid()) {
                away.start();
                controller.appHidden();
            }
        } else if (s == Qt::ApplicationActive) {
            const int lockMin = controller.autoLockMinutes();
            if (away.isValid() && controller.loggedIn() && lockMin > 0 && away.elapsed() > qint64(lockMin) * 60000
                && !controller.demo()) {
                controller.lock();
                emit controller.locked();
            }
            away.invalidate();
            controller.appActive();
        }
    });

    const int rc = app.exec();
    Database::instance().close();
    return rc;
}
