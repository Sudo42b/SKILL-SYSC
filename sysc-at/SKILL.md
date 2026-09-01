---
name: sysc-at
description: TLM-2.0 approximately-timed(AT) 모델을 IEEE 1666-2023에 맞게 작성한다. nb_transport_fw/nb_transport_bw, BEGIN_REQ/END_REQ/BEGIN_RESP/END_RESP phase 전이, base protocol 규칙, payload event queue(peq_with_cb_and_phase), request/response exclusion rule, ignorable phase, 트랜잭션 파이프라이닝을 다룰 때 사용한다. "AT 모델", "approximately-timed", "nb_transport", "phase 전이", "base protocol 위반", "PEQ", "architectural exploration", "performance analysis" 같은 요청에 사용한다. 트랜잭션당 timing point 2개로 충분하면 sysc-lt를 쓴다.
---

# TLM-2.0 Approximately-Timed (AT)

**규칙의 출처는 `../references/`다.** 이 스킬은 AT를 쓸지 판단하는 기준과 조립 순서만 담는다. 규범 문구가 필요하면 해당 파일을 읽어라 — 여기 없는 규칙을 지어내지 마라.

| 필요한 것 | 읽을 파일 |
|---|---|
| AT의 정의, LT와의 차이 | `../references/ch10-tlm2-introduction.md` §10.3.5–10.3.6 |
| `nb_transport` 반환값 의미, timing annotation | `../references/ch11-tlm2-core-interfaces.md` §11.2.3–11.2.4 |
| generic payload 수정 허용 시점·메모리 관리 | `../references/ch14-tlm2-generic-payload.md` |
| **phase 전이 표, exclusion rule, 순서 규칙** | `../references/ch15-tlm2-base-protocol.md` §15.2.3–15.2.13 |
| convenience socket, PEQ | `../references/ch16-tlm2-utilities.md` §16.2, §16.4 |

코딩 규칙: `../CODING-RULES.md`.

## AT를 쓸 조건

LRM §10.3.5:

- **적합 use case**: architectural exploration, performance analysis.
- base protocol에서 트랜잭션당 **정확히 4개의 timing point** — request의 시작·끝, response의 시작·끝.
- **temporal decoupling을 일반적으로 쓸 수 없다.** 타이밍 정확도 요구 때문에 각 프로세스가 SystemC 스케줄러와 **lock step**으로 돈다. §16.3.4 j: AT + quantum keeper 조합은 프로세스 간 통신량이 많아 보통 이득이 없다.
- annotate된 delay는 **`wait(delay)` 또는 `notify(delay)`**로 구현된다.

트랜잭션당 timing point 2개로 충분하면 **AT가 아니라 LT다** → `sysc-lt`. 시뮬레이션 도중 둘 사이를 전환하는 것도 허용된다 (§10.3.7) — reset·boot는 LT로 빠르게 통과하고 관심 구간부터 AT로.

## phase 전이 — 외워야 할 것

전체 시퀀스 (§15.2.3 b):

```
BEGIN_REQ -> END_REQ -> BEGIN_RESP -> END_RESP
```

- **`BEGIN_REQ`와 `END_RESP`는 initiator socket을 통해서만**, **`END_REQ`와 `BEGIN_RESP`는 target socket을 통해서만** 보내야 shall (§15.2.3 c).
- **같은 트랜잭션에 대한 연속된 두 `nb_transport` 호출은 phase 인자 값이 서로 달라야 shall** (§15.2.3 e).
- 전이는 **timing annotation 값과 무관하게** 일어난다 (§15.2.4 b). delay는 이후 실행을 지연시킬 뿐이다.
- **불법이거나 순서에 어긋난 phase 전이를 받으면 sender 측의 error다. 수신자 동작은 undefined이며 run-time error가 날 수 있다** (§15.2.3 q).

허용 전이 전체 표는 §15.2.4 Table 57에 있다. **직접 외우지 말고 그 표를 읽어라.**

### 반환값 세 가지 (§11.2.3.7)

| 반환값 | 뜻 | callee가 한 일 |
|---|---|---|
| `TLM_ACCEPTED` | return path 미사용 | 트랜잭션·phase·time 인자를 **수정하지 않았음 shall not**. 나중에 반대 path로 응답한다 |
| `TLM_UPDATED` | return path 사용 | 프로토콜 상태를 진행시켰다. caller는 phase·트랜잭션·time을 확인해야 should |
| `TLM_COMPLETED` | 이 hop에서 완료 | **phase 인자 값은 undefined.** 이 소켓으로 추가 transport 호출 금지 shall not |

### 암묵적 전이 (놓치기 쉬움)

