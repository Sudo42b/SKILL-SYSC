# Clause 14 — TLM-2.0 generic payload

LRM pp. 494–529.

TLM-2.0 core interface를 통해 전달되는 **transaction object의 표준 타입**. base protocol(15.2)과 밀접하게 연관된다.
메모리 맵드 버스 모델의 interoperability를 담당하는 핵심 클래스. **모든 attribute의 default·modifiability 규칙이 여기에 있다.**

---

## 14.1 Introduction

- generic payload는 두 수준에서 interoperability를 개선한다.
  1. **즉시 사용 가능한 범용 payload** — 버스 프로토콜 세부가 중요하지 않은 추상 모델에서 바로 상호운용.
     동시에 **ignorable extension 메커니즘** 제공.
  2. **특정 버스 프로토콜 상세 모델의 기반** — 구현 비용을 줄이고, 프로토콜 간 bridge/adapt 시 시뮬레이션 속도를 높인다.
- 포함하는 attribute: command, address, data, byte enables, single word transfers, burst transfers, streaming, response status.
- 메모리 맵드 버스가 아닌 프로토콜의 모델링 기반으로 써도 된다 may.
- 전형적 버스 프로토콜의 모든 attribute를 포함하지는 않지만, **extension 메커니즘**으로 애플리케이션이 자체 attribute를 추가할 수 있다.
- **initiator/target socket과 함께 쓰는 것이 권장(recommended)** — socket은 data array 해석에 쓰이는 bus width 파라미터,
  forward/backward path, 프로토콜 간 강한 타입 체크 메커니즘을 제공한다.
- blocking·non-blocking transport interface 양쪽에 쓸 수 있고, direct memory·debug transport interface에도 쓸 수 있다
  (이 경우 **제한된 attribute 집합만** 사용).

---

## 14.2 Extensions and interoperability

### 14.2.1 Overview

- extension 메커니즘은 **임의 개수·임의 타입의 extension**을 transaction object에 추가할 수 있게 한다.
  extension은 transaction과 함께 운반되는 새 attribute 집합.
- extension은 **initiator, interconnect, target 어느 쪽이든** 생성·추가·쓰기·읽기 가능. 메커니즘 자체는 제약을 두지 않는다.
  다만 무절제한 사용은 interoperability를 해치므로 **절제된 사용이 강력히 권장(strongly encouraged)**.

transport interface의 `TRANS` 템플릿 인자 / combined interface의 `TYPES` 템플릿 인자에 대해
**권장되는 대안은 정확히 셋뿐이다** (interoperability 내림차순):

| # | 방식 | 준수 등급 |
|---|---|---|
| a) | generic payload를 직접 사용 + ignorable extension + base protocol 규칙 준수 | **TLM-2.0 base-protocol-compliant** (9.2) |
| b) | `tlm_generic_payload`의 typedef를 담은 **새 protocol traits class** 정의 | **TLM-2.0 custom-protocol-compliant** (9.2) |
| c) | 새 protocol traits class + **새 transaction type** 정의 | base-protocol-compliant도 custom-protocol-compliant도 **아님** |

- generic payload에서 **새 class를 파생시키는 것은 가능하지만 interoperability 관점에서 권장 방식이 아니다**.
- 세 옵션은 하나의 시스템 모델에서 **혼용해도 된다**. 특히 a)와 b)의 혼용은 extension 메커니즘이 효율적 상호운용을
  위해 설계되었으므로 가치가 있다.

### 14.2.2 generic payload 직접 사용 + ignorable extension

- a) transaction type = `tlm_generic_payload`, phase type = `tlm_phase`, traits class = `tlm_base_protocol_types`
  (각각 `TRANS`, `TYPES`의 기본값). 표준 socket + base protocol을 쓰는 모델은 generic payload와 base protocol의
  semantics를 지키는 한 서로 interoperable하다.
- b) 이 경우 모든 generic payload extension과 extended phase는 **ignorable해야 shall**.
  **ignorable = 해당 extension을 추가한 컴포넌트 이외의 어떤 컴포넌트도 그 extension이 없는 것처럼 동작해도 된다** (14.21.1.2).
- c) ignorable extension은 정의상 target에서의 지원 여부를 **컴파일 타임에 체크하지 않으며**, 메커니즘도 이를 지원하지 않는다.
- d) generic payload는 프로토콜의 사소한 변형을 본질적으로 지원한다. 일반 원칙으로 **target은 generic payload의 모든
  기능을 지원하도록 권장(recommended)**되지만, 예를 들어 byte enable을 지원하지 않을 수 있다.
  **특정 기능을 지원할 수 없는 target은 standard error response를 생성할 의무가 있다**(14.17.2).

### 14.2.3 `tlm_generic_payload`의 typedef를 담은 새 protocol traits class 정의

- a) transaction type은 `tlm_generic_payload`, phase type은 `tlm_phase`이지만 traits class는 사용자 정의.
  **확장된 generic payload가 별개 타입으로 취급되어 socket 바인딩 시 컴파일 타임 타입 체크를 제공**한다.
- b) 새 protocol type은 **자체 규칙을 정해도 되고 may**, 그 규칙은 base protocol의 규칙 —
  **memory management 규칙(14.5)과 attribute modifiability 규칙(14.7)을 포함해** — 을 확장하거나 모순되어도 된다.
  다만 일관성·interoperability를 위해 **가능한 한 base protocol의 규칙과 코딩 스타일을 따르는 것이 권장(recommended)**.
- c) extension 메커니즘을 ignorable / non-ignorable / mandatory 어느 용도로든 제약 없이 써도 된다 may.
  extension의 semantics는 새 traits class와 함께 **철저히 문서화되어야 권장(should)**.
- d) transaction type이 `tlm_generic_payload`이므로 generic payload를 쓰는 interconnect·target을 통과할 수 있고,
  extension을 포함해 **통째로 clone 가능**하다.
- e) 하나의 traits class를 initiator→interconnect→target 경로 전체에 쓰는 것이 보통. 다만 한 프로토콜 타입의
  incoming transaction을 다른 타입의 outgoing transaction으로 변환하는 adapter/bus bridge를 interconnect로 모델링하는 것이
  가능할 수도 있다. incoming에 대해 target, outgoing에 대해 initiator로 동작하는 **transaction bridge**도 만들 수 있다.
- f) 서로 다른 traits class로 특화된 socket 사이로 generic payload transaction을 넘길 때는
  **각 extension의 semantics를 매우 신중히 검토할 의무가 사용자에게 있다**. 일반 규칙은 없다.

### 14.2.4 새 protocol traits class + 새 transaction type 정의

- a) transaction type이 generic payload와 무관해도 된다 may.
- b) combined interface와 socket을 파라미터화할 새 protocol traits class가 필요하다.
- c) 새 transaction type이 generic payload와 크게 다르거나 매우 특수한 프로토콜을 나타낼 때 정당화될 수 있다.
- d) 최대 interoperability가 목표라면 새 class 정의보다 **앞의 두 방식이 권장(recommended)**.

---

## 14.3 Generic payload attributes and member functions

- generic payload class는 **private attribute 집합**과 그 값을 get/set하는 **public access function 집합**을 갖는다.
  **access function의 정확한 구현은 implementation-defined.**
- 대부분의 attribute는 **initiator가 설정하며, 어떤 interconnect나 target도 수정하면 안 된다 shall not**.
- **interconnect나 target이 수정해도 되는 것은 address, DMI allowed, response status, extension attribute 뿐**.
  단, **read command의 경우 target은 data array도 수정해도 된다 may**.

---

## 14.4 Class definition

```cpp
namespace tlm {

class tlm_generic_payload;

class tlm_mm_interface {
public:
    virtual void free(tlm_generic_payload*) = 0;
    virtual ~tlm_mm_interface() {}
};

unsigned int max_num_extensions();

class tlm_extension_base
{
public:
    virtual tlm_extension_base* clone() const = 0;
    virtual void free() { delete this; }
    virtual void copy_from(tlm_extension_base const &) = 0;
protected:
    virtual ~tlm_extension_base() {}
};

template <typename T>
class tlm_extension : public tlm_extension_base
{
public:
    virtual tlm_extension_base* clone() const = 0;
    virtual void copy_from(tlm_extension_base const &) = 0;
    virtual ~tlm_extension() {}
    const static unsigned int ID;
};

enum tlm_gp_option {
   TLM_MIN_PAYLOAD,
   TLM_FULL_PAYLOAD,
   TLM_FULL_PAYLOAD_ACCEPTED
};

enum tlm_command {
   TLM_READ_COMMAND,
   TLM_WRITE_COMMAND,
   TLM_IGNORE_COMMAND
};

enum tlm_response_status {
    TLM_OK_RESPONSE = 1,
    TLM_INCOMPLETE_RESPONSE = 0,
    TLM_GENERIC_ERROR_RESPONSE = -1,
    TLM_ADDRESS_ERROR_RESPONSE = -2,
    TLM_COMMAND_ERROR_RESPONSE = -3,
    TLM_BURST_ERROR_RESPONSE = -4,
    TLM_BYTE_ENABLE_ERROR_RESPONSE = -5
 };

#define TLM_BYTE_DISABLED 0x0
#define TLM_BYTE_ENABLED 0xff

class tlm_generic_payload {
public:
    // Constructors and destructor
    tlm_generic_payload();
    explicit tlm_generic_payload( tlm_mm_interface* );
    virtual ~tlm_generic_payload();

private:
    // Disable copy constructor and assignment operator
    tlm_generic_payload( const tlm_generic_payload& );
    tlm_generic_payload& operator= ( const tlm_generic_payload& );

public:
   // Memory management
   void set_mm( tlm_mm_interface* );
   bool has_mm() const;
   void acquire();
   void release();
   int get_ref_count() const;
   void reset();
   void deep_copy_from( const tlm_generic_payload & );
   void update_original_from( const tlm_generic_payload & , bool use_byte_enable_on_read = true );
   void update_extensions_from( const tlm_generic_payload & );
   void free_all_extensions();

    // Access member functions
    tlm_gp_option get_gp_option() const;
    void set_gp_option( const tlm_gp_option );

    tlm_command get_command() const;
    void set_command( const tlm_command );
    bool is_read();
    void set_read();
    bool is_write();
    void set_write();

    sc_dt::uint64 get_address() const;
    void set_address( const sc_dt::uint64 );

    unsigned char* get_data_ptr() const;
    void set_data_ptr( unsigned char* );

    unsigned int get_data_length() const;
    void set_data_length( const unsigned int );

    unsigned int get_streaming_width() const;
    void set_streaming_width( const unsigned int );

    unsigned char* get_byte_enable_ptr() const;
    void set_byte_enable_ptr( unsigned char* );
    unsigned int get_byte_enable_length() const;
    void set_byte_enable_length( const unsigned int );

    // DMI hint
    void set_dmi_allowed( bool );
    bool is_dmi_allowed() const;

    tlm_response_status get_response_status() const;
    void set_response_status( const tlm_response_status );
    std::string get_response_string();
    bool is_response_ok();
    bool is_response_error();

    // Extension mechanism
    template <typename T> T* set_extension( T* );
    tlm_extension_base* set_extension( unsigned int , tlm_extension_base* );

    template <typename T> T* set_auto_extension( T* );
    tlm_extension_base* set_auto_extension( unsigned int , tlm_extension_base* );

    template <typename T> void get_extension( T*& ) const;
    template <typename T> T* get_extension() const;
    tlm_extension_base* get_extension( unsigned int ) const;

    template <typename T> void clear_extension( const T* );
    template <typename T> void clear_extension();

    template <typename T> void release_extension( T* );
    template <typename T> void release_extension();

    void resize_extensions();
};

} // namespace tlm
```

**Disabled 멤버**: copy constructor `tlm_generic_payload(const tlm_generic_payload&)`와
assignment operator `operator=`는 **private으로 비활성화**되어 있다 (14.6 c).

### `tlm_command` 값

| 값 | 의미 |
|---|---|
| `TLM_READ_COMMAND` | read command. target은 자신의 local array 내용을 data pointer가 가리키는 array로 복사해야 shall |
| `TLM_WRITE_COMMAND` | write command. target은 data pointer가 가리키는 array 내용을 local array로 복사해야 shall |
| `TLM_IGNORE_COMMAND` | ignore command = **null command**. read/write를 실행하지 않고 extension만 운반하는 용도 (기본값) |

### `tlm_response_status` 값

| 값 | 수치 | 의미 (Table 56) |
|---|---|---|
| `TLM_OK_RESPONSE` | 1 | target이 command를 성공적으로 실행함 |
| `TLM_INCOMPLETE_RESPONSE` | 0 | Target did not attempt to execute the command (**기본값**) |
| `TLM_GENERIC_ERROR_RESPONSE` | -1 | Any other error |
| `TLM_ADDRESS_ERROR_RESPONSE` | -2 | address attribute로 동작할 수 없었거나 address out-of-range |
| `TLM_COMMAND_ERROR_RESPONSE` | -3 | command를 실행할 수 없었음 |
| `TLM_BURST_ERROR_RESPONSE` | -4 | data length 또는 streaming width로 동작할 수 없었음 |
| `TLM_BYTE_ENABLE_ERROR_RESPONSE` | -5 | byte enable로 동작할 수 없었음 |

