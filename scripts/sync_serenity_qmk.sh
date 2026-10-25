#!/usr/bin/env bash
# Bring the Serenity layout over from serenity-qmk, and record which commit it came from.
#
# Usage: scripts/sync_serenity_qmk.sh            # sync from serenity-qmk's HEAD
#        scripts/sync_serenity_qmk.sh <commit>   # sync from a given serenity-qmk commit
#        scripts/sync_serenity_qmk.sh --check    # list serenity-qmk commits not synced yet
#
# SERENITY_QMK sets where serenity-qmk lives (default: ../serenity-qmk).
#
# Copies from the commit, never from uncommitted edits:
#   users/serenity                              -> users/serenity
#   keyboards/planck/keymaps/serenity/keymap.c  -> keyboards/thock/conundrum/keymaps/serenity/keymap.c
#   modules/* submodule commits                 -> the same commits here
# then writes the commit to SERENITY_QMK_COMMIT. Review with git diff, build, then commit.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC=${SERENITY_QMK:-$ROOT/../serenity-qmk}
RECORD=$ROOT/SERENITY_QMK_COMMIT
SYNCED_PATHS=(users/serenity keyboards/planck/keymaps/serenity modules)
KEYMAP_SRC=keyboards/planck/keymaps/serenity/keymap.c
KEYMAP_DEST=keyboards/thock/conundrum/keymaps/serenity/keymap.c
RULES=$ROOT/keyboards/thock/conundrum/keymaps/serenity/rules.mk

check=no
if [[ ${1:-} == --check ]]; then
    check=yes
    shift
fi

target=$(git -C "$SRC" rev-parse --verify "${1:-HEAD}^{commit}")
synced=$(head -n 1 "$RECORD")
pending=$(git -C "$SRC" log --format='%h %ad %s' --date=short "$synced..$target" -- "${SYNCED_PATHS[@]}")

if [[ $check == yes ]]; then
    if [[ -z $pending ]]; then
        echo "Up to date: serenity-qmk has no layout changes since ${synced:0:7}"
    else
        echo "serenity-qmk commits not synced yet (synced: ${synced:0:7}):"
        echo "$pending"
    fi
    exit 0
fi

if [[ -n $(git -C "$SRC" status --porcelain -- "${SYNCED_PATHS[@]}") ]]; then
    echo "Note: serenity-qmk has uncommitted layout changes; only committed work is synced"
fi

# users/serenity and the keymap
rm -rf "$ROOT/users/serenity"
git -C "$SRC" archive "$target" users/serenity | tar -x -C "$ROOT"
git -C "$SRC" show "$target:$KEYMAP_SRC" > "$ROOT/$KEYMAP_DEST"

# Module submodules, at serenity-qmk's commits
git -C "$SRC" ls-tree "$target" modules/ | while read -r _ _ commit path; do
    if [[ ! -e $ROOT/$path/.git ]]; then
        echo "Warning: serenity-qmk has a new module repo, $path; add it here with git submodule add"
        continue
    fi
    git -C "$ROOT/$path" cat-file -e "$commit^{commit}" 2>/dev/null || git -C "$ROOT/$path" fetch -q origin
    git -C "$ROOT/$path" checkout -q "$commit"
done

# QMK 0.26 has no keymap.json modules, so rules.mk and compat.h/.c list them by hand
wanted=$(git -C "$SRC" show "$target:keyboards/planck/keymaps/serenity/keymap.json" | grep -o '"[a-z_]*/[a-z_]*"' | tr -d '"' | sort)
built=$(sed -n 's/^SERENITY_MODULES = //p' "$RULES" | tr ' ' '\n' | grep -v '^getreuer/layer_lock$' | sort)
if [[ $wanted != "$built" ]]; then
    echo "Warning: serenity-qmk's modules changed. Update SERENITY_MODULES in rules.mk and compat.h/.c:"
    diff <(echo "$built") <(echo "$wanted") | sed -n 's/^</  remove/p; s/^>/  add/p'
fi

{
    echo "$target"
    echo "# serenity-qmk commit that users/serenity, the serenity keymap and modules/ were last"
    echo "# synced from. Update it with scripts/sync_serenity_qmk.sh, not by hand."
} > "$RECORD"

echo "Synced serenity-qmk ${target:0:7} ($(git -C "$SRC" log -1 --format=%s "$target"))"
if [[ -n $pending ]]; then
    echo "Brought over:"
    echo "$pending"
fi
git -C "$ROOT" status --short -- users/serenity "$KEYMAP_DEST" modules SERENITY_QMK_COMMIT
