# Matching playbook, tier 1: core

Read this tier on every matching task. Open a deeper tier when the triage
table sends you there, and always before a ceiling escalation.

| Tier | File | Scope |
|---|---|---|
| 1 | this file | protocol, scoring, triage, rule index, stops |
| 2 | [playbook-2-common.md](playbook-2-common.md) | rules scored 4 or more: try these first |
| 3 | [playbook-3-uncommon.md](playbook-3-uncommon.md) | rules scored 2-3 |
| 4 | [playbook-4-rare.md](playbook-4-rare.md) | rules scored 0-1, last resorts, dead ends |
| 5 | [playbook-5-search.md](playbook-5-search.md) | S rules: permuter, seed rotation, escalation ladder |

Tiers rank rules by score; the ID letter is the category: C source shape,
U compiler flags, inline boundaries and data/link layout, R ceilings and
last resorts, S search and escalation. IDs never change when a rule moves.

Rule format: `ID | IF mismatch | REQUIRE evidence | TRY one change`. Missing
evidence means skip the rule. These are diagnostics for this project's
compilers (GC/1.3.2 game code, GC/1.2.5 Pikmin-derived HVQM4 decoder), not
recipes. MK Deception's playbook targets a different compiler and library
versions: borrow its format, never its patterns. Cite rule IDs in notes, for
example `C01 closed`.
DecompStudio `docs {op:"read", path:"docs/decomp/playbook-2-common.md",
section:"C01"}` returns one rule.

## Scoring

Every rule carries `Score: N`. When a rule applies to a function and the
change is kept (improvement or adopted neutral seed), add +1. When the
application brings the function to 100%, add +2 instead. One function
can credit several rules when each was needed. Edit only the number; put the
symbol in the rule's exemplar list if it is the first or the clearest case.
Measurements and diaries go to DecompStudio `history`, not here. Rules are
listed by score inside each tier. Tier thresholds: 2 holds scores of 4 or
more, 3 holds 2-3, 4 holds 0-1 and the dead-end list; re-rack (move the
section, keep its ID) when a score crosses a threshold.

## Protocol

- Authority: AGENTS.md > retail ASM, callers, ELF symbols and sections, layout
  > Pikmin (lead reference) > m2c, Ghidra, permuter. The latter are hypotheses.
- Loop: read notes and history -> baseline -> classify (triage) -> one rule ->
  `try` -> keep or revert -> note. Keep the immediate `TODO: [status] NN.NN%;
  what remains` line accurate below 100; remove it at 100.
- Research freely: any trick (forcing constructs, pragmas, aliases,
  permuter output) may be used in drafts and scratch to find a match or a
  lead. It is evidence about the shape retail wants, never the answer.
- Before promote, submission or review, clean up. Promoted source never
  contains: `register`, fake `volatile`, padding or unused locals, dead sinks,
  pragmas, inline-blocking tricks, aliases that copy an in-scope value,
  reuse of an unrelated variable, invented fields, wrong prototypes, local
  re-declarations where a real header exists, asm. Exception: the same
  construct in Pikmin's same function, copied verbatim and cited.
- Raw placement: per-symbol 100% does not prove layout. Check raw offsets and
  sizes of statics (`.sbss` order) and the order of anonymous literals in the
  pool (a reversed pair leaves relocations wrong while each string matches).
- Acceptance: judge per function and per instruction; check collateral on
  every function in the TU; use `functionRelocDiffs=data_value`; a unit goes
  Matching only after the coordinator's whole-object and retail-hash audit.

## Triage

