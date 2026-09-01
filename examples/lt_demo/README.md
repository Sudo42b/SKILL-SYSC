# lt_demo — SKILL-SYSC test bench

Two programs written using only the `SKILL-SYSC` reference files as source of
truth, then compiled and run to check the skill actually produces conforming
code.

| File | What it is |
|---|---|
| `lt_model.cpp` | A TLM-2.0 **loosely-timed** model: initiator + address-decoding router + two memories. Blocking transport, DMI, debug transport, temporal decoupling via `tlm_quantumkeeper`. |
| `features_2023.cpp` | A probe for constructs Annex D records as **new in IEEE Std 1666-2023**, asserting the behavior the LRM specifies. |

Every non-obvious line cites the subclause that requires it.

## Requirements

An IEEE Std 1666-2023 implementation. Built and tested against
**Accellera SystemC 3.0.2** (`IEEE_1666_SYSTEMC == 202301L`), C++17.

SystemC 2.3.x implements IEEE 1666-2011 and will **not** build these:
`SC_NAMED`, `sc_hierarchy_scope`, `sc_stage_callback_if`, `sc_time(std::string_view)`,
`sc_delta_count_at_current_time`, `sc_unbound`, `sc_tie` and the generic payload
option attribute are all 2023 additions.

## Build and run

```bash
export SYSTEMC_HOME=$HOME/opt/systemc-3.0.2

make run      # the loosely-timed model
make check    # the 1666-2023 feature probe
```

Building SystemC 3.0.2 from source:

```bash
git clone --depth 1 --branch 3.0.2 https://github.com/accellera-official/systemc.git
cmake -S systemc -B systemc/build \
      -DCMAKE_INSTALL_PREFIX=$HOME/opt/systemc-3.0.2 \
      -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
cmake --build systemc/build -j"$(nproc)"
cmake --install systemc/build
```

## Expected output

`make run`:

```
--- phase 3: unmapped address (expect an error response) ---
  0x8000 -> TLM_ADDRESS_ERROR_RESPONSE (expected)

--- phase 4: debug transport (no delay, no side effects) ---
  transport_dbg returned 16 bytes: a5 a5 a5 a5 00 00 00 00
  simulation time unchanged: yes

--- phase 5: 64 accesses over DMI ---
  DMI hits: 78, transport calls: 3

finished at 2030 ns
```

81 accesses resolve to 3 transport calls (first touch of each memory, plus one
unmapped address) and 78 DMI hits. 2030 ns is exact: 316 + 296 + 10 + 1408 ns
across the four timed phases, with DMI latency applied **per byte** as
LRM 11.3.5 ac requires.

`make check`: `60 passed, 0 failed`.

## Known deviation of the implementation from the standard

LRM 5.10.2 declares the never-notified event as a static **data member**:

```cpp
static const sc_event none;
```

SystemC 3.0.2 provides a static member **function** instead
(`static const sc_event& none()`), so `features_2023.cpp` has to write
`sc_event::none()`. The LRM Introduction states that where the reference
simulator and the standard disagree, the standard is definitive — so this is a
deviation on the implementation side. Do not rely on this name if portability
matters.

## Known limitation of `lt_model.cpp`

Not a conformance issue: the DMI fast path skips the 10 ns of initiator-side
overhead that the transport path charges, because `try_dmi` returns before
`access()` reaches `m_qk.inc(...)`. A production model would charge both paths
consistently.
