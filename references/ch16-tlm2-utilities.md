# Clause 16 — TLM-2.0 utilities

LRM pp. 550–574.

Convenience socket, quantum keeper, payload event queue, instance-specific extension. **모두 `namespace tlm_utils`**, 헤더는 `tlm_utils/...`.

## 16.1 Overview

utilities는 **편의와 일관된 코딩 스타일**을 위해 제공되는 클래스 집합이다. **interoperability layer에 속하지 않으므로 utilities 사용은 상호운용성의 요구사항이 아니다.**

---

## 16.2 Convenience sockets

### 16.2.1.1 Description

convenience socket은 컴포넌트 모델 작성을 쉽게 하는 추가 기능을 구현한 소켓 계열. `tlm_initiator_socket`, `tlm_target_socket`에서 파생되며, **interoperability layer의 일부가 아니고 `namespace tlm_utils`에 있다.**

### 16.2.1.2 Table 59 — Socket types

- **Register callbacks?** — 소켓이 인터페이스 구현 객체에 bind되는 대신, 들어오는 interface method call에 대한 콜백 등록 멤버 함수를 제공하는가.
- **Multi-ports?** — 소켓 클래스 템플릿이 number-of-bindings와 binding policy 템플릿 인자를 제공하여 initiator socket 하나를 여러 target socket에 bind할 수 있는가 (역도 동일).
- **b - nb conversion?** — target socket이 들어오는 `b_transport` 호출을 `nb_transport_fw` 호출로, 또는 그 역으로 변환할 수 있는가. `—`는 initiator socket을 뜻함.
- **Tagged?** — 들어오는 interface method call에 어느 소켓으로 도착했는지 나타내는 id가 태깅되는가.

| Class | Register callbacks? | Multi-ports? | b - nb conversion? | Tagged? |
|---|:---:|:---:|:---:|:---:|
| `tlm_initiator_socket` | no | yes | — | no |
| `tlm_target_socket` | no | yes | no | no |
| `simple_initiator_socket` | yes | no | — | no |
| `simple_initiator_socket_tagged` | yes | no | — | yes |
| `simple_target_socket` | yes | no | **yes** | no |
| `simple_target_socket_tagged` | yes | no | **yes** | yes |
| `passthrough_target_socket` | yes | no | no | no |
| `passthrough_target_socket_tagged` | yes | no | no | yes |
| `multi_passthrough_initiator_socket` | yes | yes | — | yes |
| `multi_passthrough_target_socket` | yes | yes | no | yes |

### 16.2.1.3 Table 60 — Permitted socket bindings

binding 형태는 `From( To )` 또는 `From.bind( To )`.

| From \ To | tlm-init | simple-init | multi-init | tlm-targ | simple-targ | multi-targ |
|---|:---:|:---:|:---:|:---:|:---:|:---:|
| | *Hierarchical child-to-parent binding* | | | *Initiator-to-target binding* | | |
| **tlm-init**    | 1  |    |   | 1   | 1   | N:1 |
| **simple-init** | 1  |    |   | 1   | 1   | N:1 |
| **multi-init**  |    |    | 1 | 1:M | 1:M | N:M |
| | *Reverse binding operators* | | | *Hierarchical parent-to-child binding* | | |
| **tlm-targ**    | 1* | 1* |   | 1   | 1   |     |
| **simple-targ** | 1* | 1* |   |     |     |     |
| **multi-targ**  |    |    |   |     |     | 1   |

Key:

| 약칭 | 실제 클래스 |
|---|---|
| tlm-init | `tlm_initiator_socket` |
| simple-init | `simple_initiator_socket` 또는 `passthrough_initiator_socket` |
| multi-init | `multi_passthrough_initiator_socket` |
| tlm-targ | `tlm_target_socket` |
| simple-targ | `simple_target_socket` / `simple_target_socket_tagged` / `passthrough_target_socket` / `passthrough_target_socket_tagged` |
| multi-targ | `multi_passthrough_target_socket` |
| `1*` | method call이 `target.bind(initiator)` 방향이지만 **binding은 initiator→target** 방향이다 |

---

## 16.2.2 Simple sockets

### 16.2.2.1 Description

simple socket은 **쓰기 쉽도록** 만들어진 소켓. interoperability layer 소켓 `tlm_initiator_socket`, `tlm_target_socket`에서 파생되므로 그 타입의 소켓에 직접 bind 가능.

인터페이스 구현 객체에 소켓을 bind하는 대신, 각 simple socket은 **콜백 메서드 등록 멤버 함수**를 제공한다. 등록된 콜백은 들어오는 interface method call이 도착할 때마다 호출된다.

사용자는 모든 interface method에 대해 콜백을 등록해도 되지만 **의무는 아니다**. 특히 **simple target socket의 경우 `b_transport`와 `nb_transport_fw` 중 하나만 등록하면 되고**, 등록되지 않은 멤버 함수로 들어오는 호출은 자동으로 등록된 멤버 함수 호출로 변환된다.

- 이 변환 과정은 **간단하지 않으며(non-trivial), initiator와 target이 base protocol 규칙을 지킨다는 전제에 의존**한다.
- 따라서 `simple_target_socket`은 protocol type을 템플릿 인자로 받지만, **애플리케이션이 b/nb 변환을 사용하는 경우에만 드러나는 base protocol 의존성**이 존재한다. 구체적으로 `simple_target_socket`은 `tlm_phase_enum`의 값을 사용한다.
- base protocol이 아닌 프로토콜에서 이런 변환이 필요한 애플리케이션은 **새 convenience socket을 만들어야 한다(obliged)** — `simple_target_socket`에서 파생시켜도 됨.
- `passthrough_target_socket`은 **b/nb 변환을 지원하지 않는** `simple_target_socket`의 변종이다.

### 16.2.2.3 Header file

`tlm_utils/simple_initiator_socket.h`, `tlm_utils/simple_target_socket.h`, `tlm_utils/passthrough_target_socket.h` (shall).

### 16.2.2.2 Class definition

```cpp
namespace tlm_utils {

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types
>
class simple_initiator_socket : public tlm::tlm_initiator_socket<BUSWIDTH, TYPES>
{
public:
    typedef typename TYPES::tlm_payload_type transaction_type;
    typedef typename TYPES::tlm_phase_type          phase_type;
    typedef tlm::tlm_sync_enum                      sync_enum_type;

     simple_initiator_socket();
     explicit simple_initiator_socket( const char* n );

     void register_nb_transport_bw(
         MODULE* mod,
         sync_enum_type (MODULE::*cb)(transaction_type&, phase_type&, sc_core::sc_time&));

     void register_invalidate_direct_mem_ptr(
         MODULE* mod,
         void (MODULE::*cb)(sc_dt::uint64, sc_dt::uint64));
};

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types
>
class simple_target_socket : public tlm::tlm_target_socket<BUSWIDTH, TYPES>
{
public:
    typedef typename TYPES::tlm_payload_type transaction_type;
    typedef typename TYPES::tlm_phase_type          phase_type;
    typedef tlm::tlm_sync_enum                      sync_enum_type;

     simple_target_socket();
     explicit simple_target_socket( const char* n );

     tlm::tlm_bw_transport_if<TYPES> * operator ->();

     void register_nb_transport_fw(
         MODULE* mod,
         sync_enum_type (MODULE::*cb)(transaction_type&, phase_type&, sc_core::sc_time&));

     void register_b_transport(
         MODULE* mod,
         void (MODULE::*cb)(transaction_type&, sc_core::sc_time&));

     void register_transport_dbg(
         MODULE* mod,
         unsigned int (MODULE::*cb)(transaction_type&));

     void register_get_direct_mem_ptr(
         MODULE* mod,
         bool (MODULE::*cb)(transaction_type&, tlm::tlm_dmi&));
};

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types
>
class passthrough_target_socket : public tlm::tlm_target_socket<BUSWIDTH, TYPES>
{
public:
    typedef typename TYPES::tlm_payload_type transaction_type;
    typedef typename TYPES::tlm_phase_type       phase_type;
    typedef tlm::tlm_sync_enum                   sync_enum_type;

     passthrough_target_socket();
         explicit passthrough_target_socket( const char* n );

     void register_nb_transport_fw(
         MODULE* mod,
         sync_enum_type (MODULE::*cb)(transaction_type&, phase_type&, sc_core::sc_time&));

     void register_b_transport(
         MODULE* mod,
         void (MODULE::*cb)(transaction_type&, sc_core::sc_time&));

     void register_transport_dbg(
         MODULE* mod,
         unsigned int (MODULE::*cb)(transaction_type&));

     void register_get_direct_mem_ptr(
         MODULE* mod,
         bool (MODULE::*cb)(transaction_type&, tlm::tlm_dmi&));
};

} // namespace tlm_utils
```

