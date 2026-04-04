# 9key_makro_master – Project Notes

## Overview
Custom 3x3 macro pad with:
- RP2040 (QMK)
- WS2812 LED strip (1 LED per key, chained)
- OLED display (SSD1306)
- Encoder with push button
- Selector button

Goal: expressive but controlled RGB system + fast layer workflow

---

## Hardware

### Matrix
- 3x3 layout (9 keys)
- standard row/col scan

### LEDs
- WS2812 / NeoPixel
- physically one strip, logically per-key
- each key has exactly one LED
- mapping required (strip is serpentine)

Example mapping (to verify):
[0,1,2]
[5,4,3]
[6,7,8]

→ must match physical routing

---

## Layers

Total (current):
- BASE
- WINDOW
- TXT
- RGB
- SELECT

Future:
- expand to 9 layers (1 per key)

---

## Layer Behavior

### BASE
- calm dual-color breathing
- two colors alternating or blending

### WINDOW
- blue dominant
- "electric" feel:
  - flashing
  - scanner
  - sharp transitions

### TXT
- green
- calm / minimal
- slow fade or soft breathing

### RGB
- intentionally flashy
- rainbow / hue shift / dynamic

### SELECT
- special mode
- used to choose active layer
- LEDs represent available layers

---

## RGB System

### Modes

#### 1. WILD MODE
- full animations per layer
- expressive, dynamic

#### 2. QUIET MODE
- minimal
- only active layer key is lit
- soft breathing only

Toggle between modes required

---

## SELECT Mode Behavior

### Enter
- via encoder button (momentary)

### While active
- system cycles through layers automatically
- interval: ~500ms

### LED behavior
- currently selected layer = highlighted
- highlight moves across keys

### Manual control
- encoder rotation:
  - next / previous layer

### Exit
- release encoder button
- selected layer becomes active

---

## OLED Behavior

### Normal mode
- show:
  - current layer
  - last key pressed
  - key position (row/col)

### SELECT mode
- different display

Show:
- key index (1–9)
- layer name

Example:
"3  TXT"

---

## Key Index Mapping

Keys numbered left → right, top → bottom:

[1][2][3]
[4][5][6]
[7][8][9]

Mapping must match LED index mapping

---

## Required Features

- per-key RGB control (not global only)
- LED index mapping table
- layer-based animation system
- selector animation (timed + encoder)
- dual profile system (WILD / QUIET)
- OLED integration tied to state
- clean separation of logic:
  - input
  - rendering (RGB)
  - display (OLED)

---

## Known Issues

- GitHub Actions:
  - rework keymap failing
  - artifacts not always uploaded on failure

- QMK:
  - currently using RGBLIGHT (limited)
  - may need custom per-key control via rgblight_sethsv_at()

- keymap structure:
  - SELECT layer still not clean
  - encoder logic partially inconsistent

---

## Architecture Goals

Refactor into:

- rgb.c / rgb logic section
- selector system (state machine)
- oled display module
- clear layer abstraction

Avoid:
- hardcoded behavior inside keymap
- duplicated logic per layer

---

## Next Steps

1. Fix keymap compile errors (rework)
2. implement LED mapping array
3. build base RGB system (per-key control)
4. implement SELECT mode animation + timing
5. integrate encoder control into selector
6. implement QUIET mode
7. OLED: add SELECT mode UI
8. expand to 9 layers

---

## Notes for AI

- LEDs are individually addressable
- treat system like RGB matrix (even if strip)
- prefer explicit logic over QMK presets
- animations should be lightweight (RP2040 ok but avoid heavy loops)
- keep timing non-blocking
