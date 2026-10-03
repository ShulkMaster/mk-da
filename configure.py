#!/usr/bin/env python3

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

DEFAULT_VERSION = 0
VERSIONS = [
    "GMKE5D",
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

if not Path("extern/musyx/include").is_dir():
    try:
        subprocess.run(["git", "submodule", "update", "--init", "--recursive"], check=True)
    except (OSError, subprocess.CalledProcessError) as e:
        sys.exit(f"Failed to fetch git submodules ({e}); run `git submodule update --init --recursive`.")

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
if not config.non_matching:
    config.asm_dir = None

config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.3"
config.objdiff_tag = "v3.6.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")
if args.map:
    config.ldflags.append("-mapunused")

config.reconfig_deps = []

asm_sequence_manifest = Path("config") / config.version / "asm_sequences.json"
asm_sequence_config = json.loads(asm_sequence_manifest.read_text(encoding="utf-8"))
asm_sequence_root = config.build_dir / config.version
asm_unit_root = asm_sequence_root / "units"
asm_units = asm_sequence_config.get("units", [])
config.reconfig_deps.append(asm_sequence_manifest)
config.custom_build_rules = [
    {
        "name": "asm_sequences",
        "command": (
            "$python tools/generate_asm_sequences.py "
            '--manifest "$manifest" --version "$version" --build-root "$build_root"'
        ),
        "description": "RETAIL ASM SEQUENCES $out",
        "restat": True,
    },
]
config.custom_build_steps = {
    "pre-compile": [
        {
            "outputs": [
                asm_sequence_root / "include" / asm_sequence_config["output"],
                *(asm_unit_root / unit["name"] for unit in asm_units),
            ],
            "rule": "asm_sequences",
            "inputs": list(
                dict.fromkeys(
                    [
                        *(
                            asm_sequence_root
                            / "asm"
                            / entry.get("assembly", asm_sequence_config["assembly"])
                            for entry in asm_sequence_config["functions"]
                        ),
                        *(
                            asm_sequence_root / "asm" / unit.get("assembly", unit["name"])
                            for unit in asm_units
                        ),
                    ]
                )
            ),
            "implicit": [Path("tools/generate_asm_sequences.py"), asm_sequence_manifest],
            "variables": {
                "manifest": asm_sequence_manifest,
                "version": config.version,
                "build_root": config.build_dir,
            },
        },
    ],
}

config.scratch_preset_id = None

cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",
    "-i include",
    f"-i {config.build_dir}/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

if args.debug:
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

