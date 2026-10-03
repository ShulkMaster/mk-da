# Matching playbook, tier 2: common source shape

Rules ordered by score. Format and scoring: [tier 1](playbook-1-core.md).

## C05

Declaration order and scope. Score: 14

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

In-place derivation. Score: 11

IF retail derives a value in the same register it was loaded into, REQUIRE one
meaning for the variable, TRY computing it in steps in one local (`margin =
width; margin = (640 - margin) & ~1;`).

- Exemplars: `vDispThread_Entry`; `HVQM4DecodePcm16Ch1`/`Ch2`,
  `HVQM4DecodeAdp8xCh2` (`bytes = samples * 2; bytes *= track; code +=
  bytes;` keeps retail's shift/multiply operand order),
  `HVQM4PlayerExCreate` (audio capacity becomes a byte count, then `&= ~127`),
  `vdisp_init` (next-page pointer set from the allocation, then advanced).

## C03

Retail call boundary. Score: 8

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

## C01

Compound-assignment reassociation. Score: 6

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

Statement order follows the retail schedule. Score: 6

IF loads, stores or pointer advances are ordered differently, REQUIRE
unchanged semantics (no aliasing between the moved accesses), TRY placing
statements in retail's order (compute packed words before storing them,
advance cursors after the stores).

- Exemplars: `MCBlockDecDCNest` (next-row `dP` plus chained row stores),
  `vdisp_copy_frame` applied (75.8 -> 96.5), `gop_decode`
  applied (node output pointer loaded before the size store).

## C04

Newer-version control flow. Score: 5

IF retail has guards, fast paths or fills the reference lacks, REQUIRE a
version marker (`HVQM4_FILEVERSION` "HVQM4 1.5" vs Pikmin's older decoder)
and the retail branch structure, TRY adding only the retail-proven branch
around the reference body.

- Exemplars: `IpicPlaneDec` (guarded first/last rows on a counted
  `nblocks_v`), `MCBlockDecDCNest` (`nbasis == 8` direct DC fill),
  `IntraAotBlock` (one-basis fast path, pre-shifted DC).

## C02

Signed extrema. Score: 4

IF retail compares extrema with `cmpw` where the reference keeps `u8`
min/max, REQUIRE byte loads that still feed u8 outputs, TRY `int` extrema
while keeping the sampled value `u8`.

- Exemplars: `GetAotBasis`, `GetMCAotBasis`.

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

## C12

Assignment in the null test. Score: 1

IF retail tests an allocation's return register before storing it to a field,
and ours stores and then reloads the field, REQUIRE the field layout and the
allocator prototype, TRY `if ((p->field = Allocate(...)) == NULL)`.

- Exemplar: `decv_init` (free-node allocations).
