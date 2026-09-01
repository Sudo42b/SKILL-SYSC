# Clause 11 — TLM-2.0 core interfaces

LRM pp. 452–479.

TLM-1 core interface에 더해 TLM-2.0이 추가하는 **blocking / non-blocking transport interface, DMI, debug transport interface**.
initiator·target·interconnect 사이에서 트랜잭션을 실제로 옮기는 계층 — 소켓(Clause 13)과 generic payload(Clause 14)가 이 인터페이스 위에 얹힌다.

---

## 11.1 Overview

TLM-1의 core interface에 더해 TLM-2.0은 다음을 추가한다:

- blocking transport interface
- non-blocking transport interface
- direct memory interface (DMI)
- debug transport interface

---

## 11.2 Transport interfaces

### 11.2.1 Overview

- transport interface는 initiator / target / interconnect 사이에서 트랜잭션을 옮기는 **주 인터페이스**.
- blocking·non-blocking 둘 다 **timing annotation**과 **temporal decoupling**을 지원. 그러나 **트랜잭션 수명 중 여러 phase를 지원하는 것은 non-blocking뿐**.
- blocking transport에는 명시적 phase 인자가 없다. blocking transport와 non-blocking transport의 phase 사이의 연관은 **순전히 관념적(notional)**.
- **반환 경로(return path) 사용 여부를 값으로 알려주는 것은 non-blocking transport뿐**.
- transport interface와 generic payload는 memory-mapped bus의 빠르고 추상적인 모델링을 위해 **함께 쓰도록** 설계됨. 템플릿을 트랜잭션 타입으로 특수화하여 generic payload와 분리해 쓸 수도 있으나, **interoperability 이점 대부분을 잃는다**.
- 트랜잭션 객체의 memory management, 트랜잭션 순서, 허용되는 함수 호출 순서를 지배하는 규칙은 **템플릿 인자로 넘긴 트랜잭션 타입**에 달려 있고, 이는 다시 (소켓을 쓴다면) 소켓에 넘긴 **protocol traits class**에 달려 있다.

---

### 11.2.2 Blocking transport interface

#### 11.2.2.1 Introduction

- **loosely-timed** coding style 지원 목적.
- initiator가 **단일 함수 호출** 안에서 트랜잭션을 완료하고자 할 때 적합 — 관심 있는 timing point는 트랜잭션의 시작과 끝 둘뿐.
- **forward path(initiator → target)만** 사용.
- TLM-1의 `transport`와 의도적으로 유사하나 **동일하지 않다**:
  | | TLM-1 `transport` | TLM-2.0 `b_transport` |
  |---|---|---|
  | 트랜잭션 인자 | request를 **const reference** 하나로 | 트랜잭션을 **non-const reference**로 |
  | timing | 없음 | `sc_time&` 두 번째 인자 |
  | 응답 | response를 **값으로 반환** | 단일 트랜잭션 객체를 참조로 전달 (반환값 없음) |
- `b_transport`의 timing annotation 인자는 **하나뿐이며 호출 시·복귀 시 모두** 사용되어, 각각 현재 시뮬레이션 시각 기준 트랜잭션의 **시작 시각**과 **종료 시각**을 나타낸다.

#### 11.2.2.2 Class definition

```cpp
namespace tlm {

template <typename TRANS = tlm_generic_payload>
class tlm_blocking_transport_if : public virtual sc_core::sc_interface {
public:
    virtual void b_transport(TRANS& trans, sc_core::sc_time& t) = 0;
};

} // namespace tlm
```

#### 11.2.2.3 The TRANS template argument

- 어떤 타입의 트랜잭션도 옮길 수 있게 하려는 의도. 트랜잭션 속성의 세부가 덜 중요한 경우 interoperability를 위해 `tlm_generic_payload`가 제공됨.
- **최대 interoperability를 위해서는 기본 트랜잭션 타입 `tlm_generic_payload`를 base protocol과 함께 쓸 것(should)** (15.2 참조).
- 특정 프로토콜을 모델링하려면 자체 트랜잭션 타입으로 대체해도 된다(may).
- **서로 다른 트랜잭션 타입으로 특수화된 인터페이스를 쓰는 소켓끼리는 bind 불가** — compile-time checking은 되지만 interoperability는 제한된다.

#### 11.2.2.4 Rules

| # | 규칙 |
|---|---|
| a | `b_transport`는 직접·간접적으로 `wait`을 호출해도 된다(may). |
| b | `b_transport`는 **method process에서 호출하면 안 된다 shall not**. |
| c | initiator는 트랜잭션 객체를 호출 간에, 그리고 transport / DMI / debug transport interface 호출에 걸쳐 **재사용해도 된다(may)**. |
| d | `b_transport` **호출**이 트랜잭션의 첫 timing point, **복귀**가 마지막 timing point를 표시. |
| e | timing annotation 인자는 그 timing point들을 함수 호출·복귀가 실행된 시뮬레이션 시각(`sc_time_stamp()`)으로부터 **offset** 시킬 수 있게 한다. |
| f | callee는 트랜잭션 클래스 `TRANS`가 부과하는 제약 하에서 트랜잭션 객체를 **수정·갱신해도 된다(may)**. |
| g | 트랜잭션 객체는 **timing 정보를 담지 않는 것이 권장(recommended/should)**. timing은 `b_transport`의 `sc_time` 인자로 annotate 할 것(should). |
| h | 통상 interconnect component는 `b_transport` 호출을 forward path로 **넘겨주어야 한다(should)**. 즉 interconnect의 target socket용 `b_transport` 구현이 initiator socket의 `b_transport`를 호출해도 된다(may). |
| i | `b_transport` 구현이 `nb_transport_fw`를 호출해도 되는지는 **프로토콜 규칙에 달려 있다**. base protocol에서는 convenience socket `simple_target_socket`이 이 변환을 자동으로 해준다 (16.2.2). |
| j | `b_transport` 구현은 **`nb_transport_bw`를 호출하면 안 된다 shall not**. |

#### 11.2.2.5–11.2.2.7 Message sequence charts (informative)

> - **blocking transport (Figure 20)**: `b_transport`는 현재 SystemC evaluation phase에서 **즉시 반환해도 되고**, 스케줄러에 제어를 넘겨 나중 시뮬레이션 시각에 반환해도 된다(may). initiator thread가 blocked인 동안에도, 프로토콜에 따라 initiator의 **다른 thread가 첫 호출 반환 전에 `b_transport`를 호출하는 것이 허용될 수 있다**.
> - **temporal decoupling (Figure 21)**: temporally decoupled initiator는 현재 시뮬레이션 시각보다 앞선 notional local time에서 동작하며, 그 경우 time 인자에 **0이 아닌 값을 넘겨야 한다(should)**. initiator와 target 모두 time 인자 값을 증가시켜 local time offset을 더 진행시킬 수 있다. 반환된 time 인자를 현재 시뮬레이션 시각에 더하면 트랜잭션이 완료되는 notional time이 되지만, **initiator thread가 yield 하기 전에는 시뮬레이션 시각 자체가 진행하지 않는다**. `b_transport` 본문이 `wait`을 호출하면 **local time offset은 0으로 리셋해야 한다(should)**.
> - **the time quantum (Figure 22)**: temporally decoupled initiator는 time quantum을 초과할 때까지 local time을 진행시키다가, 그 시점에 local time을 인자로 `wait`을 직접·간접 호출해 **suspend 함으로써 동기화할 의무가 있다**. 이로써 다른 initiator들이 실행되어 따라잡는다. 원래 initiator는 시뮬레이션 시각이 다음 quantum까지 진행된 뒤에만 다시 실행되어야 한다(should).
>   - loosely-timed style에서 delay의 주 목적은 **각 initiator가 언제 제어를 반납할지 결정**하게 하는 것. 모델이 정확히 동작하기 위해 timing 세부에 의존하지 않는 것이 좋다.
>   - 한 quantum 안에서 한 initiator가 생성한 트랜잭션들은 **엄격한 순차 순서로 일어나되 시뮬레이션 시각은 진행하지 않는다**. **local time은 SystemC scheduler가 추적하지 않는다.**

---

### 11.2.3 Non-blocking transport interface

#### 11.2.3.1 Introduction

- **approximately-timed** coding style 지원 목적. 트랜잭션을 여러 phase로 쪼개고 각 phase transition마다 timing point를 두어 initiator/target 간 상호작용 순서를 상세히 모델링할 때 적합.
- **호출과 복귀 각각이 phase transition에 대응할 수 있다.**
- timing point를 2개로 제한하면 loosely-timed style에도 쓸 수 있으나 **일반적으로 권장되지 않는다(not generally recommended)**. loosely-timed에는 단순함 때문에 blocking transport가 일반적으로 선호된다. non-blocking은 **pipelined transaction** 모델링에 특히 적합(blocking으로는 어색하다).
- forward path와 backward path **둘 다** 사용 — 서로 다른 두 인터페이스 `tlm_fw_nonblocking_transport_if` / `tlm_bw_nonblocking_transport_if`.
- blocking과 유사하게 트랜잭션의 non-const reference와 timing annotation을 넘기지만, 추가로 **phase**를 넘겨 트랜잭션 상태를 나타내고, **복귀 자체가 phase transition인지**를 나타내는 enum 값을 반환한다.
- 트랜잭션 타입과 phase 타입 **둘 다** 템플릿 파라미터.

#### 11.2.3.2 Class definition

