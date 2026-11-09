#include "audioanalyzer.h"

AudioAnalyzer::AudioAnalyzer(QObject *parent)
    : QObject{parent}
{
    // When m_output receives a new audio buffer, call onBuffer() in this AudioAnalyzer object.
    connect(&m_output, &QAudioBufferOutput::audioBufferReceived, this, &AudioAnalyzer::onBuffer);
}

void AudioAnalyzer::onBuffer(const QAudioBuffer &buffer)
{
    if (!buffer.isValid() || buffer.sampleCount() == 0)
        return;

    const float *samples = buffer.constData<float>();
    const qsizetype sampleCount = buffer.sampleCount();

    double sumSquares = 0.0;

    for (qsizetype i = 0; i < sampleCount; ++i)
    {
        const double sample = samples[i];
        sumSquares += sample * sample;
    }

    m_level = std::sqrt(sumSquares / sampleCount);
    emit levelChanged();
}