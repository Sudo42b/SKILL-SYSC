# Clause 15 — TLM-2.0 base protocol and phases

LRM pp. 530–550.

`tlm_phase` + base protocol. **TLM-2.0 상호운용(interoperability)의 실질적 계약.** 소켓·generic payload·전송 인터페이스를 어떤 순서·타이밍·규칙으로 써야 하는지를 정의한다.

---

## 15.1 Phases

### 15.1.1 Description

`tlm_phase`는 non-blocking transport 인터페이스 클래스 템플릿과 base protocol이 쓰는 **기본 phase 타입**이다. base protocol의 네 phase — `BEGIN_REQ`, `END_REQ`, `BEGIN_RESP`, `END_RESP` — 에 대응하는 enumeration과 **assignment compatible**. enumeration이 아니라 class이므로 overloaded stream operator로 phase 값을 ASCII 텍스트로 출력할 수 있다.

`tlm_phase_enum`의 네 phase는 매크로 `TLM_DECLARE_EXTENDED_PHASE`로 확장 가능. 이 매크로는 `tlm_phase`에서 파생된 singleton 클래스를 만들고, 해당 객체를 반환하는 멤버 함수 `get_phase`를 제공한다.

- **최대 상호운용성을 위해 애플리케이션은 `tlm_phase_enum`의 네 phase만 쓰는 것이 권장(should)**.
- 특정 프로토콜의 세부를 모델링하려고 phase가 더 필요하면 `TLM_DECLARE_EXTENDED_PHASE`를 쓰는 것이 의도(should) — `tlm_phase`와의 assignment 호환성이 유지되므로.
- generic payload extension과 마찬가지로 **ignorable / non-ignorable(mandatory)** 구분이 phase에도 적용된다. base protocol은 **ignorable phase만 허용**.
  - ignorable phase = 수신자가 그 phase transition을 못 받은 것처럼 행동해도 되고, 따라서 송신자는 수신자의 응답 없이도 진행할 수 있어야 한다.
  - 이 의미에서 ignorable하지 않은 phase가 필요하면 **새 protocol traits class를 정의해야 should** (§14.2.3).

### 15.1.2 Class definition

```cpp
namespace tlm {

enum tlm_phase_enum {
   UNINITIALIZED_PHASE=0, BEGIN_REQ=1, END_REQ, BEGIN_RESP, END_RESP };

class tlm_phase{
public:
    tlm_phase();
    tlm_phase( const tlm_phase_enum& );
    tlm_phase& operator= ( const tlm_phase_enum& );
    operator unsigned int() const;
};

inline std::ostream& operator<< ( std::ostream& , const tlm_phase& );

#define TLM_DECLARE_EXTENDED_PHASE(name_arg) \
class tlm_phase_##name_arg : public tlm::tlm_phase{ \
public:\
     static const tlm_phase_##name_arg& get_phase();\
     implementation-defined \
}; \
static const tlm_phase_##name_arg& name_arg=tlm_phase_##name_arg::get_phase()

} // namespace tlm
```

### 15.1.3 Rules

| 항목 | 규칙 |
|---|---|
| default 생성자 | phase 값을 **0**으로 설정해야 shall — enumeration literal `UNINITIALIZED_PHASE`에 대응 |
| `tlm_phase(unsigned int)`, `operator=`, `operator unsigned int` | 대응하는 `unsigned int` 또는 enum으로 phase 값을 get/set 해야 shall |
| `operator<<` | phase 이름에 해당하는 문자열을 출력 스트림에 써야 shall. 예: `"BEGIN_REQ"` |
| `TLM_DECLARE_EXTENDED_PHASE(name_arg)` | `tlm_phase`에서 파생된 `tlm_phase_name_arg`라는 **새 singleton 클래스**를 만들어야 shall. 그렇게 만들어진 static 객체에 대한 참조를 반환하는 public 멤버 함수 `get_phase`를 가져야 shall. 매크로 인자는 `operator<<`가 출력하는 문자열로 쓰여야 shall |
| 매크로 호출 | **세미콜론으로 종료해야 shall** |
| 의도 | `static const name_arg`가 가리키는 객체가 확장 phase를 나타내며, `nb_transport`의 phase 인자로 전달해도 된다 may |

### Example (informative)

```cpp
TLM_DECLARE_EXTENDED_PHASE(ignore_me);      // 확장 phase 두 개 선언
TLM_DECLARE_EXTENDED_PHASE(internal_ph);    // target 내부에서만 사용

struct Initiator : sc_core::sc_module {
    ...
    void thread() {
         ...
         phase = tlm::BEGIN_REQ;
         delay = sc_core::sc_time(10.0, sc_core::SC_NS);
         socket->nb_transport_fw(trans, phase, delay);   // target에 BEGIN_REQ 전송
         phase = ignore_me;                              // phase 변수를 확장 phase로 설정
         delay = sc_core::sc_time(12.0, sc_core::SC_NS);
         socket->nb_transport_fw(trans, phase, delay);   // 2ns 뒤 확장 phase 전송
         ...
    }
};

struct Target : sc_core::sc_module {
    ...
     SC_CTOR(Target)
     : m_peq("m_peq", this, &Target::peq_cb) {}          // PEQ에 콜백 등록

   virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase,
sc_core::sc_time &delay) {
        std::cout << "Phase = " << phase << std::endl;  // overloaded operator<< 로 phase 출력
        m_peq.notify(trans, phase, delay);              // 내부 큐로 트랜잭션 이동
        return tlm::TLM_ACCEPTED;
   }

      void peq_cb(tlm::tlm_generic_payload &trans, const tlm::tlm_phase &phase) {   // PEQ 콜백
          sc_core::sc_time delay;
          tlm::tlm_phase phase_out;

          if (phase == tlm::BEGIN_REQ) {                 // initiator로부터 BEGIN_REQ 수신
              phase_out = tlm::END_REQ;
              delay = sc_core::sc_time(10.0, sc_core::SC_NS);
              socket->nb_transport_bw(trans, phase_out, delay);   // initiator에 END_REQ 반송
              phase_out = internal_ph;                   // 내부 이벤트 신호용 확장 phase
              delay = sc_core::sc_time(15.0, sc_core::SC_NS);
              m_peq.notify(trans, phase_out, delay);     // 내부 이벤트를 PEQ에 삽입
          }
          else if (phase == internal_ph) {                          // 내부 이벤트 수신
              phase_out = tlm::BEGIN_RESP;
              delay = sc_core::sc_time(10.0, sc_core::SC_NS);
              socket->nb_transport_bw(trans, phase_out, delay);     // initiator에 BEGIN_RESP 반송
          }
      }                                                  // initiator의 ignore_me phase는 무시

      tlm_utils::peq_with_cb_and_phase<Target, tlm::tlm_base_protocol_types> m_peq;
};
```

---

## 15.2 Base protocol

### 15.2.1 Description

base protocol은 memory-mapped bus에 인터페이스하는 컴포넌트의 TL 모델 사이 **최대 상호운용성**을 보장하기 위한 규칙 집합. 다음 interoperability layer 클래스의 사용을 **요구(requires)** 한다:

- a) TLM-2.0 core transport, direct memory, debug transport 인터페이스 (Clause 11)
- b) 소켓 클래스 `tlm_initiator_socket`, `tlm_target_socket` (또는 그 파생 클래스) (§13.2)
- c) generic payload 클래스 `tlm_generic_payload` (Clause 14)
- d) phase 클래스 `tlm_phase`

핵심 제약:

