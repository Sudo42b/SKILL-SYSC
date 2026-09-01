# Clause 17 — TLM-1 message passing interface and analysis ports

LRM pp. 575–589.

TLM-1 메시지 패싱 인터페이스(put/get/peek/transport, write/analysis), `tlm_fifo` 채널, analysis port, analysis fifo.
**TLM-1 transport 인터페이스는 TLM-2.0 transport 인터페이스와 별개다.**

각 절은 LRM 형식을 따른다: Description → Class definition → Rules / Member functions → Example.

---

## 17.1 개요

TLM-1 message passing interface의 구성:

| 구성 요소 | 내용 |
|---|---|
| blocking / non-blocking 인터페이스 | put, get, peek, transport |
| write / analysis 인터페이스 | `tlm_write_if`, `tlm_analysis_if` |
| 채널 | `tlm_fifo` |
| 분석용 | analysis port, analysis fifo |

---

## 17.2 Put, get, peek, and transport interfaces

### 17.2.1 Description

TLM-1과 TLM-2 core interface의 근본적 차이:

| | TLM-2 core interfaces | TLM-1 interfaces |
|---|---|---|
| 트랜잭션 전달 | 참조로 전달, 객체 수명이 **여러 IMC에 걸칠 수 있음** | message-passing 의미론 |
| 공유 메모리 | 사용 | caller/callee 간 **공유 메모리가 없어야 하는 것이 의도** |
| 추상화 | — | IMC가 한 모듈의 내부 상태 변화를 다른 모듈로부터 숨기는 것이 의도 |

- TLM-1 **blocking** 인터페이스 = synchronous message passing.
- TLM-1 **non-blocking** 인터페이스 = asynchronous message passing.

**단방향성**: TLM-1 메시지 패싱(= 트랜잭션 패싱)은 unidirectional. TLM-1 양방향 transport 인터페이스는 서로 반대 방향으로 별개의 메시지를 전달하는 두 개의 단방향 메시지 채널로 볼 수 있다.

**effective pass-by-value 의미론**:
- 추상 수준에서 의도 — 트랜잭션(put으로 전달되든 get으로 전달되든)의 recipient는 sender가 보낸 것과 **정확히 같은 값**을 받아야 한다.
- 일반적인 메시지 패싱 시스템은 이를 strict pass-by-value로 구현하지만, TLM-1은 소위 **effective pass-by-value** 의미론을 사용한다: 트랜잭션이 경우에 따라 참조로 전달될 수 있지만, 일단 sender가 값을 대입한 이후에는 **caller도 callee도 트랜잭션 객체를 수정하는 것이 허용되지 않는다**.

**데이터 타입 제약(요약)**: 트랜잭션 객체의 데이터 타입은 deep copy 의미론을 지원해야 권장(should) — C++ 초기화(복사 생성자) 또는 대입으로 복사본을 뜬 뒤 그 복사본을 수정해도 원본이 바뀌지 않아야 한다. 트랜잭션 객체가 자기 바깥의 공유 메모리에 대한 포인터/참조를 담고 있다면, **본 표준은 그런 공유 메모리 위치의 소유권·수명·접근·갱신 규칙을 규정하지 않는다**. 따라서 그런 공유 메모리 사용의 책임은 전적으로 애플리케이션에 있으며, 신중히 문서화되어야 권장(should).

**용어**:

| 용어 | put의 경우 | get / peek의 경우 | transport의 경우 |
|---|---|---|---|
| **sender** | caller | callee | caller = request의 sender, callee = response의 sender |
| **recipient** | callee | caller | caller = response의 recipient, callee = request의 recipient |

**transaction object** = 멤버 함수 `put`, `nb_put`, `nb_get`, `nb_peek`, `transport`에 인자로 전달되는 객체, 또는 멤버 함수 `get`, `peek`, `transport`의 반환값으로 나오는 객체.

### 17.2.2 Class definition

```cpp
namespace tlm {

template<class T>
class tlm_tag {};

// Uni-directional blocking interfaces
template < typename T >
class tlm_blocking_put_if : public virtual sc_core::sc_interface
{
public:
    virtual void put( const T &t ) = 0;
};

template < typename T >
class tlm_blocking_get_if : public virtual sc_core::sc_interface
{
public:
    virtual T get( tlm_tag<T> *t = 0 ) = 0;
    virtual void get( T &t ) { t = get(); }
};

template < typename T >
class tlm_blocking_peek_if : public virtual sc_core::sc_interface
{
public:
    virtual T peek( tlm_tag<T> *t = 0 ) const = 0;
    virtual void peek( T &t ) const { t = peek(); }
};

// Uni-directional non blocking interfaces
template < typename T >
class tlm_nonblocking_put_if : public virtual sc_core::sc_interface
{
public:
    virtual bool nb_put( const T &t ) = 0;
    virtual bool nb_can_put( tlm_tag<T> *t = 0 ) const = 0;
    virtual const sc_core::sc_event &ok_to_put( tlm_tag<T> *t = 0 ) const = 0;
};

template < typename T >
class tlm_nonblocking_get_if : public virtual sc_core::sc_interface
{
public:
    virtual bool nb_get( T &t ) = 0;
    virtual bool nb_can_get( tlm_tag<T> *t = 0 ) const = 0;
    virtual const sc_core::sc_event &ok_to_get( tlm_tag<T> *t = 0 ) const = 0;
};

template < typename T >
class tlm_nonblocking_peek_if : public virtual sc_core::sc_interface
{
public:
    virtual bool nb_peek( T &t ) const = 0;
    virtual bool nb_can_peek( tlm_tag<T> *t = 0 ) const = 0;
    virtual const sc_core::sc_event &ok_to_peek( tlm_tag<T> *t = 0 ) const = 0;
};

// Uni-directional combined blocking and non blocking interfaces
template < typename T >
class tlm_put_if :
    public virtual tlm_blocking_put_if< T > ,
    public virtual tlm_nonblocking_put_if< T > {};

template < typename T >
class tlm_get_if :
    public virtual tlm_blocking_get_if< T > ,
    public virtual tlm_nonblocking_get_if< T > {};

template < typename T >
class tlm_peek_if :
    public virtual tlm_blocking_peek_if< T > ,
    public virtual tlm_nonblocking_peek_if< T > {};

// Uni-directional combined get-peek interfaces
template < typename T >
class tlm_blocking_get_peek_if :
    public virtual tlm_blocking_get_if<T> ,
    public virtual tlm_blocking_peek_if<T> {};

template < typename T >
class tlm_nonblocking_get_peek_if :
    public virtual tlm_nonblocking_get_if<T> ,
    public virtual tlm_nonblocking_peek_if<T> {};

template < typename T >
class tlm_get_peek_if :
    public virtual tlm_get_if<T> ,
    public virtual tlm_peek_if<T> ,
    public virtual tlm_blocking_get_peek_if<T> ,
    public virtual tlm_nonblocking_get_peek_if<T> {};

// Bidirectional blocking transport interface
template < typename REQ , typename RSP >
class tlm_transport_if : public virtual sc_core::sc_interface
{
public:
    virtual RSP transport( const REQ& ) = 0;
    virtual void transport( const REQ& req , RSP& rsp ) { rsp = transport( req ); }
};

} // namespace tlm
```

