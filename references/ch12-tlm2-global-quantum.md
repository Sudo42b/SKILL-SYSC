# Clause 12 — TLM-2.0 global quantum

LRM pp. 481–482.

temporal decoupling의 시간 단위인 **time quantum**을 전역으로 관리하는 싱글턴 클래스 `tlm_global_quantum`.
loosely-timed 모델에서 각 initiator가 언제 동기화할지 결정하는 기준값이 여기서 나온다.

---

## 12.1 Introduction

- **Temporal decoupling**은 SystemC process가 시뮬레이션 시각보다 **앞서 달릴 수 있게(run ahead)** 하며, 그 허용 시간이 **time quantum**. loosely-timed coding style과 연관된다.
- context switch와 event 수를 줄여 **시뮬레이션 속도를 크게 개선**한다.
- temporally decoupled process 사이에 **명시적 동기화가 있으면 time quantum이 엄격히 필요하지는 않다** — 그 경우 process는 다음 동기화 지점까지 임의로 앞서 달려도 된다(may). 그러나 **time quantum이 필요한 process는 global quantum을 써야 한다(should)**.
- temporal decoupling을 쓸 때, `b_transport`와 *nb_transport* method에 annotate 된 delay는 **`sc_time_stamp()`가 반환하는 현재 시뮬레이션 시각(= quantum boundary) 기준의 local time offset**으로 해석된다.
- **global quantum = 연속한 quantum boundary 사이의 기본 시간 간격.** 그 값은 싱글턴 클래스 `tlm_global_quantum`이 유지한다.
- **각 process는 global time quantum을 쓰는 것이 권장(recommended)**되지만, **자신의 local time quantum을 계산하는 것도 허용된다(permitted)**.

관련 절:

| 주제 | 참조 |
|---|---|
| temporal decoupling 일반 설명 | 10.3.3 |
| timing annotation | 11.2.4 |
| quantum keeper 사용법 (`tlm_quantumkeeper` 유틸리티 클래스) | 16.3 |

---

## 12.2 Header file

global quantum의 클래스 정의는 **헤더 파일 `tlm`에 있어야 shall**.

```cpp
#include "tlm"
```

---

## 12.3 Class definition

```cpp
namespace tlm {

class tlm_global_quantum
{
public:
    static tlm_global_quantum& instance();
    virtual ~tlm_global_quantum();
    void set( const sc_core::sc_time& );
    const sc_core::sc_time& get() const;
    sc_core::sc_time compute_local_quantum();

protected:
    tlm_global_quantum();
};

} // namespace tlm
```

> 생성자가 **protected** — 애플리케이션이 직접 인스턴스화하지 않고 `instance()`로만 접근한다.

---

## 12.4 tlm_global_quantum

| # | 규칙 |
|---|---|
| a | `tlm_global_quantum`이 유지하는 **유일한(unique) global quantum**이 존재한다. 이것을 **기본 time quantum으로 간주해야 한다(should)**. 의도는 **모든 temporally decoupled initiator가 global quantum의 정수 배 시점에서, 또는 필요하면 더 자주 동기화**하는 것. |
| b | 각 initiator가 서로 다른 time quantum을 쓰는 것도 가능하지만, **모든 initiator가 global quantum을 쓰는 것이 더 전형적**. 드물게만 동기화하면 되는 initiator가 더 긴 quantum을 가질 수도 있으나, **시뮬레이션 속도에 가장 큰 악영향을 주는 것은 대개 가장 짧은 time quantum**이다. |
| c | `instance`는 **싱글턴 global quantum 객체의 참조를 반환해야 shall**. |
| d | `set`은 **global quantum 값을 인자로 넘어온 값으로 설정해야 shall**. |
| e | `get`은 **global quantum 값을 반환해야 shall**. |
| f | `compute_local_quantum`은 **유일한 global quantum에 기반해 local quantum 값을 계산·반환해야 shall**. local quantum은 **global quantum의 바로 다음 큰 정수 배에서 `sc_time_stamp` 값을 뺀 값으로 계산되어야 shall**. `compute_local_quantum`이 **global quantum의 정수 배인 시뮬레이션 시각에 호출된 경우 local quantum은 global quantum과 같아야 shall**. **그 외의 경우 local quantum은 global quantum보다 작아야 shall.** |

### 요약: member functions

| 함수 | 규칙 |
|---|---|
| `static tlm_global_quantum& instance()` | 싱글턴 객체 참조 반환 shall (§12.4 c) |
| `void set( const sc_core::sc_time& )` | global quantum 값을 인자 값으로 설정 shall (§12.4 d) |
| `const sc_core::sc_time& get() const` | 현재 global quantum 값 반환 shall (§12.4 e) |
| `sc_core::sc_time compute_local_quantum()` | `(global quantum의 다음 큰 정수 배) − sc_time_stamp()` 를 반환 shall. 현재 시각이 global quantum의 정수 배면 결과 = global quantum, 아니면 결과 < global quantum (§12.4 f) |

---

## 흔한 위반 (pitfalls)

- **`tlm_global_quantum` 객체를 직접 생성** — LRM §12.3: 생성자가 protected. §12.4 c: `instance()`가 싱글턴 참조를 반환한다. 반드시 `tlm::tlm_global_quantum::instance()`로 접근할 것.
- **initiator마다 제각각인 quantum 사용** — LRM §12.1: time quantum이 필요한 process는 global quantum을 써야 한다(should). §12.4 a: 모든 temporally decoupled initiator는 global quantum의 정수 배 시점에 동기화하는 것이 의도.
- **`get()` 값을 그대로 local quantum으로 사용** — LRM §12.4 f: local quantum은 `compute_local_quantum()`이 계산한다. 현재 시각이 quantum 경계가 아니면 **local quantum은 global quantum보다 작아야 shall** — `get()`을 쓰면 quantum 경계를 넘어가 동기화 지점이 어긋난다.
- **어떤 initiator만 지나치게 짧은 quantum을 설정** — LRM §12.4 b: 시뮬레이션 속도에 가장 큰 악영향을 주는 것은 대개 가장 짧은 time quantum.
- **annotated delay를 절대 시각으로 해석** — LRM §12.1: temporal decoupling에서 `b_transport`/*nb_transport*에 annotate 된 delay는 `sc_time_stamp()`(= quantum boundary) 기준의 **local time offset**이다.
