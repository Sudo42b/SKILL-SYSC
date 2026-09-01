#!/usr/bin/env python3
"""Check every subclause citation in the skills against the IEEE Std 1666-2023 PDF.

`verify_references.py` guards references/. This guards everything that *cites*
them - SKILL.md, CODING-RULES.md, the sysc-* sub-skills and everything under
examples/ - where a wrong citation is invisible: the number exists, it just
points at the wrong subclause. Both spellings count, `§5.2.12` in the skills and
`LRM 5.2.12` in the example sources.

Three checks per citation:

  1. the cited number exists in the LRM at all (catches typos and renumbering)
  2. if the LRM titles that subclause with an identifier (set_stack_size,
     get_direct_mem_ptr, ...) and the citing sentence instead names a *different*
     identifier that titles a *sibling* subclause, the citation is pointing at
     the wrong one. This is what catches `reset_signal_is(...) // §5.2.16`:
     §5.2.16 is set_stack_size, and reset_signal_is is its sibling §5.2.13.
     Siblings only - an unrelated subclause elsewhere in the LRM happening to be
     called `write` or `reset` says nothing.
  3. a citation used as evidence for a rule - the sentence says shall / should /
     error / undefined - may not stop at a subclause that has children. The DMI
     wait rule is §11.3.3 o), not §11.3.

The prose here is Korean and the LRM's titles are English, so a general
word-overlap test is nothing but noise; these three all key off identifiers and
structure instead. Checks 2 and 3 report, only check 1 fails the run.

Usage:  python3 tools/verify_citations.py [--pdf PATH] [--all]
Exit code 0 if every citation resolves, 1 otherwise.
"""
import argparse
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verify_references import extract  # noqa: E402  same cache, same extraction

NORMATIVE = re.compile(r"shall|should|\berror\b|undefined|implementation-defined")

# `§5.2.12` in the Korean skill files, `LRM 5.2.12` in the English example code.
CITATION = re.compile(r"(?:§|LRM\s+)(\d{1,2}\.[\d.]*\d)")

# An LRM title that names one API entity, e.g. "set_stack_size" or "sc_main".
IDENTIFIER = re.compile(r"^[a-z_][a-z0-9_]*$")
# Identifiers appearing anywhere in a title, e.g. both names in
# "reset_signal_is and async_reset_signal_is".
IDENT_TOKEN = re.compile(r"\b[a-z][a-z0-9]*(?:_[a-z0-9]+)+\b")


def lrm_titles(text):
    """subclause number -> the LRM's own title for it."""
    out = {}
    for m in re.finditer(r"^\s*((?:\d{1,2})\.[\d.]*\d)\s+(\S.*)$", text, re.M):
        title = m.group(2).strip()
        if "..." in title or len(title) > 90:   # table-of-contents line
            continue
        out.setdefault(m.group(1), title)
    return out