### 17.2.3 Blocking versus non-blocking interfaces

| # | 규칙 |
|---|---|
| a | `put`, `get`, `peek`, `transport`는 **blocking** interface method다 |
| b | `nb_put`, `nb_can_put`, `ok_to_put`, `nb_get`, `nb_can_get`, `ok_to_get`, `nb_peek`, `nb_can_peek`, `ok_to_peek`는 **non-blocking** interface method다 |
| c | Blocking interface method는 직접 또는 간접적으로 `wait`를 호출해도 된다 may |
| d | Blocking interface method는 **method process에서 호출하면 안 된다 shall not** |
| e | Non-blocking interface method는 직접 또는 간접적으로 `wait`를 **호출하면 안 된다 shall not** |
| f | Non-blocking interface method는 thread process에서든 method process에서든 호출해도 된다 may |

### 17.2.4 Blocking interface methods

- **a) 연속 호출 = 서로 다른 트랜잭션 인스턴스** — (같은 port/export를 통한) `put`의 연속 호출과 `get`의 연속 호출은, 매번 같은 트랜잭션 객체를 전달하는지 여부와 무관하게, 서로 구별되는 트랜잭션 인스턴스를 나타내야 shall. 즉 `put`의 매 호출마다 caller는 시퀀스상 다음 트랜잭션을 전달해야 shall, `get`의 매 반환마다 callee는 시퀀스상 다음 트랜잭션을 반환해야 shall.
- **b) `put`** — recipient가 트랜잭션 객체를 **수락하기 전에는 반환하면 안 된다 shall not**. 즉 recipient가 트랜잭션 객체를 실행했거나, 복사했거나, 하류로 넘길 때까지. 다시 말해 `put`에서 반환되면 caller는 callee가 그 단계에서 적절한 처리를 모두 마쳤다고 가정할 수 있다. 트랜잭션은 `put` 본문 내에서 실행될 수도 있고, `put`이 이후 처리를 위해 복사본을 떠 둘 수도 있다. 어느 경우든 caller는 곧바로 다음 트랜잭션 전송을 시도해도 된다 may.
- **c) `get`** — 다음 트랜잭션 객체가 반환 준비될 때까지 **반환하면 안 된다 shall not**. 반환되면 caller는 callee가 처리 준비된 유효한 트랜잭션 객체를 반환하고 있다고 가정할 수 있으며, 곧바로 다음 트랜잭션 get을 시도해도 된다.
- **d) `peek`** — 다음 트랜잭션 객체가 반환 준비될 때까지 반환하면 안 된다 shall not. 단 `get`과 달리 `peek`은 트랜잭션을 callee에서 **제거하면 안 된다 shall not**. 즉 (같은 port/export를 통해, 중간에 `get` 호출 없이) `peek`을 연속 호출하면 같은 트랜잭션 인스턴스를 나타내는 같은 트랜잭션 객체를 반환해야 shall. 마찬가지로 `peek` 한 번 뒤에 `get` 한 번을 호출하면 둘 다 같은 트랜잭션 인스턴스를 나타내는 같은 트랜잭션 객체를 반환해야 shall.
- **e) `transport`** — 두 개의 단방향 트랜잭션(caller→callee의 request 객체, callee→caller의 response 객체)을 결합해야 shall. `transport` 구현은 **request를 전달하는 `put` 호출 + 대응 response를 반환하는 `get` 호출과 의미론적으로 동등해야 shall**. callee가 반환하는 response 객체는 주어진 request 객체에 대한 callee의 응답을 나타내야 shall. 즉 전체 왕복 트랜잭션이 `transport` **단일 호출 안에서** 실행되어야 shall.

### 17.2.5 Non-blocking interface methods