### 16.2.2.4 Rules

- a) 각 생성자는 대응 base class의 생성자를 호출해야 shall 하며, `char*` 인자가 있으면 그대로 전달해야 shall. **default 생성자의 경우 base class 생성자의 `char*` 인자는 각각 `sc_gen_unique_name("simple_initiator_socket")`, `sc_gen_unique_name("simple_target_socket")`, `sc_gen_unique_name("passthrough_target_socket")`로 설정해야 shall.**
- b) `simple_initiator_socket`은 `simple_target_socket` 또는 `passthrough_target_socket`에 **어느 쪽 소켓의 `bind` 또는 `operator()`를 호출하든 정확히 같은 효과로** bind 가능. 어느 경우든 **forward path는 initiator socket → target socket 방향**.
- c) `simple_initiator_socket`을 `tlm_target_socket`에, `tlm_initiator_socket`을 `simple_target_socket`이나 `passthrough_target_socket`에 bind할 수 있다.
- d) `simple_initiator_socket`, `simple_target_socket`, `passthrough_target_socket`은 **콜백 등록으로만** 들어오는 interface method call을 구현할 수 있고, **자식 모듈의 다른 소켓에 계층적으로 bind하는 방식으로는 구현할 수 없다.** 반면 자식 모듈의 `simple_initiator_socket`을 부모 모듈의 `tlm_initiator_socket`에 계층적으로 bind하는 것, 부모 모듈의 `tlm_target_socket`을 자식 모듈의 `simple_target_socket`/`passthrough_target_socket`에 계층적으로 bind하는 것은 가능하다.
- e) `nb_transport_fw` 콜백을 등록했다면 target은 simple target socket에 **`b_transport` 콜백을 등록할 의무가 없다**. 이 경우 들어오는 `b_transport` 호출은 자동으로 `nb_transport_fw`용 등록 함수를 호출하게 만든다. 이때 **`nb_transport_fw`용 등록 함수는 base protocol 규칙에 맞게 구현해야 shall** (§16.2.2.5).
- f) `b_transport` 콜백을 등록했다면 target은 simple target socket에 **`nb_transport_fw` 콜백을 등록할 의무가 없다**. 이 경우 들어오는 `nb_transport_fw` 호출은 자동으로 `b_transport`용 등록 함수를 호출하고, 이어서 backward path로 `nb_transport_bw`를 호출하게 만든다.
- g) **target이 simple target socket에 `b_transport`와 `nb_transport_fw` 어느 것도 등록하지 않으면, 대응 멤버 함수가 호출될 때에 한하여(if and only if) run-time error가 발생한다.**
- h) target은 **passthrough target socket에는 `b_transport`와 `nb_transport_fw`를 등록해야 should**. 등록하지 않으면 대응 멤버 함수가 호출될 때에 한하여 **run-time error**.
- i) target은 simple/passthrough target socket에 `transport_dbg` 콜백을 등록할 의무가 없다. 등록하지 않으면 들어오는 `transport_dbg` 호출은 **`0`을 반환해야 shall**.
- j) target은 simple/passthrough target socket에 `get_direct_mem_ptr` 콜백을 등록할 의무가 없다. 등록하지 않으면 들어오는 `get_direct_mem_ptr` 호출은 **`false`를 반환해야 shall**.
- k) initiator는 simple initiator socket에 **`nb_transport_bw` 콜백을 등록해야 should**. 등록하지 않으면 `nb_transport_bw`가 호출될 때에 한하여 **run-time error**.
- l) initiator는 simple initiator socket에 `invalidate_direct_mem_ptr` 콜백을 등록할 의무가 없다. 등록하지 않으면 들어오는 `invalidate_direct_mem_ptr` 호출은 **무시되어야 shall**.

### 16.2.2.5 Simple target socket b/nb conversion

- a) `simple_target_socket`으로 `b_transport`나 `nb_transport_fw`가 호출되었는데 대응 콜백이 등록되지 않은 경우, simple target socket은 **두 인터페이스 사이의 adapter로 동작**한다. **이 경우에만, 그리고 오직 이 경우에만** `simple_target_socket` 구현이 `tlm_phase_enum` 값에 명시적으로 의존해야 shall.
- b) adapter로 동작할 때 소켓은 **initiator 관점과 target의 `b_transport`/`nb_transport_fw` 구현 관점 양쪽에서 base protocol 규칙을 지켜야 shall** (§15.2).
- c) 소켓은 주어진 트랜잭션 객체를 **수정 없이 통과시켜야 shall** 하며, **새 트랜잭션 객체를 생성해서는 안 된다 shall not**.
- d) **target이 `nb_transport_fw` 콜백만 등록한 경우, initiator는 그 initiator의 이전 `b_transport` 호출이 아직 진행 중인 동안 `nb_transport_fw`를 호출하는 것이 허용되지 않는다. 이는 현재 simple target socket 구현의 제약(limitation)이다.**
- e) (Figure 34, informative) initiator가 `nb_transport_fw`를 호출하지만 target은 `b_transport` 콜백만 등록한 경우: initiator가 `BEGIN_REQ` 전송 → 소켓이 `TLM_ACCEPTED` 반환 → 소켓이 `b_transport` 호출 → 반환 시 initiator에 `BEGIN_RESP` 전송 → initiator가 `TLM_COMPLETED` 반환. **SystemC에서 non-blocking 멤버 함수로부터 blocking 멤버 함수를 직접 호출하는 것은 허용되지 않으므로, 소켓은 `nb_transport_fw`에서 직접이 아니라 별도의 내부 thread process에서 `b_transport`를 호출할 의무가 있다(obliged).**
- f) Figure 34는 가능한 한 시나리오일 뿐이다. 마지막 전이에서 initiator가 `TLM_ACCEPTED`를 반환했을 수도 있으며, 그 경우 소켓은 이후 initiator로부터 `END_RESP`를 받을 것을 기대한다. 또 target이 `b_transport` 안에서 `wait`를 호출했을 수도 있다.
- g) (Figure 35, informative) initiator가 `b_transport`를 호출하지만 target은 `nb_transport_fw` 콜백만 등록한 경우: initiator가 `b_transport` 호출 → 소켓과 target이 base protocol 규칙을 지키며 `nb_transport`로 handshake. **target은 `END_REQ` phase를 보낼 수도 보내지 않을 수도 있으며, 곧바로 `BEGIN_RESP`로 건너뛸 수 있다.** 소켓은 `BEGIN_RESP` phase에 대한 `nb_transport_bw` 호출에서 `TLM_COMPLETED`를 반환한다.

### Example (informative)