- base protocol 규칙은 generic payload와 phase에 대한 확장을 **ignorable한 경우에만 허용**. non-ignorable extension은 **새 protocol traits class 정의를 요구** (§14.2.2).
- base protocol은 미리 정의된 클래스 `tlm_base_protocol_types`로 표현된다. 이 클래스는 **typedef 두 개 외에 아무것도 담지 않는다.** 이 클래스를 소켓의 템플릿 인자로 쓰는 모든 컴포넌트는 **관례상(by convention) base protocol 규칙을 지킬 의무가 있다**.
- 새 protocol traits class를 정의해야 하는 경우, 그 클래스에 연관된 규칙이 base protocol의 규칙을 **override**한다. 다만 일관성과 상호운용성을 위해 새 traits class의 규칙·코딩 스타일은 가능한 한 base protocol을 따르는 것이 **권장(recommended)** (§14.2.3).
- §15.2는 base protocol에 관한 것이지만 다른 프로토콜 모델링의 가이드로 써도 된다 may. 다른 protocol traits class는 phase를 추가하거나 자체 timing annotation·트랜잭션 순서 규칙을 채택해도 되며 may, 그 경우 base protocol과 호환되지 않게 될 수 있다.

### 15.2.2 Class definition

```cpp
namespace tlm {

struct tlm_base_protocol_types
{
    typedef   tlm_generic_payload                       tlm_payload_type;
    typedef   tlm_phase                                 tlm_phase_type;
};

} // namespace tlm
```

### 15.2.3 Base protocol phase sequences

- a) base protocol은 blocking transport 인터페이스, non-blocking transport 인터페이스, 또는 **둘 다** 사용을 허용. blocking transport는 phase 정보를 전달하지 않는다. base protocol과 함께 쓸 때 `nb_transport` 호출 순서 제약이 `b_transport` 호출 순서 제약보다 **더 강하다**. 따라서 `nb_transport`는 approximately-timed, `b_transport`는 loosely-timed 코딩 스타일에 가깝다.
- b) 주어진 소켓을 통한 주어진 트랜잭션의 **전체 phase transition 시퀀스**:

  ```
  BEGIN_REQ -> END_REQ -> BEGIN_RESP -> END_RESP
  ```

- c) **`BEGIN_REQ`와 `END_RESP`는 initiator socket을 통해서만**, **`END_REQ`와 `BEGIN_RESP`는 target socket을 통해서만** 전송해야 shall.
- d) blocking transport의 경우 트랜잭션 인스턴스는 `b_transport`의 단일 호출/반환과 연관된다. `b_transport` 호출↔`BEGIN_REQ`, 반환↔`BEGIN_RESP`의 대응은 **순전히 개념적(notional)**이며 `b_transport`에는 연관된 phase가 없다.
- e) base protocol에서 `nb_transport`의 **각 호출**과 `TLM_UPDATED`를 반환하는 **각 반환**은 phase transition을 일으켜야 shall. 즉 **같은 트랜잭션에 대한 연속된 두 `nb_transport` 호출은 phase 인자 값이 서로 달라야 shall**. ignorable phase extension은 허용되며, 그 경우 확장 phase의 삽입은 (무시되더라도) 이 규칙 상 phase transition으로 계산해야 shall.
- f) phase 시퀀스는 `nb_transport`가 `TLM_COMPLETED`를 반환하여 **조기 종료**될 수 있으나, 오직 다음 방식으로만:
  - interconnect 또는 target이 **forward path에서 `BEGIN_REQ` 또는 `END_RESP`를 받았을 때** `TLM_COMPLETED` 반환 가능.
  - interconnect 또는 initiator가 **backward path에서 `BEGIN_RESP`를 받았을 때** `TLM_COMPLETED` 반환 가능.
  - `TLM_COMPLETED`는 **해당 hop에 대한 트랜잭션 종료**를 뜻하며, 이 경우 caller는 phase 인자를 무시해야 should (§11.2.3.7).
  - `TLM_COMPLETED`는 **성공적 완료를 의미하지 않는다** — initiator는 response status를 확인해야 should.
- g) `END_RESP` phase로의 전이 역시 해당 hop에 대한 트랜잭션 종료를 나타내며, 이 경우 callee는 `TLM_COMPLETED`를 반환할 의무가 없다.
- h) `BEGIN_REQ`를 받은 뒤 upstream 방향으로 `TLM_COMPLETED`를 반환하면 **암묵적 `END_REQ`와 암묵적 `BEGIN_RESP`**가 함께 따라온다. 따라서 initiator는 generic payload의 response status를 확인해야 should 하며, 다음 트랜잭션의 `BEGIN_REQ`를 즉시 보내도 된다 may.
- i) `BEGIN_REQ` 수신 후의 `TLM_COMPLETED`가 암묵적 `BEGIN_RESP`를 수반하므로, **해당 소켓에 이미 진행 중인 response가 있으면 이 상황은 response exclusion rule에 의해 금지(forbidden)**. 이 경우 callee는 `TLM_COMPLETED` 대신 `TLM_ACCEPTED`를 반환하고 다음 response를 upstream으로 보내기 전에 `END_RESP`를 기다려야 should.
- j) `BEGIN_REQ` 수신 후의 `TLM_COMPLETED`는 트랜잭션 종료를 뜻하므로, interconnect나 initiator가 **같은 소켓을 통해 같은 트랜잭션에 대해 `END_RESP`를 보내는 것은 금지(forbidden)**.
- k) `BEGIN_RESP`를 받은 뒤 downstream 방향으로 `TLM_COMPLETED`를 반환하면 **암묵적 `END_RESP`**를 수반한다.
- l) 같은 트랜잭션에 대해 `END_REQ`를 먼저 받지 않은 채 downstream으로부터 `BEGIN_RESP`를 받으면, initiator는 그 `BEGIN_RESP` 직전에 **암묵적 `END_REQ`가 있었다고 가정해야 shall**. 이는 **같은 트랜잭션에 한해서만** 성립 — `BEGIN_RESP`가 다른 트랜잭션의 `END_REQ`를 함의하지 않으며, `BEGIN_REQ`를 받은 target이 이전 트랜잭션의 `END_RESP`를 추론할 수도 없다.
- m) 위 사항들은 `nb_transport`의 timing annotation 인자 값과 **무관하게** 성립한다.
- n) base protocol 트랜잭션은 (특정 hop 기준) 어느 한쪽 path에서 `TLM_COMPLETED`가 반환되거나, forward path 또는 return path에서 `END_RESP`가 전송되면 **완료**된다.
- o) `END_RESP`가 forward path로 전송된 경우 callee는 `TLM_ACCEPTED` 또는 `TLM_COMPLETED`를 반환해도 된다 may. 어느 쪽이든 트랜잭션은 완료된 것이다.
- p) 주어진 트랜잭션은 hop마다 다른 시각에 완료될 수 있다. `nb_transport`에 전달되는 트랜잭션 객체는 **memory manager를 가져야 하며(obliged)**, 객체의 lifetime은 generic payload의 reference count가 0이 될 때 끝난다. `acquire`를 호출한 컴포넌트는 트랜잭션 완료 시점 또는 그 이전에 `release`를 호출해야 should (§14.5).
- q) **컴포넌트가 불법(illegal)이거나 순서에 어긋난(out-of-order) phase transition을 받으면, 이는 sender 측의 error다. 수신자의 동작은 undefined이며 run-time error가 발생할 수 있다.**

### 15.2.4 Permitted phase transitions