- **a) 반환값의 의미** — non-blocking interface method는 callee가 다음 트랜잭션을 (nb_put의 경우) 즉시 수락할 수 있는지, (nb_get / nb_peek의 경우) 즉시 반환할 수 있는지를, 즉 현재 non-blocking 메서드 호출의 실행의 일부로서 판정할 수 있는지에 의존한다. callee가 즉시 응답할 수 있으면 non-blocking 멤버 함수(`nb_put`, `nb_can_put`, `nb_get`, `nb_can_get`, `nb_peek`, `nb_can_peek`)는 **`true`를 반환해야 shall**. 그렇지 않으면 **`false`를 반환해야 shall**, 그리고 다음 트랜잭션을 수락하거나 반환하면 안 된다 shall not.
- **b) true 반환 시** — `nb_put`, `nb_get`, `nb_peek`가 `true`를 반환하면 각각 대응하는 blocking 멤버 함수 `put`, `get`, `peek`와 동일하게 동작해야 shall. 단 `wait`를 호출하지 않고 즉시 반환해야 shall.
- **c) false 반환 시** — `nb_put`, `nb_can_put`, `nb_get`, `nb_can_get`, `nb_peek`, `nb_can_peek`가 `false`를 반환하면 각각 트랜잭션을 수락하거나 반환하지 않고 즉시 반환해야 shall. 즉 호출에 사용된 특정 port/export를 통한 트랜잭션 송수신에 관해 callee의 상태를 **변경하면 안 된다 shall not**.
- **d) put 계열 일관성** — `nb_put`과 `nb_can_put`은 주어진 시각에 주어진 port/export를 통해 서로 대체되어 호출되었을 때 **같은 값(true 또는 false)을 반환해야 shall**. 반환값은 인자로 전달된 트랜잭션 객체의 값에 **의존하면 안 된다 shall not**.
- **e) get/peek 계열 일관성** — 마찬가지로 `nb_get`, `nb_can_get`, `nb_peek`, `nb_can_peek`은 주어진 시각에 주어진 port/export를 통해 서로 대체되어 호출되었을 때 같은 값을 반환해야 shall. 즉 callee가 `nb_get`에 true를 반환하면서 `nb_can_get`에 false를 반환하는 것(및 그 역), `nb_can_get`에 true를 반환하면서 `nb_can_peek`에 false를 반환하는 것(및 그 역)은 **허용되지 않는다**.
- **f) `ok_to_put` / `ok_to_get` / `ok_to_peek`** — 각각 callee가 다음 트랜잭션을 수락하거나 반환할 준비가 될 때마다 callee가 notify하는 `sc_event`를 반환해야 shall. 이 이벤트의 notification이 caller가 같은 port/export를 통해 대응 non-blocking 메서드를 호출해 다음 트랜잭션을 put/get/peek 시도하도록 하는 신호(cue)가 되는 것이 의도다. 그러나 caller는 이 이벤트 notification 직후 non-blocking 메서드가 반드시 `true`를 반환하리라 **가정할 수 없다** — caller는 여전히 non-blocking 메서드의 반환값을 확인할 의무가 있다.
- **g) blocking vs non-blocking 일관성 의무 없음** — 같은 port/export를 통해 대응하는 blocking / non-blocking 메서드가 서로 대체되어 호출될 때 일관된 동작을 해야 할 **의무는 없다**. 다만 그렇게 하는 것이 권장된다(recommended). 예: `put`이 즉시 반환할 상황이면 `nb_put`/`nb_can_put`은 통상 `true`를 반환해야 권장(should)이나 그럴 의무는 없다. 마찬가지로 `put`이 `wait`를 호출할 상황이면 `nb_put`/`nb_can_put`은 통상 `false`를 반환해야 권장(should)이나 역시 의무는 아니다.

### 17.2.6 Argument passing and transaction lifetime

- **a) `tlm_tag<T>*` 인자** — 트랜잭션 타입만 다른 TLM-1 인터페이스의 인스턴스화가 여러 개 존재할 때 caller가 템플릿 인스턴스를 구별하는 데 사용해도 된다 may. caller는 C++ 언어 규칙상 메서드 호출을 모호성 없이 결정하기 위해 필요한 경우를 제외하면 이 인자에 값을 줄 의무가 없다. **interface method 본문은 이 인자를 사용하면 안 된다 shall not.**
- **b) `put`, `nb_put`, `transport`** (트랜잭션 객체를 `const T&`로 전달) — caller는 메서드 호출을 실행하기 **전에** 실제 인자(put/nb_put의 유일한 인자, transport의 첫 인자)로 전달되는 트랜잭션 객체를 보내려는 트랜잭션 인스턴스를 나타내는 값으로 초기화하거나 대입해야 shall.
- **c) `get`, `peek`, `nb_get`, `nb_peek`, `transport`** (트랜잭션 객체를 비-const 참조 인자 `T&`로 반환) — callee는 반환하려는 트랜잭션 인스턴스를 나타내는 값을 형식 인자에 대입해야 shall(`get`, `peek`, `transport`) / 대입해도 된다 may(`nb_get`, `nb_peek`).
- **d) 호출 중 수정 금지** — 위 두 경우 모두, 실제 인자 또는 형식 인자에 값을 초기화/대입한 이후에는 interface method 호출에서 반환될 때까지 **caller도 callee도 이 트랜잭션 객체의 값을 (직접이든 간접이든) 수정하면 안 된다 shall not**. 어떤 경우에는 메서드 호출이 블록될 수 있고, interface method 자체가 정지(suspend)된 동안 실제/형식 인자에 접근 가능한 동시 SystemC 프로세스(caller 내부든 callee 내부든)가 실행될 수 있음에 유의.
- **e) 값 반환** — `get`, `peek`, `transport`처럼 반환 타입이 void가 아닌 경우 트랜잭션 객체는 **값으로 반환**되므로 메서드 호출 중 트랜잭션 객체 수정 문제는 발생하지 않는다.
- **f) 수명** — TLM-1 interface method에 전달되는 트랜잭션 객체의 수명은 **C++ 언어 규칙**으로 결정된다. 트랜잭션 객체는 참조 인자로 전달되거나 값으로 반환되는 것 중 하나임을 기억할 것.
- **g) 유효 구간** — TLM-1 interface method에 인자로 전달된 트랜잭션 객체는, 대응하는 실제/형식 인자가 트랜잭션 인스턴스를 나타내는 값으로 초기화/대입되는 시점부터 interface method 호출에서 반환된 직후 시점까지 유효한 트랜잭션 인스턴스를 나타내야 shall. 예: `transport`의 경우 request 객체는 `transport` 호출 전 초기화/대입 시점부터 `transport` 반환 직후 시점까지 유효하고, response 객체는 `transport` 구현 내부에서 형식 인자에 값이 대입된 시점부터 `transport` 반환 직후 시점까지 유효하다.
  > 본 표준은 그 시점 **이후** 트랜잭션 객체의 유효성에 관한 어떤 의무도 정의하지 않는다. 즉 트랜잭션 객체는 interface method 호출에서 반환될 때 유효하며, 그 이후의 운명은 각 애플리케이션이 결정한다.
- **h) 복사본 소유** — 트랜잭션의 recipient는 이후 처리를 위해 트랜잭션 객체의 복사본을 떠도 된다 may. 그 경우 그 복사본이 더 이상 필요 없을 때 **파괴할 책임은 애플리케이션에 있다**.

