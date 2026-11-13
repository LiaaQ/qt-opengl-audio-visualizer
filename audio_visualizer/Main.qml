import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtMultimedia
import audio_visualizer

ApplicationWindow {
    id: window

    width: 1000
    height: 750
    minimumWidth: 650
    minimumHeight: 650
    visible: true
    title: qsTr("Audio Visualizer")
    color: "#101019"

    // Theme colours
    readonly property color accent: "#e879f9"
    readonly property color accentSecondary: "#a78bfa"
    readonly property color panelColor: "#1b1b29"
    readonly property color mutedText: "#85859b"

    function formatTime(milliseconds) {
        const totalSeconds = Math.floor(milliseconds / 1000)
        const minutes = Math.floor(totalSeconds / 60)
        const seconds = totalSeconds % 60

        return minutes + ":" + seconds.toString().padStart(2, "0")
    }

    function trackName() {
        const path = player.source.toString()

        if (!path)
            return "No track selected"

        const filename = decodeURIComponent(
            path.substring(path.lastIndexOf("/") + 1)
        )

        const extension = filename.lastIndexOf(".")

        return extension > 0
            ? filename.substring(0, extension)
            : filename
    }

    AudioAnalyzer {
        id: analyzer
    }

    MediaPlayer {
        id: player
        source: "importedcontent/heylu.mp3"
        audioBufferOutput: analyzer.output

        audioOutput: AudioOutput {
            id: audioOutput
            volume: 0.8
        }
    }

    FileDialog {
        id: dialog
        title: "Choose an audio file"
        nameFilters: ["Audio files (*.mp3 *.wav *.flac *.ogg *.m4a)"]

        onAccepted: {
            player.source = selectedFile
            player.play()
        }
    }

    // Background
    Rectangle {
        anchors.fill: parent

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#171426" }
            GradientStop { position: 1.0; color: "#101019" }
        }
    }

    // Main player panel
    Rectangle {
        id: panel

        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 760)
        height: Math.min(parent.height - 40, 680)

        color: panelColor
        radius: 20
        border.color: "#343044"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 28
            spacing: 22

            // Header
            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    spacing: 4

                    Label {
                        text: "AUDIO VISUALIZER"
                        color: accent
                        font.pixelSize: 12
                        font.bold: true
                        font.letterSpacing: 2
                    }

                    Label {
                        text: "Now playing"
                        color: "white"
                        font.pixelSize: 25
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.maximumWidth: panel.width - 100
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    radius: 20
                    color: player.playbackState === MediaPlayer.PlayingState
                           ? "#263b35" : "#303040"
                    implicitWidth: statusLabel.implicitWidth + 20
                    implicitHeight: 30

                    Label {
                        id: statusLabel
                        anchors.centerIn: parent
                        text: player.playbackState === MediaPlayer.PlayingState
                              ? "PLAYING" : "PAUSED"
                        color: player.playbackState === MediaPlayer.PlayingState
                               ? "#86efac" : "#aaaabe"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            // Track information
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 75
                radius: 12
                color: "#252436"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    Rectangle {
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 44
                        radius: 10
                        color: "#493052"

                        Label {
                            anchors.centerIn: parent
                            text: "♫"
                            color: accent
                            font.pixelSize: 25
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        Label {
                            Layout.fillWidth: true
                            text: window.trackName()
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Label {
                            text: "Audio track"
                            color: mutedText
                            font.pixelSize: 12
                        }
                    }

                    Button {
                        text: "Open file"
                        onClicked: dialog.open()

                        contentItem: Label {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 12
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            radius: 8
                            color: parent.down ? "#7c3b8d"
                                 : parent.hovered ? "#693477" : "#50305e"
                        }
                    }
                }
            }

            // Playback controls
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "PLAYBACK"
                        color: mutedText
                        font.pixelSize: 11
                        font.bold: true
                        font.letterSpacing: 1
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: formatTime(player.position)
                              + " / " + formatTime(player.duration)
                        color: "#c4c4d4"
                        font.pixelSize: 12
                        font.family: "monospace"
                    }
                }

                Slider {
                    id: seekSlider

                    Layout.fillWidth: true
                    from: 0
                    to: Math.max(1, player.duration)
                    value: player.position
                    enabled: player.seekable && player.duration > 0

                    onMoved: player.position = value

                    background: Rectangle {
                        x: seekSlider.leftPadding
                        y: seekSlider.topPadding
                           + seekSlider.availableHeight / 2 - height / 2
                        width: seekSlider.availableWidth
                        height: 5
                        radius: 3
                        color: "#393749"

                        Rectangle {
                            width: seekSlider.visualPosition * parent.width
                            height: parent.height
                            radius: 3
                            color: accent
                        }
                    }

                    handle: Rectangle {
                        x: seekSlider.leftPadding
                           + seekSlider.visualPosition
                           * (seekSlider.availableWidth - width)
                        y: seekSlider.topPadding
                           + seekSlider.availableHeight / 2 - height / 2
                        width: 14
                        height: 14
                        radius: 7
                        color: "white"
                        border.color: accent
                        border.width: 2
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 14

                    Button {
                        text: "Stop"
                        onClicked: player.stop()

                        background: Rectangle {
                            implicitWidth: 65
                            implicitHeight: 40
                            radius: 10
                            color: parent.down ? "#393749"
                                 : parent.hovered ? "#343346" : "#292838"
                        }

                        contentItem: Label {
                            text: parent.text
                            color: "#c4c4d4"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 13
                        }
                    }

                    Button {
                        text: player.playbackState === MediaPlayer.PlayingState
                              ? "Pause" : "Play"

                        onClicked: {
                            if (player.playbackState === MediaPlayer.PlayingState)
                                player.pause()
                            else
                                player.play()
                        }

                        background: Rectangle {
                            implicitWidth: 110
                            implicitHeight: 44
                            radius: 12
                            color: parent.down ? "#a34bb3"
                                 : parent.hovered ? "#d98be5" : accent
                        }

                        contentItem: Label {
                            text: parent.text
                            color: "#201326"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }
                }
            }

            // Frequency visualizer
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 280
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "FREQUENCY SPECTRUM"
                        color: mutedText
                        font.pixelSize: 11
                        font.bold: true
                        font.letterSpacing: 1
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "8 BANDS"
                        color: accentSecondary
                        font.pixelSize: 10
                        font.bold: true
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 240

                    radius: 12
                    color: "#151522"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        Repeater {
                            model: analyzer.bands

                            delegate: ColumnLayout {
                                required property real modelData
                                required property int index

                                readonly property var frequencyRanges: [
                                    "50–99",
                                    "99–197",
                                    "197–390",
                                    "390–775",
                                    "775–1.54k",
                                    "1.54k–3.05k",
                                    "3.05k–6.05k",
                                    "6.05k–12k"
                                ]

                                Layout.preferredWidth: 70
                                Layout.minimumWidth: 70
                                Layout.maximumWidth: 80
                                Layout.fillHeight: true
                                spacing: 4

                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true

                                    Rectangle {
                                        anchors.fill: parent
                                        radius: 5
                                        color: "#29283a"
                                    }

                                    Rectangle {
                                        anchors.bottom: parent.bottom
                                        width: parent.width
                                        height: parent.height * Math.max(0, Math.min(1, modelData))
                                        radius: 5

                                        gradient: Gradient {
                                            GradientStop {
                                                position: 0.0
                                                color: accentSecondary
                                            }
                                            GradientStop {
                                                position: 1.0
                                                color: accent
                                            }
                                        }
                                    }
                                }

                                Label {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: frequencyRanges[index]
                                    color: mutedText
                                    font.pixelSize: 10
                                    font.family: "monospace"
                                    horizontalAlignment: Text.AlignHCenter
                                }

                                Label {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Hz"
                                    color: "#626276"
                                    font.pixelSize: 9
                                }
                            }
                        }
                    }
                }
            }

            // Volume control
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Label {
                    text: "VOLUME"
                    color: mutedText
                    font.pixelSize: 10
                    font.bold: true
                }

                Slider {
                    id: volumeSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: audioOutput.volume

                    onMoved: audioOutput.volume = value
                }

                Label {
                    text: Math.round(volumeSlider.value * 100) + "%"
                    color: "#c4c4d4"
                    font.pixelSize: 11
                    font.family: "monospace"
                }
            }
        }
    }
}