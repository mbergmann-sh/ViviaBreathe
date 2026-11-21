# ViviaBreathe mobil (Android-Smartphone und -Tablet)

Touch-Version von ViviaBreathe. Sie verwendet denselben Kern wie das Desktop-Programm
(`../core.pri`): dieselbe verschlüsselte Datenbank (SQLite3 Multiple Ciphers, Master-Passwort),
dieselben Übungsmodule und Taktgeber, dieselbe Zeichenroutine für Animation und Verlaufsgrafik,
denselben Bericht (PDF/CSV) und den Abgleich über den eigenen Webserver. Die Oberfläche ist mit
Qt Quick geschrieben und in Aufbau und Bedienelementen an die mobile Fassung von VitaParms angelehnt.

**Stand 0.6.0** – erste Fassung für Android:

* **Ersteinrichtung:** Sprache (Deutsch/Englisch), Hinweis „kein Medizinprodukt“, dann neue
  Datenbank, *vom Webserver übernehmen* (Adresse, Abgleich-Schlüssel, Master-Passwort),
  *Sicherung einspielen* (.vbdb vom Computer) oder *mit Beispieldaten ausprobieren*.
* **Anmeldung** mit dem Master-Passwort; auf Wunsch Sperre nach 5/15/60 Minuten im Hintergrund.
* **Mehrere Benutzer** mit Auswahl in der Kopfzeile bzw. Seitenleiste; Benutzer anlegen,
  umbenennen, löschen.
* **Übungen** als Karten, nach Gruppen (Entspannung und Schlaf / Atemwege – Asthma und COPD), mit
  Überblick (heute, Minuten in 7 Tagen, Tage in Folge, nächste Erinnerung). **Atemnot-Hilfe**
  immer mit einem Tipp erreichbar (rote Schaltfläche).
* **Vorbereitung:** Beschreibung, Tipps, Haltung mit Bild, bebilderte Anleitung (TriBall),
  Atemphasen (±0,5 s), Wiederholungen, Gesamtdauer, Ziel des Atemtrainers, Anlass,
  Signaltöne (getrennt: Phasenwechsel, unterschiedliche Töne, Gong), **Vibration**,
  Hintergrundmusik (MP3 aus der Android-Dateiauswahl) mit Lautstärke, Trainingsmodus;
  *Standardwerte* setzt die Atemwerte zurück. Alles wird je Benutzer gespeichert.
* **Übung:** Animation wie am Desktop (Box, Kreis, TriBall), Phase mit Countdown, Hinweis,
  Fortschritt, Pause/Weiter/Beenden, Töne an/aus, Musiklautstärke. Der Bildschirm bleibt an;
  verlässt man die App (Anruf, andere App), hält die Übung an. Hochkant untereinander, quer
  nebeneinander.
* **Trainingsmodus:** SpO₂, Peak Flow, Puls und Befinden vor und nach der Übung;
  **Zusammenfassung** mit Notiz und Ergebnis des Atemtrainers; speichern oder verwerfen.
* **Verlauf:** Tagebuch als Liste (nach Tagen) oder **Verlaufsgrafik**, Zeitraum 7/30/90 Tage
  bzw. 1 Jahr, Filter nach Übung; Einträge ändern oder löschen; **Teilen** als PDF (mit Grafik,
  Tabelle, Notizen) oder CSV über das Android-Teilen-Menü (E-Mail, Messenger, Drucken …).
* **Erinnerungen:** Ereignisse (z. B. „Einschlafübung“ täglich 21:30) als Android-Benachrichtigung;
  Antippen öffnet die Übung mit dem richtigen Anlass und Benutzer.
* **Wecker:** Uhrzeit, Wochentage, Wiedergabeliste (oder eingebauter Weckton), Lautstärke, sanft
  lauter werden, Schlummern, Übung nach dem Aufwachen anbieten, Probehören. Klingelt auch, wenn
  die App geschlossen ist (Vordergrunddienst); in der App großes „Aus“/„Schlummern“.
