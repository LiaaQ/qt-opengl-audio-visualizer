#ifndef AUDIOANALYZER_H
#define AUDIOANALYZER_H

#include <QObject>
#include <QQmlEngine>
#include <QAudioBufferOutput>
#include <QAudioBuffer>
#include <array>
#include <QTimer>
#include <complex>
#include "fft.h"

class AudioAnalyzer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAudioBufferOutput* output READ output CONSTANT FINAL)
    Q_PROPERTY(double level READ level NOTIFY levelChanged FINAL)
    Q_PROPERTY(float delayMs READ delayMs WRITE setDelayMs NOTIFY delayMsChanged FINAL)

public:
    explicit AudioAnalyzer(QObject *parent = nullptr);
    QAudioBufferOutput *output() { return &m_output; }
    double level() const { return m_level; }
    float delayMs() const { return m_delayMs; }
    void setDelayMs(float ms);

signals:
    void levelChanged();
    void delayMsChanged();
private slots:
    void onBuffer(const QAudioBuffer &buffer);
    void update();
private:
    static constexpr int kWindow = 2048;
    static constexpr int kRing = 16384; // 370 ms of history

    QAudioBufferOutput m_output;
    QTimer m_timer;
    std::array<float, kRing> m_ring{};
    int m_write = 0;
    float m_delayMs = 0.0f;
    int m_delaySamples = 0;
    double m_level = 0.0;
    int m_tick = 0;

    Fft m_fft{kWindow};
    std::array<std::complex<float>, kWindow> m_buf{}; // FFT input and output
    std::array<float, kWindow> m_hann{};              // window, computed once
    std::array<float, kWindow / 2 + 1> m_mag{};       // magnitudes, bins 0..1024
};

#endif // AUDIOANALYZER_H
