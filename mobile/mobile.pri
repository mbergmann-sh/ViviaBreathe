# ---------------------------------------------------------------------------
#  ViviaBreathe mobil – gemeinsamer Teil der Touch-Oberfläche (Qt Quick)
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
# ---------------------------------------------------------------------------
lessThan(QT_MAJOR_VERSION, 6) | equals(QT_MAJOR_VERSION, 6):lessThan(QT_MINOR_VERSION, 4) {
    error("ViviaBreathe mobil braucht Qt 6.4 oder neuer (gefunden: $$[QT_VERSION])")
}

QT += core gui widgets network svg qml quick quickcontrols2

qtHaveModule(multimedia) {
    QT      += multimedia
    DEFINES += VB_HAVE_MULTIMEDIA
} else {
    warning("ViviaBreathe mobil: OHNE Qt Multimedia - keine Musik und keine Signaltoene")
}

CONFIG   += c++17
TEMPLATE  = app

include($$PWD/../version.pri)
DEFINES  += VIVIABREATHE_VERSION=\\\"$$VERSION\\\"
DEFINES  += QT_DISABLE_DEPRECATED_UP_TO=0x060400
DEFINES  += VB_MOBILE

include($$PWD/../core.pri)

INCLUDEPATH += $$PWD/src
SOURCES += \
    $$PWD/src/main.cpp \
    $$PWD/src/appcontroller.cpp \
    $$PWD/src/appcontroller_data.cpp \
    $$PWD/src/appcontroller_alarms.cpp \
    $$PWD/src/sessionrunner.cpp \
    $$PWD/src/breathitem.cpp \
    $$PWD/src/chartitem.cpp \
    $$PWD/src/playback.cpp \
    $$PWD/src/mobilereminders.cpp \
    $$PWD/src/platform.cpp \
    $$PWD/src/systemui.cpp \
    $$PWD/src/imageproviders.cpp
HEADERS += \
    $$PWD/src/appcontroller.h \
    $$PWD/src/sessionrunner.h \
    $$PWD/src/breathitem.h \
    $$PWD/src/chartitem.h \
    $$PWD/src/playback.h \
    $$PWD/src/mobilereminders.h \
    $$PWD/src/platform.h \
    $$PWD/src/systemui.h \
    $$PWD/src/imageproviders.h

# Nur für Bildschirmfotos/Tests am Desktop: automatischer Rundgang (qmake CONFIG+=autopilot)
autopilot {
    DEFINES += VB_AUTOPILOT
    SOURCES += $$PWD/src/autopilot.cpp
}

RESOURCES += $$PWD/mobile.qrc $$PWD/../resources/viviabreathe.qrc

# Übersetzungen (Quellsprache Deutsch). Die Texte des Kerns stehen in ../translations, die der
# Touch-Oberfläche in mobile/translations. Pflege: siehe mobile/README.md
TRANSLATIONS = $$PWD/translations/viviabreathe_mobile_en.ts
EXTRA_TRANSLATIONS = $$PWD/translations/viviabreathe_mobile_de.ts \
                     $$PWD/../translations/viviabreathe_en.ts \
                     $$PWD/../translations/viviabreathe_de.ts
CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /i18n
lupdate_only { SOURCES += $$files($$PWD/qml/*.qml, true) }

# ---------------------------------------------------------------- Android
android {
    # Versionsnummer für den Paketmanager: 0.6.0 → 6000, 1.2.3 → 1002003
    VB_PARTS = $$split(VERSION, .)
    ANDROID_VERSION_NAME = $$VERSION
    ANDROID_VERSION_CODE = $$num_add($$member(VB_PARTS, 0)000000, $$member(VB_PARTS, 1)000, $$member(VB_PARTS, 2))
    isEmpty(ANDROID_PACKAGE_SOURCE_DIR): ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    # Google Play verlangt Ziel-API 36 (Android 16) und 16-KB-Speicherseiten; beides kann Qt erst
    # ab 6.10. Qt 6.10 und neuer laufen ab Android 9 (API 28).
    !versionAtLeast(QT_VERSION, 6.10.0): error("ViviaBreathe mobil für Android braucht Qt 6.10 oder neuer (gefunden: $$QT_VERSION) – Ziel-API 36 und 16-KB-Speicherseiten")
    ANDROID_TARGET_SDK_VERSION = 36
    ANDROID_MIN_SDK_VERSION = 28
    # OpenSSL für HTTPS (Abgleich über die Website) bindet ../core.pri ein (3rdparty/android_openssl)
}

# iOS: folgt in einer späteren Ausbaustufe (Bauen und Signieren nur auf einem Mac mit Xcode)

OTHER_FILES += $$files($$PWD/qml/*.qml, true) $$PWD/README.md \
    $$PWD/android/AndroidManifest.xml $$files($$PWD/android/src/*.java, true)