* **Einschlafen:** gespeicherte Einträge mit Musik, Spieldauer, Ausblenden, Übung vorher; geplant
  wie der Wecker – mit Benachrichtigung oder automatischem Start der Musik. Die Musik läuft bei
  ausgeschaltetem Bildschirm weiter.
* **Einstellungen:** Sprache, Design (wie das System / hell / dunkel), Master-Passwort ändern,
  Sicherung teilen, Abgleich über den eigenen Webserver, Status der Erinnerungen, Über/Lizenz.
* **Größen:** Smartphone hochkant (ab 360 px Breite), Tablet hoch und quer (Seitenleiste ab
  840 px). Alle Seiten rollen senkrecht; nichts ist nur durch seitliches Wischen erreichbar.

## Was gerätebezogen ist

Musikdateien (Hintergrundmusik der Übungen, Wiedergabelisten von Wecker und Einschlafen) liegen auf
dem jeweiligen Gerät. Die App merkt sie sich deshalb **nur auf diesem Gerät** (nicht abgeglichen);
ein Pfad vom Computer wäre auf dem Telefon ohnehin ungültig und umgekehrt. Alles andere –
Einstellungen der Übungen, Tagebuch, Ereignisse, Wecker- und Einschlafzeiten – wird wie am
Computer abgeglichen. Ein am Computer eingerichteter Wecker weckt auf dem Telefon mit dem
eingebauten Weckton, bis dort Musik gewählt ist.

## Voraussetzungen

* Qt **6.10 oder neuer** mit der Komponente *Android* (ältere Versionen bricht qmake mit einer
  Meldung ab: erst Qt 6.10 kann Ziel-API 36 und 16-KB-Speicherseiten, die Google Play verlangt),
  dazu *Qt Multimedia*. (Qt Maintenance Tool → Qt 6.x → Android, Additional Libraries → Qt Multimedia)
* Qt Creator mit eingerichtetem Android-SDK/NDK
  (*Bearbeiten → Einstellungen → Geräte → Android → „Set Up SDK“* lädt alles Nötige)
* ein Android-Gerät ab Android 9 mit **Entwickleroptionen** und **USB-Debugging**:
  *Einstellungen → Über das Telefon → 7 × auf „Build-Nummer“ tippen*, danach
  *Einstellungen → System → Entwickleroptionen → USB-Debugging* einschalten

## Bauen und auf das Gerät bringen

1. `mobile/ViviaBreatheMobile.pro` in Qt Creator öffnen und das Kit **Android Qt 6.x arm64-v8a**
   wählen (für ältere 32-Bit-Geräte zusätzlich *armeabi-v7a*).
2. Das Android-Manifest liegt bei (`android/AndroidManifest.xml`): Paketname
   `de.mbergmann.viviabreathe`, Symbol `@drawable/icon`, keine Google-Sicherung
   (`allowBackup=false`), Rechte und Empfänger für Erinnerungen und Wecker, Vordergrunddienst
   für die Musik. **Nicht** mit *Create Templates* überschreiben.
3. Gerät per USB anschließen, die Rückfrage „USB-Debugging zulassen?“ bestätigen.
4. In Qt Creator **Ausführen** (Strg+R). Die App wird gebaut, installiert und gestartet.

Ohne Kabel: *Build → Build Android APK* erzeugt
`android-build/build/outputs/apk/debug/android-build-debug.apk`; diese Datei auf das Gerät
kopieren und öffnen (vorher *Installation aus unbekannten Quellen* für den Dateimanager erlauben).

