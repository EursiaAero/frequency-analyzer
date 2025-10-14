#pragma once

#include <QQuickPaintedItem>
#include <QImage>
#include <QVariantMap>
#include <vector>

#include "audiosampler.h"
#include "dft/radix2fft.h"

// === Classe WaterfallItem (Qt6) ===
// Affiche la transformation FFT des échantillons audio
// Génère les couleurs et hauteurs des barres selon le spectre sonore.

class WaterfallItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool isStarted READ isStarted NOTIFY isStartedChanged)
    Q_PROPERTY(float amplitude READ amplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(float sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)
    Q_PROPERTY(QVariantList spectrum READ spectrum NOTIFY spectrumChanged)
    Q_PROPERTY(float dominantFrequency READ dominantFrequency NOTIFY dominantFrequencyChanged)
    Q_PROPERTY(float smoothness READ smoothness WRITE setSmoothness NOTIFY smoothnessChanged) // ⬅️
    Q_PROPERTY(float barrenumbers READ barrenumber WRITE setBarrenumber NOTIFY barrenumberChanged) // ⬅️

public:
    explicit WaterfallItem(QQuickItem *parent = nullptr);
    ~WaterfallItem() override = default;

    QImage _gradientImg;
    void paint(QPainter *painter) override;
std::vector<float> _previousLevels;
    float _dominantFrequency;
    QColor base;
    // Contrôle du système audio
    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void clear();

    bool isStarted() const;
    float amplitude() const;
    float sensitivity() const;
    void setSensitivity(float value);
    QColor gradientColor(float norm) const;

    // Nouvelle propriété QML : contrôle du lissage
    float smoothness() const { return _smoothness; }                // ⬅️
    void setSmoothness(float value);                                 // ⬅️
    float barrenumber() const { return _barCount; }                // ⬅️
    void setBarrenumber(float value);

    // Sauvegarde / chargement des paramètres JSON
    Q_INVOKABLE bool saveSettingsToJson(const QVariantMap &settings);
    Q_INVOKABLE QVariantMap loadSettingsFromJson();

    QVariantList spectrum() const { return _spectrum; }
    float dominantFrequency() const { return _dominantFrequency; }

    std::vector<float> _smoothLevels;

signals:
    void isStartedChanged();
    void amplitudeChanged();
    void sensitivityChanged();
    void spectrumChanged();
    void dominantFrequencyChanged();
    void smoothnessChanged(); // ⬅️
    void barrenumberChanged();

private slots:
    void samplesCollected(std::vector<float> *samples);
    void sizeChanged();

private:
    AudioSampler _sampler;
    Radix2Fft *_dft;

    QImage _image;
    bool _samplesUpdated;
    unsigned _sampleNumber;
    int _barCount;
    float _sensitivity;
    float _amplitude;
    float _smoothness = 0.6f; // ⬅️ (0.0 = ultra fluide / 1.0 = très réactif)
    QVariantList _spectrum;
};
