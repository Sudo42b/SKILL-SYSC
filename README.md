# SKILL-SYSC

A Claude Code skill for **SystemC / TLM-2.0**, derived from **IEEE Std 1666-2023** (Language Reference Manual, 618 pp.).

Every Clause and Annex of the LRM is distilled into a matching reference file. The skill lets Claude write and review SystemC models against the standard, and cite the subclause that backs each rule.

## What it does

- **Write** — produce SystemC / TLM-2.0 code that conforms to the LRM
- **Look up** — answer "what does the LRM say about X" with a subclause citation
- **Review** — flag `shall` violations, deprecated features, and reliance on `undefined` behavior

Normative force is preserved verbatim throughout: `shall` / `should` / `may`, and `error` / `undefined` / `implementation-defined` are never softened or strengthened.

Target implementation: **Accellera SystemC 3.0.2** (`IEEE_1666_SYSTEMC == 202301L`), C++17. SystemC 2.3.x implements IEEE 1666-2011 and will not compile the 2023 additions.

## Install

### User-wide

```bash
git clone https://github.com/Sudo42b/SKILL-SYSC.git ~/.claude/skills/systemc
```

### Project-scoped

```bash
git clone https://github.com/Sudo42b/SKILL-SYSC.git .claude/skills/systemc
```

Restart Claude Code. Invoke it with `/systemc`, or let it activate on its own for SystemC-related work.

## Layout

```
SKILL-SYSC/
├── SKILL.md                                  entry point — routes to the right reference
└── references/
    ├── ch01-overview.md                      scope, shall/should/may, C++17 baseline
    ├── ch02-normative-references.md          ISO/IEC 14882:2017
    ├── ch03-terminology.md                   terminology, † classes, lifetime rules, namespaces
    ├── ch04-elaboration-simulation.md        scheduler, delta cycles, per-callback allow/deny matrix
    ├── ch05-core-language.md                 sc_module, processes, sc_port/sc_export, sc_event, sc_time
    ├── ch06-predefined-channels.md           sc_signal, sc_fifo, sc_clock, sc_mutex, sc_semaphore
    ├── ch07-datatypes.md                     sc_int/sc_uint/sc_bigint/sc_logic/sc_bv/sc_lv/sc_fixed
    ├── ch08-utilities.md                     sc_trace, sc_report, sc_vector, sc_assert
    ├── ch09-tlm2-overview.md                 TLM-2.0 compliance criteria
    ├── ch10-tlm2-introduction.md             coding styles (UT/LT/AT/CA), socket concepts
    ├── ch11-tlm2-core-interfaces.md          b_transport, nb_transport, DMI, transport_dbg
    ├── ch12-tlm2-global-quantum.md           tlm_global_quantum
    ├── ch13-tlm2-sockets.md                  initiator and target sockets
    ├── ch14-tlm2-generic-payload.md          attributes, memory management, extensions
    ├── ch15-tlm2-base-protocol.md            permitted phase transitions, transaction ordering
    ├── ch16-tlm2-utilities.md                convenience sockets, quantum keeper, PEQ
    ├── ch17-tlm1-analysis-ports.md           put/get/peek, tlm_fifo, analysis ports
    ├── annexA-glossary.md                    glossary (informative)
    ├── annexB-introduction.md                introduction to SystemC (informative)
    ├── annexC-deprecated.md                  deprecated features (informative)
    └── annexD-changes-2011-2023.md           2011 → 2023 delta (informative)
```

Only `SKILL.md` is always in context. Everything under `references/` is read on demand (progressive disclosure).

## Example prompts

```
> What happens if two processes write to the same sc_signal?
> Review this TLM initiator for base protocol compliance.
> Write an LT-style initiator module with temporal decoupling.
> Can I use SC_CTHREAD inside the end_of_elaboration callback?
> Find any deprecated SystemC features in this code.
```

## Worked example

`examples/lt_demo` is a TLM-2.0 loosely-timed model — initiator, address-decoding router, two memories — using blocking transport, DMI, debug transport, and temporal decoupling through `tlm_quantumkeeper`. Alongside it, `features_2023.cpp` asserts the specified behavior of constructs Annex D records as new in 2023. Every non-obvious line cites the subclause that requires it.

```bash
export SYSTEMC_HOME=$HOME/opt/systemc-3.0.2
cd examples/lt_demo
make run      # the loosely-timed model
make check    # the 1666-2023 feature probe -> 60 passed, 0 failed
```

See `examples/lt_demo/README.md` for how to build SystemC 3.0.2 and for one known deviation of the implementation from the standard.

## Citation

The reference content in this repository is derived entirely from the standard below. When citing results produced with this skill, **cite the standard, not the skill.**

> IEEE Std 1666™-2023 (Revision of IEEE Std 1666-2011), *IEEE Standard for Standard SystemC® Language Reference Manual*. Design Automation Standards Committee, IEEE Computer Society. Approved 5 June 2023 by the IEEE SA Standards Board. Published 8 September 2023. New York, NY, USA: The Institute of Electrical and Electronics Engineers, Inc.
> Print ISBN 978-1-5044-9867-8 (STD26278) · PDF ISBN 978-1-5044-9868-5 (STDPD26278).

### BibTeX

```bibtex
@standard{ieee1666-2023,
  title        = {{IEEE Standard for Standard SystemC\textsuperscript{\textregistered}
                   Language Reference Manual}},
  organization = {IEEE Computer Society, Design Automation Standards Committee},
  publisher    = {Institute of Electrical and Electronics Engineers},
  address      = {New York, NY, USA},
  number       = {IEEE Std 1666-2023},
  note         = {Revision of IEEE Std 1666-2011.
                  Approved 5 June 2023 by the IEEE SA Standards Board},
  year         = {2023},
  month        = sep,
  isbn         = {978-1-5044-9868-5},
  url          = {https://standards.ieee.org/ieee/1666/7071/}
}
```

### IEEE reference style

```
IEEE Standard for Standard SystemC Language Reference Manual,
IEEE Std 1666-2023 (Revision of IEEE Std 1666-2011), Sep. 2023.
```

### Subclause citations

The skill cites the subclause that backs each rule. Carry the standard along when a citation leaves the skill:

```
IEEE Std 1666-2023, §5.2.8 — SC_CTHREAD shall not be invoked
from the end_of_elaboration callback.
```

## Copyright and licensing

- **The standard itself**: Copyright © 2023 by the Institute of Electrical and Electronics Engineers, Inc. **All rights reserved.** No part of the standard may be reproduced in any form without the prior written permission of the publisher. Obtain it from [IEEE Xplore](https://standards.ieee.org/ieee/1666/7071/).
- **This repository**: a **derivative summary and restatement** of the standard's normative content. It is **not endorsed, reviewed, or approved by IEEE or Accellera**, and it does not replace the standard. When exact wording matters, consult the standard.
- Terms for this repository's own authored material (the `SKILL.md` structure, routing tables, checklists, and prose) are in [`LICENSE`](LICENSE) — MIT, scoped so that it grants no rights in the standard itself.
- Full attribution, trademark, disclaimer, and takedown information is in [`NOTICE`](NOTICE).
- SystemC® is a registered trademark in the U.S. Patent & Trademark Office, owned by the Accellera Systems Initiative. IEEE is a registered trademark owned by the Institute of Electrical and Electronics Engineers, Incorporated.

### Disclaimer

IEEE supplies its standards documents "AS IS" and "WITH ALL FAULTS," and expressly disclaims all warranties, express or implied, including merchantability, fitness for a particular purpose, and non-infringement. This repository carries no warranty either.
