#!/usr/bin/env bash
# ============================================================================
# upload_monitor.sh — build + upload a board's DEBUG firmware to a connected
# device (USB) and/or open its serial monitor. Release firmware is only
# produced by build_all.sh.
#
#   ./upload_monitor.sh [mode] [board] [port]
#
#     mode   upload | monitor           (default: upload + monitor)
#     board  esp32c3 / esp8266 / esp32s3 (default: interactive menu)
#     port   serial device              (default: auto-detect / menu)
#
# Examples:
#   ./upload_monitor.sh                      # upload + monitor (default)
#   ./upload_monitor.sh upload               # build + upload only
#   ./upload_monitor.sh monitor              # serial monitor only
#   ./upload_monitor.sh upload esp32c3 /dev/cu.usbmodem101
#   ./upload_monitor.sh monitor esp32c3
#
# Uses each board's [env:<board>-debug] (extends the release env with
# build_type = debug). Requires PlatformIO — same PIO override as
# build_all.sh (defaults to ~/.platformio/penv/bin/pio on macOS).
# ============================================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PIO="${PIO:-$HOME/.platformio/penv/bin/pio}"

# ---- mode: optional first arg (upload | monitor); default = upload + monitor
MODE=""
case "${1:-}" in
    upload | monitor) MODE="$1"; shift ;;
esac

# ---- available boards -------------------------------------------------------
boards=()
for ini in "$ROOT"/boards/*/platformio.ini; do
    [ -e "$ini" ] || continue
    b="$(basename "$(dirname "$ini")")"
    [ "$b" = "common" ] && continue
    boards+=("$b")
done

if [ "${#boards[@]}" -eq 0 ]; then
    echo "error: no board implementations found" >&2
    exit 1
fi

# ---- choose board -----------------------------------------------------------
BOARD="${1:-}"
if [ -z "$BOARD" ]; then
    echo "Choose a board:"
    for i in "${!boards[@]}"; do
        printf "  %d) %s\n" "$((i+1))" "${boards[$i]}"
    done
    printf "Board [1-%d, or name]: " "${#boards[@]}"
    read -r sel || sel=""
    if [[ "$sel" =~ ^[0-9]+$ ]] && (( sel >= 1 && sel <= ${#boards[@]} )); then
        BOARD="${boards[$((sel-1))]}"
    elif [ -n "$sel" ]; then
        BOARD="$sel"
    else
        echo "error: no board selected" >&2
        exit 1
    fi
fi

dir="$ROOT/boards/$BOARD"
if [ ! -d "$dir" ]; then
    echo "error: no boards/$BOARD" >&2
    printf "known boards: %s\n" "${boards[*]}" >&2
    exit 1
fi

if [ ! -x "$PIO" ]; then
    echo "error: PlatformIO not found at '$PIO' (set PIO=/path/to/pio)" >&2
    exit 1
fi

# ---- choose port ------------------------------------------------------------
PORT="${2:-}"
if [ -z "$PORT" ]; then
    # serial ports detected via `pio device list` (lines starting with /dev/).
    ports=($("$PIO" device list 2>/dev/null | awk '{print $1}' | grep '^/dev/' || true))
    if [ "${#ports[@]}" -eq 0 ]; then
        printf "No serial port detected. Port (blank = auto-detect): "
        read -r PORT || PORT=""
    else
        echo "Choose a port (blank = auto-detect):"
        for i in "${!ports[@]}"; do
            printf "  %d) %s\n" "$((i+1))" "${ports[$i]}"
        done
        printf "Port [1-%d, path, or blank]: " "${#ports[@]}"
        read -r sel || sel=""
        if [[ "$sel" =~ ^[0-9]+$ ]] && (( sel >= 1 && sel <= ${#ports[@]} )); then
            PORT="${ports[$((sel-1))]}"
        else
            PORT="$sel"
        fi
    fi
fi

case "$MODE" in
    monitor)
        # Serial monitor only — no build, no libdeps refresh.
        ARGS=(device monitor -e "$BOARD-debug")
        if [ -n "$PORT" ]; then
            ARGS+=(-p "$PORT")
        fi
        echo "=== Monitoring $BOARD (debug env) ==="
        ;;
    upload)
        # The <board>-debug env is a throwaway; drop its stale .pio/libdeps
        # snapshot so a config change never triggers PlatformIO's "Removing
        # unused dependencies" (which can hang on leftover plain-path specs).
        rm -rf "$dir"/.pio/libdeps/"$BOARD-debug"
        ARGS=(run -e "$BOARD-debug" -t upload)
        if [ -n "$PORT" ]; then
            ARGS+=(--upload-port "$PORT")
        fi
        echo "=== Uploading $BOARD (debug build) ==="
        ;;
    *)
        # Default: upload + monitor.
        rm -rf "$dir"/.pio/libdeps/"$BOARD-debug"
        ARGS=(run -e "$BOARD-debug" -t upload -t monitor)
        if [ -n "$PORT" ]; then
            ARGS+=(--upload-port "$PORT" --monitor-port "$PORT")
        fi
        echo "=== Uploading + monitoring $BOARD (debug build) ==="
        ;;
esac

if [ -n "$PORT" ]; then
    echo "    port: $PORT"
fi
(cd "$dir" && "$PIO" "${ARGS[@]}")
