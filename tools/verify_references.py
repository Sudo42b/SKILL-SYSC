#!/usr/bin/env python3
"""Check references/*.md against the IEEE Std 1666-2023 PDF.

Per reference file it verifies:
  1. the `LRM pp. X-Y` header matches the pages the clause actually occupies
  2. every subclause number used as a heading exists in the LRM body
  3. every second-level subclause of that clause (N.M) is covered by the file
  4. for Annex C and Annex D, every enumerated item of the annex is present

Deeper levels (N.M.P) are deliberately folded into the shared
`Description / Class definition / Constraints on usage / ...` structure, so
they are not required to appear by number.

The PDF is not redistributable and is gitignored, so this is opt-in:
point --pdf at your own licensed copy (default: ../1666-2023.pdf).

Usage:  python3 tools/verify_references.py [--pdf PATH] [--cache PATH]
Exit code 0 if everything matches, 1 otherwise.
"""
import argparse
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# reference file -> regex matching that clause's heading line in the LRM body
CLAUSES = [
    ("ch01-overview.md",              r"^1\.\s+Overview"),
    ("ch02-normative-references.md",  r"^2\.\s+Normative references"),
    ("ch03-terminology.md",           r"^3\.\s+Terminology"),
    ("ch04-elaboration-simulation.md", r"^4\.\s+Elaboration and simulation"),
    ("ch05-core-language.md",         r"^5\.\s+Core language"),
    ("ch06-predefined-channels.md",   r"^6\.\s+Predefined channel"),
    ("ch07-datatypes.md",             r"^7\.\s+SystemC data types"),
    ("ch08-utilities.md",             r"^8\.\s+SystemC utilities"),
    ("ch09-tlm2-overview.md",         r"^9\.\s+"),
    ("ch10-tlm2-introduction.md",     r"^10\.\s+"),
    ("ch11-tlm2-core-interfaces.md",  r"^11\.\s+TLM-2\.0 core interfaces"),
    ("ch12-tlm2-global-quantum.md",   r"^12\.\s+TLM-2\.0 global quantum"),
    ("ch13-tlm2-sockets.md",          r"^13\.\s+"),
    ("ch14-tlm2-generic-payload.md",  r"^14\.\s+"),
    ("ch15-tlm2-base-protocol.md",    r"^15\.\s+"),
    ("ch16-tlm2-utilities.md",        r"^16\.\s+"),
    ("ch17-tlm1-analysis-ports.md",   r"^17\.\s+"),
    ("annexA-glossary.md",            r"^Annex A\s*$"),
    ("annexB-introduction.md",        r"^Annex B\s*$"),
    ("annexC-deprecated.md",          r"^Annex C\s*$"),
    ("annexD-changes-2011-2023.md",   r"^Annex D\s*$"),
]


def extract(pdf, cache):
    if cache and os.path.exists(cache) and os.path.getmtime(cache) > os.path.getmtime(pdf):
        return open(cache, encoding="utf-8", errors="replace").read()
    text = subprocess.run(["pdftotext", "-layout", pdf, "-"],
                          capture_output=True, text=True, errors="replace").stdout
    if cache:
        open(cache, "w", encoding="utf-8").write(text)
    return text


def page_map(text):
    """[(printed_page_number or None, page_text)] in physical order.

    The printed number is the lone number sitting just above the copyright
    line in the footer; other bare numbers appear in the body, so anchoring
    on the footer is what makes this unambiguous.
    """
    out = []
    for page in text.split("\f"):
        lines = page.splitlines()
        number = None
        for i, line in enumerate(lines):
            if re.match(r"^\s*\d{1,3}\s*$", line) and any(
                    "Copyright" in nxt for nxt in lines[i + 1:i + 3]):
                number = int(line.strip())
        out.append((number, page))
    return out


RANGE = re.compile(r"((?:\d{1,2}|[A-F])\.[\d.]*\d)\s*[-–~]\s*((?:\d{1,2}|[A-F])\.[\d.]*\d)")


def numbers_covered(body):
    """Every subclause number the file mentions, expanding `5.18-5.20` ranges."""
    found = set(re.findall(r"(?<![\d.])((?:\d{1,2}|[A-F])\.[\d.]*\d)(?![\d])", body))
    for lo, hi in RANGE.findall(body):
        a, b = lo.split("."), hi.split(".")
        if len(a) == len(b) and a[:-1] == b[:-1] and a[-1].isdigit() and b[-1].isdigit():
            stem = ".".join(a[:-1])
            found |= {f"{stem}.{i}" for i in range(int(a[-1]), int(b[-1]) + 1)}
    # documenting 3.1.2 covers 3.1
    for n in list(found):
        parts = n.split(".")
        found |= {".".join(parts[:i]) for i in range(2, len(parts))}
    return found


def lrm_second_level(text):
    """clause number -> its second-level subclause numbers, from the LRM body."""
    out = {}
    for line in text.splitlines():
        m = re.match(r"\s*(\d{1,2})\.(\d{1,2})\s+\S", line)
        # a heading line is short; a body line that happens to start with a
        # number (`2.0 core interfaces consist of ...`) is not.
        if m and len(line.strip()) < 70:
            out.setdefault(m.group(1), set()).add(f"{m.group(1)}.{m.group(2)}")
    return out


