// ViviaBreathe mobil – Ablauf einer Atemübung für die Touch-Oberfläche
//
// Hält Taktgeber (BreathEngine), Signaltöne, Vibration, Hintergrundmusik und den entstehenden
// Tagebuch-Eintrag. Die Seiten (Messen vorher → Übung → Messen nachher → Zusammenfassung) lesen
// die Eigenschaften und rufen start(), pause(), proceed(), stop() und save() auf. Gezeichnet wird
// die Animation von BreathItem mit derselben Routine wie am Desktop (BreathRenderer).
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#pragma once

#include "breathengine.h"
#include "breathmodules.h"
#include "breathrenderer.h"
#include "datatypes.h"

#include <QObject>
#include <QVariantMap>

class MusicPlayer;
class QPainter;

// Einstellungen eines Moduls als Wertetabelle für QML (und zurück)
namespace SettingsMap {
QVariantMap toMap(const ModuleDef &m, const ModuleSettings &s);
ModuleSettings fromMap(const ModuleDef &m, const QVariantMap &map, const ModuleSettings &base);
}

class SessionRunner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(QString moduleId READ moduleId NOTIFY changed)
    Q_PROPERTY(QString moduleName READ moduleName NOTIFY changed)
    Q_PROPERTY(QString state READ state NOTIFY changed)      // idle preroll running paused waiting finished
    Q_PROPERTY(QString title READ title NOTIFY tick)          // „Einatmen“
    Q_PROPERTY(QString sub READ sub NOTIFY tick)              // Hinweis zur Phase
    Q_PROPERTY(QString info READ info NOTIFY tick)            // „Zyklus 3 · vergangen 0:32 · noch 4:28“
    Q_PROPERTY(int phaseRemaining READ phaseRemaining NOTIFY tick)
    Q_PROPERTY(double totalProgress READ totalProgress NOTIFY tick)
    Q_PROPERTY(bool soundOn READ soundOn WRITE setSoundOn NOTIFY changed)
    Q_PROPERTY(bool musicOn READ musicOn NOTIFY changed)
    Q_PROPERTY(int musicVolume READ musicVolume WRITE setMusicVolume NOTIFY changed)
    Q_PROPERTY(QString emergencyNote READ emergencyNote NOTIFY changed)
    Q_PROPERTY(QString postureImage READ postureImage NOTIFY changed)
    Q_PROPERTY(bool completed READ completed NOTIFY changed)
    Q_PROPERTY(bool trainingMode READ trainingMode NOTIFY changed)
    Q_PROPERTY(QVariantMap askFields READ askFields NOTIFY changed)   // spo2, peakFlow, pulse, feeling
    Q_PROPERTY(bool deviceTraining READ deviceTraining NOTIFY changed)
public:
    explicit SessionRunner(QObject *parent = nullptr);

    // Übung vorbereiten (Einstellungen aus der Vorbereitungsseite); 'event' = Anlass
    Q_INVOKABLE bool prepare(qint64 userId, const QString &moduleId, const QVariantMap &settings);
    Q_INVOKABLE void setBefore(const QVariantMap &values);       // Messwerte vor der Übung
    Q_INVOKABLE void start();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void proceed();
    Q_INVOKABLE void stop();                                      // vorzeitig beenden (zählt als Eintrag)
    Q_INVOKABLE void abort();                                     // verwerfen, ohne Eintrag
    Q_INVOKABLE void setAfter(const QVariantMap &values);        // Messwerte nach der Übung
    Q_INVOKABLE QVariantMap summary() const;                      // für die Zusammenfassung
    Q_INVOKABLE bool save(const QString &note, int deviceBalls, int deviceHoldSec);
    Q_INVOKABLE void reset();

    void paint(QPainter *p, const QRectF &rect, const QColor &fg, bool dark, const QFont &font);
    bool wantsFrame() const { return m_renderer.wantsFrame(); }

    bool active() const { return m_module != nullptr; }
    QString moduleId() const { return m_module ? m_module->id : QString(); }
    QString moduleName() const { return m_module ? m_module->name : QString(); }
    QString state() const;
    QString title() const;
    QString sub() const;
    QString info() const;
    int phaseRemaining() const;
    double totalProgress() const;
    bool soundOn() const { return m_settings.signal || m_settings.doneGong; }
    void setSoundOn(bool on);
    bool musicOn() const { return m_settings.musicEnabled && !m_settings.musicFile.isEmpty(); }
    int musicVolume() const { return m_settings.musicVolume; }
    void setMusicVolume(int v);
    QString emergencyNote() const { return m_module ? m_module->emergencyNote : QString(); }
    QString postureImage() const { return m_module ? m_module->postureImage : QString(); }
    bool completed() const { return m_rec.completed; }
    bool trainingMode() const { return m_settings.trainingMode; }
    QVariantMap askFields() const;
    bool deviceTraining() const { return m_module && m_module->deviceTraining; }
    QString lastError() const { return m_error; }

signals:
    void changed();
    void tick();
    void finished(bool completed);       // Übung zu Ende (vollständig oder vorzeitig)
    void saved();

private:
    void onPhaseChanged(int index, PhaseKind kind);
    void onFinished(bool completed);
    int repetitionsTotal() const;

    BreathEngine *m_engine = nullptr;
    MusicPlayer *m_music = nullptr;
    BreathRenderer m_renderer;
    const ModuleDef *m_module = nullptr;
    ModuleSettings m_settings;
    qint64 m_userId = 0;
    SessionRecord m_rec;
    QString m_error;
};
