---
name: systemc
description: IEEE Std 1666-2023 (SystemC LRM) 기반으로 SystemC/TLM-2.0 모델 코드를 작성·검토하고, 규칙의 근거를 절 번호로 인용한다. SystemC, TLM-2.0, sc_module, sc_signal, sc_fifo, SC_THREAD, SC_METHOD, SC_CTHREAD, sc_port, sc_export, sc_prim_channel, sc_event, sc_time, sc_int/sc_uint/sc_bigint/sc_fixed 등 SystemC 데이터 타입, b_transport/nb_transport, tlm_generic_payload, initiator/target socket, DMI, temporal decoupling, quantum keeper, sc_report/sc_trace, delta cycle이나 스케줄러 동작을 다룰 때 사용한다. "LRM이 뭐라고 하냐", "이 SystemC 코드 표준에 맞냐", "TLM-2.0 base protocol 규칙" 같은 질문에도 사용한다.
---

# SystemC / TLM-2.0 — IEEE Std 1666-2023

이 스킬은 **IEEE Std 1666-2023 Language Reference Manual 전문(618 p)을 Clause별로 정리한 reference 집합**이다.
두 가지 일을 한다.

1. **작성 — LRM 준수 SystemC/TLM-2.0 코드를 쓴다.**
2. **조회 — 규칙의 근거를 절 번호로 인용한다.**

## 작업 절차

### 1. 어느 Clause가 필요한지 먼저 판단하고, 해당 reference 파일만 읽는다

전부 읽지 마라. 아래 표에서 필요한 파일만 `Read`하라.

| 하려는 일 | 읽을 파일 |
|---|---|
| SystemC를 처음 보거나 전체 그림이 필요 | `references/annexB-introduction.md` |
| 용어의 정확한 정의 (module, channel, process, static/dynamic 등) | `references/ch03-terminology.md` |
| **스케줄러 동작, delta cycle, elaboration/simulation 순서, 콜백에서 뭐가 되고 뭐가 안 되나** | `references/ch04-elaboration-simulation.md` |
| 모듈·프로세스·포트·export·이벤트·시간·process control | `references/ch05-core-language.md` |
| `sc_signal` `sc_fifo` `sc_clock` `sc_mutex` `sc_semaphore` 등 미리 정의된 채널 | `references/ch06-predefined-channels.md` |
| `sc_int` `sc_uint` `sc_bigint` `sc_logic` `sc_bv/sc_lv` `sc_fixed` 등 데이터 타입 | `references/ch07-datatypes.md` |
| `sc_trace` `sc_report` `sc_vector` `sc_assert` | `references/ch08-utilities.md` |
| TLM-2.0 준수 조건 | `references/ch09-tlm2-overview.md` |
| **어떤 TLM coding style을 쓸지 결정** (LT/AT/CA, b_ vs nb_) | `references/ch10-tlm2-introduction.md` |
| `b_transport` `nb_transport_fw/bw` DMI `transport_dbg` | `references/ch11-tlm2-core-interfaces.md` |
| `tlm_global_quantum` | `references/ch12-tlm2-global-quantum.md` |
| initiator/target socket, 결합 인터페이스 | `references/ch13-tlm2-sockets.md` |
| `tlm_generic_payload` 속성·메모리 관리·extension | `references/ch14-tlm2-generic-payload.md` |
| **base protocol phase 전이, 트랜잭션 순서 규칙** | `references/ch15-tlm2-base-protocol.md` |
| convenience socket, `tlm_quantumkeeper`, PEQ, instance-specific extension | `references/ch16-tlm2-utilities.md` |
| TLM-1 put/get/peek, `tlm_fifo`, analysis port | `references/ch17-tlm1-analysis-ports.md` |
| 표준 자체의 적용 범위, shall/should/may 강도, C++17 baseline | `references/ch01-overview.md` |
| 규범 참조 (C++17) | `references/ch02-normative-references.md` |
| 용어집 | `references/annexA-glossary.md` |
| **쓰면 안 되는 deprecated 기능인지 확인** | `references/annexC-deprecated.md` |
| 2011 → 2023 변경점 (기존 코드 마이그레이션) | `references/annexD-changes-2011-2023.md` |

> `ch04`는 다른 모든 Clause 의미론의 기반이다. 타이밍·프로세스 실행 순서·언제 호출 가능한가가 얽힌 문제라면 **먼저 ch04를 읽어라.**

### 2. 코드를 쓴다

reference에 있는 규칙과 예제를 따른다. **reference에 없는 규칙을 지어내지 마라** — 없으면 해당 Clause 파일을 더 읽거나, 원문에 없다고 말한다.

### 3. 인용한다

