# Handoff: SKILL-SYSC — IEEE 1666-2023 SystemC skill, ready for review pass

## Session Metadata
- Created: 2026-09-01 14:34:10
- Project: /mnt/e/13_Paper/systemc/SKILL-SYSC
- Branch: main (pushed, clean except this handoff)
- Session duration: one long session — PDF ingestion, skill authoring, licensing, toolchain upgrade, worked example

### Recent Commits (for context)
  - 1220d51 docs: give per-version instructions instead of a single baseline
  - 64dcd28 feat: target SystemC 3.0.2 and add a worked TLM-2.0 LT example
  - a523e4c docs: scope the MIT license and add a NOTICE file
  - 53fe290 docs: rewrite README in English
  - db69330 docs: add IEEE Std 1666-2023 citation and copyright sections

## Handoff Chain

- **Continues from**: None (fresh start)
- **Supersedes**: None

> This is the first handoff for this task.

## Current State Summary

Built a complete Claude Code skill (`SKILL-SYSC`) from the IEEE Std 1666-2023 SystemC Language Reference Manual (618 pp.) and pushed it to https://github.com/Sudo42b/SKILL-SYSC. Every LRM Clause and Annex is distilled 1:1 into `references/`; `SKILL.md` routes to the right file on demand. The skill was then tested by using it to write a TLM-2.0 loosely-timed model plus a 60-assertion feature probe (`examples/lt_demo`), both of which build and run clean against Accellera SystemC 3.0.2, which was compiled and installed during the session specifically so that 2023-only features could actually be exercised. Everything is committed and pushed. **The user's stated next action is a review pass over the whole thing in a fresh session** — nothing is half-finished, so this handoff is about giving the reviewer enough to audit rather than to continue construction.

## Architecture Overview

Progressive disclosure. `SKILL.md` is the only file always in context; it holds a routing table mapping "what you are trying to do" to exactly one reference file, plus pre-write checks, a module skeleton, and a review checklist. `references/ch01..ch17` mirror the LRM Clauses one-for-one; `annexA/B/C/D` cover glossary, introduction, deprecated features, and the 2011→2023 delta. Nothing under `references/` is read unless the router sends you there.

The reference files reproduce the LRM's own repeating structure, which differs by region of the standard:
- **Clauses 1–4**: prose — scope, terminology/conventions, kernel semantics
- **Clauses 5–8**: `Description → Class definition → Constraints on usage → Constructors → Member functions → kind`
- **Clauses 9–17**: `Introduction → Header file → Class definition → Rules → Guidelines`

## Critical Files

| File | Purpose | Relevance |
|------|---------|-----------|
| `SKILL.md` | Router, per-version instructions, pre-write checks, review checklist | Start here; the version-conditional section is the newest and least-reviewed content |
| `references/ch04-elaboration-simulation.md` | Scheduler, delta cycles, per-callback allow/deny matrix | Every other Clause's semantics rest on this |
| `references/ch05-core-language.md` | Modules, processes, ports/exports, events, time (2345 lines) | Largest hand-written file; carries the one recorded LRM↔implementation deviation at §5.10.8 |
| `references/ch07-datatypes.md` | Data types (6273 lines) | Assembled from two subagent outputs merged mid-session — most likely place for a seam or duplication |
| `references/annexC-deprecated.md` | Deprecated features | Load-bearing: the compiler will not warn for most of these |
| `references/annexD-changes-2011-2023.md` | 2011→2023 delta | Load-bearing: tells you what will not compile on 2.3.x |
| `examples/lt_demo/lt_model.cpp` | TLM-2.0 LT model, runs to exactly 2030 ns | Regression check for the whole skill |
| `examples/lt_demo/features_2023.cpp` | 60 assertions over 2023 additions | Proves the toolchain really is 1666-2023 |
| `NOTICE` | IEEE attribution, derivative status, trademarks, takedown | Legal posture; see Decisions Made |

## Key Patterns Discovered

- **Normative force is preserved verbatim.** `shall` / `should` / `may` and `error` / `undefined` / `implementation-defined` are carried into the Korean prose unchanged. Softening a `shall` to "권장" or hardening a `should` to "필수" is a defect.
- **The standard is definitive over the implementation.** LRM Introduction: *"In the event of discrepancies between the behavior of the reference simulator and statements made in this standard, this standard shall be taken to be definitive."* So reference bodies state the standard; implementation deviations go in a separately labeled block.
- **Language split**: `README.md` in English (user-facing); `SKILL.md` and all `references/` in Korean; code comments in English.
- **Every rule carries its subclause number** so any claim can be checked against the PDF.
- Subagent authoring was driven by a STYLE.md contract kept in the session scratchpad (not preserved in the repo) — if more chapters ever need rewriting, that file is the contract that kept nine parallel agents consistent.

## Tasks Finished

