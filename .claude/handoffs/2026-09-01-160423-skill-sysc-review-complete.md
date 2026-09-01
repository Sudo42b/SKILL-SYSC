# Handoff: SKILL-SYSC — review pass done, four sub-skills added, two verifiers now guard the whole repo

## Session Metadata
- Created: 2026-09-01 16:04:23
- Project: /mnt/e/13_Paper/systemc/SKILL-SYSC
- Branch: main (pushed, clean except this handoff)
- Session duration: one long session — audit, defect fixes, four new skills, coding rules, two verifier scripts

## Recent Commits (for context)
  - a0a3225 docs: stop stating the citation count, which counting changed
  - f00cbc0 feat: extend the citation check to examples/, and to the LRM n.n spelling
  - 2b045ab docs: bring both READMEs back in step with the repository
  - 25e36c9 fix: correct six citations, narrow the router's description, and guard both
  - 45e3e2c refactor: bring examples/lt_demo in line with CODING-RULES.md
  - 94be05e feat: add sysc-lt, sysc-at, sysc-ca and sysc-verify sub-skills
  - 7ff9b18 feat: check references/ against the LRM with tools/verify_references.py
  - cb7a1cc docs: correct page ranges and two transcription defects in references/

## Handoff Chain

- **Continues from**: [2026-09-01-143410-skill-sysc-review-prep.md](./2026-09-01-143410-skill-sysc-review-prep.md)
  - Previous title: SKILL-SYSC — IEEE 1666-2023 SystemC skill, ready for review pass
- **Supersedes**: None. The predecessor's four audit items are all discharged; read it only for the construction history.

## Current State Summary

The review the previous handoff asked for is done, and it found real defects, which are fixed. The repo then grew four sub-skills (`sysc-lt`, `sysc-at`, `sysc-ca`, `sysc-verify`), a `CODING-RULES.md` that every example now follows, and two verification scripts that mechanise the audit so it does not have to be done by hand again. Everything is committed and pushed; the working tree is clean apart from this file. Both verifiers pass (`21/21` references, `259` citations with nothing wrong), and the `examples/lt_demo` regression is unchanged at 2030 ns, 78 DMI hits, 3 transport calls, 60 assertions passed. **There is no half-finished work.** The open items below are choices nobody has made yet, not loose ends.

## Codebase Understanding

## Architecture Overview

Five skills in one clone. The repo installs to `~/.claude/skills/systemc/`, and each `sysc-*/` subdirectory is discovered as its own skill named by its frontmatter — the same shape as `~/.claude/skills/gstack/ship/` surfacing as `gstack-ship`.

The split is deliberate and load-bearing:

- `references/ch01..ch17`, `annexA..D` say **what the standard says**, 1:1 with the LRM's clauses. Nothing else does.
- `SKILL.md` routes, and owns normative-force rules, per-version instructions and the review checklist.
- `sysc-lt` / `sysc-at` / `sysc-ca` say **how to assemble a model** in one coding style; `sysc-verify` says **how to audit one**. None of them restates a rule — each cites into `references/` instead, so every rule still has exactly one copy.
- `CODING-RULES.md` is the house style every example obeys.
- `tools/` holds the two verifiers.

That "one copy of every rule" property is the whole reason four separate skills are safe rather than a duplication hazard, and `tools/verify_citations.py` is what keeps it honest.

## Critical Files

| File | Purpose | Relevance |
|------|---------|-----------|
| `SKILL.md` | Router, sub-skill routing table, normative-force rules, per-version instructions, skeleton, review checklist | Entry point. Its `description` decides which of the five skills gets picked — see Important Context |
| `CODING-RULES.md` | The six coding rules, the reason for each, and the `[2.3.x]` counterpart | Every example in the repo follows it, `examples/lt_demo` included |
| `tools/verify_references.py` | `references/` vs the LRM — page ranges, subclause existence, second-level coverage, Annex C/D item completeness | 21/21. Run it after any edit under `references/` |
| `tools/verify_citations.py` | Every `§n.n` / `LRM n.n` in the skills and examples vs the LRM | 259 citations. This is the one that catches the defect class this session found |
| `sysc-at/SKILL.md` | AT: phases, exclusion rule, PEQ, a complete model | Densest normative content of the four, so the most exposed if `ch15` is ever edited |
| `sysc-ca/SKILL.md` | Cycle-accurate — quotes §10.3.8 putting CA outside TLM-2.0 rather than inventing a style | The honesty here is the point; see Important Context |
| `examples/lt_demo/lt_model.cpp` | LT model, 2030 ns / 78 DMI hits / 3 transport calls | Regression pin for the whole repo |
| `examples/lt_demo/features_2023.cpp` | 60 assertions over the 2023 additions | Proves the toolchain really is 1666-2023 |