```cpp
#include <tlm>
#include "tlm_utils/simple_initiator_socket.h"        // utilities 헤더 파일
#include "tlm_utils/simple_target_socket.h"

struct Initiator : sc_core::sc_module {
    tlm_utils::simple_initiator_socket<Initiator, 32, tlm::tlm_base_protocol_types> socket;

      SC_CTOR(Initiator)
      : socket("socket") {                            // simple socket 생성·명명
           socket.register_nb_transport_bw(this, &Initiator::nb_transport_bw);
                                                      // simple socket에 콜백 등록
           socket.register_invalidate_direct_mem_ptr(this, &Initiator::invalidate_direct_mem_ptr);
      }

      virtual tlm::tlm_sync_enum nb_transport_bw(
               tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay){
           return tlm::TLM_COMPLETED;                 // placeholder 구현
      }
      virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
      }                                               // placeholder 구현
};

struct Target : sc_core::sc_module {                   // target 컴포넌트
    tlm_utils::simple_target_socket<Target, 32, tlm::tlm_base_protocol_types> socket;

     SC_CTOR(Target)
     : socket("socket") {                              // simple socket 생성·명명
          socket.register_nb_transport_fw(this, &Target::nb_transport_fw);
                                                       // simple socket에 콜백 등록
          socket.register_b_transport(this, &Target::b_transport);
          socket.register_get_direct_mem_ptr(this, &Target::get_direct_mem_ptr);
          socket.register_transport_dbg(this, &Target::transport_dbg);
     }

     virtual void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
     }                                                 // placeholder 구현

     virtual tlm::tlm_sync_enum nb_transport_fw(
              tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_time &delay) {
          return tlm::TLM_ACCEPTED;                    // placeholder 구현
     }

     virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi_data) {
          return false;                                // placeholder 구현
     }

     virtual unsigned int transport_dbg(tlm::tlm_generic_payload &r) {
          return 0;                                    // placeholder 구현
     }
};

SC_MODULE(Top)
{
   Initiator *initiator;
   Target *target;
   SC_CTOR(Top) {
        initiator = new Initiator("initiator");
        target = new Target("target");
        initiator->socket.bind( target->socket );      // initiator socket을 target socket에 bind
   }
};
```

---

## 16.2.3 Tagged simple sockets

### 16.2.3.1 Description

tagged simple socket은 들어오는 interface method call에 **정수 id를 태깅**하여 콜백이 어느 소켓으로 들어온 호출인지 식별할 수 있게 하는 simple socket의 변종. **같은 콜백 멤버 함수를 여러 initiator socket 또는 여러 target socket에 등록하는 경우에 유용.** id는 콜백 등록 시 지정하고, **콜백 멤버 함수의 추가 첫 번째 인자로 삽입**된다.

### 16.2.3.2 Header file

대응하는 simple socket과 **같은 헤더 파일**: `tlm_utils/simple_initiator_socket.h`, `tlm_utils/simple_target_socket.h`, `tlm_utils/passthrough_target_socket.h` (shall).

### 16.2.3.3 Class definition

```cpp
namespace tlm_utils {

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types
>
class simple_initiator_socket_tagged : public tlm::tlm_initiator_socket<BUSWIDTH, TYPES>
{
public:
    typedef typename TYPES::tlm_payload_type transaction_type;
    typedef typename TYPES::tlm_phase_type         phase_type;
    typedef tlm::tlm_sync_enum                     sync_enum_type;

     simple_initiator_socket_tagged();
     explicit simple_initiator_socket_tagged( const char* n );

     void register_nb_transport_bw(
         MODULE* mod,
         sync_enum_type (MODULE::*cb)(int, transaction_type&, phase_type&, sc_core::sc_time&),
         int id);

     void register_invalidate_direct_mem_ptr(
         MODULE* mod,
         void (MODULE::*cb)(int, sc_dt::uint64, sc_dt::uint64),
         int id);
};

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types
>
class simple_target_socket_tagged : public tlm::tlm_target_socket<BUSWIDTH, TYPES>
{
public:
    typedef typename TYPES::tlm_payload_type              transaction_type;
    typedef typename TYPES::tlm_phase_type                phase_type;
    typedef tlm::tlm_sync_enum                            sync_enum_type;
    typedef tlm::tlm_fw_transport_if<TYPES>               fw_interface_type;
    typedef tlm::tlm_bw_transport_if<TYPES>               bw_interface_type;
    typedef tlm::tlm_target_socket<BUSWIDTH, TYPES> base_type;

     simple_target_socket_tagged();
     explicit simple_target_socket_tagged( const char* n );

     tlm::tlm_bw_transport_if<TYPES> * operator ->();

     void register_nb_transport_fw(
         MODULE* mod,
         sync_enum_type (MODULE::*cb)(int id, transaction_type&, phase_type&, sc_core::sc_time&),
         int id);

     void register_b_transport(
         MODULE* mod,
         void (MODULE::*cb)(int id, transaction_type&, sc_core::sc_time&),
         int id);

     void register_transport_dbg(
     MODULE* mod,
         unsigned int (MODULE::*cb)(int id, transaction_type&),
         int id);

     void register_get_direct_mem_ptr(
         MODULE* mod,
         bool (MODULE::*cb)(int id, transaction_type&, tlm::tlm_dmi&),
          int id);
};

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types
>
class passthrough_target_socket_tagged : public tlm::tlm_target_socket<BUSWIDTH, TYPES>
{
public:
    typedef typename TYPES::tlm_payload_type            transaction_type;
    typedef typename TYPES::tlm_phase_type              phase_type;
    typedef tlm::tlm_sync_enum                          sync_enum_type;

     passthrough_target_socket_tagged();
     explicit passthrough_target_socket_tagged( const char* n );

     void register_nb_transport_fw(
         MODULE* mod,
         sync_enum_type (MODULE::*cb)(int id, transaction_type&, phase_type&, sc_core::sc_time&),
         int id);

     void register_b_transport(
         MODULE* mod,
         void (MODULE::*cb)(int id, transaction_type&, sc_core::sc_time&),
         int id);

     void register_transport_dbg(
         MODULE* mod,
         unsigned int (MODULE::*cb)(int id, transaction_type&),
         int id);

     void register_get_direct_mem_ptr(
          MODULE* mod,
          bool (MODULE::*cb)(int id, transaction_type&, tlm::tlm_dmi&),
          int id);
};

} // namespace tlm_utils
```

### 16.2.3.4 Rules

- a) 각 생성자는 대응 base class 생성자를 호출해야 shall 하며 `char*` 인자가 있으면 전달해야 shall. default 생성자의 경우 `char*` 인자는 각각 `sc_gen_unique_name("simple_initiator_socket_tagged")`, `sc_gen_unique_name("simple_target_socket_tagged")`, `sc_gen_unique_name("passthrough_target_socket_tagged")`로 설정해야 shall.
- b) **`int id` 태그를 제외하면 tagged simple socket은 untagged simple socket과 동일하게 동작**한다.
- c) 주어진 콜백 멤버 함수를 **서로 다른 태그로 여러 소켓 인스턴스에 등록할 수 있다.** 이것이 tagged socket의 목적.
- d) **`int id` 태그는 콜백 등록 멤버 함수의 마지막 인자로 지정**한다. 소켓은 이 태그를 **대응 콜백 멤버 함수의 첫 번째 인자로 앞에 붙여야 shall**. 등록 함수와 콜백 함수에서 **id 태그 인자의 위치가 다르다는 점에 주의.**
- e) **tagged simple socket은 multi-socket이 아니다. tagged simple socket은 다른 컴포넌트의 여러 소켓에 bind할 수 없다** (§16.2.4 참조).

### Example (informative)

```cpp
struct my_target : sc_core::sc_module {
    tlm_utils::simple_target_socket_tagged<my_target> socket1;
    tlm_utils::simple_target_socket_tagged<my_target> socket2;

      SC_CTOR(my_target)
      : socket1("socket1"), socket2("socket2") {
           socket1.register_b_transport(this, &my_target::b_transport, 1);   // id = 1 로 등록
           socket2.register_b_transport(this, &my_target::b_transport, 2);   // id = 2 로 등록
      }
      void b_transport(int id, Transaction &trans, sc_core::sc_time &delay); // id = 1 또는 2로 호출됨
      ...
};
```

---

## 16.2.4 Multi-sockets

### 16.2.4.1 Description

