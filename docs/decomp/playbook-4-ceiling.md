# Matching playbook, tier 4: ceilings and last resorts

Apply before any tier 5 search or escalation. Format and scoring:
[tier 1](playbook-1-core.md).

## R04

Offline compiler pre-screen. Score: 6

IF a small localized residue remains, REQUIRE the exact TU command (`ninja -t
commands <object>`), TRY compiling variants locally with the project's
mwcceppc and comparing the symbol's objdump against retail before spending
`try` attempts.

- Exemplars: `HVQM4BufaCreate`, `vdisp_copy_frame`, `MCBlockDecMCNest`
  (Opus tier, each closed on its first try).

## R03

Last-resort goto. Score: 2

IF the retail CFG jumps from inside a wait loop to a shared end-of-iteration
block, REQUIRE three measured structured forms that fail, TRY one local
`goto` to that block and cite the measurements (AGENTS.md exception).

- Exemplar: `gop_decode`.

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
