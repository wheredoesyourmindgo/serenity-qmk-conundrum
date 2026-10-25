#!/usr/bin/env bash
# Run the qmk CLI against this repo only.
#
# Your global QMK config points qmk_home at another clone and overlay_dir at serenity-qmk, whose
# users/serenity would then win over this repo's copy. This runs qmk with a throwaway config
# instead, so every build here uses this repo's QMK, keyboards and users/serenity.
#
# Usage: scripts/qmk_here.sh <qmk arguments>    e.g. scripts/qmk_here.sh compile -kb thock/conundrum -km serenity
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
CONFIG=$(mktemp -t qmk_here)
trap 'rm -f "$CONFIG"' EXIT

printf '[user]\nqmk_home = %s\n' "$ROOT" > "$CONFIG"
cd "$ROOT"
qmk --config-file "$CONFIG" "$@"