multi-socket은 **소켓 하나를 다른 컴포넌트의 여러 소켓에 bind할 수 있게 하는** tagged simple socket의 변종. tagged simple socket이 "어느 소켓으로 들어왔는가"를 식별하는 것과 대조적으로, multi-socket 콜백은 **multi-port index를 태그로 써서 "다른 컴포넌트의 어느 소켓에서 왔는가"를 식별**할 수 있다. 다른 convenience socket과 달리 multi-socket은 **initiator 쪽과 target 쪽 모두에서 계층적 child-to-parent 소켓 binding을 지원**한다.

### 16.2.4.2 Header file

`tlm_utils/multi_passthrough_initiator_socket.h`, `tlm_utils/multi_passthrough_target_socket.h` (shall).

### 16.2.4.3 Class definition

```cpp
namespace tlm_utils {

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types,
    unsigned int N=0,
    sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
>
class multi_passthrough_initiator_socket : public multi_init_base< BUSWIDTH, TYPES, N, POL>
{
public:
    typedef typename TYPES::tlm_payload_type                     transaction_type;
    typedef typename TYPES::tlm_phase_type                       phase_type;
    typedef tlm::tlm_sync_enum                                   sync_enum_type;
    typedef multi_init_base<BUSWIDTH, TYPES, N, POL>             base_type;
    typedef typename base_type::base_target_socket_type          base_target_socket_type;

     multi_passthrough_initiator_socket();
     multi_passthrough_initiator_socket(const char* name);
     ~multi_passthrough_initiator_socket();

     void register_nb_transport_bw(
         MODULE* mod,
         sync_enum_type (MODULE::*cb)(int, transaction_type&, phase_type&, sc_core::sc_time&));

     void register_invalidate_direct_mem_ptr(
         MODULE* mod,
         void (MODULE::*cb)(int, sc_dt::uint64, sc_dt::uint64));

     // tlm_initiator_socket의 virtual 함수 override:
     virtual tlm::tlm_bw_transport_if<TYPES>& get_base_interface();
     virtual const tlm::tlm_bw_transport_if<TYPES>& get_base_interface() const;

     virtual sc_core::sc_export<tlm::tlm_bw_transport_if<TYPES> >& get_base_export();
     virtual const sc_core::sc_export<tlm::tlm_bw_transport_if<TYPES> >& get_base_export() const;

     virtual void bind(base_target_socket_type& s);
     void operator() (base_target_socket_type& s);

     // SystemC standard callback
     // multi_passthrough_initiator_socket::before_end_of_elaboration must be called from
     // any derived class
     void before_end_of_elaboration();

     // Bind multi initiator socket to multi initiator socket (hierarchical bind)
     virtual void bind(base_type& s);
     void operator() (base_type& s);

     tlm::tlm_fw_transport_if<TYPES>* operator[](int i);
     unsigned int size();
};

template <
    typename MODULE,
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm::tlm_base_protocol_types,
    unsigned int N=0,
    sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
>
class multi_passthrough_target_socket : public multi_target_base< BUSWIDTH, TYPES, N, POL>
{
public:
    typedef typename TYPES::tlm_payload_type transaction_type;
    typedef typename TYPES::tlm_phase_type       phase_type;
    typedef tlm::tlm_sync_enum                   sync_enum_type;

     typedef sync_enum_type
                          (MODULE::*nb_cb)(int, transaction_type&, phase_type&, sc_core::sc_time&);
     typedef void         (MODULE::*b_cb)(int, transaction_type&, sc_core::sc_time&);
     typedef unsigned int (MODULE::*dbg_cb)(int, transaction_type& txn);
     typedef bool         (MODULE::*dmi_cb)(int, transaction_type& txn, tlm::tlm_dmi& dmi);

     typedef multi_target_base<BUSWIDTH, TYPES, N, POL>          base_type;
     typedef typename base_type::base_initiator_socket_type      base_initiator_socket_type;
     typedef typename base_type::initiator_socket_type           initiator_socket_type;

     multi_passthrough_target_socket();
     multi_passthrough_target_socket(const char* name);
     ~multi_passthrough_target_socket();

     void register_nb_transport_fw                      (MODULE* mod, nb_cb cb);
     void register_b_transport                          (MODULE* mod, b_cb cb);
     void register_transport_dbg                        (MODULE* mod, dbg_cb cb);
     void register_get_direct_mem_ptr                   (MODULE* mod, dmi_cb cb);

     // tlm_target_socket의 virtual 함수 override:
     virtual tlm::tlm_fw_transport_if<TYPES>& get_base_interface();
     virtual const tlm::tlm_fw_transport_if<TYPES>& get_base_interface() const;
     virtual sc_core::sc_export<tlm::tlm_fw_transport_if<TYPES> >& get_base_export();
     virtual const sc_core::sc_export<tlm::tlm_fw_transport_if<TYPES> >& get_base_export() const;

     // SystemC standard callback
     // multi_passthrough_target_socket::end_of_elaboration must be called from any derived class
     void end_of_elaboration();

     virtual void bind(base_type& s);
     void operator() (base_type& s);

     tlm::tlm_bw_transport_if<TYPES>* operator[] (int i);
     unsigned int size();
};

} // namespace tlm_utils
```

> **주의** — 파생 클래스를 만들 경우 `multi_passthrough_initiator_socket::before_end_of_elaboration`과 `multi_passthrough_target_socket::end_of_elaboration`을 **파생 클래스에서 반드시 호출해야 한다(must)**.

### 16.2.4.4 Rules

- a) **base class `multi_init_base`와 `multi_target_base`는 implementation-defined이며, 애플리케이션이 직접 사용해서는 안 된다(should not).**
- b) 각 생성자는 대응 base class 생성자를 호출해야 shall 하며 `char*` 인자가 있으면 전달해야 shall. default 생성자의 경우 각각 `sc_gen_unique_name("multi_passthrough_initiator_socket")`, `sc_gen_unique_name("multi_passthrough_target_socket")`로 설정해야 shall.
- c) 두 클래스는 multi-socket으로 동작한다: initiator socket 하나를 여러 target socket에, target socket 하나를 여러 initiator socket에 bind 가능. 두 클래스 템플릿은 **binding 개수와 port binding policy를 지정하는 템플릿 파라미터**를 가지며, 이는 내부의 `sc_port` 템플릿 인스턴스화를 파라미터화하는 데 쓰인다.
- d) `multi_passthrough_initiator_socket` 하나를 다수의 `tlm_target_socket` / `simple_target_socket` / `passthrough_target_socket` / `multi_passthrough_target_socket`에 bind할 수 있다. 다수의 `tlm_initiator_socket` / `simple_initiator_socket` / `multi_passthrough_initiator_socket`을 하나의 `multi_passthrough_target_socket`에 bind할 수 있다.
- e) **`multi_passthrough_initiator_socket`은 정확히 하나의 다른 `multi_passthrough_initiator_socket`에만 계층적으로 bind 가능. `multi_passthrough_target_socket`은 정확히 하나의 다른 `multi_passthrough_target_socket`에만 계층적으로 bind 가능.** 이 두 경우 외에 multi-socket은 다른 소켓에 계층적으로 bind할 수 없다. **multi-socket의 다중 binding 능력은 계층적 binding에는 적용되지 않고, initiator socket을 target socket에 bind할 때만 적용된다.**
- f) 각 `operator()` 구현은 대응하는 virtual 멤버 함수 `bind` 호출로 그 효과를 달성해야 shall.
- g) **binding operator는 initiator-socket→target-socket 방향으로만 쓸 수 있다.** 즉 `tlm_target_socket`이나 `simple_target_socket`과 달리 `multi_passthrough_target_socket`에는 target socket을 initiator socket에 bind하는 operator가 **없다**.
- h) 계층적 binding에서는 **자식 모듈의 initiator multi-socket을 부모 모듈의 initiator multi-socket에** bind해야 shall 하고, **부모 모듈의 target multi-socket을 자식 모듈의 target multi-socket에** bind해야 shall. 이는 위의 initiator→target binding 방향 규칙과 일관된다.
- i) multi-socket 객체가 여러 번 bind되면 `operator[]`로 대응 객체를 지정할 수 있다. **index 값은 `bind` 또는 `operator()`가 호출된 순서로 결정된다. 이 index 값이 곧 콜백에 전달되는 id 태그를 결정한다.**
- j) 예: `multi_passthrough_initiator_socket`이 두 target에 bind된 경우, `socket[0]->nb_transport_fw(...)`와 `socket[1]->nb_transport_fw()`가 각각 두 target을 지정하고, 그 두 target에서 들어오는 `nb_transport_bw()` 호출은 태그 0과 1을 갖는다.
- k) 멤버 함수 `size`는 현재 multi-socket이 bind된 소켓 인스턴스의 개수를 반환해야 shall. SystemC multi-port와 마찬가지로, **`size`를 elaboration 중 `end_of_elaboration` 콜백 이전에 호출하면 반환값은 implementation-defined** — port binding 완료 시점이 implementation-defined이기 때문.
- l) 자식 모듈의 multi-socket에 대한 계층적 binding이 없는 경우, target은 target multi-socket에 **`b_transport`와 `nb_transport_fw` 콜백을 등록해야 should**. 등록하지 않으면 대응 멤버 함수가 호출될 때에 한하여 **run-time error**.
- m) 계층적 binding이 없는 경우, target은 target multi-socket에 `transport_dbg` 콜백을 등록할 의무가 없다. 등록하지 않으면 들어오는 `transport_dbg` 호출은 **`0`을 반환해야 shall**.
- n) 계층적 binding이 없는 경우, target은 target multi-socket에 `get_direct_mem_ptr` 콜백을 등록할 의무가 없다. 등록하지 않으면 들어오는 `get_direct_mem_ptr` 호출은 **`false`를 반환해야 shall**.
- o) 계층적 binding이 없는 경우, initiator는 initiator multi-socket에 **`nb_transport_bw` 콜백을 등록해야 should**. 등록하지 않으면 `nb_transport_bw` 호출 시에 한하여 **run-time error**.
- p) 계층적 binding이 없는 경우, initiator는 initiator multi-socket에 `invalidate_direct_mem_ptr` 콜백을 등록할 의무가 없다. 등록하지 않으면 들어오는 `invalidate_direct_mem_ptr` 호출은 **무시되어야 shall**.