```cpp
namespace tlm {

enum tlm_sync_enum { TLM_ACCEPTED, TLM_UPDATED, TLM_COMPLETED };

template <typename TRANS = tlm_generic_payload, typename PHASE = tlm_phase>
class tlm_fw_nonblocking_transport_if : public virtual sc_core::sc_interface {
public:
    virtual tlm_sync_enum nb_transport_fw(TRANS& trans, PHASE& phase, sc_core::sc_time& t) = 0;
};

template <typename TRANS = tlm_generic_payload, typename PHASE = tlm_phase>
class tlm_bw_nonblocking_transport_if : public virtual sc_core::sc_interface {
public:
    virtual tlm_sync_enum nb_transport_bw(TRANS& trans, PHASE& phase, sc_core::sc_time& t) = 0;
};

} // namespace tlm
```

#### 11.2.3.3 The TRANS and PHASE template arguments

- 임의 타입·임의 개수의 phase와 timing point로 트랜잭션을 옮길 수 있게 하려는 의도. `tlm_generic_payload`와 base protocol용 `tlm_phase`가 제공됨 (15.2).
- **최대 interoperability를 위해 기본 타입 `tlm_generic_payload` + `tlm_phase` + base protocol을 쓸 것(should)**.
- 특정 프로토콜 모델링 시 자체 트랜잭션 타입·phase 타입으로 대체해도 된다(may).
- **다른 트랜잭션 타입으로 특수화된 소켓끼리 bind 불가** (compile-time check, interoperability 제한).

#### 11.2.3.4 The nb_transport_fw and nb_transport_bw calls

| # | 규칙 |
|---|---|
| a | forward path용 `nb_transport_fw`, backward path용 `nb_transport_bw` 두 함수. **이름과 호출 방향을 제외하면 의미론은 동일**. 구분이 불필요할 때 이 문서는 이탤릭 *nb_transport*로 둘 다 지칭. base protocol에서는 forward·backward path가 **정확히 동일한 component·socket 순서를 역순으로** 지나가야 한다(should). 각 component는 initiator 쪽으로 돌아가는 트랜잭션을 **그 트랜잭션이 처음 수신된 target socket을 통해** 라우팅할 책임이 있다. |
| b | `nb_transport_fw`는 **forward path에서만**, `nb_transport_bw`는 **backward path에서만** 호출되어야 shall. |
| c | forward path의 `nb_transport_fw` 호출은 **어떤 경우에도** backward path의 `nb_transport_bw`를 직접·간접으로 호출하면 **안 된다 shall not**. 그 역도 마찬가지. |
| d | *nb_transport*는 직접·간접적으로 **`wait`을 호출하면 안 된다 shall not**. |
| e | *nb_transport*는 **thread process에서도 method process에서도** 호출해도 된다(may). |
| f | *nb_transport*는 **`b_transport`를 호출하는 것이 허용되지 않는다**. 한 가지 해법은 원래의 `nb_transport_fw`가 spawn/notify한 별도 thread process에서 `b_transport`를 호출하는 것. base protocol에는 이 변환을 자동으로 해주는 `simple_target_socket`이 제공됨 (16.2.2). |
| g | non-blocking transport는 **pipelined transaction 지원을 명시적 목적**으로 한다. 같은 process에서 연속으로 `nb_transport_fw`를 호출해 첫 트랜잭션 완료를 기다리지 않고 각각 별개 트랜잭션을 시작할 수 있다. |
| h | 원칙적으로 트랜잭션의 마지막 timing point는 **forward든 backward든** *nb_transport*의 호출 또는 복귀로 표시될 수 있다. |

#### 11.2.3.5 The trans argument

| # | 규칙 |
|---|---|
| a | 트랜잭션 객체의 **수명은 *nb_transport* 복귀 이후로 연장될 수 있다(may)** — 일련의 호출이 하나의 트랜잭션 객체를 initiator·interconnect·target 사이에서 앞뒤로 전달할 수 있다. |
| b | 한 트랜잭션 인스턴스에 여러 *nb_transport* 호출이 연관되면, **모든 호출에 동일한 하나의 트랜잭션 객체를 인자로 넘겨야 shall**. 즉 한 트랜잭션 인스턴스는 **단일 트랜잭션 객체로 표현되어야 shall**. |
| c | initiator는 하나의 트랜잭션 객체를 둘 이상의 트랜잭션 인스턴스에, 또는 transport / DMI / debug transport 호출에 걸쳐 **재사용해도 된다(may)**. |
| d | 수명이 여러 호출에 걸치므로 **caller와 callee 어느 쪽이든** 트랜잭션 객체를 수정·갱신해도 된다(may) — `TRANS`가 부과하는 제약 하에. 예: generic payload에서 target은 read command의 data array를 갱신해도 되지만 **command 필드를 갱신하면 안 된다 shall not** (14.7). |

#### 11.2.3.6 The phase argument

| # | 규칙 |
|---|---|
| a | 각 *nb_transport* 호출은 phase 객체의 **참조**를 넘긴다. base protocol에서는 **같은 phase로 연속 호출하는 것이 허용되지 않는다**. 각 phase transition에는 연관된 timing point가 있다. `sc_time` 인자를 통한 timing annotation은 그 timing point를 phase transition 대비 **지연시켜야 shall**. |
| b | phase 인자는 참조로 전달된다. **caller·callee 어느 쪽이든 phase를 수정해도 된다(may)**. |
| c | phase 인자의 의도는 각 component에게 **트랜잭션 속성을 읽거나 수정해도 되는 시점**을 알리는 것. 프로토콜 규칙이 특정 phase에 특정 component의 속성 수정을 허용한다면, 그 component는 **그 phase 동안 언제든 몇 번이든** 수정해도 된다. 프로토콜은 다른 component가 그 phase 동안 그 속성을 **읽는 것을 금지해야 하고(should)**, 다음 phase transition 이후에만 읽도록 허용해야 한다. |
| d | phase 인자 값은 **해당 hop의 프로토콜 상태 기계의 현재 상태**를 나타낸다. 단일 트랜잭션 객체가 3개 이상 component(initiator·interconnect·target) 사이를 오갈 때, **각 hop마다 (적어도 관념적으로) 별개의 상태 기계**가 필요하다. |
| e | 트랜잭션 객체와 달리 **phase 객체는 통상 caller에 국지적(local)**. 한 트랜잭션의 각 *nb_transport* 호출은 서로 다른 phase 객체를 가질 수 있다. 서로 다른 hop의 대응 phase transition은 **다른 시뮬레이션 시각에 일어날 수 있다**. |
| f | 기본 phase 타입 `tlm_phase`는 **base protocol 전용**. 다른 프로토콜은 `tlm_phase`를 쓰거나 확장하거나, 자체 phase 타입으로 대체해도 된다(may) — **interoperability는 그만큼 잃는다** (15.1). |

#### 11.2.3.7 The tlm_sync_enum return value

- **synchronize(동기화)** = 다른 process가 실행될 수 있도록 SystemC scheduler에 제어를 넘기는 것. temporal decoupling에서는 추가적 함의가 있다 (16.3.4).
- 원칙적으로 동기화는 yield(thread process면 `wait`, method process면 커널로 복귀)로 달성되지만, **temporally decoupled initiator는 `tlm_quantumkeeper`의 `sync` 멤버 함수로 동기화해야 한다(should)**. 일반적으로 initiator는 다른 SystemC process가 실행될 수 있도록 **때때로 동기화할 필요가 있다**.
- 아래 규칙은 **forward·backward path 양쪽에 모두 적용**된다.
- **반환값의 의미는 고정이며 트랜잭션 타입·phase 타입에 따라 달라지지 않는다.** 따라서 base protocol에 국한되지 않고 **모든 트랜잭션·phase 타입에 적용**된다.

| 반환값 | callee의 의무 | caller의 의무 |
|---|---|---|
| `TLM_ACCEPTED` | 호출 동안 **트랜잭션 객체·phase·time 인자의 상태를 수정했으면 안 된다 shall not**. 즉 **반환 경로를 쓰지 않음**을 뜻함. base protocol에서 ignorable phase를 무시하는 callee는 `TLM_ACCEPTED`를 반환해야 한다(should). | callee가 인자를 변경하지 않을 의무가 있으므로 **호출 후 인자 값을 무시해도 된다(may)**. 일반적으로 callee를 담은 component가 응답하려면 caller가 **먼저 yield 해야 한다**. |
| `TLM_UPDATED` | 트랜잭션 객체를 **갱신했다**. phase 상태를 수정했을 수 있고(may), 트랜잭션 객체 상태를 수정했을 수 있고, time 인자 값을 **증가**시켰을 수 있다. 즉 **반환 경로를 사용 중**이고 callee가 프로토콜 상태 기계를 진전시켰음을 뜻함. 각 인자를 실제로 수정할 의무가 있는지는 프로토콜에 달려 있다. | 호출 후 **phase·트랜잭션·time 인자를 검사하고 적절히 행동해야 한다(should)**. |
| `TLM_COMPLETED` | 트랜잭션 객체를 갱신했고 **트랜잭션이 완료**되었다. 트랜잭션 객체 상태를 수정했을 수 있고 time 인자를 증가시켰을 수 있다(may). **phase 인자의 값은 undefined**. 즉 반환 경로를 사용 중이며 **해당 소켓에 관한 한 트랜잭션이 완료**됨. | 트랜잭션 객체를 검사하고 적절히 행동하되 **phase 인자는 무시해야 한다(should)**. **현재 소켓을 통해 forward·backward 어느 경로로도 이 트랜잭션에 관한 추가 transport 호출이 있어서는 안 된다 shall not.** 여기서 completion이 **성공적 완료를 뜻하지는 않는다** — 트랜잭션 타입에 따라 caller는 트랜잭션 객체에 내장된 response status를 검사해야 할 수 있다. |

