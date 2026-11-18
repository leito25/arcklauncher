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
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Discover a game and press play."
                color: "#a6b5ca"
                font.pixelSize: 15
                Layout.fillWidth: true
            }
            Label {
                text: "Total playtime  " + profileStats.formattedPlaytime
                color: "#7dd3fc"
                font.pixelSize: 15
                font.bold: true
            }
        }

        GridView {
            id: gameList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: games
            cellWidth: width / Math.max(1, Math.floor(width / 220))
            cellHeight: 150
            delegate: Rectangle {
                required property int index
                required property string gameTitle
                required property string gameId
                width: gameList.cellWidth - 12
                height: gameList.cellHeight - 12
                radius: 12
                color: "#1d2b42"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 6
                    Rectangle {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 4
                        radius: 3
                        color: "#38bdf8"
                    }
                    Label {
                        text: gameTitle
                        color: "#f8fafc"
                        font.pixelSize: 18
                        font.bold: true
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Label {
                        text: profileStats.gamePlaytimes[gameId] || "Not played yet"
                        color: "#a6b5ca"
                        font.pixelSize: 13
                    }
                    Button {
                        text: "Play"
                        Layout.alignment: Qt.AlignRight
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
            visible: games.errorMessage.length > 0 || profileStats.errorMessage.length > 0
            text: [games.errorMessage, profileStats.errorMessage].filter(function(message) {
                return message.length > 0
            }).join("\n")
            color: "#fca5a5"
            wrapMode: Text.Wrap
        }
        Button {
            text: "Refresh games"
            onClicked: games.refresh()
        }
    }
}
