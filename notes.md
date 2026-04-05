# 9key_makro_master - Project Notes

## Goal
Build a 3x3 RP2040 macro pad with a selector workflow, OLED feedback, and RGB behavior that can evolve from simple layer colors into per-key animated visuals.

## Current Decision

- Use `keymaps/reworked` as the canonical implementation target going forward
- Use `keymaps/simple` only as a fallback reference for known simpler behavior
- Do not spend time expanding the other keymaps unless they are needed for comparison or recovery

## Confirmed Hardware

- RP2040/QMK keyboard
- 3x3 matrix
- 1 WS2812 LED per key, 9 total
- OLED display on I2C, configured as SSD1306
- Rotary encoder on GP9/GP10
- Encoder push button on GP8
- Separate selector button on GP12

## What Happened So Far

The repo currently contains multiple parallel keymap implementations rather than one settled design.

### 1. Simple keymap
- Uses a dedicated selector button on GP12 to hold the SELECT layer
- Has a compact layer set: BASE, L1, L2, SELECT
- RGB is still global layer feedback through RGBLIGHT presets
- OLED already shows layer, last keycode, and key position
- This is the clearest working baseline conceptually

### 2. VIA keymap
- Expands the layer model to BASE, NAV, EDIT, MEDIA, FN, RGB, SELECT
- Keeps the separate selector button and uses the encoder button mainly for wake behavior
- OLED UI is still simple and readable
- RGB is still mostly based on RGBLIGHT modes, not true per-key rendering
- Contains at least one obvious code issue: `refresh_feedback()` references `selector_target`, which is not defined

### 3. Reworked keymap
- This is the most ambitious prototype and is closest to the original design vision
- Introduces explicit per-key LED rendering through `rgblight_sethsv_at()`
- Adds a key-to-LED mapping array
- Adds WILD vs QUIET RGB profiles
- Adds timed SELECT cursor cycling at 500 ms
- Allows encoder rotation while in SELECT mode
- Adds a more developed OLED UI and custom select keycodes
- Current layer set is `BASE`, `WINDOW`, `TEXT`, `RGB`, `VSC`, `SELECT`
- This is now the chosen direction for future work
- It still needs cleanup and real build validation

## Current Reality vs Older Notes

Some older assumptions in this file were too absolute and did not match the repo anymore.

- SELECT mode is not just an encoder-button feature. The board has a dedicated selector button, and current keymaps already use it.
- The active layer naming is not settled yet. Different keymaps use different layer sets.
- The OLED driver is currently configured as SSD1306, not SH1106.
- Per-key RGB is not the current default behavior across the repo. It only appears in the reworked prototype.
- The project is not yet at a single canonical architecture. Logic is still mostly embedded inside individual keymap files.

## Current Known Gaps

- No single source of truth for layer names and behavior
- Selector interaction differs between keymaps
- RGB behavior differs between keymaps
- OLED behavior differs between keymaps
- Shared logic has not been extracted into reusable modules yet
- LED order still needs physical verification on the real strip
- Real QMK build validation is currently blocked by a local Windows/QMK CLI process issue during version generation, so editor diagnostics are not trustworthy and the compile check still needs a clean environment run

## Suggested Direction

### Canonical design decisions to make first

1. Choose one canonical layer model
  - Either keep the smaller practical set from VIA/simple
  - Or adopt the reworked set: BASE, WINDOW, TEXT, RGB, VSC, SELECT

2. Choose one selector interaction model
  - Recommended: selector button = enter/hold SELECT, encoder = navigate while in SELECT
  - Keep encoder press for a secondary action only if it adds something clear

3. Decide whether the project target is:
  - stable RGBLIGHT-based feedback first
  - or full custom per-key RGB as the main goal

### Technical improvements

1. Pick one keymap as the base branch for future work
  - Recommended: use `reworked` as the feature branch to continue ideas from
  - Use `simple` as the behavioral reference for fallback and sanity checks

2. Fix the obvious logic and compile risks before adding features
  - Remove stale symbols like `selector_target`
  - Verify SELECT entry/exit behavior is consistent
  - Confirm the reworked keymap actually builds under QMK

3. Extract shared logic out of keymap files
  - `selector.c` / selector state machine
  - `rgb.c` / layer visual rendering
  - `oled.c` / screen rendering
  - shared layer names and key labels in one header

4. Lock down LED mapping early
  - Test the 9 LEDs physically
  - Keep a single `key_led_map[]`
  - Document the verified physical routing in this file

5. Keep the animation model non-blocking
  - Use timer-driven updates only
  - Avoid loops that stall matrix scanning
  - Keep OLED and RGB refresh intervals explicit

6. Define what QUIET mode means exactly
  - Recommended: only the current layer slot breathes softly
  - No auto-cycling unless SELECT mode is active
  - No full-board effects outside WILD mode

## Recommended Next Steps

1. Continue fixing correctness issues in `reworked` before further feature work
2. Verify and document the real LED strip order
3. Normalize layer names across notes, OLED labels, and keymaps
4. Extract selector, RGB, and OLED code into separate modules
5. Run a clean QMK build from an environment where the CLI can invoke git/processes correctly
6. Only after that, expand toward more layers or more complex animations

## Working Assumptions

- Treat the strip as individually addressable per-key hardware
- Prefer explicit state machines over stacked QMK lighting presets
- Keep behavior lightweight and non-blocking
- Use actual QMK build results as the source of truth for compile status
- Keep this file focused on current repo state plus the next intended direction


## Current Reworked Behavior

- OLED default view shows a 3x3 legend table for the active layer
- Encoder button tap toggles OLED legend vs last-key view
- Encoder button hold enters momentary `SELECT`
- GP12 is momentary `TEXT`
- Key 1 on each main layer is still `MO(_SELECT)`
- Encoder behavior is layer-dependent:
  - `BASE` = volume up/down
  - `WINDOW` = next/previous window
  - `TEXT` = select left/right
  - `RGB` = brightness up/down
  - `VSC` = next/previous editor tab group item

## OLED Layout Notes

TXT
- `COPY` = copy
- `CUT` = cut
- `ALL` = select all
- `PASTE` = paste
- `POS1` = line start / home

WINDOW
- `WIN<` / `WIN>` = previous/next window
- `DESK<` / `DESK>` = previous/next desktop

## VSC Layer

- `VSC` replaces the older `DEV` experiment idea
- Current bindings are focused on general Visual Studio Code navigation
- Current top rows:
  - `EXPL` = Explorer
  - `SRCH` = Search
  - `GIT` = Source Control
  - `RUN` = Run and Debug
  - `EXT` = Extensions
- Bottom row currently uses editor navigation shortcuts and can still be changed

## Remaining TODO

- Decide whether QUIET mode should stay at all
- Test the `VSC` shortcuts on the real host setup
- Decide whether the `VSC` layer should target specific tools like GitHub Actions or stay focused on general VS Code navigation
- Add one or two customizable chat macros if they are still wanted
- Verify LED order on hardware
- Run a clean QMK build in a known-good environment

## VSC layer

- I need a VSC layer. its called "VSC" 

- included are ahortcuts like: Explorer, github actions, source, or simply move through actibvity bar should do it aswell. 

- The first three matrix keys are modifiers (the selector stays untouched, BAR and CHAT)

- impliment a Kombokey system on this particular layer, fit the oled output.

- CHAT Kombos : there i want to place makros for messages and prompts, just take something that i can easily edit.

- BAR kombos : starts in row 1 with
explorer, source control, github actions, github and chatgpt. 
leave the rest free if there is any space left.

