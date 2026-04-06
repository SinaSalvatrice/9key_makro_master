# 9key_makro_master - Project Notes

## My Part

### Overview

#### Goal
- Build a 3x3 RP2040 macro pad with a selector workflow, OLED feedback, and RGB behavior that can evolve from simple layer colors into per-key animated visuals.

#### Intent
- Use `keymaps/reworked` as the canonical implementation target going forward.
- Use `keymaps/simple` only as a fallback reference for known simpler behavior.
- Do not spend time expanding the other keymaps unless they are needed for comparison or recovery.

#### Hardware
- RP2040/QMK keyboard
- 3x3 matrix
- 1 WS2812 LED per key, 9 total
- OLED display on I2C, configured as SSD1306
- Rotary encoder on GP9/GP10
- Encoder push button on GP8
- Selector currently uses the first matrix key; the old GP12 selector path is optional only

### Layers

#### Ideas
- Long-term target is one useful layer per physical key slot in the selector grid.
- Current named layers are `BASE`, `WINDOW`, `TEXT`, `MEDIA`, `DEV`, `VSC`, `RGB`, `SELECT`.
- `DEV` should stay available for later experiments.
- `VSC` should stay focused on VS Code workflows.

#### Suggestions
- Keep adding layers until each selector slot has a real purpose.

### OLED

#### Ideas
TXT:
- `COPY` = copy
- `CUT` = cut
- `ALL` = select all
- `PASTE` = paste
- `POS1` = line start / home

WINDOW:
- `WIN<` / `WIN>` = previous/next window
- `DESK<` / `DESK>` = previous/next desktop

### RGB

#### Questions
- Should QUIET mode stay at all?

### VSC

#### Ideas
- `VSC` is an additional layer, not a replacement for `DEV`.
- Top row should stay `SEL`, `BAR`, `CHAT`.
- The lower six keys are shared combo targets for the held `BAR` or `CHAT` modifier.

#### Suggestions
Current `BAR` targets:
- `EXPL` = Explorer
- `SRC` = Source Control
- `GH-A` = GitHub Actions
- `GHUB` = GitHub / Pull Requests view
- `GPT` = Copilot Chat view
- `FREE` = spare slot

Current `CHAT` targets:
- `SUM` = summarize selected file/code
- `REVW` = review selected code
- `FIX` = suggest a minimal fix
- `TEST` = write tests
- `EXPL` = explain code
- `COMMIT` = write commit message text



## Your Part

### Repository

#### Status
- The repo still contains several parallel keymap experiments, but `keymaps/reworked` is the active implementation target.
- `simple`, `via`, and `bringup` are still useful references, but they are not the current source of truth for behavior.
- Some older keymaps still assume a dedicated `SELECTOR_BTN_PIN` path and would need syncing if they are revived.

#### Simple
- Still uses a dedicated selector-button flow and a compact `BASE`, `L1`, `L2`, `SELECT` layer set.
- RGB is still global layer feedback through RGBLIGHT presets.
- OLED already shows layer, last keycode, and key position.

#### VIA
- Uses the larger `BASE`, `NAV`, `EDIT`, `MEDIA`, `FN`, `RGB`, `SELECT` model.
- Still assumes the older dedicated selector-button behavior.
- `refresh_feedback()` still references `selector_target`, which is not defined.

### Layers

#### Implemented
- Reworked uses the layer set `BASE`, `WINDOW`, `TEXT`, `MEDIA`, `DEV`, `VSC`, `RGB`, `SELECT`.
- Selector slots and custom select keycodes are implemented.
- The selector entry key is the first matrix key (`MO(_SELECT)` at row `0`, col `0`), with `SELECTOR_MATRIX_ROW/COL` defined in `config.h`.
- Entering `SELECT` now keeps the cursor aligned with the currently active layer slot instead of always resetting to `BASE`.

#### Issues
- No single shared module exists yet for the common selector/OLED/RGB logic.
- The real VS Code host setup may still need command-name adjustments for some `VSC` targets.

### Input And Navigation

#### Implemented
- Key 1 on each main layer is still `MO(_SELECT)`.
- Current config uses the first matrix key as the selector; `SELECTOR_BTN_PIN` is optional and is not defined in the current `config.h`.
- If the optional GP12 path is re-enabled, hold enters `SELECT` while the button is held.
- Encoder behavior is layer-dependent:
  - `BASE` = mouse wheel up/down
  - `WINDOW` = Alt+Tab next/previous
  - `TEXT` = left/right
  - `MEDIA` = volume up/down
  - `RGB` = brightness up/down
  - `DEV` = mouse wheel up/down
  - `VSC` = Ctrl+PgDn / Ctrl+PgUp

#### Needs Testing
- Verify selector entry and exit behavior on real hardware.
- Confirm the fallback `R0C0 + R2C2 -> BASE` combo still feels useful.

### OLED

#### Implemented
- OLED shows compact status and last-key information.
- While `SELECT` is active, the OLED now shows selector status instead of the normal last-key view.
- `VSC` status text still reflects whether `BAR` or `CHAT` is the current preview mode.

#### Needs Testing
- Confirm readability and spacing on the real display.

### RGB

#### Implemented
- Reworked uses explicit per-key LED rendering through `rgblight_sethsv_at()`.
- Layer visuals and selector visuals are timer-driven.
- The center selector slot doubles as the FX toggle (`RGB_PROFILE`) in the selector grid.

#### Issues
- LED order still needs physical verification on the real strip.

### VSC

#### Implemented
- `DEV` remains a separate layer; `VSC` is an additional selectable layer.
- Top row is `SEL`, `BAR`, `CHAT`.
- The lower six keys (`VSC_1`…`VSC_6`) trigger shared targets using the current or last selected `BAR`/`CHAT` mode.
- `BAR` mode sends command-palette strings from one editable block in the keymap.
- `CHAT` mode sends editable text macros from one editable block in the keymap.

#### Issues
- The exact VS Code command palette names for some extension views may still need adjustment on the real host.
- The default chat macro text is still placeholder content and should be personalized.

#### Suggestions
- Keep the command strings and chat strings editable in one place.

### Build

#### Issues
- Real QMK build validation is still blocked locally by the Windows/QMK CLI environment issue (`WinError 2` / `build_keyboard.mk:240: missing separator`).

#### Suggestions
1. Test the `VSC` shortcuts on the real host setup.
2. Replace the default chat macros with your real prompt text.
3. Verify the exact command palette names for GitHub Actions, GitHub, and Copilot Chat in the installed extension set.
4. Verify and document the real LED strip order.
5. Run a clean QMK build in a known-good environment.

### Assumptions

- Treat the strip as individually addressable per-key hardware.
- Prefer explicit state machines over stacked QMK lighting presets.
- Keep behavior lightweight and non-blocking.
- Use actual QMK build results as the source of truth for compile status.
- Keep this file focused on current repo state plus the next intended direction.

## shortforms i will use 

- Last keycode pressed (btn,toggle) = LK