### `tlm_gp_option` 값

`TLM_MIN_PAYLOAD` (기본값) / `TLM_FULL_PAYLOAD` / `TLM_FULL_PAYLOAD_ACCEPTED` — 14.8 참조.

### byte enable 매크로

`#define TLM_BYTE_DISABLED 0x0` / `#define TLM_BYTE_ENABLED 0xff`

---

## 14.5 Generic payload memory management

### 저장소 소유권 (a, b)

- a) **initiator가 data pointer와 byte enable pointer attribute를 기존 저장소(static / automatic(stack) / dynamic(new))로
  설정할 책임이 있다 shall**. initiator는 **transaction의 lifetime이 끝나기 전에 이 저장소를 delete하면 안 된다 shall not**.
  **generic payload destructor는 이 두 array를 delete하지 않는다.**
- b) 이 절은 extension 규칙(14.21)과 함께 읽어야 한다.

### 두 가지 접근법 (c~e)

- c) generic payload는 **두 가지 별개 접근법**을 지원한다: **memory manager를 쓰는 reference counting**과
  **initiator의 ad hoc memory management**. 두 방식은 결합해도 된다.
  어떤 방식이든 **transaction object 자체와 그 extension 양쪽을 관리해야 권장(should)**.
- d) `tlm_generic_payload`의 생성·소멸은 extension array 구현 때문에 **CPU time 관점에서 비싸다고 예상된다**.
  따라서 **반복적 생성·소멸을 피해야 권장(should)**. 두 가지 권장 전략:
  transaction pool을 구현한 memory manager를 쓰거나, ad hoc이라면 **동일한 generic payload object를
  연속된 `b_transport` 호출에 재사용**(크기 1인 transaction pool). transport 호출마다 생성·소멸하는 것은
  **엄청나게 느리므로 피해야 권장(should)**.
- e) **memory manager는 `tlm_mm_interface`의 멤버 함수 `free`를 최소한 구현한 사용자 정의 class**.
  의도: pool에서 transaction object를 할당하는 메서드 제공, `free`는 pool로 반환, destructor는 pool 전체를 delete.
  **`free`는 `tlm_generic_payload::release`가 reference count가 0에 도달할 때 호출한다.**
  `tlm_mm_interface::free`는 보통 **`tlm_generic_payload::reset`을 호출**해 자동 삭제 표시된 extension을 지운다.

### 언제 memory manager가 필요한가 (f~l)

| 규칙 | 내용 |
|---|---|
| f) | `set_mm`, `acquire`, `release`, `get_ref_count`, `reset`은 **memory manager가 있을 때만 사용해야 shall**. 기본적으로 generic payload object에는 memory manager가 설정되어 있지 않다. |
| g) | ad hoc 방식은 initiator가 core interface 호출 **전**에 transaction object 메모리를 할당하고, 호출 **후**에 transaction object와 extension object를 delete하거나 pool로 돌려야 한다. |
| h) | **blocking transport / DMI / debug transport**와 함께 쓸 때는 **어느 방식이든 된다 may**. ad hoc으로 충분. memory manager가 없으면 `b_transport`·`get_direct_mem_ptr`·`transport_dbg`는 **반환 시 transaction과 extension이 무효화·삭제된다고 가정해야 권장(should)**. |
| i) | **non-blocking transport와 함께 쓸 때는 memory manager를 반드시 써야 shall.** `nb_transport`에 인자로 넘기는 transaction object는 **이미 memory manager가 설정되어 있어야 shall**. 호출자가 initiator든 interconnect든 target이든 동일. |
| j) | **blocking-to-non-blocking transport adapter**는 memory manager가 없으면 **설정해야 shall**, 그리고 호출자에게 제어를 반환하기 전에 **그 memory manager를 제거해야 shall**. reference count가 0으로 돌아오기 전에는 제거할 수 없으므로 구현상 memory manager의 `free`가 transaction object를 delete하지 않아야 한다. `simple_target_socket`이 그런 adapter의 예. |
| k) | memory manager를 쓸 때 transaction object와 extension object는 **heap에서 할당되어야 shall** (궁극적으로 `new` 또는 `malloc`). |
| l) | ad hoc일 때는 heap이든 stack이든 할당해도 된다 may. **stack 할당 시 memory leak과 segmentation fault를 피하려면 extension object의 memory management에 특히 주의**가 필요하다. |

### `set_mm` / `has_mm` (m~o)

- m) `set_mm`은 generic payload object의 memory manager를 인자로 넘어온 주소의 객체로 **설정해야 shall**.
  인자는 **null이어도 된다 may** — 이 경우 기존 memory manager가 transaction object에서 제거되지만
  memory manager 자체가 delete되지는 않는다.
  **`set_mm`은 이미 memory manager가 있고 reference count가 0보다 큰 transaction object에 대해 호출하면 안 된다 shall not.**
- n) `has_mm`은 memory manager가 설정되어 있을 때 **그리고 그때만 true를 반환해야 shall**.
  `nb_transport` 메서드 본문에서 호출하면 **true를 반환해야 권장(should)**.
- o) `b_transport`·`get_direct_mem_ptr`·`transport_dbg` 본문에서 호출하면 **true든 false든 반환해도 된다 may**.
  interconnect는 `has_mm`을 호출해 적절히 대응해도 된다.
  그렇게 하지 않는다면 **memory manager가 있는 transaction의 모든 의무(예: heap 할당)를 떠맡되 shall,
  memory manager 존재를 요구하는 멤버 함수(예: `acquire`)를 호출해서는 안 된다 shall not**.

### reference counting (p~x)

| 함수 | 규칙 |
|---|---|
| reference count | 각 generic payload object가 하나씩 가짐. **기본값 0**. |
| `acquire()` | reference count를 **증가시켜야 shall**. **memory manager 없이 호출하면 run-time error 발생.** |
| `release()` | reference count를 **감소시켜야 shall**. 그 결과 0이 되면 memory manager 객체의 `free`를 transaction object 주소를 인자로 **호출해야 shall**. **memory manager 없이 호출하면 run-time error 발생.** |
| `get_ref_count()` | reference count 값을 **반환해야 shall**. memory manager가 없으면 반환값은 0. |

- t) memory manager가 있을 때 각 initiator는 transaction object를 처음 IMC 인자로 넘기기 전에 `acquire`를,
  더 이상 필요 없을 때 `release`를 **호출해야 권장(should)**.
- u) 각 interconnect·target은 transaction object의 lifetime을 현재 IMC를 넘어 연장할 필요가 있을 때마다 `acquire`,
  더 이상 필요 없을 때 `release`를 **호출해야 권장(should)**.
- v) 컴포넌트는 **어떤 IMC나 프로세스에서든 `release`를 호출해도 된다 may**. 따라서 **`acquire`를 미리 호출하지 않았다면
  IMC 호출 후나 제어를 양보한 후에 transaction object가 여전히 유효하다고 가정할 수 없다.**
  예: initiator가 `nb_transport_bw` 구현에서, target이 `nb_transport_fw` 구현에서 `release`를 호출할 수 있다.
- w) 분석 목적으로 lifetime을 무기한 연장하고 싶다면 **reference counting 대신 transaction object를 clone해야 권장(should)**.
  즉 **reference count를 프로토콜의 정상 phase를 넘어서는 lifetime 연장에 쓰면 안 된다 (should not)**.
- x) memory manager가 있을 때, transaction object는 **initiator 자신 외에 어떤 컴포넌트도 참조를 갖고 있지 않음을
  reference count가 나타내기 전까지 새 transaction 표현이나 다른 interface에 재사용해서는 안 된다 shall not**.
  즉 initiator가 `acquire`를 호출했다고 가정하면 **reference count가 1이 될 때까지**.
  이 규칙은 같은 interface 재사용과 transport/DMI/debug transport 간 재사용 모두에 적용된다.
  서로 다른 transaction 인스턴스를 나타내기 위해 재사용할 때는 **reference count가 0이 될 때까지(= 해제될 때까지)
  재사용하지 않는 것이 best practice**.

### `reset` (y)

- 자동 삭제 표시된 extension을 **모두 delete해야 shall**하고, 대응 extension pointer를 **null로 설정해야 shall**.
- 각 extension은 **extension object의 멤버 함수 `free`를 호출해서 delete해야 shall** (사용자가 명시적 memory management를
  원한다면 오버로드할 수 있음).
- **option attribute 값을 `TLM_MIN_PAYLOAD`로 설정해야 shall.**
- transaction lifetime 끝에서 extension을 지우기 위해 보통 **`tlm_mm_interface::free`에서 호출해야 권장(should)**.

### extension 삭제/해제의 선택 (z ~ ac)

- z) `set_extension`으로 추가한 extension object는 **`release_extension` 호출로 delete해도 된다 may**.
  **`clear_extension`은 pointer만 지우고 extension object 자체는 delete하지 않는다.**
  후자는 transaction object가 memory manager 없이 stack 할당되고 extension object는 pooling되는 경우에 필요하다.
- aa) **memory manager가 없을 때**, 주어진 extension을 할당·설정한 컴포넌트가
  **`b_transport`·`get_direct_mem_ptr`·`transport_dbg`에서 제어를 반환하기 전에 그 extension을 delete하거나 clear해야
  권장(should)**. 예: `b_transport`를 구현하며 `set_mm`으로 memory manager를 붙인 interconnect는
  **자신이 추가한 모든 extension을 제거하기 전에는 `b_transport`에서 반환하면 안 된다 shall not**.
- ab) **memory manager가 있을 때**는 `set_auto_extension`으로 추가하면 memory manager가 자동으로 delete/pool한다.
  대안으로 `set_extension`으로 추가하고 명시적으로 clear하지 않은 것은 **sticky extension** —
  reference count가 0이 되어도 자동 삭제되지 않고 transaction object가 pooling되어도 그대로 남아 있을 수 있다.
  sticky extension은 transport 호출 사이에 extension object를 재생성할 필요가 없어 **특히 효율적**이며,
  transaction object가 pooling(또는 단일 재사용)되는 것에 의존한다.
- ac) **memory manager 존재 여부를 모를 때는 `set_extension`으로 추가하고 `release_extension`으로 삭제해야 권장(should)**.
  이 호출 순서는 memory manager 유무 어느 쪽에서도 안전하다. 이런 상황은 `has_mm`을 호출하지 않기로 선택한
  interconnect나 target에서만 발생할 수 있다 (initiator 내부에서는 항상 알 수 있음).

### `free_all_extensions` (ad, ae)

- **자동 삭제 표시된 것을 포함해 모든 extension을 delete해야 shall**하고, 대응 pointer를 **null로 설정해야 shall**.
  각 extension은 extension object의 `free`를 호출해 delete해야 shall.
- memory manager를 쓰지 않는 pooled transaction object에서 extension을 제거할 때 유용하다.
  memory manager가 있으면 자동 삭제 표시된 것은 이미 자동 삭제되었을 것이고, sticky extension은 삭제할 필요가 없다.

### `deep_copy_from` (af, ag)

- 인자로 넘어온 다른 transaction object의 attribute와 extension을 복사해 **현재 transaction object를 수정해야 shall**.
- **복사되는 attribute (shall)**: option, command, address, data length, byte enable length, streaming width,
  response status, DMI allowed.
- **data array와 byte enable array는 두 transaction의 대응 pointer가 모두 non-null일 때 그리고 그때만 deep copy해야 shall.**
  **현재 transaction의 array가 충분히 큰지 보장할 책임은 애플리케이션에 있다.**
- 다른 transaction의 extension이 현재 transaction에 **이미 존재하면 extension class의 `copy_from`을 호출해 복사해야 shall**.
  **그렇지 않으면 extension class의 `clone`을 호출해 새 extension object를 만들어 현재 transaction에 설정해야 shall.**
- clone의 경우, **현재 transaction에 memory manager가 있을 때 그리고 그때만** 새 extension을 자동 삭제 표시해야 shall.
- ag) 즉, memory manager가 있으면 `deep_copy_from`은 현재 object에 없던 새 extension을 자동 삭제 표시한다.
  memory manager가 없으면 auto-deletion 표시가 불가능하다.

### `update_original_from` (ah)

- 인자로 넘어온 다른 transaction object로부터 **특정 attribute와 extension을 복사해 현재 object를 수정해야 shall**.
- 의도: `deep_copy_from`으로 만든 transaction의 **response를 되돌려 주기 위해** 호출.
- **수정되는 attribute (shall)**: response status, DMI allowed.
- **data array는 다음 조건을 모두 만족할 때 그리고 그때만 deep copy해야 shall**:
  현재 transaction의 command가 `TLM_READ_COMMAND`이고, 두 transaction의 data pointer가 **모두 non-null이며 서로 다를 것**.
- **byte enable array는 byte enable pointer가 non-null이고 `use_byte_enable_on_read` 인자가 true일 때
  그리고 그때만** read command에서와 같이 복사 연산을 마스킹하는 데 쓰여야 shall. 그렇지 않으면 data array 전체를 deep copy.
- 현재 transaction의 extension은 **`update_extensions_from`에 따라 갱신되어야 shall**.
- 시그니처 기본값: `update_original_from(const tlm_generic_payload&, bool use_byte_enable_on_read = true)`.