### Example (informative)

```cpp
// multi-socket을 가진 initiator 컴포넌트
struct Initiator : sc_core::sc_module {
     tlm_utils::multi_passthrough_initiator_socket<Initiator> socket;

      SC_CTOR(Initiator) : socket("socket") {
          // 콜백 멤버 함수를 소켓에 등록
          socket.register_nb_transport_bw(this, &Initiator::nb_transport_bw);
          socket.register_invalidate_direct_mem_ptr(this, &Initiator::invalidate_direct_mem_ptr);
      }
      ...
};

struct Initiator_parent : sc_core::sc_module {
    tlm_utils::multi_passthrough_initiator_socket<Initiator_parent> socket;
    Initiator *initiator;

     SC_CTOR(Initiator_parent) : socket("socket") {
         initiator = new Initiator("initiator");
         // 자식의 initiator socket을 부모의 initiator socket에 계층적 binding
         initiator->socket.bind(socket);
     }
     ...
};

struct Target : sc_core::sc_module {
    tlm_utils::multi_passthrough_target_socket<Target> socket;

     SC_CTOR(Target) : socket("socket") {
         // 콜백 멤버 함수를 소켓에 등록
         socket.register_nb_transport_fw(this, &Target::nb_transport_fw);
         socket.register_b_transport(this, &Target::b_transport);
         socket.register_get_direct_mem_ptr(this, &Target::get_direct_mem_ptr);
         socket.register_transport_dbg(this, &Target::transport_dbg);
     }
     ...
};

// multi-socket을 가진 target 컴포넌트
struct Target_parent : sc_core::sc_module {
    tlm_utils::multi_passthrough_target_socket<Target_parent> socket;
    Target *target;

     SC_CTOR(Target_parent) : socket("socket") {
        target = new Target("target");
        // 부모의 target socket을 자식의 target socket에 계층적 binding
        socket.bind(target->socket);
     }
};

SC_MODULE(Top) {
   Initiator_parent *initiator1;
   Initiator_parent *initiator2;
   Target_parent *target1;
   Target_parent *target2;

     SC_CTOR(Top) {
        // initiator 2개, target 2개 인스턴스화
        initiator1 = new Initiator_parent("initiator1");
        initiator2 = new Initiator_parent("initiator2");
        target1 = new Target_parent("target1");
        target2 = new Target_parent("target2");

          // initiator multi-socket 2개를 target multi-socket 2개에 bind
          initiator1->socket.bind(target1->socket);
          initiator1->socket.bind(target2->socket);
          initiator2->socket.bind(target1->socket);
          initiator2->socket.bind(target2->socket);
     }
};
```

---

## 16.3 Quantum keeper

### 16.3.1 Description

Temporal decoupling은 SystemC 프로세스가 **time quantum**이라 불리는 시간만큼 simulation time보다 앞서 실행되도록 허용한다 (Clause 12).

유틸리티 클래스 `tlm_quantumkeeper`는 time quantum을 관리·상호작용하는 멤버 함수 집합을 제공한다.

- temporal decoupling 사용 시 **일관된 코딩 스타일 유지를 위해 quantum keeper 사용이 권장(recommended)**. 다만 원리상 SystemC에서 temporal decoupling을 직접 구현하는 것도 간단하다.
- **`tlm_quantumkeeper` 사용 여부와 무관하게, 모든 temporally decoupled 모델은 `tlm_global_quantum` 클래스가 유지하는 global quantum을 참조해야 should.**
- 클래스 `tlm_quantumkeeper`는 **`namespace tlm_utils`**에 있다.
- temporal decoupling 일반 설명은 §10.3.3, timing annotation은 §11.2.4 참조.

### 16.3.2 Header file

`tlm_utils/tlm_quantumkeeper.h` (shall).

### 16.3.3 Class definition

```cpp
namespace tlm_utils {

class tlm_quantumkeeper
{
public:
    static void set_global_quantum( const sc_core::sc_time& );
    static const sc_core::sc_time& get_global_quantum();

     tlm_quantumkeeper();
     virtual ~tlm_quantumkeeper();

     virtual void inc( const sc_core::sc_time& );
     virtual void set( const sc_core::sc_time& );
     virtual sc_core::sc_time get_current_time() const;
     virtual sc_core::sc_time get_local_time();

     virtual bool need_sync() const;
     virtual void sync();

     void set_and_sync(const sc_core::sc_time& t)
     {
         set(t);
         if (need_sync())
             sync();
     }

     virtual void reset();

protected:
    virtual sc_core::sc_time compute_local_quantum();
};

} // namespace tlm_utils
```

### 16.3.4 General guidelines for processes using temporal decoupling

