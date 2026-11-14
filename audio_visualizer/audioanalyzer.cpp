#include "audioanalyzer.h"

#include <cmath>
#include <algorithm>
#include <complex>


// -----------------------------------------------------------------------------
// Construction
// -----------------------------------------------------------------------------

AudioAnalyzer::AudioAnalyzer(QObject *parent)
    : QObject{parent}
{
    // When m_output receives a new audio buffer, call onBuffer() in this AudioAnalyzer object.
    connect(&m_output, &QAudioBufferOutput::audioBufferReceived, this, &AudioAnalyzer::onBuffer);

    // Update the visualisation approximately 60 times per second.
    m_timer.setInterval(16);


    connect(&m_timer, &QTimer::timeout, this, &AudioAnalyzer::update);
    m_timer.start();

    // Precalculate the Hann window used to reduce FFT spectral leakage.
    const float twoPi = 6.2831853f;
    for (int i = 0; i < kWindow; ++i) {
        const float pos = float(i) / kWindow;
        m_hann[i] = 0.5f * (1.0f - std::cos(twoPi * pos));
    }

    for(int i = 0; i < kBands + 1; i++) { // Edge frequencies for the bands
        const double pos = double(i) / kBands;
        m_edgeHz[i] = fLow * std::pow(fHigh / fLow, pos);
    }

}

// -----------------------------------------------------------------------------
// Public properties
// -----------------------------------------------------------------------------

QVariantList AudioAnalyzer::bands() const {
    QVariantList bands;
    for(float level : m_levels)
        bands.append(double(level));

    return bands;
}


// -----------------------------------------------------------------------------
// Audio input
// -----------------------------------------------------------------------------

template <typename T>
void AudioAnalyzer::pushFrames(const T *samples, qsizetype frameCount, int channelCount, float scale)
{
    if (!samples || channelCount <= 0) return;

    for (qsizetype frame = 0; frame < frameCount; ++frame) {
        float monoSample = 0.0f;

        for (int channel = 0; channel < channelCount; ++channel) {
            const qsizetype sampleIndex = frame * channelCount + channel;
            monoSample += float(samples[sampleIndex]);
        }

        m_ring[m_write] = scale * monoSample / channelCount; // mono = average of channels
        m_write = (m_write + 1) % kRing;
    }
}

void AudioAnalyzer::onBuffer(const QAudioBuffer &buffer)
{
    if (!buffer.isValid() || buffer.frameCount() <= 0)
        return;

    const QAudioFormat format = buffer.format();

    m_sampleRate = format.sampleRate();
    const int channelCount = format.channelCount();
    const qsizetype frameCount = buffer.frameCount();

    if (m_sampleRate <= 0 || channelCount <= 0)
        return;

    switch (format.sampleFormat()) {
    case QAudioFormat::Float:
        pushFrames(buffer.constData<float>(),  frameCount, channelCount, 1.0f);
        break;
    case QAudioFormat::Int16:
        pushFrames(buffer.constData<qint16>(), frameCount, channelCount, 1.0f / 32768.0f);
        break;
    case QAudioFormat::Int32:
        pushFrames(buffer.constData<qint32>(), frameCount, channelCount, 1.0f / 2147483648.0f);
        break;
    default:
        break;      // other formats: ignore for now
    }
}

// -----------------------------------------------------------------------------
// Audio analysis
// -----------------------------------------------------------------------------

void AudioAnalyzer::update()
{
    if (m_sampleRate <= 0) return;

    // Determine whether new audio samples have arrived.
    if (m_write != m_lastWrite) m_idleTicks = 0;
    else ++m_idleTicks;
    m_lastWrite = m_write;
    const bool receivingAudio = m_idleTicks < 8;     // about 130 ms with no new audio = stopped

    const int start = (m_write - kWindow + kRing) % kRing;   // start of the newest kWindow samples
    double sumSquares = 0.0;

    for (int i = 0; i < kWindow; i++) {
        const float raw = m_ring[(start + i) % kRing];            // the actual sample
        m_buf[i] = std::complex<float>(raw * m_hann[i], 0.0f);    // windowed copy for the FFT
        sumSquares += double(raw) * raw;                          // RMS uses the raw one
    }

    // Transform the audio signal from the time domain into the frequency domain.
    m_fft.forward(m_buf.data());

    for (int bin = 0; bin <= kWindow / 2; bin++)
        m_mag[bin] = std::abs(m_buf[bin]);

    // Calculate the magnitude of each frequency band.
    for (int band = 0; band < kBands; band++) {
        int lo = int(std::round(m_edgeHz[band]   * kWindow / m_sampleRate)); // First bin
        int hi = int(std::round(m_edgeHz[band+1] * kWindow / m_sampleRate)); // Last bin

        hi = std::min(hi, kWindow / 2);
        if (hi <= lo) lo = hi - 1;

        double bandPower = 0.0;

        // Sum the squared magnitudes of all bins in this band.
        for (int bin = lo; bin  < hi; bin++) {
            const double magnitude = double(m_mag[bin]);
            bandPower += magnitude * magnitude;
        }

        const double amp = std::sqrt(bandPower) / (kWindow * 0.25);

        // Track the recent peak amplitude for normalisation.
        if(receivingAudio) m_peak[band] = float(std::max(m_peak[band] * kPeakDecay, amp));

        // Smooth movement of the values, fade out when audio stops
        const float targetLevel = receivingAudio
                                 ? float(amp / std::max<double>(m_peak[band], kMinPeak))
                                 : 0.0f;
        const float smoothingRate = targetLevel > m_levels[band] ? kAttack : kRelease;
        m_levels[band] += (targetLevel - m_levels[band]) * smoothingRate;
    }

    // Calculate the overall RMS audio level.
    m_level = std::sqrt(sumSquares / kWindow);

    emit levelChanged();
    emit bandsChanged();
}