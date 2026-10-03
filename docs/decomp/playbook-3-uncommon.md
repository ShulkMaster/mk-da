# Matching playbook, tier 3: uncommon

Rules scored 2 or 3, ordered by score. Flag changes are coordinator steps:
send gate the evidence; agents never edit `configure.py`. Format and
scoring: [tier 1](playbook-1-core.md).

## U02

lmw/stmw prologues. Score: 3

IF retail saves registers with `stmw`/`lmw` and ours calls
`_savegpr`/`_restgpr`, REQUIRE the same in several functions of the lib,
TRY lib-scope `-use_lmw_stmw on`.

- Exemplar: `get_sound_data`; hvqm4player lib applied.

## U06

Function order. Score: 3

IF bytes match but DecompStudio shows `[order]`, REQUIRE retail addresses,
TRY moving definitions into retail order with real forward declarations.

- Exemplars: `_MotionComp_00/_10/_01/_11`; `GetAotBasis` applied.

## C06

Real aggregate for frame size. Score: 2

IF the body is exact but retail's frame is larger, REQUIRE every loaded byte
of the candidate aggregate to be consumed, TRY a real local array or struct
the code reads (never an unused pad).

- Exemplar: `HVQM4DecodeAdpcmCh2` (`u8 header[4]`, frame 0x30 -> 0x38).

## C07

Named loaded words. Score: 2

IF a copy reorders loads and stores, REQUIRE that each loaded word is used,
TRY naming every loaded word in a local so all loads precede the stores.

- Exemplar: `IntraAotBlock` (OrgBlock `temp0..temp3`).

## C10

Cache a field before dispatch. Score: 2

IF retail reads a field once before a switch or call chain, REQUIRE that the
field is not written in between, TRY a real local holding it.

- Exemplar: `HVQM4DecSoundDecode` (cached channel count).

## C11

Bounds in the loop's own units. Score: 2

IF prologue scheduling differs while the loop body is exact, REQUIRE that a
bound or end pointer is written in byte arithmetic the retail code does not
need, TRY expressing it in the loop's element units (`end = out + (width >>
1)` instead of `(u32*)(row + (width << 1 & ~3))`). The hidden term changes
which prologue values are computed first.

- Exemplar: `vdisp_copy_frame` (seven prologue rows, closed).

## C13

Name the real base quantity. Score: 2

IF size arithmetic has the right operations but the wrong operand staging,
REQUIRE a real base value in retail (pixel count, extent) and its type, TRY
naming it first (`u32 pixels = width * height;`) and deriving the scaled
sizes from it. Never a copy of an existing local.

- Exemplar: `decv_init` (luma pixel count before the chroma expansion closed
  it).

## R03

Last-resort goto. Score: 2

IF the retail CFG jumps from inside a wait loop to a shared end-of-iteration
block, REQUIRE three measured structured forms that fail, TRY one local
`goto` to that block and cite the measurements (AGENTS.md exception).

- Exemplar: `gop_decode`.

## U03

Per-literal read-only strings. Score: 2

IF string literals land in `.data` (or one pooled object) but retail has one
local `@NNN` per literal in `.rodata`, REQUIRE ELF local symbols, TRY
`-str reuse,readonly` (not `pool`).

- Exemplars: HVQM4Bufa, HVQM4Bufv applied.

## U04

Reverse `.sbss` declaration order. Score: 2

IF section totals match but raw symbol offsets are reversed, REQUIRE retail
offsets from the ELF, TRY declaring uninitialized statics in reverse retail
order (GC/1.3.2 emits them reversed). Objdiff's per-symbol 100% does not
check offsets.

- Exemplars: HVQM4PlayerEx, hvqm4play applied.

## U12

Literal over-allocation. Score: 2

IF retail allocates more bytes than the verified struct it accesses, REQUIRE
the literal allocator argument and complete access, stride and layout
evidence, TRY keeping the request literal with the minimal verified type and
a comment; do not invent trailing fields to make `sizeof` match.

- Exemplar: `HVQM4DecSoundCreate` (allocates 0x20, context verified 0x18).