### 17.2.7 Constraints on the transaction data type

- **a)** 아래 권장 사항의 의도는 sender와 recipient 사이 message-passing 의미론의 무결성을 보장하고, 공유 메모리를 통한 통신 가능성을 배제하는 것이다.
- **b)** 트랜잭션 객체가 포인터나 참조를 담고 있으면, 트랜잭션의 recipient는 그 포인터/참조로 접근 가능한 저장소의 내용을 **수정하지 않아야 권장(should not)**.
- **c)** 트랜잭션 객체의 데이터 타입은 **deep copy 의미론**을 가져야 권장(should) — recipient가 (C++ 초기화 또는 대입으로) 트랜잭션을 나타내는 실제/형식 인자의 복사본을 떴을 때, 그 복사본에 대한 이후 수정이 원본을 수정하지 않아야 권장(should).
- **d)** 위 제약은 여러 방식으로 충족될 수 있다. 예: 트랜잭션 객체 데이터 타입이 **copy-on-write** 의미론을 구현하여 원본과 복사본이 둘 다 공유 메모리 영역에 대한 내부 포인터를 갖되, 어느 한쪽에 대한 대입이 별도 복사본을 만들게 할 수 있다.
- **e)** 애플리케이션은 **비자명(non-trivial) 파괴 의미론**을 갖는 모든 트랜잭션 클래스(예: 비자명 deep copy 의미론을 갖는 트랜잭션)에 대해 소멸자를 제공해야 shall. 각 트랜잭션 객체가 더 이상 필요 없을 때 파괴하는 책임은 애플리케이션에 있다.
- **f)** 트랜잭션 객체 데이터 타입이 위 제약을 따르지 않으면(예: 포인터의 shallow copy를 만드는 경우), message-passing 의미론을 보장하기 위한 적절한 통신 프로토콜을 따르는 책임은 애플리케이션에 있다.

---

## 17.3 TLM-1 fifo interfaces

### 17.3.1 Description

- `tlm_fifo_debug_if` — `tlm_fifo`에 대한 디버그 접근을 제공하는 interface proper.
- `tlm_fifo_put_if` — `tlm_fifo_debug_if` + `tlm_put_if`를 결합한 interface proper.
- `tlm_fifo_get_if` — `tlm_fifo_debug_if` + `tlm_get_peek_if`를 결합한 interface proper.

이 세 인터페이스 각각은 predefined channel `tlm_fifo`가 구현한다.

### 17.3.2 Class definition

```cpp
namespace tlm {

// Fifo debug interface
template< typename T >
class tlm_fifo_debug_if : public virtual sc_core::sc_interface
{
public:
   virtual int used() const = 0;
   virtual int size() const = 0;
   virtual void debug() const = 0;

   virtual bool nb_peek( T & , int n ) const = 0;
   virtual bool nb_poke( const T & , int n = 0 ) = 0;
};

// Fifo interfaces
template < typename T >
class tlm_fifo_put_if :
    public virtual tlm_put_if<T> ,
    public virtual tlm_fifo_debug_if<T> {};

template < typename T >
class tlm_fifo_get_if :
     public virtual tlm_get_peek_if<T> ,
     public virtual tlm_fifo_debug_if<T> {
public:
         using tlm_get_peek_if<T>::nb_peek;
         using tlm_fifo_debug_if<T>::nb_peek
};
} // namespace tlm
```

> `tlm_fifo_get_if`의 `using` 선언은 `nb_peek` 오버로드의 C++ 모호성 해소용 (Annex D 41 참조).

### 17.3.3 Member functions

이 멤버 함수들은 모두 **pure virtual**이다. 아래 설명은 이 인터페이스를 구현하는 채널에서 오버라이드될 때 기대되는 정의를 가리킨다. 정확한 의미론은 채널마다 다르다.

| 함수 | 규칙 |
|---|---|
| `used()` | `get` 또는 `nb_get`으로 fifo에서 현재 꺼낼 수 있는 항목 수를 반환해야 shall |
| `size()` | fifo의 현재 크기, 즉 fifo가 어느 시점에든 담을 수 있는 최대 항목 수를 반환해야 shall |
| `debug()` | fifo의 현재 상태에 관한 진단 정보를 **표준 출력**에 인쇄해야 shall |
| `nb_peek(T&, int n)` | 주어진 위치의 항목에 대한 참조를 반환해야 shall. 위치 0 = `get`/`nb_get`으로 다음에 꺼낼 항목, 위치 `used()-1` = `put`/`nb_put`으로 가장 최근에 삽입된 항목. 주어진 위치에 항목이 없거나 위치가 음수이면 `false`, 그 외에는 `true`를 반환해야 shall |
| `nb_poke(const T&, int n = 0)` | 주어진 위치의 항목을 인자로 전달된 다른 항목으로 덮어써야 shall. 위치 의미는 `nb_peek`과 동일. 주어진 위치에 항목이 없거나 위치가 음수이면 `false`, 그 외에는 `true`를 반환해야 shall (원문은 이 반환 규칙을 `nb_peek`이라는 이름으로 서술) |

---

## 17.4 `tlm_fifo`

### 17.4.1 Description

`tlm_fifo`는 fifo(first-in-first-out 버퍼) 동작을 모델링하기 위한 **predefined primitive channel**. 각 TLM fifo는 항목 저장용 슬롯을 여러 개 갖는다. 슬롯 수는 객체 생성 시 설정되지만, TLM fifo는 생성 후 **크기 변경(resize)이 가능**하며 **unbounded일 수 있다**.

`tlm_fifo` vs `sc_fifo` 차이:
1. `tlm_fifo`는 SystemC fifo 인터페이스가 아니라 **TLM-1 message-passing 인터페이스**를 구현한다.
2. `tlm_fifo`는 고정 크기가 아니라 **resize 가능하거나 unbounded**일 수 있다.