- **`TLM_COMPLETED`로 완료할 의무는 일반적으로 없다.** 프로토콜의 마지막 phase가 *nb_transport*에 인자로 전달되면 그 소켓에 대해 트랜잭션은 어차피 완료된다 (base protocol의 마지막 phase는 `END_RESP`). **`TLM_COMPLETED`는 mandatory가 아니다.**
- 세 반환값 **어느 것이든**, 프로토콜에 따라 호출 후 caller는 callee를 담은 component가 응답을 생성하거나 트랜잭션 객체를 해제할 수 있도록 **yield 해야 할 수 있다(may)**.

#### 11.2.3.8 tlm_sync_enum summary (Table 53)

| `tlm_sync_enum` | 트랜잭션 객체 | phase | time 인자 |
|---|---|---|---|
| `TLM_ACCEPTED` | Unmodified | Unchanged | Unchanged |
| `TLM_UPDATED` | Updated | Changed | May be increased |
| `TLM_COMPLETED` | Updated | Ignored | May be increased |

> 컬럼 헤더는 §11.2.3.7 e)~g)의 서술로부터 복원한 것이다. 데이터 행은 원문 그대로.

#### 11.2.3.9–11.2.3.12 Message sequence charts (informative)

표기: `return, phase, delay` (반환값, phase 인자 값, `sc_time` 인자 값). `'-'`는 값이 쓰이지 않음을 뜻함. 예시는 base protocol phase(`BEGIN_REQ`, `END_REQ`, `BEGIN_RESP`, `END_RESP`)를 사용.

> - **backward path 사용 (Figure 23)**: *nb_transport* 수신자가 트랜잭션의 다음 상태나 다음 timing point까지의 delay를 즉시 계산할 수 없으면 **`TLM_ACCEPTED`를 반환해야 한다(should)**. caller는 스케줄러에 제어를 넘기고, callee가 응답할 준비가 되면 **반대 경로로 *nb_transport* 호출이 오기를 기대**한다. loosely-timed와 달리 여기서 caller는 initiator일 수도 target일 수도 있다. 트랜잭션은 pipeline 될 수 있다 — initiator는 이전 트랜잭션의 마지막 phase transition을 보기 전에 다음 트랜잭션을 보낼 수 있다. process가 자주 yield 하므로 **approximately-timed는 loosely-timed보다 훨씬 느리게 시뮬레이션될 것으로 예상**된다.
> - **return path 사용 (Figure 24)**: 다음 상태와 delay를 즉시 계산할 수 있으면 반대 경로 대신 **복귀 시 새 상태를 반환해도 된다(may)**. 다음 timing point가 트랜잭션의 끝이면 `TLM_UPDATED` 또는 `TLM_COMPLETED` 중 하나를 반환할 수 있다. callee는 (프로토콜 규칙 하에) **어느 단계에서든 `TLM_COMPLETED`를 반환**해 나머지 phase를 선점하고 마지막 phase로 건너뛸 수 있으며, 이는 **initiator·target 모두에 적용**된다. `TLM_UPDATED`일 때 callee는 트랜잭션·phase·timing annotation을 갱신해야 한다(should).
> - **early completion (Figure 25)**: 프로토콜에 따라 initiator나 target은 **어느 시점에든 `TLM_COMPLETED`를 반환해 트랜잭션을 조기 완료**해도 된다. 이후 **initiator도 target도 이 트랜잭션 인스턴스에 대해 추가 *nb_transport* 호출을 할 수 없다**. 이런 단축이 실제로 허용되는지는 특정 프로토콜의 규칙에 달려 있다. 이때 `BEGIN_REQ` → `END_REQ` → `BEGIN_RESP` → `END_RESP`의 phase transition은 인자로 명시되지 않고 **암묵적**이다.
> - **timing annotation (Figure 26)**: caller가 *nb_transport* 호출에 delay를 annotate 하면 이는 callee에게 **"그만큼 지연되어 수신된 것처럼 처리하라"**는 지시. approximately-timed callee는 통상 트랜잭션을 **payload event queue**에 넣어 시뮬레이션 시각이 따라잡았을 때 처리한다. delay는 forward·backward path와 각각의 return path에 모두 annotate 될 수 있다. approximately-timed initiator는 forward return path와 backward path의 수신 트랜잭션을 동일하게 처리하고, approximately-timed target은 backward return path와 forward path를 동일하게 처리할 것으로 기대된다.

---

### 11.2.4 Timing annotation with the transport interfaces

#### 11.2.4.1 Overview

- timing annotation은 blocking·non-blocking transport가 **공유**하는 기능으로, `b_transport` / `nb_transport_fw` / `nb_transport_bw`의 `sc_time` 인자로 표현된다. 이 문서는 이 세 함수를 통칭해 이탤릭 *transport*로 부른다.
- 트랜잭션 순서는 **core interface 규칙 + 프로토콜 규칙의 조합**으로 지배된다. 아래 규칙은 **프로토콜 선택과 무관하게 core interface에 적용**되며, base protocol에서는 15.2.7과 함께 읽어야 한다(should).

#### 11.2.4.2 The sc_time argument — 규칙 전부

| # | 규칙 |
|---|---|
| a | 트랜잭션 객체는 **timing 정보를 담지 않는 것이 권장(recommended)**. 모든 timing annotation은 *transport*의 `sc_time` 인자로 표현할 것(should). |
| b | time 인자는 **non-negative여야 shall**, 그리고 **현재 시뮬레이션 시각 `sc_time_stamp()` 기준 상대값으로 표현되어야 shall**. |
| c | time 인자는 *transport*의 **호출 시와 복귀 시 모두에 적용되어야 shall** (*nb_transport*의 `tlm_sync_enum` 반환값 규칙에 따름). |
| d | *nb_transport*는 time 인자 값을 **증가시켜도 되지만(may) 감소시키면 안 된다 shall not**. `b_transport`는 값을 증가시켜도 되고, **`wait`을 호출해 시뮬레이션 시각과 동기화한 경우에 한해 감소시켜도 되지만(may) — 감소량은 process가 suspend 되어 있던 시간 이하여야 한다**. 이는 SystemC 시뮬레이션에서 **시간이 거꾸로 흐르지 않는다**는 원칙과 일관된다. |
| e | 아래 설명에서 *transport* **호출 시** 트랜잭션 수신자는 **callee**, **복귀 시** 수신자는 **caller**. |
| f | 수신자는 **effective local time `sc_time_stamp() + t`에 트랜잭션을 받은 것처럼 동작해야 shall**. 즉 IMC에 연관된 timing point가 그 effective local time에 일어날 것처럼 동작해야 shall. |
| g | 일련의 *transport* 호출에서, 트랜잭션이 처리될 effective local time은 **일반적으로 증가 순서일 수도 아닐 수도 있다**. 서로 다른 initiator가 만든 트랜잭션에 대해서는 **IMC 호출 순서와 effective local time 순서가 다를 수 있다는 것이 temporal decoupling의 근본**. 순서가 뒤바뀐 timing annotation을 처리할 **책임은 수신자에게 있다**. |
| h | 0이 아닌 timing annotation을 받은 수신자는 **속도 vs 정확도 trade-off의 선택지**를 갖는다: (1) 상태 변화를 즉시 실행하고 timing annotation을 (증가시켜) 넘기거나, (2) 내부 process를 annotated time의 일부 또는 전부 경과 후 재개하도록 스케줄하고 그때 상태 변화를 실행. 이 선택은 **transport interface가 강제하지 않으며** protocol traits class나 coding style로 문서화될 수 있다. |
| i | timing 정확도나 annotation 순서대로의 처리에 관심이 없으면 **각 트랜잭션을 지연 없이 즉시 처리해도 된다(may)**. 그 후 처리 시간을 모델링하기 위해 timing annotation 값을 **증가시켜도 된다(may)**. 이 시나리오는 시스템 설계가 TLM-2.0 인터페이스 외의 명시적 메커니즘으로 올바른 인과 사슬을 강제하여 **out-of-order 실행을 허용한다는 전제**를 둔다. |
| j | 정확한 timing·실행 순서 모델을 구현하려면, 상호작용하는 다른 SystemC process 대비 **올바른 시각에 트랜잭션을 처리해야 한다(should)**. SystemC에서 미래 시각 이벤트 스케줄링의 적절한 메커니즘은 **timed event notification**. TLM-2.0은 편의를 위해 **payload event queue** 유틸리티 클래스군을 제공한다 (16.4). 즉 approximately-timed 수신자는 통상 트랜잭션을 payload event queue에 넣어야 한다(should). |
| k | 트랜잭션을 직접 처리하는 대신, **수정 없이 동일한 phase·timing annotation으로(또는 증가된 annotation으로)** 다음 *transport* 호출·복귀로 넘겨도 된다(may). |
| l | **loosely-timed** style에서는 트랜잭션이 보통 즉시 실행되어 실행 순서 = IMC 호출 순서가 되며, **`b_transport`가 권장(recommended)**. |
| m | **approximately-timed** style에서는 보통 트랜잭션이 지연되어 실행 순서 = effective local time 순서가 되며, ***nb_transport*가 권장(recommended)**. |
| n | 각 component는 위 선택을 **호출 단위로 동적으로** 할 수 있다. 예: loosely-timed component가 일련의 트랜잭션을 호출 순서대로 즉시 실행하며 annotation을 넘기다가, 바로 다음 트랜잭션은 annotation 지연 경과 후에만 실행하도록 스케줄할 수도 있다(**synchronization on demand**). 이는 coding style의 문제. |
| o | 위 선택은 **blocking·non-blocking 모두에 존재**. 예: `b_transport`는 annotation을 늘리고 즉시 반환해도 되고, annotation이 경과할 때까지 기다린 뒤 반환해도 된다. *nb_transport*는 annotation을 늘리고 `TLM_COMPLETED`를 반환해도 되고, `TLM_ACCEPTED`를 반환하고 나중 실행을 스케줄해도 된다. |
| p | 위 규칙의 귀결로, 수신 IMC 순서와 effective local time 순서가 다른 일련의 트랜잭션을 받은 component는 **그 트랜잭션들의 상호 실행 순서를 자유롭게 선택할 수 있다**. **권장은 전부 호출 순서로 실행하거나 전부 effective local time 순서로 실행하는 것이지만, 의무는 아니다**. |
| q | 실제로 수신 트랜잭션의 실행 순서는 **사실상 항상 IMC 호출 순서와 같아야 한다(should)** — component는 annotation과 무관하게 IMC 복귀 전에 실행하거나(loosely-timed), 적절한 미래 시각에 실행을 스케줄하고 `TLM_ACCEPTED`를 반환(approximately-timed)하여 caller에게 다음 트랜잭션 발행 전에 기다리라고 알리기 때문. (**`TLM_ACCEPTED` 자체는 caller의 다음 트랜잭션 발행을 금지하지 않지만**, base protocol에서는 request/response exclusion rule이 금지할 수 있다.) |
| r | timing annotation은 temporal decoupling 관점으로도 설명된다. 0이 아닌 annotation은 수신자에게 **"시간을 warp 하라"는 초대**로 볼 수 있다. 수신자는 time warp에 들어가도 되고, 나중 처리를 위해 큐에 넣고 yield 해도 된다. loosely-timed 모델에서는 time warping이 일반적으로 수용 가능. 반면 target이 다른 비동기 이벤트에 의존한다면 트랜잭션의 미래 상태를 확실히 예측하기 위해 **시뮬레이션 시각이 진행하기를 기다려야 할 수 있다**. |
| s | temporal decoupling 일반 설명은 10.3.3 참조. |
| t | quantum 설명은 16.3 참조. |

