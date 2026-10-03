# Matching playbook, tier 4: rare and ceilings

Rules scored 0 or 1, plus the dead-end list. Apply before any tier 5 search
or escalation. Format and scoring: [tier 1](playbook-1-core.md).

## C12

Assignment in the null test. Score: 1

IF retail tests an allocation's return register before storing it to a field,
and ours stores and then reloads the field, REQUIRE the field layout and the
allocator prototype, TRY `if ((p->field = Allocate(...)) == NULL)`.

- Exemplar: `decv_init` (free-node allocations).

## U01

extab means C++ exceptions on. Score: 1

IF the object has extab/extabindex entries, REQUIRE the ELF SECTION symbols,
TRY `-Cpp_exceptions on` at lib or object scope.

IF every function and data section is exact but linking fails the hash on
two bytes inside `.extab` (uninitialized record padding), REQUIRE the retail
bytes at that offset, and the coordinator sets `extab_padding=[b0, b1]` on the
Object (dtk extab clean). This is metadata, not source.

- Exemplar: hvqm4player lib applied.
- Padding: Gecko_ExceptionPPC `[0x12, 0x00]`, movieplayer `[0x02, 0x55]`.

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

## R01

Coloring checklist. Score: 0

IF only register numbers differ, REQUIRE an identical CFG and memory
accesses, TRY in order: C01 grouping, C05 one declaration/scope change, C08
or C10 real locals; then tier 5. A second copy of an in-scope value, reuse of
an unrelated variable, or a branch-only alias is not honest, even at 100%.

## R02

Guarded global reload. Score: 0

IF retail reloads a global between a guard compare and its update (`if (n > 0)
n--;` emitted as load, compare, reload, subtract), REQUIRE that no call or
store intervenes. Facts: no GC 1.0-2.7 flag, pragma, type, wrapper or
control-flow shape reproduces it; only `volatile` does. Go to U11.

## R05

Dead ends, do not repeat.

- Compiler version sweeps (GC 1.1 to 2.6) on hvqm4dec and the player lib:
  1.2.5 for the decoder and 1.3.x for the player lib are best; no other
  version fixes a coloring residue.
- `-opt level=3`, `-schedule on`, `nocse`, `-O3`, `-O4,s` on HVQM4PlayerEx: no
  residue fixed; `-O4,s` and `nocse` regress.
- Narrowing a cached local to its field's storage type (`u8`/`u16`) because
  the producer is narrow: it changes the frame and compares. Keep the
  reference's promoted `int` cache unless retail conversions prove the
  width (`MCBlockDecMCNest` `blocks`: frame 0x68 -> 0x70).
- Collapsing two parameters that carry equal values (source and destination
  stride) into one local or direct field reads: it loses retail's shared load
  and argument copy (`MCBlockDecMCNest`: 89.47%, 87.85%).
- ELF symbol size and scope establish storage, not C qualifiers or optimizer
  settings: empty `.debug`/`.line` and uniform `.mwcats` records carry no
  volatile or opt-level evidence (`lib_link_counter`).
- Asm stubs for coloring ceilings: rejected by the user and regress callers
  (`MCBlockDecMCNest` stub broke seven functions).