- [x] Extracted the 618-page PDF to text, split by Clause, stripped page furniture
- [x] Authored `references/ch01`–`ch05` and `annexB` directly
- [x] Fanned out nine subagents for `ch06`–`ch17` and `annexA/C/D`; merged the two-part `ch07` and relocated its duplicate pitfalls sections to the end
- [x] Wrote `SKILL.md` (router, pre-write checks, skeleton, review checklist) and English `README.md`
- [x] Resolved the unrelated-history merge with the pre-existing GitHub repo; pushed
- [x] Added Citation section (prose, BibTeX, IEEE style, subclause form) and Copyright/Disclaimer sections
- [x] Scoped `LICENSE` (MIT) and added `NOTICE`
- [x] Built and installed Accellera SystemC 3.0.2 to `~/opt/systemc-3.0.2`; wired `~/.bashrc`
- [x] Wrote, built, and ran `examples/lt_demo` (LT model + 2023 feature probe); moved it into the repo
- [x] Replaced the flat "baseline" note with explicit per-version instructions, verified against both 3.0.2 and 2.3.2

## Files Modified

| File | Changes | Rationale |
|------|---------|-----------|
| `SKILL.md` | Router + per-version instruction section | Entry point; user asked specifically for "on 3.0.0 and above, do X" phrasing |
| `README.md` | Rewritten in English; Citation, Copyright, Worked example sections | User requested English; citation requested explicitly |
| `LICENSE` | MIT body untouched; scope preamble prepended | MIT text must stay byte-exact to remain MIT |
| `NOTICE` | New | Discloses IEEE copyright without pretending to license it |
| `references/ch05-core-language.md` | Added `sc_event::none` deviation note at §5.10.8 | Found while running the feature probe |
| `.gitignore` | Build artifacts; `!examples/lt_demo/Makefile` negation | The stock C++ template ignores `Makefile` (line 51), which would have shipped a non-building example |
| `examples/lt_demo/*` | New — model, probe, Makefile, README | Skill validation |
| `~/.bashrc` | Appended a marked SystemC 3.0.2 block (backup at `~/.bashrc.bak.20260901-*`) | Outside the repo — note for the reviewer |

## Decisions Made

| Decision | Options Considered | Rationale |
|----------|-------------------|-----------|
| Keep MIT, add `NOTICE`; do **not** "match IEEE's license" | (a) copy IEEE's all-rights-reserved text into LICENSE, (b) CC BY 4.0, (c) make repo private, (d) MIT + NOTICE | IEEE Std 1666-2023 is all-rights-reserved with no grant to mirror. Copying that into LICENSE would forbid `git clone` — the repo's only use — and would not strengthen IEEE's rights, which are unaffected by what this repo's LICENSE says. Only this repo's own authorship can be licensed. User chose (d). |
| Reference bodies follow the standard; deviations get a side note | Rewrite ch05 to match SystemC 3.0.2 | The LRM Introduction makes the standard definitive. Rewriting bodies to match an implementation destroys the citation value that is the whole point of the skill. |
| One file per Clause (21 files) | Single file; 5 thematic groups | User chose 1:1 mapping for navigability |
| Install SystemC 3.0.2 under `~/opt`, leave 2.3.x alone | Replace the system install | `/opt` and `/usr/local` are not writable (no sudo); SONAMEs differ so coexistence is safe |
| Per-version instructions rather than a single baseline | Flat "target 3.0.2" statement | User asked for it directly; the skill is reference material others follow, so version ambiguity is a defect |

## Immediate Next Steps

1. **Spot-check reference fidelity against the PDF.** Pick a handful of `shall` / `error` / `implementation-defined` statements — especially in the nine subagent-written files (`ch06`–`ch17`, `annexA/C/D`) — and verify wording and subclause numbers against `/mnt/e/13_Paper/systemc/1666-2023.pdf`. The subagents were held to `STYLE.md` but their output was not line-by-line audited.
2. **Re-run the example as a regression.** `export SYSTEMC_HOME=$HOME/opt/systemc-3.0.2 && cd examples/lt_demo && make run && make check` → expect exactly `finished at 2030 ns`, `DMI hits: 78, transport calls: 3`, and `60 passed, 0 failed`.
3. **Review `ch07-datatypes.md` for merge seams.** It was concatenated from two subagent outputs and its pitfalls sections were moved programmatically; check heading order and that nothing was duplicated or orphaned.
4. **Sanity-check the tables that were reconstructed, not copied.** LRM Table 53 (`ch11`, §11.2.3.8) had a corrupted header in the PDF text extraction; the column headers were reconstructed from §11.2.3.7 prose and the file says so. Verify against the PDF.

## Blockers/Open Questions

- [ ] None blocking. The repo builds, runs, and is pushed.

## Deferred Items

- **`lt_model.cpp` DMI/transport cost asymmetry** — the DMI fast path skips the 10 ns of initiator-side overhead the transport path charges, because `try_dmi` returns before `access()` reaches `m_qk.inc(...)`. A modeling inconsistency, not a conformance bug. Documented in `examples/lt_demo/README.md`; left as-is because fixing it would change the 2030 ns regression number.
- **A deliberately-broken example for testing the skill's *review* capability** — offered and not taken up. Would seed violations (streaming width 0, `b_transport` from a method process, per-access DMI latency) and check the skill flags them.
- **`sc_suspend_all` / `sc_unsuspendable` family** — documented in `ch04` but not exercised by `features_2023.cpp`; needs an external OS thread to test meaningfully.

