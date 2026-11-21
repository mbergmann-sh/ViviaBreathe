// ViviaBreathe mobil – Start, Anmeldung, Benutzer, Einstellungen und Abgleich (Teil von AppController)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "appcontroller.h"
#include "mobilereminders.h"
#include "platform.h"
#include "playback.h"
#include "sessionrunner.h"

#include "audio.h"
#include "database.h"
#include "dbsync.h"
#include "demodata.h"
#include "i18n.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleHints>
#include <QTimer>

namespace {
const char *kLangChosen = "mobile/languageChosen";
const char *kDisclaimer = "mobile/disclaimerAccepted";
const char *kTheme = "mobile/theme";
const char *kAutoLock = "mobile/autoLockMinutes";

QString passwordProblem(const QString &pw)
{
    if (pw.size() < 8) return AppController::tr("Das Master-Passwort muss mindestens 8 Zeichen lang sein.");
    bool lower = false, upper = false, digit = false, other = false;
    for (const QChar c : pw) {
        if (c.isLower()) lower = true;
        else if (c.isUpper()) upper = true;
        else if (c.isDigit()) digit = true;
        else other = true;
    }
    const int classes = int(lower) + int(upper) + int(digit) + int(other);
    if (classes < 2 && pw.size() < 14)
        return AppController::tr("Bitte Buchstaben mit Ziffern oder Sonderzeichen mischen – oder einen langen Satz (ab 14 Zeichen) verwenden.");
    return {};
}

QString demoPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/viviabreathe-demo.sqlite");
}
} // namespace

AppController::AppController(bool demo, QObject *parent) : QObject(parent), m_demo(demo)
{
    m_runner = new SessionRunner(this);
    m_playback = new Playback(this);
    m_reminders = new MobileReminders(this);
    connect(m_runner, &SessionRunner::saved, this, &AppController::bump);
    connect(&Database::instance(), &Database::changed, this, [this] {
        if (loggedIn() && !m_demo) m_reminders->scheduleSoon();
    });
    connect(&DbSync::instance(), &DbSync::started, this, &AppController::syncStateChanged);
    connect(&DbSync::instance(), &DbSync::finished, this, [this](bool ok, const QString &msg) {
        emit syncStateChanged();
        if (ok) {
            bump();
            emit usersChanged();
        }
        emit syncFinished(ok, msg);
    });
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] {
        if (themeMode() == 0) emit themeChanged();
    });
#endif
    Tones::preload();
}

AppController::~AppController() = default;

// ---------------------------------------------------------------- Eigenschaften

QString AppController::appName() const { return QStringLiteral("ViviaBreathe"); }
QString AppController::version() const { return vbVersion(); }
bool AppController::isAndroid() const { return Platform::isAndroid(); }
QString AppController::language() const { return I18n::language(); }
bool AppController::languageChosen() const { return QSettings().value(QLatin1String(kLangChosen), false).toBool(); }
bool AppController::disclaimerAccepted() const { return QSettings().value(QLatin1String(kDisclaimer), false).toBool(); }

bool AppController::hasDatabase() const
{
    return Database::fileHasContent(Database::defaultPath());
}

bool AppController::loggedIn() const { return Database::instance().isOpen(); }

int AppController::themeMode() const { return QSettings().value(QLatin1String(kTheme), 0).toInt(); }

void AppController::setThemeMode(int m)
{
    m = qBound(0, m, 2);
    if (m == themeMode()) return;
    QSettings().setValue(QLatin1String(kTheme), m);
    emit themeChanged();
}

bool AppController::dark() const
{
    const int m = themeMode();
    if (m == 1) return false;
    if (m == 2) return true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#else
    return false;
#endif
}

QVariantList AppController::users() const
{
    QVariantList l;
    if (!loggedIn()) return l;
    for (const User &u : Database::instance().users())
        l << QVariantMap{ { QStringLiteral("id"), u.id }, { QStringLiteral("name"), u.name } };
    return l;
}

QString AppController::userName() const
{
    if (!loggedIn() || !m_userId) return {};
    const auto u = Database::instance().user(m_userId);
    return u ? u->name : QString();
}

int AppController::autoLockMinutes() const { return QSettings().value(QLatin1String(kAutoLock), 0).toInt(); }

void AppController::setAutoLockMinutes(int m)
{
    if (m == autoLockMinutes()) return;
    QSettings().setValue(QLatin1String(kAutoLock), qMax(0, m));
    emit settingsChanged();
}

