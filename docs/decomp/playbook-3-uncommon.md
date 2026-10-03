# Matching playbook, tier 3: uncommon (flags, inline, layout)

Rules ordered by score. Flag changes are coordinator steps: send gate the
evidence; agents never edit `configure.py`. Format and scoring:
[tier 1](playbook-1-core.md).

## U05

Static inline boundaries. Score: 5

IF a reference helper has no retail symbol but is live, or retail shows a
fast path the shared helper cannot reproduce, REQUIRE the retail function set
(ELF symbols) and a measured rejection of the shared form, TRY `static inline`
for live helpers (no out-of-line body), and a small one-purpose inline helper
only when the shared form is measured worse; record the measurement.

- Exemplars: `PrediAotBlock` (`GetMCAotSumOne`), `IntraAotBlock`
  (`GetAotSumOne`); hvqm4dec cleanup applied (35 functions exactly).

## U08

Pikmin same-function exception. Score: 4

IF the frame differs by a small fixed amount and Pikmin's same function uses
`STACK_PAD_VAR` (or another forbidden construct), REQUIRE that exact
construct in that function, TRY copying it verbatim and cite it in a note.
Never extend it to other functions.

- Exemplars: `_readTree` (`STACK_PAD_VAR(2)`), `MCBlockDecDCNest` (Pikmin's
  unused `int j` sets the frame; removing it changes five rows).

## U11

Genuine volatile on shared state. Score: 5

IF retail reloads a variable between a guard and its update with no call or
store in between, REQUIRE the minimal repro to reuse the load on every
compiler version and flag, every honest non-volatile form to fail, the
variable to be truly shared (thread or interrupt state), and a user ruling,
TRY `volatile` on its declaration only (never on accesses), and cite the
evidence in a note. Without that evidence, volatile stays a research trick.
A user may also accept volatile from a strong dirty lead when a function
escalates unmatched. Then the note must say so: "accepted after escalation
because no honest form matched; revisit if a clean form is found", so later
agents know the ruling rests on the failure to match, not on positive
evidence.
MWCC keys load merging on the qualifier, so a `const`-qualified read
(`*(const s32*)&g`, a const pointer local, or a const macro) also reproduces
the reload. That is a fake alias, not an alternative: reject it.

- Exemplars: `_HVQM4PlayerExClose`, `HVQM4PlayerExCreate` (`static volatile
  s32 lib_link_counter;`, one declaration closed both); `vdisp_init`
  (`FrameBuffer`, accepted after escalation from a 90.33 -> 97.19 lead).

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

## U07

Unit-owned data and stripped-helper strings. Score: 2

IF `.data` lacks a global, REQUIRE the ELF OBJECT symbol (scope, size), TRY
defining it in the unit with retail constness. IF `.rodata` has strings that
no retail code references, the linker stripped their function: keep a stripped
helper only under the user's stripped-code ruling; never emit named arrays
for anonymous literals. IF the object owns codec or lookup tables, import
only the ELF OBJECT ranges (size, address, relocation targets), check them
byte for byte against the DOL, and never regenerate them from a formula or
bring in unrelated upstream tables.

- Exemplars: `HVQM4_FILEVERSION` applied; Adp8x tables and `_dect`; `"DoMalloc movie"` in hvqm4play
  pending a user ruling.

## U12

Literal over-allocation. Score: 2

IF retail allocates more bytes than the verified struct it accesses, REQUIRE
the literal allocator argument and complete access, stride and layout
evidence, TRY keeping the request literal with the minimal verified type and
a comment; do not invent trailing fields to make `sizeof` match.

- Exemplar: `HVQM4DecSoundCreate` (allocates 0x20, context verified 0x18).

## U01

extab means C++ exceptions on. Score: 1

IF the object has extab/extabindex entries, REQUIRE the ELF SECTION symbols,
TRY `-Cpp_exceptions on` at lib or object scope.

- Exemplar: hvqm4player lib applied.

## U09

Unsigned literal compares. Score: 1

IF a mode chain compares with `cmplwi`, REQUIRE an unsigned field or unsigned
literals in retail, TRY `== 4U` style literals.

- Exemplar: `gop_decode` applied.

## U10

Remove obsolete reference dispatch support. Score: 1

IF a reference helper or function table becomes unused after C03 restores
retail's direct calls, REQUIRE no retail ELF symbol for it, no table data or
`.rela.data` in the retail object, and no retail bytes that need it, TRY
deleting the table and its typedef. Keep the call shape: when the
reference calls a dispatch helper in that function, a `static inline` helper
with direct dispatch may still be the original form (see C03).

- Exemplar: hvqm4dec `_MotionComp` and `func[]` (`.data` reached 100%).
