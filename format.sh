#!/usr/bin/env bash
# ============================================================================
# format.sh — run clang-format over the C/C++ sources in croaster/ and boards/.
#
#   ./format.sh            # format in place (-i)
#   ./format.sh --check    # verify formatting (exit 1 if any file needs changes)
#   ./format.sh --diff     # show the unified diff without writing
#
# Uses clang-format from PATH (override with CLANG_FORMAT=/path/to/clang-format).
# Style comes from the .clang-format at the repo root. Build artifacts
# (.pio, builds) are skipped.
# ============================================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"

MODE="inplace"
case "${1:-}" in
    --check) MODE="check" ;;
    --diff)  MODE="diff"  ;;
    "")
        ;;
    *)
        echo "usage: $0 [--check|--diff]" >&2
        exit 2
        ;;
esac

if ! command -v "$CLANG_FORMAT" >/dev/null 2>&1; then
    echo "error: clang-format not found ('$CLANG_FORMAT'; set CLANG_FORMAT=...)" >&2
    exit 1
fi

# All C/C++ sources under croaster/ and boards/, skipping .pio build artifacts.
FILES=()
while IFS= read -r -d '' f; do
    FILES+=("$f")
done < <(
    find "$ROOT/croaster" "$ROOT/boards" \
        -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \
                   -o -name '*.cc' -o -name '*.cxx' -o -name '*.ino' \) \
        -not -path '*/.pio/*' \
        -print0
)

if [ "${#FILES[@]}" -eq 0 ]; then
    echo "no C/C++ sources found under croaster/ or boards/"
    exit 0
fi

case "$MODE" in
    check)
        # --dry-run prints a diff; --Werror makes it exit non-zero when any
        # file is not formatted.
        if ! "$CLANG_FORMAT" --dry-run --Werror "${FILES[@]}" >/dev/null; then
            echo "error: some files need formatting (run ./format.sh)" >&2
            exit 1
        fi
        echo "all ${#FILES[@]} files are formatted"
        ;;
    diff)
        "$CLANG_FORMAT" --dry-run "${FILES[@]}"
        ;;
    inplace)
        "$CLANG_FORMAT" -i "${FILES[@]}"
        echo "formatted ${#FILES[@]} files (croaster/ + boards/)"
        ;;
esac