- `BEGIN_REQ` 수신 후 upstream으로 `TLM_COMPLETED` → **암묵적 `END_REQ` + 암묵적 `BEGIN_RESP`** (§15.2.3 h).
- `BEGIN_RESP` 수신 후 downstream으로 `TLM_COMPLETED` → **암묵적 `END_RESP`** (§15.2.3 k).
- `END_REQ` 없이 `BEGIN_RESP`를 받으면 initiator는 **직전에 암묵적 `END_REQ`가 있었다고 가정해야 shall** (§15.2.3 l). **같은 트랜잭션에 한해서만** 성립한다.

## exclusion rule — AT에서 가장 많이 깨지는 규칙

§15.2.12 Table 58:

> **`nb_transport`에서, 같은 소켓에 두 request 또는 두 response가 outstanding — 금지(Forbidden).**

귀결 (§15.2.3 i): `BEGIN_REQ` 수신 후의 `TLM_COMPLETED`는 암묵적 `BEGIN_RESP`를 수반하므로, **그 소켓에 이미 진행 중인 response가 있으면 금지된다.** 이 경우 `TLM_COMPLETED` 대신 **`TLM_ACCEPTED`를 반환하고 `END_RESP`를 기다려야 should.**

순서 규칙 요약 (§15.2.9, §15.2.12):

- request routing은 **결정적이어야 shall** 하고 **address·command attribute에만 의존해야 shall**. 진행 중인 트랜잭션이 있는 동안 **address map을 수정하면 안 된다 shall not**.
- 같은 소켓으로 들어온 **주소가 겹치는 여러 동시 request**는 **받은 순서대로** 실행·forward해야 shall.
- **response는 재정렬되어도 된다.** request 순서대로 돌아온다는 보장은 없다.
- 주소가 겹치지 않는 request들의 실행 순서는 **보장되지 않는다.**

## 조립 순서

### 1. PEQ로 delay를 소비한다

annotate된 delay를 직접 `wait`하지 말고 payload event queue에 넣는다 (§11.2.4.2 j, §16.4). `tlm_utils::peq_with_cb_and_phase`는 트랜잭션과 phase를 함께 큐잉하고 시각이 되면 콜백한다.

### 2. 돌아가는 최소 AT 모델

initiator와 target이 4-phase를 모두 거치고, 각 hop에서 `TLM_ACCEPTED`를 반환해 backward path를 쓴다.