각 fifo는 위에서 설명한 `tlm_put_if`, `tlm_get_if`, `tlm_peek_if`, `tlm_fifo_debug_if`의 의미론을, 그 인터페이스들을 통해 전달된 트랜잭션 객체 시퀀스를 단일 FIFO 버퍼에 저장함으로써 구현해야 shall. `get`/`nb_get` 호출은 앞서 `put`/`nb_put`으로 fifo에 삽입된 순서와 동일한 순서로 트랜잭션을 꺼내야 shall. **`get`/`nb_get`으로 트랜잭션 객체가 fifo에서 꺼내지면, fifo는 그 트랜잭션 객체의 내부 복사본이나 참조를 보유하면 안 된다 shall not.**

`tlm_fifo`는 **17.4.6의 delta cycle 의미론**을 구현해야 shall.

### 17.4.2 Class definition

```cpp
namespace tlm {

template <typename T>
class tlm_fifo :
    public virtual tlm_fifo_get_if<T>,
    public virtual tlm_fifo_put_if<T>,
    public sc_core::sc_prim_channel
{
public:
    explicit tlm_fifo( int size_ = 1 );
    explicit tlm_fifo( const char* name_, int size_ = 1 );
    virtual ~tlm_fifo();

    T get( tlm_tag<T> *t = 0 );
    bool nb_get( T& );
    bool nb_can_get( tlm_tag<T> *t = 0 ) const;
    const sc_core::sc_event &ok_to_get( tlm_tag<T> *t = 0 ) const;

    T peek( tlm_tag<T> *t = 0 ) const;
    bool nb_peek( T& ) const;
    bool nb_can_peek( tlm_tag<T> *t = 0 ) const;
    const sc_core::sc_event &ok_to_peek( tlm_tag<T> *t = 0 ) const;

    void put( const T& );
    bool nb_put( const T& );
    bool nb_can_put( tlm_tag<T> *t = 0 ) const;
    const sc_core::sc_event& ok_to_put( tlm_tag<T> *t = 0 ) const;

    void nb_expand( unsigned int n = 1 );
    void nb_unbound( unsigned int n = 16 );
    bool nb_reduce( unsigned int n = 1 );
    bool nb_bound( unsigned int n );

    bool nb_peek( T & , int n ) const;
    bool nb_poke( const T & , int n = 0 );

    int used() const;
    int size() const;
    void debug() const;

    const char* kind() const;
} // namespace tlm
```

### 17.4.3 Template parameter T

`tlm_fifo` 템플릿에 전달되는 typename 인자는 **대입에 대한 사전 정의 의미론으로 충분한 C++ 타입**(예: 기본 타입이나 포인터)이거나, 다음 규칙을 모두 지키는 타입 `T`여야 shall:

- **a)** 기본 대입 의미론으로 객체 상태를 대입하기에 부족하면, 타입 `T`에 대해 다음 대입 연산자가 정의되어야 권장(should). 구현은 fifo 슬롯에 쓰이는 값 또는 fifo 슬롯에서 읽히는 값을 복사할 때 이 연산자를 사용해야 shall.
  ```cpp
  const T& operator= ( const T& );
  ```
- **b)** 타입 `T`에 대해 어떤 생성자든 존재한다면, `T`에 대한 **default constructor가 정의되어야 shall**.

> NOTE 1 — 대입 연산자가 객체의 전체 상태를 대입할 의무는 없다. 다만 통상 그렇게 하는 것이 좋다. 예를 들어 fifo를 통해 전파되지 않아야 할 진단 정보가 객체에 연관되어 있을 수 있다.
>
> NOTE 2 — SystemC data type proper(`sc_dt::sc_int`, `sc_dt::sc_logic` 등)는 모두 위 규칙 집합을 준수한다.
>
> NOTE 3 — `sc_module*` 타입을 fifo로 전달하는 것은 합법이지만, 모듈 계층의 남용으로 간주되며 따라서 나쁜 관행이다.

### 17.4.4 Constructors and destructor

| 생성자 | 규칙 |
|---|---|
| `explicit tlm_fifo( int size_ = 1 );` | 초기화 리스트에서 기저 클래스 생성자를 `sc_prim_channel( sc_gen_unique_name( "fifo" ) )`로 호출해야 shall |
| `explicit tlm_fifo( const char* name_, int size_ = 1 );` | 초기화 리스트에서 기저 클래스 생성자를 `sc_prim_channel( name_ )`로 호출해야 shall |

두 생성자 모두 파라미터 `size_`의 값으로 fifo의 슬롯 수를 초기화해야 shall. 이 값은 **양수, 음수, 0 모두 가능**하다.

| `size_` | 의미 |
|---|---|
| 양수 | 해당 크기의 **bounded** fifo. bounded fifo는 full이 될 수 있다 |
| 음수 | **unbounded** fifo. unbounded fifo는 full이 될 수 없다 |
| **0** | 동작이 **undefined** |

`virtual ~tlm_fifo();` — 소멸자는 FIFO 버퍼를 삭제해야 shall, 그리고 모든 트랜잭션 객체를 삭제해야 shall.

### 17.4.5 Member functions

`tlm_put_if`, `tlm_get_if`, `tlm_peek_if`의 멤버 함수는 17.2에서 설명한 대로 구현되어야 shall — 여기에 17.4.6의 delta cycle 의미론이 더해진다.

