# Serenity for the Thock Conundrum

QMK firmware for the [Thock](https://thock.co) Conundrum, a 4x12 ortholinear board with rubber dome EC switches. It's QMK 0.26.11 with Thock's board support ported from [thockco/qmk](https://github.com/thockco/qmk), plus two keymaps:

| Keymap | What | Firmware file |
|---|---|---|
| `serenity` | My [Serenity](https://github.com/wheredoesyourmindgo/serenity-qmk) layout, synced from serenity-qmk | `thock_conundrum_serenity.uf2` |
| `default` | Thock's stock keymap: Qwerty, Colemak and Dvorak with Lower and Raise | `thock_conundrum_default.uf2` |

**Layout reference:** interactive diagrams of every Serenity layer are at [serenity-layout.fyi/firmware](https://www.serenity-layout.fyi/firmware).

**Prebuilt firmware** for both keymaps is on the [Releases](../../releases/tag/latest) page.

## Why QMK 0.26

Thock's own firmware is a QMK fork from 2019. The Conundrum uses an ATSAMD51G18A, and QMK dropped support for that chip family (`arm_atsam`, Massdrop's code for the CTRL and ALT) in 0.27 ([qmk/qmk_firmware#24337](https://github.com/qmk/qmk_firmware/pull/24337)). 0.26.11 is the last release that has it, so this repo stays there.

Key sensing is not open source. The pads are read through the chip's capacitive touch peripheral (PTC) by `lib/ec_touch/lib/libec_touch_4x12.a`, a prebuilt library with Thock's code and Microchip's QTouch library inside. It comes from thockco/qmk unchanged, with no source or license. `keyboards/thock/conundrum/matrix.c` turns its readings into key presses: pressed above 125, released below 75.

The library also has a bug that froze every Conundrum now and then until it was replugged ([thockco/qmk#7](https://github.com/thockco/qmk/issues/7)): its touch interrupt waits for each measurement to start, and USB could interrupt that wait long enough for it to never end. `matrix.c` puts every other interrupt below the touch interrupt so that can't happen.

## What's changed from QMK 0.26.11

`git diff 0.26.11` shows everything. In short:

| Where | What |
|---|---|
| `platforms/arm_atsam`, `tmk_core/protocol/arm_atsam` | Thock's 2020 chip changes, behind options so the CTRL and ALT still build: the SAMD51G18A, no crystal (`ARM_ATSAM_DFLL_ONLY`), the 1ms timer on TC2, a PTC clock (`ARM_ATSAM_PTC_ENABLE`), the `uf2boot` bootloader, `ARM_ATSAM_FLOAT_ABI` (the library is soft-float), and `NO_USB2422` / `NO_SR_EXP` to leave out Massdrop's USB hub and shift register. Also opt-in freeze diagnostics (`watchdog.c`, see [Diagnosing freezes](#diagnosing-freezes)) |
| `lib/arm_atsam/.../samd51g18a.h`, `lib/ec_touch` | The SAMD51G18A headers and Thock's library |
| `keyboards/thock/conundrum` | The keyboard and both keymaps |
| `users/serenity`, `modules/` | Serenity, copied from serenity-qmk, and its community modules as submodules |
| `quantum/quantum.h` | Includes the header a keymap names in `QMK_COMPAT_H` (see [How Serenity runs on QMK 0.26](#how-serenity-runs-on-qmk-026)) |

## One-time setup

Native builds use the QMK CLI and the compilers its installer puts in `~/Library/Application Support/qmk` (already there if serenity-qmk is set up; see its README):

```sh
git clone --recurse-submodules https://github.com/wheredoesyourmindgo/serenity-qmk-conundrum.git ~/Development/serenity-qmk-conundrum
```

If the clone was made without `--recurse-submodules`, run `git submodule update --init --recursive`.

Without the QMK CLI, builds can run in QMK's Docker image instead (see below).

## Everyday use

```sh
scripts/build.sh                            # both keymaps
scripts/build.sh serenity                   # one keymap
scripts/build.sh serenity -e QWERTY_BASE=yes
DOCKER=1 scripts/build.sh                   # in QMK's Docker image (Docker Desktop must be running)
```

The firmware lands in the repo root. To flash, put the keyboard in its bootloader and copy the `.uf2` file to the drive that appears; the keyboard restarts when the copy finishes. The reset key is a long press on Boot (Hardware layer, third row, second from the right) in Serenity, or the top-right key on Adjust (Lower + Raise) in the default keymap.

`scripts/build.sh` runs `qmk` through `scripts/qmk_here.sh`, which gives it a throwaway config pointing at this repo. Plain `qmk compile` would use the global config, where `overlay_dir` points at serenity-qmk, and its `users/serenity` would win over the copy here.

## Syncing with serenity-qmk

serenity-qmk is where Serenity is developed. Its changes come over with a script, and `SERENITY_QMK_COMMIT` records which serenity-qmk commit this repo was last synced from:

```sh
scripts/sync_serenity_qmk.sh --check     # list serenity-qmk commits not synced yet
scripts/sync_serenity_qmk.sh             # sync from serenity-qmk's HEAD (or pass a commit)
git diff                                 # review
scripts/build.sh serenity
git commit -am "Sync serenity-qmk <commit>"
```

The script copies from a commit, never from uncommitted edits: `users/serenity`, the Planck Serenity keymap (`keyboards/planck/keymaps/serenity/keymap.c`, the same 4x12 grid) into `keyboards/thock/conundrum/keymaps/serenity/keymap.c`, and the `modules/` submodule commits. It looks for serenity-qmk in `../serenity-qmk`; set `SERENITY_QMK` to use another path.

It warns when serenity-qmk's module list changes. QMK 0.26 has no `keymap.json` modules, so a new module also has to be added by hand to `SERENITY_MODULES` in the Serenity keymap's `rules.mk` and to `compat.h`/`compat.c`.

## How Serenity runs on QMK 0.26

serenity-qmk targets QMK 0.28 and later. Everything that bridges the gap lives in `keyboards/thock/conundrum/keymaps/serenity`, so `users/serenity` stays an exact copy:

| File | What |
|---|---|
| `compat.h` | Stands in for the header QMK 0.28 generates from `keymap.json`: the modules' keycodes (`SOSM_*`, `SELWORD`, `QK_LLCK`, …), and `set_single_default_layer()` |
| `compat.c` | Calls the modules' hooks in QMK 0.28's order: Custom Shift Keys, Stackable One-Shot and Select Word before `process_record_user`, Layer Lock after it |
| `process_layer_lock.h` | Layer Lock comes from `modules/getreuer/layer_lock`, the module QMK's built-in Layer Lock grew out of |
| `serenity_introspection.c` | Compiles the Custom Shift Keys table with the module code that reads it |
| `rules.mk` | Builds the modules as plain sources and turns LTO off (QMK warns it often fails on `arm_atsam`, and there's room) |

Left out for now: Chordal Hold (QMK 0.28), which `users/serenity/tapping.c` uses for the arrow cluster. On 0.26 its `get_chordal_hold()` is never called, so a fast roll across the arrows may occasionally fire a modifier.

## Diagnosing freezes

A diagnostic build logs the touch sensing to the console and recovers from freezes on its own:

```sh
scripts/build.sh serenity -e EC_TOUCH_DEBUG=yes
qmk console -t | tee -a ~/conundrum-console.log   # leave running while you use the keyboard
```

It adds (all off in normal builds):

- **Status lines** every 2 seconds: `ec ok` with the measurement rate, the library's error code and state, the strongest key and the range of key baselines. `ec STALL`/`ec ERROR` (followed by every key's readings) if measuring stops or the library reports an error.
- **A watchdog**: if the main loop stops for 250ms, the chip resets itself, like a quick replug.
- **Breadcrumbs** in backup RAM, which survives that reset. After each status line, `ec boot` gives the reset cause (`01` power on, `20` watchdog, `40` software reset or freeze) and `last=`, where the previous run was (`enum breadcrumb` in `tmk_core/protocol/arm_atsam/watchdog.h`). `ec freeze` gives the instruction it was stuck on, whether it was inside an interrupt, and which interrupts were pending and running; a crash records itself the same way.

To turn a freeze address into a function and line, keep the build's `.build/thock_conundrum_serenity.elf` and run `arm-none-eabi-addr2line -f -e <elf> <pc>` (the toolchain is in `~/Library/Application Support/qmk/bin`).

The console only empties while the host reads it, so a sleeping Mac makes this build wait on it and reset every 15 seconds or so. That's the logging, not the keyboard.

## Editor setup (VS Code + clangd)

1. Accept VS Code's prompt to install the recommended extensions (includes clangd and cSpell).
2. Generate a compile database covering both keymaps:

   ```sh
   scripts/compiledb.py              # both keymaps (builds each one)
   scripts/compiledb.py serenity     # refresh just one, keeping the other
   ```

`compile_commands.json` is gitignored and holds machine-specific paths. Re-run the script after changing a `rules.mk` or `config.h`, then **clangd: Restart language server**.

`.clangd` drops GCC's `-D__STDC_VERSION__=202311L` from the database: GCC 15 defaults to C23, but QMK builds with `-std=gnu11`, and with both clang's `stdbool.h` leaves `bool` undefined.

Spelling: `cspell.json` checks this repo's own files (not QMK's), and `project-words.txt` holds the extra words. To check from the command line: `npx cspell lint --no-progress`.

## GitHub Actions

Every push builds both keymaps in QMK's Docker image and publishes the firmware on the [Releases](../../releases/tag/latest) page.

## Credits

- [Thock](https://thock.co) and Marcin Chmiel for the Conundrum and its QMK support
- [QMK](https://qmk.fm), and Massdrop for the `arm_atsam` support
- [Pascal Getreuer](https://getreuer.info) for the Custom Shift Keys, Select Word and Layer Lock modules

QMK is GPL licensed. `lib/ec_touch` is Thock's prebuilt library and isn't covered by that license; it's included as Thock published it in thockco/qmk.