## Key Patterns Discovered

- **Every claim in this repo is checkable, and now mechanically checked.** A reference file states a rule with its subclause number; a skill cites that number; a verifier confirms the number resolves to the subclause that actually carries the rule. Preserve that chain when adding anything.
- **Citations name the subclause carrying the rule, never its parent.** `§11.3.3 o`, not `§11.3`. `verify_citations.py` enforces this whenever the sentence contains `shall` / `should` / `error`.
- **Skill descriptions decide routing, bodies do not.** A routing table inside `SKILL.md` cannot fix a description that claims a child's keywords, because matching happens before the body is read.
- **Examples are executed before they are written down.** Every `cpp` block in `SKILL.md`, `CODING-RULES.md` and the three modeling skills was extracted, compiled under `-Wall -Wextra` against SystemC 3.0.2, and run. Where a skill prints expected output, that is the output the program produced.
- The language split still holds: `README.md` in English, `SKILL.md` / `CODING-RULES.md` / `references/` / `sysc-*` in Korean, code comments in English.

## Work Completed

## Tasks Finished

- [x] Discharged all four audit items from the previous handoff (fidelity spot-check, regression, ch07 merge seams, Table 53)
- [x] Found and fixed page ranges off by one from Clause 8 through Annex D in 15 reference files
- [x] Corrected the Table 53 note — the corruption is the standard's own typesetting, not our extraction
- [x] Removed a duplicated Annex D item 38 and an invented `## 6.0` heading in ch06
- [x] Wrote `tools/verify_references.py` (four checks, `--selftest`, negative-tested)
- [x] Added `sysc-lt`, `sysc-at`, `sysc-ca`, `sysc-verify`
- [x] Wrote `CODING-RULES.md` after two rounds of correction with the user
- [x] Brought `examples/lt_demo` in line with those rules without moving the regression numbers
- [x] Audited the five skills; fixed six wrong or imprecise citations
- [x] Narrowed the `systemc` description, which had been claiming the sub-skills' trigger keywords
- [x] Wrote `tools/verify_citations.py`, then extended it to `examples/` and the `LRM n.n` spelling (148 → 259 citations covered)
- [x] Brought both READMEs back in step with the repository

## Files Modified

| File | Changes | Rationale |
|------|---------|-----------|
| `references/*.md` (15 files) | `LRM pp. X–Y` headers corrected | The standard's own table of contents is off by one from Clause 8 on, and the headers had inherited it |
| `references/ch11-tlm2-core-interfaces.md` | Table 53 note rewritten | It blamed PDF extraction for an error that is in the published table |
| `references/annexD-changes-2011-2023.md` | Item 38 de-duplicated | It appeared in two sections |
| `references/ch06-predefined-channels.md` | `## 6.0` renamed | Not an LRM subclause; it is this file's own summary table |
| `SKILL.md` | Sub-skill routing table, skeleton rewritten to the coding rules, description narrowed, two `struct` examples converted | Routing correctness and self-consistency with `CODING-RULES.md` |
| `CODING-RULES.md` | New | House style, with the reason for each rule and the 2.3.x counterpart |
| `sysc-{lt,at,ca,verify}/SKILL.md` | New | Coding-style assembly and audit procedure |
| `examples/lt_demo/*.cpp` | `struct`→`class`, `explicit`, `m_x`→`x_`, `Top`'s submodules private + `SC_NAMED` | The one place contradicting the coding rules |
| `tools/verify_references.py`, `tools/verify_citations.py` | New | Mechanise the audit |
| `README.md`, `examples/lt_demo/README.md` | Layout tree completed, stale claims corrected, verifier docs | Both had drifted |
| `.gitignore` | `.lrm-cache.txt`, `tools/__pycache__/` | Extraction cache is as non-redistributable as the PDF |