| 함수 | 규칙 |
|---|---|
| `void nb_expand( unsigned int n = 1 )` | **bounded** fifo의 크기를 인자만큼 증가시켜야 shall (`new_size = previous_size + n`). 그리고 `ok_to_put`이 반환하는 이벤트가 fifo의 **바로 다음 update phase**에 notify되게 해야 shall. fifo가 unbounded이면 `nb_expand`의 동작은 **undefined** |
| `void nb_unbound( unsigned int n = 16 )` | fifo가 unbounded가 되게 해야 shall. 그리고 fifo가 이전에 unbounded였는지와 무관하게 `ok_to_put`이 반환하는 이벤트가 바로 다음 update phase에 notify되게 해야 shall. 인자 값이 fifo의 현재 크기보다 크면 fifo를 그 값으로 resize해야 shall. 그렇지 않으면 fifo 크기는 변하지 않아야 shall |
| `bool nb_reduce( unsigned int n = 1 )` | 새 크기가 현재 fifo 안의 항목 수보다 작지 않은 한, `true`를 반환하고 bounded fifo의 크기를 인자만큼 줄여야 shall (`new_size = max(previous_size - n, used)`). 제안된 크기가 현재 항목 수보다 작으면 `false`를 반환하고 크기를 현재 항목 수로 줄여야 shall. fifo가 unbounded이면 크기를 수정하지 않고 `false`를 반환해야 shall |
| `bool nb_bound( unsigned int n )` | 이전에 bounded였는지와 무관하게 fifo가 bounded가 되게 해야 shall. fifo 크기는 인자 값과 현재 항목 수 중 **큰 쪽**으로 설정되어야 shall (`new_size = max(n, used)`). 새 크기가 인자 값과 같은 경우에 한해(if and only if) `true`를 반환해야 shall |
| `bool nb_peek( T&, int n ) const` | 주어진 위치의 항목을 반환해야 shall. 위치 0 = `get`/`nb_get`으로 다음에 꺼낼 항목, 위치 `used()-1` = `put`/`nb_put`으로 가장 최근 삽입된 항목. 위치에 항목이 없거나 위치가 음수이면 `false`, 그 외 `true` |
| `bool nb_poke( const T&, int n = 0 )` | 주어진 위치의 항목을 인자로 전달된 항목으로 덮어써야 shall. 위치 의미는 위와 동일. 위치에 항목이 없거나 위치가 음수이면 `false`, 그 외 `true` |
| `int used() const` | `get`, `nb_get`, `nb_peek`으로 꺼낼 수 있거나 `nb_poke`로 수정할 수 있는 현재 항목 수를 반환해야 shall. `put`/`nb_put`은 **다음 update phase**에 `used` 값을 증가시킬 수 있고 may, `get`/`nb_get`은 `used` 값을 **즉시** 감소시킬 수 있으며 may, `peek`/`nb_peek`/`nb_poke`는 `used` 값을 변경하면 안 된다 shall not |
| `int size() const` | fifo의 현재 크기, 즉 어느 시점에든 담을 수 있는 최대 항목 수를 반환해야 shall. fifo는 resize될 수 있다. **음이 아닌 size = bounded**(full이 될 수 있음). 음이 아닌 size에 대해 `put`, `nb_put`, `get`, `nb_get`은 size 값을 변경하면 안 된다 shall not. **음수 size = unbounded**이며, 이 경우 size의 실제 값은 **undefined** |
| `void debug() const` | fifo의 현재 상태에 관한 진단 정보를 표준 출력에 인쇄해야 shall. 진단 정보의 상세 내용은 **undefined** |
| `const char* kind() const` | 문자열 **`"tlm_fifo"`** 를 반환해야 shall |

### 17.4.6 Delta cycle semantics

- `put` 또는 `nb_put` 호출로 fifo에 삽입된 트랜잭션은 **다음 delta cycle**에서만 `get`, `nb_get`, `peek`, `nb_peek`에 가용해져야 shall. 단 fifo가 full이 되는 것에 관해서는 **현재 evaluation phase**의 이후 `put`/`nb_put` 호출에 즉시 영향을 주어야 shall.
- `get` 또는 `nb_get` 호출로 fifo에 생긴 빈 슬롯은 **다음 delta cycle**에서만 `put`/`nb_put`에 가용해져야 shall. 단 현재 evaluation phase의 이후 `get`/`nb_get` 호출에는 즉시 영향을 주어야 shall.
- 즉, 주어진 evaluation phase에서의 `put`, `nb_put`, `get`, `nb_get` 호출은 관련 상태 변수를 primitive channel의 **update phase**에서 수정하게 하여, 그 효과가 **바로 다음 evaluation phase**에서만 보이게 해야 shall. 여기에는 `ok_to_put`, `ok_to_get`, `ok_to_peek`가 반환하는 이벤트의 notification도 포함되어야 shall.
- 예: 빈 fifo에 대해 주어진 evaluation phase 내에서 `put`을 연속 호출하면 fifo가 full이 되어 `put`이 블록될 수 있는데, 이때도 `nb_get`은 여전히 `false`를 반환할 수 있다. 마찬가지로 full fifo에 대해 주어진 evaluation phase 내에서 `get`을 연속 호출하면 fifo가 비어 `get`이 블록될 수 있는데, 이때도 `nb_put`은 여전히 `false`를 반환할 수 있다.
- **`tlm_fifo_debug_if`의 `nb_peek`과 `nb_poke`는** 현재 evaluation phase에서 `put`/`nb_put`으로 fifo에 삽입된 트랜잭션에 접근할 수 없고, 현재 evaluation phase에서 `get`/`nb_get`으로 fifo에서 이미 제거된 트랜잭션에도 접근할 수 없다.

#### Example

```cpp
struct Top : sc_core::sc_module {
    typedef tlm_fifo<int> fifo_t;

    fifo_t *fifo;

    Top(sc_core::sc_module_name _name) {
        fifo = new fifo_t(2);              // 크기 2의 bounded fifo 생성
        SC_THREAD(T1);
        SC_THREAD(T2);
    }

    sc_dt::uint64 delta;

    void T1() {
        sc_assert(fifo->size() == 2);
        for (int i = 0; i < 4; i++)
            fifo->put(i);                  // 세 번째 호출이 블록됨

        fifo->nb_expand(2);                // fifo 크기 증가
        sc_assert(fifo->size() == 4);

        for (int i = 4; i < 8; i++)
            fifo->put(i);

        sc_assert(fifo->nb_reduce(3));     // fifo 크기 감소
        sc_assert(fifo->size() == 1);

        for (int i = 8; i < 12; i++)
            fifo->put(i);                  // 두 번째 호출이 블록됨

        fifo->nb_unbound();                // fifo를 unbounded로
        sc_assert(fifo->size() < 0);

        delta = sc_delta_count();

        for (int i = 101; i <= 104; i++)
            fifo->put(i);

        sc_assert(sc_delta_count() == delta);
        sc_assert(fifo->used() == 0);
    }

    void T2() {
        for (int i = 0; i < 12; i++)
            sc_assert(fifo->get() == i);

        sc_assert(fifo->get() == 101);
        sc_assert(fifo->used() == 3);
        sc_assert(sc_delta_count() == delta + 1);

        sc_assert(fifo->get() == 102);
        sc_assert(fifo->used() == 2);
        sc_assert(sc_delta_count() == delta + 1);

        sc_assert(fifo->get() == 103);
        sc_assert(fifo->used() == 1);
        sc_assert(sc_delta_count() == delta + 1);

        sc_assert(fifo->get() == 104);
        sc_assert(fifo->used() == 0);
        sc_assert(sc_delta_count() == delta + 1);
    }
};
```