bool AppController::syncBusy() const { return DbSync::instance().isBusy(); }
QObject *AppController::runner() const { return m_runner; }
QObject *AppController::playback() const { return m_playback; }
bool AppController::multimedia() const { return MusicPlayer::available(); }
bool AppController::vibration() const { return Tones::vibrationAvailable(); }

void AppController::setError(const QString &e)
{
    if (e == m_error) return;
    m_error = e;
    emit errorChanged();
}

void AppController::bump()
{
    ++m_revision;
    emit revisionChanged();
}

// ---------------------------------------------------------------- Sprache, Hinweis

void AppController::setLanguage(const QString &lang)
{
    QSettings().setValue(QLatin1String(kLangChosen), true);
    if (lang != I18n::language()) {
        I18n::setLanguage(lang);
        I18n::install(*QCoreApplication::instance());
        if (m_engine) m_engine->retranslate();
    }
    emit languageChanged();
    bump();
}

void AppController::acceptDisclaimer()
{
    QSettings().setValue(QLatin1String(kDisclaimer), true);
    emit sessionChanged();
}

// ---------------------------------------------------------------- Datenbank, Anmeldung

bool AppController::openDb(const QString &path, const QString &password)
{
    QString err;
    const auto r = Database::instance().open(path, password, &err);
    if (r == Database::OpenResult::WrongPassword) { setError(tr("Das Master-Passwort ist falsch.")); return false; }
    if (r != Database::OpenResult::Ok) { setError(err.isEmpty() ? tr("Die Datenbank lässt sich nicht öffnen.") : err); return false; }
    setError(QString());
    return true;
}

bool AppController::createDatabase(const QString &password, const QString &repeat, const QString &firstUser)
{
    const QString problem = passwordProblem(password);
    if (!problem.isEmpty()) { setError(problem); return false; }
    if (password != repeat) { setError(tr("Die beiden Eingaben stimmen nicht überein.")); return false; }
    if (firstUser.trimmed().isEmpty()) { setError(tr("Bitte einen Namen für den ersten Benutzer eingeben.")); return false; }
    const QString path = Database::defaultPath();
    if (Database::fileHasContent(path)) { setError(tr("Auf diesem Gerät gibt es schon eine Datenbank.")); return false; }
    QFile::remove(path);
    if (!openDb(path, password)) return false;
    if (Database::instance().users().isEmpty()) Database::instance().addUser(firstUser.trimmed());
    afterLogin();
    return true;
}

bool AppController::login(const QString &password)
{
    if (password.isEmpty()) { setError(tr("Bitte das Master-Passwort eingeben.")); return false; }
    if (!openDb(Database::defaultPath(), password)) return false;
    DbSync::adoptInitialState(Database::defaultPath());
    afterLogin();
    return true;
}

void AppController::afterLogin()
{
    Database &db = Database::instance();
    const QList<User> us = db.users();
    qint64 last = db.deviceSetting(QStringLiteral("ui/lastUser")).toLongLong();
    bool found = false;
    for (const User &u : us) found = found || u.id == last;
    m_userId = found ? last : (us.isEmpty() ? 0 : us.first().id);
    emit usersChanged();
    emit userChanged();
    emit sessionChanged();
    bump();
    if (!m_demo) {
        m_reminders->scheduleNow();
        if (!m_reminders->permissionGranted() && !QSettings().value(QStringLiteral("mobile/notificationAsked"), false).toBool()
            && (!Database::instance().events().isEmpty() || !Database::instance().alarms().isEmpty())) {
            QSettings().setValue(QStringLiteral("mobile/notificationAsked"), true);
            m_reminders->requestPermission();      // erst fragen, wenn es etwas zu erinnern gibt
        }
        const DbSync::Config c = DbSync::config();
        if (c.enabled && DbSync::isReady(c)) {
            DbSync::instance().watchDatabase();
            if (c.autoSync) QTimer::singleShot(1500, this, [] { DbSync::instance().sync(); });
        }
    }
}

void AppController::lock()
{
    if (!loggedIn()) return;
    if (m_runner->active()) m_runner->abort();
    if (!m_demo) {
        m_reminders->scheduleNow();
        const DbSync::Config c = DbSync::config();
        if (c.enabled && c.autoSync && DbSync::isReady(c) && DbSync::instance().status().pending) {
            QString msg;
            DbSync::instance().syncBlocking(&msg);
        }
    }
    Database::instance().close();
    m_userId = 0;
    m_demo = false;
    emit userChanged();
    emit usersChanged();
    emit sessionChanged();
}