### `update_extensions_from` (ai)

- 다른 transaction object로부터 **현재 object에 이미 존재하는 extension만** 복사해 현재 object의 extension을 수정해야 shall.
- 복사는 extension class의 **`copy_from` 호출로** 이루어져야 shall.

### 전형적 사용 (aj, ak)

- `deep_copy_from` / `update_original_from` / `update_extensions_from`의 전형적 사용처는 **transaction bridge**:
  incoming request를 deep copy → initiator socket으로 전송 → response를 받으면 해당 attribute와 extension을
  원래 transaction object로 다시 복사. bridge는 array를 deep copy할 수도, pointer만 복사할 수도 있다.
- ak) 이 의무들은 generic payload에 적용된다. 원칙적으로 generic payload와 무관한 transaction type에도 유사한
  의무가 적용될 수 있다.

---

## 14.6 Constructors, assignment, and destructor

- a) **default 생성자**는 generic payload attribute들을 **기본값으로 설정해야 shall** (이후 절에 정의된 값).
- b) `tlm_generic_payload(tlm_mm_interface*)`는 attribute를 기본값으로 설정하고, **인자로 넘어온 주소의 객체를
  memory manager로 설정해야 shall**. default 생성자 호출 직후 `set_mm`을 호출하는 것과 동등.
- c) **copy constructor와 assignment operator는 비활성화(disabled)되어 있다.**
- d) **virtual destructor `~tlm_generic_payload`는 자동 삭제 표시된 것을 포함해 모든 extension을 delete해야 shall.**
  각 extension은 extension object의 `free`를 호출해 delete해야 shall.
  **destructor는 data array나 byte enable array를 delete하면 안 된다 shall not.**

---

## 14.7 Default values and modifiability of attributes ★

base protocol에서의 기본값과 수정 가능성.

### Table 54 — Default values and modifiability of attributes

| Attribute | Default value | Modifiable by interconnect? | Modifiable by target? |
|---|---|---|---|
| Option | `TLM_MIN_PAYLOAD` | No | **Yes** |
| Command | `TLM_IGNORE_COMMAND` | No | No |
| Address | `0` | **Yes** | No |
| Data pointer | `0` | No | No |
| Data length | `0` | No | No |
| Byte enable pointer | `0` | No | No |
| Byte enable length | `0` | No | No |
| Streaming width | `0` | No | No |
| DMI allowed | `false` | **Yes** | **Yes** |
| Response status | `TLM_INCOMPLETE_RESPONSE` | No | **Yes** |
| Extension pointers | `0` | **Yes** | **Yes** |

### Table 55 — Modifiability of generic payload arrays

| Arrays | Default value | Modifiable by interconnect? | Modifiable by target? |
|---|---|---|---|
| Data array | — | No | **Read command only** |
| Byte enable array | — | No | No |

### 규칙

- a) IMC로 transaction object를 넘기기 전에 **attribute 값을 설정할 책임은 initiator에 있다**.
  transport interface의 경우 **extension pointer를 제외한 모든 generic payload attribute를 설정해야 shall**.
  DMI와 debug transport interface는 자체 규칙이 있다(11.3.4, 11.4.4).
  option attribute로 DMI·debug transport가 minimal set을 쓸지 full set을 쓸지 결정할 수 있다.
  **transaction object가 pooling·재사용될 때 attribute가 올바르게 설정되도록 주의해야 권장(should)**.
- b) transaction object가 pool로 반환되거나 재사용될 때, **그 transaction 인스턴스의 lifetime 끝에서 modifiability
  규칙은 적용이 중단된다**. memory manager가 있으면 reference count가 0이 되는 시점, 없으면 IMC에서 반환되는 시점.
  새 transaction으로 재사용되면 규칙이 새로 적용된다.
- c) transaction object를 IMC(`b_transport`, `nb_transport_fw`, `get_direct_mem_ptr`, `transport_dbg`)의 인자로 넘긴 후,
  **initiator가 transaction lifetime 동안 수정해도 되는 attribute는 extension pointer 뿐이다**.
- d) **interconnect는 address attribute를 수정해도 되지만 may, forward path의 어떤 TLM-2.0 core interface method에도
  그 transaction을 넘기기 전에만 가능하다.** 일단 downstream 컴포넌트에 참조를 넘긴 후에는
  **transaction의 전체 lifetime 동안 다시 address attribute를 수정하는 것이 허용되지 않는다**.
- e) 그 결과, address attribute는 forward path IMC(`b_transport`, `get_direct_mem_ptr`, `transport_dbg`)에
  진입하는 즉시 유효하다. `nb_transport_fw`의 경우 **phase가 `BEGIN_REQ`일 때만** 진입 즉시 유효.
  **forward path TLM-2.0 IMC에서 반환된 후의 address attribute는 가장 downstream의 interconnect가 설정한 값을 가지며,
  transaction routing 목적으로는 undefined로 간주해야 권장(should)**.
- f) **write command에서는 interconnect와 target 모두 data array를 수정할 수 없다.
  read command에서는 target만 data array를 수정해도 된다.**
- g) 주어진 transaction object에 대해 **target은 DMI allowed, response status, (read command의 경우) data array를
  transaction object를 처음 받은 시점부터 upstream 방향으로 response를 넘기는 시점 사이 언제든 수정해도 된다 may.**
  **upstream으로 response를 보낸 후에는 이 attribute들을 수정하는 것이 허용되지 않는다.**
  여기서 "response를 보낸다"는 것은 다음 중 하나:
  `b_transport`·`get_direct_mem_ptr`·`transport_dbg`에서 제어를 반환할 때,
  `nb_transport`에 `BEGIN_RESP` phase를 인자로 넘길 때,
  `nb_transport`에서 `TLM_COMPLETED`를 반환할 때.
- h) **DMI allowed가 false이면 interconnect는 이를 수정할 수 없다.**
  그러나 **target이 true로 설정하면 interconnect는 response를 upstream으로 넘기면서 false로 되돌려도 된다 may.**
  즉 interconnect는 target이 설정한 DMI allowed를 **clear하는 것은 허용**된다.
- i) initiator는 **response를 받은 후에야** DMI allowed, response status, (read command의) data array가
  target이 수정한 값이라고 가정해도 된다 may.
- j) 위 규칙이 특정 시간 창에서 수정을 허용하면, **그 창 안에서는 언제든 몇 번이든 수정해도 된다 may**.
  다른 컴포넌트는 **그 창이 끝난 시점의 값만 읽어야 shall** (extension은 예외).
- k) initiator/interconnect/target의 역할은 **동적으로 바뀔 수 있다**. 예를 들어 interconnect는 response status를
  수정할 수 없지만, 같은 컴포넌트가 특정 transaction에 대해 **target 역할을 맡아** 수정할 수 있다.
  target 역할일 때 그 transaction을 **더 downstream으로 넘기는 것은 금지된다**.
- l) generic payload가 **direct memory / debug transport interface**의 transaction type으로 쓰일 때,
  이 절의 modifiability 규칙은 해당 attribute에 적용되어야 shall:
  **DMI의 경우 command와 address attribute**, **debug transport의 경우 command, address, data pointer, data length attribute**.

---

## 14.8 Option attribute

- a) option attribute는 **DMI와 debug transport interface가 minimal set을 쓸지 full set을 쓸지** 결정한다.
  minimal set은 이전 버전 TLM-2.0과의 하위 호환성을 위해 지원된다.
- b) `set_gp_option`은 인자 값으로 설정해야 shall, `get_gp_option`은 현재 값을 반환해야 shall.
- c) **기본값은 `TLM_MIN_PAYLOAD`여야 shall.**
- d) 한 가지 예외를 빼면 initiator·interconnect·target은 **option attribute를 무시해도 된다 may**.
  이는 표준 발행 전후 개발된 컴포넌트 모두에 적용된다.
  **유일한 예외는 initiator가 `TLM_FULL_PAYLOAD_ACCEPTED` 값의 option을 가진 transaction을 받았을 때.**
- e) **DMI를 통해 보낼 때**, target이 response status를 설정해 주기를 요구하는 initiator는
  **option을 `TLM_FULL_PAYLOAD`로 설정해야 shall**, 이 경우 initiator는
  **byte enable pointer, byte enable length, streaming width, DMI allowed, response status를 Table 54의 기본값으로
  설정해야 shall**.
- f) **debug transport를 통해 보낼 때**, target이 byte enable pointer/byte enable length/streaming width를 사용하거나
  DMI allowed/response status를 설정해 주기를 요구하는 initiator는 **option을 `TLM_FULL_PAYLOAD`로 설정해야 shall**,
  이 경우 initiator는 **DMI allowed와 response status를 기본값으로 설정해야 shall**.
- g) **blocking 또는 non-blocking transport interface**를 통해 보낼 때, option은 **`TLM_MIN_PAYLOAD`로 설정되어야 shall**하고,
  **어떤 컴포넌트도 수정하면 안 된다 shall not**.
- h) option이 `TLM_MIN_PAYLOAD`이면 **어떤 interconnect나 target도 그 값을 수정하면 안 된다 shall not**.
- i) **DMI**에서 option이 `TLM_MIN_PAYLOAD`이면 target은 **command와 address를 제외한 모든 attribute를 무시해도 된다 may**.
- j) **DMI**에서 option이 `TLM_FULL_PAYLOAD`이면 target은 값을 `TLM_FULL_PAYLOAD_ACCEPTED`로 설정해도 되고 may,
  그 경우 initiator와 target은 **14.17의 규칙에 따라 response status를 설정하고 그에 따라 동작해야 shall**.
- k) **debug transport**에서 option이 `TLM_MIN_PAYLOAD`이면 target은 **byte enable pointer, byte enable length,
  streaming width, DMI allowed, response status를 무시해도 된다 may**.
- l) **debug transport**에서 option이 `TLM_FULL_PAYLOAD`이면 target은 `TLM_FULL_PAYLOAD_ACCEPTED`로 설정해도 되고 may,
  그 경우 initiator와 target은 14.13, 14.14, 14.15, 14.16, 14.17의 규칙에 따라
  byte enable pointer, byte enable length, streaming width, DMI allowed, response status를 설정하고
  그에 따라 동작해야 shall.
- m) target이 `TLM_FULL_PAYLOAD_ACCEPTED`로 설정하지 않기로 하면, target은
  **byte enable pointer/byte enable length/streaming width를 무시해야 shall**하고
  **DMI allowed나 response status를 설정하면 안 된다 shall not**.
- n) initiator가 `TLM_FULL_PAYLOAD`로 설정했는데 target이 `TLM_FULL_PAYLOAD_ACCEPTED`로 설정하지 않으면,
  initiator는 **target이 `TLM_MIN_PAYLOAD`인 것처럼 동작했다고 가정해야 shall**.
  이 상황에서는 target이 attribute를 잘못 해석했을 수 있다 (예: debug transport에서 byte enable을 무시).
- o) **interconnect는 option 값을 수정하면 안 된다 shall not.**
  (일반적으로 interconnect로 동작하는 컴포넌트가 error response를 반환하기 위해 target으로 동작할 수 있으며,
  그 경우 option을 `TLM_FULL_PAYLOAD_ACCEPTED`로 설정해도 된다 may.)
- p) option을 `TLM_FULL_PAYLOAD`로 설정한 initiator는 **transaction object lifetime 끝에서 option을
  `TLM_MIN_PAYLOAD`로 되돌려야 shall**. memory manager가 있으면 `reset`이 `TLM_MIN_PAYLOAD`로 설정해야 shall.
  **memory manager가 없으면 initiator가 명시적으로 되돌릴 의무가 있다.**
- q) option 값은 **현재 transaction 인스턴스에만 적용되어야 shall**하며, 다른 transaction이나 다른 interface에 대한
  initiator/interconnect/target의 동작에 대해서는 아무것도 함의하지 않는다.
  **각 컴포넌트는 initiator·target의 능력 맵을 만들지 말고 각 transaction을 개별적으로 검사해야 권장(should)**.

---

## 14.9 Command attribute

- a) `set_command`는 인자 값으로 설정해야 shall, `get_command`는 현재 값을 반환해야 shall.
- b) `set_read`/`set_write`는 command를 각각 `TLM_READ_COMMAND`/`TLM_WRITE_COMMAND`로 설정해야 shall.
  `is_read`/`is_write`는 현재 command가 각각 그 값일 때 **그리고 그때만 true를 반환해야 shall**.
- c) read/write/ignore command의 정의 = command attribute가 각각 `TLM_READ_COMMAND`/`TLM_WRITE_COMMAND`/`TLM_IGNORE_COMMAND`.
- d) **read command 수신 시 target은 자신의 local array 내용을 data pointer가 가리키는 array로 복사해야 shall** —
  이 표준이 정의한 generic payload의 모든 semantics를 지키면서.
- e) **write command 수신 시 target은 data pointer가 가리키는 array 내용을 local array로 복사해야 shall.**
- f) target이 read/write command를 실행할 수 없으면 **standard error response를 생성해야 shall**.
  **권장 response status는 `TLM_COMMAND_ERROR_RESPONSE`.**
- g) ignore command는 **null command**. 의도는 read/write를 실행하지 않고 **extension을 운반하는 수단**으로 쓰는 것.
  단, extension에 관한 규칙은 세 command 모두 동일하다.
