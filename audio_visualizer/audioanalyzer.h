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
    Q_PROPERTY(QVariantList bands READ bands NOTIFY bandsChanged FINAL)

public:
    explicit AudioAnalyzer(QObject *parent = nullptr);
    QAudioBufferOutput *output() { return &m_output; }
    QVariantList bands() const;
    double level() const { return m_level; }

signals:
    void levelChanged();
    void bandsChanged();
private slots:
    void onBuffer(const QAudioBuffer &buffer);
    void update();
private:
    static constexpr int kWindow = 2048;
    static constexpr int kRing = 16384; // 370 ms of history
    static constexpr int kBands = 8;
    static constexpr double fLow = 50.0;
    static constexpr double fHigh = 12000.0;
    static constexpr double kPeakDecay = 0.998;
    static constexpr double kMinPeak = 0.02;
    static constexpr float kAttack = 0.8f;
    static constexpr float kRelease = 0.08f;

    QAudioBufferOutput m_output;
    QTimer m_timer;

    int m_lastWrite = 0;
    int m_idleTicks = 0;

    int m_write = 0;
    int m_sampleRate = 44100;
    int m_tick = 0;
    double m_level = 0.0;

    Fft m_fft{kWindow};
    std::array<float, kRing> m_ring{};
    std::array<std::complex<float>, kWindow> m_buf{}; // FFT input and output
    std::array<float, kWindow> m_hann{};              // window, computed once
    std::array<float, kWindow / 2 + 1> m_mag{};       // magnitudes, bins 0..1024
    std::array<double, kBands + 1> m_edgeHz{};          // Edge frequencies of bands
    std::array<float, kBands> m_peak{};
    std::array<float, kBands> m_levels{};

    template <typename T>
    void pushFrames(const T *s, qsizetype frames, int channels, float scale);
};

#endif // AUDIOANALYZER_H
