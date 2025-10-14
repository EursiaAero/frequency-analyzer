import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers
import Qt5Compat.GraphicalEffects
import Qt.labs.platform
import QtQuick.Particles
import QtDataVisualization
import QtQml.Models
import hu.timur 1.0
import "qrc:/qml" // 🔹 dossier où se trouve *.qml et
import "qrc:/obj" // 🔹 dossier où se trouve *.meshes/node3.mesh

ApplicationWindow {
    visible: true
    visibility: Window.Maximized
    minimumHeight: 600
    minimumWidth: 800

    property var start
    property bool readyForSave: false
    // valeur simulée de la fréquence (0–1)
    property real amplitudeSlideValue: 0.6   // 0..1 → couleur
    property real frequency: 0.5   // 0..1 → vitesse
    property bool lastWasPulse: false
    property real hue: 0.33   // vert
    property real baseSize: 20
    property real turbulence: 10
    property real initialAngle:100
    property real particuleSpan:50
    property real particuleSize:5

    Item {
        id: root
        anchors.fill: parent

        // === CONFIGURATION ===
        QtObject {
            id: cfg
            property real initialAngle: 2.2
            property real thickness: 0.04
            property int density: 260
            property int life: 2200
            property real turb: 6.0
            property real fric: 0.01
            property real brightness: 1.0
            property real heightStretch: 1.0
            property real sensitivity: 0.5
            property real smoothness: 0.5
            property int barrenumbers: 150
            property real particuleSpan:50
            property real particuleSize:5
        }

        //  Rectangle { anchors.fill: parent; color: "black"; opacity:0}

        // === SCÈNE TERRE ===
        View3D {
            id: mainScene
            anchors.fill: parent
            environment: ExtendedSceneEnvironment {
                // 🟦 Couleur de fond (fallback)
                backgroundMode: ExtendedSceneEnvironment.SkyBox
                clearColor: "black"

                // 🟨 Ta texture HDR comme Skybox et LightProbe
                lightProbe: Texture {
                    source: "qrc:/obj/image/HDR_blue_nebulae-1.hdr"
                    mappingMode: Texture.LightProbe
                }

                // 🧠 Effets supplémentaires (si souhaités)
                exposure: 1.0
                tonemapMode: ExtendedSceneEnvironment.TonemapModeFilmic

                probeExposure: 0.6

                // --- effets modernes dispos avec ExtendedSceneEnvironment ---
                glowEnabled: true
                glowStrength: 0.4
                glowBloom: 0.2
                glowHDRScale: 1.0


                temporalAAEnabled: true
                antialiasingMode: ExtendedSceneEnvironment.MSAA
                antialiasingQuality: ExtendedSceneEnvironment.High
            }
            // 🌌 Lumière ambiante douce bleutée
            DirectionalLight {
                id: ambientFill
                eulerRotation: Qt.vector3d(90, 0, 0)   // vient d’en haut
                brightness: 0.3
                castsShadow: false
                color: Qt.rgba(0.2, 0.3, 0.6, 1.0)
            }

            DirectionalLight {
                id: sunLight
                // ☀️ Orientation : soleil à gauche, légèrement en hauteur
                eulerRotation: Qt.vector3d(-25, -90, 0)

                // 💡 Intensité et teinte chaude
                brightness: 2.0
                color: Qt.rgba(1.0, 0.92, 0.7, 1.0)  // ton doré / solaire

                castsShadow: true

                // --- Qualité des ombres ---
                shadowMapQuality: DirectionalLight.ShadowMapQualityVeryHigh
                shadowMapFar: 300
                shadowBias: 0.0005
                shadowFactor: 1.0

                // --- Halo solaire (effet visuel simulé) ---
                Rectangle {
                    id: sunGlow
                    anchors.centerIn: parent
                    width: 500; height: 500
                    color: Qt.rgba(1.0, 0.8, 0.4, 0.3)
                    radius: width / 2
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: RadialGradient {
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: Qt.rgba(1.0, 0.9, 0.6, 0.6) }
                                GradientStop { position: 0.4; color: Qt.rgba(1.0, 0.8, 0.4, 0.25) }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                        }
                    }
                    visible: true
                    opacity: 0.8
                    Behavior on opacity { NumberAnimation { duration: 2000; easing.type: Easing.InOutQuad } }
                }
            }

            PerspectiveCamera {
                id: mainCamera
                position: Qt.vector3d(0, -2, 30)
            }

            // 🌍 Terre (importée via balsam)
            Earth {
                id: earth
                position: Qt.vector3d(0, 0, 0)
                SequentialAnimation on eulerRotation.y {
                    loops: Animation.Infinite
                    NumberAnimation { from: 0; to: 360; duration: 40000 }
                }
            }

            WaterSphere {
                id: glassSphere
                scale: Qt.vector3d(19.91, 19.91, 19.91)
                position: earth.position//Qt.vector3d(0.015, -0.01, -0.03)
                //eulerRotation:earth.eulerRotation
                //Component.onCompleted:  console.log("Shader uniforms:", waterMat.properties)
            }

        }

        ParticleSystem {
            id: ps
            anchors.fill: parent

            // === Peinture des particules ===
            ImageParticle {
                id: particles2
                groups: ["jet"]
                source: "qrc:/particleresources/star.png"
                colorVariation: 0.2
                alpha: 0.9
                entryEffect: ImageParticle.None
            }

            // === Émetteur gauche (vers la droite) ===
            Emitter {
                id: emitterLeft
                system: ps
                group: "jet"
                x: parent.width*0.62
                y:parent.height*0.4
                width: 50
                height: 40
                lifeSpan: 1000
                size: 20
                sizeVariation: 10
                velocity: AngleDirection {
                    angle: 0
                    angleVariation: cfg.initialAngle
                    magnitude: 500
                }
            }

            // === Émetteur droit (vers la gauche) ===
            Emitter {
                id: emitterRight
                system: ps
                group: "jet"
                x: parent.width*0.35
                y:parent.height*0.4
                width: 50
                height: 40
                lifeSpan: 1000
                size: 20
                sizeVariation: 10
                velocity: AngleDirection {
                    angle: 180
                    angleVariation: cfg.initialAngle
                    magnitude: 500
                }
            }

            // === Affecteur : dispersion progressive ===
            Affector {
                system: ps
                groups: ["jet"]
                anchors.fill: parent
                acceleration: AngleDirection {
                    angleVariation: 20 + 60 * particuleSize
                    magnitude: 20 + 40 * particuleSize
                }
            }

            // === Dynamique audio ===
            Connections {
                target: plot
                function onSpectrumChanged() {
                    if (!plot.spectrum || plot.spectrum.length < 32)
                        return

                    // === Analyse spectrale ===
                    const bass  = plot.spectrum[2]   // graves
                    const mid   = plot.spectrum[10]  // médiums
                    const high  = plot.spectrum[24]  // aigus
                    var amplitude   = plot.amplitude / 100.0

                    // === Couleur selon le spectre (bleu → rouge) ===
                    const weighted = (bass * 0.5 + mid * 0.7 + high * 1.0)
                    hue = 0.55 - Math.min(0.55, weighted * 0.6)

                    // === Réactivité brutale (sans lissage) ===
                    const energy = (bass * 1.3 + mid * 0.8 + high * 0.5)
                    const impulse = Math.pow(amplitude, 2.4)

                    // 💥 Émission très brutale, liée aux basses
                    const baseEmit = 80 + energy * 6000 * impulse
                    emitterLeft.emitRate  = baseEmit
                    emitterRight.emitRate = baseEmit

                    // 💨 Vitesse et durée courtes = effet percussif
                    const jetLife  = 600 + 500 * amplitude
                    const jetSpeed = 400 + 1000 * impulse
                    emitterLeft.lifeSpan  = jetLife*particuleSpan
                    emitterRight.lifeSpan = emitterLeft.lifeSpan
                    emitterLeft.velocity.magnitude  = jetSpeed
                    emitterRight.velocity.magnitude = emitterLeft.velocity.magnitude

                    // 🌈 Taille + couleur réactive
                    const bassBoost = Math.pow(bass * 2, 1.2)
                    const newSize = 10 + 50 * impulse + 20 * bassBoost
                    emitterLeft.size = newSize*particuleSize
                    emitterRight.size = emitterLeft.size
                    particles2.color = Qt.hsla(hue, 1.0, 0.55 + bassBoost*amplitude * 0.4, 1.0)
                    earth.basecolor = Qt.hsla(hue, 1.0, 0.55 + bassBoost*amplitude * 0.4, 1.0)
                }
            }
        }



        // === WATERFALL FFT ===
        Item {
            anchors.bottom:parent.bottom
            width:parent.width
            height:parent.height/3
            Waterfall {
                id: plot
                property real lastAmp: 0
                property int lastWasPulse: 0
                barrenumbers:168
                smoothness: 0.4  // plus petit = plus fluide
                sensitivity: 0.05
                anchors.fill: parent
                visible: true
                opacity: 0.8  // un peu de transparence globale
            }
        }

        // === PANNEAU DE RÉGLAGES ===
        Rectangle {
            id: settingsPanel
            width: parent.width / 5
            height: parent.height
            anchors.top: parent.top
            anchors.right: parent.right
            color: "#101010"
            opacity: 0.0
            visible: true
            border.color: "#444"
            border.width: 1

            property real offset: width
            transform: Translate { x: settingsPanel.offset }
            Behavior on offset { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
            Behavior on opacity { NumberAnimation { duration: 300 } }

            property bool mouseInside: false
            property real hideDelay: 5000

            Timer {
                id: hideTimer
                interval: settingsPanel.hideDelay
                repeat: false
                onTriggered: {
                    settingsPanel.offset = settingsPanel.width
                    settingsPanel.opacity = 0.0
                }
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
                acceptedButtons: Qt.AllButtons
                onEntered: {
                    settingsPanel.mouseInside = true
                    hideTimer.stop()
                    console.log("panel enter")
                }
                onExited: {
                    settingsPanel.mouseInside = false
                    hideTimer.restart()
                    console.log("panel exit")
                }
            }

            Column {
                anchors.centerIn: parent
                spacing: 10
                width: parent.width


                Text { text: "⚙️ Réglages Particules"; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter }

                Text { text: "Particules Size : " + particuleSize.toFixed(2); color: "lightgray" }
                Slider { id:particuleSizeSlider;from: 0; to: 10;width: settingsPanel.width; value: cfg.particuleSize; onValueChanged: {particuleSize = value; readyForSave ? settingsManager.saveAll() : ""}}

                Text { text: "Particule Span : " + particuleSpan.toFixed(2); color: "lightgray" }
                Slider { id:particuleSpanSlider;from: 0; to: 5;width: settingsPanel.width; value: cfg.particuleSpan; onValueChanged: {particuleSpan = value; readyForSave ? settingsManager.saveAll() : ""}}

                Text { text: "Barre Numbers : " + barreNumbersSlider.value; color: "lightgray" }
                Slider { id:barreNumbersSlider;from: 50; to: 200;stepSize: 10;width: settingsPanel.width; value: cfg.barrenumbers; onValueChanged: {plot.barrenumbers = value; readyForSave ? settingsManager.saveAll() : ""}}

                Text { text: "Sensibilité (Waterfall)"; color: "lightgray" }
                Slider { id: sensitivitySlider; width: settingsPanel.width; from: 0; to: 0.5; stepSize: 0.0001; value: cfg.sensitivity; onValueChanged: {plot.sensitivity = 0.0001 + value * (0.04 - 0.0001); readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Luminosité"; color: "lightgray" }
                Slider { id: brightnessSlider; width: settingsPanel.width; from: 0.2; to: 2.0; stepSize: 0.01; value: cfg.brightness; onValueChanged: {cfg.brightness = value; readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Initial Angle"; color: "lightgray" }
                Slider { id: initialAngleSlider; width: settingsPanel.width; from: 0; to: 10; stepSize: 0.1; value: cfg.initialAngle; onValueChanged: {cfg.initialAngle = value; readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Épaisseur"; color: "lightgray" }
                Slider { id: thicknessSlider; width: settingsPanel.width; from: 0.01; to: 0.3; stepSize: 0.01; value: cfg.thickness; onValueChanged: {cfg.thickness = value; readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Densité"; color: "lightgray" }
                Slider { id: densitySlider; width: settingsPanel.width; from: 50; to: 400; stepSize: 10; value: cfg.density; onValueChanged: {cfg.density = value; readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Durée de vie"; color: "lightgray" }
                Slider { id: lifeSlider; width: settingsPanel.width; from: 500; to: 4000; stepSize: 100; value: cfg.life; onValueChanged: {cfg.life = value; readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Turbulence"; color: "lightgray" }
                Slider { id: turbSlider; width: settingsPanel.width; from: 0; to: 20; stepSize: 1; value: cfg.turb; onValueChanged: {cfg.turb = value; readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Friction"; color: "lightgray" }
                Slider { id: fricSlider; width: settingsPanel.width; from: 0.0; to: 0.05; stepSize: 0.001; value: cfg.fric; onValueChanged: {cfg.fric = value; readyForSave ? settingsManager.saveAll() : ""} }

                Text { text: "Hauteur des particules"; color: "lightgray" }
                Slider { id: heightStretchSlider; width: settingsPanel.width; from: 0.5; to: 3.0; stepSize: 0.1; value: cfg.heightStretch; onValueChanged: {cfg.heightStretch = value; readyForSave ? settingsManager.saveAll() : ""} }
            }


        }
        Rectangle {
            id: edgeTrigger
            width: 8; height: parent.height; color: "transparent"
            anchors.right: parent.right
            MouseArea {
                anchors.fill: parent; hoverEnabled: true
                onEntered: {
                    settingsPanel.offset = parent.width - 5
                    settingsPanel.opacity = 0.95
                    hideTimer.stop()
                }
            }
        }


        QtObject {
            id: settingsManager
            function saveAll() {
                var obj = {
                    barrenumbers:barreNumbersSlider.value,
                    sensitivity: sensitivitySlider.value,
                    brightness: brightnessSlider.value,
                    initialAngle: initialAngleSlider.value,
                    thickness: thicknessSlider.value,
                    density: densitySlider.value,
                    life: lifeSlider.value,
                    turb: turbSlider.value,
                    fric: fricSlider.value,
                    heightStretch: heightStretchSlider.value,
                    particuleSize:particuleSizeSlider.value,
                    particuleSpan:particuleSpanSlider.value
                }
                plot.saveSettingsToJson(obj)
                console.log("[✅] Sliders sauvegardés via C++ ->", JSON.stringify(obj))
            }
            function loadAll() {
                var obj = plot.loadSettingsFromJson()
                //console.log("[✅] Sliders rechargés via C++ ->", JSON.stringify(obj))
                barreNumbersSlider.value=obj.barrenumbers
                sensitivitySlider.value = obj.sensitivity
                brightnessSlider.value = obj.brightness
                initialAngleSlider.value = obj.initialAngle
                thicknessSlider.value = obj.thickness
                densitySlider.value = obj.density
                lifeSlider.value = obj.life
                turbSlider.value = obj.turb
                fricSlider.value = obj.fric
                heightStretchSlider.value = obj.heightStretch
                particuleSizeSlider.value=obj.particuleSize
                particuleSpanSlider.value=obj.particuleSpan
                return true
            }
        }
    }
    Component.onCompleted: {
        start=plot.start()
        readyForSave = settingsManager.loadAll()
        settingsPanel.offset = root.width - 5
        settingsPanel.opacity = 0.95
        console.log("Qt RHI backend:", Qt.application.rhiBackend)
        var f = Qt.resolvedUrl("qrc:/shaders/water.frag")
        var xhr = new XMLHttpRequest()
        xhr.open("GET", f, false)
        xhr.send()
        //   console.log("[DEBUG] Shader chargé depuis:", f, "taille:", xhr.responseText.length)
        //   console.log("[DEBUG] Contenu du shader:", xhr.responseText.slice(0, 200))

    }

    onClosing: settingsManager.saveAll()
}
