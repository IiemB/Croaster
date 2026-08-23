#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CSV_SOURCE="$SCRIPT_DIR/custom32c3sm.csv"
ARDUINO_PACKAGES_DIR="$HOME/Library/Arduino15/packages/esp32/hardware/esp32"
ARDUINO_USER_DATA_DIR="$HOME/Library/Application Support/arduino-ide"

PARTITION_KEY="custom32c3sm"
BOARD_PREFIX="makergo_c3_supermini"

log() {
    printf '[INFO] %s\n' "$1"
}

warn() {
    printf '[WARN] %s\n' "$1"
}

fail() {
    printf '[ERROR] %s\n' "$1" >&2
    exit 1
}

print_step() {
    printf '\n========== %s ==========' "$1"
    printf '\n'
}

require_macos() {
    if [[ "$(uname -s)" != "Darwin" ]]; then
        fail "This script is for macOS only."
    fi
}

find_latest_esp32_core_dir() {
    if [[ ! -d "$ARDUINO_PACKAGES_DIR" ]]; then
        fail "ESP32 core directory not found at: $ARDUINO_PACKAGES_DIR"
    fi

    local latest_dir
    latest_dir="$(find "$ARDUINO_PACKAGES_DIR" -mindepth 1 -maxdepth 1 -type d | sort -V | tail -n 1)"

    if [[ -z "$latest_dir" ]]; then
        fail "No ESP32 core versions found in: $ARDUINO_PACKAGES_DIR"
    fi

    printf '%s\n' "$latest_dir"
}

copy_partition_csv() {
    local core_dir="$1"
    local partitions_dir="$core_dir/tools/partitions"
    local csv_target="$partitions_dir/custom32c3sm.csv"

    [[ -f "$CSV_SOURCE" ]] || fail "Missing source CSV: $CSV_SOURCE"

    log "Using ESP32 core: $core_dir"
    log "Ensuring partitions directory exists: $partitions_dir"
    mkdir -p "$partitions_dir"

    log "Copying partition CSV to: $csv_target"
    cp "$CSV_SOURCE" "$csv_target"

    log "CSV installed successfully."
}

ensure_partition_entries() {
    local core_dir="$1"
    local boards_file=""

    if [[ -f "$core_dir/boards.txt" ]]; then
        boards_file="$core_dir/boards.txt"
    elif [[ -f "$core_dir/board.txt" ]]; then
        boards_file="$core_dir/board.txt"
    else
        fail "Could not find boards file in: $core_dir"
    fi

    print_step "Update boards file"
    log "Target boards file: $boards_file"

    local line1="${BOARD_PREFIX}.menu.PartitionScheme.${PARTITION_KEY}=Custom SuperMini"
    local line2="${BOARD_PREFIX}.menu.PartitionScheme.${PARTITION_KEY}.build.partitions=${PARTITION_KEY}"
    local line3="${BOARD_PREFIX}.menu.PartitionScheme.${PARTITION_KEY}.upload.maximum_size=1900544"

    if grep -Fq "$line1" "$boards_file" && grep -Fq "$line2" "$boards_file" && grep -Fq "$line3" "$boards_file"; then
        log "Partition entries already exist. No changes needed."
        return
    fi

    if grep -Fq "MakerGO ESP32 C3 SuperMini" "$boards_file"; then
        log "MakerGO section found. Appending partition entries near the section."
    else
        warn "MakerGO section not found. Appending entries at end of file."
    fi

    {
        printf '\n# Added by setup_custom_partition_macos.sh\n'
        printf '%s\n' "$line1"
        printf '%s\n' "$line2"
        printf '%s\n' "$line3"
    } >> "$boards_file"

    log "Boards file updated."
}

clear_arduino_ide_user_data() {
    print_step "Delete Arduino IDE user data"
    log "Target path: $ARDUINO_USER_DATA_DIR"

    if [[ -d "$ARDUINO_USER_DATA_DIR" ]]; then
        rm -rf "$ARDUINO_USER_DATA_DIR"
        log "Deleted Arduino IDE user data directory."
    else
        log "Directory does not exist. Nothing to delete."
    fi
}

main() {
    print_step "Validate environment"
    require_macos
    log "Running on macOS."

    print_step "Find installed ESP32 core"
    local core_dir
    core_dir="$(find_latest_esp32_core_dir)"

    print_step "Install custom partition CSV"
    copy_partition_csv "$core_dir"

    ensure_partition_entries "$core_dir"
    clear_arduino_ide_user_data

    print_step "Done"
    log "Setup complete. Restart Arduino IDE 2.x to apply updates."
}

main "$@"