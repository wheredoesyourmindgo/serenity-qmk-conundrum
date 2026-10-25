#!/usr/bin/env python3
"""Build one clangd compile database covering every Conundrum keymap.

`qmk compile --compiledb` describes a single keymap, overwrites the previous
database, and wipes .build, taking the previous keymap's generated headers
with it. Keymaps never appear in it either, because QMK #includes them from
another file. So this builds each keymap, keeps a copy of its generated
headers, adds an entry for its keymap, and merges everything into
compile_commands.json.

Usage: scripts/compiledb.py                # every keymap below
       scripts/compiledb.py serenity       # rebuild just these
"""

import json
import re
import shlex
import shutil
import subprocess
import sys
from functools import partial
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DB = ROOT / "compile_commands.json"
STASH = ROOT / ".cache" / "compiledb"
KEYBOARD = "thock/conundrum"
KEYMAPS = ["serenity", "default"]
OBJ_SRC = re.compile(r"\.build/(obj_[^/\"]+)/src")
KEYMAP_C = re.compile(r'^-DKEYMAP_C="(.+)"$')
DEP_FLAGS = {"-MMD", "-MP"}


def build(keymap):
    """Compile one keymap and return its database entries."""
    subprocess.run(
        [ROOT / "scripts" / "qmk_here.sh", "compile", "--compiledb", "-kb", KEYBOARD, "-km", keymap],
        check=True,
        stdout=subprocess.DEVNULL,
    )
    entries = json.loads(DB.read_text())
    for e in entries:
        # QMK 0.26 writes a command string; clangd and the steps below want an argument list
        if "command" in e:
            e["arguments"] = shlex.split(e.pop("command"))
    return entries


def relocate(entries):
    """Copy generated headers out of .build and repoint entries at the copies."""
    copied = set()

    def repoint(match, directory):
        obj = match.group(1)
        dest = STASH / obj / "src"
        if obj not in copied:
            shutil.rmtree(dest, ignore_errors=True)
            shutil.copytree(Path(directory) / ".build" / obj / "src", dest)
            copied.add(obj)
        return str(dest)

    for e in entries:
        fix = partial(OBJ_SRC.sub, partial(repoint, directory=e["directory"]))
        args, skip = [], False
        for a in e["arguments"]:
            if skip:
                skip = False
            elif a == "-MF":
                skip = True
            elif a not in DEP_FLAGS:
                args.append(fix(a))
        e["arguments"] = args
        e["file"] = fix(e["file"])
    return entries


def keymap_entry(entries):
    """Make an entry for the keymap from the flags used to compile the rest."""
    for e in entries:
        for a in e["arguments"]:
            if m := KEYMAP_C.match(a):
                return {**e, "file": m.group(1)}
    return None


def main():
    keymaps = KEYMAPS
    previous = []
    if sys.argv[1:]:
        keymaps = [k for k in KEYMAPS if k in sys.argv[1:]]
        if DB.exists():
            previous = json.loads(DB.read_text())  # keep the other keymaps

    merged = {}
    for keymap in keymaps:
        print(f"Building {KEYBOARD}:{keymap}")
        entries = relocate(build(keymap))
        if km := keymap_entry(entries):
            entries.append(km)
        for e in entries:
            path = str((Path(e["directory"]) / e["file"]).resolve())
            merged.setdefault(path, e)  # shared files keep the first keymap's flags
    for e in previous:
        merged.setdefault(str((Path(e["directory"]) / e["file"]).resolve()), e)

    DB.write_text(json.dumps(list(merged.values()), indent=2))
    print(f"Wrote {len(merged)} entries for {len(keymaps)} keymaps to {DB.name}")


if __name__ == "__main__":
    main()