```cpp
#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

class Target : public sc_core::sc_module {
public:
    tlm_utils::simple_target_socket<Target> SC_NAMED(socket);

    explicit Target(sc_core::sc_module_name name)
        : sc_core::sc_module(name), peq_(this, &Target::on_peq) {
        socket.register_nb_transport_fw(this, &Target::nb_transport_fw);
    }

private:
    tlm_utils::peq_with_cb_and_phase<Target> peq_;
    tlm::tlm_generic_payload* in_flight_ = nullptr;   // 이 소켓의 outstanding response (§15.2.12)

    tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                       tlm::tlm_phase& phase,
                                       sc_core::sc_time& delay) {
        if (phase == tlm::BEGIN_REQ) {
            trans.acquire();                          // §15.2.3 p — nb_transport 트랜잭션은 memory manager 필수
            peq_.notify(trans, phase, delay);         // delay를 직접 wait 하지 않는다
            return tlm::TLM_ACCEPTED;                 // backward path로 나중에 응답한다
        }
        if (phase == tlm::END_RESP) {                 // response 종료 — 이 hop 완료
            peq_.notify(trans, phase, delay);
            return tlm::TLM_COMPLETED;                // §15.2.3 o — ACCEPTED도 허용된다
        }
        return tlm::TLM_ACCEPTED;                     // ignorable phase는 ACCEPTED shall (§15.2.5 b)
    }

    void on_peq(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase) {
        sc_core::sc_time zero = sc_core::SC_ZERO_TIME;

        if (phase == tlm::BEGIN_REQ) {
            tlm::tlm_phase end_req = tlm::END_REQ;    // target socket으로만 보낼 수 있다 (§15.2.3 c)
            socket->nb_transport_bw(trans, end_req, zero);

            trans.set_response_status(tlm::TLM_OK_RESPONSE);  // §15.2.11 c/d — BEGIN_RESP 전에 설정할 의무
            if (in_flight_ == nullptr) {              // response exclusion rule (§15.2.12)
                in_flight_ = &trans;
                tlm::tlm_phase begin_resp = tlm::BEGIN_RESP;
                socket->nb_transport_bw(trans, begin_resp, zero);
            }
        } else if (phase == tlm::END_RESP) {
            in_flight_ = nullptr;
            trans.release();
        }
    }
};

class Initiator : public sc_core::sc_module {
public:
    tlm_utils::simple_initiator_socket<Initiator> SC_NAMED(socket);

    explicit Initiator(sc_core::sc_module_name name)
        : sc_core::sc_module(name), mm_(), peq_(this, &Initiator::on_peq) {
        socket.register_nb_transport_bw(this, &Initiator::nb_transport_bw);
        SC_THREAD(run);
    }

    int responses() const { return responses_; }

private:
    // §15.2.3 p — nb_transport에 넘기는 트랜잭션은 memory manager를 가져야 한다
    class Mm : public tlm::tlm_mm_interface {
    public:
        void free(tlm::tlm_generic_payload* t) override { delete t; }
    };

    Mm mm_;
    tlm_utils::peq_with_cb_and_phase<Initiator> peq_;
    int responses_ = 0;

    void run() {
        for (int i = 0; i < 4; ++i) {
            auto* trans = new tlm::tlm_generic_payload(&mm_);
            trans->acquire();
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            trans->set_address(static_cast<sc_dt::uint64>(i) * 4);
            trans->set_data_length(4);
            trans->set_streaming_width(4);
            trans->set_data_ptr(reinterpret_cast<unsigned char*>(&payload_));
            trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
            socket->nb_transport_fw(*trans, phase, delay);

            wait(end_resp_);      // 같은 소켓에 두 request가 outstanding 되지 않도록 (§15.2.12)
        }
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,
                                       tlm::tlm_phase& phase,
                                       sc_core::sc_time& delay) {
        peq_.notify(trans, phase, delay);
        return tlm::TLM_ACCEPTED;
    }

    void on_peq(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase) {
        if (phase == tlm::BEGIN_RESP) {       // END_REQ를 못 봤다면 암묵적으로 있었던 것 (§15.2.3 l)
            ++responses_;
            sc_core::sc_time zero = sc_core::SC_ZERO_TIME;
            tlm::tlm_phase end_resp = tlm::END_RESP;
            socket->nb_transport_fw(trans, end_resp, zero);
            trans.release();
            end_resp_.notify(sc_core::SC_ZERO_TIME);
        }
    }

    unsigned int  payload_ = 0xa5a5a5a5;
    sc_core::sc_event SC_NAMED(end_resp_);
};

class Top : public sc_core::sc_module {
public:
    explicit Top(sc_core::sc_module_name name) : sc_core::sc_module(name) {
        init.socket.bind(targ.socket);
    }
    Initiator SC_NAMED(init);
    Target    SC_NAMED(targ);
};

int sc_main(int, char*[]) {
    Top top("top");
    sc_core::sc_start();
    sc_core::sc_stop();
    std::cout << "responses: " << top.init.responses() << "\n";
    return 0;
}
```

## 자주 틀리는 것

- **`TLM_ACCEPTED`를 반환하면서 트랜잭션·phase·time을 수정** — §11.2.3.7 e: `TLM_ACCEPTED`는 셋 다 수정하지 않았음을 뜻한다 **shall not**. 뭔가 갱신했으면 `TLM_UPDATED`를 반환하라.
- **`TLM_COMPLETED` 반환 후 phase 값을 읽음** — §11.2.3.7 g: **undefined**. 그 소켓으로 추가 transport 호출도 금지 shall not.
- **같은 소켓에 request 두 개를 동시에 outstanding** — §15.2.12: **금지**. 다음 `BEGIN_REQ` 전에 `END_REQ`를 받아야 한다.
- **`END_REQ`를 initiator socket으로 보냄** — §15.2.3 c: `END_REQ`/`BEGIN_RESP`는 **target socket 전용**.
- **연속된 두 호출에 같은 phase 값** — §15.2.3 e: 서로 달라야 shall.
- **memory manager 없이 `nb_transport`에 트랜잭션을 넘김** — §15.2.3 p: **obliged**. `acquire`한 컴포넌트는 완료 시점 또는 그 전에 `release`해야 should.
- **ignorable phase를 이해하지 못한 채 전파** — §15.2.5 g: 의미를 완전히 이해할 때만 넘겨도 된다 **shall not** 전파. 무시할 거면 `TLM_ACCEPTED`를 반환하라 shall.
- **`BEGIN_REQ` 이전이나 `END_RESP` 이후에 ignorable phase** — §15.2.5 e: **shall not**, base protocol 위반이며 error.
- **write에 `TLM_OK_RESPONSE`를 target 확인 없이 설정** — §15.2.11 c: target이 성공적으로 완료했음을 뜻해야 shall. interconnect가 대신 알릴 수 없다.
- **response가 request 순서대로 온다고 가정** — §15.2.9 g: 보장 없다.