Table 57. `req`, `//req`, `rsp`, `//rsp`는 각각 `BEGIN_REQ`, `END_REQ`, `BEGIN_RESP`, `END_RESP`를 뜻한다.

| Previous state | Calling path | Phase arg on call | Phase arg on return | Status on return | Response valid | End-of-life | Next state |
|---|---|---|---|---|:---:|:---:|---|
| `//rsp` | Forward  | `BEGIN_REQ`  | —            | Accepted  |   |   | `req`  |
| `//rsp` | Forward  | `BEGIN_REQ`  | `END_REQ`    | Updated   |   |   | `//req` |
| `//rsp` | Forward  | `BEGIN_REQ`  | `BEGIN_RESP` | Updated   | X |   | `rsp`  |
| `//rsp` | Forward  | `BEGIN_REQ`  | —            | Completed | X | X | `//rsp` |
| `req`   | Backward | `END_REQ`    | —            | Accepted  |   |   | `//req` |
| `req`   | Backward | `BEGIN_RESP` | —            | Accepted  | X |   | `rsp`  |
| `req`   | Backward | `BEGIN_RESP` | `END_RESP`   | Updated   | X | X | `//rsp` |
| `req`   | Backward | `BEGIN_RESP` | —            | Completed | X | X | `//rsp` |
| `//req` | Backward | `BEGIN_RESP` | —            | Accepted  | X |   | `rsp`  |
| `//req` | Backward | `BEGIN_RESP` | `END_RESP`   | Updated   | X | X | `//rsp` |
| `//req` | Backward | `BEGIN_RESP` | —            | Completed | X | X | `//rsp` |
| `rsp`   | Forward  | `END_RESP`   | —            | Accepted  | X | X | `//rsp` |
| `rsp`   | Forward  | `END_RESP`   | —            | Completed | X | X | `//rsp` |

표 해설:

- b) 이 phase state 전이는 `nb_transport`의 `sc_time` 인자(timing annotation) 값과 **독립적**이다. timing annotation 값과 무관하게 표의 상태 전이가 일어난다. (timing annotation은 이후 실행을 지연시키는 효과만 가질 수 있다.)
- c) *Previous state* — `nb_transport` 호출 **전** 해당 hop의 상태.
- d) *Calling path* — forward path(`nb_transport_fw`)인지 backward path(`nb_transport_bw`)인지.
- e) *Phase argument on call* — 호출 시 phase 인자 값. callee에게 제시되는 phase.
- f) *Phase argument on return* — 반환 시 phase 인자 값. **`TLM_UPDATED`를 반환할 때만 유효**.
- g) *Status on return* — `nb_transport` 반환값: Accepted(`TLM_ACCEPTED`) / Updated(`TLM_UPDATED`) / Completed(`TLM_COMPLETED`).
- h) *Response valid* — 반환 시 트랜잭션의 response status attribute가 유효하면 체크됨.
- i) *End-of-life* — 해당 hop에 대해 트랜잭션 수명이 끝났으면(즉 그 hop에서 그 트랜잭션에 대한 추가 `nb_transport` 호출이 허용되지 않으면) 체크됨.
- j) *Next state* — `nb_transport` 반환 후 hop의 상태.
- k) phase transition은 caller가 일으킬 수도(반환 phase 열이 '—'), callee가 일으킬 수도 있다.
- l) **ignorable phase extension은 `BEGIN_REQ`와 `END_RESP` 사이 어느 지점에나 삽입해도 된다 may.**
- m) response가 valid하다고 해서 성공적 완료를 뜻하지는 않는다. 성공했을 수도 아닐 수도 있다.
- n) Figure 29는 hop 하나에서 base protocol이 허용하는 `nb_transport` 호출 시퀀스를 그래프로 나타낸다. Start에서 End까지의 경로가 단일 트랜잭션 인스턴스의 허용 호출 시퀀스다.

### 15.2.5 Ignorable phases

확장 phase는 **ignorable phase인 경우에 한해** base protocol과 함께 사용 가능.

- a) 일반적으로 base protocol의 네 phase에 확장 phase를 더하는 **권장 방법은 새 protocol traits class 정의**(§14.2.3). ignorable phase는 확장 phase의 특수·제한된 경우이며, 주 목적은 모델의 타이밍 정확도를 높이기 위해 **timing point를 추가**하는 것이다.
- b) `nb_transport` 호출에서 **callee가 phase를 무시하는 경우 `TLM_ACCEPTED`를 반환해야 shall**. callee가 `TLM_UPDATED`를 반환하는 경우, caller는 return path의 phase를 무시해도 되지만 may, 무시한다는 것을 나타내는 특정 조치를 취할 의무는 없다.
- c) `nb_transport` 인터페이스는 caller가 "callee가 phase를 무시했는지"와 "callee가 반대 path로 나중에 응답할 것인지"를 구별할 수단을 **제공하지 않는다**. 두 경우 모두 callee는 `TLM_ACCEPTED`를 반환해야 shall.
- d) ignorable phase의 존재가 네 base phase의 **순서나 의미를 바꾸어서는 안 되고 shall not**, base protocol의 어떤 규칙도 깨뜨려서는 안 된다.
- e) **ignorable phase는 주어진 소켓의 주어진 트랜잭션에서 `BEGIN_REQ` 이전이나 `END_RESP` 이후에 발생해서는 안 된다 shall not. 그런 경우는 base protocol 위반이며 error.**
- f) ignorable phase의 존재가 generic payload attribute의 유효성 규칙이나 수정 규칙을 바꾸어서는 안 된다 shall not. 예: ignorable phase 수신 시 interconnect는 **address attribute, DMI allowed attribute, extension만** 수정 가능 (§14.7).
- g) 아래의 transparent component를 제외하고, **ignorable phase의 수신자가 그 phase를 인식하지 못하면(즉 무시하면) 그 phase를 forward·backward·return 어느 path로도 전파해서는 안 된다 shall not.** 즉 컴포넌트는 **phase의 의미를 완전히 이해할 때만** 그 phase를 `nb_transport` 인자로 넘겨도 된다.
- h) 수신자가 ignorable phase를 인식하는 경우, base protocol을 위반하지 않는 한 그 컴포넌트의 동작은 base protocol의 범위 밖이며 **base protocol에 의해 undefined**. 수신자는 그 phase가 속한 확장 프로토콜의 의미를 따라야 should.
- i) 정의상 ignorable phase를 보내는 컴포넌트는 수신자에게 `nb_transport`의 `TLM_ACCEPTED` 반환이라는 최소 응답 외의 어떤 응답도 **요구·요청할 수 없다**. 응답을 요구하는 phase는 정의상 ignorable하지 않으며, 그 경우 권장 접근은 base protocol의 확장이 아니라 **새 protocol traits class 정의**다. 이렇게 하면 호환되지 않는 프로토콜을 나타내는 소켓끼리의 binding이 방지된다.
- j) 반대로, 들어온 확장 phase를 **인식하는** base-protocol-compliant 컴포넌트는 사전에 합의된 확장 프로토콜 규칙에 따라 반대 path로 다른 확장 phase를 보내 응답해도 된다. base protocol 규칙을 깨지 않는 한 TLM-2.0 표준이 허용한다.
- k) **transparent interconnect component** — 같은 컴포넌트 안의 target socket과 initiator socket 사이에서 TLM-2.0 interface method call을 즉시·직접 통과시키는 컴포넌트. 이 표준에서 transparent component를 인정하는 **유일한 의도는 checker와 monitor**를 허용하기 위함이다 (보통 target socket 1개, initiator socket 1개, 양방향 모든 트랜잭션을 수정 없이 통과).
- l) **transparent component 안에서 TLM-2.0 core interface method 구현은 simulation time을 소비하거나 delay를 삽입하거나 `wait`를 호출해서는 안 되며 shall not, 반대편 소켓을 통해 동일한 interface method call을 즉시 해야 shall.** 모든 인자를 그대로 통과시키고, **트랜잭션 객체·phase·delay를 포함한 어떤 인자 값도 수정해서는 안 된다 shall not** — 단 generic payload extension은 예외. 그런 transparent component를 통한 routing은 **고정되어야 shall** 하며, 트랜잭션 attribute나 phase에 의존해서는 안 된다.
- m) 위 규칙의 귀결로, transparent component는 확장 phase나 ignorable phase를 양방향으로 그대로 통과시킨다.