| Diff symptom | Rules |
|---|---|
| Add/or operands commuted in one row | C01, R01 |
| Signed vs unsigned compare (`cmpw`/`cmplw`), byte truncations | C02, U09 |
| Reference inlines or table-dispatches, retail calls directly (or reverse) | C03, U05 |
| Control flow or extra guards absent from the reference | C04 |
| Same operations, registers swapped | C05, R01, then tier 5 |
| Frame larger than ours, body otherwise exact | C06, U08 |
| Load/store order around a copy differs | C07, C09 |
| Prologue scheduling differs, loop body exact | C11, C09 |
| Field stored then reloaded for a null test | C12 |
| Size arithmetic staged differently | C13, C01 |
| A value is derived in place in retail | C08 |
| Value reloaded vs cached before a dispatch | C10, R02 |
| `_savegpr`/`_restgpr` vs `stmw`/`lmw` | U02 |
| Literals in `.data` instead of `.rodata`, pooled vs per-literal | U03, U07 |
| Object has extab/extabindex entries | U01 |
| `.sbss`/`.bss` offsets reversed | U04 |
| Bytes exact but `[order]` | U06 |
| Unit-owned global missing from `.data` | U07 |
| Allocation literal larger than the struct | U12 |
| Retail strings with no code reference | U07 |
| Redundant global reload in a guarded RMW | R02, U11 |
| Shared end-of-iteration code needs a jump | R03 |
| Scheduling-only residue across many objects of one library | U13 |
| `fmadds` where retail has `fmuls` + `fadds` | C14 |
| Indirect call through a pointer local adds `mr r12` | C15 |
| Byte load converted to float/int with swapped registers | C16 |
| Extra `addi` keeps a field address of a `void*` input | C17 |
| Default constant loaded before a compare retail does first | C18 |
| 0/1 result of a call shifted through a different register | C19 |
| Index from a pointer difference with a signed divide | C20 |
| Loop pointer, buffer and input colored in the wrong order | C24 |
| Only coloring or scheduling left | R01, R04, tier 5 |

## Rule index

Scores in parentheses.

Tier 2, common:
C05 declaration order and scope (14), C08 in-place derivation (11), C03 retail
call boundary (8), R04 offline compiler pre-screen (8), C01 compound-
assignment reassociation (6), C09 statement order follows the retail schedule
(6), U07 unit-owned data and stripped-helper strings (6), U13 library
optimization level (6), C16 genuine byte locals (4), C04 newer-version
control flow (5), U05 static inline boundaries (5), C02 signed extrema (4),
U08 Pikmin same-function exception (4), U11 genuine volatile on shared state
(4).

Tier 3, uncommon:
U02 lmw/stmw prologues (3), U06 function order (3), C06 real aggregate for
frame size (2), C07 named loaded words (2), C10 cache a field before dispatch
(2), C11 bounds in the loop's own units (2), C13 name the real base quantity
(2), C14 named products against fused multiply-add (2), C15 conditional
call target (2), C17 byte-pointer base for read-only inputs (2), C18 conditional expression
for clamps (2), C19 boolean result form (2), C20 signed index from a pointer
difference (2), C24 indexed scan before a cursor local (2), R03 last-resort
goto (2), U03 per-literal read-only
strings (2), U04 reverse `.sbss` declaration order (3), U12 literal
over-allocation (2).

Tier 4, rare and ceilings:
C12 assignment in the null test (1), U01 extab means C++ exceptions on (1),
U09 unsigned literal compares (1), U10 remove obsolete reference dispatch
support (1), R01 coloring checklist (0), R02 guarded global reload (0), R05
dead ends (do not repeat).

Tier 5, search and escalation:
S03 permuter honesty filter (6), S04 escalation ladder (5), S02 neutral seed
rotation (4), S01 permuter ng procedure (3).

## Stop

- Unknown calls, offsets or control flow are not coloring: mark `borked` or
  `breakthrough needed` and name the missing evidence.
- Only coloring left after tier 2-4 checks: run tier 5. A function leaves its
  owner only through the S04 ladder; a final failure is parked in
  DecompStudio as `soft_ceiling` with a note and its TODO line, never in
  `docs/asm.md`.

## Maintaining the tiers

Amend the rule where it lives: precondition, action, one exemplar. No
percentages, attempt logs or duplicate rows. Agents propose new rules or
score bumps to the coordinator (gate), who edits these files.
