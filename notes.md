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
- Separate selector button on GP12

### Layers

#### Ideas
- Long-term target is one useful layer per physical key slot in the selector grid.
- Current named layers are `BASE`, `WINDOW`, `TEXT`, `RGB`, `DEV`, `VSC`, `SELECT`.
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

#### Issues
- The default chat macros still need to be replaced with real prompt/message text.

## Your Part

### Repository

#### Status
- The repo currently contains multiple parallel keymap implementations rather than one settled design.

#### Simple
- Uses a dedicated selector button on GP12 to hold the SELECT layer.
- Has a compact layer set: `BASE`, `L1`, `L2`, `SELECT`.
- RGB is still global layer feedback through RGBLIGHT presets.
- OLED already shows layer, last keycode, and key position.

#### VIA
- Expands the layer model to `BASE`, `NAV`, `EDIT`, `MEDIA`, `FN`, `RGB`, `SELECT`.
- Keeps the separate selector button and uses the encoder button mainly for wake behavior.

#### Issues
- VIA contains at least one obvious code issue: `refresh_feedback()` references `selector_target`, which is not defined.

### Layers

#### Implemented
- Reworked is the active implementation target.
- Reworked uses the layer set `BASE`, `WINDOW`, `TEXT`, `RGB`, `DEV`, `VSC`, `SELECT`.
- Selector slots and custom select keycodes are implemented.

#### Issues
- No single source of truth for shared layer logic outside the keymap yet.
- Shared logic has not been extracted into modules yet.

### Input And Navigation

#### Implemented
- Encoder button tap toggles OLED legend vs last-key view.
- Encoder button hold enters momentary `SELECT`.
- GP12 is momentary `TEXT`.
- Key 1 on each main layer is still `MO(_SELECT)`.
- Encoder behavior is layer-dependent:
  - `BASE` = volume up/down
  - `WINDOW` = next/previous window
  - `TEXT` = select left/right
  - `RGB` = brightness up/down
  - `DEV` = mouse wheel up/down
  - `VSC` = next/previous editor tab group item

#### Needs Testing
- Verify selector entry and exit behavior on real hardware.

### OLED

#### Implemented
- OLED default view shows a 3x3 legend table for the active layer.
- OLED last-key view shows layer, key label, keycode, function, and matrix position.
- OLED legend rendering changes on `VSC` depending on whether `BAR` or `CHAT` is active.

#### Needs Testing
- Verify readability on the real display.

### RGB

#### Implemented
- Reworked uses explicit per-key LED rendering through `rgblight_sethsv_at()`.
- Layer visuals and selector visuals are timer-driven.

#### Issues
- LED order still needs physical verification on the real strip.

### VSC

#### Implemented
- `DEV` is still present as a separate layer.
- `VSC` was added as another selectable layer.
- `BAR` mode sends command-palette commands from one editable string block in the keymap.
- `CHAT` mode sends editable text macros from one editable string block in the keymap.

#### Issues
- The exact VS Code command palette names for some extension views may need adjustment depending on installed extensions.

#### Suggestions
- Keep the command strings and chat strings editable in one place.

### Build

#### Issues
- Real QMK build validation is still blocked by the local Windows/QMK CLI environment issue.

#### Suggestions
1. Test the `VSC` shortcuts on the real host setup.
2. Replace the default chat macros with your real text.
3. Verify the exact command palette names for GitHub Actions, GitHub, and Copilot Chat in the installed extension set.
4. Verify and document the real LED strip order.
5. Run a clean QMK build in a known-good environment.

### Assumptions

- Treat the strip as individually addressable per-key hardware.
- Prefer explicit state machines over stacked QMK lighting presets.
- Keep behavior lightweight and non-blocking.
- Use actual QMK build results as the source of truth for compile status.
- Keep this file focused on current repo state plus the next intended direction.

