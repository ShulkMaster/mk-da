# Matching playbook, tier 2: common

Rules scored 4 or more, ordered by score. Format and scoring:
[tier 1](playbook-1-core.md). The ID letter is the category (C source shape,
U flags, inline and layout, R ceilings and last resorts); the tier is how
often the rule has paid off.

## C05

Declaration order and scope. Score: 25

IF only registers differ and lifetimes look shifted, REQUIRE identical CFG and
memory accesses, TRY one honest change of where a real local is declared
(outer vs block scope, before or after another local's first use).

- Exemplars: `HVQM4DecodeAdpcmCh1` (block-scoped accumulator with an
  advancing cursor), `MotionComp` (per-plane and inner-loop declaration
  scopes), `HVQM4BufvCreate` (node/last/first order, last node set after the
  aligned base), `MCBlockDecMCNest` (GC/1.2.5 colors in declaration order:
  per-plane locals declared at function scope before `nestP`, which retail
  colors last), `gop_decode` (declaration order after output-pointer
  staging), `vdisp_init` (next, framebuffer, p, end).

## C08

In-place derivation. Score: 13

IF retail derives a value in the same register it was loaded into, REQUIRE one
meaning for the variable, TRY computing it in steps in one local (`margin =
width; margin = (640 - margin) & ~1;`).

- Exemplars: `vDispThread_Entry`; `HVQM4DecodePcm16Ch1`/`Ch2`,
  `HVQM4DecodeAdp8xCh2` (`bytes = samples * 2; bytes *= track; code +=
  bytes;` keeps retail's shift/multiply operand order),
  `HVQM4PlayerExCreate` (audio capacity becomes a byte count, then `&= ~127`),
  `vdisp_init` (next-page pointer set from the allocation, then advanced).

## C03

Retail call boundary. Score: 10

IF the reference expands a helper inline or dispatches through a function
table but retail calls each routine directly (or the reverse), REQUIRE retail
`bl` targets and the helper's own symbol, TRY dropping `inline` on helpers
retail calls and replacing table dispatch with direct branches on the real
selector bits.

- Exemplars: `MotionComp`, `PrediAotBlock` (direct `_MotionComp_*` calls on
  separate x/y half-pel flags), `IntraAotBlock` (non-inline `GetAotBasis`
  for both paths), `MCBlockDecMCNest` (Pikmin's `_MotionComp` call kept as a
  `static inline` helper doing the direct dispatch; its inlined parameter
  copies are the separate width value retail colors).

## R04

Offline compiler pre-screen. Score: 8

IF a small localized residue remains, REQUIRE the exact TU command (`ninja -t
commands <object>`), TRY compiling variants locally with the project's
mwcceppc and comparing the symbol's objdump against retail before spending
`try` attempts.

- Exemplars: `HVQM4BufaCreate`, `vdisp_copy_frame`, `MCBlockDecMCNest`
  (Opus tier, each closed on its first try).

## C01

Compound-assignment reassociation. Score: 7

IF one add (or or/xor) has its operands commuted, REQUIRE the same operations
and registers otherwise, TRY changing statement grouping: MWCC rewrites
`x += a + b` as `(a + x) + b`, so fold a constant term into the previous
statement or split the sum into separate `x += a; x += b;` statements so the
running value stays the left operand.

- Exemplars: `HVQM4BufaCreate` (`= product + 64; += nodes; += 32;`),
  `HVQM4BufvCreate` (`data += size; data = (data + 63) & ~63;` stops
  hoisting `size + 63` out of the loop), `_HVQM4PlayerExThread` (`end =
  sizeof(gop); end += pos + gop.dataSize;`).

## C09

Statement order follows the retail schedule. Score: 8

IF loads, stores or pointer advances are ordered differently, REQUIRE
unchanged semantics (no aliasing between the moved accesses), TRY placing
statements in retail's order (compute packed words before storing them,
advance cursors after the stores).

- Exemplars: `MCBlockDecDCNest` (next-row `dP` plus chained row stores),
  `vdisp_copy_frame` applied (75.8 -> 96.5), `gop_decode`
  applied (node output pointer loaded before the size store).

## U07

Unit-owned data and stripped-helper strings. Score: 11

IF `.data` lacks a global, REQUIRE the ELF OBJECT symbol (scope, size), TRY
defining it in the unit with retail constness. IF `.rodata` has strings that
no retail code references, the linker stripped their function: keep a stripped
helper only under the user's stripped-code ruling; never emit named arrays
for anonymous literals. IF the object owns codec or lookup tables, import
only the ELF OBJECT ranges (size, address, relocation targets), check them
byte for byte against the DOL, and never regenerate them from a formula or
bring in unrelated upstream tables. An ordinary unused non-inline function
in a header explains literals with no surviving code body, and its include
position explains their numbering and pool order; values in its body that
only the literals support (flags, line numbers) stay marked unproven.

IF anonymous literals are emitted in the wrong order, the first use may come
from a stripped out-of-line copy: a plain `static` helper that `-inline auto`
expands at its call sites still emits a body (linker-stripped), and that
body's position sets literal numbering. Split a helper only where retail
evidence shows separate source regions (e.g. line-number groups).

- Exemplars: `HVQM4_FILEVERSION` applied; Adp8x tables and `_dect`;
  `mpeg_fullscreen` and `vdisp_init` (`close_sound` split from `close_movie`
  emits "hvqm4play.c" first); `"DoMalloc movie"` in hvqm4play
  pending a user ruling.

## U13

Library optimization level. Score: 6

IF several functions in one library are exact in body but differ only in
prologue, epilogue or load scheduling (or keep CTR loops and folded sign
extensions retail lacks), REQUIRE independent research proofs from different
objects that `#pragma scheduling off` or `optimization_level` closes them
with unchanged honest source, TRY an offline sweep of `-O` levels over every
function in the library (R04) and apply the level with zero regressions at
lib scope; keep single-function flag leads (for example `-opt nocse`) as
notes until a second object agrees.

- Exemplars: renderware lib `-O2,p` (baerr, bafsys, baimmedi, baimras,
  resmem, rwstring, osintf closed with existing source; `_rwResHeapAlloc`
  `-opt nocse` held as a lead).

## C16

Genuine byte locals. Score: 6

IF retail loads a byte (`lbz`) and converts it to float or subtracts it as a
small integer, but a source local widened to `u32`/`s32` swaps registers or
adds conversions, REQUIRE the byte-sized load in retail, TRY keeping a
block-local `u8` for the loaded value and casting at the point of use (a
signed delta gets its own explicit cast).

- Exemplars: `_rwGeneratePerspClippedVertexZLO` (block-local `u8` plus a
  direct interpolation expression), `_rwGeneratePerspClippedVertexXHI` (21
  rows closed versus `u32`).

## C04

Newer-version control flow. Score: 5

IF retail has guards, fast paths or fills the reference lacks, REQUIRE a
version marker (`HVQM4_FILEVERSION` "HVQM4 1.5" vs Pikmin's older decoder)
and the retail branch structure, TRY adding only the retail-proven branch
around the reference body.

- Exemplars: `IpicPlaneDec` (guarded first/last rows on a counted
  `nblocks_v`), `MCBlockDecDCNest` (`nbasis == 8` direct DC fill),
  `IntraAotBlock` (one-basis fast path, pre-shifted DC).

## U05

Static inline boundaries. Score: 12

IF a reference helper has no retail symbol but is live, or retail shows a
fast path the shared helper cannot reproduce, REQUIRE the retail function set
(ELF symbols) and a measured rejection of the shared form, TRY `static inline`
for live helpers (no out-of-line body), and a small one-purpose inline helper
only when the shared form is measured worse; record the measurement.

- Exemplars: `PrediAotBlock` (`GetMCAotSumOne`), `IntraAotBlock`
  (`GetAotSumOne`); hvqm4dec cleanup applied (35 functions exactly).

## C02

Signed extrema. Score: 4

IF retail compares extrema with `cmpw` where the reference keeps `u8`
min/max, REQUIRE byte loads that still feed u8 outputs, TRY `int` extrema
while keeping the sampled value `u8`.

- Exemplars: `GetAotBasis`, `GetMCAotBasis`.

## U08

Pikmin same-function exception. Score: 4

IF the frame differs by a small fixed amount and Pikmin's same function uses
`STACK_PAD_VAR` (or another forbidden construct), REQUIRE that exact
construct in that function, TRY copying it verbatim and cite it in a note.
Never extend it to other functions.

- Exemplars: `_readTree` (`STACK_PAD_VAR(2)`), `MCBlockDecDCNest` (Pikmin's
  unused `int j` sets the frame; removing it changes five rows).

## U11

Genuine volatile on shared state. Score: 4

IF retail reloads a variable between a guard and its update with no call or
store in between, REQUIRE the minimal repro to reuse the load on every
compiler version and flag, every honest non-volatile form to fail, the
variable to be truly shared (thread or interrupt state), and a user ruling,
TRY `volatile` on its declaration only (never on accesses), and cite the
evidence in a note. Without that evidence, volatile stays a research trick.
A qualifier that reproduces the reload proves the reload, not the original
qualifier; keep the note's qualification.
A user may also accept volatile from a strong dirty lead when a function
escalates unmatched. Then the note must say so: "accepted after escalation
because no honest form matched; revisit if a clean form is found", so later
agents know the ruling rests on the failure to match, not on positive
evidence.
MWCC keys load merging on the qualifier, so a `const`-qualified read
(`*(const s32*)&g`, a const pointer local, or a const macro) also reproduces
the reload. That is a fake alias, not an alternative: reject it.

- Exemplars: `_HVQM4PlayerExClose`, `HVQM4PlayerExCreate` (`static volatile
  s32 lib_link_counter;`, one declaration closed both). Revisit worked once:
  `vdisp_init`'s escalation-accepted `FrameBuffer` volatile was later
  replaced by honest C (three real reads of `FrameBuffer[0]` for start, end
  and cursor, plus C05 declaration order), so always reopen these.
