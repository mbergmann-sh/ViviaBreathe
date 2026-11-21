// ViviaBreathe mobil – Brücke zu Android (Java-Klassen in android/src/de/mbergmann/viviabreathe)
//
// Am Desktop (Testbetrieb) sind die Funktionen ohne Wirkung bzw. liefern sinnvolle Ersatzwerte.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace Platform {
bool isAndroid();
void keepScreenOn(bool on);                      // während einer Übung den Bildschirm anlassen
void vibrate(int ms);
bool persistUri(const QString &uri);              // Leserecht einer gewählten Datei (content://) dauerhaft behalten
QString displayName(const QString &fileOrUri);    // lesbarer Dateiname (auch für content://)
bool shareFiles(const QStringList &files, const QString &mime, const QString &subject, const QString &chooserTitle);
// Aktion aus einer angetippten Benachrichtigung abholen (einmalig): {kind, ref, user}; leer = keine
QVariantMap takeLaunchAction();
void openAppNotificationSettings();
// Eine Datei aus der Android-Dateiauswahl (content://…) in einen eigenen Pfad kopieren
bool copyToFile(const QString &uri, const QString &target, QString *error);
}