bool AppController::startDemo()
{
    if (loggedIn()) Database::instance().close();
    const QString path = demoPath();
    QFile::remove(path);
    if (!openDb(path, QStringLiteral("demo"))) return false;
    DemoData::create();
    m_demo = true;
    afterLogin();
    return true;
}

void AppController::setDemoMode()
{
    m_demo = true;
}

bool AppController::restoreBackup(const QString &url, const QString &password)
{
    if (Database::fileHasContent(Database::defaultPath())) {
        setError(tr("Auf diesem Gerät gibt es schon eine Datenbank. Bitte zuerst die App-Daten löschen oder die Sicherung am Computer einspielen."));
        return false;
    }
    const QString tmp = workDir(QStringLiteral("import")) + QStringLiteral("/import.vbdb");
    QString err;
    if (!Platform::copyToFile(url, tmp, &err)) { setError(tr("Die Sicherung lässt sich nicht lesen: %1").arg(err)); return false; }
    // erst prüfen, ob das Passwort passt
    const auto r = Database::instance().open(tmp, password, &err);
    Database::instance().close();
    if (r == Database::OpenResult::WrongPassword) { setError(tr("Das Master-Passwort passt nicht zu dieser Sicherung.")); return false; }
    if (r != Database::OpenResult::Ok) { setError(tr("Das ist keine ViviaBreathe-Sicherung: %1").arg(err)); return false; }
    QFile::remove(Database::defaultPath());
    if (!QFile::copy(tmp, Database::defaultPath())) { setError(tr("Die Sicherung lässt sich nicht übernehmen.")); return false; }
    QFile::remove(tmp);
    return login(password);
}

bool AppController::setupFromServer(const QString &url, const QString &key, const QString &password)
{
    if (url.trimmed().isEmpty() || key.trimmed().isEmpty()) { setError(tr("Bitte Adresse und Abgleich-Schlüssel eingeben.")); return false; }
    if (Database::fileHasContent(Database::defaultPath())) { setError(tr("Auf diesem Gerät gibt es schon eine Datenbank.")); return false; }
    QString err;
    if (!DbSync::downloadInitial(url.trimmed(), key.trimmed(), Database::defaultPath(), &err)) { setError(err); return false; }
    if (!login(password)) {
        QFile::remove(Database::defaultPath());      // falsches Passwort: nicht halb eingerichtet zurücklassen
        return false;
    }
    DbSync::Config c = DbSync::config();
    c.enabled = true;
    c.url = url.trimmed();
    c.uploadKey = key.trimmed();
    DbSync::saveConfig(c);
    DbSync::instance().watchDatabase();
    return true;
}

bool AppController::changePassword(const QString &oldPw, const QString &newPw, const QString &repeat)
{
    if (!loggedIn()) return false;
    if (!Database::instance().verifyPassword(oldPw)) { setError(tr("Das bisherige Master-Passwort ist falsch.")); return false; }
    const QString problem = passwordProblem(newPw);
    if (!problem.isEmpty()) { setError(problem); return false; }
    if (newPw != repeat) { setError(tr("Die beiden Eingaben stimmen nicht überein.")); return false; }
    QString err;
    if (!Database::instance().changePassword(newPw, &err)) { setError(err); return false; }
    setError(QString());
    return true;
}

QString AppController::workDir(const QString &sub) const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') + sub;
    QDir().mkpath(dir);
    return dir;
}

bool AppController::shareFiles(const QStringList &files, const QString &mime, const QString &subject)
{
    if (files.isEmpty()) { setError(tr("Es gibt nichts zu teilen.")); return false; }
    m_lastShared = files;
    if (!Platform::shareFiles(files, mime, subject, tr("Teilen mit …"))) {
        setError(tr("Teilen ist auf diesem Gerät nicht möglich."));
        return false;
    }
    return true;
}

bool AppController::shareBackup()
{
    if (!loggedIn()) return false;
    const QString f = workDir(QStringLiteral("share")) + QStringLiteral("/ViviaBreathe-%1.vbdb")
                          .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HHmm")));
    QString err;
    if (!Database::instance().backupTo(f, &err)) { setError(err); return false; }
    return shareFiles({ f }, QStringLiteral("application/octet-stream"), tr("ViviaBreathe-Sicherung"));
}

// ---------------------------------------------------------------- Benutzer

void AppController::selectUser(qint64 id)
{
    if (id == m_userId || !loggedIn()) return;
    m_userId = id;
    Database::instance().setDeviceSetting(QStringLiteral("ui/lastUser"), QString::number(id));
    emit userChanged();
    bump();
}