규칙을 근거로 들 때는 **절 번호와 규범 강도를 함께** 쓴다.

```
LRM §5.2.8 — SC_CTHREAD는 end_of_elaboration 콜백에서 호출하면 안 된다 (shall not).
```

**강도를 바꾸지 마라.** `shall`을 "권장"으로 낮추거나 `should`를 "필수"로 올리면 잘못된 인용이다.

| 원문 | 뜻 | 위반 시 |
|---|---|---|
| **shall** | 필수 | 비적합 코드. 반드시 고침 |
| **should** | 권장 | 이유 있으면 이탈 가능. 근거를 남길 것 |
| **may** | 허용 | 선택지 |
| **can** | 가능성 서술 | 규칙 아님 |
| **error** | 구현이 `SC_ERROR` severity로 report | 런타임에 걸릴 수 있음 |
| **undefined** | 구현에 아무 의무 없음 | 절대 의존 금지 |
| **implementation-defined** | 벤더마다 다름 | 이식성 없는 코드가 됨 |

NOTE와 예제는 **informative** — 규범이 아니다. 인용할 때 그 사실을 밝혀라.

## 구현 기준선 — Accellera SystemC 3.0.2

작성한 코드는 **IEEE 1666-2023 구현에서 컴파일되어야** 한다. 기준 구현은 **Accellera SystemC 3.0.2** (`IEEE_1666_SYSTEMC == 202301L`, C++17). `examples/lt_demo`가 이 조합으로 검증되어 있다.

- **SystemC 2.3.x는 IEEE 1666-2011 구현이다.** 2023 신규 기능(`SC_NAMED`, `sc_hierarchy_scope`, `sc_stage_callback_if`, `sc_time(std::string_view)`, `sc_delta_count_at_current_time`, `sc_suspend_all` 계열, `sc_unbound`/`sc_tie`, generic payload option attribute)은 거기서 컴파일되지 않는다. 무엇이 2023 신규인지는 `references/annexD-changes-2011-2023.md`로 확인하라.
- **코드를 쓰기 전에 `references/annexC-deprecated.md`를 확인하라.** 3.0.2는 deprecated 구성요소에 컴파일 경고를 낸다. 특히 `SC_HAS_PROCESS`는 2023에서 불필요해졌고(Annex C ah / Annex D 10) 경고 대상이다 — **생성자에서 `SC_METHOD`/`SC_THREAD`를 쓸 때 붙이지 마라.**

### 알려진 LRM–구현 불일치

표준이 확정적(definitive)이다 — LRM Introduction: *"In the event of discrepancies between the behavior of the reference simulator and statements made in this standard, this standard shall be taken to be definitive."* 아래는 그럼에도 코드가 컴파일되게 하려면 알아야 할 항목이다.

| LRM | 표준의 선언 | SystemC 3.0.2 | 실제로 쓸 것 |
|---|---|---|---|
| §5.10.2 / §5.10.8 | `static const sc_event none;` (데이터 멤버) | `static const sc_event& none()` (함수) | `sc_event::none()` |

## 코드 작성 전 항상 확인할 것

### 필수 확인 5가지

1. **지금이 elaboration인가 simulation인가?** 어느 콜백 안인가?
   → 인스턴스화·바인딩·`SC_*` 매크로·`sensitive`·event finder는 대부분 **elaboration 전용**. 콜백별 허용/금지 매트릭스는 `ch04`에 있다.
2. **프로세스 종류가 맞는가?**
   - `wait()` → **thread / clocked thread 전용**. method process에서 호출하면 **error**
   - `next_trigger()` → **method process 전용**. thread에서 호출하면 **error**
   - clocked thread는 `wait()`와 `wait(int)`만 가능. 그 외 오버로드는 **error**
3. **notification 종류가 맞는가?**
   - `notify()` = immediate → **비결정적**. elaboration 중과 세 콜백에서 만들면 **error**
   - `notify(SC_ZERO_TIME)` = delta → 결정적
   - `notify(t>0)` = timed
   - 한 event에 **pending notification은 최대 1개**, 가장 이른 것만 살아남는다
4. **이 코드가 `sc_prim_channel::update()` 안인가?** 그렇다면 immediate notification, process control, 다른 채널 write 전부 금지.
5. **deprecated 기능을 쓰고 있지 않은가?** → `annexC-deprecated.md`

### 자주 나오는 잘못