---

## 17.5 Analysis interface and analysis ports

### 17.5.1 Overview

Analysis port는 트랜잭션을 **여러 컴포넌트에 분배**하여 분석(기능 정확성 검사, 기능 커버리지 통계 수집 등)하는 것을 지원하기 위한 것이다.

- 핵심 기능: **하나의 port를 여러 채널 또는 subscriber에 바인드**할 수 있고, port 자신이 `write` interface method 호출을 각 subscriber에게 복제한다.
- analysis port는 0개 이상의 subscriber 또는 다른 analysis port에 바인드될 수 있고, 바인드되지 않은 채로 있을 수도 있다.
- `tlm_analysis_port`는 **`sc_port`가 아니라 `sc_object`에서 파생**된다. 따라서 analysis port는 엄밀히 말해 port가 아니다. Analysis port는 **시뮬레이션 중 동적으로** 인스턴스화·삭제·바인드·언바인드될 수 있다.
- 각 subscriber는 `tlm_analysis_if`의 `write` 멤버 함수를 구현한다. 이 멤버 함수는 트랜잭션에 대한 **const 참조**를 받으며, subscriber는 이를 즉시 처리해도 된다 may. 그렇지 않고 subscriber가 트랜잭션 수명을 연장하고자 한다면 **트랜잭션 객체의 deep copy를 뜰 의무가 있다**. 그 시점에서 subscriber는 사실상 새 트랜잭션의 initiator가 되며 따라서 그 복사본의 메모리 관리에 책임을 진다.
- **Analysis port는 모델의 주 동작 경로(main operational pathway)에 사용하지 않아야 권장(should not)** — 데이터를 옆으로 빼내어 분석에 넘기는 곳에만 사용한다.
- `tlm_analysis_if`는 `tlm_write_if`에서 파생된다. 후자는 분석 전용이 아니며 다른 목적으로 사용해도 된다 may (예: 16.4 PEQ 참조).
- `tlm_analysis_fifo`는 단순히 **무한 `tlm_fifo`**이며, `tlm_analysis_if`를 구현하여 트랜잭션을 fifo에 write한다.

### 17.5.2 Class definition

```cpp
namespace tlm {

// Write interface
template <typename T>
class tlm_write_if : public virtual sc_core::sc_interface {
public:
    virtual void write( const T& ) = 0;
};

// Analysis interface
template <typename T>
class tlm_analysis_if : public virtual tlm_write_if<T>
{
};

// Analysis port
template < typename T>
class tlm_analysis_port : public sc_core::sc_object , public virtual tlm_analysis_if<T>
{
public:
    tlm_analysis_port();
    tlm_analysis_port( const char * );

    // bind and () work for both interfaces and analysis ports,
    // since analysis ports implement the analysis interface

    virtual void bind( tlm_analysis_if<T> & );
    void operator() ( tlm_analysis_if<T> & );
    virtual bool unbind( tlm_analysis_if<T> & );

    void write( const T & );
};

// Analysis fifo - an unbounded tlm_fifo
template<typename T>
class tlm_analysis_fifo :
    public tlm_fifo<T> ,
    public virtual tlm_analysis_if<T> ,

public:
   tlm_analysis_fifo( const char *nm ) : tlm_fifo<T>( nm, -16 ) {}
   tlm_analysis_fifo() : tlm_fifo<T>( -16 ) {}

   void write( const T &t ) { nb_put( t ); }
};

} // namespace tlm
```

### 17.5.3 Rules

| # | 규칙 |
|---|---|
| a | `tlm_write_if`와 `tlm_analysis_if`(및 그 delayed variant)는 **단방향·비협상(non-negotiated)·non-blocking** 트랜잭션 레벨 인터페이스다. 즉 callee는 인자로 전달된 트랜잭션을 **즉시 수락하는 것 외에 선택지가 없다** |
| b | 생성자는 문자열 인자를 기저 클래스 `sc_object`의 생성자에 전달하여 모듈 계층 내 인스턴스의 string name을 설정해야 shall |
| c | `bind`는 인자로 전달된 subscriber를 analysis port 인스턴스에 등록하여, `write` 호출이 등록된 subscriber에게 전달되게 해야 shall. **하나의 analysis port 인스턴스에 여러 subscriber를 등록해도 된다 may** |
| d | `operator()`의 구현은 virtual 멤버 함수 `bind`를 호출함으로써 그 효과를 달성해야 shall |
| e | 주어진 analysis port 인스턴스에 등록된 subscriber가 **0개일 수 있다**. 그 경우 `write` 호출은 전파되지 않아야 shall |
| f | `unbind`는 `bind`의 효과를 되돌려야 shall — 인자로 전달된 subscriber가 그 analysis port 인스턴스의 subscriber 목록에서 제거되어야 shall |
| g | `bind`와 `unbind`는 **elaboration 중**에 호출할 수도 있고 **시뮬레이션 중 동적으로** 호출할 수도 있다 |
| h | `tlm_analysis_port::write`는 그 analysis port 인스턴스에 등록된 **모든** subscriber의 `write` 멤버 함수를 호출해야 shall — 인자를 const 참조로 전달하면서 |
| i | `write`는 non-blocking이다. `wait`를 **호출하면 안 된다 shall not** |
| j | `write`는 const 참조 인자로 전달된 트랜잭션 객체를 **수정하면 안 되며 shall not**, 그 트랜잭션 객체에 연관된 어떤 데이터(generic payload의 data 배열, byte enable 배열 등)도 수정하면 안 된다 shall not |
| k | subscriber의 `write` 구현이 caller에게 제어를 반환하기 전에 트랜잭션을 처리할 수 없다면, 그 subscriber는 트랜잭션 객체의 **deep copy를 뜨고**, 이후 그 복사본에 연관된 메모리를 관리할 **책임을 져야 shall** |
| l | `tlm_analysis_fifo`의 생성자들은 각각 **unbounded `tlm_fifo`** 를 생성해야 shall |
| m | `tlm_analysis_fifo::write`는 기저 클래스 `tlm_fifo`의 `nb_put`을 호출하며 인자를 `nb_put`에 전달해야 shall |