#### Example (informative)

```cpp
// -------------------------------------------------------------------------------------
// 각종 interface method 정의
// -------------------------------------------------------------------------------------

void b_transport(TRANS &trans, sc_core::sc_time &t) {
   // Loosely-timed coding style
   execute_transaction(trans);
   t = t + latency;
}

void b_transport(TRANS &trans, sc_core::sc_time &t) {
   // target에서 동기화하는 loosely-timed, 또는 synchronization-on-demand
   wait(t);
   execute_transaction(trans);
   t = sc_core::SC_ZERO_TIME;
}

tlm::tlm_sync_enum nb_transport_fw(TRANS &trans, PHASE &phase, sc_core::sc_time &t) {
    // non-blocking transport를 쓰는 pseudo-loosely-timed coding style (권장되지 않음)
    execute_transaction(trans);
    t = t + latency;
    return tlm::TLM_COMPLETED;
}

tlm::tlm_sync_enum nb_transport_fw(TRANS &trans, PHASE &phase, sc_core::sc_time &t) {
    // Approximately-timed coding style
    // 시각 sc_time_stamp() + t 에 실행하도록 payload event queue에 트랜잭션을 넣는다
    payload_event_queue->notify(trans, phase, t);
    // 트랜잭션 reference count 증가
    trans.acquire();
    return tlm::TLM_ACCEPTED;
}

tlm::tlm_sync_enum nb_transport_fw(TRANS &trans, PHASE &phase, sc_core::sc_time &t) {
    // backward path를 사용하는 approximately-timed coding style
    payload_event_queue->notify(trans, phase, t);
    trans.acquire();
    // phase와 time 인자를 수정
    phase = tlm::END_REQ;
    t = t + accept_delay;
    return tlm::TLM_UPDATED;
}

// -------------------------------------------------------------------------------------
// b_transport interface method 호출, loosely-timed coding style
// -------------------------------------------------------------------------------------

initialize_transaction(T1);
socket->b_transport(T1, t);                                      // t는 증가할 수 있다
process_response(T1);

initialize_transaction(T2);
socket->b_transport(T2, t);                                      // t는 증가할 수 있다
process_response(T2);

// initiator는 트랜잭션마다 또는 일련의 트랜잭션 후에 sync 할 수 있다
quantum_keeper->set(t);
if (quantum_keeper->need_sync())
     quantum_keeper->sync();

// -------------------------------------------------------------------------------------
// nb_transport interface method 호출, approximately-timed coding style
// -------------------------------------------------------------------------------------

initialize_transaction(T3);
status = socket->nb_transport_fw(T3, phase, t);

if (status == tlm::TLM_ACCEPTED) {
     // 아무 동작 없음. 단, nb_transport_bw 호출이 들어올 것을 기대
} else if (status == tlm::TLM_UPDATED) {                // backward path 사용 중
     payload_event_queue->notify(T3, phase, t);
} else if (status == tlm::TLM_COMPLETED) {              // 조기 완료
     // timing annotation은 여러 방법 중 하나로 반영 가능
     // (1) 여기처럼 기다리거나
     wait(t);
     process_response(T3);
     // (2) event notification을 만들거나
     // response_event.notify( t );
     // (3) 다음 transport method 호출로 넘기거나 (코드 생략)
}
```

---

### 11.2.5 Migration path from TLM-1

- 구 TLM-1 인터페이스와 신 TLM-2.0 인터페이스는 **둘 다 TLM-2.0 표준의 일부**. TLM-1 blocking/non-blocking 인터페이스는 그 자체로 여전히 유용하다 (예: HDL 설계의 기능 검증 환경 구축).
- 구·신 blocking transport 인터페이스의 유사성이 **legacy TLM-1 모델과 TLM-2.0 사이의 adapter 작성을 쉽게 하려는 의도**.

---

## 11.3 Direct memory interface (DMI)

### 11.3.1 Introduction