- 모듈 인스턴스를 `sc_module`이 아닌 struct 안에 두기 → 허용되나 **비권장**. 데이터 멤버로 둘 것
- 포트를 이름과 위치 **양쪽**으로 바인딩 → **shall not**
- export를 두 번 바인딩하거나 미바인딩으로 남기기 → **error**
- `x.range(7,4)`를 `auto`로 받기 → † proxy 타입을 잡는다. 값 타입으로 변환할 것 (`ch03` §3.2.4)
- `sc_start()` 를 `sc_main` 밖에서 호출 → **shall only** `sc_main`에서
- `sc_stop()` 후 `sc_start()` 재호출 → **error**
- `sc_main` 반환 전에 `sc_stop()`을 안 부름 → `end_of_simulation` 콜백이 안 불린다 (**should**)
- `sc_set_time_resolution`을 두 번 호출하거나 non-zero `sc_time` 생성 후 호출 → **error**
- `sc_unwind_exception`을 잡고 다시 던지지 않음 → **error**
- 빈 event list를 `wait`/`next_trigger`에 전달 → **error**

## 스켈레톤

새 모델을 시작할 때의 최소 골격. 상세 규칙은 `ch05`, `ch06` 참조.

```cpp
#include "systemc"                 // systemc.h 아닌 systemc 권장 (§5.1.2)

SC_MODULE(Producer) {
    sc_core::sc_out<int>  SC_NAMED(out);      // §5.2.9 SC_NAMED로 이름 일치 보장
    sc_core::sc_in<bool>  SC_NAMED(clk);

    SC_CTOR(Producer) {
        SC_THREAD(run);
        sensitive << clk.pos();               // §5.2.14 프로세스 생성 직후에만 유효
    }

    void run() {
        for (;;) {                            // §5.2.11 조기 종료 방지 관용구
            wait();                           // static sensitivity로 재개
            out.write(++count);
        }
    }
    int count = 0;                            // §5.2.10 지속 상태는 데이터 멤버로
};

SC_MODULE(Top) {
    sc_core::sc_clock       SC_NAMED(clk);
    sc_core::sc_signal<int> SC_NAMED(sig);
    Producer p;

    SC_CTOR(Top) : p("p") {
        p.clk(clk);                           // §5.12.7 named binding
        p.out(sig);
    }
};

int sc_main(int argc, char* argv[]) {         // §4.4.5.2 전역 네임스페이스, 이 시그니처 shall
    Top top("top");                           // elaboration = 여기부터 첫 sc_start 직전까지
    sc_core::sc_start(100, sc_core::SC_NS);
    sc_core::sc_stop();                        // §4.4.5.3 end_of_simulation 콜백을 위해 should
    return 0;
}
```

## 검토 체크리스트

기존 SystemC 코드를 리뷰할 때 순서대로 확인한다.

- [ ] `sc_module` 파생 클래스의 모든 생성자가 `sc_module_name` 파라미터를 **정확히 하나** 갖는가 (§5.2.3)
- [ ] 모든 모듈 인스턴스에 문자열 이름을 넘겼는가, 변수명과 일치하는가 (§4.2.2)
- [ ] `SC_METHOD`/`SC_THREAD`/`SC_CTHREAD` 연관 함수가 `void f()` 시그니처인가 (§5.2.8)
- [ ] `sensitive`/`dont_initialize`/`reset_signal_is`/`set_stack_size` 호출이 **직전에 생성된 프로세스**를 겨냥하는가, 사이에 모듈 인스턴스화가 끼지 않았는가 (§5.2.13~16)
- [ ] 모든 포트가 policy에 맞게 바인딩되었는가, 모든 export가 정확히 한 번 바인딩되었는가 (§5.12.3, §5.13.4)
- [ ] `operator->` / `operator[]`를 `end_of_elaboration` 이전에 부르지 않는가 (§5.12.4)
- [ ] event finder를 `end_of_elaboration`이나 simulation 중에 부르지 않는가 (§5.7.3)
- [ ] `update()` 안에서 금지된 동작(immediate notify, process control, `sc_prim_channel` 멤버 호출)을 하지 않는가 (§5.15.6)
- [ ] 공유 상태 접근이 명시적으로 동기화되어 있는가 — evaluation phase 실행 순서는 **구현 정의** (§4.3.2.3)
- [ ] `sc_signal` writer policy 위반이 없는가 (→ `ch06`)
- [ ] TLM: base protocol phase 전이가 허용된 것인가 (→ `ch15`)
- [ ] TLM: generic payload 속성을 수정이 허용된 시점에만 쓰는가, 메모리 관리 규칙을 지키는가 (→ `ch14`)
- [ ] `implementation-defined`나 `undefined` 동작에 의존하지 않는가

## 출처

IEEE Std 1666-2023, *IEEE Standard for Standard SystemC® Language Reference Manual*. Copyright © 2023 IEEE.
이 스킬의 reference 파일은 표준의 규범적 내용을 요약·정리한 것이며 표준 원문을 대체하지 않는다. 정확한 문언이 필요하면 원문을 확인하라.