## Decisions Made

| Decision | Options Considered | Rationale |
|----------|-------------------|-----------|
| Four separate sub-skills rather than sections in one router | (a) examples only, (b) extra `references/` files, (c) four separate skills | The user chose (c) after being shown the duplication risk. It is safe only because the skills cite rather than restate, and `verify_citations.py` now enforces that |
| No `sysc-ca` TLM-2.0 coding style; no `sysc-ut` at all | Invent a CA phase set on the base protocol | §10.3.8 puts CA outside TLM-2.0 and §10.3.2 makes no provision for untimed. §15.2.5 i independently blocks CA phases on the base protocol: they would have to be ignorable, and an ignorable phase may not require a response. Inventing either would violate this repo's own rule against making up rules |
| `SC_HAS_PROCESS` forbidden, not required | The user's first draft mandated it | It is deprecated in 1666-2023 and the only thing SystemC 3.0.2 flags, so mandating it would make every new file warn — against the user's own `-Wall -Wextra` clean rule |
| Ports/exports/sockets stay `public` | "all members private" | The parent binds them; private is a compile error, demonstrated |
| Nothing declared with `SC_NAMED` carries a trailing underscore | Underscore everything private | The variable name becomes the hierarchical name, which is the path shown in traces and reports (§5.17). Underscores belong to plain C++ members only |
| `verify_citations.py` keys off identifiers and structure, not word overlap | Compare the LRM's title against the citing sentence | The prose is Korean and the titles are English, so overlap flagged 90 of 148. Identifier-sibling and parent-citation checks flag 0 of 259 with both real defect classes still caught |
| Dropped a citation rather than repairing it | Point the skeleton's `private`+underscore comment at some other subclause | It is this repo's convention, not an LRM rule. §5.2.10 was wrong and so would any replacement be |

## Pending Work

## Immediate Next Steps

1. **Confirm the four sub-skills are actually discovered.** This is the one thing not verified empirically. The nested-skill layout was inferred from `~/.claude/skills/gstack/ship/SKILL.md` surfacing as `gstack-ship`, and each new `SKILL.md` sets `name: sysc-lt` and so on. After a Claude Code restart, check that `/sysc-lt`, `/sysc-at`, `/sysc-ca` and `/sysc-verify` appear. If they surface as `systemc-sysc-lt` — i.e. the name is derived from the path rather than the frontmatter — rename the four directories to `lt/`, `at/`, `ca/`, `verify/` and set the frontmatter names to `systemc-lt` and so on, then update the routing table in `SKILL.md` and the install section of `README.md`.
2. **Run both verifiers before trusting anything.** They need a licensed PDF:
   ```bash
   python3 tools/verify_references.py --pdf ../1666-2023.pdf   # expect 21/21
   python3 tools/verify_citations.py --pdf ../1666-2023.pdf    # expect 259, 0, 0, 0
   python3 tools/verify_references.py --selftest && python3 tools/verify_citations.py --selftest
   ```
3. **Re-run the regression.** `export SYSTEMC_HOME=$HOME/opt/systemc-3.0.2 && cd examples/lt_demo && make run && make check` → exactly `finished at 2030 ns`, `DMI hits: 78, transport calls: 3`, `60 passed, 0 failed`.
4. **Decide whether the deferred items below are wanted.** None is required for the repo to be correct.

## Blockers/Open Questions

- [ ] None blocking. The repo builds, runs, verifies and is pushed.
- [ ] Open question, not a blocker: is nested-skill naming taken from the frontmatter or the directory path? See Immediate Next Steps 1.

## Deferred Items