> **예 (informative)** — initiator가 만드는 ignorable phase의 예: write command에서 데이터 전송의 첫 beat를 표시하는 phase. 이를 인식하는 interconnect/target은 command·address 가용 시점과 데이터 전송 시작 시점을 구별할 수 있다. 무시하는 target은 `BEGIN_REQ`를 command·address·data 가용의 단일 타이밍 기준으로 삼아야 한다.
> target이 만드는 예: split transaction을 표시하는 phase. 인식하는 initiator는 split phase 수신 즉시 다음 request를 보낼 수 있다. 무시하는 initiator는 첫 request의 response를 받을 때까지 기다릴 수 있다.

### 15.2.6 Base protocol timing parameters and flow control

- a) 네 phase로 **request accept delay**(연속 트랜잭션 사이 최소 initiation interval), **latency of the target**, **response accept delay**를 모델링할 수 있다. 이 정도의 타이밍 granularity가 approximately-timed 코딩 스타일에 적합.

  ```
  BEGIN_REQ  ──┐
               │ request accept delay
  END_REQ    ──┘
               │ latency of target
  BEGIN_RESP ──┐
               │ response accept delay
  END_RESP   ──┘
  ```

- b) **write command**에서 `BEGIN_REQ` phase는 데이터가 initiator→interconnect→target으로 전송 가능해지는 시점을 표시. 개념적으로 `BEGIN_REQ`로의 전이는 데이터 전송 첫 beat의 시작에 대응. **전송 시간을 계산하고 다음 전송을 받을 준비가 되면 `END_REQ`를 upstream으로 보내는 것은 downstream 컴포넌트의 책임.** downstream이 마지막 beat 종료까지 `END_REQ`를 지연시키는 것이 자연스럽지만 **의무는 아니다**.
- c) **read command**에서 `BEGIN_RESP` phase는 데이터가 target→interconnect→initiator로 전송 가능해지는 시점을 표시. 전송 시간 계산과 준비 완료 시 `END_RESP` 하향 전송은 **upstream 컴포넌트의 책임**. 마지막 beat까지 `END_RESP`를 지연시키는 것이 자연스럽지만 의무는 아니다.
- d) read command에서 downstream 컴포넌트가 `nb_transport_fw`에서 `TLM_COMPLETED`를 반환해 트랜잭션을 조기 종료하면, (`END_RESP`를 보낼 수 없으므로) 데이터 전송 시간을 다른 방법으로 계산하는 것은 upstream 컴포넌트의 책임이다.
- e) **Request exclusion rule** — base protocol에서 initiator나 interconnect는 **직전 트랜잭션에 대해 downstream으로부터 `END_REQ` 또는 `BEGIN_RESP`를 받거나, downstream이 `nb_transport_fw`에서 `TLM_COMPLETED`를 반환하여 이전 트랜잭션을 완료할 때까지, 같은 소켓을 통해 `BEGIN_REQ` phase로 새 트랜잭션을 보내서는 안 된다 shall not.** multi-socket의 경우 이 규칙은 그 소켓에 bind된 **개별 initiator-target 인터페이스(즉 각 index)마다** 적용된다.
- f) **Response exclusion rule** — base protocol에서 target이나 interconnect는 **직전 트랜잭션에 대해 upstream으로부터 `END_RESP`를 받거나, 어떤 컴포넌트가 그 hop에서 `TLM_COMPLETED`를 반환하여 이전 트랜잭션을 완료할 때까지, 같은 소켓을 통해 `BEGIN_RESP` phase로 새 트랜잭션에 응답해서는 안 된다 shall not.** multi-socket의 경우 backward path에서 개별 initiator-target 인터페이스(각 index)마다 적용된다.
- g) **request/response exclusion rule을 포함한 모든 phase transition 규칙은 오직 method call 순서에만 기반해야 shall 하며, time 인자(timing annotation) 값에 영향받아서는 안 된다 shall not.**
- h) non-blocking transport로 같은 소켓을 통해 보낸 연속 트랜잭션은 **pipeline** 가능. 각 `BEGIN_REQ`(또는 `BEGIN_RESP`)에 `END_REQ`(또는 `END_RESP`)로 즉시 응답하면 interconnect는 임의 개수의 트랜잭션이 동시에 in-flight 상태가 되도록 허용. 즉시 응답하지 않으면 **flow control**을 행사하게 된다.
- i) 같은 소켓에 두 개의 outstanding request 또는 response를 금지하는 이 규칙은 **non-blocking transport 인터페이스에만 적용되어야 shall** 하며, `b_transport` 호출에는 **직접적인 영향이 없어야 shall**. (단 `b_transport` 자신이 `nb_transport_fw`를 호출하는 경우 간접적 영향은 있을 수 있다.)
- j) 주어진 트랜잭션에서 **`BEGIN_REQ`는 항상 initiator에서 시작해야 shall** 하며 0개 이상의 interconnect를 거쳐 target에 도달한다. interconnect는 upstream으로부터 `BEGIN_REQ`를 받기 전에 downstream으로 `BEGIN_REQ`를 보내는 것이 **허용되지 않는다**.
- k) 주어진 트랜잭션에서 **`BEGIN_RESP`는 항상 target에서 시작해야 shall** 하며 0개 이상의 interconnect를 거쳐 initiator에 도달한다. interconnect는 downstream으로부터 `BEGIN_RESP`를 받기 전에 upstream으로 `BEGIN_RESP`를 보내는 것이 허용되지 않는다. `BEGIN_RESP`가 명시적이든 `TLM_COMPLETED`에 의해 암묵적이든 동일.
- l) 반면 interconnect는 downstream으로부터 `END_REQ`를 받기 **전에** upstream으로 `END_REQ`를 보내도 된다 may. 마찬가지로 upstream으로부터 `END_RESP`를 받기 전에 downstream으로 `END_RESP`를 보내도 된다 may. 명시적/암묵적 모두 해당.
- m) `END_REQ`와 `END_RESP`는 **주로 인접 컴포넌트 간 flow control용**. 이 두 phase는 어떤 표준 generic payload attribute의 유효성도 알리지 않는다. 이 둘은 end-to-end로 인과적으로 전파되지 않으므로 initiator-target 간 extension 유효성 신호로는 **신뢰할 수 없으나**, 인접한 두 컴포넌트 사이의 extension 유효성 신호로는 쓸 수 있다.
- n) interconnect가 대응 phase를 받기 전에 확장 phase를 보내도 되는지는 **해당 확장 phase에 연관된 규칙에 달려 있다.**

