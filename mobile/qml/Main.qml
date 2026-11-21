// ViviaBreathe mobil – Hauptfenster: Sprache → Hinweis → Einrichtung/Anmeldung → Arbeitsbereich
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
import QtQuick
import QtQuick.Controls
import QtQuick.Window
import "components"
import "pages"

ApplicationWindow {
    id: win
    visible: true
    width: 1280
    height: 800
    title: app.appName
    color: Theme.paper
    font.family: Theme.family
    font.pixelSize: Theme.fsM

    readonly property bool wide: width >= 840          // Tablet quer: Seitenleiste statt Leiste unten
    readonly property bool phone: width < 600           // Smartphone hochkant: knappere Kopfzeilen
    function toast(msg, error) { toastItem.show(msg, error) }

    function startPage() {
        if (!app.languageChosen) return languagePage
        if (!app.disclaimerAccepted) return disclaimerPage
        if (app.loggedIn) return homePage
        return app.hasDatabase ? loginPage : setupPage
    }
    function goStart() { stack.replace(null, startPage()) }
    function home() { return stack.find(function(item) { return item.objectName === "home" }) }

    // ---- Übung: Vorbereitung → (Messen vorher) → Übung → (Messen nachher) → Zusammenfassung
    function openPrep(moduleId, eventName) {
        stack.push(prepComp, { moduleId: moduleId, presetEvent: eventName || "" })
    }
    function openGuide(moduleId) { stack.push(guideComp, { moduleId: moduleId }) }
    function beginSession() {          // nach app.startSession()
        if (app.runner.trainingMode) stack.replace(stack.currentItem, measureComp, { before: true })
        else stack.replace(stack.currentItem, runComp)
    }
    function quickHelp() {             // Atemnot-Hilfe: sofort, ohne Vorbereitung
        const id = app.emergencyModuleId()
        if (!id) return
        if (app.startSession(id, app.moduleSettings(id))) stack.push(runComp, { quick: true })
        else win.toast(app.lastError, true)
    }
    function sessionFinished() {
        if (app.runner.trainingMode) stack.replace(stack.currentItem, measureComp, { before: false })
        else stack.replace(stack.currentItem, summaryComp)
    }
    // Einschlafen mit Übung vorher: nach der Übung startet die Musik
    property var pendingSleep: null
    function backHome(msg) {
        const h = win.home()
        if (h) stack.pop(h); else win.goStart()
        if (msg) win.toast(msg)
        if (win.pendingSleep) {
            const p = win.pendingSleep
            win.pendingSleep = null
            if (app.startSleep(p.planId, p.minutes)) win.toast(qsTr("Gute Nacht!"))
            else win.toast(app.lastError, true)
        }
    }

    // Seite direkt öffnen (Bildschirmfotos/Prüfungen des Autopiloten)
    function openPageFile(file, props) { return stack.push(Qt.resolvedUrl("pages/" + file), props || {}) }

    // „Zurück“ (Taste oder Wischgeste): geöffnete Seite schließen, auf der Startseite zuerst zu „Übungen“
    function goBack() {
        if (alarmOverlay.visible) return true
        const cur = stack.currentItem
        if (cur && typeof cur.goBack === "function" && cur.goBack()) return true
        if (stack.depth > 1) { stack.pop(); return true }
        return false
    }
    onClosing: (close) => {
        if (Qt.platform.os === "android" && win.goBack()) close.accepted = false
    }

    // Benachrichtigung angetippt: Übung öffnen, Einschlafen anbieten, Wecker zeigen
    function handleLaunch() {
        if (!app.loggedIn) return
        const a = app.takeLaunchAction()
        if (!a.kind) return
        if (a.kind === "event" && a.moduleId) {
            win.backHome()
            win.openPrep(a.moduleId, a.eventName)
        } else if (a.kind === "sleep") {
            win.backHome()
            stack.push(sleepRunComp, { planId: Number(a.ref) })
        }
    }
    Connections {
        target: app
        function onLaunchActionPending() { Qt.callLater(win.handleLaunch) }
        function onLocked() { win.goStart(); win.toast(qsTr("Aus Sicherheitsgründen gesperrt – bitte erneut anmelden.")) }
        function onSessionChanged() { if (app.loggedIn) Qt.callLater(win.handleLaunch) }
        function onSyncFinished(ok, message) { if (app.loggedIn) win.toast(ok ? message : qsTr("Abgleich: %1").arg(message), !ok) }
    }

    StackView {
        id: stack
        objectName: "stack"
        anchors.fill: parent
        // Android 15+: nicht unter Status- und Navigationsleiste (ab Qt 6.9 polstert das Fenster selbst)
        anchors.topMargin: systemUi.windowPads ? 0 : systemUi.top
        anchors.bottomMargin: systemUi.windowPads ? 0 : systemUi.bottom
        anchors.leftMargin: systemUi.windowPads ? 0 : systemUi.left
        anchors.rightMargin: systemUi.windowPads ? 0 : systemUi.right
        initialItem: win.startPage()
        pushEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 140 } }
        pushExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 140 } }
        popEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 140 } }
        popExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 140 } }
        replaceEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 140 } }
        replaceExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 140 } }
    }

    Component { id: languagePage; LanguagePage { onDone: win.goStart() } }
    Component { id: disclaimerPage; DisclaimerPage { onDone: win.goStart() } }
    Component { id: setupPage; SetupPage { onDone: win.goStart() } }
    Component { id: loginPage; LoginPage { onDone: win.goStart(); onSetupRequested: stack.push(setupPage, { fromLogin: true }) } }
    Component { id: homePage; HomePage { objectName: "home" } }
    Component { id: prepComp; PrepPage {
        onClosed: { win.pendingSleep = null; stack.pop() }
        onStarted: win.beginSession() } }
    Component { id: guideComp; GuidePage { onClosed: stack.pop() } }
    Component { id: measureComp; MeasurePage {
        onDone: { if (before) stack.replace(stack.currentItem, runComp); else stack.replace(stack.currentItem, summaryComp) } } }
    Component { id: runComp; RunPage { onFinished: win.sessionFinished(); onAborted: win.backHome() } }
    Component { id: summaryComp; SummaryPage { onClosed: (msg) => win.backHome(msg) } }
    Component { id: sleepRunComp; SleepRunPage { onClosed: stack.pop() } }

    // Wecker klingelt: Vollbild mit „Aus“ und „Schlummern“ (auch vor der Anmeldung)
    AlarmOverlay { id: alarmOverlay; anchors.fill: parent; z: 900
                   onWakeExercise: (moduleId) => { if (app.loggedIn) { win.backHome(); win.openPrep(moduleId, qsTr("Nach dem Aufwachen")) } } }

    Toast { id: toastItem }
}
