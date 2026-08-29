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
- `src/` — the **Croaster library core** (display/pin-agnostic). Consumed by
  every implementation via `lib_deps = ../..`. Board specifics never go here.
- `implementation/<board>/` — a standalone PlatformIO project per board
  (`esp32c3`, `esp8266`, `esp32s3`). Each owns its `platformio.ini`,
  `src/main.cpp` + `src/config.h` (pins/dummy/LED), board-specific display
  code, and a per-board README. `implementation/common/` holds the shared
  SSD1306 display used by the non-LVGL boards.
- `builds/` — gitignored; firmware output of `build_all.sh`.

## Build & release
- `./build_all.sh [board...]` — builds every implementation (or the listed
  ones) and copies each `firmware.bin` into `builds/`. Per-board manual build:
  `cd implementation/<board> && pio run -e <env> -t upload`.
- PlatformIO is **not on PATH** on macOS — the script uses
  `~/.platformio/penv/bin/pio`; set `PIO=...` to override.
- ⚠️ **Gotcha:** implementations consume the library from `../..`; PlatformIO
  snapshots it into `.pio/libdeps/<env>/Croaster/`. **Delete `.pio/libdeps`**
  after editing `src/` library code, or the build silently uses the stale copy.
- Each implementation defines its board id via the `-DCROASTER_BOARD_NAME=`
  build flag in its `platformio.ini` (read by
  `CroasterDeviceIdentity::boardName()`). There are **no board-specific
  compile-time guards** — each board builds its own folder unconditionally.

## Conventions
- Board config lives in the implementation (`config.h`, `pins.h`,
  `platformio.ini`, README) — never in the library.
- 4-space indent, Allman braces, ~120 col width (matches the library sources).
- No Arduino IDE — PlatformIO is the only workflow.
- Board ids are lowercase: `esp32s3` / `esp32c3` / `esp8266` / `unknown`
  (match the ICRM app's `CroasterBoardTypes` enum).

## Before you finish a session
- Working tree clean (or leftovers explicitly noted in CONTEXT.md → Next steps).
- `CONTEXT.md` reflects exactly what is committed and what is pending.