- h) **ignore command 수신 시 target은 write나 read command를 실행하면 안 된다 shall not.**
  특히 **write command가 수정할 local array 값이나 data pointer가 가리키는 array 값을 수정하면 안 된다 shall not.**
  단, extension을 포함한 generic payload의 어떤 attribute든 값을 **사용해도 된다 may**.
- i) ignore command 수신 시, 보통 interconnect로 동작하는 컴포넌트는 transaction을 target 쪽으로 전달하거나
  (= interconnect로 동작) **error response를 반환해도 된다** (= target으로 동작).
  read/write를 다르게 라우팅하는 컴포넌트라면 error response를 반환할 것으로 기대된다.
- j) target이 transaction을 받고 generic payload attribute 값을 스스로 만족스럽게 검사했다면
  ignore command를 성공적으로 실행한 것으로 간주된다 (14.17 참조).
- k) **command attribute는 initiator가 설정해야 shall**하고, **어떤 interconnect나 target도 덮어쓰면 안 된다 shall not**.
- l) **기본값은 `TLM_IGNORE_COMMAND`여야 shall.**

---

## 14.10 Address attribute

- a) `set_address`는 인자 값으로 설정해야 shall, `get_address`는 현재 값을 반환해야 shall.
- b) read/write command에서 target은 address를 **읽거나 쓰는 연속 데이터 블록의 시스템 메모리 맵상 시작 주소로
  해석해야 shall**. 이 주소는 **host computer의 endianness에 따라 data array의 첫 바이트에 대응할 수도, 아닐 수도 있다**.
- c) data array의 주어진 바이트에 연관된 주소는 **address attribute, array index, streaming width attribute,
  host endianness, socket의 width에 의존**한다 (14.18).
- d) address 값은 **word-aligned일 필요가 없다** (다만 로컬 socket width의 바이트 배수이면 주소 계산이 상당히 단순해진다).
- e) 주어진 address로 transaction을 실행할 수 없으면(예: out-of-range) **standard error response를 생성해야 shall**.
  **권장 response status는 `TLM_ADDRESS_ERROR_RESPONSE`.**
- f) **address는 initiator가 설정해야 shall하지만, 하나 이상의 interconnect가 덮어써도 된다 may.**
  주소 변환(시스템 메모리 맵의 절대 주소 → target이 아는 메모리 맵의 상대 주소) 등에 필요할 수 있다.
  **한 번 덮어쓰면 이전 값은 소실된다** (명시적으로 어딘가에 저장하지 않았다면).
- g) **기본값은 0이어야 shall.**

---

## 14.11 Data pointer attribute

- a) `set_data_ptr`/`get_data_ptr`은 pointer 값을 설정/반환한다. **array의 내용이 아니라 pointer 값**임에 주의.
- b) read/write command에서 target은 나머지 attribute의 semantics를 지키면서 data array로/에서 데이터를 복사해야 shall.
- c) **data array와 byte enable array의 저장소 할당 책임은 initiator에 있다.**
  저장소는 initiator 내 데이터의 최종 source/destination(레지스터 파일, 캐시 등)일 수도 있고 임시 버퍼일 수도 있다.
- d) 일반적으로 generic payload data array의 조직은 initiator/target의 local storage 조직과 독립적이다.
  다만 generic payload는 **대부분의 경우 단 한 번의 `memcpy` 호출로 target과 데이터를 주고받을 수 있게** 설계되었다
  (target이 generic payload와 같은 저장소 조직을 쓴다고 가정). 이 가정은 시뮬레이션 효율을 위한 것이고
  표현력을 제한하지 않는다 — target은 복사하면서 데이터를 원하는 대로 변환해도 된다.
- e) **read 또는 write command에서 data pointer가 null인 transaction object로 transport interface를 호출하는 것은 error다.**
- f) `TLM_IGNORE_COMMAND`의 경우 data pointer는 data array를 가리켜도 되고 **null이어도 된다**.
- g) data pointer가 null이 아니면 **data array의 길이는 data length attribute 값(바이트) 이상이어야 shall**.
- h) **data pointer는 initiator가 설정해야 shall**하고, **어떤 interconnect나 target도 덮어쓰면 안 된다 shall not**.
- i) **write command 또는 `TLM_IGNORE_COMMAND`에서 data array의 내용은 initiator가 설정해야 shall**하고,
  **어떤 interconnect나 target도 덮어쓰면 안 된다 shall not**.
- j) **read command에서 data array의 내용은 target이 덮어써도 되지만 may** (byte enable의 semantics를 지키면서),
  **다른 어떤 컴포넌트도 안 되며, target이 response를 보내기 전에만 가능하다.**
  여기서 "response를 보낸다"는 `b_transport`·`get_direct_mem_ptr`·`transport_dbg`에서 제어 반환,
  `nb_transport`에 `BEGIN_RESP` phase 전달, `nb_transport`에서 `TLM_COMPLETED` 반환 중 하나.
- k) **기본값은 0(null pointer)여야 shall.**

---

## 14.12 Data length attribute

- a) `set_data_length`/`get_data_length`.
- b) read/write command에서 target은 data length를 **data array로/에서 복사할 바이트 수**로 해석해야 shall —
  **byte enable로 비활성화된 바이트를 포함해서**.
- c) **data length는 initiator가 설정해야 shall**하고, **어떤 interconnect나 target도 덮어쓰면 안 된다 shall not**.
- d) **read/write command에서 data length를 0으로 설정하면 안 된다 shall not.**
  0바이트를 전송하려면 **command를 `TLM_IGNORE_COMMAND`로 설정해야 권장(should)**.
- e) **`TLM_IGNORE_COMMAND`이고 data pointer가 null이면 data length 값은 undefined다.**
- f) interoperability layer의 표준 socket class(또는 그 파생)를 burst transfer에 쓸 때,
  **각 전송의 word length는 socket의 `BUSWIDTH` 템플릿 파라미터로 결정되어야 shall**.
  **BUSWIDTH는 data length와 독립적이며 bit 단위로 표현되어야 shall.**
  **data length ≤ BUSWIDTH / 8**이면 사실상 single-word transfer를, **더 크면 burst**를 모델링하는 것이다.
  하나의 transaction이 서로 다른 bus width의 socket을 통과해도 된다. BUSWIDTH는 전송 지연 계산에 쓰일 수 있다.
- g) target은 자신의 word length보다 큰 data length의 transaction을 **지원할 수도 안 할 수도 있다**
  (word length가 BUSWIDTH로 주어지든 다른 값으로 주어지든).
- h) 주어진 data length로 transaction을 실행할 수 없으면 **standard error response를 생성해야 shall**하고,
  **data array 내용을 수정하면 안 된다 shall not**. **권장 response status는 `TLM_BURST_ERROR_RESPONSE`.**
- i) **기본값은 0이어야 shall** — data pointer가 null이 아닌 한 **invalid한 값**이다.
  따라서 data pointer가 null이 아니면 IMC로 넘기기 전에 **data length를 명시적으로 설정해야 shall**.

---

## 14.13 Byte enable pointer attribute

- a) `set_byte_enable_ptr`/`get_byte_enable_ptr`.
- b) byte enable array의 원소 해석 (shall):
  **0 = 대응 바이트 disabled, 0xff = 대응 바이트 enabled. 그 외 모든 값의 의미는 undefined.**
  0xff는 array를 **mask로 직접 쓸 수 있도록** 선택되었다. 편의 매크로 `TLM_BYTE_DISABLED` / `TLM_BYTE_ENABLED` 제공.
- c) byte enable은 각 beat의 주소 증가량이 유효 전송 바이트 수보다 큰 burst를 만들거나,
  버스의 선택된 byte lane에 word를 배치하는 데 쓸 수 있다. 더 추상적으로는 data array에 임의 패턴의 구멍이 뚫린
  **"lacy burst"**를 만들 수 있다.
- d) byte enable mask는 **작은 패턴을 반복 적용**하거나 **data array 전체를 덮는 큰 패턴**으로 정의해도 된다 (14.18).
- e) **byte enable array의 원소 수는 byte enable length attribute로 주어져야 shall.**
- f) byte enable pointer를 **0(null)로 설정해도 되며 may**, 이 경우 현재 transaction에서
  **byte enable을 쓰지 않아야 shall**하고 **byte enable length는 무시되어야 shall**.
- g) byte enable을 쓴다면 **pointer 설정, byte enable array 저장소 할당, array 내용 설정 모두 initiator가 해야 shall**.
  **pointer도 array 내용도 어떤 interconnect나 target이 덮어쓰면 안 된다 shall not.**
- h) byte enable pointer가 non-null이면 target은 **아래 정의된 byte enable semantics를 구현하거나
  standard error response를 생성해야 shall**. **권장 response status는 `TLM_BYTE_ENABLE_ERROR_RESPONSE`.**
- i) **write command**: interconnect나 target은 data array 내 **disabled 바이트 값을 무시해야 권장(should)**.
  disabled 바이트가 어떤 동작에도 영향을 주지 않는 것이 **권장(recommended)**.
  어차피 무시되므로 initiator는 그 바이트에 아무 값이나 넣어도 된다 may.
- j) **write command**: target이 transaction data array에서 local array로 byte-by-byte 복사할 때,
  **disabled 바이트에 대응하는 local array의 바이트 값을 수정하지 않아야 권장(should not)**.
- k) **read command**: interconnect나 target은 data array 내 **disabled 바이트 값을 수정하지 않아야 권장(should not)**.
  initiator는 disabled 바이트가 수정되지 않는다고 가정해도 된다.
- l) **read command**: target이 local array에서 transaction data array로 byte-by-byte 복사할 때,
  **disabled 바이트에 대응하는 local array의 바이트 값을 무시해야 권장(should)**.
- m) 이런 byte enable semantics나 generic payload의 다른 semantics를 위반해야 한다면
  **새 protocol traits class를 만드는 것이 권장 방식(14.2.3)**.
- n) **기본값은 0(null pointer)여야 shall.**

---

## 14.14 Byte enable length attribute

- a) `set_byte_enable_length`/`get_byte_enable_length`.
- b) read/write command에서 target은 이를 **byte enable array의 원소 수**로 해석해야 shall.
- c) **initiator가 설정해야 shall**하고 **어떤 interconnect나 target도 덮어쓰면 안 된다 shall not**.
- d) **주어진 data array 원소에 적용할 byte enable은 다음 식으로 계산되어야 shall**:

  ```
  byte_enable_array_index = data_array_index % byte_enable_length
  ```

  즉 **byte enable array가 data array에 반복적으로 적용된다**.
- e) byte enable length가 data length보다 커도 된다 may. 이 경우 남는 byte enable은
  read/write command 동작에 **영향을 주지 않아야 권장(should not)**하지만 extension이 쓸 수는 있다.
- f) **byte enable pointer가 0(null)이면 byte enable length는 어떤 interconnect나 target도 무시해야 shall.
  byte enable pointer가 non-0이면 byte enable length는 non-0이어야 shall.**
- g) 주어진 byte enable length로 실행할 수 없으면 **standard error response를 생성해야 shall**.
  **권장 response status는 `TLM_BYTE_ENABLE_ERROR_RESPONSE`.**
- h) **기본값은 0이어야 shall.**

---

## 14.15 Streaming width attribute

- a) `set_streaming_width`/`get_streaming_width`.
- b) read/write command에서 target은 현재 streaming width 값을 **해석하고 그에 따라 동작해야 shall**.
- c) **streaming은 컴포넌트가 data array를 해석하는 방식에 영향을 준다.**
  stream은 연속된 notional beat에서 일어나는 데이터 전송 시퀀스로, **각 beat는 generic payload의 address attribute로
  주어지는 동일한 시작 주소를 갖는다**. **streaming width는 stream의 width — 즉 각 beat에서 전송되는 바이트 수 — 를
  결정해야 shall**. 다시 말해 streaming은 **data array의 각 바이트에 연관된 local address**에 영향을 준다.
  그 외 모든 면에서 data array의 조직은 streaming의 영향을 받지 않는다.
- d) data array의 바이트들은 컴포넌트 내에서 대응하는 local address 시퀀스를 갖는다.
  **최저 주소 = address attribute 값. 최고 주소 = `address_attribute + streaming_width - 1`.**
  target에서 각 바이트가 복사되는 주소는 **각 beat 시작 시 address attribute 값으로 설정되어야 shall**.
- e) data array 해석 관점에서, streaming width를 가진 **하나의 transaction은 다음과 기능적으로 동등해야 shall**:
  각각 원래와 같은 address를 갖고, data length가 원래의 streaming width와 같으며,
  각 beat마다 원래 data array의 서로 다른 subset을 data array로 갖는 transaction 시퀀스.
  이 subset은 바이트 순서를 유지한 채 원래 data array를 따라 내려간다.
- f) **streaming width가 0이면 invalid하다 shall.**
  streaming 전송이 필요 없으면 **streaming width를 data length 이상의 값으로 설정해야 권장(should)**.