- **A deliberately-broken example** to prove the review checklist actually fires. Offered twice, not taken up. It would seed known violations (streaming width 0, `b_transport` from a method process, per-access DMI latency, a `TLM_ACCEPTED` return that modified the transaction) and check `sysc-verify` flags them. This is the one gap where the repo asserts a capability it does not demonstrate.
- **`lt_model.cpp` DMI/transport cost asymmetry.** The DMI fast path skips the 10 ns of initiator-side overhead the transport path charges, because `try_dmi` returns before `access()` reaches `qk_.inc(...)`. A modeling inconsistency, not a conformance bug; documented in `examples/lt_demo/README.md`. Left alone because fixing it moves the 2030 ns regression number.
- **`sc_suspend_all` / `sc_unsuspendable` family** — documented in `ch04`, not exercised by `features_2023.cpp`; testing it needs an external OS thread.
- **`references/` still uses `SC_MODULE` / `SC_CTOR` in its examples** — correctly so. Those reproduce the LRM's own example code; rewriting them to the house style would misquote the standard. Do not "fix" them.

## Context for Resuming Agent

## Important Context

**The previous handoff's job was to get this audited. It is audited. This one's job is to not undo it.**

Five things are load-bearing:

1. **Two of the defects found this session are errors in IEEE Std 1666-2023 itself. Do not "correct" the repo back toward them.**
   - The PDF's table of contents is off by one from Clause 8 onward — it puts Clause 8 on p. 411; the body prints it on p. 412, and the shift runs through Annex D. Clauses 1–7 are right. The `LRM pp.` headers now carry the real body pages, and `verify_references.py` derives them from the body, so an edit that "restores" the TOC values will fail the check.
   - Table 53 (§11.2.3.8) has a corrupted header row in the published PDF: it prints the `TLM_ACCEPTED` data row in the header position. `pdftotext -bbox` shows the two rows at distinct y-coordinates, so this is typesetting, not extraction. The reconstructed headers come from §11.2.3.7 e)'s ordering. The note in `ch11` says all this — leave it.

2. **A skill's `description` is what routing sees; its body is not.** The `systemc` description used to list `b_transport`, `nb_transport`, `temporal decoupling`, `quantum keeper` and DMI, written when it was the only skill here, and it was still claiming its own children's triggers. It now covers the core language and lookup and hands coding-style work off explicitly. If you add a sixth skill, check the overlap rather than assuming a routing table will sort it out.

3. **Normative force is still verbatim, and a clean compile still proves nothing about deprecated usage.** Both invariants from the previous handoff hold unchanged. SystemC 3.0.2 marks only `SC_HAS_PROCESS` as `[[deprecated]]`; the other 33 Annex C items compile silently, so `annexC-deprecated.md` has to be consulted by hand. `SKILL.md` says so and `sysc-verify` carries the grep for it.

4. **The license posture is unchanged and must stay that way.** `LICENSE` is MIT scoped by a preamble to this repo's own authorship; `NOTICE` carries the IEEE attribution. The standard is all-rights-reserved and cannot be sublicensed here. `.lrm-cache.txt` is gitignored for the same reason the PDF is — it is the standard's text.

5. **`verify_citations.py`'s heuristics are tuned, and the tuning is not arbitrary.** Three specifics will look like arbitrary constants and are not:
   - The identifier check only complains when the citing sentence names a **sibling** subclause's identifier. Without the sibling restriction, an unrelated subclause elsewhere in the LRM called `write` or `reset` produced six false positives.
   - The forward context window is 240 characters because example code puts a citation in a comment *above* the lines it documents. At 60 it misread §4.6.9 (`sc_get_status`) as being about `sc_stop`.
   - A general word-overlap test between the LRM's English titles and the Korean prose flagged 90 of 148 citations and is useless here. It was tried and removed.

## Assumptions Made

- Nested skill directories are discovered and named from their frontmatter. Inferred from the `gstack` plugin's layout, **not confirmed in a running session** — this is Immediate Next Step 1.
- `pdftotext` (poppler) page splitting on form feeds is stable enough to derive page ranges. The printed page number is read from the lone number above the `Copyright ©` footer line, because the first bare number on a page is often body text.
- The user has legitimate licensed access to the standard. The PDF at `/mnt/e/13_Paper/systemc/1666-2023.pdf` is watermarked for Konkuk Univ. via IEEE Xplore. The repo publishes a derivative summary, not the standard.
- SystemC 3.0.2 (`IEEE_1666_SYSTEMC == 202301L`) is the right reference implementation to target, confirmed by the version macro.

