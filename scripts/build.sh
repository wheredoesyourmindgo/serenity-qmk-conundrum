#!/usr/bin/env bash
# Build Conundrum firmware into the repo root (thock_conundrum_<keymap>.uf2).
#
# Usage: scripts/build.sh                            # serenity and default
#        scripts/build.sh serenity                   # one keymap
#        scripts/build.sh serenity -e QWERTY_BASE=yes
#        DOCKER=1 scripts/build.sh                   # in QMK's Docker image, no local toolchain needed
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
KEYBOARD=thock/conundrum

keymaps=()
while [[ $# -gt 0 && $1 != -* ]]; do
    keymaps+=("$1")
    shift
done
[[ ${#keymaps[@]} -gt 0 ]] || keymaps=(serenity default)

for keymap in "${keymaps[@]}"; do
    if [[ -n ${DOCKER:-} ]]; then
        # make takes options as VAR=value, so drop qmk's -e
        make_args=()
        for arg in "$@"; do
            [[ $arg == -e ]] || make_args+=("$arg")
        done
        docker run --rm --user "$(id -u):$(id -g)" -w /qmk_firmware -v "$ROOT":/qmk_firmware \
            ghcr.io/qmk/qmk_cli make "$KEYBOARD:$keymap" ${make_args[@]+"${make_args[@]}"}
    else
        "$ROOT/scripts/qmk_here.sh" compile -kb "$KEYBOARD" -km "$keymap" "$@"
    fi
done