- g) streaming width 값은 **data array의 길이나 data array에 저장된 바이트 수에 영향을 주면 안 된다 shall (no affect)**.
- h) streaming width가 socket의 width(바이트 단위)와 다르면 **width conversion 문제**가 발생할 수 있다 (14.18).
- i) 주어진 streaming width로 실행할 수 없으면 **standard error response를 생성해야 shall**.
  **권장 response status는 `TLM_BURST_ERROR_RESPONSE`.**
- j) streaming은 byte enable과 함께 써도 되며, 이 경우 보통 **streaming width = byte enable length**로 한다.
  streaming width를 byte enable length의 배수로 하는 것도 말이 된다.
  byte enable length를 streaming width의 배수로 하면 **각 beat마다 다른 바이트가 enabled됨**을 뜻한다.
- k) **initiator가 설정해야 shall**하고 **어떤 interconnect나 target도 덮어쓰면 안 된다 shall not**.
- l) **기본값은 0이어야 shall.**

---

## 14.16 DMI allowed attribute

- a) `set_dmi_allowed`는 인자 값으로 설정해야 shall, `is_dmi_allowed`는 현재 값을 반환해야 shall.
- b) DMI allowed는 initiator에게 **direct memory pointer를 얻어 봐도 좋다는 hint**를 제공한다.
  해당 transaction이 DMI로 수행될 수 있었다면 **target은 이 값을 true로 설정해야 권장(should)** (11.3.9).
- c) **기본값은 `false`여야 shall.**

---

## 14.17 Response status attribute

### 14.17.1 Overview

- a) `set_response_status`/`get_response_status`.
- b) `is_response_ok`는 현재 값이 `TLM_OK_RESPONSE`일 때 **그리고 그때만** true를 반환해야 shall.
  `is_response_error`는 `TLM_OK_RESPONSE`가 **아닐 때 그리고 그때만** true를 반환해야 shall.
- c) `get_response_string`은 현재 response status 값을 **텍스트 문자열로 반환해야 shall**.
- d) 일반 원칙으로 **target은 generic payload의 모든 기능을 지원하도록 권장(recommended)**되지만,
  지원하지 않는 경우 **standard error response를 생성해야 shall** (14.17.2).
- e) **response status는 initiator가 `TLM_INCOMPLETE_RESPONSE`로 설정해야 shall**하고,
  target이 덮어쓸 수도 있고 아닐 수도 있다.
  **interconnect가 덮어쓰면 안 된다 shall not.**
  `TLM_INCOMPLETE_RESPONSE`는 **target으로 동작하는 컴포넌트가 command 실행을 시도조차 하지 않았음**을
  나타내는 데 써야 권장(should) — 보통 interconnect로 동작하는 컴포넌트가 response를 반환한 경우 등.
  단, 그런 컴포넌트도 target으로 동작하는 것이므로 **어떤 error response로든 설정하는 것은 허용된다**.
- f) target이 command를 성공적으로 실행할 수 있으면 **`TLM_OK_RESPONSE`로 설정해야 shall**.
  그렇지 않으면 **Table 56의 여섯 error response 중 아무것이나 설정해도 된다 may**.
  원인에 맞는 적절한 error response를 고르는 것이 권장(should).
- g) error를 검출했지만 구체적 error response를 고를 수 없으면 **`TLM_GENERIC_ERROR_RESPONSE`로 설정해도 된다 may**.
- h) **기본값은 `TLM_INCOMPLETE_RESPONSE`여야 shall.**
- i) `TLM_IGNORE_COMMAND`의 경우, transaction을 받았고 read/write를 실행할 수 있는 상태였던 target은
  **`TLM_OK_RESPONSE`를 반환해야 권장(should)**. 그렇지 않으면 read/write에 적용했을 기준으로
  **재량껏 error response를 설정해도 된다 may**. 예를 들어 byte enable을 지원하지 않는 target은
  `TLM_BYTE_ENABLE_ERROR_RESPONSE`를 반환하는 것이 **허용되지만 의무는 아니다**.
- j) generic payload extension이나 extended phase의 존재가 target으로 하여금 다른 response status를 반환하게 해도 된다 may —
  ignorable extension 규칙을 지키는 한. 즉 **base protocol 내에서 extension이 command를 실패시키는 것도 허용되고,
  target이 extension을 무시해서 command가 성공하는 것도 허용된다.**
- k) **target은 transaction lifetime의 적절한 시점에 response status를 설정할 책임이 있다 shall**:
  blocking transport의 경우 **`b_transport`에서 제어를 반환하기 전**,
  non-blocking transport + base protocol의 경우 **`BEGIN_RESP` phase를 보내거나 `TLM_COMPLETED`를 반환하기 전**.
- l) initiator는 `BEGIN_RESP` phase 전이를 받았을 때나 transaction 완료 후에
  **항상 response status를 확인해야 권장(should)**. 값이 항상 `TLM_OK_RESPONSE`임을 미리 안다면 무시해도 되지만,
  **일반적으로는 그렇지 않다. response status를 무시하는 initiator는 자기 책임이다.**
- m) target은 error response 선택에 어느 정도 재량이 있다. 예를 들어 command와 address가 모두 잘못되면
  `TLM_ADDRESS_ERROR_RESPONSE`, `TLM_COMMAND_ERROR_RESPONSE`, `TLM_GENERIC_ERROR_RESPONSE` 어느 것이든 정당할 수 있다.
  **initiator는 동작 결정에 여섯 error 범주의 구분만 의존하지 않아야 권장(should not)**.
  진단 메시지 내용 결정에는 써도 된다 may.

### 14.17.2 The standard error response

target이 generic payload transaction을 받으면 **다음 중 하나만(one and only one) 수행해야 권장(should)**:

1. transaction이 나타내는 command를 실행 — generic payload attribute의 semantics와 모델링 대상 컴포넌트의
   공개 문서화된 semantics를 지키면서 — 하고 **response status를 `TLM_OK_RESPONSE`로 설정**.
2. **위에 기술된 다섯 error response 중 하나로 response status를 설정**.
3. **표준 SystemC report handler로 report를 생성** (네 가지 표준 severity 중 아무것) — command가 실패했거나
   무시되었음을 나타내며 — 하고 **response status를 `TLM_OK_RESPONSE`로 설정**.

> 이 셋 중 정확히 하나를 수행하는 것이 **권장(recommended)**이지만,
> **구현이 이 권장을 강제할 의무도 없고 강제하는 것이 허용되지도 않는다 (not obliged or permitted to enforce)**.

> generic payload가 아닌 transaction type의 target도 같은 원칙을 따르는 것이 권장(recommended)이지만,
> 그런 transaction의 semantics와 error response 메커니즘 세부는 **이 표준의 범위 밖(outside the scope)**이다.

corner case 예시 (informative):

| # | 상황 | 판단 |
|---|---|---|
| a) | read/write 모두 지원하지만 write가 non-sticky한 메모리 맵드 레지스터 (write 후 read가 방금 쓴 데이터를 반환하지 않음) | 컴포넌트의 정상 기대 동작이면 **1번**에 해당 |
| b) | data attribute 값을 완전히 무시하고 bit를 set하는 write command 구현 | 정상 기대 동작이면 **1번** |
| c) | read-only memory가 response status로 error를 알리지 않고 write를 무시 | target 상태를 바꾸지 않고 무시하는 것이므로, **최소한 severity `SC_INFO` 또는 `SC_WARNING`의 SystemC report를 생성해야 권장(should)** |
| d) | write command를 read 수행으로 구현하거나 그 반대 | **어떤 상황에서도 하면 안 된다 (should not under any circumstances)** — generic payload semantics의 근본적 위반 |
| e) | read command를 의도대로 구현하되 부가 side-effect 있음 | **1번**에 해당 |
| f) | 주소 지정 레지스터 파일 target이 out-of-range 주소의 write를 받음 | **`TLM_ADDRESS_ERROR_RESPONSE` 설정하거나 SystemC report 생성해야 권장(should)** |
| g) | passive 시뮬레이션 버스 모니터 target이 모델링된 버스의 물리적 범위 밖 주소의 transaction을 받음 | **1번**으로 로깅하고 error response를 내지 않아도 되고 may, **3번**으로 report를 생성해도 된다 may |

정리: 1/2/3의 구분은 궁극적으로 사례별 실용적 판단이지만,
**generic payload의 확정적 규칙은 target이 항상 이 셋 중 정확히 하나를 수행해야 권장(should)한다**는 것.

### Example — command, address, data, response status

```cpp
// generic payload의 command, address, data, response status 사용

// The initiator
void thread() {
    tlm::tlm_generic_payload trans;                 // 기본 generic payload 생성
    sc_time delay;
    trans.set_command(tlm::TLM_WRITE_COMMAND);                     // write command
    trans.set_data_length(4);                                      // 4바이트 write
    trans.set_byte_enable_ptr(0);                                  // byte enable 미사용
    trans.set_streaming_width(4);                                  // streaming 미사용
    for (int i = 0; i < RUN_LENGTH; i += 4) {                      // 일련의 transaction 생성
         int word = i;
         trans.set_address(i);                                     // address 설정
         trans.set_data_ptr((unsigned char *)(&word));             // 지역 변수 'word'에서 write
         trans.set_dmi_allowed(false);                             // DMI hint clear
         trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);  // response status clear

         init_socket->b_transport(trans, delay);
         if (trans.is_response_error())                            // b_transport 반환값 확인
             SC_REPORT_ERROR("TLM-2.0", trans.get_response_string().c_str());
         ...
     }
}

// The target
virtual void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & t) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64 adr = trans.get_address();
    unsigned char *ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();
    unsigned char *byt = trans.get_byte_enable_ptr();
    unsigned int wid = trans.get_streaming_width();

    if (adr + len > m_length) {                 // 저장소 주소 overflow 검사
         trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
         return;
    }

    if (byt) {                                   // target이 byte enable을 지원하지 못함
         trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
         return;
    }
    if (wid < len) {                             // target이 streaming width를 지원하지 못함
         trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
         return;
    }

    if (cmd == tlm::TLM_WRITE_COMMAND)                                // command 실행
         memcpy(&m_storage[adr], ptr, len);
    else if (cmd == tlm::TLM_READ_COMMAND)
         memcpy(ptr, &m_storage[adr], len);

    trans.set_response_status(tlm::TLM_OK_RESPONSE);                  // 성공적 완료
}
```

### Example — byte enables

```cpp
// The initiator
void thread() {
    tlm::tlm_generic_payload trans;
    sc_time delay;

    static word_t byte_enable_mask = 0x0000fffful;   // host endianness와 무관하게 MSB..LSB

    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_data_length(4);
    trans.set_byte_enable_ptr(reinterpret_cast<unsigned char *>(&byte_enable_mask));
    trans.set_byte_enable_length(4);
    trans.set_streaming_width(4);
    ...
}

// The target
virtual void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64 adr = trans.get_address();
    unsigned char *ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();
    unsigned char *byt = trans.get_byte_enable_ptr();
    unsigned int bel = trans.get_byte_enable_length();
    unsigned int wid = trans.get_streaming_width();

    if (cmd == tlm::TLM_WRITE_COMMAND) {
         if (byt) {
             for (unsigned int i = 0; i < len; i++) // byte enable을 data array에 반복 적용
                 if (byt[i % bel] == tlm::TLM_BYTE_ENABLED)
                      m_storage[adr + i] = ptr[i];  // byte enable[i]가 data ptr[i]에 대응
         } else
             memcpy(&m_storage[adr], ptr, len);     // byte enable 없음
    } else if (cmd == tlm::TLM_READ_COMMAND) {
         if (byt) {                                 // target이 byte enable 있는 read를 지원하지 않음
             trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
             return;
         } else
             memcpy(ptr, &m_storage[adr], len);
    }
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
```

---

## 14.18 Endianness

### 14.18.1 Introduction

- **host endianness**(호스트 머신)와 **modeled endianness**(모델링 대상 initiator/target) 둘 다 관련된다.
  이 절은 generic payload data array와 byte enable array의 조직에 관한 규칙을 정의한다.
- 일반 원칙: **generic payload data array의 조직은 각 initiator/interconnect/target 내부에서 국지적으로 알 수 있는
  정보에만 의존한다** — 로컬 socket의 width, host endianness, 모델링 대상 컴포넌트의 endianness.
- 거시적 원칙: generic payload는 mixed-endian 시스템의 컴포넌트가 **MSB↔MSB, LSB↔LSB로 배선**되어 있다고 가정한다.
  endianness가 다른 컴포넌트 사이로 word가 전달되면 MSB…LSB 관계는 보존되지만 각 컴포넌트에서 보는
  바이트의 local address는 필연적으로 바뀐다 (**address swizzling**).
  반대로 local address가 불변이 되도록 배선된 mixed-endian 시스템이라면 **명시적 byte swap을 TLM-2.0 모델에 삽입해야 한다**.
- endianness 관련 interoperability를 위해서는 **이 절의 규칙만 지키면 된다**.
  data array 조직을 돕는 helper function 집합이 제공된다 (14.20).

### 14.18.2 Rules

- a) 이하 규칙에서 data array는 `data`, byte enable array는 `be`로 표기.
- b) interoperability layer의 표준 socket class(또는 그 파생)를 쓸 때, **data·byte enable array 내용은
  transaction이 로컬로 송수신되는 socket의 `BUSWIDTH` 템플릿 파라미터로 해석되어야 shall**.
  **유효 word length는 `(BUSWIDTH + 7)/8` 바이트로 계산되어야 shall** — 이하 `W`로 표기.
