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

## C14

Named products against fused multiply-add. Score: 2

IF retail keeps separate `fmuls`/`fadds` where our build emits `fmadds`
under `-fp_contract on`, REQUIRE that the rest of the library still matches
with contraction on, TRY assigning each product to its own `f32` local
before the sum instead of changing the flag.

- Exemplars: `VectorMultVector` (70.92 -> 100 with named products, then
  C05).

## C15

Conditional call target. Score: 2

IF retail selects between two functions and calls through `r12` with no
extra `mr` into `r12`, while a function-pointer local adds moves, REQUIRE
that both targets are real retail symbols, TRY calling the conditional
directly, `(cond ? a : b)(args)`, keeping retail's branch sense (`beq` vs
`bne` decides which arm comes first).

- Exemplars: `_rwForAllEdges` (100; the reversed form 99.67).

## C17

Byte-pointer base for read-only inputs. Score: 2

IF a `void*` input makes MWCC keep a field address (an extra `addi`) that
retail folds into each access, REQUIRE that retail only reads through the
input, TRY typing the input as a byte pointer (`u8*`; `const` is neutral
here) so field offsets fold into the loads.

- Exemplars: `_rpWriteSectRights` (98.03 -> 100; `const void*` stays
  98.03, `u8*` and `const u8*` both close; matches the independent
  `_rpWriteWorldRights` shape).

## C18

Conditional expression for clamps. Score: 2

IF retail compares before loading the default constant, but a source
`x = default; if (...) x = other;` hoists the constant load ahead of the
compare, REQUIRE the same operands in both forms, TRY a single conditional
expression (`x = cond ? other : default;`).

- Exemplars: `RwStreamWriteReal` (chunk-size select closed the compare and
  load order).

## C19

Boolean result form. Score: 2

IF a call's result becomes a 0/1 value and the shift/`mr` order around it
differs (`cntlzw`; `srwi` into `r0` then `mr` vs straight into the saved
register), REQUIRE the same call and use, TRY the other spelling: `!f()`
and `f() == 0` materialize differently in MWCC.

- Exemplars: `RwStreamClose` (`!` closed the shift into `r0` then `mr r31`).

## C20

Signed index from a pointer difference. Score: 2

IF retail derives an element index by dividing a pointer difference by the
record size with a signed divide (`divw`, or a signed shift sequence), and
an unsigned index collapses register webs or drops a temporary, REQUIRE the
signed divide in retail, TRY an `s32` index (`(p - base) / size` on typed
pointers or a signed byte difference) instead of `u32`.

- Exemplars: `PipelineNodeDestroy` (99.39 -> 100; signed divide by 40).

## C21

Equality operand order. Score: 2

IF coloring differs around an equality test against a constant, especially
in recursive code MWCC inlines, REQUIRE the same compare and branch in
retail, TRY swapping the operands (`1 == depth` instead of `depth == 1`).
The order changes value numbering in the inlined copies; C01 covers
compound reassociation, not plain comparisons.

- Exemplars: `AllocateToLeaf` (97.5 -> 100, early ng lead after 18
  candidates).

## C22

Constant field base before a dynamic offset. Score: 2

IF retail adds a dynamic byte offset to a base plus a constant field offset
(`addi` then `stwx`) where our build folds the field into the store, REQUIRE
that both forms address the same word, TRY associating the constant field
base first (`(base + 0xC) + offset` instead of `base + (offset + 0xC)`).
Never hide the offset inside a pointer representation.

- Exemplars: `_rwResourcesClose` (95.91 -> 100).

## C23

Retail-justified access type per read. Score: 2

IF retail loads the same word twice where our build merges the reads (CSE),
REQUIRE that each access type is independently proven by retail (a signed
`cmpw`, a byte load, an unsigned export), TRY reading it through that type
at each use (a signed view for the compare, the declared `u32` for the
export). A type chosen only to stop CSE is forcing.

- Exemplars: `_rwDlGetRenderState` (99.75 -> 100; signed compare at 0x88,
  audit PASS).

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

Reverse `.sbss` declaration order. Score: 3

IF section totals match but raw symbol offsets are reversed, REQUIRE retail
offsets from the ELF, TRY declaring uninitialized statics in reverse retail
order (GC/1.3.2 emits them reversed). Objdiff's per-symbol 100% does not
check offsets: zero-filled `.sbss` compares equal and relocations compare by
name, so only the linked DOL hash catches a wrong layout.

- Exemplars: HVQM4PlayerEx, hvqm4play applied; `babinwor` (statics added
  next to each user passed objdiff but failed the hash until declared in
  reverse retail order).

## U12

Literal over-allocation. Score: 2

IF retail allocates more bytes than the verified struct it accesses, REQUIRE
the literal allocator argument and complete access, stride and layout
evidence, TRY keeping the request literal with the minimal verified type and
a comment; do not invent trailing fields to make `sizeof` match.

- Exemplar: `HVQM4DecSoundCreate` (allocates 0x20, context verified 0x18).