> **Example (informative)** — timing annotation과 request/response exclusion rule의 상호작용:
>
> ```cpp
> void initiator_1_thread_process() {
>    using namespace sc_core;
>    using namespace tlm;
>
>      // initiator가 +1000ns에 실행될 request 전송
>      phase = BEGIN_REQ;
>      delay = sc_time(1000.0, SC_NS);
>      status = socket->nb_transport_fw(T1, phase, delay);
>      assert(status == TLM_UPDATED && phase == END_REQ && delay == sc_time(1010.0, SC_NS));
>      // END_REQ가 즉시 반환되어 +1010ns에 실행됨
>
>      // 이는 권장 코딩 스타일이 아님
>      // loosely-timed였다면 initiator는 b_transport를 호출했을 것
>      // approximately-timed였다면 downstream이 동기화를 위해 TLM_ACCEPTED를 반환하고
>      // initiator는 END_REQ를 기다려야 했을 것
>
>      // initiator는 즉시 다음 request를 보낼 수 있음, +1050ns에 실행
>      phase = BEGIN_REQ;
>      delay = sc_time(1050.0, SC_NS);
>      status = socket->nb_transport_fw(T2, phase, delay);
>      assert(status == TLM_UPDATED && phase == END_REQ && delay == sc_time(1060.0, SC_NS));
>
>      // 기술적으로는 더 이른 local time +500ns에 다음 request를 보내는 것도 허용되나
>      // timing annotation을 감소시키는 것은 권장 코딩 스타일이 아님
>      phase = BEGIN_REQ;
>      delay = sc_time(500.0, SC_NS);
>      status = socket->nb_transport_fw(T3, phase, delay);
>      assert(status == TLM_UPDATED && phase == END_REQ && delay == sc_time(510.0, SC_NS));
>
>      // initiator가 제어를 양보 — 다른 initiator가 재개되고 simulation time이 진행됨
>      wait(...);
> }
> void initiator_2_thread_process() {
>    using namespace sc_core;
>    using namespace tlm;
>
>      // 아래 호출들이 위 첫 initiator의 트랜잭션 스트림에 이어진다고 가정
>
>      // 두 번째 initiator가 +10ns에 실행될 request 전송
>      // downstream에서 본 timing annotation이 +510ns에서 +10ns로 감소
>      // loosely-timed initiator의 전형적 동작
>      phase = BEGIN_REQ;
>      delay = sc_time(10.0, SC_NS);
>      status = socket->nb_transport_fw(T4, phase, delay);
>      assert(status == TLM_UPDATED && phase == END_REQ && delay == sc_time(30.0, SC_NS));
>
>      // 이전 request와 겹치는 +20ns 실행 request 전송
>      // hop의 현재 phase가 END_REQ이므로 기술적으로 허용되나 권장되지 않음
>      phase = BEGIN_REQ;
>      delay = sc_time(20.0, SC_NS);
>      status = socket->nb_transport_fw(T5, phase, delay);
>      assert(status == TLM_UPDATED && phase == END_REQ && delay == sc_time(40.0, SC_NS));
>
>      // 이전 두 request보다 앞선 +0ns 실행 request 전송
>      // 기술적으로 허용되나 권장되지 않음
>      phase = BEGIN_REQ;
>      delay = sc_time(0.0, SC_NS);
>      status = socket->nb_transport_fw(T6, phase, delay);
>      assert(status == TLM_UPDATED && phase == END_REQ && delay == sc_time(60.0, SC_NS));
>      wait(...);
> }
> ```

### 15.2.7 Base protocol rules concerning timing annotation

- a) 이 규칙들은 §11.2.4와 함께 읽어야 should.
- b) `b_transport`와 `nb_transport` 구현이 time 인자 `t`를 수정하는 방식에는 제약이 있다 — **effective local time `sc_time_stamp() + t`가 함수 호출과 반환 사이에서 non-decreasing이어야 한다** (§11.2.4.2).
- c) 주어진 소켓의 주어진 트랜잭션에 대한 연속된 `nb_transport` 호출과 반환에서, **effective local time의 시퀀스는 non-decreasing이어야 shall**. effective local time = `sc_time_stamp() + t`. 이 목적상 **호출과 반환 모두 하나의 시퀀스의 일부로 간주해야 shall**. forward path와 backward path 모두에 적용. 의도는 주어진 트랜잭션에 대해 **시간이 거꾸로 흐르지 않게** 하는 것.
- d) 위 규칙은 `b_transport`의 호출과 반환 사이에도 적용된다 (§11.2.4.2).
- e) 나아가 주어진 트랜잭션 객체에 대해, request가 initiator→target으로 전파되고 response가 target→initiator로 전파되는 동안 **각 transport method 호출·반환이 주는 effective local time 시퀀스는 non-decreasing이어야 shall**. 이때 request 전파에는 `b_transport` 호출과 `BEGIN_REQ` phase가, response 전파에는 `b_transport` 반환, `BEGIN_RESP` phase, `TLM_COMPLETED`가 포함된다.
- f) effective local time은 timing annotation(time 인자) 값을 늘리거나, SystemC simulation time을 진행시키거나(**`b_transport`만 해당**), 둘 다로 증가시킬 수 있다 may.
- g) **서로 다른 트랜잭션 객체 사이**에는 `b_transport`/`nb_transport` 호출의 effective local time이 non-decreasing이어야 할 의무가 **없다**. 그렇지만 각 initiator process는 일반적으로 non-decreasing effective local time 순서로 호출하는 것이 **권장(recommended)**. 그렇지 않으면 downstream은 out-of-order 트랜잭션이 별개의 initiator에서 온 것으로 추론하여 실행 순서를 자유롭게 선택할 수 있게 된다. 다만 서로 다른 loosely-timed initiator의 스트림이 합류하는 곳에서는 out-of-order effective local time이 발생할 수 있다.
- h) 주어진 소켓에서 initiator는 **같은 트랜잭션 객체를 서로 다른 시각에 blocking/non-blocking transport, DMI, debug transport 인터페이스로 전달해도 된다**. 또한 initiator는 같은 트랜잭션 객체를 다른 트랜잭션 인스턴스로 **재사용해도 된다** — 모두 generic payload의 memory management 규칙(§14.5)을 따르는 한.

### 15.2.8 Base protocol rules concerning b_transport

- a) **`b_transport` 호출은 re-entrant**. `b_transport` 구현은 `wait`를 호출할 수 있고, 그동안 다른 initiator의 다른 트랜잭션 객체에 대해 timing annotation 제약 없이 또 다른 `b_transport` 호출이 이루어질 수 있다.
- b) 같은 initiator 모듈 안에 여러 프로세스가 있는 경우, **각 프로세스는 트랜잭션 순서 규칙상 별개의 initiator로 간주해야 shall**. 구체적으로, 같은 모듈의 서로 다른 thread에서 온 `b_transport` 호출들 사이에는 — 같은 소켓이든 다른 소켓이든 — **순서 제약이 없다**.
- c) interconnect나 target은 동시에 진행되는 여러 `b_transport` 호출이 서로 다른 initiator thread에서, 그리고 동시적인 `nb_transport_fw` 호출과는 다른 프로세스에서 온 것임을 항상 추론할 수 있고, 따라서 그 호출들 사이 상호 순서에 제약이 없음을 안다. **`b_transport`에서는 한 request가 다른 request를 추월(overtake)해도 된다.**
- d) **같은 소켓을 통해 같은 트랜잭션 객체에 대해 `b_transport`를 re-entrant 호출하는 것은 금지(forbidden)**.

