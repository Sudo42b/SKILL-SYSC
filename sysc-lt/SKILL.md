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

`get_direct_mem_ptr` 구현은 **직접·간접적으로 `wait`를 호출하면 안 된다 shall not** (§11.3.3 o).

### 6. 돌아가는 최소 LT 모델

quantum keeper, `b_transport`, DMI를 한 파일에 담은 것. 전체 라우터·다중 target 예제는 `../examples/lt_demo/`.

```cpp
#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/tlm_quantumkeeper.h"

class Memory : public sc_core::sc_module {
public:
    tlm_utils::simple_target_socket<Memory> SC_NAMED(socket);

    explicit Memory(sc_core::sc_module_name name, sc_core::sc_time latency)
        : sc_core::sc_module(name), latency_(latency), mem_(SIZE, 0) {
        socket.register_b_transport(this, &Memory::b_transport);
        socket.register_get_direct_mem_ptr(this, &Memory::get_direct_mem_ptr);
    }

private:
    static const unsigned SIZE = 256;
    const sc_core::sc_time     latency_;
    std::vector<unsigned char> mem_;

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
        const sc_dt::uint64 addr = trans.get_address();
        const unsigned      len  = trans.get_data_length();
        if (addr + len > SIZE) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
        if (trans.get_command() == tlm::TLM_WRITE_COMMAND)
            std::memcpy(&mem_[addr], trans.get_data_ptr(), len);
        else
            std::memcpy(trans.get_data_ptr(), &mem_[addr], len);

        delay += latency_;                                  // §11.2.4.2 d — 늘리는 것만 허용
        trans.set_dmi_allowed(true);
        trans.set_response_status(tlm::TLM_OK_RESPONSE);    // §15.2.11 c/d — 반환 전에 설정할 의무
    }

    bool get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi& dmi) {
        dmi.allow_read_write();                             // §11.3.3 o — 여기서 wait 금지
        dmi.set_dmi_ptr(mem_.data());
        dmi.set_start_address(0);
        dmi.set_end_address(SIZE - 1);
        dmi.set_read_latency(latency_);
        dmi.set_write_latency(latency_);
        return true;
    }
};

class Cpu : public sc_core::sc_module {
public:
    tlm_utils::simple_initiator_socket<Cpu> SC_NAMED(socket);

    explicit Cpu(sc_core::sc_module_name name) : sc_core::sc_module(name) {
        SC_THREAD(run);
    }

    int transports() const { return transports_; }

private:
    tlm_utils::tlm_quantumkeeper qk_;
    tlm::tlm_dmi dmi_;
    bool         dmi_valid_ = false;
    int          transports_ = 0;

    void run() {
        qk_.set_global_quantum(sc_core::sc_time(200, sc_core::SC_NS));
        qk_.reset();                                        // §16.3.5 a — 생성자가 local quantum을 계산하지 않는다

        for (unsigned i = 0; i < 32; ++i) {
            unsigned char word = static_cast<unsigned char>(i);
            access(tlm::TLM_WRITE_COMMAND, i, word);
        }
        std::cout << "transport calls: " << transports_
                  << "   sc_time_stamp: " << sc_core::sc_time_stamp()
                  << "   effective local: " << qk_.get_current_time() << "\n";
        sc_core::sc_stop();
    }

    void access(tlm::tlm_command cmd, sc_dt::uint64 addr, unsigned char& word) {
        if (dmi_valid_ && addr >= dmi_.get_start_address() && addr <= dmi_.get_end_address()) {
            unsigned char* p = dmi_.get_dmi_ptr() + (addr - dmi_.get_start_address());
            if (cmd == tlm::TLM_WRITE_COMMAND) *p = word; else word = *p;
            qk_.inc(dmi_.get_write_latency());              // DMI 경로도 지연을 부과한다
            if (qk_.need_sync()) qk_.sync();
            return;
        }

        tlm::tlm_generic_payload trans;
        trans.set_command(cmd);
        trans.set_address(addr);
        trans.set_data_ptr(&word);
        trans.set_data_length(1);
        trans.set_streaming_width(1);
        trans.set_byte_enable_ptr(nullptr);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        sc_core::sc_time delay = qk_.get_local_time();      // §16.3.4 i — local offset을 인자로 넘긴다
        socket->b_transport(trans, delay);
        ++transports_;
        if (trans.is_response_error()) SC_REPORT_ERROR("cpu", trans.get_response_string().c_str());

        qk_.set(delay);                                     // 반환된 delay를 반영
        if (qk_.need_sync()) qk_.sync();                    // local offset > local quantum 일 때만

        if (trans.is_dmi_allowed()) {                       // §11.3 — 이제 DMI를 물어봐도 된다
            tlm::tlm_dmi d;
            tlm::tlm_generic_payload probe;
            probe.set_address(addr);
            probe.set_command(cmd);
            dmi_valid_ = socket->get_direct_mem_ptr(probe, d);
            if (dmi_valid_) dmi_ = d;
        }
    }
};

class Top : public sc_core::sc_module {
public:
    explicit Top(sc_core::sc_module_name name) : sc_core::sc_module(name) {
        cpu.socket.bind(mem.socket);
    }
    Cpu    SC_NAMED(cpu);
    Memory SC_NAMED(mem, sc_core::sc_time(20, sc_core::SC_NS));
};

int sc_main(int, char*[]) {
    Top top("top");
    sc_core::sc_start();
    if (sc_core::sc_get_status() != sc_core::SC_STOPPED) sc_core::sc_stop();
    return 0;
}
```