- c) `W`는 data array 내 word의 길이 = 로컬 socket을 통해 한 beat에 전송될 수 있는 데이터 양.
  data array는 **한 word, part-word, 또는 여러 연속 word/part-word**를 담을 수 있다.
  **첫 word와 마지막 word만 part-word일 수 있다.** 이는 generic payload 내부 조직을 말하는 것이지
  모델링 대상 아키텍처의 조직을 말하는 게 아니다.
- d) 하나의 transaction object가 서로 다른 width의 socket을 통과하면, data array의 word length는 socket에 따라
  다르게 계산된다 (width conversion 참조).
- e) **data array 내 각 word 안의 바이트 순서는 host-endian이어야 shall.**
  little-endian host에서는 주어진 word 안에서 `data[n]`이 `data[n+1]`보다 **덜 유효(less significant)**,
  big-endian host에서는 `data[n]`이 `data[n+1]`보다 **더 유효(more significant)**.
- f) **data array의 word 경계는 address-aligned여야 shall** — 즉 word length `W`의 정수배 주소에 놓여야 shall.
  다만 **address attribute도 data length attribute도 word length의 배수일 필요는 없다.**
  그래서 첫/마지막 word가 part-word일 수 있다.
- g) **data array 내 word 순서는 모델링 시스템의 메모리 맵상 주소로 결정되어야 shall.**
  array index가 streaming width 값보다 작은 경우, 연속 word들의 local address는 증가 순서여야 shall하며,
  (선행 part-word를 제외하고) **`address_attribute - (address_attribute % W) + NW`** 와 같아야 shall
  (N은 음이 아닌 정수, %는 나눗셈의 나머지).
- h) 즉, `{a,b,c,d}`를 array index 증가 순서로 표기하고 `LSBN`을 N번째 word의 LSB라 하면,
  **little-endian host**: `{..., MSB0, LSB1, ..., MSB1, LSB2, ...}`,
  **big-endian host**: `{... LSB0, MSB1, ... LSB1, MSB2, ...}`.
  각 full word의 바이트 수는 `W`, 총 바이트 수는 `data_length`.
- i) 위 규칙들은 사실상 initiator와 target이 **LSB-to-LSB, MSB-to-MSB로 연결됨**을 의미한다.
  대다수 initiator/target이 native endianness와 무관하게 host endianness로 모델링되는 경우
  (**"arithmetic mode"**) 최적의 시뮬레이션 속도를 내도록 선택되었다.
- j) **애플리케이션은 host endianness와 무관해야 강력히 권장(strongly recommended)** —
  어느 endianness의 host에서 실행해도 같은 동작을 모델링해야 한다. helper function이나 조건부 컴파일이 필요할 수 있다.
- k) initiator나 target이 native endianness로 모델링되고 그것이 host endianness와 다르면,
  data array로/에서 데이터를 전송할 때 **word 안의 바이트 순서를 swap해야 한다**. helper function이 제공된다.

**l~n) 예제 (informative)**

```cpp
int data = 0xAABBCCDD;
trans.set_data_ptr( reinterpret_cast<unsigned char*>( &data ) );
trans.set_data_length(4);
trans.set_address(0);
socket->b_transport(trans, delay);
```

C++ 컴파일러는 리터럴 `0xAABBCCDD`를 host-endian 형태로 해석한다. 어느 쪽이든 MSB는 `0xAA`, LSB는 `0xDD`.
이것이 의도라면 이 코드는 유효하고 host endianness와 무관하다. 다만 네 바이트의 array index는 달라진다:

| modeled / host | `data[0]` | local address |
|---|---|---|
| Little-endian model, little-endian host | `0xDD` | 0 |
| Big-endian model, little-endian host | `0xDD` | 3 |
| Little-endian model, big-endian host | `0xAA` | 3 |
| Big-endian model, big-endian host | `0xAA` | 0 |

- n) 위 코드는 **little도 big도 아닌 endianness의 host로는 포터블하지 않다**. 그런 경우 data array를
  **byte addressing만으로 접근하도록 재작성해야 한다.**
- o) little-endian 모델과 big-endian 모델이 같은 transaction을 해석하면 **MSB와 LSB가 무엇인지는 정의상 일치**하지만,
  word의 바이트에 접근하는 **local address는 서로 다르다**.

**p~t) 정렬·part-word 예제**

- p) data length도 address도 `W`의 정수배일 필요는 없다. 다만 word 경계에 정렬되고 `W`가 2의 거듭제곱이면
  data array 접근이 상당히 단순해진다. **48-bit socket 한가운데의 3바이트를 나타내는 address와 data length도
  완전히 정당하다.** 특정 target이 주어진 address나 data length를 지원할 수 없으면
  **standard error response를 생성해야 권장(should)** (14.17).
- q) 예: little-endian host, `W = 4`, `address = 1`, `data_length = 4` →
  첫 word는 주소 1..3의 3바이트, 둘째 word는 주소 4의 1바이트.
- r) 예: `W = 8`, `address = 5`, `data = {1,2}` → local address 5, 6의 두 바이트가 endianness에 따라 다른 순서로 접근된다.
- s) part-word·non-aligned 전송은 언제나 **`W`의 정수배 + byte enable**로 표현할 수 있다.
  따라서 하나의 transaction이 여러 개의 동등하게 유효한 generic payload 표현을 가질 수 있다.
  예 (little-endian host, little-endian initiator):

  ```
  address = 2, W = 4, data = {1}
    ≡ address = 0, W = 4, data = {x, x, 1, x}, be = {0, 0, 0xff, 0}

  address = 2, W = 4, data = {1,2,3,4}
    ≡ address = 0, W = 4, data = {x, x, 1, 2, 3, 4, x, x},
      be = {0, 0, 0xff, 0xff, 0xff, 0xff, 0, 0}
  ```
- t) part-word 접근에서 byte enable이 필요한지는 endianness에 의존한다.
  첫 word 전체 + 둘째 word의 LSB에 접근하려는 경우:

  ```
  little-endian host: address = 0, W = 4, data = {1,2,3,4,5}
  big-endian host   : address = 0, W = 4, data = {4,3,2,1,x,x,x,5},
                      be = {0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0xff}
  ```

**u~x) Width conversion**

- u) 두 socket이 바인딩되면 필연적으로 BUSWIDTH가 같다. 그러나 transaction이 target socket에서
  **다른 bus width의 initiator socket으로 전달될 수 있고**, 이때 **width conversion을 고려해야 한다** (Figure 27).
  width conversion에는 **자체적인 intrinsic endianness**가 있다 — 넓은 socket의 LSB를 먼저 뽑는지 MSB를 먼저 뽑는지에 따라.
- v) **width conversion의 endianness가 host endianness와 일치하면 conversion은 사실상 공짜** —
  하나의 transaction object를 수정 없이 socket에서 socket으로 전달할 수 있다.
  그렇지 않으면 **두 개의 별도 generic payload transaction object가 필요하다**.
- w) **좁은 socket → 넓은 socket** 변환 시에는 outgoing transaction에 address alignment를 수행할지 선택해야 한다.
  **address alignment를 수행하면 항상 새 generic payload transaction object 생성이 필요하다.**
- x) **streaming width가 0이 아니고 `W`와 다를 때도 유사한 width conversion 문제가 발생**한다.
  host endianness와 원하는 width conversion endianness에 따라 data array를 어떤 순서로 읽어 내릴지 선택해야 한다.

---

## 14.19 Helper functions to determine host endianness

### 14.19.2 Definition

```cpp
namespace tlm {

enum tlm_endianness {
   TLM_UNKNOWN_ENDIAN,
   TLM_LITTLE_ENDIAN,
   TLM_BIG_ENDIAN };

inline tlm_endianness get_host_endianness(void);
inline bool host_has_little_endianness(void);
inline bool has_host_endianness(tlm_endianness endianness);

} // namespace tlm
```

### 14.19.3 Rules

| 함수 | 규칙 |
|---|---|
| `get_host_endianness()` | host의 endianness를 **반환해야 shall** |
| `host_has_little_endianness()` | host가 little-endian일 때 **그리고 그때만** true를 반환해야 shall |
| `has_host_endianness(e)` | host endianness가 인자가 나타내는 것과 같을 때 **그리고 그때만** true를 반환해야 shall |

- d) **host가 little도 big도 아니면 위 세 함수의 반환값은 undefined다 shall.**

---

## 14.20 Helper functions for endianness conversion

### 14.20.1 Introduction

- data array 조직 규칙은 잘 정의되어 있고 단순한 경우 host-independent C++ 코드 작성은 어렵지 않다.
  그러나 non-aligned addressing과 socket width와 다른 data word width가 얽히면 복잡해지므로 helper function이 제공된다.
- **interoperability는 endianness 규칙 준수에만 의존하며, helper function 사용은 interoperability에 필수가 아니다.**
- 동기: initiator의 generic payload 생성 코드를 host endianness를 거의 신경 쓰지 않고 한 번 작성한 뒤,
  **단 한 번의 함수 호출로 host endianness에 맞게 변환**하는 것.
  각 변환 함수는 기존 transaction을 **in-place로 수정한다**.
- 함수는 **쌍(pair)으로 구성** — `to_hostendian` / `from_hostendian` — 이며 **항상 함께 써야 권장(should)**.
  `to_hostendian`은 initiator가 transport interface로 transaction을 보내기 **전**에,
  `from_hostendian`은 response를 받았을 때 호출해야 권장(should).
- 네 쌍이 제공된다: `_generic`이 가장 일반적·강력, `_word`·`_aligned`·`_single`은 제한된 경우만 처리하는 변형.
  **`_generic`의 변환은 계산 비용이 상대적으로 크므로, 가능하면 다른 함수를 선호해야 권장(should)**.
- **arithmetic mode** = 모델링 대상 컴포넌트의 endianness와 무관하게 산술 연산 효율을 위해 data word를
  host-endian 형식으로 저장하는 방식. **byte order mode** = host endianness를 무시하고 바이트를 주소 오름차순으로
  array에 저장하는 방식. **시뮬레이션 속도를 위해 arithmetic mode 사용이 권장(recommended)**.
  byte order mode는 data array 복사 시 byte swapping이 필요할 수 있다.
- **data word** 개념: TLM-2.0 socket width와도 generic payload data array의 word width와도 독립적.
  data word는 컴포넌트 모델 안에서 (모델링 대상의 endianness와 무관하게) **바이트를 host-endian 순서로 저장하는
  레지스터**를 나타내려는 의도. data word width가 socket width와 다르면 hostendian 함수가 endianness 변환을
  수행해야 할 수 있다. data word가 1바이트면 hostendian 함수는 사실상 byte order mode 변환을 수행한다.

### 14.20.2 Definition

```cpp
namespace tlm {

template<class DATAWORD>
inline void tlm_to_hostendian_generic(tlm_generic_payload *, unsigned int );
template<class DATAWORD>
inline void tlm_from_hostendian_generic(tlm_generic_payload *, unsigned int );

template<class DATAWORD>
inline void tlm_to_hostendian_word(tlm_generic_payload *, unsigned int);
template<class DATAWORD>
inline void tlm_from_hostendian_word(tlm_generic_payload *, unsigned int);

template<class DATAWORD>
inline void tlm_to_hostendian_aligned(tlm_generic_payload *, unsigned int);
template<class DATAWORD>
inline void tlm_from_hostendian_aligned(tlm_generic_payload *, unsigned int);

template<class DATAWORD>
inline void tlm_to_hostendian_single(tlm_generic_payload *, unsigned int);
template<class DATAWORD>
inline void tlm_from_hostendian_single(tlm_generic_payload *, unsigned int);

inline void tlm_from_hostendian(tlm_generic_payload *);

} // namespace tlm
```

### 14.20.3 Rules

- a) `to_hostendian` 계열의 첫 인자는 **transport interface로 보내도 유효할 generic payload transaction object의
  포인터여야 권장(should)**. transaction object를 생성·초기화한 **후**, IMC로 넘기기 **전**에만 호출해야 권장(should).
- b) `from_hostendian` 계열의 첫 인자는 **앞서 `to_hostendian`에 넘겼던 transaction object의 포인터여야 shall**.
  initiator가 response를 받았거나 transaction이 완료되었을 때만 호출해야 권장(should).
  **함수가 transaction과 그 array를 수정할 수 있으므로 transaction object lifetime 끝에서만 호출해야 권장(should)**.
- c) `to_hostendian`을 호출했다면 **같은 템플릿 인자·함수 인자로 대응하는 `from_hostendian`도 호출해야 권장(should)**.
  대안으로 `tlm_from_hostendian(tlm_generic_payload*)`를 호출해도 된다.
  이 함수는 transaction object에 (**ignorable extension으로**) 저장된 추가 context 정보를 써서 템플릿·함수 인자를
  복원하지만 **실행이 근소하게 느리다**.
- d) hostendian 함수의 둘째 인자는 **transaction이 통과하는 로컬 socket의 width를 바이트로** 표현한 값이어야 권장(should).
  이는 로컬 socket 기준 data array의 word length와 같다. **이 값은 2의 거듭제곱이어야 shall.**
