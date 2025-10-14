import QtQuick
import QtQuick3D

Node {
    id: node
    // Nodes:
    Node {
        id: waterSphere_obj
        objectName: "waterSphere.obj"
        Model {
            id: sphere
            objectName: "Sphere"
            source: "../obj/meshes/sphere.mesh"
            materials: [ waterMat ]
        }
    }

    // Resources
    CustomMaterial {
        id: waterMat
        shadingMode: CustomMaterial.Shaded
        fragmentShader: "qrc:/shaders/water.frag"

        property real waterTime: 0.0

        // ✅ Texture de normales
        property Texture waterNormalTex: Texture {
            source: "qrc:/obj/textures/Water_normal3.png"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }

        // ✅ Texture diffuse (couleur de base)
        property Texture waterDiffuseTex: Texture {
            source: "qrc:/obj/textures/Water3.jpg"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }

        NumberAnimation on waterTime {
            from: 0; to: 6.283
            duration: 6000
            loops: Animation.Infinite
        }
    }








    // Animations:

}