- a) **최대 시뮬레이션 속도를 위해 모든 initiator는 temporal decoupling을 써야 should** 하고, 다른 runnable SystemC 프로세스 수는 0이거나 최소화되어야 should.
- b) 이상적 시나리오에서는 runnable SystemC 프로세스가 temporally decoupled initiator에만 속하며, 각 프로세스는 SystemC 커널에 양보하기 전에 자기 time quantum 끝까지 앞서 실행한다.
- c) 다른 프로세스와의 통신이 명시적으로 동기화된다면 temporally decoupled initiator가 time quantum을 쓸 의무는 없다. **time quantum을 쓴다면 initiator 간 전형적 통신 간격보다 작게 선택해야 should** — 그렇지 않으면 중요한 프로세스 상호작용이 소실되어 모델이 깨질 수 있다.
- d) *Yield* = thread process에서는 `wait` 호출, method process에서는 함수에서 return.
- e) temporal decoupling은 표준 SystemC 시뮬레이션 커널의 컨텍스트에서 돌아가므로, 이벤트 스케줄·프로세스 suspend/resume이 가능하고 loosely-timed 모델을 다른 코딩 스타일과 섞을 수 있다.
- f) 모든 initiator가 temporal decoupling을 쓸 의무는 없다. 쓰는 프로세스와 안 쓰는 프로세스를 섞을 수 있다. 다만 **temporal decoupling을 쓰지 않는 프로세스는 시뮬레이션 속도의 병목이 되기 쉽다**.
- g) 각 temporally decoupled initiator는 지역 처리 지연과 통신 지연을 **local time offset**이라 불리는 지역 변수에 누적해도 된다 may. **local time offset 유지에 quantum keeper 사용이 권장(recommended)**.
- h) **`sc_time_stamp` 호출은 현재 time quantum 시작 시점 또는 그 근처의 simulation time을 반환한다.**
- i) local time offset은 SystemC 스케줄러가 알지 못한다. transport 인터페이스 사용 시 **local time offset을 `b_transport` 또는 `nb_transport` 메서드의 인자로 넘겨야 should**.
- j) temporal decoupling + quantum keeper와 `nb_transport`를 함께 쓰는 것이 배제되지는 않지만, approximately-timed 코딩 스타일 고유의 높은 프로세스 간 통신량이 temporal decoupling의 속도 이점을 상쇄하므로 **보통 유리하지 않다**.
- k) quantum 안에서 프로세스가 재개되는 순서는 SystemC 스케줄러의 통제 하에 있으며 SystemC 규칙상 **indeterminate**. 명시적 동기화 메커니즘이 없다면, 한 프로세스가 수정하고 다른 프로세스가 읽는 변수의 **읽히는 값은 indeterminate**. 새 값은 현재 quantum에 보일 수도 다음 quantum에 보일 수도 있고, 애플리케이션은 새 값이 정확히 언제 보이는지에 관용적이어야 한다. 그렇지 않다면 **적절한 동기화 메커니즘으로 변수 접근을 보호해야 should**.
- l) temporally decoupled 프로세스에서 변수·객체에 접근하면, 현재 프로세스나 현재 quantum에서 이미 실행된 다른 temporally decoupled 프로세스가 수정하지 않은 한 **현재 time quantum 시작 시점의 값**을 준다. 특히 temporally decoupled 프로세스에서 접근한 **`sc_signal`은 현재 quantum 시작 시점과 같은 값**을 갖는다. 이는 (`sc_time_stamp`가 반환하는) 통상적 SystemC simulation time이 quantum 안에서 진행하지 않기 때문이다.

### 16.3.5 tlm_quantumkeeper — Rules

- a) **생성자는 local time offset을 `SC_ZERO_TIME`으로 설정해야 shall** 하지만 virtual 멤버 함수 `compute_local_quantum`을 **호출해서는 안 된다 shall not**. 생성자가 local quantum을 계산하지 않으므로, 애플리케이션은 **quantum keeper 객체 생성 직후 `reset`을 호출해야 should**.
- b) `tlm_quantumkeeper` 구현은 `sc_time` 클래스의 **static 객체를 생성해서는 안 되지만 shall not**, 생성자는 `sc_time` 객체를 생성해도 된다 may. 이는 애플리케이션이 **첫 quantum keeper 객체를 생성하기 전에만(before, and only before) `sc_core::sc_set_time_resolution`을 호출할 수 있음**을 함의한다.

| 멤버 함수 | 규칙 |
|---|---|
| `set_global_quantum` | global quantum 값을 인자 값으로 설정해야 shall. **local quantum은 수정하지 않아야 shall not.** 호출 후 local quantum 재계산을 위해 `reset` 호출 권장(recommended) |
| `get_global_quantum` | global quantum의 현재 값을 반환해야 shall |
| `get_local_time` | **local time offset** 값을 반환해야 shall |
| `get_current_time` | **effective local time** = `sc_time_stamp() + local_time_offset` 을 반환해야 shall |
| `inc` | 인자 값을 local time offset에 **더해야 shall** |
| `set` | local time offset을 인자 값으로 **설정해야 shall** |
| `need_sync` | **local time offset이 local quantum보다 클 때에만(if and only if) `true`를 반환해야 shall** |
| `sync` | `wait( local_time_offset )`를 호출하여 simulation time이 effective local time과 같아질 때까지 프로세스를 suspend해야 shall. 그 다음 `reset`을 호출해야 shall |
| `set_and_sync` | `set`, `need_sync`, `sync`를 차례로 호출하는 편의 메서드. **override하지 않아야 should not** |
| `reset` | `compute_local_quantum`을 호출해야 shall. local time offset을 `SC_ZERO_TIME`으로 되돌려야 shall |
| `compute_local_quantum` (protected) | `tlm_global_quantum` 클래스의 `compute_local_quantum`을 호출해야 shall. **더 작은 local quantum 값을 계산하도록 override해도 된다 may** |

- m) `tlm_quantumkeeper`는 quantum keeper의 **기본 구현으로 간주해야 should**. 애플리케이션이 `tlm_quantumkeeper`에서 자체 quantum keeper를 파생시켜 `compute_local_quantum`을 override해도 되지만 may, **이는 흔치 않은 일이다**.
- n) **local time offset이 local quantum 이상이면 프로세스는 커널에 yield해야 should.** `sync` 호출로 하는 것이 **강력히 권장(strongly recommended)**.
- o) **time quantum 끝에서 동기화를 강제하는 메커니즘은 없다. `need_sync`를 확인하고 필요 시 `sync`를 호출하는 것은 initiator의 책임이다.**
- p) **`b_transport`는 스스로 yield할 수 있어 호출 전후로 `sc_time_stamp` 값이 달라질 수 있다.** local time offset과 모든 timing annotation은 항상 현재 `sc_time_stamp` 값에 **상대적**으로 표현된다. **`b_transport` 또는 `nb_transport_fw`에서 반환된 후 quantum keeper의 local time offset을 `set`으로 설정하고 `need_sync`로 동기화 필요 여부를 확인하는 것은 initiator의 책임이다.**
- q) initiator가 time quantum 끝 이전에 동기화해야 한다면(즉 simulation time이 local time을 따라잡도록 실행을 suspend해야 한다면), **`sync` 호출 또는 명시적 이벤트 대기**로 할 수 있다 may. 이로써 다른 프로세스가 실행될 기회를 얻으며, 이를 **synchronization-on-demand**라 한다.
- r) **`sync`를 자주 호출하면 temporal decoupling의 효과가 감소한다.**

### Example (informative)

```cpp
struct Initiator : sc_core::sc_module {                // Loosely-timed initiator
    tlm_utils::simple_initiator_socket<Initiator> init_socket;
    tlm_utils::tlm_quantumkeeper m_qk;                 // quantum keeper

     SC_CTOR(Initiator) : init_socket("init_socket") {
        SC_THREAD(thread);                             // initiator 프로세스
        ...
        m_qk.set_global_quantum(sc_core::sc_time(1.0, sc_core::SC_US)); // global quantum 교체
        m_qk.reset();                                  // local quantum 재계산
     }

     void thread() {
         tlm::tlm_generic_payload trans;
         sc_core::sc_time delay;
         trans.set_command(tlm::TLM_WRITE_COMMAND);
         trans.set_data_length(4);

           for (int i = 0; i < RUN_LENGTH; i += 4) {
           int word = i;
           trans.set_address(i);
           trans.set_data_ptr((unsigned char *)(&word));

               delay = m_qk.get_local_time();          // local time으로 b_transport annotate
               init_socket->b_transport(trans, delay);
               qk.set(delay);                          // target이 소비한 시간으로 qk 갱신

               m_qk.inc(sc_core::sc_time(100.0, sc_core::SC_NS)); // initiator가 소비한 추가 시간
               if (m_qk.need_sync())
                   m_qk.sync();                        // local time을 quantum과 비교
           }
     }
     ...
};
```