## Important Context

**The reviewer's job is auditing, not building.** Everything is committed and pushed; there is no half-written state to recover.

Three things are load-bearing and easy to break:

1. **Normative force must survive editing.** Every `shall` / `should` / `may` / `error` / `undefined` / `implementation-defined` in the reference files is deliberate. If a review pass "improves" the Korean prose, verify it did not soften a `shall`.

2. **A clean compile proves nothing about deprecated usage.** SystemC 3.0.2 marks *only* `SC_HAS_PROCESS` as `[[deprecated]]`. Verified by grepping the installed headers: `notify_delayed`, `end_module`, `sc_cycle`, `sc_initialize`, `SC_DEFAULT_STACK_SIZE`, `SC_MAX_NUM_DELTA_CYCLES`, and `sc_object::get_parent` all still exist and compile silently. `annexC-deprecated.md` must be consulted by hand. `SKILL.md` says this explicitly — do not let a later edit drop it.

3. **This repo restates an all-rights-reserved standard.** `LICENSE` is scoped by a preamble; `NOTICE` carries the disclosure. Do not widen the license, do not delete the scope preamble, and do not restate reference bodies to match the implementation.

## Assumptions Made

- SystemC 3.0.2 (tag `3.0.2`, `IEEE_1666_SYSTEMC == 202301L`) is the correct 1666-2023 reference implementation to target. Confirmed by the version macro, not just the release notes.
- The user has legitimate licensed access to the standard (the PDF is watermarked for Konkuk Univ. via IEEE Xplore). The repo publishes a derivative summary, not the standard.
- Page ranges in the reference headers (`LRM pp. X–Y`) were taken from the PDF table of contents, not observed in the extracted body text, which had no page numbers after cleaning.

## Potential Gotchas

- **`~/.bashrc` returns early for non-interactive shells** (stock Ubuntu guard at line 6). `bash -lc` gets none of the SystemC variables. Verify env changes with `bash -ic`, and export `SYSTEMC_HOME` explicitly in scripts or CI.
- **`.gitignore` line 51 ignores `Makefile`** (from the GitHub C++ template, meant for CMake-generated ones). `examples/lt_demo/Makefile` is tracked only because of an explicit `!` negation. Any `.gitignore` rewrite must keep it.
- **`sc_event::none` is a function in 3.0.2, a data member in the LRM.** Write `sc_event::none()` in code; the reference body still says what the standard says, with a labeled deviation note.
- **`sc_unbound` binds only to `sc_port<sc_signal_inout_if<T>>`** (LRM §6.31). Binding it to an input is an error — this was hit for real during the session. Inputs use `sc_tie::value`.
- **The PDF is gitignored** (`*.pdf`) and lives at `/mnt/e/13_Paper/systemc/1666-2023.pdf`, one level above the repo. A reviewer needs it to verify fidelity.
- **Do not read subagent transcripts** under the session task directory — they are full JSONL and will flood context.

## Tools/Services Used

- g++ 11.4.0, C++17, CMake 4.3.1, GNU Make 4.3, 24 cores
- `pdftotext -layout` for PDF extraction
- Accellera SystemC 3.0.2 at `~/opt/systemc-3.0.2` (built from source this session)
- SystemC 2.3.2 at `/opt/systemc`; SystemC 2.3.3 from the `libsystemc-dev` deb in `/usr/lib/x86_64-linux-gnu` — both IEEE 1666-2011, both left untouched
- `git` + GitHub remote `origin` → `https://github.com/Sudo42b/SKILL-SYSC.git`

### Active Processes

- None. No servers, no background jobs, no running simulations.

### Environment Variables

Names only — none of these hold secrets:

- `SYSTEMC_HOME` (set to `~/opt/systemc-3.0.2` by the `~/.bashrc` block)
- `SYSTEMC_HOME_3_0`, `SYSTEMC_HOME_2_3`
- `LD_LIBRARY_PATH`, `CMAKE_PREFIX_PATH`

## Related Resources

- Repo: https://github.com/Sudo42b/SKILL-SYSC
- Standard: IEEE Std 1666-2023 — https://standards.ieee.org/ieee/1666/7071/ (local copy: `/mnt/e/13_Paper/systemc/1666-2023.pdf`, gitignored)
- SystemC upstream: https://github.com/accellera-official/systemc (tag `3.0.2`)
- `examples/lt_demo/README.md` — build instructions, expected output, the known implementation deviation, and the known model limitation
- `NOTICE` — attribution, derivative status, trademarks, takedown path
- Project memory: `~/.claude/projects/-mnt-e-13-Paper-systemc/memory/` — `skill-sysc-project.md`, `systemc-toolchain-layout.md`, `sw-lee-working-style.md`

---

**Security Reminder**: Before finalizing, run `validate_handoff.py` to check for accidental secret exposure.
