// Android: Hinweis, wenn Benachrichtigungen oder pünktliche Wecker nicht erlaubt sind
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Rectangle {
    id: h
    property var st: app.reminderStatus()
    property bool needExact: false        // Wecker: pünktlich wichtig
    function refresh() { st = app.reminderStatus() }
    readonly property bool problem: app.isAndroid && (!st.permission || (needExact && !st.exact))
    visible: problem
    Layout.fillWidth: true
    implicitHeight: col.implicitHeight + 24
    radius: Theme.radius
    color: Theme.accentWash
    border.color: Theme.accent
    Connections { target: Qt.application; function onStateChanged() { h.refresh() } }
    ColumnLayout {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
        spacing: 6
        Label { visible: !h.st.permission; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink
                text: qsTr("Benachrichtigungen sind für ViviaBreathe ausgeschaltet – Erinnerungen und Wecker erscheinen dann nicht.") }
        VButton { visible: !h.st.permission; text: qsTr("Benachrichtigungen erlauben …"); onClicked: app.requestNotificationPermission() }
        Label { visible: h.needExact && !h.st.exact; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.ink
                text: qsTr("Pünktliche Wecker sind nicht erlaubt – Android kann Wecker und Erinnerungen dann um einige Minuten verschieben.") }
        VButton { visible: h.needExact && !h.st.exact; text: qsTr("Pünktliche Wecker erlauben …"); onClicked: app.openExactAlarmSettings() }
    }
}
