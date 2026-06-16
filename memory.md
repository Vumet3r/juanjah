# Juanjah — Multitap Delay

_Colores jamaicanos: oro (#fcca46), verde (#2d6a4f / #40916c), rojo (#d62828)_

Creado: 2026-06-16

## Proyecto

- **Ruta:** `/Users/javierbleda/Personal/Personal projects/vst-plugin/`
- **Build:** `build/` (generado por CMake)
- **Código fuente:** `source/`

### Estructura del código

```
vst-plugin/
├── CMakeLists.txt
├── source/
│   ├── PluginProcessor.h/.cpp   — Procesador (parámetros + DSP loop)
│   ├── PluginEditor.h/.cpp      — UI con sliders rotatorios
│   ├── DelayLine.h              — Línea de retardo circular (interpolada)
│   ├── LFO.h                    — Oscilador senoidal
│   └── Distortion.h             — Saturación (tanh soft-clip)
├── lib/JUCE/                    — JUCE 8.0.13 (tag, detached HEAD)
├── memory.md
└── .gitignore
```

### Parámetros del plugin

| Grupo | Parámetro | Rango | Default |
|-------|-----------|-------|---------|
| Mix | Dry/Wet | 0-1 | 0.3 |
| Tap Tempo | BPM + botón TAP | 40-200 | 120 |
| Tap 1-3 | Sync (ON/OFF) / Note div (1/4..1/32) | — | Sync ON, 1/4..1/8 |
| Tap 1-3 | Time ms / Level / Feedback | 10-2000ms / 0-1 / 0-0.95 | 100..400ms / 0.5..0.2 / 0.2 |
| Modulación | Rate / Depth | 0-10Hz / 0-1 | 0.5Hz / 0.15 |
| Distorsión | Drive / Blend | 1-15 (skewed) / 0-1 | 3.0 / 0.0 |

### Notas de diseño

- **Tap Tempo**: BPM se ajusta tocando el botón TAP; taps en Sync convierten Note → delay time automáticamente
- **Distorsión**: Drive con skew 0.4 para mejor resolución en valores bajos (más manejable)
- **UI**: Knobs custom pintados (arcos + needle), secciones coloreadas (teal/amber/purple/red)

## Dependencias del sistema

## Dependencias del sistema

### Background image

Para poner una imagen de fondo:
1. Coloca `bg.png` en `Resources/`
2. Recompila: `cmake --build build`
3. La imagen se embeberá automáticamente en el plugin vía `juce_add_binary_data`
4. En `PluginEditor.cpp`, los controles se posicionan en `resized()` con coordenadas absolutas
5. Para ajustar la posición de cada control al diseño de la imagen, edita los `setBounds()` en `resized()` — cada knob/botón tiene su propia línea con `x, y, w, h`

### Para crear el fondo

1. Pídele a una IA o diseñador una imagen de **820×500 px**
2. Marca en la imagen dónde quieres cada control (Mix, Tap 1/2/3, Sync, Note, Time, Level, Fb, Mod Rate/Depth, Dist Drive/Blend)
3. Dime las coordenadas (x,y) de cada uno y las ajustamos en el código

### CMake 4.3.3

- Instalado via: `brew install cmake`
- Binario: `/opt/homebrew/bin/cmake`
- Fórmula: `/opt/homebrew/Cellar/cmake/4.3.3` (4.041 ficheros, 64.9MB)
- Para desinstalar: `brew uninstall cmake`

### JUCE 8.0.13

- **No** instalado via brew. Clonado localmente en el proyecto:
  - `lib/JUCE/` (detached HEAD en tag 8.0.13)
  - Repo: https://github.com/juce-framework/JUCE.git
- Para actualizar: `cd lib/JUCE && git fetch --tags && git checkout <nuevo-tag>`
- Para limpiar: borrar `lib/JUCE/` y reconfigurar

### Xcode Command Line Tools

- Ruta: `/Library/Developer/CommandLineTools`
- Ya presente en el sistema (AppleClang 21.0.0)

## Destino de los plugins compilados

| Formato | Ruta de instalación |
|---------|---------------------|
| VST3 | `~/Library/Audio/Plug-Ins/VST3/Juanjah.vst3` |
| AU (Audio Unit) | `~/Library/Audio/Plug-Ins/Components/Juanjah.component` |

*(Instalación automática vía `COPY_PLUGIN_AFTER_BUILD`)*

## Targets de build

```bash
cmake --build build          # Compila todo
cmake --build build --target Juanjah_VST3   # Solo VST3
cmake --build build --target Juanjah_AU     # Solo AU
```

## Para limpiar todo

```bash
brew uninstall cmake
rm -rf "/Users/javierbleda/Personal/Personal projects/vst-plugin/build"
rm -rf "/Users/javierbleda/Personal/Personal projects/vst-plugin/lib/JUCE"
rm -rf ~/Library/Audio/Plug-Ins/VST3/Juanjah.vst3
rm -rf ~/Library/Audio/Plug-Ins/Components/Juanjah.component
```

## Reconstruir desde cero

```bash
rm -rf build && cmake -B build -S . && cmake --build build
```