#### Example

```cpp
struct Trans {                                              // 분석용 트랜잭션 클래스
    int i;
};

struct Subscriber : sc_core::sc_object, tlm::tlm_analysis_if<Trans> {
    Subscriber(const char *n) : sc_core::sc_object(n) {}
    virtual void write(const Trans &t)
    {
        std::cout << "Hello, got" << t.i << std::endl;      // write 멤버 함수의 구현
    }
};

SC_MODULE(Child) {
    tlm::tlm_analysis_port<Trans> ap;

    SC_CTOR(Child) : ap("ap") {
        SC_THREAD(thread);
    }

    void thread() {
        Trans t = {999};
        ap.write(t);    // analysis port의 write 멤버 함수에 대한 interface method call
    }
};

SC_MODULE(Parent) {
    tlm::tlm_analysis_port<Trans> ap;
    Child *child;

    SC_CTOR(Parent) : ap("ap") {
        child = new Child("child");
        child->ap.bind(ap);           // child의 analysis port를 parent의 analysis port에 바인드
    }
};

SC_MODULE(Top) {
    Parent *parent;
    Subscriber *subscriber1;
    Subscriber *subscriber2;

    SC_CTOR(Top) {
        parent = new Parent("parent");
        subscriber1 = new Subscriber("subscriber1");
        subscriber2 = new Subscriber("subscriber2");
        parent->ap.bind(*subscriber1);   // analysis port를 두 개의 별개 subscriber에 바인드
        parent->ap.bind(*subscriber2);   // 이것이 analysis port의 핵심 기능
    }
};
```

---

## 흔한 위반 (pitfalls)

- **method process에서 `put`/`get`/`peek`/`transport` 호출** — LRM §17.2.3 d): blocking interface method는 method process에서 호출하면 안 된다 shall not. `SC_THREAD`에서 호출하거나 `nb_*` 계열로 바꿔라.
- **non-blocking 메서드 안에서 `wait` 호출** — LRM §17.2.3 e): non-blocking interface method는 직간접적으로 `wait`를 호출하면 안 된다 shall not.
- **`put`에 넘긴 객체를 호출 중/직후에 수정** — LRM §17.2.6 d): 값을 대입한 뒤 interface method가 반환할 때까지 caller도 callee도 트랜잭션 객체를 수정하면 안 된다 shall not. 호출이 블록되는 동안 동시 프로세스가 그 객체를 건드릴 수 있음에 유의.
- **`ok_to_put` 이벤트를 받고 `nb_put`이 성공했다고 가정** — LRM §17.2.5 f): 이벤트 notification 직후에도 non-blocking 메서드가 `true`를 반환한다고 가정할 수 없다. 반환값을 반드시 확인하라.
- **`nb_get`이 true인데 `nb_can_get`은 false인 채널 구현** — LRM §17.2.5 e): `nb_get`/`nb_can_get`/`nb_peek`/`nb_can_peek`는 같은 시각·같은 port에서 같은 값을 반환해야 shall.
- **interface method 본문에서 `tlm_tag<T>*` 인자를 사용** — LRM §17.2.6 a): 본문은 이 인자를 사용하면 안 된다 shall not. 오직 오버로드 모호성 해소용.
- **얕은 복사 포인터를 담은 트랜잭션 타입 사용** — LRM §17.2.7 b), c), f): deep copy 의미론이 권장되며(should), 그렇지 않으면 message-passing 의미론 보장 책임은 애플리케이션에 있다.
- **`tlm_fifo`를 size 0으로 생성** — LRM §17.4.4: size 0인 fifo의 동작은 **undefined**. 양수(bounded) 또는 음수(unbounded)를 써라.
- **unbounded fifo에 `nb_expand` 호출** — LRM §17.4.5: unbounded fifo에 대한 `nb_expand`는 **undefined**. `nb_bound`로 먼저 bounded로 만들어라.
- **unbounded fifo의 `size()` 반환값에 의존** — LRM §17.4.5: 음수 size는 unbounded를 뜻하며 실제 값은 **undefined**. 부호만 검사하라.
- **`put` 직후 같은 evaluation phase에서 `get`으로 꺼내려 함** — LRM §17.4.6: `put`으로 삽입된 트랜잭션은 다음 delta cycle에서만 `get`/`nb_get`/`peek`/`nb_peek`에 가용해진다 shall.
- **디버그용 `nb_peek(T&, int)`로 방금 put한 항목을 보려 함** — LRM §17.4.6: `tlm_fifo_debug_if`의 `nb_peek`/`nb_poke`는 현재 evaluation phase에 삽입/제거된 트랜잭션에 접근할 수 없다.
- **subscriber의 `write`에서 트랜잭션 참조를 저장해 두고 나중에 사용** — LRM §17.5.3 k): 즉시 처리하지 못하면 deep copy를 뜨고 그 메모리를 관리할 책임을 져야 shall.
- **subscriber의 `write`에서 트랜잭션 또는 그 data/byte enable 배열을 수정** — LRM §17.5.3 j): 수정하면 안 된다 shall not.
- **`write` 구현에서 `wait` 호출** — LRM §17.5.3 i): `write`는 non-blocking이며 `wait`를 호출하면 안 된다 shall not.
- **analysis port를 주 데이터 경로에 사용** — LRM §17.5.1: analysis port는 모델의 주 동작 경로가 아니라 분석용 tap에만 사용해야 권장(should not use elsewhere).