- DMI는 initiator가 target 소유 메모리 영역에 **직접 포인터로 접근**하게 하는 수단. 한 번 확립되면 initiator → interconnect → target으로 이어지는 다중 `b_transport`/*nb_transport* 호출 경로를 **우회**하여 시뮬레이션 속도를 크게 높일 수 있다.
- 인터페이스는 **두 개**: forward path(initiator → target)용, backward path(target → initiator)용.
  - **forward**: 주어진 주소에 대해 특정 접근 모드(예: read/write)를 요청하고, DMI region 경계를 담은 **`tlm_dmi` 타입 DMI descriptor의 참조를 반환**.
  - **backward**: target이 앞서 확립된 DMI 포인터를 **무효화**할 때 사용.
- forward/backward path는 interconnect component를 0개·1개·다수 지날 수 있으나, **같은 소켓을 지나는 대응 transport 호출의 forward/backward path와 동일해야 한다(should)**.
- 기본 DMI 트랜잭션 타입은 `tlm_generic_payload`이며 **command와 address 속성만** 사용된다.
- DMI는 transport interface와 **동일한 확장 방식**을 따른다 — DMI request는 ignorable extension을 담을 수 있지만, **non-ignorable/mandatory extension은 새 protocol traits class 정의를 요구** (14.2.3).
- DMI descriptor는 initiator가 사용할 **latency 값을 반환**하여 loosely-timed 모델링에 충분한 timing 정확도를 제공한다.
- DMI 포인터를 debug에 써도 되지만, debug 트래픽은 보통 가볍고 메모리 접근보다 I/O가 지배적이므로 **debug transport interface 자체로 대개 충분**. DMI 포인터를 debug에 쓴다면 **latency 값은 무시해야 한다(should)**.

### 11.3.2 Class definition

```cpp
namespace tlm {

class tlm_dmi
{
public:
    tlm_dmi() { init(); }

    void init();

    enum dmi_access_e {
       DMI_ACCESS_NONE = 0x00,
       DMI_ACCESS_READ = 0x01,
       DMI_ACCESS_WRITE = 0x02,
       DMI_ACCESS_READ_WRITE = DMI_ACCESS_READ | DMI_ACCESS_WRITE
    };

    unsigned char*   get_dmi_ptr() const;
    sc_dt::uint64    get_start_address() const;
    sc_dt::uint64    get_end_address() const;
    sc_core::sc_time get_read_latency() const;
    sc_core::sc_time get_write_latency() const;
    dmi_access_e     get_granted_access() const;
    bool is_none_allowed() const;
    bool is_read_allowed() const;
    bool is_write_allowed() const;
    bool is_read_write_allowed() const;

    void set_dmi_ptr(unsigned char* p);
    void set_start_address(sc_dt::uint64 addr);
    void set_end_address(sc_dt::uint64 addr);
    void set_read_latency(sc_core::sc_time t);
    void set_write_latency(sc_core::sc_time t);
    void set_granted_access(dmi_access_e t);
    void allow_none();
    void allow_read();
    void allow_write();
    void allow_read_write();
};

template <typename TRANS = tlm_generic_payload>
class tlm_fw_direct_mem_if : public virtual sc_core::sc_interface
{
public:
    virtual bool get_direct_mem_ptr(TRANS& trans, tlm_dmi& dmi_data) = 0;
};

class tlm_bw_direct_mem_if : public virtual sc_core::sc_interface
{
public:
    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) = 0;
};

} // namespace tlm
```

> `tlm_bw_direct_mem_if`는 **템플릿이 아니다** (트랜잭션 타입에 독립).

### 11.3.3 get_direct_mem_ptr

| # | 규칙 |
|---|---|
| a | `get_direct_mem_ptr`는 **initiator나 interconnect component만 호출해야 shall** — target은 호출하지 않는다. |
| b | `trans` 인자는 **initiator가 생성한** DMI 트랜잭션 객체의 참조를 넘겨야 shall. |
| c | `dmi_data` 인자는 **initiator가 생성한** DMI descriptor의 참조여야 shall. |
| d | interconnect component는 호출을 forward path로 넘겨야 한다(should). 즉 target socket용 구현이 initiator socket의 `get_direct_mem_ptr`를 호출해도 된다(may). |
| e | 각 `get_direct_mem_ptr` 호출은 **대응하는 transport 호출과 정확히 같은 경로**를 따라야 shall. 즉 각 DMI request는 **하나의 initiator와 하나의 target 간 상호작용**이어야 하며, 그 둘은 transport interface로 단일 트랜잭션 객체를 주고받는 initiator/target 역할도 해야 한다. **두 번째 트랜잭션 객체를 만드는 component(예: non-trivial width converter)를 지나는 경로에서는 DMI를 쓸 수 없다.** (DMI가 속도상 필수라면 모델 구조를 재조정해야 할 수 있다.) |
| f | interconnect component는 `trans`와 `dmi_data`를 forward 방향으로 넘겨야 shall하며, **허용되는 유일한 수정은 DMI 트랜잭션 객체의 address 속성 값**뿐. 트랜잭션의 address 속성과 DMI descriptor는 **복귀 시(target → initiator로 호출 스택을 풀 때) 둘 다 수정될 수 있다**. |
| g | target이 해당 주소에 DMI 접근을 지원할 수 있으면 **DMI descriptor 멤버를 설정하고 반환값을 `true`로 해야 shall**. target이 DMI를 허가할 때 DMI descriptor는 **허가되는 접근의 상세를 나타낸다**. |
| h | 지원할 수 없으면 **주소 범위와 타입 멤버만 설정하고 반환값을 `false`로 해야 shall**. target이 DMI를 거부할 때 DMI descriptor는 **거부되는 접근의 상세를 나타낸다**. |
| i | target은 이 절의 규칙 하에 **메모리 영역의 임의 부분(비연속 영역 포함)에 대해 DMI를 허가하거나 거부해도 된다(may)**. |
| j | target이 허가하고 `true`를 설정한 경우, **interconnect component가 복귀 시 반환값을 `false`로 바꿔 거부해도 된다(may)**. **역은 허용되지 않는다 — target이 거부했으면 interconnect가 허가하면 안 된다 shall not.** |
| k | 여러 호출에 대해 target은 **동일 메모리 영역을 여러 initiator에게 동시에 허가해도 된다(may)**. **동기화와 coherency 책임은 애플리케이션에 있다.** |
| l | 각 호출은 **연속(contiguous) 메모리 영역에 대한 단일 DMI 포인터만** 반환할 수 있으므로, 실제로 각 DMI request는 **단일 target만이 충족**할 수 있다. 주소 범위가 연속이라도 메모리가 여러 target에 흩어져 있으면 **target마다 별도 DMI request가 필요**할 가능성이 높다. |
| m | 어떤 영역에 대한 read/write가 target에 **side effect**(메모리 상태 외의 다른 상태 변화)를 일으킨다면, target은 그 타입의 DMI 접근을 **허가하지 않아야 한다(should not)**. 단 예컨대 write만 side effect가 있다면 **read DMI는 허가해도 된다(may)**. |
| n | `get_direct_mem_ptr` 구현은 **`invalidate_direct_mem_ptr`를 호출해도 된다(may)**. |
| o | `get_direct_mem_ptr` 구현은 직접·간접적으로 **`wait`을 호출하면 안 된다 shall not**. |

### 11.3.4 TRANS template argument and tlm_generic_payload class

| # | 규칙 |
|---|---|
| a | `tlm_fw_direct_mem_if` 템플릿은 **DMI 트랜잭션 클래스 타입으로 파라미터화되어야 shall**. |
| b | 트랜잭션 객체는 **직접 메모리 접근이 요청되는 주소**와 **요청 접근 타입(read/write)**을 나타내는 속성을 담아야 shall. base protocol에서는 generic payload의 **command와 address 속성**이어야 shall. |
| c | `TRANS` 템플릿 인자의 **기본값은 `tlm_generic_payload` 클래스여야 shall**. |
| d | 최대 interoperability를 위해 DMI 트랜잭션 클래스는 `tlm_generic_payload`여야 한다(should). non-ignorable extension이나 다른 트랜잭션 타입 사용은 interoperability를 제한한다. |
| e | **initiator가 DMI 트랜잭션 객체를 생성·관리하고**, `get_direct_mem_ptr`에 넘기기 전에 적절한 속성을 설정할 **책임을 져야 shall**. |
| f | **command 속성은 initiator가 설정해야 shall**하며 **interconnect나 target이 수정하면 안 된다 shall not**. base protocol에서 허용값은 read 접근에 `TLM_READ_COMMAND`, write 접근에 `TLM_WRITE_COMMAND`. |
| g | base protocol에서 command 속성이 **`TLM_IGNORE_COMMAND` 값을 갖는 것은 금지(forbidden)**. 단 다른 프로토콜에서는 이 값을 써도 된다(may). |
| h | **address 속성은 initiator가** 직접 메모리 접근을 요청하는 주소로 설정해야 shall. |
| i | DMI 트랜잭션 객체를 forward path로 넘기는 interconnect component는 **같은 소켓의 대응 transport interface에서와 정확히 동일하게** address 속성을 decode·필요 시 수정해야 한다(should). 예: target의 주소 폭과 system memory map 상 위치에 따라 address를 mask. |
| j | interconnect component는 **addressing error를 감지하면 호출을 넘기지 않아도 된다**. |
| k | base protocol에서 generic payload option 속성이 `TLM_MIN_PAYLOAD`이면, initiator는 **command와 address 외의 속성을 설정할 의무가 없고**, target·interconnect는 다른 모든 속성을 무시해도 된다(may) — 특히 response status와 DMI allowed 속성을 무시해도 된다. target이 option 속성을 `TLM_FULL_PAYLOAD_ACCEPTED`로 설정하면 **target은 response status를 설정해야 shall**하고 initiator는 14.8에 따라 response status를 검사해야 한다(should). **DMI allowed 속성은 transport·debug transport interface 전용 의도.** |
| l | initiator는 트랜잭션 객체를 DMI 호출 간, 그리고 DMI / transport / debug transport 호출에 걸쳐 **재사용해도 된다(may)**. |
| m | DMI 트랜잭션에 속성을 더 추가해야 하면, 무관한 트랜잭션 클래스로 대체하기보다 **generic payload에 extension을 추가하는 것이 권장**. 예: target이 CPU 종류별로 DMI request를 다르게 처리하도록 CPU ID를 담기. 그런 extension이 non-ignorable이면 **새 protocol traits class 정의가 필요**. |

### 11.3.5 tlm_dmi class

**속성과 기본값** (규칙 b, 각 항의 initial value 언급을 종합):

| 속성 | 기본값 | 설정/조회 |
|---|---|---|
| DMI pointer | `0` | `set_dmi_ptr` / `get_dmi_ptr` |
| granted access type | `DMI_ACCESS_NONE` | `set_granted_access` / `get_granted_access`, `allow_*`, `is_*_allowed` |
| start address | `0` | `set_start_address` / `get_start_address` |
| end address | `sc_dt::uint64`의 **최대값** | `set_end_address` / `get_end_address` |
| read latency | `SC_ZERO_TIME` | `set_read_latency` / `get_read_latency` |
| write latency | `SC_ZERO_TIME` | `set_write_latency` / `get_write_latency` |

| # | 규칙 |
|---|---|
| a | DMI descriptor는 `tlm_dmi` 객체. **initiator가 생성해야 shall**하되, 멤버는 **interconnect나 target이 설정해도 된다(may)**. |
| b | 위 표의 6개 속성을 가져야 shall하며 기본값은 위와 같아야 shall. |
| c | `init`은 멤버를 **기본값으로 초기화해야 shall**. |
| d | DMI descriptor는 initiator가 `get_direct_mem_ptr`에 인자로 넘길 때 **반드시 기본 상태(default state)여야 shall**. DMI descriptor 객체를 pooling 한다면 initiator는 **넘기기 전에 기본 상태로 리셋해야 shall** — 이를 위해 `init`을 호출해도 된다(may). |
| e | interconnect는 target 방향으로 넘길 때 DMI descriptor 수정이 허용되지 않으므로, **target이 받을 때 descriptor는 초기 상태여야 shall**. |
| f | `set_dmi_ptr`는 DMI pointer 속성을 인자 값으로 설정해야 shall. `get_dmi_ptr`는 현재 값을 반환해야 shall. |
| g | **DMI pointer 속성은 target이** start address 속성 값에 대응하는 저장 위치를 가리키도록 설정해야 shall. 이 start address는 `get_direct_mem_ptr` 호출에서 요청된 주소 **이하여야 shall**. 초기값은 `0`이어야 shall. |
| h | DMI region의 저장소는 `unsigned char*` 타입으로 표현되며, **generic payload의 data array와 동일한 조직(organization)을 가져야 shall**. target이 그런 조직의 메모리 영역 포인터를 반환할 수 없으면 DMI를 지원할 수 없는 것이며 `get_direct_mem_ptr`는 **`false`를 반환해야 한다(should)**. 메모리 조직·endianness 전체 설명은 14.18 참조. |
| i | interconnect component는 **복귀 경로에서 DMI pointer 속성을 수정**하여 허가되는 영역을 **제한해도 된다(permitted)**. |
| j | `set_granted_access` / `get_granted_access`는 granted access type 속성을 설정/반환해야 shall. 초기값은 `DMI_ACCESS_NONE`이어야 shall. |
| k | `allow_none` / `allow_read` / `allow_write` / `allow_read_write`는 각각 granted access type을 `DMI_ACCESS_NONE` / `DMI_ACCESS_READ` / `DMI_ACCESS_WRITE` / `DMI_ACCESS_READ_WRITE`로 설정해야 shall. |
| l | `is_none_allowed`는 granted access type이 `DMI_ACCESS_NONE`일 때에만(iff) `true`를 반환해야 shall. `is_read_allowed`는 `DMI_ACCESS_READ` 또는 `DMI_ACCESS_READ_WRITE`일 때에만 shall. `is_write_allowed`는 `DMI_ACCESS_WRITE` 또는 `DMI_ACCESS_READ_WRITE`일 때에만 shall. `is_read_write_allowed`는 `DMI_ACCESS_READ_WRITE`일 때에만 shall. |
| m | **target은 granted access type을 허가 또는 거부되는 접근 타입으로 설정해야 shall.** target은 read 요청에 read 또는 read/write를 허가(또는 거부)할 수 있고, write 요청에 write 또는 read/write를 허가(또는 거부)할 수 있다. interconnect component는 복귀 경로에서 `DMI_ACCESS_READ_WRITE`를 `DMI_ACCESS_READ`나 `DMI_ACCESS_WRITE`로 덮어써 **허가 타입을 제한해도 된다(permitted)**. |
| n | **read와 write 접근을 모두 거부하려는 target은 granted access type을 `DMI_ACCESS_READ_WRITE`로 설정해야 하며(should), `DMI_ACCESS_NONE`으로 설정하면 안 된다.** |
| o | target은 read/write/read-write가 아닌 **다른 종류의 접근**(DMI 트랜잭션 객체 extension으로 요청된)을 허가/거부함을 나타내려면 granted access type을 `DMI_ACCESS_NONE`으로 설정해야 한다(should). 이 값은 extension이 사전 정의된 접근 타입을 불필요·무의미하게 만드는 경우에만 써야 하며(should), **base protocol에서는 쓰면 안 된다(should not)**. |
| p | **initiator는 granted access type으로 허가된 모드의 DMI 접근만 사용할 책임이 있다** (base protocol이 아닌 경우 extension이나 다른 DMI 트랜잭션 타입으로 허가된 것 포함). |
| q | `set_start_address`/`set_end_address`는 인자 값으로 설정해야 shall, `get_start_address`/`get_end_address`는 현재 값을 반환해야 shall. |
| r | **start/end address 속성은 target이 설정(또는 interconnect가 수정)하여 DMI region의 첫 바이트와 마지막 바이트 주소를 가리켜야 shall.** 그 region이 허가인지 거부인지는 `get_direct_mem_ptr`의 반환값(`true`/`false`)으로 결정된다. 전체 메모리 영역을 거부하려는 target은 start=0, end=`sc_dt::uint64` 최대값으로 설정해도 된다(may). |
| s | target은 **호출당 단일 연속 영역만** 허가·거부할 수 있다. start==end로 단일 주소를 지정할 수도, 임의로 큰 영역을 지정할 수도 있다. |
| t | 특정 타입의 DMI 접근을 특정 영역에 허가받은 initiator는 **무효화될 때까지 그 영역 어디서든** 해당 타입의 접근을 수행해도 된다(may). 즉 **DMI request의 주소로 제한되지 않는다**. |
| u | 호출을 넘기는 interconnect component는 **address 인자에 하듯 start/end address 속성도 변환할 의무가 있다**. DMI descriptor의 주소 변환은 **`get_direct_mem_ptr` 복귀 경로에서 일어나야 shall**. 예: target이 자신이 아는 memory map 내 상대 주소로 start address를 설정하면, interconnect가 그것을 system memory map의 절대 주소로 되돌릴 의무가 있다. 초기값은 각각 `0`과 `sc_dt::uint64` 최대값이어야 shall. |
| v | interconnect component는 start/end address를 수정하여 **허가 영역을 좁히거나 거부 영역을 넓혀도 된다(permitted)**. |
| w | `get_direct_mem_ptr`가 `true`를 반환하면 start/end address가 나타내는 영역은 **DMI 접근이 허용되는(allowed) 영역**, `false`를 반환하면 **허용되지 않는(disallowed) 영역**. |
| x | 두 번 이상의 호출을 받은 target/interconnect는 **겹치는 allowed 영역 여러 개**를, 또는 **겹치는 disallowed 영역 여러 개**를 반환해도 된다(may). |
| y | target/interconnect는 **동일 접근 타입에 대해 하나는 allowed, 다른 하나는 disallowed인 겹치는 DMI 영역을, 첫 영역을 `invalidate_direct_mem_ptr`로 무효화하는 중간 호출 없이 반환하면 안 된다 shall not** (예: 둘 다 read 또는 read/write, 둘 다 write 또는 read/write인 경우). |
| z | 즉 **DMI 영역의 정의는 생성 순서에 의존해서는 안 된다 shall not** — 중간에 `invalidate_direct_mem_ptr`로 첫 영역을 무효화한 경우는 예외. 구체적으로, **disallowed 영역 생성이 동일 접근 타입의 기존 allowed 영역에 구멍을 뚫는 것은 허용되지 않아야 shall**, 그 역도 마찬가지. |
| aa | target이 (DMI를 전혀 지원하지 않아서 등의 이유로) **전체 주소 공간을 disallow** (start=0, end=최대값) 할 수 있는데, 이 경우 interconnect component는 그 disallowed 영역을 **target이 차지하는 memory map 부분으로 clip 해야 한다(should)**. clip 하지 않으면 initiator가 **시스템 전체 주소 공간에서 DMI가 금지되었다고 오해**하게 된다. |
| ab | `set_read_latency`/`set_write_latency`는 인자 값으로 설정해야 shall, `get_read_latency`/`get_write_latency`는 현재 값을 반환해야 shall. |
| ac | **read/write latency 속성은 바이트당 평균 latency로 설정되어야 shall.** 즉 direct memory 연산을 수행하는 initiator는 **DMI descriptor의 latency × 동등한 transport 트랜잭션이 전송했을 바이트 수**로 실제 latency를 계산해야 shall. 초기값은 `SC_ZERO_TIME`이어야 shall. **interconnect와 target 모두 latency 값을 증가시켜도 되어(may)**, DMI descriptor가 target에서 initiator로 되돌아오면서 latency가 누적된다. granted access type 값에 따라 **둘 중 하나 또는 둘 다가 유효**하다. |
| ad | **initiator는 DMI 포인터로 메모리에 접근할 때마다 latency를 존중할 책임이 있다.** 무시하면 **timing 부정확**이 생길 수 있다. |

**Example (informative)** — 전체 메모리 영역에 대한 DMI 거부:

```cpp
bool get_direct_mem_ptr( TRANS& trans, tlm::tlm_dmi& dmi_data ) {
   // 전체 메모리 영역에 대해 DMI 접근을 거부
   dmi_data.allow_read_write();
   dmi_data.set_start_address( 0x0 );
   dmi_data.set_end_address( (sc_dt::uint64)-1 );
   return false;
}
```

### 11.3.6 invalidate_direct_mem_ptr

| # | 규칙 |
|---|---|
| a | `invalidate_direct_mem_ptr`는 **target 또는 interconnect component만 호출해야 shall**. |
| b | **target은 기존 DMI region의 유효성이나 접근 타입을 바꿀 변경이 일어나기 전에 반드시 호출할 의무가 있다** — 예: 기존 DMI region의 주소 범위를 좁히기 전, 접근 타입을 read/write에서 read로 바꾸기 전, 주소 공간을 re-map 하기 전. |
| c | `start_range`와 `end_range` 인자는 무효화할 주소 범위의 **첫 주소와 마지막 주소여야 shall**. |
| d | 호출을 받은 initiator는 그 주소 범위와 **겹치는 모든 기존 DMI region을 즉시 무효화하고 폐기해야 shall**. |
| e | 부분적으로만 겹치는 경우 initiator는 **기존 region의 경계를 조정하거나 region 전체를 무효화해도 된다(may)**. |
| f | 각 DMI region은 **target이 `invalidate_direct_mem_ptr`로 명시적으로 무효화할 때까지 유효해야 shall**. 각 initiator는 유효 DMI region 표를 유지하고 무효화 전까지 계속 사용해도 된다(may). |
| g | interconnect component는 호출을 **backward path로 넘길 의무가 있으며**, 대응 transport interface에서와 같이 address 인자를 decode·필요 시 수정해야 한다. transport는 forward path에서, DMI는 backward path에서 주소를 변환하므로 **두 변환은 서로 역이어야 한다(should)**. |
| h | target의 단일 호출에 대해 interconnect component는 initiator들에게 **여러 번 호출해도 된다(may)**. 같은 target의 direct memory pointer를 여러 initiator가 가질 수 있으므로, **모든 initiator에 대해 호출하는 것이 안전한 구현**. |
| i | interconnect component는 `start_range=0`, `end_range=sc_dt::uint64` 최대값으로 하여 **initiator의 모든 direct memory pointer를 무효화**할 수 있다. |
| j | **모든 TLM-2.0 core interface method 구현은 `invalidate_direct_mem_ptr`를 호출해도 된다(may).** |
| k | `invalidate_direct_mem_ptr` 구현은 직접·간접적으로 **`get_direct_mem_ptr`를 호출하면 안 된다 shall not**. |
| l | `invalidate_direct_mem_ptr` 구현은 직접·간접적으로 **`wait`을 호출하면 안 된다 shall not**. |

### 11.3.7 DMI versus transport

- DMI는 정의상 **interconnect component를 우회**하는 initiator-target 직결 인터페이스. transport interface는 우회할 수 없다.
- **상태를 유지하거나 side effect가 있는 interconnect component**(buffered interconnect, cache memory 모델 등)에 주의. transport는 interconnect 상태를 읽고 갱신하지만 **DMI는 우회**한다. **가장 안전한 대안은 그런 interconnect가 항상 DMI를 거부하는 것.**
- initiator가 transport와 direct memory pointer 사이를 **오갈 수 있고**, 한 initiator는 DMI를, 다른 initiator는 transport를 쓸 수도 있다. 특히 transport 호출이 timing annotation을 동반할 수 있음을 고려해 주의해야 하며, **이는 애플리케이션의 책임**. 예: target이 DMI와 transport를 동시에 지원하거나, transport가 호출될 때마다 모든 DMI 포인터를 무효화할 수 있다.

### 11.3.8 DMI and temporal decoupling

| # | 규칙 |
|---|---|
| a | DMI region은 **오직 target 또는 interconnect가 `invalidate_direct_mem_ptr`를 호출해야만** 무효화될 수 있다. |
| b | **initiator는 DMI 포인터를 사용하기 전에 해당 region이 여전히 유효한지 확인할 책임이 있다** (아래 고려사항 하에). |
| c | SystemC의 co-routine 의미론상, initiator가 실행 중이면 **initiator가 yield 하기 전에는 다른 SystemC process가 실행될 수 없다**. 특히 다른 process가 DMI 포인터를 무효화할 수 없다(현재 process는 할 수도 있다). 결과적으로 temporally decoupled initiator가 **DMI 포인터를 쓸 때마다 반복 확인할 필요는 없다**. |
| d | initiator가 만든 interface method call이 **다른 component로 하여금 `invalidate_direct_mem_ptr`를 호출하게 하여** 그 initiator가 쓰는 DMI region이 무효화될 수 있다. yield 없이 실행되는 temporally decoupled initiator에도 해당될 수 있다. |
| e | initiator가 다른 component와 상호작용하지 않고 yield 하지 않는 동안에는 **모든 유효 DMI region은 계속 유효하다**. |
| f | temporally decoupled initiator가 yield 한 뒤, **다른 temporally decoupled initiator가 현재 time quantum 안에서 그 DMI region을 무효화**할 수 있다. 이는 temporal decoupling 자체의 근본적 부정확성을 반영하는 것이며, **이 절의 규칙 위반은 아니다**. |

### 11.3.9 Optimization using a DMI hint

| # | 규칙 |
|---|---|
| a | **DMI hint = DMI allowed 속성**. DMI 접근 가능 여부를 알기 위해 `get_direct_mem_ptr`를 반복 polling 할 필요를 없애 시뮬레이션 속도를 최적화하는 메커니즘. initiator는 transport interface로 넘긴 일반 트랜잭션의 DMI allowed 속성을 확인하면 된다. |
| b | generic payload가 DMI allowed 속성을 제공한다. 사용자 정의 트랜잭션도 유사 메커니즘을 구현할 수 있으며, 그 경우 **target이 DMI allowed 값을 적절히 설정해야 한다(should)**. |
| c | **DMI allowed 속성 사용은 선택**. initiator는 generic payload의 DMI allowed 속성을 **무시해도 자유**. |
| d | 이를 활용하려는 initiator에게 **권장되는 동작 순서**: <br>1) 주소를 자신의 유효 DMI region 캐시와 대조한다(should). <br>2) 기존 DMI 포인터가 없으면 transport interface로 일반 트랜잭션을 수행한다(should). <br>3) 그 후 트랜잭션의 DMI allowed 속성을 확인한다(should). <br>4) 속성이 DMI 허용을 나타내면 `get_direct_mem_ptr`를 호출한다(should). <br>5) 반환값에 따라 유효 DMI region 캐시를 수정한다(should). |

---

## 11.4 Debug transport interface

### 11.4.1 Introduction

- debug transport interface는 transport interface와 **동일한 forward path(initiator → target)로** target의 저장소를 읽고 쓰되, **일반 트랜잭션에 수반되는 delay·wait·event notification·side effect 없이** 수행하는 수단. 즉 **non-intrusive**.
- 동일 경로를 따르므로 구현이 **일반 트랜잭션과 같은 주소 변환**을 수행할 수 있다.
- 용례(informative): ISS에 붙은 소프트웨어 디버거가 시뮬레이션된 CPU 관점에서 메모리를 peek/poke; 진단 목적의 시스템 메모리 스냅샷; elaboration 종료 시점의 메모리 영역 초기화.
- 기본 debug 트랜잭션 타입은 `tlm_generic_payload`이며 **command, address, data length, data pointer 속성만** 사용된다.
- debug 트랜잭션은 transport interface와 **동일한 확장 방식**을 따른다 — ignorable extension은 가능하나 **non-ignorable/mandatory extension은 새 protocol traits class 정의를 요구** (7.2.3).

### 11.4.2 Class definition

```cpp
namespace tlm {

template <typename TRANS = tlm_generic_payload>
class tlm_transport_dbg_if : public virtual sc_core::sc_interface
{
public:
   virtual unsigned int transport_dbg(TRANS& trans) = 0;
};

} // namespace tlm
```

> `transport_dbg`에는 **`sc_time` 인자도 phase 인자도 없다.**

### 11.4.3 TRANS template argument and tlm_generic_payload class

| # | 규칙 |
|---|---|
| a | `tlm_transport_dbg_if` 템플릿은 **debug 트랜잭션 클래스 타입으로 파라미터화되어야 shall**. |
| b | debug 트랜잭션 클래스는 target에게 debug 접근의 **command, address, data length, data pointer**를 나타내는 속성을 담아야 shall. base protocol에서는 generic payload의 대응 속성이어야 shall. |
| c | `TRANS` 템플릿 인자의 **기본값은 `tlm_generic_payload`여야 shall**. |
| d | 최대 interoperability를 위해 debug 트랜잭션 클래스는 `tlm_generic_payload`여야 한다(should). non-ignorable extension이나 다른 트랜잭션 타입은 interoperability를 제한한다. |
| e | 속성을 더 추가해야 하면 무관한 트랜잭션 클래스 대체보다 **generic payload extension 추가가 권장**. 그런 extension이 non-ignorable/mandatory이면 **새 protocol traits class 정의가 필요**. |

### 11.4.4 Rules

| # | 규칙 |
|---|---|
| a | `transport_dbg` 호출은 **일반 트랜잭션의 transport interface와 동일한 forward path를 따라야 shall**. |
| b | `trans` 인자는 debug 트랜잭션 객체의 참조를 넘겨야 shall. |
| c | **initiator가 debug 트랜잭션 객체를 생성·관리**하고, 넘기기 전에 적절한 속성을 설정할 **책임을 져야 shall**. |
| d | **command 속성은 initiator가 설정해야 shall**하며 **interconnect나 target이 수정하면 안 된다 shall not**. base protocol에서 허용값: `TLM_READ_COMMAND`(target read), `TLM_WRITE_COMMAND`(target write), `TLM_IGNORE_COMMAND`. |
| e | command가 `TLM_IGNORE_COMMAND`인 트랜잭션을 받은 target은 **read/write를 실행하지 않아야 하지만(should not)**, extension을 포함한 generic payload의 **어떤 속성 값이든 사용해 확장 debug 트랜잭션을 실행해도 된다(may)**. |
| f | transport interface와 마찬가지로, debug transport interface에서 **non-ignorable/mandatory generic payload extension을 쓰면 새 protocol traits class 정의가 필요**. |
| g | **address 속성은 initiator가** 읽거나 쓸 영역의 첫 주소로 설정해야 shall. |
| h | interconnect component는 **같은 소켓의 대응 transport interface에서와 정확히 동일하게** address 속성을 decode·필요 시 수정해야 한다(should) (예: target 주소 폭과 memory map 위치에 따른 masking). |
| i | interconnect component는 **addressing error를 감지하면 호출을 넘기지 않아도 된다**. |
| j | debug payload가 여러 interconnect를 지나면 **address 속성이 여러 번 수정될 수 있다**. initiator에 반환될 때 **address 속성의 원래 값은 덮어써져 있을 수 있다**. |
| k | **data length 속성은 initiator가** 읽거나 쓸 바이트 수로 설정해야 shall하며 **interconnect나 target이 수정하면 안 된다 shall not**. data length는 **0이어도 되고**, 그 경우 target은 **어떤 바이트도 읽거나 쓰면 안 되며 shall not**, data pointer는 null이어도 된다(may). |
| l | **data pointer 속성은 initiator가** (write면) target으로 복사될 값이 담긴 배열, (read면) target에서 값이 복사될 배열의 주소로 설정해야 shall하며 **interconnect나 target이 수정하면 안 된다 shall not**. 이 배열은 **initiator가 할당해야 shall**하며 **`transport_dbg` 복귀 전에 삭제하면 안 된다 shall not**. 배열 크기는 **data length 이상이어야 shall**. data length가 0이면 data pointer는 null이어도 되고(may) 배열을 할당하지 않아도 된다. |
| m | target의 `transport_dbg` 구현은 가능하다면 (interconnect를 통한 주소 변환 후) **주어진 주소에서 주어진 바이트 수를 읽거나 써야 shall**. **write command의 경우 target은 data array의 내용을 수정하면 안 된다 shall not**. |
| n | data array는 transport interface에서 쓰이는 **generic payload의 data array와 동일한 조직을 가져야 shall**. `transport_dbg` 구현은 **target 내부 저장소 조직과 generic payload 조직 사이의 변환 책임을 져야 shall**. |
| o | base protocol에서 generic payload option 속성이 `TLM_MIN_PAYLOAD`이면, initiator는 **command, address, data length, data pointer 외의 속성을 설정할 의무가 없고**, target·interconnect는 다른 모든 속성(특히 response status)을 무시해도 된다(may). |
| p | base protocol에서 initiator가 option 속성을 `TLM_FULL_PAYLOAD`로 설정하면, initiator는 **byte enable pointer, byte enable length, streaming width 값을 설정해야 shall**하고 **DMI allowed와 response status를 14.8에 기술된 기본값으로 설정해야 shall**. |
| q | base protocol에서 target이 option 속성을 `TLM_FULL_PAYLOAD_ACCEPTED`로 설정하면, target은 **byte enable pointer, byte enable length, streaming width 값에 따라 동작해야 shall**하고 **DMI allowed와 response status를 14.8에 기술된 대로 설정해야 shall**. |
| r | initiator는 트랜잭션 객체를 호출 간, 그리고 debug transport / transport / DMI 호출에 걸쳐 **재사용해도 된다(may)**. |
| s | **`transport_dbg`는 실제로 읽거나 쓴 바이트 수를 반환해야 shall** — data length보다 **작을 수 있다**. base protocol에서 initiator가 option 속성을 `TLM_FULL_PAYLOAD`로 설정하면 이 count는 **enabled·disabled 바이트를 모두 포함해야 shall**. **target이 연산을 수행할 수 없으면 `0`을 반환해야 shall.** `TLM_IGNORE_COMMAND`인 경우 **target은 반환값을 자유롭게 선택**할 수 있다. |
| t | `transport_dbg`는 직접·간접적으로 **`wait`을 호출하면 안 된다 shall not**. 또한 **debug write command 실행의 즉각적 효과 외에는 interconnect나 target의 어떤 상태 변화도 일으키지 않아야 한다(should not)**. |

---

## 흔한 위반 (pitfalls)

- **method process에서 `b_transport` 호출** — LRM §11.2.2.4 b: `b_transport`는 method process에서 호출되면 안 된다 shall not (`wait` 호출 가능하므로). thread process(`SC_THREAD`)에서 호출할 것.
- **`nb_transport_fw`/`nb_transport_bw` 구현 안에서 `wait` 호출** — LRM §11.2.3.4 d: *nb_transport*는 직접·간접으로 `wait`을 호출하면 안 된다 shall not. 지연이 필요하면 payload event queue나 timed event notification을 쓸 것 (§11.2.4.2 j).
- **`nb_transport_fw` 안에서 곧바로 `nb_transport_bw` 호출** — LRM §11.2.3.4 c: forward path 호출이 직접·간접으로 backward path를 호출하는 것은 어떤 경우에도 금지 shall not. 응답은 별도 process/PEQ를 거쳐 보낼 것.
- **`b_transport` 구현에서 `nb_transport_bw` 호출** — LRM §11.2.2.4 j: shall not. (`nb_transport_fw` 호출 허용 여부는 프로토콜에 달렸다 — §11.2.2.4 i.)
- **`nb_transport` 안에서 `b_transport` 호출** — LRM §11.2.3.4 f: 허용되지 않는다. 별도 thread process로 넘기거나 base protocol이면 `simple_target_socket`을 쓸 것.
- **`TLM_ACCEPTED` 반환 시 트랜잭션/phase/time을 수정** — LRM §11.2.3.7 e: `TLM_ACCEPTED`는 그 셋을 수정하지 않았음을 뜻한다 shall not. 무언가 갱신했다면 `TLM_UPDATED`를 반환할 것.
- **`TLM_COMPLETED` 반환 후 phase 값을 읽음** — LRM §11.2.3.7 g: phase 값은 **undefined**. 또한 그 소켓으로 추가 transport 호출을 하면 안 된다 shall not.
- **time 인자를 감소시킴** — LRM §11.2.4.2 d: *nb_transport*는 감소 금지 shall not. `b_transport`는 `wait`으로 동기화한 경우에 한해, suspend 되어 있던 시간 이하만큼만 감소 가능.
- **음수 또는 절대 시각을 time 인자로 전달** — LRM §11.2.4.2 b: non-negative여야 하고 `sc_time_stamp()` 기준 상대값이어야 shall.
- **`b_transport`에서 `wait` 호출 후 local time offset을 0으로 리셋하지 않음** — LRM §11.2.2.6: `wait` 호출 시 local time offset은 0으로 리셋해야 한다(should).
- **한 트랜잭션 인스턴스에 서로 다른 트랜잭션 객체를 사용** — LRM §11.2.3.5 b: 동일 인스턴스의 모든 *nb_transport* 호출에 **하나의 같은 객체**를 넘겨야 shall.
- **pooled DMI descriptor를 리셋하지 않고 `get_direct_mem_ptr`에 전달** — LRM §11.3.5 d: 넘길 때 descriptor는 기본 상태여야 shall. `init()`을 호출할 것.
- **DMI 전면 거부에 `DMI_ACCESS_NONE` 사용** — LRM §11.3.5 n: read·write 모두 거부하려면 `allow_read_write()`(= `DMI_ACCESS_READ_WRITE`)로 설정해야 하며(should) `DMI_ACCESS_NONE`을 쓰면 안 된다. `DMI_ACCESS_NONE`은 base protocol에서 쓰면 안 된다(should not, §11.3.5 o).
- **interconnect가 전면 disallow 영역을 clip 하지 않음** — LRM §11.3.5 aa: clip 하지 않으면 initiator가 시스템 전체에서 DMI가 금지되었다고 오해한다.
- **DMI latency를 총 latency로 오해** — LRM §11.3.5 ac: latency는 **바이트당 평균**. initiator가 전송 바이트 수를 곱해 실제 latency를 계산해야 shall.
- **side effect가 있는 메모리 영역에 DMI 허가** — LRM §11.3.3 m: 해당 타입의 DMI를 허가하지 않아야 한다(should not). buffered/cache interconnect는 항상 DMI를 거부하는 것이 가장 안전 (§11.3.7 b).
- **target이 DMI 거부했는데 interconnect가 허가로 바꿈** — LRM §11.3.3 j: 허가→거부는 가능하지만 거부→허가는 shall not.
- **DMI 영역 변경 전 `invalidate_direct_mem_ptr` 미호출** — LRM §11.3.6 b: 유효성이나 접근 타입을 바꾸는 어떤 변경보다 먼저 호출할 의무가 있다. §11.3.5 y/z: 무효화 없이 겹치는 allowed/disallowed 영역을 반환하면 안 된다 shall not.
- **`invalidate_direct_mem_ptr` 구현에서 `get_direct_mem_ptr`나 `wait` 호출** — LRM §11.3.6 k, l: 둘 다 shall not.
- **`transport_dbg`가 상태를 바꾸거나 `wait` 호출** — LRM §11.4.4 t: `wait` 호출 금지 shall not, debug write의 즉각 효과 외 상태 변화 금지(should not).
- **`transport_dbg`의 반환값을 data length와 동일하다고 가정** — LRM §11.4.4 s: 더 작을 수 있고, 수행 불가 시 `0`. 반환값을 확인할 것.
- **`transport_dbg` 반환 전에 data array를 해제** — LRM §11.4.4 l: 배열은 initiator가 할당하고 복귀 전에 삭제하면 안 된다 shall not. 크기는 data length 이상이어야 shall.
- **debug write에서 target이 data array를 수정** — LRM §11.4.4 m: shall not.
- **서로 다른 `TRANS`로 특수화된 소켓을 bind 시도** — LRM §11.2.2.3, §11.2.3.3: bind 불가(compile-time 검출). interoperability를 위해 `tlm_generic_payload` + `tlm_phase` + base protocol을 쓸 것(should).