cflags_musyx = [
    "-proc gekko",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i extern/musyx/include",
    "-inline auto,depth=4",
    "-O4,p",
    "-fp hard",
    "-enum int",
    "-sym on",
    "-Cpp_exceptions off",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

config.linker_version = "GC/1.3.2"


def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


def MusyX(objects: List[Object], major: int = 2, minor: int = 0, patch: int = 0) -> Dict[str, Any]:
    return {
        "lib": "musyx",
        "mw_version": "GC/1.3.2",
        "src_dir": "extern/musyx/src",
        "cflags": [
            *cflags_musyx,
            f"-DMUSY_VERSION_MAJOR={major}",
            f"-DMUSY_VERSION_MINOR={minor}",
            f"-DMUSY_VERSION_PATCH={patch}",
        ],
        "progress_category": "musyx",
        "objects": objects,
    }


Matching = True
NonMatching = False
Equivalent = config.non_matching


def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = False
config.warn_missing_source = False
config.libs = [
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_runtime,
        "progress_category": "sdk",
        "objects": [
            Object(Matching, "MSL_C.PPCEABI.bare.H/abort_exit.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/alloc.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ansi_files.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ansi_fp.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/buffer_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/char_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ctype.c", extra_cflags=["-char signed", "-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/direct_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/errno.c", extra_cflags=["-char signed", "-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/file_io.c", extra_cflags=["-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/file_pos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/locale.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/mbstring.c", extra_cflags=["-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/mem.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/mem_funcs.c", extra_cflags=["-char signed", "-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/misc_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/printf.c", extra_cflags=["-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/qsort.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/rand.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/scanf.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/signal.c", extra_cflags=["-inline deferred,auto", "-char signed"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/string.c", extra_cflags=["-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/strtold.c", extra_cflags=["-char signed", "-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/strtoul.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/uart_console_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/float.c", extra_cflags=["-char signed", "-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/wchar_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_atan2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_fmod.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_log.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_pow.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_rem_pio2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_cos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_rem_pio2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_sin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_tan.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_atan.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_copysign.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_cos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_floor.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_frexp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_ldexp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_modf.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_nextafter.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_sin.c", extra_cflags=["-char signed", "-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_tan.c", extra_cflags=["-char signed", "-inline deferred,auto"]),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_atan2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_fmod.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_log.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_pow.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/math_ppc.c"),
        ],
    },
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_runtime,
        "progress_category": "sdk",
        "objects": [
            Object(Matching, "Runtime.PPCEABI.H/__mem.c"),
            Object(Matching, "Runtime.PPCEABI.H/__va_arg.c"),
            Object(Matching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(Matching, "Runtime.PPCEABI.H/NewMore.cp", extra_cflags=["-Cpp_exceptions on", "-RTTI on", "-str nopool"]),
            Object(Matching, "Runtime.PPCEABI.H/NMWException.cp", extra_cflags=["-Cpp_exceptions on", "-RTTI on", "-str nopool"]),
            Object(Matching, "Runtime.PPCEABI.H/GCN_mem_alloc.c", extra_cflags=["-str nopool"]),
        ],
    },
    DolphinLib("odemuexi2", [Object(Matching, "dolphin/odemuexi2/DebuggerDriver.c")]),
    {
        "lib": "TRK_MINNOW_DOLPHIN",
        "mw_version": "GC/1.3.2",
        "cflags": [*cflags_runtime, "-sdata 0", "-sdata2 0", "-inline off", "-enum min"],
        "progress_category": "sdk",
        "objects": [
            Object(Matching, "dolphin/trk_minnow_dolphin/target_options.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/mslsupp.c", mw_version="GC/1.3", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/mainloop.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/nubevent.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/notify.c", extra_cflags=["-inline on"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/mutex_TRK.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/targcont.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/main_TRK.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/nubinit.c", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/msg.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/msgbuf.c", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/serpoll.c", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/dispatch.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/usr_put.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/msghndlr.c", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/support.c", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/flush_cache.c"),
            Object(Matching, "dolphin/trk_minnow_dolphin/mem_TRK.c", mw_version="GC/1.3", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/__exception.s", src_dir=asm_unit_root, generated=True),
            Object(Matching, "dolphin/trk_minnow_dolphin/targimpl.c", mw_version="GC/1.3", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/dolphin_trk.c", mw_version="GC/1.3", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min", "-inline auto,deferred"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/dolphin_trk_glue.c", mw_version="GC/1.3", cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0", "-enum min", "-inline auto,deferred", "-str reuse,nopool,readonly"]),
            Object(Matching, "dolphin/trk_minnow_dolphin/targsupp.s", src_dir=asm_unit_root, generated=True),
            Object(Matching, "dolphin/trk_minnow_dolphin/mpc_7xx_603e.c"),
        ],
    },
    DolphinLib(
        "exi",
        [
            Object(Matching, "dolphin/exi/EXIBios.c"),
            Object(Matching, "dolphin/exi/EXIUart.c"),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(Matching, "dolphin/gx/GXInit.c", extra_cflags=["-opt nopeephole", '-pragma "opt_unroll_loops off"']),
            Object(Matching, "dolphin/gx/GXFifo.c"),
            Object(Matching, "dolphin/gx/GXDisplayList.c"),
            Object(Matching, "dolphin/gx/GXFrameBuf.c"),
            Object(Matching, "dolphin/gx/GXTransform.c", extra_cflags=["-fp_contract off"]),
            Object(Matching, "dolphin/gx/GXGeometry.c"),
            Object(Matching, "dolphin/gx/GXBump.c"),
            Object(Matching, "dolphin/gx/GXTev.c"),
            Object(Matching, "dolphin/gx/GXPixel.c"),
            Object(Matching, "dolphin/gx/GXStubs.c"),
            Object(Matching, "dolphin/gx/GXMisc.c"),
            Object(Matching, "dolphin/gx/GXLight.c"),
            Object(Matching, "dolphin/gx/GXAttr.c"),
            Object(Matching, "dolphin/gx/GXTexture.c"),
            Object(Matching, "dolphin/gx/GXPerf.c"),
        ],
    ),
    DolphinLib("base", [Object(Matching, "dolphin/base/PPCArch.c")]),
    DolphinLib(
        "card",
        [
            Object(Matching, "dolphin/card/CARDBios.c"),
            Object(Matching, "dolphin/card/CARDUnlock.c"),
            Object(Matching, "dolphin/card/CARDMount.c"),
            Object(Matching, "dolphin/card/CARDFormat.c"),
            Object(Matching, "dolphin/card/CARDRdwr.c"),
            Object(Matching, "dolphin/card/CARDBlock.c"),
            Object(Matching, "dolphin/card/CARDDir.c"),
            Object(Matching, "dolphin/card/CARDDelete.c"),
            Object(Matching, "dolphin/card/CARDWrite.c"),
            Object(Matching, "dolphin/card/CARDRead.c"),
            Object(Matching, "dolphin/card/CARDCreate.c"),
            Object(Matching, "dolphin/card/CARDOpen.c"),
            Object(Matching, "dolphin/card/CARDCheck.c"),
            Object(Matching, "dolphin/card/CARDStat.c"),
            Object(Matching, "dolphin/card/CARDStatEx.c"),
            Object(Matching, "dolphin/card/CARDNet.c"),
        ],
    ),
    DolphinLib("pad", [Object(Matching, "dolphin/pad/Pad.c")]),
    DolphinLib("vi", [Object(Matching, "dolphin/vi/vi.c")]),
    DolphinLib("si", [Object(Matching, "dolphin/si/SIBios.c"), Object(Matching, "dolphin/si/SISamplingRate.c")]),
    DolphinLib(
        "ar",
        [
            Object(Matching, "dolphin/ar/ar.c"),
            Object(Matching, "dolphin/ar/arq.c"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(Matching, "dolphin/ai.c"),
        ],
    ),
    DolphinLib(
        "db",
        [
            Object(Matching, "dolphin/db.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(Matching, "dolphin/dsp/dsp.c"),
            Object(Matching, "dolphin/dsp/dsp_debug.c"),
            Object(Matching, "dolphin/dsp/dsp_task.c"),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(Matching, "dolphin/dvd/dvdlow.c"),
            Object(Matching, "dolphin/dvd/dvdfs.c"),
            Object(Matching, "dolphin/dvd/dvd.c"),
            Object(Matching, "dolphin/dvd/dvdqueue.c"),
            Object(Matching, "dolphin/dvd/dvderror.c"),
            Object(Matching, "dolphin/dvd/fstload.c"),
        ],
    ),
    DolphinLib(
        "mtx",
        [
            Object(Matching, "dolphin/mtx/mtx.c"),
            Object(Matching, "dolphin/mtx/mtxvec.c"),
            Object(Matching, "dolphin/mtx/mtx44.c"),
            Object(Matching, "dolphin/mtx/vec.c"),
            Object(Matching, "dolphin/mtx/quat.c"),
        ],
    ),
    DolphinLib("amcstubs", [Object(Matching, "dolphin/amcstubs/AmcExi2Stubs.c")]),
    DolphinLib("odenotstub", [Object(Matching, "dolphin/odenotstub/odenotstub.c")]),
    DolphinLib(
        "os",
        [
            Object(Matching, "dolphin/os/OS.c", extra_cflags=["-opt nopeephole"],
                   symbol_aliases={"__OSDBJUMPEND": "__OSSetExceptionHandler"}),
            Object(Matching, "dolphin/os/OSAlarm.c"),
            Object(Matching, "dolphin/os/OSAlloc.c"),
            Object(Matching, "dolphin/os/OSResetSW.c"),
            Object(Matching, "dolphin/os/OSRtc.c"),
            Object(Matching, "dolphin/os/OSFont.c"),
            Object(Matching, "dolphin/os/OSArena.c"),
            Object(Matching, "dolphin/os/OSAudioSystem.c"),
            Object(Matching, "dolphin/os/OSCache.c", extra_cflags=["-opt nopeephole"]),
            Object(Matching, "dolphin/os/OSContext.c", extra_cflags=["-opt nopeephole"]),
            Object(Matching, "dolphin/os/OSError.c"),
            Object(Matching, "dolphin/os/OSInterrupt.c", extra_cflags=["-opt nopeephole"]),
            Object(Matching, "dolphin/os/OSLink.c"),
            Object(Matching, "dolphin/os/OSMemory.c"),
            Object(Matching, "dolphin/os/OSMutex.c"),
            Object(Matching, "dolphin/os/OSReset.c"),
            Object(Matching, "dolphin/os/OSReboot.c", extra_cflags=["-opt nopeephole"]),
            Object(Matching, "dolphin/os/OSThread.c"),
            Object(Matching, "dolphin/os/OSTime.c"),
            Object(Matching, "dolphin/os/OSSync.c", extra_cflags=["-opt nopeephole"]),
            Object(Matching, "dolphin/os/__ppc_eabi_init.cpp", extra_cflags=["-opt nopeephole"]),
            Object(Matching, "dolphin/os/__start.c"),
        ],
    ),
    MusyX(
        [
            Object(Matching, "musyx/runtime/seq.c"),
            Object(Matching, "musyx/runtime/synth.c"),
            Object(Matching, "musyx/runtime/seq_api.c"),
            Object(Matching, "musyx/runtime/snd_synthapi.c"),
            Object(Matching, "musyx/runtime/stream.c"),
            Object(Matching, "musyx/runtime/synthdata.c"),
            Object(Matching, "musyx/runtime/synthmacros.c"),
            Object(Matching, "musyx/runtime/synthvoice.c"),
            Object(Matching, "musyx/runtime/synth_ac.c"),
            Object(Matching, "musyx/runtime/synth_adsr.c"),
            Object(Matching, "musyx/runtime/synth_dbtab.c"),
            Object(Matching, "musyx/runtime/synth_vsamples.c"),
            Object(Matching, "musyx/runtime/s_data.c"),
            Object(Matching, "musyx/runtime/hw_dspctrl.c"),
            Object(Matching, "musyx/runtime/hw_volconv.c"),
            Object(Matching, "musyx/runtime/snd3d.c"),
            Object(Matching, "musyx/runtime/snd_init.c"),
            Object(Matching, "musyx/runtime/snd_math.c"),
            Object(Matching, "musyx/runtime/snd_midictrl.c"),
            Object(Matching, "musyx/runtime/snd_service.c"),
            Object(Matching, "musyx/runtime/hardware.c"),
            Object(Matching, "musyx/runtime/hw_aramdma.c"),
            Object(Matching, "musyx/runtime/hw_dolphin.c"),
            Object(Matching, "musyx/runtime/hw_memory.c"),
            Object(Matching, "musyx/runtime/dsp_import.c"),
        ]
    ),
    {
        "lib": "hvqm4player",
        "mw_version": "GC/1.3.2",
        "cflags": [*cflags_base, "-Cpp_exceptions on", "-str reuse,readonly", "-use_lmw_stmw on"],
        "progress_category": "hvqm4",
        "objects": [
            Object(Matching, "hvqm4/HVQM4Bufa.c"),
            Object(Matching, "hvqm4/HVQM4Bufv.c"),
            Object(
                Matching,
                "hvqm4/hvqm4play.c",
                extra_cflags=["-i extern/musyx/include", "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN"],
            ),
            Object(Matching, "hvqm4/HVQM4PlayerEx.c"),
        ],
    },
    {
        "lib": "movie",
        "mw_version": "GC/1.3.2",
        "cflags": [*cflags_base, "-Cpp_exceptions on", "-str reuse,readonly", "-use_lmw_stmw on"],
        "progress_category": "movie",
        "objects": [
            Object(NonMatching, "movie/mwMoviePlayerGC.cpp"),
            Object(NonMatching, "movie/seqfile.c"),
            Object(NonMatching, "movie/errcode.c"),
            Object(NonMatching, "movie/movieplayer.cpp"),
            Object(NonMatching, "movie/mwMemNewDelete.cpp"),
        ],
    },
    {
        "lib": "hvqm4dec",
        "mw_version": "GC/1.2.5",
        "cflags": [*cflags_base, "-fp_contract off"],
        "progress_category": "hvqm4",
        "objects": [
            Object(Matching, "hvqm4/HVQM4Adpcm.c"),
            Object(Matching, "hvqm4/HVQM4Pcm16.c"),
            Object(Matching, "hvqm4/HVQM4Adp8x.c"),
            Object(Matching, "hvqm4/HVQM4DecSnd.c"),
            Object(Matching, "hvqm4/hvqm4dec.c"),
        ],
    },
    {
        "lib": "renderware",
        "mw_version": "GC/1.3.2",
        "cflags": [*(flag for flag in cflags_base if flag not in ("-O4,p", "-fp_contract on")), "-O2,p", "-fp_contract off"],
        "progress_category": "renderware",
        "objects": [
            Object(NonMatching, "renderware/babinary.c"),
            Object(Matching, "renderware/bacolor.c"),
            Object(Matching, "renderware/baerr.c"),
            Object(Matching, "renderware/bafsys.c"),
            Object(Matching, "renderware/baimmedi.c"),
            Object(NonMatching, "renderware/bamatrix.c"),
            Object(NonMatching, "renderware/bamemory.c"),
            Object(NonMatching, "renderware/baresour.c"),
            Object(Matching, "renderware/bastream.c"),
            Object(Matching, "renderware/batkbin.c"),
            Object(Matching, "renderware/batkreg.c"),
            Object(NonMatching, "renderware/bavector.c"),
            Object(Matching, "renderware/resmem.c"),
            Object(Matching, "renderware/rwstring.c"),
            Object(Matching, "renderware/osintf.c"),
            Object(Matching, "renderware/babbox.c"),
            Object(NonMatching, "renderware/baframe.c"),
            Object(NonMatching, "renderware/baimage.c"),
            Object(Matching, "renderware/baimras.c"),
            Object(NonMatching, "renderware/baraster.c"),
            Object(NonMatching, "renderware/baresamp.c"),
            Object(NonMatching, "renderware/basync.c"),
            Object(Matching, "renderware/batypehf.c"),
            Object(NonMatching, "renderware/palquant.c"),
            Object(NonMatching, "renderware/dl2drend.c"),
            Object(NonMatching, "renderware/dlrendst.c"),
            Object(NonMatching, "renderware/dltoken.c"),
            Object(Matching, "renderware/bapipe.c"),
            Object(NonMatching, "renderware/nodeCullTriangle.c"),
            Object(NonMatching, "renderware/nodeImmMangleLineIndices.c"),
            Object(NonMatching, "renderware/nodeImmMangleTriangleIndices.c"),
            Object(Matching, "renderware/nodeImmRenderSetup.c"),
            Object(Matching, "renderware/nodeImmStash.c"),
            Object(Matching, "renderware/nodeSubmitLine.c"),
            Object(Matching, "renderware/nodeSubmitTriangle.c"),
            Object(NonMatching, "renderware/nodeTransform.c"),
            Object(NonMatching, "renderware/p2altmdl.c"),
            Object(NonMatching, "renderware/p2clpcom.c"),
            Object(Matching, "renderware/p2core.c"),
            Object(NonMatching, "renderware/p2define.c"),
            Object(NonMatching, "renderware/p2dep.c"),
            Object(NonMatching, "renderware/p2heap.c"),
            Object(NonMatching, "renderware/p2renderstate.c"),
            Object(Matching, "renderware/p2resort.c"),
            Object(Matching, "renderware/babinwor.c"),
            Object(Matching, "renderware/basector.c"),
            Object(Matching, "renderware/nodeMaterialScatter.c"),
            Object(NonMatching, "renderware/nodePostLight.c"),
        ],
    },
]


def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    if not config.non_matching:
        return objects
    if module_id == 0:
        return objects + ["dummy.c"]
    return objects


config.progress_categories = [
    ProgressCategory("sdk", "Dolphin SDK"),
    ProgressCategory("musyx", "MusyX"),
    ProgressCategory("hvqm4", "HVQM4"),
    ProgressCategory("movie", "Movie player"),
    ProgressCategory("renderware", "RenderWare"),
]
config.progress_each_module = args.verbose
config.progress_report_args = [
]

if args.mode == "configure":
    generate_build(config)
elif args.mode == "progress":
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