**Release-Build:** Android installiert nur signierte Pakete. Einmalig unter *Projekte → Build
(Release) → Build Android APK → Application Signature*: *Keystore → Erstellen…* (Datei z. B.
`viviabreathe-release.keystore` außerhalb des Projektordners, Alias `viviabreathe`), dann *Sign
package* anhaken. Keystore und Passwörter gehören nicht ins Projekt; gut aufbewahren – nur mit
demselben Schlüssel lassen sich spätere Versionen als Update installieren. Stürzt nur der Release-
Build beim Start ab: unter *Projekte → Build (Release) → qmake → ABIs* dieselbe Architektur wie im
Debug-Build wählen (meist **arm64-v8a**), dann *Bereinigen* und neu erstellen. Die Fehlermeldung
steht im Protokoll des Geräts: `adb logcat -d -t 1500 > viviabreathe-log.txt`.

**HTTPS (Abgleich):** Android bringt kein OpenSSL für Qt mit; `../core.pri` bindet die
mitgelieferten Bibliotheken ein (`3rdparty/android_openssl`, OpenSSL 3 von KDAB, Apache-2.0).
Den Haken *Include prebuilt OpenSSL libraries* in Qt Creator **nicht** zusätzlich setzen. In der App
zeigt *Einstellungen → Abgleich*, ob TLS bereit ist.

## Android-Besonderheiten

* **Benachrichtigungen (Android 13+):** Die App fragt einmal, sobald es Ereignisse oder Wecker
  gibt; später über *Einstellungen → Erinnerungen und Wecker*.
* **Pünktliche Wecker (Android 12+):** Wecker nutzen `setAlarmClock` (Wecker-Symbol in der
  Statusleiste). Dafür braucht es das Recht „Wecker und Erinnerungen“ (`SCHEDULE_EXACT_ALARM`);
  ab Android 14 erteilt es der Anwender selbst (*Einstellungen → Pünktliche Wecker erlauben*).
  Ohne das Recht stellt die App die Termine ungefähr – Android kann sie dann um einige Minuten
  verschieben, und der Wecker zeigt nur eine Benachrichtigung statt die Musik zu starten.
* **14-Tage-Plan:** Die Datenbank ist verschlüsselt, Android kann sie nicht lesen. Die App meldet
  deshalb beim Öffnen, nach Änderungen und beim Wechsel in den Hintergrund die Termine der
  nächsten 14 Tage (alle Benutzer) bei Android an; nach einem Neustart stellt `BootReceiver` sie
  wieder. Wer die App länger als 14 Tage nicht öffnet, bekommt danach keine Erinnerungen mehr.
* **Sperrbildschirm:** Erinnerungen zeigen dort nur „ViviaBreathe – Erinnerung“; der Wecker zeigt
  Bezeichnung, „Aus“ und „Schlummern“.
* **Musikdateien** werden über die Android-Dateiauswahl gewählt; die App behält das Leserecht
  dauerhaft (`takePersistableUriPermission`). Wird eine Datei gelöscht oder verschoben, weckt der
  Wecker mit dem eingebauten Ton.
* **Zurück:** schließt die geöffnete Seite, führt aus einem Bereich zu „Übungen“ und beendet die
  App erst dort. Während einer Übung fragt „Zurück“, ob die Übung beendet werden soll.
* **Darstellung bis an den Rand (Android 15+):** wie VitaParms (`src/systemui.*`,
  `SystemBars.java`): Qt ab 6.9 polstert den Inhalt selbst; die Symbole der Systemleisten passen
  sich dem hellen/dunklen Design an.

## Testen am Desktop

Die App lässt sich am Computer bauen und starten (Kit *Desktop Qt 6.x*), z. B.
`ViviaBreathe --demo --phone` (Fenster in Telefongröße, Beispieldaten) oder `--tablet`.
Wecker und Einschlafmusik spielen dort über Qt Multimedia statt über den Android-Dienst.

**Autopilot** (Bildschirmfotos und Prüfung auf zu breite Elemente):

```sh
mkdir build-autopilot && cd build-autopilot
qmake6 CONFIG+=autopilot ../mobile/ViviaBreatheMobile.pro && make -j
QT_QPA_PLATFORM=offscreen VB_AUTOPILOT_OUT=/tmp/shots VB_AUTOPILOT_LANG=de ./ViviaBreathe
```

