import QtQuick
import QtQuick3D

Node {
    id: root

    property int barCount: 128
    property real radius: 18.0
    property real barWidth: 0.3
    property real barThickness: 0.3
    property var bars: []

    // === Met à jour depuis le C++ ===
    function updateSpectrum(values) {
        if (!values || values.length === 0)
            return

        const n = Math.min(values.length, barCount)
        for (let i = 0; i < n; ++i) {
            const bar = bars[i]
            if (!bar)
                continue

            const v = values[i]
            const hue = 0.66 - v * 0.66
            const color = Qt.hsla(hue, 1.0, 0.5, 1.0)

            bar.scale.y = 0.3 + v * 8.0
            bar.materials[0].baseColor = color
            bar.materials[0].emissiveFactor = Qt.vector3d(v, v * 0.5, v * 0.2)
        }
    }

    // === Barres circulaires autour de Y ===
    Repeater3D {
        id: rep
        model: root.barCount

        delegate: Model {
            id: bar
            source: "#Cube"

            property real angle: (index / root.barCount) * 360
            property real rad: angle * Math.PI / 180.0

            // ✅ position en cercle horizontal autour de Y
            position: Qt.vector3d(
                Math.cos(rad) * root.radius,
                0,
                Math.sin(rad) * root.radius
            )

            // ✅ chaque barre tournée vers le centre
            eulerRotation: Qt.vector3d(0, -angle, 0)

            scale: Qt.vector3d(root.barWidth, 0.3, root.barThickness)

            materials: PrincipledMaterial {
                id: mat
                baseColor: Qt.hsla(index / root.barCount, 1.0, 0.5, 1.0)
                roughness: 0.25
                specularAmount: 0.6
                metalness: 0.1
                opacity: 0.9
                blendMode: PrincipledMaterial.SourceOver
                emissiveFactor: Qt.vector3d(0.05, 0.05, 0.05)
            }

            Behavior on scale.y {
                NumberAnimation { duration: 80; easing.type: Easing.OutQuad }
            }

            Component.onCompleted: root.bars[index] = bar
            Component.onDestruction: root.bars[index] = null
        }
    }

    // ✅ Anneau repère translucide (ne projette pas d’ombre)
    Model {
        id: ring
        source: "#Cylinder"
        scale: Qt.vector3d(root.radius, 0.02, root.radius)
        eulerRotation: Qt.vector3d(90, 0, 0)
        materials: PrincipledMaterial {
            baseColor: Qt.rgba(0.2, 0.6, 1.0, 0.15)
            blendMode: PrincipledMaterial.Screen
            lighting: PrincipledMaterial.NoLighting
            opacity: 0.25
        }
    }
}
