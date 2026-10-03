# Matching playbook, tier 5: search and escalation

Format and scoring: [tier 1](playbook-1-core.md). AGENTS.md "DecompStudio"
covers tool access and profiles.

## S03

Permuter honesty filter. Score: 6

IF a candidate scores better, REQUIRE porting it by hand or through its
`replace` edits and checking it with `try`, TRY keeping only honest C.
Dirty candidates are welcome as leads; strip and measure each component
separately before porting anything: ask what each forcing construct does
to lifetimes or order (a pragma that fixes CSE points at a reload, an alias
points at a separate live value) and find the honest source that does the
same. An alias that fixes coloring often marks the parameter copies of a
lost inline helper: check whether the reference calls a helper there and
restore that call shape (C03). Log the lead; only clean source is promoted.
Reject: pragmas (`opt_propagation`, `optimization_level`), `new_var` copies
of in-scope values, reuse of unrelated variables, unsequenced expressions,
redundant same-value stores, duplicated stores, invented helpers. Use
`alternatives:true` and `tuning_report:true` for the engine's own honesty
findings, but audit by hand anyway: `honest_only` and `all_passes_honest`
still returned pragma-bearing leads (`_HVQM4PlayerExThread`).

- Exemplars: `vdisp_copy_frame` (helper and unused chain stripped from the
  96.54% lead), `MCBlockDecMCNest` (the `new_var = tWidth` zero led to
  Pikmin's `_MotionComp` call shape).

## S04

Escalation ladder. Score: 5

IF the owner tier has spent the item's 100 combined attempts (30 per lease)
and at least 120k ng compiles,
REQUIRE a `soft_ceiling:` abandon with best score, counts and ruled-out
forms, TRY the next tier: a fresh Opus agent (20 attempts), then a Fable
agent (15 attempts). A final failure stays parked as `soft_ceiling` with its
TODO line and note.

- Exemplars: `HVQM4BufaCreate`, `vdisp_copy_frame`, `MCBlockDecMCNest`
  (closed at the Opus tier).

## S02

Neutral seed rotation. Score: 4

IF the permuter returns an honest byte-equivalent variant, REQUIRE that the
honesty filter passes (S03), TRY adopting it in the draft (`try` with
`keep:true`) and permuting again from it. The permuter is very
seed-sensitive.

- Exemplars: `vDispThread_Entry` (centered-temp seed led to the closing
  lead), `MCBlockDecMCNest` and `vdisp_copy_frame` applied.

## S01

Permuter ng procedure. Score: 3

IF only coloring or scheduling remains, REQUIRE an established CFG, ABI and
layout, TRY an early probe first (one ng run of up to 30 s right after the
draft's CFG, ABI and layout are right; ng converges fast, so clean leads
show up within a few thousand candidates), then `submit {commands:[{op:
"permute", symbol, engine:"ng", seconds:120}], wait:false}` plus `fence`,
one run in flight per agent (runs queue machine-wide; the cap includes
queueing), until at least 120k candidates have compiled; then `engine:"rust"` or `mode:"call_args"` (rust
only) for argument staging around calls. If ng runs keep stopping early on
dirty zeros despite `dirty_zero_s`/`honest_share`/`unlicensed_factor`,
finish the 120k with `engine:"rust"` (behavior-preserving transforms only;
its compiles count toward the total). Log engine, compiled count and result
in history.

- Exemplars: `vdisp_copy_frame` (75.8 -> 96.5), `vDispThread_Entry` lead,
  `MCBlockDecMCNest` (98.90 -> 99.69 declaration lead).