- e) 템플릿 인자는 endianness 변환을 위한 **내부 initiator data word를 나타내는 타입이어야 권장(should)**.
  `sizeof(DATAWORD)`로 data word의 바이트 폭을 결정하고, 복사 시 `DATAWORD`의 assignment operator를 쓴다.
  **`sizeof(DATAWORD)`는 2의 거듭제곱이어야 shall.**
- f) **`to_hostendian` 구현은 context 정보를 저장하기 위해 transaction object에 extension을 추가한다.**
  따라서 **`to_hostendian`은 `from_hostendian` 호출 전에 한 번만 호출할 수 있다.**

**g) 모든 쌍에 공통인 제약** ("정수배"는 1×, 2×, 3×, … 를 뜻함):

1. socket width는 **2의 거듭제곱이어야 shall**
2. data word width는 **2의 거듭제곱이어야 shall**
3. streaming width attribute는 **data word width의 정수배여야 shall**
4. data length attribute는 **streaming width attribute의 정수배여야 shall**

**h) `hostendian_generic`**: 추가 제약 없음. **byte enable, streaming, non-aligned address·word width를 모두 지원**.

**i) `hostendian_word`, `hostendian_aligned`, `hostendian_single` 공통 추가 제약:**

1. data word width는 socket width 이하여야 shall — 따라서 socket width는 data word width의 **2의 거듭제곱 배**여야 shall
2. **streaming width attribute는 data length attribute와 같아야 shall** — 즉 **streaming 미지원**
3. byte enable granularity는 data word width보다 세밀하면 안 된다 shall —
   즉 **주어진 data word 안의 바이트는 전부 enabled이거나 전부 disabled여야 shall**
4. byte enable이 있으면 **byte enable length attribute는 data length attribute와 같아야 shall**

**j) `hostendian_aligned`만의 추가 제약:**

1. address attribute는 **socket width의 정수배여야 shall**
2. data length attribute는 **socket width의 정수배여야 shall**

**k) `hostendian_single`만의 추가 제약:**

1. data length attribute는 **data word width와 같아야 shall**
2. data array는 **data word 경계를 넘으면 안 되고 shall not**, 따라서 socket 경계도 넘으면 안 된다

---

## 14.21 Generic payload extensions

### 14.21.1.1 Overview

extension 메커니즘은 generic payload의 필수 구성 요소이며 **generic payload와 분리해 쓰도록 의도되지 않았다**.
목적은 generic payload에 attribute를 추가하는 것. extension은 **ignorable / non-ignorable, mandatory / non-mandatory**일 수 있다.

### 14.21.1.2 Ignorable extensions

- **ignorable = 해당 extension을 추가한 컴포넌트 외의 어떤 컴포넌트도 그 extension이 없는 것처럼 동작해도 된다.**
  따라서 추가한 컴포넌트는 다른 컴포넌트가 extension 존재에 반응한다고 **의존할 수 없고**,
  수신한 컴포넌트도 다른 컴포넌트가 그 extension을 인식했다고 의존할 수 없다.
  이 정의는 generic payload extension과 extended phase에 동일하게 적용된다.
- **컴포넌트는 ignorable extension의 부재 때문에 실패하면 안 되고 shall not, error response를 생성해도 안 된다 shall not.**
  이런 의미에서 ignorable extension은 **non-mandatory extension**이기도 하다.
  **ignorable extension의 존재 때문에는 실패하거나 error response를 생성해도 되지만 may, 무시하는 선택지도 있다.**
- 일반적으로 ignorable extension은 **명백하고 안전한 default 값이 존재해서** 어떤 interconnect나 target도
  extension 부재 시 그 default를 가정하고 정상 동작할 수 있는 것으로 볼 수 있다
  (예: transaction의 privilege level — default는 최저 레벨).
- 보조·side-band·시뮬레이션 관련 정보나 메타데이터 운반에 쓸 수 있다 (고유 transaction ID, 생성 wall-time, 진단 파일명 등).
- **ignorable extension은 base protocol이 허용한다.**

### 14.21.1.3 Non-ignorable and mandatory extensions

- **non-ignorable extension** = 존재하면 transaction을 받는 모든 컴포넌트가 반드시 반응할 의무가 있는 extension.
- **mandatory extension** = 반드시 존재해야 하는 extension.
- 둘 다 특정 프로토콜의 상세 모델링을 위해 generic payload를 특화할 때 써도 된다 may.
  **둘 다 새 protocol traits class의 정의를 요구한다.**

### 14.21.2 Rationale

두 가지: (1) generic payload core attribute 집합의 변형을 나르는 TLM-2.0 socket을 같은 traits class로 특화해
adaption/bridging 없이 직접 바인딩할 수 있게 함. (2) 같은 generic payload와 extension 메커니즘 기반의
서로 다른 프로토콜 간 손쉬운 adaption. extension 메커니즘이 없다면 attribute 추가마다 새 transaction class 정의가 필요해
다수의 adapter가 필요해진다.

### 14.21.3 Extension pointers, objects and transaction bridges

- extension은 **`tlm_extension`에서 파생된 타입의 객체**. generic payload는 **extension object 포인터 array**를 갖는다.
  **모든 generic payload object는 모든 extension 타입의 인스턴스를 하나씩 나를 수 있다.**
- 포인터 array는 **등록된 모든 extension마다 slot**을 갖는다. `set_extension`은 단순히 포인터를 덮어쓰며
  원칙적으로 initiator·interconnect·target 어디서든 호출할 수 있다. 유연하지만 오용되기 쉬우므로
  **extension object의 소유권과 삭제를 사용자가 잘 이해하고 신중히 고려해야 한다.**
- 두 개의 별도 generic payload transaction 사이에 **transaction bridge**를 만들 때는,
  필요하다면 incoming transaction의 extension을 outgoing transaction으로 복사하고,
  **outgoing transaction과 그 extension을 소유·관리할 책임이 bridge에 있다.**
  data array와 byte enable array도 마찬가지. `deep_copy_from`과 `update_original_from`이 이를 위해 제공된다.
  bridge가 outgoing에 extension을 더 추가하면 그 extension은 bridge 소유.

### 14.21.4 Rules

**생성·정의**

- a) extension은 **initiator, interconnect, target 어느 컴포넌트든 추가할 수 있다.** initiator에 국한되지 않는다.
- b) **generic payload 인스턴스마다 임의 개수의 extension을 추가해도 된다 may.**
- c) ignorable extension의 경우 (추가한 컴포넌트를 제외한) **어떤 컴포넌트든 무시하는 것이 허용**되며,
  ignorable extension은 mandatory extension이 아니다. ignorable extension의 부재나 그에 대한 응답의 부재 때문에
  컴포넌트가 실패한다면 interoperability가 파괴된다.
- d) **주어진 extension의 존재를 강제하는 내장 메커니즘도, extension이 ignorable함을 보장하는 메커니즘도 없다.**
- e) **각 extension의 semantics는 application-defined여야 shall. 미리 정의된 extension은 없다.**
- f) extension은 **사용자 정의 class를 `tlm_extension`에서 파생시켜 만들어야 shall** —
  **사용자 정의 class 이름 자체를 `tlm_extension`의 템플릿 인자로 넘기고**, 그 class의 객체를 생성.
  사용자 정의 extension class는 generic payload의 확장 attribute를 나타내는 멤버를 포함해도 된다.
- g) `tlm_extension_base`의 virtual 멤버 함수 **`free`는 extension object를 delete해야 shall**.
  사용자 정의 memory management를 위해 오버라이드해도 되지만 **필수는 아니다**.
- h) `tlm_extension`의 pure virtual 함수 **`clone`은 사용자 정의 extension class에서 정의되어 확장 attribute를 포함해
  extension object를 clone해야 shall**. generic payload memory management와 함께 쓰려는 의도.
  **원본 객체가 소멸되어도 사본이 가시적 side-effect 없이 살아남을 수 있도록 사본을 생성해야 shall.**
- i) pure virtual 함수 **`copy_from`은 다른 extension object의 attribute를 복사해 현재 extension object를
  수정하도록 사용자 정의 class에서 정의되어야 shall**.

**ID 할당 (j~n)**

- j) **class template `tlm_extension`을 인스턴스화하는 행위 자체가 public data member `ID`를 초기화해야 shall**하고,
  이것이 **해당 extension을 generic payload에 등록하고 고유 ID를 할당하는 효과를 가져야 shall**.
  **ID는 실행 중인 프로그램 전체에서 고유해야 shall.**
  즉 **`tlm_extension`의 각 인스턴스화는 서로 다른 ID를 가져야 shall**하고,
  **주어진 타입의 모든 extension object는 같은 ID를 공유해야 shall**.
- k) generic payload는 **재조정 가능한(re-sizable) array에 extension 포인터를 저장하는 것처럼 동작해야 shall**하며,
  **extension의 ID가 그 array 내 포인터의 index를 준다.**
  extension 등록은 그 extension을 위한 array index를 예약해야 shall.
  **각 generic payload object는 현재 실행 프로그램에 등록된 모든 extension의 포인터를 저장할 수 있는 array를
  포함해야 shall.**
- l) **transaction 생성 시 extension array의 포인터들은 null이어야 shall.**
- m) **각 generic payload object는 주어진 extension 타입당 최대 하나의 객체 포인터만 저장할 수 있다**
  (서로 다른 타입은 여러 개 가능). 같은 타입의 extension 객체를 여러 개 참조하려면
  utility class **`instance_specific_extension`**을 쓸 수 있다 (16.5).
- n) **`max_num_extensions`는 extension 타입의 개수(= extension array의 크기)를 반환해야 shall.**
  따라서 **extension 타입은 0부터 `max_num_extensions()-1`까지 번호가 매겨져야 shall.**

**접근 함수 (o~y)**

- o) `set_extension`, `set_auto_extension`, `get_extension`, `clear_extension`, `release_extension`은
  여러 형태로 제공되며 extension을 각각 다른 방식으로 지정한다: **함수 템플릿, extension 포인터 인자, ID 인자**.
  **ID 인자를 쓰는 형태는 generic payload object를 clone할 때 같은 전문적 프로그래밍 작업용이며
  일반 애플리케이션 용도가 아니다.**

| 함수 | 규칙 |
|---|---|
| `set_extension(T*)` (p) | 타입 T의 extension object 포인터를 인자 값으로 **설정해야 shall**. 인자는 **등록된 extension의 포인터여야 shall**. **반환값은 null pointer여도 된다 may.** 이전 표준 버전에서는 같은 타입의 기존 extension 포인터의 이전 값을 반환하려는 의도였으므로, 하위 호환이 필요하면 null이 아닌 값을 반환해도 된다. |
| `set_auto_extension(T*)` (p) | 위와 유사하되 **extension을 자동 삭제(auto deletion)로 표시해야 shall** |
| `set_extension(unsigned int, tlm_extension_base*)` (q) | 첫 인자가 주는 array index 위치의 포인터를 둘째 인자 값으로 **설정해야 shall**. **주어진 index는 extension ID로 등록되어 있어야 shall — 아니면 함수의 동작은 undefined다.** **반환값은 해당 index의 이전 포인터 값이어야 shall** (null일 수 있음). |
| `set_auto_extension(unsigned int, tlm_extension_base*)` (q) | 위와 유사하되 자동 삭제 표시 |
| **(p, q 공통)** | **이 멤버 함수들로 같은 타입의 extension을 여러 개 같은 generic payload에 붙이면 안 된다 shall not. 그런 구현은 standard error response를 생성해야 shall.** |
| `get_extension(T*&)` / `T* get_extension()` (u) | 주어진 타입의 extension object 포인터를 **반환해야 shall** — 없으면 **null pointer**. 타입 T는 `tlm_extension`에서 파생된 타입의 객체 포인터여야 shall. **존재하지 않는 extension을 이 함수 템플릿으로 조회하는 것은 error가 아니다.** |
| `get_extension(unsigned int)` (v) | 인자가 주는 ID의 extension object 포인터를 **반환해야 shall**. **주어진 index는 extension ID로 등록되어 있어야 shall — 아니면 동작은 undefined다.** 해당 index의 포인터가 extension object를 가리키지 않으면 **null pointer를 반환해야 shall**. |
| `clear_extension(const T*)` / `clear_extension<T>()` (w) | 주어진 extension을 generic payload object에서 **제거해야 shall** — 즉 extension array의 대응 포인터를 **null로 설정해야 shall**. 인자가 있으면 `tlm_extension` 파생 타입 객체의 포인터여야 shall. **`clear_extension`은 extension object를 delete하면 안 된다 shall not.** |
| `release_extension(T*)` / `release_extension<T>()` (x) | transaction object에 **memory manager가 있으면 extension을 자동 삭제로 표시해야 shall**. **없으면 extension object의 `free`를 호출해 delete하고 대응 포인터를 null로 설정해야 shall.** 인자가 있으면 `tlm_extension` 파생 타입 객체의 포인터여야 shall. |

- r) **memory manager가 있을 때 `set_auto_extension` 호출은 `set_extension` 호출 직후 같은 extension에 대해
  `release_extension`을 호출하는 것과 동등하다.**
  **memory manager가 없을 때 `set_auto_extension` 호출은 run-time error를 일으킨다.**
