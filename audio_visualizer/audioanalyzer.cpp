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
}

void AudioAnalyzer::onBuffer(const QAudioBuffer &buffer)
{
    if (!buffer.isValid() || buffer.sampleCount() == 0 || buffer.format().sampleFormat() != QAudioFormat::Float || buffer.format().channelCount() != 2)
        return;

    const float *samples = buffer.constData<float>();

    for (qsizetype i = 0; i < buffer.frameCount(); ++i)
    {
        m_ring[m_write] = 0.5f * (samples[2*i] + samples[2*i+1]);
        m_write = (m_write + 1) % kRing;
    }
}

void AudioAnalyzer::update()
{
    const int start = (m_write - m_delaySamples - kWindow + kRing) % kRing;
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

    int idxMax = 0;
    float maxMag = 0.0f;
    for (int k = 1; k < kWindow / 2; k++) {
        if (m_mag[k] > maxMag) {
            idxMax = k;
            maxMag = m_mag[k];
        }
    }

    if(++m_tick % 60 == 0)
        qDebug() << idxMax * 44100.0 / kWindow << "Hz, magnitude" << maxMag;

    m_level = std::sqrt(sumSquares / kWindow);
    emit levelChanged();
}

void AudioAnalyzer::setDelayMs(float ms)
{
    ms = std::clamp(ms, 0.0f, 300.0f);
    if (ms == m_delayMs) return;
    m_delayMs = ms;
    m_delaySamples = int(ms * 44.1f); // 44.1 samples per ms at 44.1 kHz
    emit delayMsChanged();
}