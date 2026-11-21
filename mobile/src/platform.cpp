// ViviaBreathe mobil – Brücke zu Android (siehe platform.h)
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#include "platform.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniObject>
#include <QtCore/qcoreapplication_platform.h>

namespace {
const char *kDevice = "de/mbergmann/viviabreathe/Device";
QJniObject context() { return QJniObject(QNativeInterface::QAndroidApplication::context()); }
QJniObject jstr(const QString &s) { return QJniObject::fromString(s); }
}
#endif

namespace Platform {

bool isAndroid()
{
#ifdef Q_OS_ANDROID
    return true;
#else
    return false;
#endif
}

void keepScreenOn(bool on)
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(kDevice, "keepScreenOn", "(Landroid/content/Context;Z)V",
                                       context().object(), jboolean(on));
#else
    Q_UNUSED(on)
#endif
}

void vibrate(int ms)
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(kDevice, "vibrate", "(Landroid/content/Context;I)V", context().object(), jint(ms));
#else
    Q_UNUSED(ms)
#endif
}

bool persistUri(const QString &uri)
{
#ifdef Q_OS_ANDROID
    if (!uri.startsWith(QLatin1String("content:"))) return true;
    return QJniObject::callStaticMethod<jboolean>(kDevice, "persistUri", "(Landroid/content/Context;Ljava/lang/String;)Z",
                                                  context().object(), jstr(uri).object<jstring>());
#else
    Q_UNUSED(uri)
    return true;
#endif
}

QString displayName(const QString &f)
{
    if (f.isEmpty()) return {};
#ifdef Q_OS_ANDROID
    if (f.startsWith(QLatin1String("content:"))) {
        const QString n = QJniObject::callStaticObjectMethod(kDevice, "displayName",
            "(Landroid/content/Context;Ljava/lang/String;)Ljava/lang/String;", context().object(),
            jstr(f).object<jstring>()).toString();
        if (!n.isEmpty()) return n;
    }
#endif
    if (f.contains(QLatin1String("://"))) {
        const QUrl u(f);
        const QString n = u.isLocalFile() ? QFileInfo(u.toLocalFile()).fileName() : u.fileName();
        return n.isEmpty() ? f : QUrl::fromPercentEncoding(n.toUtf8());
    }
    return QFileInfo(f).fileName();
}

bool shareFiles(const QStringList &files, const QString &mime, const QString &subject, const QString &chooserTitle)
{
#ifdef Q_OS_ANDROID
    return QJniObject::callStaticMethod<jboolean>(kDevice, "share",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
        context().object(), jstr(files.join(QLatin1Char('\n'))).object<jstring>(), jstr(mime).object<jstring>(),
        jstr(subject).object<jstring>(), jstr(chooserTitle).object<jstring>());
#else
    Q_UNUSED(mime) Q_UNUSED(subject) Q_UNUSED(chooserTitle)
    return !files.isEmpty();
#endif
}

QVariantMap takeLaunchAction()
{
#ifdef Q_OS_ANDROID
    const QString json = QJniObject::callStaticObjectMethod("de/mbergmann/viviabreathe/OpenActivity", "take",
        "(Landroid/content/Context;)Ljava/lang/String;", context().object()).toString();
    if (json.isEmpty()) return {};
    return QJsonDocument::fromJson(json.toUtf8()).object().toVariantMap();
#else
    return {};
#endif
}

void openAppNotificationSettings()
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(kDevice, "openNotificationSettings", "(Landroid/content/Context;)V", context().object());
#endif
}

bool copyToFile(const QString &uri, const QString &target, QString *error)
{
    // Qt öffnet content://-Adressen unter Android direkt mit QFile
    const QUrl u(uri);
    const QString src = u.isLocalFile() ? u.toLocalFile() : uri;
    QFile in(src);
    if (!in.open(QIODevice::ReadOnly)) {
        if (error) *error = in.errorString();
        return false;
    }
    QFile::remove(target);
    QFile out(target);
    if (!out.open(QIODevice::WriteOnly)) {
        if (error) *error = out.errorString();
        return false;
    }
    while (!in.atEnd()) {
        const QByteArray chunk = in.read(1 << 20);
        if (chunk.isEmpty() && in.error() != QFile::NoError) break;
        if (out.write(chunk) != chunk.size()) {
            if (error) *error = out.errorString();
            return false;
        }
    }
    return true;
}

} // namespace Platform
