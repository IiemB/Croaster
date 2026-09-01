# AGENTS.md — Working rules for this repo

## Read first
Start every session by reading **`CONTEXT.md`** — the single source of truth
for the current state of the Croaster firmware (architecture, git state, WIP,
gotchas, next steps). If it's missing, recreate it from the codebase before
doing work.

## Keep `CONTEXT.md` current
Update `CONTEXT.md` in the **same change** as the work that makes it stale:

- Change architecture, file layout, build commands, or the firmware version.
- Add/remove a board implementation or a library feature.
- Land a user-visible firmware/UI change.
- Add a gotcha that took effort to learn.
- Change the git state (new commits, branch, WIP direction).
- Complete or drop a "Next steps" item.

Rules: keep it concise (short bullets/tables, not prose); don't duplicate what
the code, READMEs, or CHANGELOG already say; commit it with the change it
describes.

## Repository layout
- `croaster/` — the **Croaster library core** (display/pin-agnostic):
  `croaster/src/` + `croaster/library.json`. Consumed by every board via
  `lib_deps = ../../croaster`. Board specifics never go here.
- `boards/<board>/` — a standalone PlatformIO project per board (`esp32c3`,
  `esp8266`, `esp32s3`). Each owns its `platformio.ini`, `src/main.cpp` +
  `src/config.h` (pins/dummy/LED), board-specific display code, and a per-board
  README. `boards/common/` holds the shared SSD1306 display used by the
  non-LVGL boards.
- `builds/` — gitignored; firmware output of `build_all.sh`.

## Build & release
- `./build_all.sh [board...]` — builds every board (or the listed ones) and
  copies each `firmware.bin` into `builds/`. Per-board manual build:
  `cd boards/<board> && pio run -e <env> -t upload`.
- `./upload_monitor.sh <board> [port]` — build + upload a **DEBUG** build of
  one board to the connected device, then open the serial monitor
  (`pio run -e <board>-debug -t upload -t monitor`). Release firmware comes
  only from `build_all.sh`: each board env is pinned to `build_type = release`,
  and the `<board>-debug` env `extends` it.
- PlatformIO is **not on PATH** on macOS — the script uses
  `~/.platformio/penv/bin/pio`; set `PIO=...` to override.
- Boards consume the library via **`symlink://`** lib_deps
  (`Croaster=symlink://../../croaster`, `CroasterDisplaySSD1306=symlink://../common`);
  PlatformIO links the live sources into `.pio/libdeps/<env>/`, so edits to
  `croaster/src/` are picked up with no `.pio/libdeps` clearing.
- Each board hardcodes its id via `CroasterDeviceIdentity::setBoardName("...")`
  in its `main.cpp` (read by
  `CroasterDeviceIdentity::boardName()`; reported to the app as `board`).
  There are **no board-specific compile-time guards** — each board builds its
  own folder unconditionally.

## Conventions
- Board config lives in the board (`config.h`, `pins.h`, `platformio.ini`,
  README) — never in the library.
- 4-space indent, Allman braces, ~120 col width (matches the library sources).
- No Arduino IDE — PlatformIO is the only workflow.
- Board ids are lowercase: `esp32s3` / `esp32c3` / `esp8266` / `unknown`
  (match the ICRM app's `CroasterBoardTypes` enum).

## Before you finish a session
- Working tree clean (or leftovers explicitly noted in CONTEXT.md → Next steps).
- `CONTEXT.md` reflects exactly what is committed and what is pending.

## Recent UI work (2026-09-01)

- `esp32s3` LVGL UI: added an `About` page with firmware version, board id,
  and short chip ID; `darkMode` switch and `brightness` slider live on the
  About page.
- Navigation changes: chart moved to a vertical swipe stack (swipe down →
  chart, swipe up → main). Horizontal swipe toggles between Main ⇄ About.
- `LvglTouch` now exposes a vertical-swipe callback. LVGL screen animations
  were shortened for snappier transitions.
- A build was produced after the changes: `builds/Croaster_esp32s3_0.62.bin`.

When working on UI or navigation, update both `CONTEXT.md` and `AGENTS.md`
in the same commit to preserve context for future sessions.