> **Example (informative)** — re-entrant `b_transport` 호출:
>
> ```cpp
> // 두 개의 initiator thread process
> void thread1() {
>     socket->b_transport(T1, sc_core::sc_time(100.0, sc_core::SC_NS));
> }
>
> void thread2() {
>    wait(10.0, sc_core::SC_NS);
>    socket->b_transport(T2, sc_core::sc_time(50.0, sc_core::SC_NS)); // T2가 T1을 추월
> }
>
> // target의 b_transport 구현
> void b_transport( TRANS& trans, sc_core::sc_time& t ) {
>     wait(t);
>     execute(trans);                    // T1은 100ns에, T2는 60ns에 실행됨
>     t = sc_core::SC_ZERO_TIME;
> }
> ```

### 15.2.9 Base protocol rules concerning request and response ordering

목적: initiator가 특정 target으로 pipeline된 일련의 request를 보냈을 때, 그 request들이 **보낸 순서대로 target에서 실행**되도록 보장하는 것. generic payload 트랜잭션은 initiator/target의 identity를 저장하지 않으므로, initiator는 들어온 소켓의 identity로만, target은 address/command attribute 값으로만 추론된다. **겹치지 않는 주소로 보낸 request들의 실행 순서는 보장되지 않는다.**

- a) base protocol은 **서로 다른 소켓으로 들어온** request/response가 상호 지연·인터리브되거나 임의 순서로 실행되는 것을 허용. 예: interconnect가 특정 target socket이나 특정 data length의 request에 높은 우선순위를 부여해 추월시키는 것, 또는 서로 다른 initiator socket으로 돌아온 response를 원래 request 수신 순서에 맞게 재정렬하는 것.
- b) **request routing은 결정적(deterministic)이어야 shall** 하며, 트랜잭션 객체의 **address와 command attribute에만 의존해야 shall**. (transport, DMI, debug transport 인터페이스에 공통인 유일한 attribute이므로.) **진행 중인 트랜잭션이 있는 동안 address map을 수정해서는 안 된다 shall not.**
- c) initiator나 interconnect가 forward path로 **주소가 겹치는 여러 동시 request**를 보내면, 그 request들은 **같은 initiator socket으로 라우팅되어야 shall**.
  - *multiple concurrent requests* = 대응하는 response를 target으로부터 아직 받지 못한 request들.
  - *overlapping addresses* = 트랜잭션 객체 data array의 최소 한 바이트가 같은 주소를 공유.
  - 같은 주소에 대한 read/write request라도 **동시가 아니면** 다른 소켓으로 라우팅해도 된다 may.
- d) interconnect(또는 target)가 **같은 target socket**으로 들어온 `nb_transport_fw` 호출을 통해 주소가 겹치는 여러 동시 request를 받으면, **받은 순서와 같은 순서로 forward 전송(또는 실행)해야 shall**. *same order* = 같은 interface method call 순서. (interface method call 순서와 effective local time 순서가 다르면, 수신 컴포넌트는 주소와 무관하게 임의 순서로 실행해도 된다. 또 이 규칙은 request들이 서로 다른 initiator에서 왔더라도 성립한다.)
- e) 위 규칙은 들어오는 `b_transport` 호출에는 적용되지 않는다 — `b_transport`의 여러 동시 request에는 순서 제약이 없다. 반대로 **들어오는 `nb_transport_fw` 호출을 나가는 `b_transport` 호출로 변환하는 경우, `nb_transport` 순서 규칙을 강제하기 위해 `b_transport` 호출들을 직렬화해야(must serialize) 한다.**
- f) 반면 interconnect나 target은 **다른 target socket으로 들어왔거나, 다른 initiator socket으로 나가거나, 주소가 겹치지 않거나, `b_transport`로 들어온** 여러 동시 request를 재정렬해도 된다.
- g) **response는 재정렬되어도 된다. response가 request를 보낸 순서대로 initiator에 돌아온다는 보장은 없다.**
- h) 기술적으로는 ignorable extension으로 interconnect가 여러 동시 request를 재정렬하게 만들 수 있으며, 그 경우 extension을 추가한 initiator는 target에서의 out-of-order 실행을 감내할 수 있어야 한다. 반대로 **response가 request 순서대로 돌아오도록 강제하는 extension은 ignorable하지 않으므로 base protocol에서 허용되지 않는다.**

### 15.2.10 Base protocol rules for switching between b_transport and nb_transport

- a) initiator나 interconnect 안의 **각 thread는 서로 다른 트랜잭션 객체에 대해 `b_transport`와 `nb_transport_fw` 호출을 전환해도 된다**. 의도는 loosely-timed와 approximately-timed 코딩 스타일 사이 간헐적 전환을 허용하는 것. 두 호출을 인터리브하는 initiator는 **타이밍 정확도에 대한 기대를 낮춰야 should**.
- b) **모든 interconnect와 target은 blocking·non-blocking transport 인터페이스를 모두 지원할 의무가 있고(obliged)**, 내부 상태 정보를 **두 인터페이스 모두에서 접근 가능하도록 유지해야** 한다. 같은 소켓으로 들어오든 다른 소켓으로 들어오든 마찬가지.
- c) initiator/interconnect 안의 thread는 **같은 트랜잭션 인스턴스에 대해 `b_transport`와 `nb_transport_fw`를 둘 다 호출해서는 안 된다 shall not.** 단, **같은 트랜잭션 객체**가 매번 다른 트랜잭션 인스턴스를 나타내는 경우에는 둘 다 호출해도 된다.
- d) initiator/interconnect 안의 thread는 **같은 thread의 이전 `nb_transport_fw` 호출로 진행 중인 트랜잭션(reference count가 0이 아닌 트랜잭션)이 남아 있으면 `b_transport`를 호출하지 않는 것이 권장(recommended)**. 그렇지 않으면 downstream이 두 트랜잭션이 별개 initiator에서 왔다고 잘못 추론할 수 있다.
- e) convenience socket `simple_target_socket`은 base protocol target이 `b_transport`와 `nb_transport_fw` 중 하나만 구현하면서 두 인터페이스를 모두 지원하는 방법의 예를 제공한다 (§16.2.2).

### 15.2.11 Other base protocol rules

- a) **주어진 트랜잭션 객체를 여러 병렬 소켓이나 여러 병렬 경로로 동시에 보내서는 안 된다 shall not.** 각 트랜잭션 인스턴스는 컴포넌트·소켓 집합을 통과하는 **유일하고 잘 정의된 경로를 가져야 shall** 하며, 그 경로는 트랜잭션 인스턴스의 lifetime 동안 **고정되어야 shall** 하고 transport, DMI, debug transport 인터페이스에 공통이다. 물론 같은 소켓으로 보낸 서로 다른 트랜잭션은 다른 경로를 탈 수 있다. 또한 컴포넌트가 interconnect로 동작할지 target으로 동작할지 **동적으로 선택해도 된다**.
- b) upstream 컴포넌트는 자신이 interconnect에 연결되었는지 target에 직접 연결되었는지 **알아서도 안 되고 알 필요도 없어야 should**. downstream 컴포넌트도 마찬가지로 interconnect인지 initiator인지 알 필요가 없어야 should.
- c) **write 트랜잭션(`TLM_WRITE_COMMAND`)에서 `TLM_OK_RESPONSE`는 write command가 target에서 성공적으로 완료되었음을 나타내야 shall.** target은 `b_transport`에서 반환하기 전, backward/return path로 `BEGIN_RESP`를 보내기 전, 또는 `TLM_COMPLETED`를 반환하기 전에 **response status를 설정할 의무가 있다(obliged)**. 즉 **interconnect가 target의 성공 확인 없이 write 트랜잭션 완료를 알리는 것은 허용되지 않는다.** 의도는 target 시뮬레이션 모델 내 저장소의 coherency 보장.
- d) **read 트랜잭션(`TLM_READ_COMMAND`)에서 `TLM_OK_RESPONSE`는 read command가 완료되었고 generic payload data array가 target에 의해 수정되었음을 나타내야 shall.** target은 `b_transport` 반환 전, `BEGIN_RESP` 전송 전, 또는 `TLM_COMPLETED` 반환 전에 response status를 설정할 의무가 있다.