## Potential Gotchas

- **`~/.bashrc` returns early for non-interactive shells.** `bash -lc` gets no SystemC variables. Export `SYSTEMC_HOME` explicitly in scripts, or use `bash -ic`.
- **`.gitignore` line 51 ignores `Makefile`.** `examples/lt_demo/Makefile` is tracked only through an explicit `!` negation. Any `.gitignore` rewrite must keep it.
- **The PDF lives one level above the repo** at `/mnt/e/13_Paper/systemc/1666-2023.pdf` and is gitignored. Both verifiers default to `../1666-2023.pdf` and take `--pdf`; both have a `--selftest` that needs no PDF.
- **Writing a citation count into the docs changes the count.** The README said "259 citations" in a sentence that gave `§5.2.12` and `LRM 5.2.12` as examples of the two spellings — which the checker counts, making it 261. Both places now describe the spellings without printing one. Do not reintroduce a hard total.
- **`sc_event::none` is a function in 3.0.2 and a data member in the LRM.** Write `sc_event::none()` in code; the reference body still states the standard, with a labelled deviation note at §5.10.8.
- **`sc_unbound` binds only to `sc_port<sc_signal_inout_if<T>>`** (§6.31). Inputs use `sc_tie::value`.
- **`SC_NAMED` is variadic** (`sc_macros.h:120`), so it carries extra constructor arguments: `SC_NAMED(mem, 0x1000, sc_time(20, SC_NS))`. Because it supplies a default member initializer, the constructor's initializer list must not also initialise that member.
- **The session-handoff skill's own validator has a bug.** `validate_handoff.py:68` matches `##?\s*<name>`, which cannot match `###`, while the scaffold template emits exactly the required sections at `###`. Filling the template as generated scores about 63. This document promotes those headings to `##` to work around it. Fixing it properly means changing that regex to `#{1,3}` or lowering the template's headings — it is someone else's skill, so it was left alone.
- **Do not read subagent transcripts** under the session task directory; they are full JSONL and will flood context.

## Environment State

## Tools/Services Used

- g++ 11.4.0, C++17, GNU Make 4.3, 24 cores
- `pdftotext` (poppler-utils) — `-layout`, `-raw` and `-bbox` were all used during the Table 53 investigation
- Accellera SystemC 3.0.2 at `~/opt/systemc-3.0.2`, built from source in the previous session
- SystemC 2.3.2 at `/opt/systemc` and 2.3.3 from `libsystemc-dev` — both IEEE 1666-2011, both untouched
- `git` + GitHub remote `origin` → `https://github.com/Sudo42b/SKILL-SYSC.git`

## Active Processes

- None. No servers, no background jobs, no running simulations.

## Environment Variables

Names only — none hold secrets:

- `SYSTEMC_HOME` (set to `~/opt/systemc-3.0.2` by a marked block in `~/.bashrc`)
- `SYSTEMC_HOME_3_0`, `SYSTEMC_HOME_2_3`
- `LD_LIBRARY_PATH`, `CMAKE_PREFIX_PATH`

## Related Resources

- Repo: https://github.com/Sudo42b/SKILL-SYSC
- Standard: IEEE Std 1666-2023 — https://standards.ieee.org/ieee/1666/7071/ (local copy `/mnt/e/13_Paper/systemc/1666-2023.pdf`, gitignored)
- SystemC upstream: https://github.com/accellera-official/systemc (tag `3.0.2`)
- `CODING-RULES.md` — the house style, and the reason behind each rule
- `examples/lt_demo/README.md` — build instructions, expected output, the known implementation deviation, the known model limitation
- `NOTICE` — attribution, derivative status, trademarks, takedown path
- Project memory at `~/.claude/projects/-mnt-e-13-Paper-systemc/memory/` — `skill-sysc-project.md`, `skill-sysc-review-findings.md`, `systemc-toolchain-layout.md`, `sw-lee-working-style.md`

---

**Security Reminder**: Before finalizing, run `validate_handoff.py` to check for accidental secret exposure.
