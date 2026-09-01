---
name: sysc-lt
description: TLM-2.0 loosely-timed(LT) 모델을 IEEE 1666-2023에 맞게 작성한다. b_transport, temporal decoupling, tlm_quantumkeeper, global quantum, DMI, transport_dbg, simple_initiator_socket/simple_target_socket을 쓰는 코드를 쓰거나 고칠 때 사용한다. "LT 모델", "loosely-timed", "virtual platform", "빠른 시뮬레이션", "temporal decoupling", "quantum keeper", "DMI로 가속", "소프트웨어 부팅용 모델" 같은 요청에 사용한다. 다중 timing point나 phase 전이가 필요하면 sysc-at을 쓴다.
---

# TLM-2.0 Loosely-Timed (LT)

**규칙의 출처는 `../references/`다.** 이 스킬은 LT를 쓸지 판단하는 기준과 조립 순서만 담는다. 규범 문구가 필요하면 해당 파일을 읽어라 — 여기 없는 규칙을 지어내지 마라.

| 필요한 것 | 읽을 파일 |
|---|---|
| LT의 정의, temporal decoupling, global quantum | `../references/ch10-tlm2-introduction.md` §10.3.3–10.3.4 |
| `b_transport` / DMI / `transport_dbg` 규칙 | `../references/ch11-tlm2-core-interfaces.md` |
| `tlm_global_quantum` | `../references/ch12-tlm2-global-quantum.md` |
| generic payload 속성·메모리 관리 | `../references/ch14-tlm2-generic-payload.md` |
| `b_transport` 관련 base protocol 규칙 | `../references/ch15-tlm2-base-protocol.md` §15.2.8 |
| `tlm_quantumkeeper`, convenience socket | `../references/ch16-tlm2-utilities.md` §16.2, §16.3 |

코딩 규칙: `../CODING-RULES.md` — `SC_NAMED`로 선언하는 소켓은 `socket`, 순수 C++ 멤버인 quantum keeper는 `qk_`. 돌아가는 전체 예제: `../examples/lt_demo/`.

## LT를 쓸 조건

LRM §10.3.3이 정하는 것:

- **트랜잭션당 timing point가 2개면 충분한가** — blocking transport는 호출과 반환, 딱 둘만 준다. 첫째는 request의 시작, 둘째는 response의 시작. 두 시점은 같은 시각일 수도 다른 시각일 수도 있다.
- **적합 use case**: MPSoC virtual platform에서의 **소프트웨어 개발**. timer와 interrupt를 모델링할 수 있어 OS를 부팅하고 타깃 코드를 실행하기에 충분하다.

다중 timing point, phase 전이, request/response 파이프라인 관찰이 필요하면 **LT가 아니라 AT다** → `sysc-at`.

## 조립 순서

### 1. 소켓

`tlm_utils::simple_initiator_socket` / `simple_target_socket`을 쓴다 (§16.2.2). target이 `b_transport` 콜백만 등록해도 소켓이 `nb_transport_fw`를 어댑터로 변환해준다 — **모든 target은 두 인터페이스를 다 지원할 의무가 있다** (§15.2.10 b).

### 2. Quantum keeper

`tlm_utils::tlm_quantumkeeper` (`tlm_utils/tlm_quantumkeeper.h`). **생성 직후 `reset()`을 호출해야 should** — 생성자는 `compute_local_quantum`을 부르지 않는다 (§16.3.5 a).

```cpp
qk_.set_global_quantum(sc_core::sc_time(1, sc_core::SC_US));
qk_.reset();                                  // §16.3.5 a — 생성자가 local quantum을 계산하지 않는다
```

**`sc_set_time_resolution`은 첫 quantum keeper 객체 생성 전에만 호출할 수 있다** (§16.3.5 b).

### 3. initiator 루프

```cpp
void run() {
    for (;;) {
        sc_core::sc_time delay = qk_.get_local_time();   // §16.3.4 i — local offset을 인자로 넘긴다
        socket->b_transport(trans, delay);              // callee가 delay를 늘릴 수 있다
        qk_.set(delay);                                  // 반환된 delay를 반영
        if (qk_.need_sync()) qk_.sync();                 // local offset > local quantum 일 때만
    }
}
```

- `need_sync()`는 **local time offset이 local quantum보다 클 때에만** true (§16.3.5).
- `sync()`는 `wait(local_time_offset)` 후 `reset()`을 한다.
- quantum 끝을 기다리지 않고 앞당겨 양보하는 것이 **synchronization-on-demand** (§10.3.4).

### 4. target

```cpp
void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    // ... 접근 수행 ...
    delay += latency_;                                   // §11.2.4.2 d — 늘리는 것만 허용
    trans.set_response_status(tlm::TLM_OK_RESPONSE);     // §15.2.11 c/d — 반환 전에 설정할 의무
}
```

### 5. DMI (선택)

DMI는 LT의 주 가속 수단이다. temporal decoupling만으로 약 10X, **DMI와 결합하면 100X**까지 가능하다 (§10.3.3).

`get_direct_mem_ptr` 구현은 **직접·간접적으로 `wait`를 호출하면 안 된다 shall not** (§11.3).

## 자주 틀리는 것

- **quantum을 통신 간격보다 크게 잡는다** — §16.3.4 c: initiator 간 전형적 통신 간격보다 **작게** 잡아야 should. 크면 중요한 상호작용이 소실되어 모델이 깨진다.
- **time quantum으로 동기화를 대신한다** — §10.3.4: quantum 메커니즘은 **올바른 시스템 동기화를 보장하기 위한 것이 아니다.** 시뮬레이션 메커니즘일 뿐이고 명시적 동기화 설계를 대체하지 않는다.
- **temporally decoupled 프로세스에서 `sc_signal`을 읽고 최신값을 기대한다** — §16.3.4 l: 읽히는 값은 **현재 quantum 시작 시점의 값**이다. `sc_time_stamp()`는 quantum 안에서 진행하지 않는다 (§16.3.4 h).
- **quantum 안의 프로세스 재개 순서에 의존한다** — §16.3.4 k: **indeterminate**. 보호가 필요하면 명시적 동기화를 써라.
- **`delay`를 줄인다** — §11.2.4.2 d: `b_transport`는 **`wait`을 호출해 시뮬레이션 시각과 동기화한 경우에 한해서만** 줄일 수 있고, 줄이는 양은 suspend되어 있던 시간 이하여야 한다.
- **같은 thread에서 `nb_transport_fw` 트랜잭션이 살아 있는데 `b_transport`를 부른다** — §15.2.10 d: 권장되지 않는다. downstream이 별개 initiator라고 오해한다.
- **`b_transport`와 `nb_transport_fw`를 같은 트랜잭션 인스턴스에 둘 다 호출** — §15.2.10 c: **shall not**.
