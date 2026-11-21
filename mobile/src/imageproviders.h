// ViviaBreathe mobil – Symbole und Illustrationen für QML, passend zum hellen/dunklen Design
//   image://icon/<name>?c=<Farbe>&a=<Akzent>     Symbol aus resources/icons/ui (#COLOR/#ACCENT)
//   image://illus/<name>?dark=1                 Illustration aus resources/illustrations
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include <QQuickImageProvider>

class IconProvider : public QQuickImageProvider
{
public:
    IconProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}
    QImage requestImage(const QString &id, QSize *size, const QSize &requested) override;
};

class IllustrationProvider : public QQuickImageProvider
{
public:
    IllustrationProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}
    QImage requestImage(const QString &id, QSize *size, const QSize &requested) override;
};