---

## 16.4 Payload event queue (PEQ)

### 16.4.1 Description

PEQ는 **SystemC 이벤트 알림의 큐를 유지하는 클래스**로, 각 알림이 연관된 트랜잭션 객체를 나른다. 각 트랜잭션은 delay가 annotate되어 PEQ에 쓰이고, **현재 simulation time + annotate된 delay** 시점에 PEQ 뒤쪽에서 나온다.

- 유틸리티로 **PEQ 두 종류**가 제공된다. 그 자체로 유용할 뿐 아니라, approximately-timed 코딩 스타일에서 timing annotation의 의미를 이해하는 데 개념적으로 중요하다. 다만 **여기 제공된 PEQ 없이도 approximately-timed 모델 구현은 가능하다.**
- approximately-timed 모델에서는 `nb_transport`로 전달받은 트랜잭션을 annotate된 delay와 함께 PEQ에 넣는 것이 적절한 경우가 많다. PEQ가 `nb_transport` 호출에 연관된 timing point를 올바른 simulation time에 스케줄해 준다.
- 트랜잭션은 멤버 함수 `notify`에 delay를 넘겨 삽입한다. **즉시 알림(immediate notification)을 스케줄하는 `notify` 오버로드도 있다.** delay는 현재 simulation time(`sc_time_stamp`)에 더해져 트랜잭션이 PEQ 뒤에서 나올 시각을 계산한다. 이벤트 스케줄링은 내부적으로 SystemC timed event notification으로 관리되며, `sc_event`의 성질 — 알림이 pending인 상태에서 `notify`가 호출되면 **가장 이른 simulation time의 알림이 남고 다른 알림은 취소된다** — 을 활용한다.
- **`peq_with_get`** — 멤버 함수 `get_event`가 트랜잭션을 꺼낼 준비가 될 때마다 notify되는 이벤트를 반환. **`get_next_transaction`을 반복 호출**하여 가용 트랜잭션을 하나씩 꺼내야 should.
- **`peq_with_cb_and_phase`** — 콜백 멤버 함수를 생성자 인자로 등록하고, 각 트랜잭션이 나올 때 그 함수가 호출됨. 이 PEQ는 각 알림에 **트랜잭션 객체와 phase 객체를 모두** 실어 나르며, 둘 다 콜백 인자로 전달된다. (예: §15.1)

### 16.4.2 Header file

`tlm_utils/peq_with_get.h`, `tlm_utils/peq_with_cb_and_phase.h` (shall).

### 16.4.3 Class definition

```cpp
namespace tlm_utils {

template <class PAYLOAD>
class peq_with_get : public sc_core::sc_object
{
public:
    typedef PAYLOAD transaction_type;

      peq_with_get(const char* name);

      void notify( transaction_type& trans, const sc_core::sc_time& t );
      void notify( transaction_type& trans );

      transaction_type* get_next_transaction();
      sc_core::sc_event& get_event();
      void cancel_all();
};

template<typename OWNER, typename TYPES=tlm::tlm_base_protocol_types>
class peq_with_cb_and_phase : public sc_core::sc_object
{
public:
    typedef typename TYPES::tlm_payload_type tlm_payload_type;
    typedef typename TYPES::tlm_phase_type       tlm_phase_type;
    typedef void (OWNER::*cb)(tlm_payload_type&, const tlm_phase_type&);

      peq_with_cb_and_phase(OWNER* , cb );
      peq_with_cb_and_phase(const char* , OWNER* , cb);
      ~peq_with_cb_and_phase();

      void notify ( tlm_payload_type& , const tlm_phase_type& , const sc_core::sc_time& );
      void notify ( tlm_payload_type& , const tlm_phase_type& );
      void cancel_all();
};

} // namespace tlm_utils
```

### 16.4.4 Rules

공통:

- a) `notify`는 트랜잭션을 PEQ에 삽입해야 shall. 트랜잭션은 **`t1 + t2` 시각에 나와야 shall** — `t1`은 `notify` 호출 시점의 `sc_time_stamp()` 반환값, `t2`는 `notify`의 `sc_time` 인자 값. **즉시 알림의 경우 트랜잭션은 SystemC 스케줄러의 현재 evaluation phase에 나와야 shall.**
- b) 트랜잭션은 **어떤 순서로든 큐에 넣을 수 있고**, 위 규칙이 주는 순서로 나온다. **삽입 순서대로 나오는 것은 아니다.**
- c) **PEQ에 동시에 들어갈 수 있는 트랜잭션 수에 제한은 없다.**
- d) 여러 트랜잭션이 같은 시각에 나오도록 큐잉되면, **모두 같은 evaluation phase(같은 delta cycle)에서, 삽입된 순서로 나와야 shall**.
- e) **`cancel_all`은 큐에 있는 모든 트랜잭션을 즉시 제거해야 shall** — 생성 직후 상태로 되돌린다. **이것이 PEQ에서 트랜잭션을 제거하는 유일한 방법이다.**

`peq_with_get`:

- f) `PAYLOAD` 템플릿 인자는 PEQ가 쓰는 **트랜잭션 타입의 이름이어야 shall**.
- g) `get_event`는 **다음 트랜잭션이 나올 준비가 되었을 때 notify되는 이벤트에 대한 참조를 반환해야 shall**. 같은 evaluation phase(같은 delta cycle)에 여러 트랜잭션이 나올 준비가 되면 **이벤트는 한 번만 notify된다**.
- h) `get_next_transaction`은 나올 준비가 된 트랜잭션 객체의 포인터를 반환해야 shall 하며, 그 객체를 **PEQ에서 제거해야 shall**. 대응 이벤트 알림이 일어난 evaluation phase에 트랜잭션을 회수하지 않아도, **현재 시각이나 이후 시각의 후속 `get_next_transaction` 호출로 여전히 회수 가능**.
- i) 현재 evaluation phase에 회수할 트랜잭션이 더 없으면 `get_next_transaction`은 **null pointer를 반환해야 shall**.

`peq_with_cb_and_phase`:

- j) `TYPES` 템플릿 인자는 PEQ가 쓰는 트랜잭션·phase 타입을 담은 **protocol traits class의 이름이어야 shall**.
- k) `OWNER` 템플릿 인자는 PEQ 콜백 멤버 함수가 속한 **클래스의 타입이어야 shall**. 보통 PEQ 인스턴스의 부모 모듈.
- l) 생성자의 `OWNER*` 인자는 PEQ 콜백 멤버 함수가 속한 **객체에 대한 포인터여야 shall**. 보통 PEQ 인스턴스의 부모 모듈.
- m) 생성자의 `cb` 인자는 PEQ 콜백 멤버 함수의 이름이어야 shall 하며, **멤버 함수여야 shall**.
- n) 구현은 트랜잭션 객체가 나올 준비가 될 때마다 PEQ 콜백 멤버 함수를 호출해야 shall. **콜백의 첫 인자는 트랜잭션 객체에 대한 참조, 둘째 인자는 phase 객체에 대한 참조** — 대응하는 `notify`에 전달된 것.
- o) **구현은 PEQ 콜백 멤버 함수를 SystemC method process에서 호출해야 shall. 따라서 콜백 멤버 함수는 non-blocking이어야 shall** (`wait` 불가).
- p) 구현은 **각 트랜잭션마다 PEQ 콜백 멤버 함수를 한 번만 호출해야 shall**. 콜백 호출 후 구현은 그 트랜잭션 객체를 **PEQ에서 제거해야 shall**. PEQ 콜백 멤버 함수는 같은 evaluation phase에 여러 번 호출될 수 있다.

---

## 16.5 Instance-specific extensions

### 16.5.1 Description