- s) extension이 자동 삭제로 표시되면 그 extension object는 **사용자 정의 memory manager의 `free` 구현이
  delete하거나 pooling해야 권장(should)**. `free`는 transaction object의 reference count가 0이 될 때
  `tlm_generic_payload::release`가 호출한다. extension object는 `tlm_generic_payload::reset` 호출로,
  또는 extension object 자신의 `free` 호출로 delete될 수 있다.
- t) generic payload object가 이미 설정 대상 타입의 non-null extension 포인터를 갖고 있으면 **이전 포인터는 덮어써진다**.
- y) **`release_extension`의 동작은 transaction object의 memory manager 유무에 의존한다.**
  **memory manager가 있으면 자동 삭제 표시만 되고 extension은 계속 접근 가능**.
  **memory manager가 없으면 포인터가 지워질 뿐 아니라 extension object 자체가 delete된다.**
  **존재하지 않는 extension object를 release하지 않도록 주의해야 권장(should)** — **run-time error가 발생한다.**
- z) **`clear_extension`과 `release_extension`은 자동 삭제로 표시된 extension에 대해 호출하면 안 된다 shall not**
  (예: `set_auto_extension`으로 설정했거나 이미 `release_extension`으로 release된 것).
  **그렇게 하면 run-time error가 발생할 수 있다.**

**array 크기 조정 (aa, ab)**

- aa) 각 generic payload transaction은 **등록된 모든 extension의 포인터를 저장할 충분한 공간을 할당해야 권장(should)**.
  두 방법 중 하나로 달성:
  **(1) C++ static initialization 이후에 transaction object를 생성**하거나,
  **(2) static initialization 이후 transaction object를 처음 쓰기 전에 `resize_extensions`를 호출**.
  (1)에서는 extension array 크기 설정 책임이 generic payload 생성자에 있고,
  (2)에서는 extension을 처음 접근하기 전에 `resize_extensions`를 호출할 책임이 애플리케이션에 있다.
- ab) **`resize_extensions`는 등록된 모든 extension을 수용하도록 generic payload의 extension array 크기를
  늘려야 shall.**

### Example — ignorable extension

```cpp
// 사용자 정의 extension class
struct ID_extension : tlm::tlm_extension<ID_extension> {

    ID_extension() : transaction_id(0) {}

    virtual tlm_extension_base *clone() const { // pure virtual clone을 반드시 오버라이드
         ID_extension *t = new ID_extension;
         t->transaction_id = this->transaction_id;
         return t;
    }

    // pure virtual copy_from을 반드시 오버라이드
    virtual void copy_from(tlm_extension_base const &ext) {
         transaction_id = static_cast<ID_extension const &>(ext).transaction_id;
    }

    unsigned int transaction_id;
};

// The initiator
struct Initiator : sc_core::sc_module {
    ...
    void thread() {
        tlm::tlm_generic_payload trans;
        ...
        ID_extension *id_extension = new ID_extension;
        trans.set_extension(id_extension);       // transaction에 extension 추가

        for (int i = 0; i < RUN_LENGTH; i += 4) {
        ...
            ++id_extension->transaction_id;      // 새 transaction마다 id 증가
            ...
            socket->b_transport(trans, delay);
            ...
        }
    }
}

// The target
virtual void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t) {
    ...
    ID_extension *id_extension;
    trans.get_extension(id_extension);                 // extension 조회
    if (id_extension) {                                // extension은 mandatory가 아님
         char txt[80];
         sprintf(txt, "Received transaction id %d", id_extension->transaction_id);
         SC_REPORT_INFO("TLM-2.0", txt);
    }
    ...
}
```

### Example — mandatory extension을 가진 새 protocol traits class

```cpp
struct cmd_extension : tlm::tlm_extension<cmd_extension> { // 사용자 정의 mandatory extension class
    cmd_extension() : increment(false) {}

    virtual tlm::tlm_extension_base *clone() const {
         cmd_extension *t = new cmd_extension;
         t->increment = this->increment;
         return t;
    }

    virtual void copy_from(tlm::tlm_extension_base const &ext) {
         increment = static_cast<cmd_extension const &>(ext).increment;
    }

    bool increment;
};

struct my_protocol_types {                       // 사용자 정의 protocol traits class
    typedef tlm::tlm_generic_payload tlm_payload_type;
    typedef tlm::tlm_phase tlm_phase_type;
};

struct Initiator : sc_core::sc_module {
    tlm_utils::simple_initiator_socket<Initiator, 32, my_protocol_types> socket;
    ...
    void thread() {
        tlm::tlm_generic_payload trans;
        cmd_extension *extension = new cmd_extension;
        trans.set_extension(extension);                  // transaction에 extension 추가
        ...
        trans.set_command(tlm::TLM_WRITE_COMMAND);       // write command 실행
        socket->b_transport(trans, delay);
        ...
        trans.set_command(tlm::TLM_IGNORE_COMMAND);
        extension->increment = true;                     // increment command 실행
        socket->b_transport(trans, delay);
        ...
    }
    ...
}

// The target
tlm_utils::simple_target_socket<Memory, 32, my_protocol_types> socket;

virtual void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t) {
    tlm::tlm_command cmd = trans.get_command();
    ...
    cmd_extension *extension;
    trans.get_extension(extension);                      // command extension 조회
    ...
    if (!extension) {                                    // extension 존재 확인
         trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
         return;
    }

    if (extension->increment) {
         if (cmd != tlm::TLM_IGNORE_COMMAND) {           // read/write와의 충돌 검출
              trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
              return;
         }
         ++m_storage[adr];                               // increment command 실행
         memcpy(ptr, &m_storage[adr], len);
    }
    ...
}
```

---

## error / undefined / implementation-defined 조건 총정리

**error (run-time error 포함)**

- §14.11 e) read/write command에서 **data pointer가 null**인 transaction으로 transport interface 호출 → **error**
- §14.5 q) memory manager 없이 `acquire` 호출 → **run-time error**
- §14.5 r) memory manager 없이 `release` 호출 → **run-time error**
- §14.21.4 r) memory manager 없이 `set_auto_extension` 호출 → **run-time error**
- §14.21.4 y) 존재하지 않는 extension object를 `release_extension` → **run-time error**
- §14.21.4 z) 자동 삭제 표시된 extension에 `clear_extension`/`release_extension` → **run-time error 가능**
- §14.21.4 p), q) 같은 타입의 extension을 여러 개 붙이는 구현 → **standard error response를 생성해야 shall**

**undefined**

- §14.12 e) `TLM_IGNORE_COMMAND`이고 data pointer가 null이면 **data length 값은 undefined**
- §14.13 b) byte enable array 원소가 0도 0xff도 아닌 값이면 **의미는 undefined**
- §14.7 e) forward path IMC에서 반환된 후의 **address attribute는 routing 목적으로 undefined로 간주해야 권장**
- §14.19.3 d) host가 little도 big도 아니면 **`get_host_endianness`·`host_has_little_endianness`·`has_host_endianness`의
  반환값은 undefined**
- §14.21.4 q), v) 등록되지 않은 index를 `set_extension(unsigned int, ...)` / `get_extension(unsigned int)`에 넘기면
  **함수 동작은 undefined**

**implementation-defined**

- §14.3 generic payload attribute access function의 **정확한 구현은 implementation-defined**

**invalid values**

- §14.12 i) data length 기본값 0은 **data pointer가 null이 아닌 한 invalid**
- §14.15 f) **streaming width 0은 invalid shall**

**표준이 강제를 금지하는 것**

- §14.17.2 target이 세 가지 행동 중 정확히 하나를 하도록 **구현이 강제할 의무도 없고 강제가 허용되지도 않는다**

---

## 흔한 위반 (pitfalls)

- **`nb_transport`에 memory manager 없는 transaction을 넘김** — LRM §14.5 i): non-blocking transport에서는
  memory manager를 반드시 써야 shall하고, `nb_transport` 인자로 넘어가는 transaction은 이미 memory manager가
  설정되어 있어야 shall. `tlm_generic_payload(tlm_mm_interface*)` 생성자나 `set_mm`을 쓸 것.
- **transport 호출마다 generic payload를 생성·소멸** — LRM §14.5 d): extension array 때문에 매우 느리며 피해야 권장(should).
  transaction pool을 쓰거나 같은 객체를 `b_transport` 호출들에 재사용할 것.
- **data length를 설정하지 않고 transport 호출** — LRM §14.12 i): 기본값 0은 data pointer가 null이 아닌 한 invalid.
  명시적으로 설정해야 shall.
- **streaming width를 0으로 두고 read/write** — LRM §14.15 f): streaming width 0은 invalid shall.
  streaming이 필요 없으면 data length 이상의 값으로 설정해야 권장(should).
- **`response_status`를 초기화하지 않고 transaction 재사용** — LRM §14.17.1 e): initiator가
  `TLM_INCOMPLETE_RESPONSE`로 설정해야 shall. pooling된 객체에서 이전 응답이 남아 있기 쉽다 (§14.7 a).
- **interconnect가 downstream에 넘긴 뒤 address를 다시 수정** — LRM §14.7 d): 일단 downstream에 참조를 넘기면
  transaction 전체 lifetime 동안 address 재수정이 허용되지 않는다.
- **target이 write command에서 data array를 수정** — LRM §14.7 f), §14.11 i): write와 `TLM_IGNORE_COMMAND`에서
  data array 내용은 initiator가 설정하며 어떤 interconnect·target도 덮어쓰면 안 된다 shall not.
  target이 수정해도 되는 것은 **read command일 때뿐**.
- **target이 response 전송 후에 response status/DMI allowed/data array를 수정** — LRM §14.7 g):
  `b_transport` 반환, `BEGIN_RESP` 전달, `TLM_COMPLETED` 반환 후에는 수정이 허용되지 않는다.
- **generic payload destructor가 data array를 해제할 것으로 기대** — LRM §14.5 a), §14.6 d):
  destructor는 data array도 byte enable array도 delete하지 않는다 shall not. 저장소 소유는 initiator 책임.
- **byte enable length를 0으로 두고 byte enable pointer만 설정** — LRM §14.14 f):
  byte enable pointer가 non-0이면 byte enable length는 non-0이어야 shall.
- **byte enable array에 0/0xff 외의 값 사용** — LRM §14.13 b): 그 외 값의 의미는 undefined.
  `TLM_BYTE_DISABLED` / `TLM_BYTE_ENABLED` 매크로를 쓸 것.
- **byte enable을 data array 전체 길이로만 만들 수 있다고 가정** — LRM §14.14 d):
  `byte_enable_array_index = data_array_index % byte_enable_length`로 **반복 적용**된다.
- **`clear_extension`이 extension을 delete한다고 착각** — LRM §14.5 z), §14.21.4 w):
  `clear_extension`은 포인터만 지우고 객체를 delete하지 않는다 shall not. delete가 필요하면 `release_extension`.
- **memory manager 없이 `set_auto_extension` 호출** — LRM §14.21.4 r): run-time error.
  memory manager 유무를 모를 때는 `set_extension` + `release_extension` 조합이 안전 (§14.5 ac).
- **자동 삭제 표시된 extension을 다시 release/clear** — LRM §14.21.4 z): run-time error 가능.
- **static initialization 중에 생성한 transaction에서 extension 접근** — LRM §14.21.4 aa):
  static initialization 이후에 객체를 생성하거나, 첫 사용 전에 `resize_extensions`를 호출해야 권장(should).
- **blocking/non-blocking transport에서 option attribute를 `TLM_MIN_PAYLOAD` 외의 값으로 설정** —
  LRM §14.8 g): transport interface에서는 `TLM_MIN_PAYLOAD`여야 shall하고 어떤 컴포넌트도 수정하면 안 된다 shall not.
- **`TLM_FULL_PAYLOAD`를 쓰고 lifetime 끝에서 되돌리지 않음** — LRM §14.8 p): memory manager가 없으면
  initiator가 명시적으로 `TLM_MIN_PAYLOAD`로 되돌릴 의무가 있다.
- **ignorable extension의 부재를 이유로 error response 반환** — LRM §14.21.1.2: 컴포넌트는 ignorable extension의
  부재 때문에 실패하거나 error response를 생성하면 안 된다 shall not.
- **target이 지원 못 하는 기능을 조용히 무시** — LRM §14.2.2 d), §14.17.2: 지원할 수 없는 기능이 있으면
  standard error response를 생성할 의무가 있다 (또는 SystemC report + `TLM_OK_RESPONSE`).
- **`to_hostendian`을 `from_hostendian` 없이 두 번 호출** — LRM §14.20.3 f): `to_hostendian`이 context 저장용
  extension을 추가하므로 `from_hostendian` 전에 한 번만 호출할 수 있다.
- **`acquire` 없이 IMC 반환 후 transaction이 유효하다고 가정** — LRM §14.5 v): 다른 컴포넌트가 어느 IMC나
  프로세스에서든 `release`를 호출할 수 있다.
- **reference count로 분석용 lifetime을 무기한 연장** — LRM §14.5 w): reference count를 프로토콜의 정상 phase를
  넘어선 lifetime 연장에 쓰면 안 된다 (should not). clone할 것.
