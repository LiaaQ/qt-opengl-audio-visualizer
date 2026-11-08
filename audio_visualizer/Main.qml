import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtMultimedia

ApplicationWindow {
    id: window
    width: 900
    height: 600
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("Audio Visualizer")

    MediaPlayer {
        id: player
        audioOutput: AudioOutput { volume : 0.8 }
    }

    FileDialog {
        id: dialog
        nameFilters: ["Audio files (*.mp3 *.wav *.flac *.ogg *.m4a)"]
        onAccepted: { player.source = selectedFile; player.play() }
    }

    Column {
        anchors.centerIn: parent
        spacing: 12
        width: parent.width * 0.8

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

        Slider {
            width: parent.width
            from: 0; to: player.duration
            value: player.position
            onMoved: player.position = value
        }
    }

}