### 15.2.12 Summary of base protocol transaction ordering rules

Table 58. base protocol의 순서 규칙은 세 범주의 합집합이다: core transport 인터페이스의 timing annotation 규칙, base protocol 고유의 causality/phase 규칙, pipeline된 request가 initiator가 기대한 순서로 target에서 실행되도록 보장하는 base protocol 고유 규칙.

| Circumstance | Ordering rule |
|---|---|
| Effective local time 순서 ≠ interface method call 순서 | 수신자는 **임의 순서**로 실행·라우팅해도 된다. **다른 모든 규칙에 우선한다** |
| 같은 소켓·같은 트랜잭션에 대한 연속된 transport method 호출·반환 | Effective local time 순서가 **non-decreasing이어야 shall** |
| 같은 initiator process의 연속된 transport method 호출 | Effective local time 순서 non-decreasing **권장(recommended)** |
| 같은 initiator process의 연속된 transport method 호출 | 이전 `nb_transport` 트랜잭션이 아직 살아 있으면 `b_transport` 호출을 **하지 않는 것이 권장** |
| 같은 소켓, **다른** 트랜잭션에 대한 연속된 transport method 호출 | Effective local time 순서 의무 **없음**. 단 들어온 트랜잭션 스트림이 non-decreasing이었다면 non-decreasing 권장 |
| `nb_transport`에서, 같은 소켓에 두 request 또는 두 response가 outstanding | **금지(Forbidden)** |
| 서로 다른 소켓으로 들어온 트랜잭션 | 실행·라우팅 순서에 대한 의무 **없음** |
| 주소가 겹치는 여러 동시 request | forward로 라우팅한다면 **같은 소켓으로 보내야 shall** |
| 같은 소켓으로 `nb_transport`를 통해 들어온, 주소가 겹치는 여러 동시 request | **받은 순서와 같은 순서로** 실행·forward 라우팅해야 shall |
| `b_transport`로 들어온 여러 동시 request | 실행·forward 라우팅 순서에 대한 의무 **없음** |
| 여러 동시 response | 실행·backward 라우팅 순서에 대한 의무 **없음** |

---

## 15.2.13 Guidelines for creating base-protocol-compliant components

### 15.2.13.1 Overview

§15.2.13은 base protocol 컴포넌트 작성 가이드라인 모음이다. 문서 다른 곳에서 더 자세히 설명된 규칙 일부를 편의상 간략히 재진술한 것.

### 15.2.13.2 base protocol initiator 작성 가이드라인

- a) memory-mapped bus 연결마다 `tlm_initiator_socket`(또는 파생 클래스) initiator socket **한 개**를 인스턴스화한다.
- b) `tlm_initiator_socket`의 `TYPES` 템플릿 인자는 **기본값 `tlm_base_protocol_types`**를 쓰게 둔다.
- c) 멤버 함수 **`nb_transport_bw`와 `invalidate_direct_mem_ptr`를 구현**한다. (convenience socket `simple_initiator_socket`을 인스턴스화하면 명시적 구현을 피할 수 있다.)
- d) `b_transport`나 `nb_transport_fw`에 넘기기 전 **generic payload 트랜잭션 객체의 모든 attribute를 설정**한다. 특히 **response status와 DMI hint attribute를 호출 전에 reset**하는 것을 잊지 말 것. (byte enable pointer가 0이면 byte enable length는 설정할 필요 없고, extension도 쓸 필요 없다.)
- e) generic payload extension 메커니즘을 쓸 때는 그 extension이 **target과 모든 interconnect에게 ignorable한지 확인**한다.
- f) **phase sequencing, flow control, timing annotation, transaction ordering**에 관한 base protocol 규칙을 따른다.
- g) 트랜잭션 완료 시(또는 `BEGIN_RESP` 수신 후) **response status attribute 값을 확인**한다.

### 15.2.13.3 nb_transport를 호출하는 initiator 작성 가이드라인

- a) 트랜잭션을 `nb_transport_fw` 인자로 넘기기 전에 트랜잭션 객체에 **memory manager를 설정하고 `acquire`를 호출**한다. 트랜잭션 완료 시 **`release`를 호출**한다.
- b) `nb_transport_fw` 호출 시 트랜잭션 상태에 따라 phase 인자를 **`BEGIN_REQ` 또는 `END_RESP`**로 설정한다. **이전 트랜잭션의 `END_REQ`를 받거나 추론하기 전에 `BEGIN_REQ`를 보내지 말 것.**
- c) 주어진 트랜잭션에 대해 `nb_transport_fw`를 연속 호출할 때 **effective local time(simulation time + timing annotation)이 non-decreasing 시퀀스인지 확인**한다.
- d) 들어오는 phase 값 `END_REQ`와 `BEGIN_RESP`에 적절히 응답한다 — backward path(`nb_transport_bw` 호출), return path(`nb_transport_fw`가 `TLM_UPDATED` 반환), 암묵적(예: `nb_transport_fw`가 `TLM_COMPLETED` 반환) 어느 경로든. **들어오는 `BEGIN_REQ`와 `END_RESP`는 illegal이다.** 그 외 모든 들어오는 phase 값은 **ignorable로 취급**한다.

### 15.2.13.4 base protocol target 작성 가이드라인

- a) memory-mapped bus 연결마다 `tlm_target_socket`(또는 파생 클래스) target socket 한 개를 인스턴스화한다.
- b) `TYPES` 템플릿 인자는 기본값 `tlm_base_protocol_types`를 쓰게 둔다.
- c) 멤버 함수 **`b_transport`, `nb_transport_fw`, `get_direct_mem_ptr`, `transport_dbg`를 구현**한다. (`simple_target_socket`을 쓰면 전부 명시적으로 구현하지 않아도 된다.)
- d) `b_transport`와 `nb_transport_fw` 구현에서 **response status, DMI hint, extension을 제외한 generic payload의 모든 attribute를 검사하고 그에 따라 동작**한다. generic payload의 전체 기능을 구현하는 대신, 특정 attribute에 대해 **error response를 생성해 응답해도 된다 may**. **response status attribute를 성공/실패에 맞게 설정**한다.
- e) phase sequencing, flow control, timing annotation, transaction ordering에 관한 base protocol 규칙을 따른다.
- f) `get_direct_mem_ptr` 구현에서 **`false`를 반환하거나**, generic payload의 command·address attribute 값을 검사·처리하고 반환값과 DMI descriptor(`tlm_dmi`)의 모든 attribute를 적절히 설정한다.
- g) `transport_dbg` 구현에서 **`0`을 반환하거나**, generic payload의 command, address, data length, data pointer attribute 값을 검사·처리한다.
- h) 각 인터페이스에서 target은 generic payload의 ignorable extension을 검사·처리해도 되지만 may, **의무는 아니다**.

### 15.2.13.5 nb_transport를 호출하는 target 작성 가이드라인

