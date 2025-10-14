// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//
// Frequency Analyzer — Qt6 port
// Original: Timur Kristóf, 2014
// Updated for Qt 6.6+ by ChatGPT (2025)

#include "audiosampler.h"

#include <QDebug>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSource>
#include <QTimer>
#include <QtEndian>
#include <QMetaMethod>   // ✅ nécessaire pour isSignalConnected() et fromSignal()


AudioSampler::AudioSampler(QObject *parent)
    : QIODevice(parent)
{
    _started = false;
    _samplesToWait = 4096;
    _samples = new std::vector<float>();
    _audioSource = nullptr;
}

AudioSampler::~AudioSampler() {
    stop();
    delete _samples;
}

void AudioSampler::elapsed() {
    if (isSignalConnected(QMetaMethod::fromSignal(&AudioSampler::samplesCollected))) {
        std::vector<float> *samplesCopy = new std::vector<float>(*_samples);
        emit samplesCollected(samplesCopy);
    }
    _samples->clear();
}

bool AudioSampler::start() {
    if (_started)
        return true;

    // --- Configuration du format audio ---
    QAudioFormat format;
    format.setSampleRate(int(samplingFrequency()));
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    // --- Liste des périphériques disponibles ---
    QList<QAudioDevice> inputs = QMediaDevices::audioInputs();
    if (inputs.isEmpty()) {
        qWarning() << "[AudioSampler] Aucun périphérique d’entrée audio détecté.";
        return false;
    }

    // --- Sélection automatique du périphérique ---
    QAudioDevice chosenDevice = QMediaDevices::defaultAudioInput();

    for (const QAudioDevice &dev : inputs) {
        QString name = dev.description().toLower();

        if (name.contains("cable output") || name.contains("vb-audio")) {
            chosenDevice = dev;
            break;
        }

        if (name.contains("stereo mix") ||
            name.contains("mixage stéréo") ||
            name.contains("what u hear") ||
            name.contains("loopback")) {
            chosenDevice = dev;
            break;
        }
    }

    _device = chosenDevice;

    if (!_device.isFormatSupported(format)) {
        qWarning() << "[AudioSampler] Format demandé non supporté, utilisation du format le plus proche.";
        format = _device.preferredFormat();
    }

    qDebug() << "[AudioSampler] Utilisation du périphérique:" << _device.description()
             << "=>" << format.sampleRate() << "Hz,"
             << format.channelCount() << "ch,"
             << format.bytesPerSample()*8 << "bits";

    // --- Création du flux ---
    _audioSource = new QAudioSource(_device, format, this);
    _audioSource->setBufferSize(_samplesToWait * sizeof(qint16));
    _audioSource->setVolume(1.0);

    // --- Démarrage ---
    this->open(QIODevice::WriteOnly);
    _audioSource->start(this);

    if (_audioSource->error() != QAudio::NoError) {
        qWarning() << "[AudioSampler] Impossible de démarrer l’entrée audio:" << _audioSource->error();
        return false;
    }

    _started = true;
    qDebug() << "[AudioSampler] Capture démarrée sur" << _device.description();
    return true;
}

void AudioSampler::stop() {
    if (!_started)
        return;

    if (_audioSource) {
        _audioSource->stop();
        _audioSource->deleteLater();
        _audioSource = nullptr;
    }

    _samples->clear();
    this->close();

    _started = false;
    qDebug() << "[AudioSampler] Capture arrêtée.";
}

bool AudioSampler::isStarted() const {
    return _started;
}

quint32 AudioSampler::samplingFrequency() const {
    return 44100;
}

quint32 AudioSampler::samplesToWait() const {
    return _samplesToWait;
}

void AudioSampler::setSamplesToWait(quint32 value) {
    _samplesToWait = value;
}

qint64 AudioSampler::readData(char *data, qint64 maxlen) {
    Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return 0;
}

qint64 AudioSampler::writeData(const char *data, qint64 len) {
    if (!_started || !_audioSource)
        return 0;

    const qint16 *samples = reinterpret_cast<const qint16*>(data);
    qint64 sampleCount = len / 2;

    for (qint64 i = 0; i < sampleCount; ++i) {
        _samples->push_back(static_cast<float>(samples[i]));
        if (_samples->size() >= _samplesToWait)
            elapsed();
    }

    return len;
}
