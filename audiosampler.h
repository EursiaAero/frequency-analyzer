#pragma once

#include <QIODevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QAudioDevice>
#include <QObject>
#include <vector>

// === Classe AudioSampler (Qt6) ===
// Capture du son depuis le périphérique d’entrée (loopback / VB-Audio / Mixage stéréo)
// Émet périodiquement un signal "samplesCollected" contenant un bloc d’échantillons.

class AudioSampler : public QIODevice
{
    Q_OBJECT

public:
    explicit AudioSampler(QObject *parent = nullptr);
    ~AudioSampler() override;

    bool start();               // Démarre la capture
    void stop();                // Arrête la capture
    bool isStarted() const;

    quint32 samplingFrequency() const;
    quint32 samplesToWait() const;
    void setSamplesToWait(quint32 value);

signals:
    void samplesCollected(std::vector<float> *samples);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private slots:
    void elapsed();

private:
    bool _started;
    quint32 _samplesToWait;

    std::vector<float> *_samples;

    QAudioFormat _format;
    QAudioDevice _device;
    QAudioSource *_audioSource = nullptr;
};