- a) `nb_transport_bw` 호출 시 트랜잭션 상태에 따라 phase 인자를 **`END_REQ` 또는 `BEGIN_RESP`**로 설정한다. **이전 트랜잭션의 `END_RESP`를 받거나 추론하기 전에 `BEGIN_RESP`를 보내지 말 것.**
- b) 주어진 트랜잭션에 대해 `nb_transport_bw`를 연속 호출할 때 effective local time이 non-decreasing 시퀀스인지 확인한다.
- c) 들어오는 phase 값 `BEGIN_REQ`와 `END_RESP`에 적절히 응답한다 — forward path(`nb_transport_fw` 호출), return path(`nb_transport_bw`가 `TLM_UPDATED` 반환), 암묵적(예: `nb_transport_bw`가 `TLM_COMPLETED` 반환) 어느 경로든. **들어오는 `END_REQ`와 `BEGIN_RESP`는 illegal이다.** 그 외 모든 들어오는 phase 값은 ignorable로 취급한다.
- d) `nb_transport_fw` 구현에서 멤버 함수 반환 후에도 트랜잭션 객체의 포인터/참조를 유지해야 한다면 **`acquire`를 호출**한다. 트랜잭션 객체 사용이 끝나면 **`release`를 호출**한다.

### 15.2.13.6 base protocol interconnect component 작성 가이드라인

- a) memory-mapped bus 연결마다 `tlm_initiator_socket` 또는 `tlm_target_socket`(또는 파생 클래스) 소켓 한 개를 인스턴스화한다.
- b) 각 소켓의 `TYPES` 템플릿 인자는 기본값 `tlm_base_protocol_types`를 쓰게 둔다.
- c) **각 initiator socket마다 `nb_transport_bw`, `invalidate_direct_mem_ptr`를**, **각 target socket마다 `b_transport`, `nb_transport_fw`, `get_direct_mem_ptr`, `transport_dbg`를 구현**한다. (convenience socket을 쓰면 전부 명시적으로 구현하지 않아도 된다.)
- d) 들어오는 interface method call을 forward·backward 양쪽 path에 적절히 전달한다 — **request/response exclusion rule, transaction ordering rule, `TLM_COMPLETED` 이후 추가 호출 금지 규칙을 지키면서**. **ignorable phase는 전달하지 말 것(Do not pass on).** `get_direct_mem_ptr`와 `transport_dbg` 구현은 트랜잭션 객체를 forward하지 않고 각각 `false`와 `0`을 반환해도 된다 may.
- e) transport 인터페이스 구현에서 **interconnect가 수정 가능한 generic payload attribute는 address, DMI hint, extension뿐**. 다른 attribute는 수정하지 말 것. 다른 attribute를 수정해야 한다면 **새 트랜잭션 객체를 생성해 스스로 initiator가 되어야 should**.
- f) forward path에서 generic payload의 address attribute를 decode하고, 시스템 메모리 맵상 target 위치에 따라 필요하면 address attribute를 수정한다. transport, DMI, debug transport 인터페이스 모두에 적용.
- g) transport 인터페이스 구현에서 phase sequencing, flow control, timing annotation, transaction ordering에 관한 base protocol 규칙을 따른다.
- h) `get_direct_mem_ptr` 구현에서 **forward path에서는 DMI descriptor attribute를 수정하지 말 것**. **return path에서는** DMI pointer, DMI start/end address, DMI access attribute를 적절히 수정할 것.
- i) `invalidate_direct_mem_ptr` 구현에서 backward path로 호출을 전달하기 전에 **address range 인자를 수정**한다.
- j) `nb_transport_fw` 구현에서 함수 반환 후에도 트랜잭션 객체의 포인터/참조를 유지해야 한다면 `acquire`를 호출하고, 사용이 끝나면 `release`를 호출한다.
- k) 각 인터페이스에서 interconnect는 generic payload의 ignorable extension을 검사·처리해도 되지만 의무는 아니다. 트랜잭션을 더 확장해야 한다면 **그 extension이 다른 컴포넌트에게 ignorable한지 확인**하고, **extension에 대한 generic payload memory management 규칙을 지킬 것.**

---

## 흔한 위반 (pitfalls)

- **같은 phase로 연속 `nb_transport` 호출** — §15.2.3 e): 같은 트랜잭션에 대한 연속 두 호출은 phase 값이 달라야 shall. 상태 머신을 hop별로 유지하고 Table 57에 없는 전이를 만들지 말 것.
- **`END_REQ` 받기 전에 다음 `BEGIN_REQ` 전송** — §15.2.6 e) request exclusion rule 위반. 직전 트랜잭션의 `END_REQ`/`BEGIN_RESP`/`TLM_COMPLETED` 중 하나를 받을 때까지 대기할 것.
- **`END_RESP` 받기 전에 다음 `BEGIN_RESP` 전송** — §15.2.6 f) response exclusion rule 위반. `TLM_COMPLETED`를 `BEGIN_REQ` 응답으로 반환하는 것도 암묵적 `BEGIN_RESP`이므로 같은 위반이 된다 (§15.2.3 i)). 이 경우 `TLM_ACCEPTED`를 반환하고 `END_RESP`를 기다릴 것.
- **timing annotation으로 순서 규칙을 우회하려는 시도** — §15.2.4 b), §15.2.6 g): 모든 phase transition·exclusion 규칙은 **method call 순서에만** 기반한다. delay를 크게 준다고 exclusion rule이 완화되지 않는다.
- **effective local time이 감소** — §15.2.7 c): 같은 트랜잭션의 호출·반환 시퀀스에서 `sc_time_stamp() + t`는 non-decreasing이어야 shall. `t`를 줄여서 반환하지 말 것.
- **initiator가 response status를 reset하지 않고 전송** — §15.2.13.2 d): 호출 전 response status와 DMI hint를 reset해야 한다.
- **interconnect가 address 외 attribute 수정** — §15.2.13.6 e), §15.2.5 f): interconnect가 수정 가능한 것은 address, DMI hint, extension뿐. 다른 것이 필요하면 새 트랜잭션 객체를 만들어 initiator가 될 것.
- **인식하지 못하는 ignorable phase를 그대로 전파** — §15.2.5 g): 의미를 완전히 이해하는 phase만 전달해도 된다. transparent component만 예외.
- **`BEGIN_REQ` 이전 / `END_RESP` 이후에 ignorable phase 삽입** — §15.2.5 e): base protocol 위반이자 **error**.
- **`nb_transport`에 memory manager 없는 트랜잭션 전달** — §15.2.3 p): `nb_transport`에 넘기는 트랜잭션 객체는 memory manager를 가져야 한다. `acquire`를 부른 쪽은 완료 시점 이전에 `release`도 불러야 should.
- **같은 트랜잭션 인스턴스에 `b_transport`와 `nb_transport_fw` 혼용** — §15.2.10 c): shall not. 같은 *객체*를 다른 *인스턴스*로 재사용하는 것은 허용.
- **같은 소켓·같은 트랜잭션 객체로 re-entrant `b_transport`** — §15.2.8 d): forbidden.
- **write 완료를 target 확인 없이 interconnect가 보고** — §15.2.11 c): 허용되지 않는다. `TLM_OK_RESPONSE`는 target에서의 성공 완료를 뜻해야 shall.
- **response가 request 순서대로 돌아온다고 가정** — §15.2.9 g): 보장 없음. 순서를 강제하는 extension은 ignorable하지 않아 base protocol에서 허용되지 않는다 (§15.2.9 h)).
- **out-of-order phase 수신 시 동작을 기대** — §15.2.3 q): 불법/순서 위반 phase는 sender의 error이며 수신자 동작은 **undefined** (run-time error 가능).
