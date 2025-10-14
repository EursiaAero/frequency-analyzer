// waterfallitem.cpp — Port Qt 6.6+
// Original © Timur Kristóf
// Updated 2025 by ChatGPT

#include "waterfallitem.h"
#include "audiosampler.h"
#include "dft/radix2fft.h"

#include <QDebug>
#include <QCoreApplication>
#include <QPainter>
#include <QLinearGradient>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>

// === Constructeur ===
WaterfallItem::WaterfallItem(QQuickItem *parent)
    : QQuickPaintedItem(parent),
    _sampler(this),
    _dft(nullptr),
    _samplesUpdated(false),
    _sampleNumber(0),
    _sensitivity(0.05f),
    _amplitude(0.0f)

{
    connect(&_sampler, &AudioSampler::samplesCollected, this, &WaterfallItem::samplesCollected);
    connect(this, &QQuickItem::widthChanged, this, &WaterfallItem::sizeChanged);
    connect(this, &QQuickItem::heightChanged, this, &WaterfallItem::sizeChanged);

    setVisible(true);
    setFlag(QQuickItem::ItemHasContents, true);
    setOpacity(1.0);
    setAntialiasing(true);
    setAcceptedMouseButtons(Qt::NoButton);
    setFlag(QQuickItem::ItemClipsChildrenToShape, false);
    setFlag(QQuickItem::ItemHasContents, true);

    // 🔹 Transparence et rendu correct dans la scène QML
    setOpacity(1.0);
    setZ(999);

    setEnabled(true);
    setTextureSize(QSize(int(width()), int(height())));
_previousLevels.resize(_barCount, 0.0f);
    _sampleNumber = _sampler.samplesToWait();
    _image = QImage(int(width()), int(height()), QImage::Format_ARGB32_Premultiplied);
    _image.fill(Qt::transparent);
    _dft = new Radix2Fft(_sampleNumber);

    // === Prépare le gradient couleur (spectre) ===
    _gradientImg = QImage(500, 1, QImage::Format_ARGB32);
    _gradientImg.fill(Qt::transparent);

    QLinearGradient gradient(0, 0, _gradientImg.width(), 0);
    gradient.setColorAt(0.0, QColor(0, 191, 255));
    gradient.setColorAt(0.3, QColor(0, 255, 0));
    gradient.setColorAt(0.5, QColor(255, 255, 0));
    gradient.setColorAt(0.7, QColor(254, 163, 71));
    gradient.setColorAt(1.0, QColor(255, 0, 0)); // 🔥 Finir par rouge, pas noir


    QPainter gp(&_gradientImg);
    gp.fillRect(QRect(0, 0, _gradientImg.width(), 1), gradient);
    gp.end();

    update();
}

// === Taille modifiée ===
void WaterfallItem::sizeChanged() {
    _image = QImage(int(width()), int(height()), QImage::Format_ARGB32_Premultiplied);
    _image.fill(Qt::transparent);
    setTextureSize(QSize(int(width()), int(height())));
    _previousLevels.resize(_barCount, 0.0f);

    update();
}

void WaterfallItem::paint(QPainter *painter) {
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->drawImage(0, 0, _image); // remplace intégralement le contenu
}


// === Contrôle audio ===
bool WaterfallItem::start() {
    bool ok = _sampler.start();
    emit isStartedChanged();
    return ok;
}

void WaterfallItem::stop() {
    _sampler.stop();
    emit isStartedChanged();
}

bool WaterfallItem::isStarted() const {
    return _sampler.isStarted();
}

void WaterfallItem::clear() {
    _image.fill(Qt::transparent);
    update();
}

float WaterfallItem::amplitude() const { return _amplitude; }
float WaterfallItem::sensitivity() const { return _sensitivity; }