bool AppController::addUser(const QString &name)
{
    if (name.trimmed().isEmpty()) { setError(tr("Bitte einen Namen eingeben.")); return false; }
    QString err;
    const qint64 id = Database::instance().addUser(name.trimmed(), &err);
    if (!id) { setError(err); return false; }
    emit usersChanged();
    selectUser(id);
    return true;
}

bool AppController::renameUser(qint64 id, const QString &name)
{
    if (name.trimmed().isEmpty()) { setError(tr("Bitte einen Namen eingeben.")); return false; }
    QString err;
    if (!Database::instance().renameUser(id, name.trimmed(), &err)) { setError(err); return false; }
    emit usersChanged();
    emit userChanged();
    return true;
}

bool AppController::deleteUser(qint64 id)
{
    if (Database::instance().users().size() <= 1) { setError(tr("Der letzte Benutzer lässt sich nicht löschen.")); return false; }
    QString err;
    if (!Database::instance().deleteUser(id, &err)) { setError(err); return false; }
    if (id == m_userId) {
        m_userId = Database::instance().users().value(0).id;
        Database::instance().setDeviceSetting(QStringLiteral("ui/lastUser"), QString::number(m_userId));
        emit userChanged();
    }
    emit usersChanged();
    bump();
    return true;
}

// ---------------------------------------------------------------- Abgleich

QVariantMap AppController::syncConfig() const
{
    QVariantMap r;
    if (!loggedIn()) return r;
    const DbSync::Config c = DbSync::config();
    r.insert(QStringLiteral("enabled"), c.enabled);
    r.insert(QStringLiteral("autoSync"), c.autoSync);
    r.insert(QStringLiteral("url"), c.url);
    r.insert(QStringLiteral("key"), c.uploadKey);
    QString tls;
    r.insert(QStringLiteral("tls"), DbSync::tlsAvailable(&tls));
    r.insert(QStringLiteral("tlsText"), tls);
    const DbSync::Status st = DbSync::instance().status();
    r.insert(QStringLiteral("last"), st.last.isValid() ? QLocale().toString(st.last.toLocalTime(), QLocale::ShortFormat) : QString());
    r.insert(QStringLiteral("lastResult"), st.lastResult);
    r.insert(QStringLiteral("pending"), st.pending);
    r.insert(QStringLiteral("revision"), st.revision);
    return r;
}

bool AppController::saveSyncConfig(const QVariantMap &m)
{
    if (!loggedIn() || m_demo) return false;
    DbSync::Config c;
    c.enabled = m.value(QStringLiteral("enabled")).toBool();
    c.autoSync = m.value(QStringLiteral("autoSync"), true).toBool();
    c.url = m.value(QStringLiteral("url")).toString().trimmed();
    c.uploadKey = m.value(QStringLiteral("key")).toString().trimmed();
    QString why;
    if (c.enabled && !DbSync::isReady(c, &why)) { setError(why); return false; }
    DbSync::saveConfig(c);
    if (c.enabled) DbSync::instance().watchDatabase();
    setError(QString());
    return true;
}

void AppController::syncNow()
{
    if (!loggedIn() || m_demo) return;
    const DbSync::Config c = DbSync::config();
    QString why;
    if (!c.enabled || !DbSync::isReady(c, &why)) {
        emit syncFinished(false, why.isEmpty() ? tr("Der Abgleich ist nicht eingerichtet.") : why);
        return;
    }
    DbSync::instance().sync();
}

QString AppController::generateSyncKey() const { return DbSync::generateUploadKey(); }

void AppController::syncIfDue()
{
    if (!loggedIn() || m_demo) return;
    const DbSync::Config c = DbSync::config();
    if (c.enabled && c.autoSync && DbSync::isReady(c) && !DbSync::instance().isBusy()) DbSync::instance().sync();
}

void AppController::appHidden()
{
    if (loggedIn() && !m_demo) {
        m_reminders->scheduleNow();
        syncIfDue();
    }
}

void AppController::appActive()
{
    m_playback->refresh();
    if (loggedIn() && !m_demo) m_reminders->scheduleSoon();
    emit launchActionPending();      // QML holt eine angetippte Benachrichtigung mit takeLaunchAction()
}

QString AppController::licenseText() const
{
    QFile f(QStringLiteral(":/license/LICENSE"));
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QString::fromUtf8(f.readAll());
}