def annex_items(pages, span, pattern):
    """The enumerated item labels of an annex, read off its own pages."""
    items, seen = [], set()
    for number, body in pages:
        if number is None or not (span[0] <= number <= span[1]):
            continue
        for label in re.findall(pattern, body, re.M):
            if label not in seen:
                seen.add(label)
                items.append(label)
    return items


def clause_pages(pages):
    """reference filename -> (first_printed_page, last_printed_page)."""
    starts = {}
    for name, pat in CLAUSES:
        for idx, (_, body) in enumerate(pages):
            # the same heading text appears in the table of contents, where it is
            # followed by dot leaders and a page number - skip those lines.
            if any(".." not in line for line in body.splitlines() if re.match(pat, line)):
                starts[name] = idx
                break
    order = [n for n, _ in CLAUSES if n in starts]
    last_numbered = max(i for i, (n, _) in enumerate(pages) if n is not None)
    spans = {}
    for i, name in enumerate(order):
        begin = starts[name]
        end = starts[order[i + 1]] - 1 if i + 1 < len(order) else last_numbered
        spans[name] = (pages[begin][0], pages[end][0])
    return spans


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--pdf", default=os.path.join(os.path.dirname(REPO), "1666-2023.pdf"))
    ap.add_argument("--cache", default=os.path.join(REPO, ".lrm-cache.txt"))
    ap.add_argument("--selftest", action="store_true",
                    help="check this script's own number handling; needs no PDF")
    args = ap.parse_args()

    if args.selftest:
        c = numbers_covered("## 5.18-5.20 attributes\n### 3.1.2 x\n7.5.3.5~7.5.3.7\n")
        assert {"5.18", "5.19", "5.20"} <= c, c          # range expanded
        assert {"3.1", "3.1.2"} <= c, c                  # ancestor implied
        assert {"7.5.3.5", "7.5.3.6", "7.5.3.7"} <= c, c  # deep range, tilde
        assert "5.21" not in c and "3.2" not in c, c
        page = "body\n  2.0 core interfaces consist of the blocking and non-blocking transport\n"
        assert lrm_second_level(page) == {}, "long body line is not a heading"
        assert lrm_second_level("5.19 sc_attribute\n") == {"5": {"5.19"}}
        print("selftest ok")
        return 0

    if not os.path.exists(args.pdf):
        sys.exit(f"LRM PDF not found: {args.pdf}\nPass --pdf with your own licensed copy.")

    text = extract(args.pdf, args.cache)
    pages = page_map(text)
    spans = clause_pages(pages)
    # every subclause number that heads a paragraph in the LRM body
    lrm_numbers = set(re.findall(r"^\s*((?:\d{1,2}|[A-F])\.[\d.]*\d)\s+\S", text, re.M))
    second_level = lrm_second_level(text)

    failures = 0
    print(f"{'file':<34} {'declared':>11} {'actual':>11} {'cover':>7}  notes")
    for name, _ in CLAUSES:
        path = os.path.join(REPO, "references", name)
        body = open(path, encoding="utf-8").read()

        m = re.search(r"^LRM pp\. (\d+)[-–](\d+)\.", body, re.M)
        declared = (int(m.group(1)), int(m.group(2))) if m else None
        actual = spans.get(name)

        heads = re.findall(r"^#{2,6} ((?:\d{1,2}|[A-F])\.[\d.]*\d)\b", body, re.M)
        unknown = sorted({h for h in heads if h not in lrm_numbers})

        if name.startswith("annexC"):
            want_items = annex_items(pages, actual, r"^\s*([a-z]{1,2})\) ")
            missing_items = [i for i in want_items
                             if not re.search(r"^\| %s \|" % i, body, re.M)]
        elif name.startswith("annexD"):
            want_items = annex_items(pages, actual, r"^\s*(\d{1,2})\) ")
            missing_items = [i for i in want_items
                             if not re.search(r"^\| %s \|" % i, body, re.M)]
        else:
            want_items, missing_items = [], []

        clause = re.match(r"ch0*(\d+)", name)
        want = second_level.get(clause.group(1), set()) if clause else set()
        uncovered = sorted(want - numbers_covered(body),
                           key=lambda s: [int(x) for x in s.split(".")])

        ok = (declared is None or declared == actual) and not unknown \
            and not uncovered and not missing_items
        failures += not ok
        d = "-" if declared is None else f"{declared[0]}-{declared[1]}"
        a = "?" if actual is None else f"{actual[0]}-{actual[1]}"
        if want:
            cover = f"{len(want) - len(uncovered)}/{len(want)}"
        elif want_items:
            cover = f"{len(want_items) - len(missing_items)}/{len(want_items)}"
        else:
            cover = "-"
        notes = []
        if unknown:
            notes.append("NOT IN LRM: " + ",".join(unknown))
        if uncovered:
            notes.append("MISSING: " + ",".join(uncovered))
        if missing_items:
            notes.append("MISSING ITEMS: " + ",".join(missing_items))
        print(f"{'OK ' if ok else 'FAIL'} {name:<29} {d:>11} {a:>11} {cover:>7}  "
              + ("; ".join(notes) if notes else "ok"))

    print(f"\n{len(CLAUSES) - failures}/{len(CLAUSES)} files verified")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
