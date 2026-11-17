import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 800
    height: 520
    minimumWidth: 520
    minimumHeight: 360
    visible: !games.running
    title: "Arcade Launcher"
    color: "#101626"
    onClosing: Qt.quit()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 36
        spacing: 18

        Label {
            text: "ARCADE"
            color: "#7dd3fc"
            font.pixelSize: 14
            font.bold: true
            font.letterSpacing: 4
        }
        Label {
            text: "Choose your game"
            color: "#f8fafc"
            font.pixelSize: 30
            font.bold: true
        }
        Label {
            text: "Discover a game and press play."
            color: "#a6b5ca"
            font.pixelSize: 15
        }

        ListView {
            id: gameList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            clip: true
            model: games
            delegate: Rectangle {
                required property int index
                required property string gameTitle
                width: gameList.width
                height: 100
                radius: 12
                color: "#1d2b42"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 18
                    Rectangle {
                        width: 52
                        height: 52
                        radius: 26
                        color: "#38bdf8"
                    }
                    Label {
                        text: gameTitle
                        color: "#f8fafc"
                        font.pixelSize: 20
                        font.bold: true
                        Layout.fillWidth: true
                    }
                    Button {
                        text: "Play"
                        onClicked: games.launch(index)
                    }
                }
            }
            Label {
                anchors.centerIn: parent
                visible: gameList.count === 0
                text: "No games found. Build a game, then refresh."
                color: "#a6b5ca"
            }
        }

        Label {
            Layout.fillWidth: true
            visible: games.errorMessage.length > 0
            text: games.errorMessage
            color: "#fca5a5"
            wrapMode: Text.Wrap
        }
        Button {
            text: "Refresh games"
            onClicked: games.refresh()
        }
    }
}