void WaterfallItem::setSensitivity(float value) {
    if (!qFuzzyCompare(_sensitivity, value)) {
        _sensitivity = value;
        emit sensitivityChanged();
    }
}
void WaterfallItem::samplesCollected(std::vector<float> *samples)
{
    std::vector<std::complex<float>> result = _dft->compute(*samples);
    delete samples;

    const int W = int(width());
    const int H = int(height());

    if ((int)_smoothLevels.size() < _barCount)
        _smoothLevels.resize(_barCount, 0.0f);
    if ((int)_previousLevels.size() < _barCount)
        _previousLevels.resize(_barCount, 0.0f);

    QImage img(W, H, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);

    // --- paramètres globaux (une seule fois) ---
    const float noiseFloor = 200.0f;
    const float perspective = 0.7f;

    // sensibilité + normalisation équilibrées
    const float baseRange = 5000.0f;
    const float sensFactor = std::max(_sensitivity, 0.0001f);
    const float adaptiveRange = baseRange * (1.0f / std::pow(sensFactor * 5.0f, 0.6f));

    // lissage
    const float baseAttack  = 0.25f;
    const float baseRelease = 0.08f;
    const float attack  = baseAttack  * (0.3f + _smoothness * 4.0f * 1.5f);
    const float release = baseRelease * (0.3f + _smoothness * 4.0f * 1.5f);

    // fréquence dominante (facultatif pour l’affichage)
    float maxVal = 0.0f; unsigned maxIndex = 0u;
    for (unsigned i = 0; i < result.size() / 2; ++i) {
        float mag = std::abs(result[i]);
        if (mag > maxVal) { maxVal = mag; maxIndex = i; }
    }
    _dominantFrequency = (_sampler.samplingFrequency() * float(maxIndex)) / float(result.size());
    emit dominantFrequencyChanged();

    for (int i = 0; i < _barCount; ++i) {
        // --- colonne pixel-alignée (réutilisée partout) ---
        const float colWf = W / float(_barCount);
        const int   colX  = int(std::floor(i * colWf));
        const int   colW  = int(std::ceil(colWf));

        // efface proprement la colonne précédente
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(QRect(colX, 0, colW, H), Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        unsigned idx = static_cast<unsigned>(std::pow(float(i) / (_barCount - 1), 3.0f) * (_sampleNumber / 2 - 1));
        if (idx >= result.size()) continue;

        // Correction fréquentielle de base
        float freqBoost = 0.5f + 1.2f * std::pow((float(i) / _barCount), 0.8f);
        float freqNorm  = 1.0f / std::sqrt(1.0f + 8.0f * (float(i) / _barCount));
        float magnitude = std::abs(result[idx]) * freqBoost * freqNorm;

        // Ensuite boost haute fréquence
        float freqRatio = float(i) / _barCount;
        float highBoost = 1.0f + 2.5f * std::pow(freqRatio, 2.0f);
        magnitude *= highBoost;
        magnitude *= highBoost;

        // Filtrage du bruit
        if (magnitude < noiseFloor * 0.2f)
            magnitude = 0.0f;

        // Log + normalisation
        float logAmp  = std::log10(1.0f + magnitude / (adaptiveRange * 0.6f));
        float target  = std::clamp(std::pow(logAmp, 0.75f), 0.0f, 1.0f);


        // lissage up/down
        float current = _smoothLevels[i];
        current += (target - current) * ((target > current) ? attack : release);
        _smoothLevels[i] = current;

        // géométrie barre
        const float barH   = current * (H * 0.9f);
        const float zDepth = 1.0f - (float(i) / _barCount) * (1.0f - perspective);
        const float barW   = colWf * zDepth;

        QRectF rect( i * colWf + (barW * 0.1f),
                    H - barH,
                    barW * 0.8f,
                    barH );

        // si la barre descend, effacer juste le haut résiduel avec la même colonne pixel
        if (_previousLevels[i] > current) {
            const int topY    = int(std::floor(H - (_previousLevels[i] * (H * 0.9f))));
            const int newTopY = int(std::floor(H - (current         * (H * 0.9f))));
            const int h       = std::max(0, newTopY - topY);
            if (h > 0) {
                p.setCompositionMode(QPainter::CompositionMode_Source);
                p.fillRect(QRect(colX, topY, colW, h), Qt::transparent);
                p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            }
        }

        // coupe minimale pour éviter les “mini ronds” et le coût CPU
        if (current < 0.02f || barH < 3.0f) {
            _previousLevels[i] = current;
            continue;
        }

        // couleur base & amplitude
        QColor base = gradientColor(current);
        _amplitude = current * 100.0f;

        // cylindre (limite du reflet pour éviter la saturation au rouge)
        const float reflectStrength = std::clamp(current * 0.9f, 0.0f, 0.85f);
        const QColor whiteLine = QColor::fromRgbF(1.0, 1.0, 1.0, reflectStrength * 0.6f);
        const QColor blendWhite = QColor::fromRgbF(
            base.redF()   * (1.0f - reflectStrength) + whiteLine.redF()   * reflectStrength,
            base.greenF() * (1.0f - reflectStrength) + whiteLine.greenF() * reflectStrength,
            base.blueF()  * (1.0f - reflectStrength) + whiteLine.blueF()  * reflectStrength,
            1.0f
            );

        QLinearGradient grad(rect.left(), rect.top(), rect.right(), rect.top());
        grad.setColorAt(0.00, QColor(0, 0, 0, 0));
        grad.setColorAt(0.35, base);
        grad.setColorAt(0.45, blendWhite);
        grad.setColorAt(0.55, base);
        grad.setColorAt(1.00, QColor(0, 0, 0, 0));

        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect, rect.width() / 2.5f, rect.width() / 2.5f);

        // tête (ellipse) avec seuil + alpha progressif
        QRectF topEllipse(rect.x(), rect.y() - rect.width() / 2.0f,
                          rect.width(), rect.width() * 0.8f);

        auto smoothstep = [](float e0, float e1, float x) {
            float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        };
        const float headA = smoothstep(0.08f, 0.20f, current);
        if (headA > 0.0f) {
            QRadialGradient ellipseGrad(topEllipse.center(), rect.width() / 2.0f);
            QColor c0 = base.lighter(130); c0.setAlphaF(headA);
            QColor c1 = base;              c1.setAlphaF(headA);
            QColor c2 = base.darker(170);  c2.setAlphaF(headA);
            ellipseGrad.setColorAt(0.00, c0);
            ellipseGrad.setColorAt(0.50, c1);
            ellipseGrad.setColorAt(1.00, c2);

            p.setBrush(ellipseGrad);
            const int penAlpha = int(220 * headA);
            if (penAlpha >= 8)
                p.setPen(QPen(QColor(255, 255, 255, penAlpha), 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            else
                p.setPen(Qt::NoPen);

            p.drawEllipse(topEllipse);
        }

        // mémoriser la hauteur courante (une seule fois, en fin d’itération)
        _previousLevels[i] = current;
    }

    p.end();
    _image = img;

    // spectre pour le QML (même normalisation que ci-dessus)
    _spectrum.clear();
    for (int i = 0; i < 128; ++i) {
        unsigned idx = i * result.size() / 128;
        float mag = std::abs(result[idx]);
        float norm = std::clamp(std::log10(1.0f + mag / adaptiveRange), 0.0f, 1.0f);
        _spectrum.append(norm);
    }
    emit spectrumChanged();
    update();
}

void WaterfallItem::setSmoothness(float value) {
    float clamped = std::clamp(value, 0.0f, 1.0f);
    if (!qFuzzyCompare(_smoothness, clamped)) {
        _smoothness = clamped;
        emit smoothnessChanged();
    }
}

void WaterfallItem::setBarrenumber(float value) {
    _barCount = int(std::max(1.0f, value));
    _smoothLevels.resize(_barCount, 0.0f);
    _previousLevels.resize(_barCount, 0.0f);
    emit barrenumberChanged();
}
// === Sauvegarde des paramètres ===
bool WaterfallItem::saveSettingsToJson(const QVariantMap &settings)
{
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dirPath);
    QString filePath = dirPath + "/settings.json";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "❌ Impossible d'écrire dans" << filePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(settings);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "[💾] Paramètres sauvegardés dans" << filePath;
    return true;
}

// === Chargement des paramètres ===
QVariantMap WaterfallItem::loadSettingsFromJson()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/settings.json";
    QFile file(filePath);

    QVariantMap defaults = {
        {"sensitivity", 0.5},
        {"brightness", 1.0},
        {"vx", 2.2},
        {"thickness", 0.04},
        {"density", 260},
        {"life", 2200},
        {"turb", 6.0},
        {"fric", 0.01},
        {"heightStretch", 1.0}
    };

    if (!file.exists()) {
        qWarning() << "⚠️ Aucun fichier JSON trouvé, valeurs par défaut utilisées.";
        return defaults;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "❌ Impossible d'ouvrir" << filePath;
        return defaults;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "⚠️ Fichier JSON invalide, restauration des valeurs par défaut.";
        return defaults;
    }

    QVariantMap loaded = doc.object().toVariantMap();
    qDebug() << "[✅] Paramètres chargés depuis" << filePath;
    return loaded;
}

QColor WaterfallItem::gradientColor(float norm) const
{
    int x = qBound(0, int(norm * (_gradientImg.width() - 1)), _gradientImg.width() - 1);
    return QColor::fromRgba(_gradientImg.pixel(x, 0));
}
