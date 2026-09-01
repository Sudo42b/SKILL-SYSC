---
name: sysc-ca
description: cycle-accurate(CA) SystemC 모델링. IEEE 1666-2023 §10.3.8은 CA를 TLM-2.0의 범위 밖으로 명시하므로, 이 스킬은 TLM-2.0 CA 스타일을 지어내지 않고 표준이 실제로 규정하는 수단 — sc_clock, SC_CTHREAD, reset_signal_is, sc_signal writer policy, delta cycle 의미론, TLM-1 — 으로 사이클 단위 모델을 쓰는 법을 알려준다. "cycle-accurate", "CA 모델", "사이클 단위", "RTL 수준 SystemC", "clocked thread", "SC_CTHREAD", "파이프라인 스테이지", "TLM-2.0으로 CA 되나" 같은 요청에 사용한다.
---

# Cycle-Accurate (CA) 모델링

## 먼저 알아야 할 것 — TLM-2.0에는 CA 코딩 스타일이 없다

LRM §10.3.8, 축자:

> Cycle-accurate modeling is beyond the scope of TLM-2.0 at present. It is possible to create cycle-accurate models using SystemC and TLM-1 as it stands, but the requirement for the standardization of a cycle-accurate coding style remains an open issue, possibly to be addressed by a future Accellera Systems Initiative standard.
>
> In principle only, the approximately-timed coding style **might** be extended to encompass cycle-accurate modeling by defining an appropriate set of phases and rules. The TLM-2.0 release includes sufficient machinery for this, but **the details have not been worked out**.

따라서:

- **"TLM-2.0 CA coding style"이라는 것은 존재하지 않는다.** 그런 규칙을 요구받으면 표준에 없다고 말하라.
- CA 모델은 **SystemC 코어 언어 + TLM-1**으로 만든다. 이건 표준이 인정하는 방법이다.
- base protocol에 CA용 phase를 얹는 것은 **권장 경로가 아니다** — 아래 참조.

**규칙의 출처는 `../references/`다.** 여기 없는 규칙을 지어내지 마라.

| 필요한 것 | 읽을 파일 |
|---|---|
| **delta cycle, 스케줄러, 평가/갱신 분리** — CA 의미론의 토대 | `../references/ch04-elaboration-simulation.md` |
| `SC_CTHREAD`, `reset_signal_is`, event finder, process control | `../references/ch05-core-language.md` |
| `sc_clock`, `sc_signal` writer policy, `sc_buffer` | `../references/ch06-predefined-channels.md` |
| `sc_int` / `sc_uint` / `sc_bv` / `sc_lv` — RTL 워드 | `../references/ch07-datatypes.md` |
| TLM-1 `put`/`get`/`peek`, `tlm_fifo`, analysis port | `../references/ch17-tlm1-analysis-ports.md` |
| CA가 범위 밖이라는 근거 | `../references/ch10-tlm2-introduction.md` §10.3.8 |

코딩 규칙: `../CODING-RULES.md`.

## 왜 base protocol을 CA로 확장하면 안 되는가

§15.2.5가 막는다. base protocol에 phase를 더하려면 그 phase가 **ignorable**해야 하는데:

- §15.2.5 i: **ignorable phase를 보내는 컴포넌트는 `TLM_ACCEPTED` 외의 어떤 응답도 요구·요청할 수 없다.** 응답을 요구하는 phase는 정의상 ignorable하지 않다.
- CA 모델링에 필요한 phase(스테이지 진입, stall, grant 등)는 전부 응답을 요구한다 → **ignorable이 아니다.**
- §15.2.5 i가 그 경우의 권장 경로를 명시한다: **base protocol 확장이 아니라 새 protocol traits class 정의**(§14.2.3). 그래야 호환되지 않는 프로토콜끼리 소켓 binding이 방지된다.
- §15.2.5 e: ignorable phase는 `BEGIN_REQ` 이전이나 `END_RESP` 이후에 발생하면 **shall not** — base protocol 위반이며 **error**.

즉 CA를 원하면 generic payload/base protocol을 쓰지 말고 **자체 프로토콜을 정의하거나 신호 수준으로 내려가라.**

## 표준이 주는 CA 수단

### 1. `sc_clock` + `SC_CTHREAD`

clocked thread는 클록 에지마다 한 번 재개된다. 사이클 단위 상태 기계에 맞는 유일한 프로세스 종류다.

**제약 (§5.2.12, ch05):**

- clocked thread는 **`wait()`와 `wait(int)`만** 호출할 수 있다. 그 외 오버로드는 **error** — §5.2.12 축자: *"It shall be an error for a clocked thread process to call any other overloaded form of the function wait."*
- `reset_signal_is`는 프로세스 생성 **직후에만** 유효하다 (§5.2.13). 사이에 모듈 인스턴스화가 끼면 안 된다.
- `SC_CTHREAD`는 **deprecated**다 (Annex C `ac)`) — **단, 두 번째 인자가 event finder인 형태는 예외이며 여전히 지원된다.** `clk.pos()`는 event finder이므로 `SC_CTHREAD(f, clk.pos())`는 정상이다.

### 2. delta cycle 의미론이 사이클 정확도를 만든다

`sc_signal`은 평가/갱신이 분리되어 있다 — 같은 delta 안에서 쓴 값은 다음 delta에 보인다. 이것이 하드웨어 레지스터의 동시 갱신을 모델링한다. **`sc_signal`이 아니라 평범한 변수를 쓰면 사이클 정확도가 깨진다.** 자세한 것은 `ch04`.

