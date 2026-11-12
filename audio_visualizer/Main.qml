import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtMultimedia
import audio_visualizer

ApplicationWindow {
    id: window
    width: 900
    height: 600
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("Audio Visualizer")

    function formatTime(milliseconds) {
            const totalSeconds = Math.floor(milliseconds / 1000)
            const minutes = Math.floor(totalSeconds / 60)
            const seconds = totalSeconds % 60

            return minutes + ":" + seconds.toString().padStart(2, "0")
    }

    AudioAnalyzer { id: analyzer }

    Rectangle {
        width: 600
        height: 20
        color: "#333333"

        Rectangle {
            width: analyzer.level * 600
            height: parent.height
            color: "hotpink"
        }
    }

    MediaPlayer {
        id: player
        source: "importedcontent/heylu.mp3"
        audioBufferOutput: analyzer.output
        audioOutput: AudioOutput { volume : 0.8 }
    }

    FileDialog {
        id: dialog
        nameFilters: ["Audio files (*.mp3 *.wav *.flac *.ogg *.m4a)"]
        onAccepted: { player.source = selectedFile; player.play() }
    }

    Column {
        anchors.centerIn: parent
        spacing: 16
        width: parent.width * 0.8

        Label {
            text: {
                const path = player.source.toString()
                return "Currently playing: " + path.substring(path.lastIndexOf("/") + 1)
            }
            font.pixelSize: 22
            font.bold: true
        }

        Row {
            spacing: 8
            Button { text: "Open..."; onClicked: dialog.open() }
            Button {
                text: player.playbackState === MediaPlayer.PlayingState ? "Pause" : "Play"
                onClicked: player.playbackState === MediaPlayer.PlayingState
                           ? player.pause() : player.play()
            }
            Button { text: "Stop"; onClicked: player.stop() }
        }

        // Playback position
        Column {
            width: parent.width
            spacing: 4

            Row {
                width: parent.width

                Label {
                    text: "Playback position"
                    font.bold: true
                }

                Item {
                    width: parent.width - 200
                    height: 1
                }

                Label {
                    text: formatTime(player.position)
                        + " / " + formatTime(player.duration)
                }
            }

            Slider {
                width: parent.width
                from: 0
                to: Math.max(1, player.duration)
                value: player.position

                onMoved: player.position = value
            }
        }

        Row {
            spacing: 6
            Repeater {
                model: analyzer.bands                 // 8 numbers
                delegate: Item {
                    required property real modelData  // the value for this bar
                    width: 40; height: 200
                    Rectangle {
                        width: parent.width
                        height: parent.height * parent.modelData
                        anchors.bottom: parent.bottom
                        color: "hotpink"
                    }
                }
            }
        }

        // Slider {
        //     width: parent.width
        //     from: 0; to: 300
        //     value: analyzer.delayMs
        //     onMoved: analyzer.delayMs = value
        // }
    }

}