Er geht alle Seiten in Smartphone- (390 × 844), Tablet-quer- (1280 × 800) und Tablet-hoch-Größe
(800 × 1280) sowie im dunklen Design durch, startet Übungen (Box, 4-7-8 mit Trainingsmodus,
TriBall, Atemnot-Hilfe), öffnet alle Bearbeiten-Seiten, lässt den Wecker klingeln, erzeugt PDF und
CSV und meldet Elemente, die über den Rand bzw. ihren sichtbaren Bereich hinausragen.

## Übersetzungen

Die Texte der Touch-Oberfläche stehen in `translations/viviabreathe_mobile_en.ts` (englisch) bzw.
`…_mobile_de.ts` (nur deutsche Mehrzahlformen); die Texte des Kerns kommen aus
`../translations`. Nach Textänderungen (im Projektordner):

```sh
lupdate -no-obsolete mobile/qml mobile/src -ts mobile/translations/viviabreathe_mobile_en.ts
lupdate -pluralonly -no-obsolete mobile/qml mobile/src -ts mobile/translations/viviabreathe_mobile_de.ts
python3 tools/fill_ts.py      # Englisch aus tools/translations_en.py und tools/translations_mobile_en.py
```

## Aufbau

```
mobile/ViviaBreatheMobile.pro, mobile.pri   Projekt (Kern aus ../core.pri)
mobile/src/main.cpp                Start, Schrift, QML-Typen, Hintergrund/Vordergrund
mobile/src/appcontroller*.cpp      Bindeglied QML ↔ Kern („app“): Anmeldung, Benutzer, Übungen,
                                   Tagebuch, Berichte, Ereignisse, Wecker, Einschlafen, Abgleich
mobile/src/sessionrunner.*         Ablauf einer Übung (Taktgeber, Töne, Vibration, Musik, Eintrag)
mobile/src/breathitem.*            Atemanimation als QML-Element (BreathRenderer aus dem Kern)
mobile/src/chartitem.*             Verlaufsgrafik als QML-Element (ChartRenderer aus dem Kern)
mobile/src/playback.*              Musik für Wecker/Einschlafen (Android: PlaybackService)
mobile/src/mobilereminders.*       Termine der nächsten 14 Tage → Android (Reminders.java)
mobile/src/platform.*              Brücke zu Android (Device.java, OpenActivity.java)
mobile/src/systemui.*              Systemleisten (Android 15+), wie VitaParms
mobile/src/imageproviders.*        Symbole und Illustrationen in Designfarben
mobile/src/autopilot.cpp           Rundgang für Bildschirmfotos/Prüfungen (nur Desktop)
mobile/qml/Main.qml, Theme.qml     Hauptfenster, Ablauf der Übung, Farben/Schrift/Maße
mobile/qml/pages/                  Seiten (Übungen, Vorbereitung, Übung, Verlauf, …)
mobile/qml/components/             Bedienelemente (große Trefferflächen, umbrechend)
mobile/android/                    Manifest, Symbole, Java-Klassen
mobile/fonts/                      Atkinson Hyperlegible (SIL Open Font License 1.1)
```

## Noch nicht enthalten

* iOS (Bauen und Signieren nur auf einem Mac mit Xcode)
* Handbuch (PDF) – wie vereinbart erst auf Aufforderung
* Google Play: Paket ist vorbereitet (Ziel-API 36, 16-KB-Seiten über Qt 6.10, kein
  `USE_EXACT_ALARM`); Datenschutzerklärung und Store-Eintrag fehlen noch.

## Lizenz

© 2026 Michael Bergmann – PolyForm Noncommercial License 1.0.0 (siehe `../LICENSE`).
Schrift: Atkinson Hyperlegible, © Braille Institute of America, SIL Open Font License 1.1
(`fonts/OFL.txt`).
