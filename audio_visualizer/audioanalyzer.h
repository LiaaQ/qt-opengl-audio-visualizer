#ifndef AUDIOANALYZER_H
#define AUDIOANALYZER_H

#include <QObject>
#include <QQmlEngine>
#include <QAudioBufferOutput>
#include <QAudioBuffer>

class AudioAnalyzer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAudioBufferOutput* output READ output CONSTANT FINAL)
    Q_PROPERTY(double level READ level NOTIFY levelChanged FINAL)

public:
    explicit AudioAnalyzer(QObject *parent = nullptr);
    QAudioBufferOutput *output() { return &m_output; }
    double level() const { return m_level; }

signals:
    void levelChanged();
private slots:
    void onBuffer(const QAudioBuffer &buffer);
private:
    QAudioBufferOutput m_output;
    double m_level = 0.0;
    bool m_formatLogged = false;
};

#endif // AUDIOANALYZER_H