출력:

```
transport calls: 1   sc_time_stamp: 600 ns   effective local: 640 ns
```

읽는 법 — **이 두 줄이 LT의 전부다.**

- **transport 호출은 32번 중 1번뿐이다.** 첫 접근에서 target이 `set_dmi_allowed(true)`를 걸었고, 이후 31번은 DMI 포인터로 직접 갔다. §10.3.3이 말하는 100X 가속이 이것이다.
- **`sc_time_stamp()`는 600 ns인데 effective local time은 640 ns다.** 모델이 한 일은 32 × 20 ns = 640 ns인데 시뮬레이션 시각은 마지막 quantum 경계(200 ns × 3)에 머물러 있다. §16.3.4 h가 말하는 그대로 — `sc_time_stamp`는 **현재 quantum 시작 시점**을 반환한다. 이 40 ns의 간극이 temporal decoupling이고, 이 안에서 `sc_signal`을 읽으면 quantum 시작 시점의 값이 나온다 (§16.3.4 l).

## Untimed(UT)는 어떻게 되는가

§10.3.2가 명시한다: **TLM-2.0은 untimed coding style에 명시적 대비를 하지 않는다.** 현대의 버스 기반 시스템은 임베디드 프로세서 위 소프트웨어를 모델링하려면 어떤 형태로든 시간 개념을 요구하기 때문이다.

- **"정확도가 명시되지 않은 제한적 타이밍 정보를 담은 모델"을 untimed라 부르는 관행이 있는데, TLM-2.0에서 그런 모델은 loosely-timed로 분류된다.** 즉 그 요구는 이 스킬이 맞다.
- **진짜 untimed 모델링은 TLM-1 core interface가 지원한다** → `../references/ch17-tlm1-analysis-ports.md`. `tlm_fifo`의 blocking `put`/`get`이 명시적 동기화 지점을 만든다.
- §10.3.10 Table 52는 untimed를 **hardware functional verification의 verification environment**에만 배정한다.

`sysc-ut` 같은 별도 스킬은 없다. TLM-2.0에 그런 스타일이 정의돼 있지 않아서다 — 지어내지 마라.

## 자주 틀리는 것

- **quantum을 통신 간격보다 크게 잡는다** — §16.3.4 c: initiator 간 전형적 통신 간격보다 **작게** 잡아야 should. 크면 중요한 상호작용이 소실되어 모델이 깨진다.
- **time quantum으로 동기화를 대신한다** — §10.3.4: quantum 메커니즘은 **올바른 시스템 동기화를 보장하기 위한 것이 아니다.** 시뮬레이션 메커니즘일 뿐이고 명시적 동기화 설계를 대체하지 않는다.
- **temporally decoupled 프로세스에서 `sc_signal`을 읽고 최신값을 기대한다** — §16.3.4 l: 읽히는 값은 **현재 quantum 시작 시점의 값**이다. `sc_time_stamp()`는 quantum 안에서 진행하지 않는다 (§16.3.4 h).
- **quantum 안의 프로세스 재개 순서에 의존한다** — §16.3.4 k: **indeterminate**. 보호가 필요하면 명시적 동기화를 써라.
- **`delay`를 줄인다** — §11.2.4.2 d: `b_transport`는 **`wait`을 호출해 시뮬레이션 시각과 동기화한 경우에 한해서만** 줄일 수 있고, 줄이는 양은 suspend되어 있던 시간 이하여야 한다.
- **같은 thread에서 `nb_transport_fw` 트랜잭션이 살아 있는데 `b_transport`를 부른다** — §15.2.10 d: 권장되지 않는다. downstream이 별개 initiator라고 오해한다.
- **`b_transport`와 `nb_transport_fw`를 같은 트랜잭션 인스턴스에 둘 다 호출** — §15.2.10 c: **shall not**.