def context(body, pos, before=260, after=240):
    """The text a citation sits in.

    Backwards only to the start of its own line - prose puts the citation after
    the claim. Forwards further, because example code puts the citation in a
    comment above the lines it documents.
    """
    lo = max(body.rfind("\n", 0, pos), pos - before)
    return body[lo:pos + after]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--pdf", default=os.path.join(os.path.dirname(REPO), "1666-2023.pdf"))
    ap.add_argument("--cache", default=os.path.join(REPO, ".lrm-cache.txt"))
    ap.add_argument("--all", action="store_true",
                    help="list every citation, not just the questionable ones")
    ap.add_argument("--selftest", action="store_true",
                    help="check this script's own parsing; needs no PDF")
    args = ap.parse_args()

    if args.selftest:
        t = lrm_titles("5.2.13 reset_signal_is and async_reset_signal_is\n"
                       "5.2.16 set_stack_size\n"
                       "11.3 Direct memory interface\n"
                       "11.3.3 get_direct_mem_ptr\n"
                       "9.9 Nope ......................... 12\n")
        assert t["5.2.16"] == "set_stack_size", t
        assert "9.9" not in t, "a table-of-contents line is not a heading"
        assert IDENT_TOKEN.findall(t["5.2.13"]) == \
            ["reset_signal_is", "async_reset_signal_is"], "both names indexed"
        kids = {n.rsplit(".", 1)[0] for n in t if n.rsplit(".", 1)[0] in t}
        assert "11.3" in kids and "11.3.3" not in kids, kids
        body = "x\n    reset_signal_is(rst, true);   // §5.2.16 comment\n"
        assert "reset_signal_is" in context(body, body.index("§")), "sees its own line"
        assert [m.group(1) for m in CITATION.finditer("§5.2.12 and LRM 11.3.3 o")] == \
            ["5.2.12", "11.3.3"], "both spellings"
        assert not CITATION.search("LRM pp. 220-411"), "a page range is not a citation"
        assert not NORMATIVE.search("just a routing pointer")
        assert NORMATIVE.search("this shall not happen")
        print("selftest ok")
        return 0

    if not os.path.exists(args.pdf):
        sys.exit(f"LRM PDF not found: {args.pdf}\nPass --pdf with your own licensed copy.")

    titles = lrm_titles(extract(args.pdf, args.cache))

    files = ["SKILL.md", "CODING-RULES.md"] + sorted(
        os.path.join(d, "SKILL.md") for d in os.listdir(REPO)
        if d.startswith("sysc-") and os.path.isdir(os.path.join(REPO, d)))
    for root, _, names in os.walk(os.path.join(REPO, "examples")):
        files += sorted(os.path.relpath(os.path.join(root, n), REPO) for n in names
                        if n.endswith((".cpp", ".h", ".md")))

    has_children = {n.rsplit(".", 1)[0] for n in titles
                    if n.rsplit(".", 1)[0] in titles}
    by_identifier = {}
    for n, t in titles.items():
        for tok in IDENT_TOKEN.findall(t):
            by_identifier.setdefault(tok, n)

    missing, wrong_id, too_shallow, total = [], [], [], 0
    for rel in files:
        body = open(os.path.join(REPO, rel), encoding="utf-8").read()
        for m in CITATION.finditer(body):
            num = m.group(1)
            total += 1
            title = titles.get(num)
            line = body.count("\n", 0, m.start()) + 1
            if title is None:
                missing.append((rel, line, num))
                continue

            ctx = context(body, m.start())
            if IDENTIFIER.match(title) and title not in ctx:
                # Only a complaint if the sentence names some other subclause's
                # identifier instead - that is the one it should have cited.
                parent = num.rsplit(".", 1)[0]
                named = [t for t, n in by_identifier.items()
                         if t != title and t in ctx
                         and n.rsplit(".", 1)[0] == parent]
                if named:
                    best = max(named, key=len)
                    wrong_id.append((rel, line, num, title, best, by_identifier[best]))
            if num in has_children and NORMATIVE.search(ctx):
                too_shallow.append((rel, line, num, title))
            if args.all:
                print(f"  {rel}:{line:<4} §{num:<10} {title[:56]}")

    if missing:
        print("NOT IN THE LRM")
        for rel, line, num in missing:
            print(f"  {rel}:{line}  §{num}")
    if wrong_id:
        print("\nLIKELY THE WRONG SUBCLAUSE - the LRM titles it after an identifier the citing sentence never mentions:")
        for rel, line, num, title, named, right in wrong_id:
            print(f"  {rel}:{line:<4} §{num} is {title}, but the sentence is about {named} (§{right})")
    if too_shallow:
        print("\nCITES A PARENT - this backs a shall/should/error, so name the subclause that carries the rule:")
        for rel, line, num, title in too_shallow:
            print(f"  {rel}:{line:<4} §{num:<10} {title[:52]}  (has subclauses)")

    print(f"\n{total} citations, {len(missing)} not in the LRM, "
          f"{len(wrong_id)} likely wrong, {len(too_shallow)} citing a parent")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
