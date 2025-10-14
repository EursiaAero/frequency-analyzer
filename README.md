# frequency-analyzer
Qt 6.10 MSVC2022 frequency-analyzer 3D + 2D based on Timur Kristóf project
# 🌊 Audio Visualizer 3D — Qt 6.10

A real-time **audio spectrum visualizer** built with **Qt 6.10 / C++ / QML**, featuring:  
- a dynamic **FFT waterfall spectrum**,  
- synchronized **particle jets reacting to sound**,  
- and a **3D Earth scene** with HDR lighting and atmospheric glow effects.

---

## 🖼️ Screenshot

<img width="1907" height="1013" alt="image" src="https://github.com/user-attachments/assets/7386feb6-c069-4852-964c-5812ddb9c727" />


*Live 3D visualization combining FFT waterfall, particle jets, and rotating Earth.*

---

## 🚀 Features

### 🎵 Audio Analysis
- Real-time audio capture using `QAudioSource`.
- Fast **Radix-2 FFT** transform implemented in C++.
- Frequency band separation (bass, mid, treble).
- Adjustable sensitivity and smoothness.

### 🌈 Waterfall FFT Display
- Smooth rendering via `QQuickPaintedItem`.
- Color spectrum from blue → green → yellow → red.
- Adaptive normalization based on amplitude and sensitivity.
- Settings automatically saved/loaded from JSON.

### 💫 Particle Jet System (QML)
- Dual **polar jets** synchronized with amplitude peaks.
- Dynamic particle size, density, and color based on FFT.
- “Explosive” bursts on bass frequencies.
- Progressive dispersion and natural motion.

### 🌍 3D Earth Scene (QtQuick3D)
- Rotating Earth model (`.mesh`) with realistic lighting.
- HDR environment using `.hdr` skybox.
- **Additive halo and dynamic light color**.
- Camera with perspective projection.

### ⚙️ Built-in Control Panel
Adjustable live parameters:
- Particle size, lifespan, turbulence  
- FFT sensitivity, brightness, and bar count  
- Density, friction, and frequency stretch  
- All values saved persistently via JSON (C++ backend).

---

## 🧱 Technologies Used

| Component | Description |
|------------|-------------|
| **Qt 6.10** | Main framework |
| **QtQuick / QML** | Reactive UI and animation |
| **QtQuick3D** | 3D rendering, HDR lighting |
| **Qt Multimedia** | Audio input and analysis |
| **C++ (WaterfallItem)** | FFT computation and drawing |
| **JSON / QVariantMap** | Persistent user settings |

---

## 🧑‍💻 Based on the work of

This project is **inspired and extended from**  
👉 [**Timur Kristóf**]([https://github.com/timur](https://github.com/Venemo/frequency-analyzer)).

The FFT engine and rendering core (`WaterfallItem`) are derived from his elegant open-source implementation,  
adapted for:
- Qt 6.6+ / 6.10 compatibility,  
- 3D integration,  
- and reactive particle jet system.

Special thanks to **Timur Kristóf** for his foundational work 🙏.

---

## 🧩 Build Instructions

### Requirements
- Qt 6.10 (QtQuick, QtQuick3D, Qt Multimedia)
- CMake ≥ 3.21
- C++17 compatible compiler
- download https://www.spacespheremaps.com/wp-content/uploads/HDR_blue_nebulae-1.hdr and place it to obj\image

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run
```bash
./AudioVisualizer3D
```

---

## 📘 Credits

| Resource | Author / License |
|-----------|------------------|
| Earth mesh & textures | CC0 / NASA Visible Earth |
| HDR skybox “Blue Nebulae” | PolyHaven.com (CC0) |
| Star particle texture | Custom / Free use |
| Base FFT implementation | © Timur Kristóf |

---

## 🪶 License

Released under the **MIT License**,  
including original portions © Timur Kristóf.  
Please retain all original copyright notices.
