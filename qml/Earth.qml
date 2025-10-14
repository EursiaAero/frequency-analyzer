import QtQuick3D
import QtQuick
import QtQuick3D.Helpers

Node {
    id: root

    // 🔸 Expose le matériau interne au monde extérieur
    //     -> permet d'écrire earthMaterial.baseColor = ...
    property alias earthMaterial: mat
    property var basecolor: Qt.hsla(1.1, 1.0, 0.5, 1.0)

    // Le mesh converti par Balsam (chemin relatif au dossier de ce QML)
    Model {
        id: earthModel
        source: "../obj/meshes/node3.mesh"
        scale: Qt.vector3d(20, 20, 20)  // ⬅️ essaye entre 5 et 20 selon le modèle
        materials: [ mat ]

        // Tu peux ajuster center/scale ici si nécessaire
    }
    // Matériau par défaut (modifiable depuis l'extérieur via l'alias)
    PrincipledMaterial {
        id: mat
        property real hue: 0.0   // 🌈 teinte dynamique
        baseColor: basecolor//Qt.hsla(hue, 1.0, 0.5, 1.0)
        roughness: 0.1        // plus diffus
        specularAmount: 0.1   // reflets plus doux
        emissiveFactor: 10.0
        opacity: 1.0
        lighting: PrincipledMaterial.NoLighting
        //blendMode: PrincipledMaterial.Multiply
        // ☀️ Lumière dynamique couplée à la teinte

        // animation continue de la teinte
       /*NumberAnimation on hue {
            from: 0.0; to: 1.0
            duration: 10000
            loops: Animation.Infinite
            running: true
        }*/
    }
}
