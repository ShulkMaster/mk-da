# Mortal Kombat: Deadly Alliance

GameCube decompilation project initialized from
[encounter/dtk-template](https://github.com/encounter/dtk-template)
(template revision `95a941f755919ebe50c1725a4ce73524470e7a02`).

Supported game copy: **GMKE5D, USA, revision 1**.

Original game files and generated build artifacts are ignored by Git.

GitHub Actions builds GMKE5D using the private image
`ghcr.io/shulkmaster/mk-da-build:main`, supplied by the private
`ShulkMaster/mk-da-build` sibling repository. CI verifies the retail SHA-1 and
publishes symbol maps and progress reports. See the
[build workflow](https://github.com/ShulkMaster/mk-da/actions/workflows/build.yml)
and [CI setup](docs/github_actions.md).

## Build

Install Python 3 and Ninja. The build automatically downloads its pinned tools;
Linux and macOS use wibo to run the CodeWarrior linker.

Place your own USA revision 1 disc image in `orig/GMKE5D/`. DTK supports RVZ
directly, as well as ISO and other disc formats. Alternatively, place the
extracted executable at `orig/GMKE5D/sys/main.dol`.

```sh
python3 configure.py
ninja
```

The build verifies `build/GMKE5D/main.dol` against SHA-1
`3560bd0c0814d2f1ceaa819946d71dad4e488d2e`.

When given a disc image, DTK extracts the executable into `orig/GMKE5D/sys/`
on the first build.

## Initial configuration

- `configure.py` selects GMKE5D and registers source objects and build settings.
- `config/GMKE5D/config.yml` identifies and verifies the original executable.
- `config/GMKE5D/symbols.txt` starts with symbols imported from the disc's
  `files/mk5gc_release.elf`, whose conversion to DOL exactly matches `sys/main.dol`.
  DTK adds analysis symbols and regenerates the `_ctors` and `_dtors` labels.
- `config/GMKE5D/splits.txt` records section alignment and the required C++
  exception runtime split. The remaining objects are generated automatically;
  translation unit boundaries still need research.
- GC/1.3.2 links the project successfully. GC/1.2.5n is very likely the Dolphin
  SDK compiler, pending comparisons with more SDK files. The game compiler
  remains unconfirmed.
- Generated `objdiff.json` supports the future matching workflow.

See [the setup documentation](docs/getting_started.md) and
[split configuration documentation](docs/splits.md) for the next steps.

## Attribution

This project builds on work from the
[Metroid Prime decompilation project](https://github.com/PrimeDecomp/prime).
Credit and thanks to the PrimeDecomp contributors.

