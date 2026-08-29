#!/usr/bin/env bash
# ============================================================================
# build_all.sh — build every Croaster board implementation and collect the
# resulting firmware into ./builds/ (gitignored).
#
#   ./build_all.sh            # build all boards
#   ./build_all.sh esp32s3    # build one board (any subdir of implementation/)
#
# Requires PlatformIO. It is not on PATH on macOS by default, so the script
# uses ~/.platformio/penv/bin/pio first; override with PIO=/path/to/pio.
# ============================================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PIO="${PIO:-$HOME/.platformio/penv/bin/pio}"
OUT="$ROOT/builds"

if [ ! -x "$PIO" ]; then
    echo "error: PlatformIO not found at '$PIO' (set PIO=/path/to/pio)" >&2
    exit 1
fi

# Boards to build: CLI args, or auto-discover implementation/*/platformio.ini.
BOARDS=("$@")
if [ "${#BOARDS[@]}" -eq 0 ]; then
    for ini in "$ROOT"/implementation/*/platformio.ini; do
        [ -e "$ini" ] || continue
        b="$(basename "$(dirname "$ini")")"
        [ "$b" = "common" ] && continue # shared display lib, not a board
        BOARDS+=("$b")
    done
fi

rm -rf "$OUT"
mkdir -p "$OUT"

for b in "${BOARDS[@]}"; do
    dir="$ROOT/implementation/$b"
    if [ ! -d "$dir" ]; then
        echo "skip: no implementation/$b"
        continue
    fi

    echo "=== Building $b ==="
    (cd "$dir" && "$PIO" run)

    for fw in "$dir"/.pio/build/*/firmware.bin; do
        [ -e "$fw" ] || continue
        env="$(basename "$(dirname "$fw")")"
        dest="$OUT/$b-$env.bin"
        cp "$fw" "$dest"
        bytes="$(wc -c < "$fw" | tr -d ' ')"
        kb="$(awk "BEGIN{printf \"%.1f\", $bytes/1024}")"
        printf "  -> %-24s %8s KB  %s\n" "$b-$env" "$kb" "$dest"
    done
done

echo "=== Firmware in $OUT ==="
ls -lh "$OUT"