generic payload는 extension 객체 포인터 배열을 가지므로 **각 트랜잭션 객체는 extension 타입마다 최대 하나의 인스턴스만 담을 수 있다.** 이 메커니즘만으로는 같은 extension을 한 트랜잭션 객체에 여러 개 추가할 수 없다. §16.5는 **instance-specific extension** — 같은 타입의 extension을 한 트랜잭션 객체에 여러 개 추가하는 것 — 을 제공하는 유틸리티를 설명한다.

Rules (Introduction 본문에 규범적으로 기술됨):

- instance-specific extension 타입은 클래스 템플릿 `instance_specific_extension`으로 만들며, `tlm_extension`과 유사하게 사용한다.
- **`tlm_extension`과 달리 애플리케이션은 virtual 멤버 함수 `clone`과 `copy_from`을 구현할 필요가 없고, 구현하는 것이 허용되지도 않는다(not required or permitted).**
- **접근 메서드는 `set_extension`, `get_extension`, `clear_extension`, `resize_extensions`로 제한된다.**
- **instance-specific extension의 자동 삭제는 지원되지 않는다. 따라서 `set_extension`을 호출한 컴포넌트는 `clear_extension`도 호출해야 should.**
- `tlm_extension`과 마찬가지로 **`resize_extensions`는 트랜잭션 객체가 static initialization 중에 생성되는 경우에만 호출하면 된다.**
- instance-specific extension은 **`instance_specific_extension_accessor` 타입 객체를 통해 접근**한다. 이 클래스는 접근 메서드를 호출할 수 있는 **proxy 객체를 반환하는 `operator()`**를 제공한다.
- **`instance_specific_extension_accessor` 타입의 각 객체는 같은 트랜잭션 객체에 대해서도 서로 구별되는 extension 객체 집합에 접근한다.**
- §16.5.3 클래스 정의에서 **이탤릭 항목은 implementation-defined 이름이며 애플리케이션이 직접 사용해서는 안 된다(should not).**

### 16.5.2 Header file

`tlm_utils/instance_specific_extensions.h` (shall).

### 16.5.3 Class definition

```cpp
namespace tlm_utils {

template <typename T>
class instance_specific_extension : public implementation-defined {
public:
    virtual ~instance_specific_extension();
};

template<typename U>
class proxy {
public:
    template <typename T> T* set_extension( T* );
    template <typename T> void get_extension( T*& ) const;
    template <typename T> void clear_extension( const T* );
    void resize_extensions();
};

class instance_specific_extension_accessor {
public:
    instance_specific_extension_accessor();

     template<typename T> proxy< implementation-defined >& operator() ( T& );
};

} // namespace tlm_utils
```

### Example (informative)

```cpp
struct my_extn : tlm_utils::instance_specific_extension<my_extn> {
    int num;                                     // 사용자 정의 extension attribute
};

struct Interconnect : sc_core::sc_module {
     tlm_utils::simple_target_socket<Interconnect> targ_socket;
     tlm_utils::simple_initiator_socket<Interconnect> init_socket;
     ...
     tlm_utils::instance_specific_extension_accessor accessor;
     static int count;

     virtual tlm::tlm_sync_enum nb_transport_fw(
              tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {
          my_extn *extn;
          accessor(trans).get_extension(extn);         // 기존 extension 획득
          if (extn) {
              accessor(trans).clear_extension(extn);   // 기존 extension 삭제
          } else {
              extn = new my_extn;
              extn->num = count++;
              accessor(trans).set_extension(extn);     // 새 extension 추가
          }
          return init_socket->nb_transport_fw(trans, phase, delay);
     }
     ...
};

SC_MODULE(Top) {
...
    SC_CTOR(Top) {
       // 트랜잭션 객체가 Interconnect 인스턴스 두 개를 통과
       interconnect1 = new Interconnect("interconnect1");
       interconnect2 = new Interconnect("interconnect2");
       interconnect1->init_socket.bind(interconnect2->targ_socket);
       ...
    }
};
```

---

## 흔한 위반 (pitfalls)

- **simple target socket에 `b_transport`/`nb_transport_fw` 중 어느 것도 등록하지 않음** — §16.2.2.4 g): 대응 함수가 호출되면 **run-time error**. 최소 하나는 등록할 것.
- **`passthrough_target_socket`에 b/nb 자동 변환을 기대** — §16.2.2.1, §16.2.2.4 h): passthrough는 변환을 지원하지 않는다. 둘 다 등록해야 should.
- **b/nb 변환 중 `b_transport`가 진행 중인데 `nb_transport_fw` 호출** — §16.2.2.5 d): target이 `nb_transport_fw`만 등록한 경우 허용되지 않는다 (현재 구현의 limitation).
- **b/nb 변환을 base protocol 아닌 프로토콜에서 사용** — §16.2.2.1: 변환은 `tlm_phase_enum`과 base protocol 규칙에 의존한다. 다른 프로토콜이면 자체 convenience socket을 만들어야 한다.
- **tagged socket 콜백 시그니처에서 id 위치 혼동** — §16.2.3.4 d): **등록 함수에서는 마지막 인자, 콜백 함수에서는 첫 번째 인자**.
- **tagged simple socket을 multi-socket처럼 사용** — §16.2.3.4 e): tagged simple socket은 multi-socket이 아니며 여러 소켓에 bind할 수 없다.
- **`multi_passthrough_target_socket`에서 `target.bind(initiator)` 시도** — §16.2.4.4 g): multi target socket에는 reverse binding operator가 **없다**. binding은 initiator→target 방향뿐.
- **multi-socket을 여러 소켓에 계층적으로 bind** — §16.2.4.4 e): 계층적 binding은 같은 종류의 multi-socket **정확히 하나**에만 가능.
- **파생 클래스에서 `before_end_of_elaboration`/`end_of_elaboration` 미호출** — §16.2.4.3: multi-socket 파생 클래스는 base의 해당 콜백을 반드시 호출해야 한다(must).
- **`end_of_elaboration` 이전에 `size()` 호출** — §16.2.4.4 k): 반환값이 **implementation-defined**.
- **quantum keeper 생성 후 `reset()` 미호출** — §16.3.5 a): 생성자는 `compute_local_quantum`을 호출하지 않는다. 생성 직후 `reset` 호출 권장.
- **`set_global_quantum` 후 `reset()` 미호출** — §16.3.5 c): global quantum 설정은 local quantum을 갱신하지 않는다.
- **첫 quantum keeper 생성 후 `sc_set_time_resolution` 호출** — §16.3.5 b): 첫 객체 생성 **이전에만** 호출 가능.
- **동기화가 자동으로 될 것이라 기대** — §16.3.5 o): quantum 끝에서 동기화를 강제하는 메커니즘은 없다. `need_sync` 확인 후 `sync` 호출은 initiator의 책임.
- **`b_transport` 반환 후 local time offset 갱신 누락** — §16.3.5 p): `b_transport`는 yield할 수 있어 `sc_time_stamp`가 바뀐다. 반환된 delay로 `set`을 호출하고 `need_sync`를 확인할 것.
- **temporally decoupled 프로세스에서 `sc_signal` 값이 즉시 갱신될 것이라 기대** — §16.3.4 l): quantum 시작 시점의 값을 준다.
- **PEQ 삽입 순서대로 나올 것이라 기대** — §16.4.4 b): 나오는 순서는 `sc_time_stamp() + t` 기준이다. 같은 시각이면 삽입 순서 (§16.4.4 d)).
- **`peq_with_cb_and_phase` 콜백에서 `wait` 호출** — §16.4.4 o): 콜백은 method process에서 호출되므로 **non-blocking이어야 shall**.
- **PEQ에서 트랜잭션을 개별 제거하려 시도** — §16.4.4 e): `cancel_all`(전체 제거)이 유일한 제거 방법. (`peq_with_get`의 `get_next_transaction`은 회수 시 제거.)
- **instance-specific extension에 `clone`/`copy_from` 구현** — §16.5.1: 필요하지도 않고 **허용되지도 않는다**.
- **`set_extension` 후 `clear_extension` 누락** — §16.5.1: instance-specific extension은 **자동 삭제되지 않는다**.
