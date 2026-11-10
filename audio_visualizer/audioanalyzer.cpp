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
    double sumSquares = 0.0;
    int start = (m_write - m_delaySamples - kWindow + kRing) % kRing;

    for(int i = 0; i < kWindow; i++) {
        const double s = m_ring[(start + i) % kRing];
        sumSquares += s * s;
    }

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