`sc_buffer`는 값이 바뀌지 않아도 write마다 이벤트를 낸다 — handshake 모델링에 쓴다 (`ch06`).

### 3. 돌아가는 최소 CA 모델

2단 파이프라인. 각 스테이지가 클록 에지마다 레지스터를 갱신한다.

```cpp
#include "systemc"

class Stage : public sc_core::sc_module {
public:
    sc_core::sc_in<bool>            SC_NAMED(clk);
    sc_core::sc_in<bool>            SC_NAMED(rst);
    sc_core::sc_in<sc_dt::sc_uint<8>>  SC_NAMED(din);
    sc_core::sc_out<sc_dt::sc_uint<8>> SC_NAMED(dout);

    explicit Stage(sc_core::sc_module_name name)
        : sc_core::sc_module(name) {
        SC_CTHREAD(tick, clk.pos());     // 두 번째 인자가 event finder — Annex C ac의 예외, deprecated 아님
        reset_signal_is(rst, true);      // §5.2.13 — 프로세스 생성 직후에만 유효
    }

private:
    void tick() {
        dout.write(0);                   // reset 동작
        for (;;) {
            wait();                      // clocked thread는 wait()와 wait(int)만 (§5.2.12)
            dout.write(din.read());      // sc_signal이라 다음 delta에 보인다 = 레지스터 한 단
        }
    }
};

class Top : public sc_core::sc_module {
public:
    explicit Top(sc_core::sc_module_name name) : sc_core::sc_module(name) {
        s0.clk(clk);  s0.rst(rst);  s0.din(src);  s0.dout(mid);
        s1.clk(clk);  s1.rst(rst);  s1.din(mid);  s1.dout(sink);
        SC_THREAD(drive);
        SC_METHOD(observe);
        sensitive << clk.posedge_event();
        dont_initialize();
    }

private:
    sc_core::sc_clock SC_NAMED(clk, sc_core::sc_time(10, sc_core::SC_NS));
    sc_core::sc_signal<bool>            SC_NAMED(rst);
    sc_core::sc_signal<sc_dt::sc_uint<8>> SC_NAMED(src);
    sc_core::sc_signal<sc_dt::sc_uint<8>> SC_NAMED(mid);
    sc_core::sc_signal<sc_dt::sc_uint<8>> SC_NAMED(sink);
    Stage SC_NAMED(s0);
    Stage SC_NAMED(s1);
    int cycle_ = 0;

    void drive() {
        rst.write(true);
        wait(sc_core::sc_time(25, sc_core::SC_NS));
        rst.write(false);
        for (unsigned v = 1; v <= 4; ++v) {
            src.write(v);
            wait(sc_core::sc_time(10, sc_core::SC_NS));
        }
    }

    void observe() {
        std::cout << "cycle " << ++cycle_
                  << "  src=" << src.read()
                  << "  mid=" << mid.read()
                  << "  sink=" << sink.read() << "\n";
    }
};

int sc_main(int, char*[]) {
    Top top("top");
    sc_core::sc_start(sc_core::sc_time(100, sc_core::SC_NS));
    sc_core::sc_stop();
    return 0;
}
```

각 스테이지가 레지스터 한 단이므로 `src`의 값이 `mid`에 한 사이클, `sink`에 두 사이클 뒤에 나타난다. 실제 출력:

```
cycle 4  src=1  mid=0  sink=0
cycle 5  src=2  mid=1  sink=0
cycle 6  src=3  mid=2  sink=1
cycle 7  src=4  mid=3  sink=2
```

`observe`가 `SC_METHOD` + `dont_initialize()`로 클록 posedge에 샘플링하므로, 각 행은 그 에지 **직전**의 레지스터 값이다.

### 4. 트랜잭션 수준이 필요하면 TLM-1

§10.3.8이 "SystemC와 TLM-1으로 CA 모델을 만드는 것은 가능하다"고 할 때의 TLM-1이 `ch17`이다. `tlm_fifo`는 blocking `put`/`get`으로 스테이지 간 backpressure를 모델링한다. generic payload와 base protocol의 순서 규칙에 얽매이지 않는다.

## 자주 틀리는 것

- **"TLM-2.0 CA 스타일"을 요구받고 지어냄** — 존재하지 않는다 (§10.3.8). 표준에 없다고 말하고 위 수단을 제시하라.
- **base protocol에 CA phase를 얹음** — §15.2.5 i가 막는다. 새 protocol traits class를 정의하라 (§14.2.3).
- **`sc_signal` 대신 평범한 멤버 변수로 스테이지 간 값을 전달** — 평가/갱신 분리가 없어져 사이클 정확도가 깨진다. `ch04`.
- **clocked thread에서 `wait(sc_time)` 호출** — **error**. `wait()`와 `wait(int)`만 된다.
- **`reset_signal_is`를 프로세스 생성과 떨어뜨려 호출** — §5.2.13: 직전에 생성된 프로세스를 겨냥해야 한다.
- **`SC_CTHREAD`가 통째로 deprecated라고 착각** — Annex C `ac)`: 두 번째 인자가 event finder인 형태는 **예외이며 여전히 지원된다.**
- **`sc_signal` writer policy 위반** — 기본은 `SC_ONE_WRITER`다 (Annex D `24)`). 두 프로세스가 같은 신호를 쓰면 걸린다. `ch06`.
