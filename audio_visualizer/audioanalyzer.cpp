#include "audioanalyzer.h"
#include <cmath>
#include <algorithm>

AudioAnalyzer::AudioAnalyzer(QObject *parent)
    : QObject{parent}
{
    // When m_output receives a new audio buffer, call onBuffer() in this AudioAnalyzer object.
    connect(&m_output, &QAudioBufferOutput::audioBufferReceived, this, &AudioAnalyzer::onBuffer);

    m_timer.setInterval(16);
    connect(&m_timer, &QTimer::timeout, this, &AudioAnalyzer::update);
    m_timer.start();


    const float twoPi = 6.2831853f;
    for (int i = 0; i < kWindow; ++i)
        m_hann[i] = 0.5f * (1.0f - std::cos(twoPi * i / kWindow));

    for(int i = 0; i < kBands + 1; i++) // Edge frequencies for the bands
        m_edgeHz[i] = fLow * std::pow(fHigh / fLow, double(i) / kBands);
}

QVariantList AudioAnalyzer::bands() const {
    QVariantList bands;
    for(float val : m_levels)
        bands.append(double(val));

    return bands;
}

template <typename T>
void AudioAnalyzer::pushFrames(const T *s, qsizetype frames, int channels, float scale)
{
    for (qsizetype i = 0; i < frames; ++i) {
        float sum = 0.0f;
        for (int c = 0; c < channels; ++c)
            sum += float(s[i * channels + c]);
        m_ring[m_write] = scale * sum / channels;     // mono = average of channels
        m_write = (m_write + 1) % kRing;
    }
}

void AudioAnalyzer::onBuffer(const QAudioBuffer &buffer)
{
    if (!buffer.isValid() || buffer.frameCount() == 0)
        return;

    const QAudioFormat fmt = buffer.format();
    m_sampleRate = fmt.sampleRate();
    const int ch = fmt.channelCount();
    const qsizetype n = buffer.frameCount();

    switch (fmt.sampleFormat()) {
    case QAudioFormat::Float:
        pushFrames(buffer.constData<float>(),  n, ch, 1.0f);
        break;
    case QAudioFormat::Int16:
        pushFrames(buffer.constData<qint16>(), n, ch, 1.0f / 32768.0f);
        break;
    case QAudioFormat::Int32:
        pushFrames(buffer.constData<qint32>(), n, ch, 1.0f / 2147483648.0f);
        break;
    default:
        break;      // other formats: ignore for now
    }
}

void AudioAnalyzer::update()
{
    const int delaySamples = std::min(int(m_delayMs * m_sampleRate / 1000.0f),
                                          kRing - kWindow);
    const int start = (m_write - delaySamples - kWindow + kRing) % kRing;
    double sumSquares = 0.0;

    for(int i = 0; i < kWindow; i++) {
        const float s = m_ring[(start + i) % kRing] * m_hann[i];
        m_buf[i] = std::complex<float>(s, 0.0f);

        const double s2 = m_ring[(start + i) % kRing];
        sumSquares += s2 * s2;
    }

    m_fft.forward(m_buf.data());

    for (int k = 0; k <= kWindow / 2; k++)
        m_mag[k] = std::abs(m_buf[k]);

    for (int i = 0; i < kBands; i++) {
        int lo = int(std::round(m_edgeHz[i]   * kWindow / m_sampleRate));
        int hi = int(std::round(m_edgeHz[i+1] * kWindow / m_sampleRate));
        hi = std::min(hi, kWindow / 2);

        if (hi <= lo) lo = hi - 1;

        double power = 0.0;
        for (int k = lo; k < hi; k++)
            power += double(m_mag[k]) * m_mag[k];

        const double amp = std::sqrt(power) / (kWindow * 0.25);
        m_peak[i] = float(std::max(m_peak[i] * kPeakDecay, amp));
        m_levels[i] = float(amp / std::max<double>(m_peak[i], kMinPeak));

        // Smooth movement of the values
        const float target = float(amp / std::max<double>(m_peak[i], kMinPeak));
        const float rate = target > m_levels[i] ? kAttack : kRelease;
        m_levels[i] += (target - m_levels[i]) * rate;
    }

    if (++m_tick % 60 == 0) {
        QDebug d = qDebug();
        for (float v : m_levels)
            d << v;
    }

    m_level = std::sqrt(sumSquares / kWindow);
    emit levelChanged();
    emit bandsChanged();
}

void AudioAnalyzer::setDelayMs(float ms)
{
    ms = std::clamp(ms, 0.0f, 300.0f);
    if (ms == m_delayMs) return;
    m_delayMs = ms;
    emit delayMsChanged();
}