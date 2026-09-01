# Clause 6 — Predefined channel class definitions

LRM pp. 165–219.

LRM Clause 6. signal / buffer / clock / fifo / mutex / semaphore / event queue와 그에 대응하는 특수화 포트(`sc_in`, `sc_out`, `sc_inout` 계열). **모듈 간 배선을 실제로 만드는 계층.**

각 절은 LRM 형식을 따른다: Description → Class definition → Constraints on usage → Constructors → Member functions → kind.

---

## 6.0 `kind()` 반환 문자열 요약

| 클래스 | `kind()` | 절 |
|---|---|---|
| `sc_signal` | `"sc_signal"` | 6.4.10 |
| `sc_buffer` | `"sc_buffer"` | 6.6.4 |
| `sc_clock` | `"sc_clock"` | 6.7.6 |
| `sc_in` | `"sc_in"` | 6.8.3 |
| `sc_inout` | `"sc_inout"` | 6.10.3, 6.11.3 |
| `sc_out` | `"sc_out"` | 6.12.3 |
| `sc_signal_resolved` | `"sc_signal_resolved"` | 6.13.5 |
| `sc_in_resolved` | `"sc_in_resolved"` | 6.14.3 |
| `sc_inout_resolved` | `"sc_inout_resolved"` | 6.15.3 |
| `sc_out_resolved` | `"sc_out_resolved"` | 6.16.3 |
| `sc_signal_rv` | `"sc_signal_rv"` | 6.17.3 |
| `sc_in_rv` | `"sc_in_rv"` | 6.18.3 |
| `sc_inout_rv` | `"sc_inout_rv"` | 6.19.3 |
| `sc_out_rv` | `"sc_out_rv"` | 6.20.3 |
| `sc_fifo` | `"sc_fifo"` | 6.23.11 |
| `sc_fifo_in` | `"sc_fifo_in"` | 6.24.3 |
| `sc_fifo_out` | `"sc_fifo_out"` | 6.25.3 |
| `sc_mutex` | `"sc_mutex"` | 6.27.4 |
| `sc_semaphore` | `"sc_semaphore"` | 6.29.4 |
| `sc_event_queue` | `"sc_event_queue"` | 6.30.5 |

---

## 6.1 `sc_signal_in_if`

### Description
`sc_signal` 등 predefined channel이 사용하는 interface proper. signal 값에 **read 접근**을 준다.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_signal_in_if
: virtual public sc_interface
{
    public:
        virtual const T& read() const = 0;
        virtual const sc_event& value_changed_event() const = 0;
        virtual bool event() const = 0;

    protected:
        sc_signal_in_if();

    private:
        // Disabled
        sc_signal_in_if( const sc_signal_in_if<T>& );
        sc_signal_in_if<T>& operator= ( const sc_signal_in_if<T>& );
};

}         // namespace sc_core
```

### 6.1.3 Member functions

모두 pure virtual. 아래는 이 인터페이스를 구현하는 채널에서 override될 때 **기대되는** 정의이며, 정확한 의미론은 채널마다 다르다.

| 함수 | 규칙 |
|---|---|
| `read()` | 채널의 **현재 값에 대한 참조**를 반환해야 shall |
| `value_changed_event()` | 채널 값이 쓰이거나 수정될 때마다 notify되는 event 참조를 반환해야 shall |
| `event()` | 채널 값이 **직전 delta cycle**에 그리고 **현재 시뮬레이션 시각**에 쓰이거나 수정된 경우에만(if and only if) `true` 반환 shall |

> NOTE — 값이 수정된 지점이 직전 delta cycle의 evaluation phase인지 update phase인지는 hierarchical channel이냐 primitive channel(예: `sc_signal`)이냐에 따라 다르다.

---

## 6.2 `sc_signal_in_if<bool>` / `sc_signal_in_if<sc_dt::sc_logic>`

### Description
2치(two-valued) signal에 적합한 추가 멤버 함수를 제공하는 interface proper 특수화.

### Class definition

```cpp
namespace sc_core {

template <>
class sc_signal_in_if<bool>
: virtual public sc_interface
{
    public:
        virtual const T& read() const = 0;

        virtual const sc_event& value_changed_event() const = 0;
        virtual const sc_event& posedge_event() const = 0;
        virtual const sc_event& negedge_event() const = 0;

        virtual bool event() const = 0;
        virtual bool posedge() const = 0;
        virtual bool negedge() const = 0;

    protected:
        sc_signal_in_if();

    private:
        // Disabled
        sc_signal_in_if( const sc_signal_in_if<bool>& );
        sc_signal_in_if<bool>& operator= ( const sc_signal_in_if<bool>& );
};

template <>
class sc_signal_in_if<sc_dt::sc_logic>
: virtual public sc_interface
{
    // ... sc_signal_in_if<bool>과 동일 패턴 (read / value_changed_event /
    //     posedge_event / negedge_event / event / posedge / negedge)
    private:
        // Disabled
        sc_signal_in_if( const sc_signal_in_if<sc_dt::sc_logic>& );
        sc_signal_in_if<sc_dt::sc_logic>& operator= ( const sc_signal_in_if<sc_dt::sc_logic>& );
};

}          // namespace sc_core
```

### 6.2.3 Member functions

나머지 함수는 6.1.3 참조. 아래 목록은 불완전(incomplete)하다.

| 함수 | 규칙 |
|---|---|
| `posedge_event()` | `read()`가 반환하는 값이 **변할 때** 그리고 새 값이 `true` 또는 `'1'`일 때마다 notify되는 event 참조 shall |
| `negedge_event()` | 값이 변하고 새 값이 `false` 또는 `'0'`일 때마다 notify되는 event 참조 shall |
| `posedge()` | 값이 **직전 delta cycle의 update phase**에 현재 시각에서 변했고 새 값이 `true`/`'1'`인 경우에만 `true` shall |
| `negedge()` | 같은 조건으로 새 값이 `false`/`'0'`인 경우에만 `true` shall |

---

## 6.3 `sc_signal_inout_if` / `sc_signal_write_if` / `sc_writer_policy`

### Description
`sc_signal_inout_if`는 signal 값에 **read + write** 접근을 주는 interface proper. `sc_signal_write_if`와 `sc_signal_in_if` **둘 다에서 파생되어야** shall.

### Class definition

```cpp
namespace sc_core {

#define SC_DEFAULT_WRITER_POLICY implementation-defined

enum sc_writer_policy
{
   SC_ONE_WRITER,
   SC_MANY_WRITERS
};

template <class T>
class sc_signal_write_if
: virtual public sc_interface
{
    public:
        virtual sc_writer_policy get_writer_policy() const { return SC_DEFAULT_WRITER_POLICY; }
        virtual void write( const T& ) = 0;

    protected:
        sc_signal_write_if();

    private:
        // Disabled
        sc_signal_write_if( const sc_signal_write_if<T>& );
        sc_signal_write_if<T>& operator= ( const sc_signal_write_if<T>& );
};

template <class T>
class sc_signal_inout_if
: public sc_signal_in_if<T> , public sc_signal_write_if<T>
{
    protected:
        sc_signal_inout_if();

    private:
        // Disabled
        sc_signal_inout_if( const sc_signal_inout_if<T>& );
        sc_signal_inout_if<T>& operator= ( const sc_signal_inout_if<T>& );
};

}          // namespace sc_core
```

### 6.3.3 Member functions

- `get_writer_policy()` — 해당 채널 인스턴스의 writer policy 값을 반환해야 shall. 채널에서 override하지 않으면 `SC_DEFAULT_WRITER_POLICY`를 반환해야 shall.
- **`SC_DEFAULT_WRITER_POLICY`는 기본적으로 `SC_ONE_WRITER`와 같아야 shall** (이전 표준과의 하위 호환). 다만 **구현이 다른 값을 정의할 수도 있다(may)** → `SC_DEFAULT_WRITER_POLICY`는 **implementation-defined**.
- `write( const T& )` — 채널 값을 수정하되, 채널이 새 값을 갖는 것으로 보이는 시점은 **다음 delta cycle이고 그 이전은 아니어야** shall.
- `write`는 해당 채널 인스턴스에 설정된 **writer policy를 지켜야** shall (one writer 또는 many writers 허용).

---

## 6.4 `sc_signal`

### Description
디지털 전자 신호를 실어 나르는 **한 가닥 배선(single piece of wire)** 을 모델링하는 predefined primitive channel.

### Class definition

```cpp
namespace sc_core {

template <class T, sc_writer_policy WRITER_POLICY = SC_DEFAULT_WRITER_POLICY>
class sc_signal
: public sc_signal_inout_if<T>, public sc_prim_channel
{
    public:
        sc_signal();
        explicit sc_signal( const char* );
        sc_signal( const char* name_, const T& initial_value_ );
        sc_signal( const char* name_, const sc_dt::sc_logic initial_value_ );
        virtual ~sc_signal();

        virtual void register_port( sc_port_base&, const char* );

        virtual const T& read() const;
        operator const T& () const;

        virtual sc_writer_policy get_writer_policy() const;
        virtual void write( const T& );
        sc_signal<T,WRITER_POLICY>& operator= ( const T& );
        sc_signal<T,WRITER_POLICY>& operator= ( const sc_signal<T,WRITER_POLICY>& );

        virtual const sc_event& default_event() const;
        virtual const sc_event& value_changed_event() const;
        virtual bool event() const;

        virtual void print( std::ostream& = std::cout ) const;
        virtual void dump( std::ostream& = std::cout ) const;
        virtual const char* kind() const;

    protected:
        virtual void update();

    private:
        // Disabled
        sc_signal( const sc_signal<T,WRITER_POLICY>& );
};

template <class T, sc_writer_policy WRITER_POLICY>
inline std::ostream& operator<< ( std::ostream&, const sc_signal<T,WRITER_POLICY>& );

}         // namespace sc_core
```

Disabled: **복사 생성자**. (대입 연산자는 위 public 목록대로 `write`로 동작.)

### 6.4.3 Template parameter T

인자는 대입·동등 비교의 predefined 의미론으로 충분한 C++ 타입(fundamental type, 포인터 등)이거나, 아래 규칙을 **모두** 지키는 타입 T여야 shall.

| 항목 | 규칙 |
|---|---|
| a) `bool T::operator== ( const T& );` | **정의되어야 shall.** 두 값이 signal 전파 관점에서 구별 불가하면 `true`를 반환해야 should (즉 값이 다를 때만 event 발생). 구현은 event 발생 판정에 이 연산자를 사용해야 shall |
| b) `std::ostream& operator<< ( std::ostream&, const T& );` | **정의되어야 shall.** 상태를 스트림에 복사해야 should. 포맷은 **undefined by this standard**. 구현은 `print`/`dump` 구현에 이 연산자를 사용해야 shall |
| c) `const T& operator= ( const T& );` | 기본 대입 의미론이 부적합하면 정의해야 should. 어느 경우든 좌변이 우변과 (a의 동등 연산자로) 구별 불가해지도록 충분해야 should. 구현은 값 대입·복사에 이 연산자를 사용해야 shall |
| d) 기본 생성자 | T에 **어떤 생성자라도 존재하면** default constructor를 정의해야 shall |
| e) `void sc_trace( sc_trace_file*, const T&, const std::string& );` | 이 signal에 `sc_in`/`sc_inout`/`sc_out` 포트가 bind되는 경우 **정의되어야 shall** |

> NOTE 1 — 동등/대입 연산자가 객체 상태 전부를 비교·대입할 의무는 없다(다만 보통은 그렇게 해야 should).
> NOTE 2 — SystemC data types proper (`sc_dt::sc_int`, `sc_dt::sc_logic` 등)는 모두 위 규칙을 만족한다.
> NOTE 3 — `sc_module::operator==`가 없으므로 `sc_module`을 `sc_signal`의 template 인자로 넘기는 것은 illegal. `sc_module*`은 legal하지만 module hierarchy 남용이며 bad practice.

### 6.4.4 Reading and writing signals — WRITER_POLICY 규칙

읽기: `read()` 또는 `operator const T& ()`.
쓰기: `write()` 또는 `operator=`.

| WRITER_POLICY | 규칙 |
|---|---|
| `SC_ONE_WRITER` | 시뮬레이션 중 **어느 시점에서든** 둘 이상의 process instance가 같은 signal 인스턴스에 쓰면 **error** shall |
| `SC_MANY_WRITERS` | **같은 evaluation phase 안에서** 둘 이상의 process instance가 쓰면 **error** shall. 서로 다른 delta cycle이면 서로 다른 process가 써도 된다 may |

- signal은 **elaboration 중에도 쓸 수 있다 may** (4.5의 elaboration/simulation callback 포함) — 초기값 설정 목적.
- `sc_main`에서 elaboration 중 또는 시뮬레이션이 멈춰 있는 동안(즉 `sc_start` 호출 전후) 쓸 수 있다 may.
- signal은 보통 evaluation phase에서 읽고 쓰지만, **값이 실제로 바뀌는 것은 뒤이은 update phase**뿐이다.
- 쓰기 결과 값이 **실제로 변한 경우에만(if and only if)** value-changed event가 **바로 뒤의 delta notification phase**에 notify되어야 shall.
- 하나의 evaluation phase(또는 elaboration 중, 또는 `sc_main`에서) 여러 번 쓰면 다음 update phase에서 반영되는 값은 **가장 마지막 write**가 결정한다 — **last write wins** shall.

> NOTE 1 — 특수화 포트 `sc_inout`/`sc_out`은 elaboration 중 signal 초기화를 위한 `initialize` 멤버 함수를 갖는다.
> NOTE 2 — elaboration 중 signal을 읽으면 T의 default constructor가 만든 초기값이 반환된다.
> NOTE 3 — 같은 evaluation phase에서 쓰고 읽으면 **옛 값**이 읽힌다. 쓴 값은 다음 evaluation phase가 되어야 읽을 수 있다.

### 6.4.5 Constructors

| 생성자 | 동작 |
|---|---|
| `sc_signal()` | base를 `sc_prim_channel( sc_gen_unique_name("signal") )`로 호출 shall. 값은 T의 default constructor로 초기화 shall |
| `explicit sc_signal( const char* name_ )` | base를 `sc_prim_channel( name_ )`로 호출 shall. 값은 T의 default constructor로 초기화 shall |
| `sc_signal( const char* name_, const T& initial_value_ )` | base `sc_prim_channel( name_ )`. **event와 뒤따르는 delta cycle을 발생시키지 않고** `initial_value_`로 초기화 shall |
| `sc_signal( const char* name_, sc_dt::sc_logic initial_value_ )` | 동일 — event 없이 초기화 shall |

### 6.4.6 `register_port`

`sc_interface::register_port`를 override하여 error check를 수행해야 shall.

- `SC_ONE_WRITER`이면 **`sc_signal_inout_if` 타입 포트를 둘 이상 bind하면 error** shall.
- `SC_MANY_WRITERS`이면 `sc_signal_inout_if` 타입 포트를 하나 이상 bind해도 된다 may.

### 6.4.7 ~ 6.4.9 Member functions

| 함수 | 규칙 |
|---|---|
| `read()` | 현재 값 참조를 반환해야 shall. **signal 상태를 수정하면 안 된다 shall not** |
| `operator const T& ()` | `read()`가 반환하는 현재 값 참조를 반환 shall |
| `get_writer_policy()` | 해당 인스턴스의 `WRITER_POLICY` template 인자 값을 반환 shall |
| `write( const T& )` | 다음 delta cycle에 새 값으로 보이도록 수정 shall. **primitive channel의 update request 메커니즘**으로 수행 shall. **elaboration 중 호출 가능 may** — 이 경우 update request는 **initialization phase에서 실행**되어야 shall |
| `operator=` | `{ write(arg); return *this; }` / `{ write(arg.read()); return *this; }` 와 동등 shall |
| `update()` | `sc_prim_channel::update`를 구현이 override shall. 현재 값을 새 값으로 바꾸고, **값이 변했으면** value-changed event를 바로 뒤 delta notification phase에 notify하도록 해야 shall |
| `default_event()`, `value_changed_event()` | **둘 다 value-changed event 참조**를 반환 shall |
| `event()` | 값이 **직전 delta cycle의 update phase**에 현재 시각에서 변한 경우에만 `true` shall — 즉 직전 evaluation phase에 `write`/`operator=`가 호출되었고 쓴 값이 이전 값과 달랐을 때 |

> NOTE (`update`) — `update`는 scheduler가 호출하며 애플리케이션이 호출하는 것이 보통은 아니다. 다만 `sc_signal` 파생 클래스의 `update`에서 `sc_signal::update`를 호출하는 것은 may.
> NOTE (`event`) — 같은 signal 인스턴스의 value-changed event가 notify된 직접 결과로 실행된 process에서 호출하면 `event()`는 `true`를 반환한다.

### 6.4.10 Diagnostic member functions

| 함수 | 규칙 |
|---|---|
| `print(std::ostream&)` | `operator<< (std::ostream&, T&)`를 호출해 현재 값을 출력 shall. **추가 문자를 출력하면 안 된다 shall not** |
| `dump(std::ostream&)` | 최소한 **hierarchical name, 현재 값, 새 값**을 출력 shall. 포맷은 **implementation-defined** |
| `kind()` | `"sc_signal"` 반환 shall |

### 6.4.11 `operator<<`

```cpp
template <class T, sc_writer_policy WRITER_POLICY>
inline std::ostream& operator<< ( std::ostream& , const sc_signal<T,WRITER_POLICY>& );
```
두 번째 인자의 현재 값을 `operator<< ( std::ostream& , T& )` 호출로 첫 인자 스트림에 출력 shall.

### Example

```cpp
SC_MODULE(M) {
   sc_core::sc_signal<int> sig;
   SC_CTOR(M) {
       SC_THREAD(writer);
       SC_THREAD(reader);
       SC_METHOD(writer2);
       sensitive << sig;                                // default event에 sensitive
   }

     void writer() {
         using namespace sc_core;
         wait(50.0, SC_NS);
         sig.write(1);
         sig.write(2);
         wait(50.0, SC_NS);
         sig = 3;                                       // operator= ( const T& ) 호출
     }

     void reader() {
         wait(sig.value_changed_event());
         int i = sig.read();                            // 값 2를 읽음
         wait(sig.value_changed_event());
         i = sig;                                       // operator const T& () 호출, 값 3 반환
     }

     void writer2() {
         sig.write(sig + 1);                            // error. signal은 writer가 여럿이면 안 됨
     }
};
```

> NOTE — 관련 클래스: `sc_signal<bool,WRITER_POLICY>` / `sc_signal<sc_dt::sc_logic,WRITER_POLICY>`(2치 신호용 추가 함수), `sc_buffer`(값 변화 여부와 무관하게 write마다 value-changed event notify), `sc_clock`(주기적 clock 생성), `sc_signal_resolved`(다중 writer 허용), `sc_in`/`sc_out`/`sc_inout`(특수화 포트).

---

## 6.5 `sc_signal<bool,WRITER_POLICY>` / `sc_signal<sc_dt::sc_logic,WRITER_POLICY>`

### Description
2치 신호에 적합한 추가 멤버 함수를 제공하는 predefined primitive channel 특수화.

### Class definition

```cpp
namespace sc_core {

template <sc_writer_policy WRITER_POLICY>
class sc_signal<bool,WRITER_POLICY>
: public sc_signal_inout_if<bool>, public sc_prim_channel
{
    public:
        sc_signal();
        explicit sc_signal( const char* );
        virtual ~sc_signal();

        virtual void register_port( sc_port_base&, const char* );

        virtual const bool& read() const;
        operator const bool& () const;

        virtual sc_writer_policy get_writer_policy() const;
        virtual void write( const bool& );
        sc_signal<bool,WRITER_POLICY>& operator= ( const bool& );
        sc_signal<bool,WRITER_POLICY>& operator= ( const sc_signal<bool,WRITER_POLICY>& );

        virtual const sc_event& default_event() const;

        virtual const sc_event& value_changed_event() const;
        virtual const sc_event& posedge_event() const;
        virtual const sc_event& negedge_event() const;

        virtual bool event() const;
        virtual bool posedge() const;
        virtual bool negedge() const;

        virtual void print( std::ostream& = std::cout ) const;
        virtual void dump( std::ostream& = std::cout ) const;
        virtual const char* kind() const;

    protected:
        virtual void update();

    private:
        // Disabled
        sc_signal( const sc_signal<bool,WRITER_POLICY>& );
};

template <sc_writer_policy WRITER_POLICY>
class sc_signal<sc_dt::sc_logic,WRITER_POLICY>
: public sc_signal_inout_if<sc_dt::sc_logic,WRITER_POLICY>, public sc_prim_channel
{
    // ... sc_signal<bool,WRITER_POLICY>와 동일 패턴 (bool 자리에 sc_dt::sc_logic)
    // 주의: 원문 선언에는 get_writer_policy가 나열되어 있지 않다
    private:
        // Disabled
        sc_signal( const sc_signal<sc_dt::sc_logic,WRITER_POLICY>& );
};

}          // namespace sc_core
```

### 6.5.3 Member functions

나머지는 6.4 참조 (목록 불완전).

| 함수 | 규칙 |
|---|---|
| `posedge_event()` | `read()` 값이 변하고 새 값이 `true`/`'1'`일 때마다 notify되는 event 참조 shall |
| `negedge_event()` | 값이 변하고 새 값이 `false`/`'0'`일 때마다 notify되는 event 참조 shall |
| `posedge()` | **직전 delta cycle의 update phase**에 현재 시각에서 값이 변했고 새 값이 `true`/`'1'`인 경우에만 `true` shall |
| `negedge()` | 같은 조건, 새 값이 `false`/`'0'`인 경우에만 `true` shall |

### Example

```cpp
SC_MODULE(M) {
   sc_core::sc_signal<bool> clk;
   ...

     void thread_process() {
         for (;;) {
             if (clk.posedge())
                  wait(clk.negedge_event());
                  ...
         }
     }
};
```

---

## 6.6 `sc_buffer`

### Description
`sc_signal`에서 파생된 predefined primitive channel. **버퍼에 write될 때마다** value-changed event가 notify된다 — 값이 바뀌었을 때만 notify되는 `sc_signal`과 다르다.

### Class definition

```cpp
namespace sc_core {

template <class T, sc_writer_policy WRITER_POLICY = SC_DEFAULT_WRITER_POLICY>
class sc_buffer
: public sc_signal<T,WRITER_POLICY>
{
    public:
        sc_buffer();
        explicit sc_buffer( const char* );
        sc_buffer( const char* name_, const T& initial_value_ );

        virtual void write( const T& );

        sc_buffer<T,WRITER_POLICY>& operator= ( const T& );
        sc_buffer<T,WRITER_POLICY>& operator= ( const sc_signal<T,WRITER_POLICY>& );
        sc_buffer<T,WRITER_POLICY>& operator= ( const sc_buffer<T,WRITER_POLICY>& );

        virtual const char* kind() const;

    protected:
        virtual void update();

    private:
        // Disabled
        sc_buffer( const sc_buffer<T,WRITER_POLICY>& );
};

}         // namespace sc_core
```

### 6.6.3 Constructors

| 생성자 | base 호출 |
|---|---|
| `sc_buffer()` | `sc_signal( sc_gen_unique_name("buffer") )` shall |
| `explicit sc_buffer( const char* name_ )` | `sc_signal( name_ )` shall |
| `sc_buffer( const char* name_, const T& initial_value_ )` | `sc_signal( name_ )` shall. **event와 뒤따르는 delta cycle 없이** `initial_value_`로 초기화 shall |

### 6.6.4 Member functions

- `write( const T& )` — 다음 delta cycle에 새 값으로 보이도록 수정 shall. primitive channel의 update request 메커니즘 사용 shall.
- `operator=` — `write(arg)` / `write(arg.read())`와 동등 shall (세 오버로드 모두).
- `update()` — `sc_signal::update`를 override shall. 현재 값을 새 값으로 바꾸고, **버퍼 값이 변했는지와 무관하게** value-changed event를 바로 뒤 delta notification phase에 notify하도록 해야 shall.
  - 즉 현재 값이 V일 때 `write(V)`를 호출하면, `write`는 새 값 V를 **implementation-defined 저장 영역**(현재 값과 구분되는)에 넣고 `request_update`를 호출한다. update phase에서 `update`가 불려 현재 값을 V로 설정하며, 값은 그대로지만 **value-changed event는 notify된다**.
- `kind()` — `"sc_buffer"` shall.

### Example

```cpp
SC_MODULE(M) {
   sc_core::sc_buffer<int> buf;

     SC_CTOR(M) {
        SC_THREAD(writer);
        SC_METHOD(reader);
        sensitive << buf;
     }

     void writer() {
         buf.write(1);
         wait(sc_core::SC_ZERO_TIME);
         buf.write(1);
     }

     void reader() {                    // initialization 때 실행되고 이후 buf = 0, 1, 1 로 두 번 더 실행
         std::cout << buf << std::endl;
     }
};
```

---

## 6.7 `sc_clock`

### Description
`sc_signal`에서 파생된 predefined primitive channel. 디지털 clock signal을 모델링. 값과 event는 **`sc_signal_in_if<bool>` 인터페이스를 통해 접근**된다.

### Class definition

```cpp
namespace sc_core {

class sc_clock
: public sc_signal<bool>
{
    public:
        sc_clock();
        explicit sc_clock( const char* name_ );

        sc_clock( const char* name_,
                  const sc_time& period_,
                  double duty_cycle_ = 0.5,
                  const sc_time& start_time_ = SC_ZERO_TIME,
                  bool posedge_first_ = true );

        sc_clock( const char* name_,
                  double period_v_,
                  sc_time_unit period_tu_,
                  double duty_cycle_ = 0.5 );

        sc_clock( const char* name_,
                  double period_v_,
                  sc_time_unit period_tu_,
                  double duty_cycle_,
                  double start_time_v_,
                  sc_time_unit start_time_tu_,
                  bool posedge_first_ = true );

        virtual ~sc_clock();

        virtual void write( const bool& );

        const sc_time& period() const;
        double duty_cycle() const;
        const sc_time& start_time() const;
        bool posedge_first() const;

        virtual const char* kind() const;

    protected:
        virtual void before_end_of_elaboration();

    private:
        // Disabled
        sc_clock( const sc_clock& );
        sc_clock& operator= ( const sc_clock& );
};

typedef sc_in<bool> sc_in_clk ;

}          // namespace sc_core
```

Disabled: 복사 생성자 **및 대입 연산자**.

### 6.7.3 Characteristic properties

| 속성 | 정의 | 제약 | 기본값 |
|---|---|---|---|
| **Period** | `false`→`true` 연속 전이 사이의 시간 간격. `true`→`false` 간격과 같아야 shall | **0보다 커야 shall** | **1 nanosecond** |
| **Duty cycle** | 한 주기 중 값이 `true`인 비율 | **0.0과 1.0 사이(양끝 배제, exclusive)** shall | **0.5** |
| **Start time** | clock 값의 첫 전이(`false→true` 또는 `true→false`)의 절대 시각 | — | **zero** |
| **Posedge_first** | `true`면 clock은 `false`로 초기화되고 start time에 `false→true`. `false`면 `true`로 초기화되고 start time에 `true→false` | — | **true** |

> NOTE — clock에는 stop time이 없지만, `sc_stop`이 호출되면 어떤 경우든 멈춘다.

### 6.7.4 Constructors

- 생성자는 인자로 주어진 characteristic property를 설정해야 shall. **인자로 주어지지 않은 속성은 6.7.3의 기본값**을 가져야 shall.
- default constructor는 base를 `sc_signal( sc_gen_unique_name("clock") )`로 호출해야 shall.

### 6.7.5 `write` — 호출 금지

- **애플리케이션이 `write`를 호출하면 error** shall. base class `sc_signal`의 `write`는 clock에 적용되지 않는다.

### 6.7.6 Diagnostic member functions

| 함수 | 규칙 |
|---|---|
| `period()` | clock의 period 반환 shall |
| `duty_cycle()` | duty cycle 반환 shall |
| `start_time()` | start time 반환 shall |
| `posedge_first()` | posedge_first 속성값 반환 shall |
| `kind()` | `"sc_clock"` 반환 shall |

### 6.7.7 `before_end_of_elaboration`

- `sc_prim_channel`에 정의된 `before_end_of_elaboration`을 구현이 override해야 shall. 그 동작은 **implementation-defined**.

> NOTE 1 — 구현은 `before_end_of_elaboration`을 이용해 clock을 생성하는 static process를 spawn할 수 있다 may.
> NOTE 2 — 파생 클래스에서 이 함수를 override하면, implementation-defined 동작을 부르기 위해 파생 클래스의 override에서 현재 클래스의 `before_end_of_elaboration`을 **명시적으로 호출해야 should**.

### 6.7.8 `sc_in_clk`

```cpp
typedef sc_in<bool> sc_in_clk ;
```
편의 및 하위 호환용 typedef. 애플리케이션은 `sc_in_clk`와 `sc_in<bool>`을 **교환 사용해도 된다 may**.

---

## 6.8 `sc_in`

### Description
signal용 특수화 포트 클래스. bind된 채널의 특정 멤버 함수에 편리하게 접근시킨다. 모듈의 **입력 핀** 모델링에 사용 가능.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_in
: public sc_port<sc_signal_in_if<T>,1>
{
    public:
        sc_in();
        explicit sc_in( const char* );
        virtual ~sc_in();

        virtual void bind ( const sc_signal_in_if<T>& );
        void operator() ( const sc_signal_in_if<T>& );

        virtual void bind ( sc_port<sc_signal_in_if<T>, 1>& );
        void operator() ( sc_port<sc_signal_in_if<T>, 1>& );

        virtual void bind ( sc_port<sc_signal_inout_if<T>, 1>& );
        void operator() ( sc_port<sc_signal_inout_if<T>, 1>& );

        virtual void end_of_elaboration();

        const T& read() const;
        operator const T& () const;

        const sc_event& default_event() const;
        const sc_event& value_changed_event() const;
        bool event() const;
        sc_event_finder& value_changed() const;

        virtual const char* kind() const;

    private:
        // Disabled
        sc_in( const sc_in<T>& );
        sc_in<T>& operator= ( const sc_in<T>& );
};

template <class T>
inline void sc_trace( sc_trace_file*, const sc_in<T>&, const std::string& );

}         // namespace sc_core
```

### 6.8.3 Member functions

- 생성자는 인자를 base class `sc_port`의 대응 생성자에 넘겨야 shall.
- `operator()`의 구현은 virtual 멤버 `bind` 호출로 효과를 내야 shall. `bind`는 파라미터를 그대로 인자로 넘겨 `sc_port::bind`를 호출해야 shall.
- `read()`, `operator const T&()` — `sc_port::operator->`로 bind된 객체의 `read`를 호출해야 shall: `(*this)->read()`
- `default_event`, `value_changed_event`, `event` — 각각 bind된 객체의 대응 멤버를 `operator->`로 호출해야 shall: `(*this)->event()`
- `value_changed()` — `sc_event_finder` 참조를 반환해야 shall. event finder 객체 자체는 `value_changed_event`로 구성되어야 shall (5.7 참조).
- `kind()` — `"sc_in"` shall.

### 6.8.4 `sc_trace`

```cpp
template <class T>
inline void sc_trace( sc_trace_file*, const sc_in<T>&, const std::string& );
```
- 두 번째 인자 포트가 bind된 **채널**을 trace해야 shall (8.1.1). 두 번째 인자 타입이 `const T&`인 `sc_trace`를 호출해서 수행 shall (6.4.3).
- **`sc_trace` 호출 시점에 포트가 bind되어 있지 않아도 된다.** 이 경우 구현은 포트가 bind되어 signal 정체가 알려질 때까지 trace 호출을 **연기해야 shall**.

### 6.8.5 `end_of_elaboration`

- `sc_port`에 정의된 `end_of_elaboration`을 구현이 override해야 shall. 동작은 **implementation-defined**.

> NOTE 1 — 구현은 `end_of_elaboration`으로 `sc_trace`의 지연 호출을 구현할 수 있다 may.
> NOTE 2 — 파생 클래스에서 override하면 현재 클래스의 `end_of_elaboration`을 명시적으로 호출해야 should.

---

## 6.9 `sc_in<bool>` / `sc_in<sc_dt::sc_logic>`

### Description
2치 신호용 추가 멤버 함수를 제공하는 특수화 포트 클래스.

### Class definition

```cpp
namespace sc_core {

template <>
class sc_in<bool>
: public sc_port<sc_signal_in_if<bool>,1>
{
    public:
        sc_in();
        explicit sc_in( const char* );
        virtual ~sc_in();

        virtual void bind ( const sc_signal_in_if<bool>& );
        void operator() ( const sc_signal_in_if<bool>& );

        virtual void bind ( sc_port<sc_signal_in_if<bool>, 1>& );
        void operator() ( sc_port<sc_signal_in_if<bool>, 1>& );

        virtual void bind ( sc_port<sc_signal_inout_if<bool>, 1>& );
        void operator() ( sc_port<sc_signal_inout_if<bool>, 1>& );

        virtual void end_of_elaboration();

        const bool& read() const;
        operator const bool& () const;

        const sc_event& default_event() const;
        const sc_event& value_changed_event() const;
        const sc_event& posedge_event() const;
        const sc_event& negedge_event() const;

        bool event() const;
        bool posedge() const;
        bool negedge() const;

        sc_event_finder& value_changed() const;
        sc_event_finder& pos() const;
        sc_event_finder& neg() const;

        virtual const char* kind() const;

    private:
        // Disabled
        sc_in( const sc_in<bool>& );
        sc_in<bool>& operator= ( const sc_in<bool>& );
};

template <>
inline void sc_trace<bool>( sc_trace_file*, const sc_in<bool>&, const std::string& );

template <>
class sc_in<sc_dt::sc_logic>
: public sc_port<sc_signal_in_if<sc_dt::sc_logic>,1>
{
    // ... sc_in<bool>과 동일 패턴 (bool 자리에 sc_dt::sc_logic)
    private:
        // Disabled
        sc_in( const sc_in<sc_dt::sc_logic>& );
        sc_in<sc_dt::sc_logic>& operator= ( const sc_in<sc_dt::sc_logic>& );
};

template <>
inline void
sc_trace<sc_dt::sc_logic>( sc_trace_file*, const sc_in<sc_dt::sc_logic>&, const std::string& );

}         // namespace sc_core
```

### 6.9.3 Member functions

나머지 멤버와 `sc_trace`는 6.8.3 참조 (목록 불완전).

- `posedge_event`, `negedge_event`, `posedge`, `negedge` — 각각 bind된 객체의 대응 멤버를 `operator->`로 호출해야 shall: `(*this)->negedge()`
- **`pos()` / `neg()`** — `sc_event_finder` 참조를 반환해야 shall. event finder 객체는 각각 `posedge_event` / `negedge_event`로 구성되어야 shall (5.7 참조).

---

## 6.10 `sc_inout`

### Description
signal용 특수화 포트 클래스. 모듈의 **출력 핀 또는 양방향 핀** 모델링에 사용 가능.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_inout
: public sc_port<sc_signal_inout_if<T>,1>
{
    public:
        sc_inout();
        explicit sc_inout( const char* );
        virtual ~sc_inout();

        void initialize( const T& );
        void initialize( const sc_signal_in_if<T>& );

        virtual void end_of_elaboration();

        const T& read() const;
        operator const T& () const;

        void write( const T& );
        sc_inout<T>& operator= ( const T& );
        sc_inout<T>& operator= ( const sc_signal_in_if<T>& );
        sc_inout<T>& operator= ( const sc_port< sc_signal_in_if<T>, 1>& );
        sc_inout<T>& operator= ( const sc_port< sc_signal_inout_if<T>, 1>& );
        sc_inout<T>& operator= ( const sc_inout<T>& );

        const sc_event& default_event() const;
        const sc_event& value_changed_event() const;
        bool event() const;
        sc_event_finder& value_changed() const;

        virtual const char* kind() const;

    private:
        // Disabled
        sc_inout( const sc_inout<T>& );
};

template <class T>
inline void sc_trace( sc_trace_file*, const sc_inout<T>&, const std::string& );

}          // namespace sc_core
```

### 6.10.3 Member functions

- 생성자는 인자를 base `sc_port`의 대응 생성자에 넘겨야 shall.
- `read()`, `operator const T&()` — `(*this)->read()` shall.
- `write`, `operator=` — bind된 객체의 `write`를 `operator->`로 호출해야 shall. 파라미터가 인터페이스나 포트이면 `read`로 값을 얻는다:
  ```cpp
  sc_inout<T>& operator= ( const sc_inout<T>& port_ )
  { (*this)->write( port_->read() ); return *this; }
  ```
- **`write`는 포트가 bind되기 전 elaboration 중에 호출하면 안 된다 shall not** (6.10.4 참조).
- `default_event`, `value_changed_event`, `event` — `(*this)->event()` 식으로 호출 shall.
- `value_changed()` — `value_changed_event`로 구성된 `sc_event_finder` 참조 반환 shall.
- `kind()` — `"sc_inout"` shall.

### 6.10.4 `initialize`

- bind된 signal의 초기값을 그 signal의 `write`를 호출해 설정해야 shall. 실인자가 채널이면 그 채널 값을 읽어 초기값을 정한다 shall.
- **`initialize` 호출 시점에 포트가 bind되어 있지 않아도 된다.** 이 경우 구현은 포트가 bind되어 signal 정체가 알려질 때까지 `write` 호출을 **연기해야 shall**.

> NOTE 1 — `sc_in` 포트는 정확히 하나의 signal에 bind되지만, bind는 부모 모듈의 포트를 통해 간접적으로 이뤄질 수 있다.
> NOTE 2 — `initialize`의 목적은 포트가 bind되기 전에 elaboration 중 값을 초기화하는 것이다. 다만 `initialize`는 **elaboration 중에도 simulation 중에도 호출될 수 있다 may**.

### 6.10.5 `sc_trace`

`sc_in`과 동일: bind된 채널을 trace shall, 미bind 시 구현이 호출을 연기해야 shall.

### 6.10.6 `end_of_elaboration`

`sc_port`의 `end_of_elaboration`을 구현이 override해야 shall. 동작은 **implementation-defined**.

> NOTE 1 — 구현은 이를 이용해 `initialize`와 `sc_trace`의 지연 호출을 구현할 수 있다 may.
> NOTE 2 — 파생 클래스에서 override하면 현재 클래스 버전을 명시 호출해야 should.

### 6.10.7 Binding

`sc_signal_inout_if`가 `sc_signal_in_if`에서 파생되므로:

- 자식 모듈의 **`sc_in` 포트 → 부모 모듈의 `sc_inout` 포트** bind: **가능**.
- 자식 모듈의 **`sc_inout` 포트 → 부모 모듈의 `sc_in` 포트** bind: **불가능(cannot)**.

---

## 6.11 `sc_inout<bool>` / `sc_inout<sc_dt::sc_logic>`

### Description
2치 신호용 추가 멤버 함수를 제공하는 특수화 포트 클래스.

### Class definition

```cpp
namespace sc_core {

template <>
class sc_inout<bool>
: public sc_port<sc_signal_inout_if<bool>,1>
{
    public:
        sc_inout();
        explicit sc_inout( const char* );
        virtual ~sc_inout();

        void initialize( const bool& );
        void initialize( const sc_signal_in_if<bool>& );

        virtual void end_of_elaboration();

        const bool& read() const;
        operator const bool& () const;

        void write( const bool& );
        sc_inout<bool>& operator= ( const bool& );
        sc_inout<bool>& operator= ( const sc_signal_in_if<bool>& );
        sc_inout<bool>& operator= ( const sc_port< sc_signal_in_if<bool>, 1>& );
        sc_inout<bool>& operator= ( const sc_port< sc_signal_inout_if<bool>, 1>& );
        sc_inout<bool>& operator= ( const sc_inout<bool>& );

        const sc_event& default_event() const;
        const sc_event& value_changed_event() const;
        const sc_event& posedge_event() const;
        const sc_event& negedge_event() const;

        bool event() const;
        bool posedge() const;
        bool negedge() const;

        sc_event_finder& value_changed() const;
        sc_event_finder& pos() const;
        sc_event_finder& neg() const;

        virtual const char* kind() const;

    private:
        // Disabled
        sc_inout( const sc_inout<bool>& );
};

template <>
inline void sc_trace<bool>( sc_trace_file*, const sc_inout<bool>&, const std::string& );

template <>
class sc_inout<sc_dt::sc_logic>
: public sc_port<sc_signal_inout_if<sc_dt::sc_logic>,1>
{
    // ... sc_inout<bool>과 동일 패턴 (bool 자리에 sc_dt::sc_logic)
    private:
        // Disabled
        sc_inout( const sc_inout<sc_dt::sc_logic>& );
};

template <>
inline void
sc_trace<sc_dt::sc_logic>( sc_trace_file*, const sc_inout<sc_dt::sc_logic>&, const std::string& );

}         // namespace sc_core
```

### 6.11.3 Member functions

나머지 멤버와 `sc_trace`는 6.10 참조 (목록 불완전).

- `posedge_event`, `negedge_event`, `posedge`, `negedge` — `(*this)->negedge()` 식으로 bind된 객체의 대응 멤버 호출 shall.
- `pos()` / `neg()` — 각각 `posedge_event` / `negedge_event`로 구성된 `sc_event_finder` 참조 반환 shall (5.7).
- `kind()` — `"sc_inout"` shall.

---

## 6.12 `sc_out`

### Description
`sc_inout`에서 파생. 파생 클래스라는 데서 오는 차이(생성자·대입 연산자 등)를 빼면 `sc_inout`과 동일. 두 클래스를 모두 두는 목적은 **의도 표현** — `sc_out`은 출력 핀, `sc_inout`은 양방향 핀.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_out
: public sc_inout<T>
{
    public:
        sc_out();
        explicit sc_out( const char* );
        virtual ~sc_out();

        sc_out<T>& operator= ( const T& );
        sc_out<T>& operator= ( const sc_signal_in_if<T>& );
        sc_out<T>& operator= ( const sc_port< sc_signal_in_if<T>, 1>& );
        sc_out<T>& operator= ( const sc_port< sc_signal_inout_if<T>, 1>& );
        sc_out<T>& operator= ( const sc_out<T>& );

        virtual const char* kind() const;

    private:
        // Disabled
        sc_out( const sc_out<T>& );
};

}         // namespace sc_core
```

### 6.12.3 Member functions

- 생성자는 인자를 base `sc_inout<T>`의 대응 생성자에 넘겨야 shall.
- 대입 연산자 동작은 `sc_inout`과 동일하되 클래스명만 `sc_out`으로 치환한 것이어야 shall.
- `kind()` — `"sc_out"` shall.

---

## 6.13 `sc_signal_resolved`

### Description
`sc_signal`에서 파생된 predefined primitive channel. **resolved signal**은 `sc_signal_resolved` 또는 `sc_signal_rv` 객체를 말한다. `sc_signal`과 달리 **여러 process가 쓸 수 있고**, 충돌 값은 채널 내에서 resolve된다.

### Class definition

```cpp
namespace sc_core {

class sc_signal_resolved
: public sc_signal<sc_dt::sc_logic,SC_MANY_WRITERS>
{
    public:
        sc_signal_resolved();
        explicit sc_signal_resolved( const char* );
        sc_signal_resolved( const char* name_, const data_type & initial_value_ );
        virtual ~sc_signal_resolved();

        virtual void register_port( sc_port_base&, const char* );

        virtual void write( const sc_dt::sc_logic& );
        sc_signal_resolved& operator= ( const sc_dt::sc_logic& );
        sc_signal_resolved& operator= ( const sc_signal_resolved& );

        virtual const char* kind() const;

    protected:
        virtual void update();

    private:
        // Disabled
        sc_signal_resolved( const sc_signal_resolved& );
};

}         // namespace sc_core
```

### 6.13.3 Constructors

| 생성자 | base 호출 |
|---|---|
| `sc_signal_resolved()` | `sc_signal( sc_gen_unique_name("signal_resolved") )` shall |
| `explicit sc_signal_resolved( const char* name_ )` | `sc_signal( name_ )` shall |
| `sc_signal_resolved( const char* name_, const data_type& initial_value_ )` | `sc_signal( name_ )` shall. **event와 뒤따르는 delta cycle 없이** 초기화 shall |

### 6.13.4 Resolution semantics

- `write` 또는 `operator=`로 쓴다. `sc_signal`처럼 `operator=`는 `write`를 호출해야 shall.
- 각 resolved signal은 **written values 리스트**를 유지해야 shall — 그 객체에 쓴 **각각의 서로 다른 process instance마다 하나의 값**. 리스트는 각 process instance가 가장 최근에 쓴 값을 저장 shall.
- 쓴 값이 이전에 쓴 값과 **다르거나**, 그 process instance가 **처음 쓰는 경우에만(if and only if)** `write`가 `request_update`를 호출해야 shall.
- update phase에서 `update`는 먼저 written values 리스트로 단일 resolved value를 계산하고, 그 값으로 `sc_signal`과 유사한 update 의미론을 수행해야 shall.
- 값은 각 process instance가 **처음 쓸 때** 리스트에 추가되어야 shall. **리스트에서 값이 제거되면 안 된다 shall not.** 처음 쓰기 전까지 그 process instance는 resolved value 계산에 **기여하지 않아야 shall**.

**resolved value 계산 알고리즘**
1. 리스트의 복사본을 만든다.
2. 복사본에서 임의의 두 값을 골라 Table 3의 진리표에 따라 하나로 치환한다.
3. 값이 하나 남을 때까지 2)를 반복한다. 그것이 resolved value.

**Table 3 — Resolution table for `sc_signal_resolved`**

|   | `'0'` | `'1'` | `'Z'` | `'X'` |
|---|---|---|---|---|
| **`'0'`** | `'0'` | `'X'` | `'0'` | `'X'` |
| **`'1'`** | `'X'` | `'1'` | `'1'` | `'X'` |
| **`'Z'`** | `'0'` | `'1'` | `'Z'` | `'X'` |
| **`'X'`** | `'X'` | `'X'` | `'X'` | `'X'` |

- 어떤 process instance가 처음 쓰기 전에는, 그 process가 쓴 값은 resolution 계산상 사실상 `'Z'`다.
- 반면 resolved signal의 **기본 초기값**(첫 write 전 `read`가 반환할 값)은 **`'X'`** 다.
- 따라서 **resolved signal에 쓰는 각 process instance는 time zero에 한 번 write를 수행하는 것이 강력히 권장된다(strongly recommended)**.

> NOTE 1 — Table 3 함수에 값을 넘기는 순서는 결과에 영향을 주지 않는다.
> NOTE 2 — resolved value 계산은 그 signal에 쓰는 **모든** process가 가장 최근에 쓴 값으로 수행된다 — 가장 최근 write가 현재 delta cycle인지, 이전 delta cycle인지, 더 이전 시각인지와 무관하게.
> NOTE 3 — 동일한 resolution 의미론이 process가 직접 접근하든 포트를 통해 간접 접근하든 똑같이 적용된다.

### 6.13.5 Member functions

- `register_port` — `sc_signal`의 것을 override해야 shall. `sc_signal::register_port`가 수행하는 **multiple output port error check가 비활성화**되도록 한다.
- `write`, `operator=`, `update` — 6.13.4의 다중 writer 관련 차이를 제외하면 `sc_signal`의 대응 멤버와 동일한 동작 shall.
- `kind()` — `"sc_signal_resolved"` shall.

### Example

```cpp
SC_MODULE(M) {
   sc_core::sc_signal_resolved sig;

     SC_CTOR(M) {
        SC_THREAD(T1);
        SC_THREAD(T2);
        SC_THREAD(T3);
     }

     void T1() {
                                                        // Time=0 ns, written value 없음        sig=X
          wait(10.0, sc_core::SC_NS);
          sig = sc_dt::SC_LOGIC_0;                      // Time=10 ns, written values=0         sig=0
          wait(20.0, sc_core::SC_NS);
          sig = sc_dt::SC_LOGIC_Z;                      // Time=30 ns, written values=Z,Z       sig=Z
     }

     void T2() {
         wait(20.0, sc_core::SC_NS);
         sig = sc_dt::SC_LOGIC_Z;                       // Time=20 ns, written values=0,Z       sig=0
         wait(30.0, sc_core::SC_NS);
         sig = sc_dt::SC_LOGIC_0;                       // Time=50 ns, written values=Z,0,1     sig=X
}

     void T3() {
         wait(40.0, sc_core::SC_NS);
         sig = sc_dt::SC_LOGIC_1;                       // Time=40 ns, written values=Z,Z,1     sig=1
     }
};
```

---

## 6.14 `sc_in_resolved`

### Description
resolved signal용 특수화 포트. 파생 원본인 `sc_in<sc_dt::sc_logic>`과 동작이 유사. **유일한 차이**: `sc_in_resolved` 포트는 **`sc_signal_resolved` 클래스 채널에 bind되어야 shall**. (`sc_in<sc_dt::sc_logic>`은 `sc_signal<sc_dt::sc_logic,WRITER_POLICY>` 또는 `sc_signal_resolved` 어느 쪽에도 bind될 수 있다 may.)

### Class definition

```cpp
namespace sc_core {

class sc_in_resolved
: public sc_in<sc_dt::sc_logic>
{
    public:
        sc_in_resolved();
        explicit sc_in_resolved( const char* );
        virtual ~sc_in_resolved();

        virtual void end_of_elaboration();

        virtual const char* kind() const;

    private:
        // Disabled
        sc_in_resolved( const sc_in_resolved& );
        sc_in_resolved& operator= (const sc_in_resolved& );
};

}         // namespace sc_core
```

### 6.14.3 Member functions

- 생성자는 인자를 base `sc_in<sc_dt::sc_logic>`의 대응 생성자에 넘겨야 shall.
- **`end_of_elaboration`은 error check를 수행해야 shall — 포트가 `sc_signal_resolved` 채널에 bind되지 않았으면 error.**
- `kind()` — `"sc_in_resolved"` shall.

> NOTE — 언제나 그렇듯 포트는 부모 모듈의 포트를 통해 간접적으로 bind될 수 있다.

---

## 6.15 `sc_inout_resolved`

### Description
resolved signal용 특수화 포트. `sc_inout<sc_dt::sc_logic>`에서 파생. **유일한 차이**: `sc_signal_resolved` 채널에 bind되어야 shall.

### Class definition

```cpp
namespace sc_core {

class sc_inout_resolved
: public sc_inout<sc_dt::sc_logic>
{
    public:
        sc_inout_resolved();
        explicit sc_inout_resolved( const char* );
        virtual ~sc_inout_resolved();

        virtual void end_of_elaboration();

        sc_inout_resolved& operator= ( const sc_dt::sc_logic& );
        sc_inout_resolved& operator= ( const sc_signal_in_if<sc_dt::sc_logic>& );
        sc_inout_resolved& operator= ( const sc_port<sc_signal_in_if<sc_dt::sc_logic>, 1>& );
        sc_inout_resolved& operator= ( const sc_port<sc_signal_inout_if<sc_dt::sc_logic>, 1>& );
        sc_inout_resolved& operator= ( const sc_inout_resolved& );

        virtual const char* kind() const;

    private:
        // Disabled
        sc_inout_resolved( const sc_inout_resolved& );
};

}          // namespace sc_core
```

### 6.15.3 Member functions

- 생성자는 인자를 base `sc_inout<sc_dt::sc_logic>`의 대응 생성자에 넘겨야 shall.
- **`end_of_elaboration`은 error check 수행 shall — `sc_signal_resolved` 채널에 bind되지 않았으면 error.**
- 대입 연산자 동작은 `sc_inout<sc_dt::sc_logic>`과 동일하되 클래스명만 치환 shall.
- `kind()` — `"sc_inout_resolved"` shall.

> NOTE — 포트는 부모 모듈 포트를 통해 간접 bind될 수 있다.

---

## 6.16 `sc_out_resolved`

### Description
`sc_inout_resolved`에서 파생. 파생 클래스라는 데서 오는 차이(생성자·대입 연산자)를 빼면 동일. 의도 표현용 — `sc_out_resolved`는 resolved signal에 연결된 출력 핀, `sc_inout_resolved`는 양방향 핀.

### Class definition

```cpp
namespace sc_core {

class sc_out_resolved
: public sc_inout_resolved
{
    public:
        sc_out_resolved();
        explicit sc_out_resolved( const char* );
        virtual ~sc_out_resolved();

        sc_out_resolved& operator= ( const sc_dt::sc_logic& );
        sc_out_resolved& operator= ( const sc_signal_in_if<sc_dt::sc_logic>& );
        sc_out_resolved& operator= ( const sc_port<sc_signal_in_if<sc_dt::sc_logic>, 1>& );
        sc_out_resolved& operator= ( const sc_port<sc_signal_inout_if<sc_dt::sc_logic>, 1>& );
        sc_out_resolved& operator= ( const sc_out_resolved& );

        virtual const char* kind() const;

    private:
        // Disabled
        sc_out_resolved( const sc_out_resolved& );
};

}         // namespace sc_core
```

### 6.16.3 Member functions

- 생성자는 인자를 base `sc_inout_resolved`의 대응 생성자에 넘겨야 shall.
- 대입 연산자 동작은 `sc_inout_resolved`와 동일하되 클래스명 치환 shall.
- `kind()` — `"sc_out_resolved"` shall.

---

## 6.17 `sc_signal_rv`

### Description
`sc_signal`에서 파생된 predefined primitive channel. `sc_signal_resolved`와 유사하나, base template `sc_signal`의 인자가 `sc_dt::sc_logic`이 아니라 **`sc_dt::sc_lv<W>`**.

### Class definition

```cpp
namespace sc_core {

template <int W>
class sc_signal_rv
: public sc_signal<sc_dt::sc_lv<W>,SC_MANY_WRITERS>
{
    public:
        sc_signal_rv();
        explicit sc_signal_rv( const char* );
        virtual ~sc_signal_rv();

        virtual void register_port( sc_port_base&, const char* );

        virtual void write( const sc_dt::sc_lv<W>& );
        sc_signal_rv<W>& operator= ( const sc_dt::sc_lv<W>& );
        sc_signal_rv<W>& operator= ( const sc_signal_rv<W>& );

        virtual const char* kind() const;

    protected:
        virtual void update();

    private:
        // Disabled
        sc_signal_rv( const sc_signal_rv<W>& );
};

}         // namespace sc_core
```

### 6.17.3 Semantics and member functions

- 의미론은 resolve 대상 값이 `sc_dt::sc_lv`라는 차이를 빼면 **`sc_signal_resolved`와 동일해야 shall** (6.13.4).
- 값은 resolved signal을 통해 **atomic하게 전파되어야 shall** — 어느 process가 쓴 벡터의 **어느 한 비트라도 바뀌면** event가 notify되고 **벡터 전체**가 resolve되고 update되어야 shall.
- written values 리스트는 `sc_dt::sc_lv` 값을 담아야 shall. 리스트 구성·갱신 목적에서 각 `sc_dt::sc_lv` 값은 **atomic하게 취급되어야 shall**.
- 쓴 값이 이전 쓴 값과 (한 비트 이상에서) **다르거나** 그 process가 **처음 쓰는 경우에만(if and only if)** `write`가 `request_update`를 호출해야 shall.
- resolved value는 벡터의 **각 비트 위치마다** 6.13.4 규칙을 적용해 벡터 전체에 대해 계산되어야 shall.
- default constructor는 base를 `sc_signal( sc_gen_unique_name("signal_rv") )`로 호출해야 shall.
- `kind()` — `"sc_signal_rv"` shall.

---

## 6.18 `sc_in_rv`

### Description
resolved signal용 특수화 포트. `sc_in<sc_dt::sc_lv<W>>`에서 파생. **유일한 차이**: `sc_in_rv` 포트는 **`sc_signal_rv` 채널에 bind되어야 shall**. (`sc_in<sc_dt::sc_lv<W>>`은 `sc_signal<sc_dt::sc_lv<W>,WRITER_POLICY>` 또는 `sc_signal_rv`에 bind될 수 있다 may.)

### Class definition

```cpp
namespace sc_core {

template <int W>
class sc_in_rv
: public sc_in<sc_dt::sc_lv<W>>
{
    public:
        sc_in_rv();
        explicit sc_in_rv( const char* );
        virtual ~sc_in_rv();

        virtual void end_of_elaboration();

        virtual const char* kind() const;

    private:
        // Disabled
        sc_in_rv( const sc_in_rv<W>& );
        sc_in_rv<W>& operator= ( const sc_in_rv<W>& );
};

}         // namespace sc_core
```

### 6.18.3 Member functions

- 생성자는 인자를 base `sc_in<sc_dt::sc_lv<W>>`의 대응 생성자에 넘겨야 shall.
- **`end_of_elaboration`은 error check 수행 shall — `sc_signal_rv` 채널에 bind되지 않았으면 error.**
- `kind()` — `"sc_in_rv"` shall.

> NOTE — 포트는 부모 모듈 포트를 통해 간접 bind될 수 있다.

---

## 6.19 `sc_inout_rv`

### Description
resolved signal용 특수화 포트. `sc_inout<sc_dt::sc_lv<W>>`에서 파생. **유일한 차이**: `sc_signal_rv` 채널에 bind되어야 shall.

### Class definition

```cpp
namespace sc_core {

template <int W>
class sc_inout_rv
: public sc_inout<sc_dt::sc_lv<W>>
{
    public:
        sc_inout_rv();
        explicit sc_inout_rv( const char* );
        virtual ~sc_inout_rv();

        sc_inout_rv<W>& operator= ( const sc_dt::sc_lv<W>& );
        sc_inout_rv<W>& operator= ( const sc_signal_in_if<sc_dt::sc_lv<W>>& );
        sc_inout_rv<W>& operator= ( const sc_port<sc_signal_in_if<sc_dt::sc_lv<W>>, 1>& );
        sc_inout_rv<W>& operator= ( const sc_port<sc_signal_inout_if<sc_dt::sc_lv<W>>, 1>& );
        sc_inout_rv<W>& operator= ( const sc_inout_rv<W>& );

        virtual void end_of_elaboration();

        virtual const char* kind() const;

    private:
        // Disabled
        sc_inout_rv( const sc_inout_rv<W>& );
};

}          // namespace sc_core
```

### 6.19.3 Member functions

- 생성자는 인자를 base `sc_inout<sc_dt::sc_lv<W>>`의 대응 생성자에 넘겨야 shall.
- **`end_of_elaboration`은 error check 수행 shall — `sc_signal_rv` 채널에 bind되지 않았으면 error.**
- 대입 연산자 동작은 `sc_inout<sc_dt::sc_lv<W>>`와 동일하되 클래스명 치환 shall.
- `kind()` — `"sc_inout_rv"` shall.

---

## 6.20 `sc_out_rv`

### Description
`sc_inout_rv`에서 파생. 파생 클래스 차이(생성자·대입 연산자)를 빼면 동일. 의도 표현용 — `sc_out_rv`는 resolved vector에 연결된 출력 핀, `sc_inout_rv`는 양방향 핀.

### Class definition

```cpp
namespace sc_core {

template <int W>
class sc_out_rv
: public sc_inout_rv<W>
{
    public:
        sc_out_rv();
        explicit sc_out_rv( const char* );
        virtual ~sc_out_rv();

        sc_out_rv<W>& operator= ( const sc_dt::sc_lv<W>& );
        sc_out_rv<W>& operator= ( const sc_signal_in_if<sc_dt::sc_lv<W>>& );
        sc_out_rv<W>& operator= ( const sc_port<sc_signal_in_if<sc_dt::sc_lv<W>>, 1>& );
        sc_out_rv<W>& operator= ( const sc_port<sc_signal_inout_if<sc_dt::sc_lv<W>>, 1>& );
        sc_out_rv<W>& operator= ( const sc_out_rv<W>& );

        virtual const char* kind() const;

    private:
        // Disabled
        sc_out_rv( const sc_out_rv<W>& );
};

}          // namespace sc_core
```

### 6.20.3 Member functions

- 생성자는 인자를 base `sc_inout_rv<W>`의 대응 생성자에 넘겨야 shall.
- 대입 연산자 동작은 `sc_inout_rv<W>`와 동일하되 클래스명 치환 shall.
- `kind()` — `"sc_out_rv"` shall.

---

## 6.21 `sc_fifo_in_if`

### Description
interface proper. predefined channel `sc_fifo`가 구현한다. fifo 채널에 **read 접근**을 주며, 두 개의 하위 interface proper `sc_fifo_nonblocking_in_if`와 `sc_fifo_blocking_in_if`에서 파생된다.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_fifo_nonblocking_in_if
: virtual public sc_interface
{
    public:
        virtual bool nb_read( T& ) = 0;
        virtual const sc_event& data_written_event() const = 0;
};

template <class T>
class sc_fifo_blocking_in_if
: virtual public sc_interface
{
    public:
        virtual void read( T& ) = 0;
        virtual T read() = 0;
};

template <class T>
class sc_fifo_in_if : public sc_fifo_nonblocking_in_if<T>, public sc_fifo_blocking_in_if<T>
{
    public:
        virtual int num_available() const = 0;

    protected:
        sc_fifo_in_if();

    private:
        // Disabled
        sc_fifo_in_if( const sc_fifo_in_if<T>& );
        sc_fifo_in_if<T>& operator= ( const sc_fifo_in_if<T>& );
};

}          // namespace sc_core
```

### 6.21.3 Member functions

모두 pure virtual. 정확한 의미론은 채널마다 다르다.

| 함수 | 규칙 |
|---|---|
| `read`, `nb_read` | fifo에 **가장 먼저 쓰인(least recently written)** 값을 반환하고 그 값을 fifo에서 **제거**해 다시 읽을 수 없게 해야 shall |
| `read` (fifo 비었을 때) | 값이 쓰일 때까지 **suspend해야 shall** |
| `nb_read` (fifo 비었을 때) | **즉시 반환해야 shall**. 반환값이 값을 읽었는지 여부를 나타내야 shall |
| `data_written_event` | fifo에 값이 쓰일 때마다 notify되는 event 참조 shall |
| `num_available` | 현재 fifo에서 읽을 수 있는 값의 개수 반환 shall |

**`void read(T&)` 호출 시 애플리케이션 의무 (`sc_fifo_blocking_in_if`)**
- 실인자의 **수명이 함수 호출 시점부터 호출 완료 시점까지 유지되도록 보장해야 shall**.
- 그 기간 동안 실인자의 값을 **수정하면 안 된다 shall not**.

---

## 6.22 `sc_fifo_out_if`

### Description
interface proper. `sc_fifo`가 구현. fifo 채널에 **write 접근**을 주며 `sc_fifo_nonblocking_out_if`와 `sc_fifo_blocking_out_if`에서 파생.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_fifo_nonblocking_out_if
: virtual public sc_interface
{
    public:
        virtual bool nb_write( const T& ) = 0;
        virtual const sc_event& data_read_event() const = 0;
};

template <class T>
class sc_fifo_blocking_out_if
: virtual public sc_interface
{
    public:
        virtual void write( const T& ) = 0;
};

template <class T>
class sc_fifo_out_if : public sc_fifo_nonblocking_out_if<T>, public sc_fifo_blocking_out_if<T>
{
    public:
        virtual int num_free() const = 0;

    protected:
        sc_fifo_out_if();

    private:
        // Disabled
        sc_fifo_out_if( const sc_fifo_out_if<T>& );
        sc_fifo_out_if<T>& operator= ( const sc_fifo_out_if<T>& );
};

}          // namespace sc_core
```

### 6.22.3 Member functions

| 함수 | 규칙 |
|---|---|
| `write`, `nb_write` | 인자 값을 fifo에 써야 shall |
| `write` (fifo 가득 찼을 때) | 값이 읽힐 때까지 **suspend해야 shall** |
| `nb_write` (fifo 가득 찼을 때) | **즉시 반환해야 shall**. 반환값은 **빈 슬롯에 값이 쓰였는지** 여부를 나타내야 shall |
| `data_read_event` | fifo에서 값이 읽힐 때마다 notify되는 event 참조 shall |
| `num_free` | 쓰기를 받을 수 있는 **비어 있는 슬롯 개수** 반환 shall |

**`void write(const T&)` 호출 시 애플리케이션 의무 (`sc_fifo_blocking_out_if`)** — `read`와 동일: 실인자 수명을 호출 완료까지 보장해야 shall, 그 기간 값 수정 금지 shall not.

---

## 6.23 `sc_fifo`

### Description
first-in-first-out 버퍼를 모델링하는 predefined primitive channel. 각 fifo는 값을 저장하는 **슬롯**을 갖고, **슬롯 개수는 객체 생성 시 고정**된다.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_fifo
: public sc_fifo_in_if<T>, public sc_fifo_out_if<T>, public sc_prim_channel
{
    public:
        explicit sc_fifo( int size_ = 16 );
        explicit sc_fifo( const char* name_, int size_ = 16);
        virtual ~sc_fifo();

        virtual void register_port( sc_port_base&, const char* );

        virtual void read( T& );
        virtual T read();
        virtual bool nb_read( T& );
        operator T ();

        virtual void write( const T& );
        virtual bool nb_write( const T& );
        sc_fifo<T>& operator= ( const T& );

        virtual const sc_event& data_written_event() const;
        virtual const sc_event& data_read_event() const;

        virtual int num_available() const;
        virtual int num_free() const;

        virtual void print( std::ostream& = std::cout ) const;
        virtual void dump( std::ostream& = std::cout ) const;
        virtual const char* kind() const;

    protected:
        virtual void update();

    private:
        // Disabled
        sc_fifo( const sc_fifo<T>& );
        sc_fifo& operator= ( const sc_fifo<T>& );
};

template <class T>
inline std::ostream& operator<< ( std::ostream&, const sc_fifo<T>& );

}          // namespace sc_core
```

Disabled: 복사 생성자 **및 `sc_fifo<T>` 대입 연산자**.

### 6.23.3 Template parameter T

인자는 대입의 predefined 의미론으로 충분한 C++ 타입이거나, 아래를 **모두** 지키는 타입 T여야 shall.

| 항목 | 규칙 |
|---|---|
| a) `std::ostream& operator<< ( std::ostream&, const T& );` | **정의되어야 shall**. 상태를 스트림에 복사해야 should. 포맷은 **undefined by this standard**. 구현은 `print`/`dump`에 사용 shall |
| b) `const T& operator= ( const T& );` | 기본 대입 의미론이 상태 대입에 부적합하면 정의해야 should. 구현은 fifo 슬롯에 값을 쓰거나 읽어낼 때 이 연산자를 사용 shall |
| c) 기본 생성자 | T에 어떤 생성자라도 존재하면 default constructor를 정의해야 shall |

> 주의: `sc_signal`(6.4.3)과 달리 `operator==`와 `sc_trace` 요구가 **없다**.

> NOTE 1 — 대입 연산자가 객체 상태 전부를 대입할 의무는 없다(보통은 그래야 should).
> NOTE 2 — SystemC data types proper는 모두 위 규칙을 만족한다.
> NOTE 3 — `sc_module*`을 fifo로 넘기는 것은 legal하지만 module hierarchy 남용이며 bad practice.

### 6.23.4 Constructors

| 생성자 | base 호출 |
|---|---|
| `explicit sc_fifo( int size_ = 16 )` | `sc_prim_channel( sc_gen_unique_name("fifo") )` shall |
| `explicit sc_fifo( const char* name_, int size_ = 16 )` | `sc_prim_channel( name_ )` shall |

- **기본 슬롯 개수는 16.**
- 두 생성자 모두 슬롯 개수를 `size_`로 초기화해야 shall. **슬롯 개수는 0보다 커야 shall.**

### 6.23.5 `register_port`

`sc_interface::register_port`를 override해 error check를 수행해야 shall.

- **`sc_fifo_in_if` 타입 포트를 둘 이상 bind하면 error.**
- **`sc_fifo_out_if` 타입 포트를 둘 이상 bind하면 error.**

### 6.23.6 Member functions for reading — delta cycle 규칙

| 함수 | 규칙 |
|---|---|
| `read(T&)`, `read()`, `nb_read(T&)` | 가장 먼저 쓰인 값을 반환하고 fifo에서 제거해야 shall. **한 delta cycle 안에 여러 값을 읽어도 된다 may**. 읽히는 순서는 쓰인 순서와 **정확히 일치해야 shall** |
| — 핵심 | **현재 delta cycle에 쓰인 값은 그 delta cycle에서 읽을 수 없고, 바로 다음 delta cycle에서 읽을 수 있게 된다** |
| `read` (비었을 때) | data-written event가 notify될 때까지 **suspend해야 shall**. 그 시점에 (바로 뒤 evaluation phase에서) resume하여 가장 먼저 쓰인 값의 읽기를 완료한 뒤 반환 shall |
| `nb_read` (비었을 때) | **fifo 상태를 수정하지 않고, `request_update`를 호출하지 않고, `false`를 반환하며 즉시 반환 shall**. 읽을 값이 있으면 `true` shall |
| `operator T ()` | `operator T (){ return read(); }` 와 동등 shall |

### 6.23.7 Member functions for writing — delta cycle 규칙

| 함수 | 규칙 |
|---|---|
| `write(const T&)`, `nb_write(const T&)` | 인자 값을 fifo에 써야 shall. **한 delta cycle 안에 여러 값을 써도 된다 may** |
| — 핵심 | **현재 delta cycle에 값을 읽어 생긴 빈 슬롯은 그 delta cycle에서는 쓰기용으로 free해지지 않고, 바로 다음 delta cycle이 되어야 free해진다** |
| `write` (가득 찼을 때) | data-read event가 notify될 때까지 **suspend해야 shall**. 그 시점에 (바로 뒤 evaluation phase에서) resume해 쓰기를 완료한 뒤 반환 shall |
| `nb_write` (가득 찼을 때) | **fifo 상태를 수정하지 않고, `request_update`를 호출하지 않고, `false`를 반환하며 즉시 반환 shall**. free 슬롯이 있으면 `true` shall |
| `operator=` | `sc_fifo<T>& operator= ( const T& a ) { write( a ); return *this; }` 와 동등 shall |

### 6.23.8 The update phase

- `read`, `nb_read`, `write`, `nb_write`는 `sc_prim_channel::request_update`를 호출해 읽기/쓰기 행위를 완료해야 shall.
- `update()` — `sc_prim_channel::update`를 override해 **읽을 수 있는 값의 개수와 쓰기 가능한 free 슬롯 개수를 갱신**해야 shall. 필요에 따라 data-written event 또는 data-read event를 **바로 뒤 delta notification phase**에 notify하도록 해야 shall.

> NOTE — fifo가 비어 있는데 같은 delta cycle의 evaluation phase에서 `write`와 `read`가 (같은 process든 다른 두 process든) 모두 호출되면, write는 그 delta cycle에 완료되지만 read는 fifo가 비어 있으므로 suspend한다. 읽을 수 있는 값 개수는 update phase에서 1로 증가하고, read는 다음 delta cycle에 방금 쓴 값을 반환하며 완료된다.

### 6.23.9 Member functions for events

| 함수 | 규칙 |
|---|---|
| `data_written_event()` | data-written event 참조 반환 shall. **값이 fifo에 쓰인 delta cycle의 끝에 오는 delta notification phase**에 notify된다 |
| `data_read_event()` | data-read event 참조 반환 shall. **값이 fifo에서 읽힌 delta cycle의 끝에 오는 delta notification phase**에 notify된다 |

### 6.23.10 `num_available` / `num_free`

| 함수 | 규칙 |
|---|---|
| `num_available()` | **현재 delta cycle에서** 읽을 수 있는 값의 개수 반환 shall. 계산은 현재 delta cycle에 읽힌 값을 **차감해야 shall**, 현재 delta cycle에 쓰인 값을 **더하면 안 된다 shall not** |
| `num_free()` | **현재 delta cycle에서** 쓰기용으로 free한 빈 슬롯 개수 반환 shall. 현재 delta cycle에 쓰인 슬롯을 **차감해야 shall**, 현재 delta cycle의 읽기로 비워진 슬롯을 **더하면 안 된다 shall not** |

### 6.23.11 Diagnostic member functions

| 함수 | 규칙 |
|---|---|
| `print(std::ostream&)` | 읽기 가능한 저장 값 목록을 **쓰인 순서대로** `operator<< (std::ostream&, T&)`로 출력 shall. 포맷은 **implementation-defined** |
| `dump(std::ostream&)` | 최소한 fifo의 **hierarchical name**과 읽기 가능한 저장 값 목록을 출력 shall. 포맷은 **implementation-defined** |
| `kind()` | `"sc_fifo"` shall |

### 6.23.12 `operator<<`

```cpp
template <class T>
inline std::ostream& operator<< ( std::ostream&, const sc_fifo<T>& );
```
`print`를 호출해 두 번째 인자 fifo의 내용을 첫 인자 스트림에 `operator<< (std::ostream&, T&)`로 출력 shall.

### Example

```cpp
SC_MODULE(M) {
   sc_core::sc_fifo<int> fifo;

     SC_CTOR(M) : fifo(4) {
        SC_THREAD(T);
     }

     void T() {
         int d;
         fifo.write(1);
         fifo.print(std::cout);                         // 1
         fifo.write(2);
         fifo.print(std::cout);                         // 1 2
         fifo.write(3);
         fifo.print(std::cout);                         // 1 2 3
         std::cout << fifo.num_available();             // 읽을 수 있는 값 0개
         std::cout << fifo.num_free();                  // free 슬롯 1개
         fifo.read(d);                                  // read는 suspend, 다음 delta cycle에 반환
         fifo.print(std::cout);                         // 2 3
         std::cout << fifo.num_available();             // 읽을 수 있는 값 2개
         std::cout << fifo.num_free();                  // free 슬롯 1개
         fifo.read(d);
         fifo.print(std::cout);                         // 3
         fifo.read(d);
         fifo.print(std::cout);                         // 비어 있음
         std::cout << fifo.num_available();             // 읽을 수 있는 값 0개
         std::cout << fifo.num_free();                  // free 슬롯 1개
         wait(sc_core::SC_ZERO_TIME);
         std::cout << fifo.num_free();                  // free 슬롯 4개
     }
};
```

---

## 6.24 `sc_fifo_in`

### Description
fifo에서 **읽을 때** 쓰는 특수화 포트 클래스.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_fifo_in
: public sc_port<sc_fifo_in_if<T>,0>
{
    public:
        sc_fifo_in();
        explicit sc_fifo_in( const char* );
        virtual ~sc_fifo_in();

        void read( T& );
        T read();
        bool nb_read( T& );
        const sc_event& data_written_event() const;
        sc_event_finder& data_written() const;
        int num_available() const;
        virtual const char* kind() const;

    private:
        // Disabled
        sc_fifo_in( const sc_fifo_in<T>& );
        sc_fifo_in<T>& operator= ( const sc_fifo_in<T>& );
};

}          // namespace sc_core
```

### 6.24.3 Member functions

- 생성자는 인자를 base `sc_port`의 대응 생성자에 넘겨야 shall.
- `read`, `nb_read`, `data_written_event`, `num_available` — 각각 bind된 객체의 대응 멤버를 `operator->`로 호출해야 shall: `T read() { return (*this)->read(); }`
- `data_written()` — `data_written_event`로 구성된 `sc_event_finder` 참조 반환 shall (5.7).
- `kind()` — `"sc_fifo_in"` shall.

---

## 6.25 `sc_fifo_out`

### Description
fifo에 **쓸 때** 쓰는 특수화 포트 클래스.

### Class definition

```cpp
namespace sc_core {

template <class T>
class sc_fifo_out
: public sc_port<sc_fifo_out_if<T>,0>
{
    public:
        sc_fifo_out();
        explicit sc_fifo_out( const char* );
        virtual ~sc_fifo_out();

        void write( const T& );
        bool nb_write( const T& );
        const sc_event& data_read_event() const;
        sc_event_finder& data_read() const;
        int num_free() const;
        virtual const char* kind() const;

    private:
        // Disabled
        sc_fifo_out( const sc_fifo_out<T>& );
        sc_fifo_out<T>& operator= ( const sc_fifo_out<T>& );
};

}          // namespace sc_core
```

### 6.25.3 Member functions

- 생성자는 인자를 base `sc_port`의 대응 생성자에 넘겨야 shall.
- `write`, `nb_write`, `data_read_event`, `num_free` — 각각 bind된 객체의 대응 멤버를 `operator->`로 호출 shall: `void write( const T& a ) { (*this)->write( a ); }`
- `data_read()` — `data_read_event`로 구성된 `sc_event_finder` 참조 반환 shall (5.7).
- `kind()` — `"sc_fifo_out"` shall.

### Example

```cpp
// sc_fifo<>의 template 인자로 넘길 타입
class U {
 public:
    U(int val = 0) {                         // 생성자가 하나라도 있으면 default constructor 필요
         ptr = new int;
         *ptr = val;
    }

     int get() const { return *ptr; }

     void set(int i) { *ptr = i; }

     // 기본 대입 의미론이 부적합
     const U &operator=(const U &arg) {
         *(this->ptr) = *(arg.ptr);
         return *this;
     }

private:
    int *ptr;
};

// operator<< 필수
std::ostream &operator<<(std::ostream &os, const U &arg) { return (os << arg.get()); }

SC_MODULE(M1) {
   sc_core::sc_fifo_out<U> fifo_out;

     SC_CTOR(M1) {
        SC_THREAD(producer);
     }

     void producer() {
         U u;
         for (int i = 0; i < 4; i++) {
             u.set(i);
             bool status;
             do {
                 wait(1.0, sc_core::SC_NS);
                 status = fifo_out.nb_write(u);                  // Non-blocking write
             } while (!status);
         }
     }
};

SC_MODULE(M2) {
   sc_core::sc_fifo_in<U> fifo_in;

     SC_CTOR(M2) {
        SC_THREAD(consumer);
        sensitive << fifo_in.data_written();
     }

     void consumer() {
         for (;;) {
             wait(fifo_in.data_written_event());
             U u;
             bool status = fifo_in.nb_read(u);
             std::cout << u << " ";                              // 0 1 2 3
         }
     }
};

SC_MODULE(Top) {
     sc_core::sc_fifo<U> fifo;
     M1 m1;
     M2 m2;

     SC_CTOR(Top)
     : m1("m1"), m2("m2") {
         m1.fifo_out(fifo);
         m2.fifo_in(fifo);
     }
};
```

---

## 6.26 `sc_mutex_if`

### Description
interface proper. predefined channel `sc_mutex`가 구현한다.

### Class definition

```cpp
namespace sc_core {

class sc_mutex_if
: virtual public sc_interface
{
    public:
        virtual int lock() = 0;
        virtual int trylock() = 0;
        virtual int unlock() = 0;

    protected:
        sc_mutex_if();

    private:
        // Disabled
        sc_mutex_if( const sc_mutex_if& );
        sc_mutex_if& operator= ( const sc_mutex_if& );
};

}          // namespace sc_core
```

### 6.26.3 Member functions
멤버 함수의 동작은 `sc_mutex`(6.27)에서 정의된다.

---

## 6.27 `sc_mutex`

### Description
동시 process가 공유하는 자원 접근을 제어하는 **mutual exclusion lock**을 모델링하는 predefined channel.

- mutex는 **unlocked 또는 locked 두 배타적 상태 중 하나**여야 shall.
- **한 시점에 단 하나의 process만** 주어진 mutex를 lock할 수 있다.
- mutex는 **그것을 lock한 바로 그 process instance만 unlock할 수 있다**. 이후 다른 process가 lock하는 것은 가능 may.

### Class definition

```cpp
namespace sc_core {

class sc_mutex
: public sc_mutex_if, public sc_object
{
    public:
        sc_mutex();
        explicit sc_mutex( const char* );

        virtual int lock();
        virtual int trylock();
        virtual int unlock();

        virtual const char* kind() const;

    private:
        // Disabled
        sc_mutex( const sc_mutex& );
        sc_mutex& operator= ( const sc_mutex& );
};

}          // namespace sc_core
```

### 6.27.3 Constructors

| 생성자 | base 호출 |
|---|---|
| `sc_mutex()` | `sc_object( sc_gen_unique_name("mutex") )` shall |
| `explicit sc_mutex( const char* name_ )` | `sc_object( name_ )` shall |

두 생성자 모두 **mutex를 unlock 상태로** 만들어야 shall.

### 6.27.4 Member functions

| 함수 | 규칙 | 반환값 |
|---|---|---|
| `lock()` | unlocked면 lock하고 반환 shall. locked면 (다른 process에 의해) unlock될 때까지 **suspend해야 shall**; 그 시점에 resume해 같은 규칙을 다시 적용해 lock 시도 shall | **무조건 0** shall |
| `trylock()` | unlocked면 lock하고 반환 shall. locked면 **즉시 반환**하며 mutex는 locked로 남아야 shall | unlocked였으면 `0`, locked였으면 `–1` shall |
| `unlock()` | 이미 unlocked면 아무 일 없이 반환, mutex는 unlocked로 남음 shall. **호출 process가 아닌 다른 process instance가 lock한 상태면** mutex는 locked로 남음 shall. 호출 process가 lock한 상태면 unlock shall | 앞의 두 경우 `–1`, 성공 시 `0` shall |
| `kind()` | — | `"sc_mutex"` shall |

- **같은 delta cycle에 여러 process가 lock을 시도하면**, 그 delta cycle에 어느 process instance가 lock을 얻는지는 **non-deterministic** — evaluation phase 내에서 process가 resume되는 순서에 달려 있다.
- `unlock` 성공 시 mutex unlock을 기다리며 suspend한 process가 있으면 **정확히 하나**에게 lock이 주어져야 shall (선택은 non-deterministic), 나머지는 다시 suspend shall. 이는 **단일 evaluation phase 안에서** 이뤄져야 shall — 즉 구현은 unlock 사실을 다른 process에 알리는 데 **immediate notification을 사용해야 shall**.

---

## 6.28 `sc_semaphore_if`

### Description
interface proper. predefined channel `sc_semaphore`가 구현한다.

### Class definition

```cpp
namespace sc_core {

class sc_semaphore_if
: virtual public sc_interface
{
    public:
        virtual int wait() = 0;
        virtual int trywait() = 0;
        virtual int post() = 0;
        virtual int get_value() const = 0;

    protected:
        sc_semaphore_if();

    private:
        // Disabled
        sc_semaphore_if( const sc_semaphore_if& );
        sc_semaphore_if& operator= ( const sc_semaphore_if& );
};

}          // namespace sc_core
```

### 6.28.3 Member functions
멤버 함수의 동작은 `sc_semaphore`(6.29)에서 정의된다.

---

## 6.29 `sc_semaphore`

### Description
공유 자원에 대한 **제한된 동시 접근**을 제공하는 software semaphore를 모델링하는 predefined channel. semaphore는 정수 값(**semaphore value**)을 가지며, 생성 시 허용 동시 접근 수로 설정된다.

### Class definition

```cpp
namespace sc_core {

class sc_semaphore
: public sc_semaphore_if, public sc_object
{
    public:
        explicit sc_semaphore( int );
        sc_semaphore( const char*, int );

        virtual int wait();
        virtual int trywait();
        virtual int post();
        virtual int get_value() const;

        virtual const char* kind() const;

    private:
        // Disabled
        sc_semaphore( const sc_semaphore& );
        sc_semaphore& operator= ( const sc_semaphore& );
};

}          // namespace sc_core
```

**default constructor 없음** — `int` 인자가 필수.

### 6.29.3 Constructors

| 생성자 | base 호출 |
|---|---|
| `explicit sc_semaphore( int )` | `sc_object( sc_gen_unique_name("semaphore") )` shall |
| `sc_semaphore( const char* name_, int )` | `sc_object( name_ )` shall |

두 생성자 모두 semaphore value를 `int` 파라미터 값으로 설정해야 shall. **그 값은 non-negative여야 shall.**

### 6.29.4 Member functions

| 함수 | 규칙 | 반환값 |
|---|---|---|
| `wait()` | value > 0이면 **감소시키고 반환** shall. value == 0이면 (다른 process가) 값을 증가시킬 때까지 **suspend**; 그 시점에 resume해 같은 규칙을 다시 적용 shall | **무조건 0** shall |
| `trywait()` | value > 0이면 감소시키고 반환 shall. value == 0이면 **값을 수정하지 않고 즉시 반환** shall | 성공 `0`, 실패 `–1` shall |
| `post()` | semaphore value를 **증가**시켜야 shall. 대기 중 suspend된 process가 있으면 **정확히 하나**만 값을 감소시키도록 허용되어야 shall(선택은 non-deterministic), 나머지는 다시 suspend shall. **단일 evaluation phase 안에서** 이뤄져야 shall — 구현은 **immediate notification을 사용해야 shall** | **무조건 0** shall |
| `get_value()` | semaphore value 반환 shall | — |
| `kind()` | — | `"sc_semaphore"` shall |

- **semaphore value는 음수가 되면 안 된다 shall not.**
- 같은 delta cycle에 여러 process가 값을 감소시키려 하면, 어느 process가 감소시키고 어느 process가 suspend하는지는 **non-deterministic** — evaluation phase 내 resume 순서에 달림.

> NOTE 1 — semaphore value는 서로 다른 process가 감소/증가시켜도 된다 may.
> NOTE 2 — semaphore value는 생성자로 설정한 값을 **초과할 수도 있다 may**.

---

## 6.30 `sc_event_queue`

### Description
event queue를 표현. `sc_event`처럼 `notify` 멤버 함수를 갖지만, `sc_event`와 달리 **hierarchical channel**이며 **여러 notification이 pending될 수 있다**.

### Class definition

```cpp
namespace sc_core {

class sc_event_queue_if
: public virtual sc_interface
{
    public:
        virtual void notify( double , sc_time_unit ) = 0;
        virtual void notify( const sc_time& ) = 0;
        virtual void cancel_all() = 0;
};

class sc_event_queue
: public sc_event_queue_if , public sc_module
{
    public:
        sc_event_queue( sc_module_name name_=
           sc_module_name(sc_gen_unique_name("event_queue")));
        ~sc_event_queue();

        virtual const char* kind() const;

        virtual void notify( double , sc_time_unit );
        virtual void notify( const sc_time& );
        virtual void cancel_all();

        virtual const sc_event& default_event() const;
};

}          // namespace sc_core
```

### 6.30.3 Constraints on usage

- `sc_event_queue`는 hierarchical channel이므로 **객체는 elaboration 중에만 생성 가능**.

> NOTE — `sc_event_queue` 객체는 `sc_event`가 필요한 대부분의 문맥에서 쓸 수 없지만, `sc_interface::default_event`를 구현하므로 **static sensitivity를 만드는 데는 쓸 수 있다**.

### 6.30.4 Constructors

```cpp
sc_event_queue( sc_module_name name_= sc_module_name(sc_gen_unique_name("event_queue")));
```
module name 인자를 base class `sc_module`의 생성자로 그대로 넘겨야 shall.

### 6.30.5 kind
`kind()`는 `"sc_event_queue"` 반환 shall.

### 6.30.6 Member functions

**`notify( double, sc_time_unit )` / `notify( const sc_time& )`**

- 인자가 **zero time**을 나타내면 default event에 **delta notification**을 일으켜야 shall.
- 인자가 **non-zero time**이면 default event에 **timed notification**을 일으켜야 shall. 시각은 `notify` 호출 시점의 simulation time에 상대적 — 즉 인자 값이 현재 simulation time에 **더해져** notify 시각이 정해진다.
- 이미 하나 이상의 notification이 pending인 상태에서 `notify`가 호출되면, 새 notification은 기존 pending에 **추가로 큐잉되어야 shall**. 각 큐잉된 notification은 `notify` 의미론이 정하는 시각에 발생해야 shall — **`notify` 호출 순서와 무관하게**.
- **default event는 어느 한 delta cycle에 두 번 이상 notify되면 안 된다 shall not.** 같은 delta cycle에 여러 notification이 pending이면 그것들은 **연속된 delta cycle**에 발생해야 shall. 같은 simulation time에 여러 timed notification이 pending이면, 그 시각의 **첫 delta cycle부터 시작해 delta cycle 시퀀스에 빈틈 없이 연속으로** 발생해야 shall.

**`cancel_all()`**
- 이 event queue 객체의 **모든 pending notification**(delta·timed 모두)을 **즉시 삭제해야 shall**. 다른 event queue 객체에는 영향이 없어야 shall.

**`default_event()`**
- default event 참조를 반환해야 shall.
- **notification을 큐잉하는 메커니즘은 implementation-defined** — 단, event queue 객체는 `notify` 호출 한 번마다 한 번 notify되는 **단일 default event**를 제공해야 한다.

> NOTE — event queue notification은 anonymous하다 — default event가 나르는 정보는 notification 시각뿐이다. default event에 sensitive한 process instance는 **어느 `notify` 호출이 그 notification을 일으켰는지 알 수 없다**.

### Example

```cpp
SC_MODULE(Mod) {
   sc_core::sc_event_queue EQ;
   ...

     SC_CTOR(Mod) {
        SC_THREAD(T);
        SC_METHOD(M);
        sensitive << EQ;
        dont_initialize();
     }

     void T() {
          EQ.notify(2.0, sc_core::SC_NS);                        // M은 time 2ns에 실행
          EQ.notify(1.0, sc_core::SC_NS);                        // M은 time 1ns, 1번째 또는 2번째 delta cycle
          EQ.notify(sc_core::SC_ZERO_TIME);                      // M은 time 0ns에 실행
          EQ.notify(1.0, sc_core::SC_NS);                        // M은 time 1ns, 2번째 또는 1번째 delta cycle
     }
};
```

---

## 6.31 `sc_stub`, `sc_unbound`, `sc_tie`

### Class definition

```cpp
namespace sc_core
{

class sc_stub implementation-defined

implementation-defined sc_unbound;

template <typename T>
implementation-defined sc_tie::value(const T&);

} // namespace sc_core
```

### Rules

- `sc_stub`은 **stub 역할을 하는 predefined channel**을 정의해야 shall. 이 클래스는 `sc_unbound`와 `sc_tie` 정의에 사용된다. **클래스 정의는 implementation-defined.**

**`sc_unbound`**
- static object `sc_unbound`는 **unbound connection**을 나타내는 predefined channel을 정의해야 shall.
- 애플리케이션에서 `sc_unbound`를 사용할 때마다 **새 predefined channel이 생성되어야 shall**. 이름은 접두어 `"sc_unbound"` + 밑줄 + 0–9 문자로 된 하나 이상의 십진 숫자열.
- **`sc_port<sc_signal_inout_if<T>>` 타입이 아닌 포트에 `sc_unbound`를 bind하면 error** shall.
- 이 채널에 대한 read와 write는 **허용되어야 shall**. **읽히는 값은 implementation-defined.** 쓰인 값은 **무시되어야 shall**. 값을 써도 **event를 notify하면 안 된다 shall not**.

> NOTE — 값이 undefined이므로 애플리케이션은 `sc_port<sc_signal_in_if<T>>` 타입 입력 포트에 `sc_unbound`를 사용할 수 없다.

**`sc_tie::value`**
- 포트를 지정된 타입 T의 값으로 tie하기 위한 predefined channel을 반환해야 shall. **값의 타입은 포트가 bind되는 연관 채널의 타입과 호환되어야 shall.**
- `sc_tie`를 사용할 때마다 **새 predefined channel이 생성되어야 shall**. 이름은 접두어 `"sc_tie"` + 밑줄 + 하나 이상의 십진 숫자열.
- **`sc_port<sc_signal_in_if<T>>` 또는 `sc_port<sc_signal_inout_if<T>>` 타입이 아닌 포트에 `sc_tie`를 bind하면 error** shall.
- read와 write는 허용되어야 shall. **읽히는 값은 지정된 predefined type에 대응해야 shall.** 쓰인 값은 무시되어야 shall. 값을 써도 event를 notify하면 안 된다 shall not.

---

## 흔한 위반 (pitfalls)

- **두 process가 같은 `sc_signal`에 write** — LRM §6.4.4: `SC_ONE_WRITER`(기본)에서는 시뮬레이션 중 어느 시점이든 둘 이상의 process instance가 쓰면 error. 여러 writer가 필요하면 `sc_signal<T, SC_MANY_WRITERS>`를 쓰되, 그래도 **같은 evaluation phase 안의 다중 writer는 여전히 error**다. 진짜 다중 구동이면 `sc_signal_resolved` / `sc_signal_rv`를 쓴다.
- **`sc_signal_inout_if` 포트를 한 signal에 둘 이상 bind** — LRM §6.4.6: `SC_ONE_WRITER`이면 error. `SC_MANY_WRITERS`이거나 `sc_signal_resolved`(§6.13.5에서 이 검사를 비활성화)를 써야 한다.
- **`SC_DEFAULT_WRITER_POLICY`가 항상 `SC_ONE_WRITER`라고 가정** — LRM §6.3.3: 기본값은 `SC_ONE_WRITER`와 같아야 하지만 **구현이 다른 값을 정의할 수 있다**. 정책에 의존하는 코드는 template 인자를 **명시**하라.
- **같은 evaluation phase에서 signal에 쓰고 바로 읽어 새 값을 기대** — LRM §6.4.4 NOTE 3: 옛 값이 읽힌다. 새 값은 다음 evaluation phase에서 읽힌다.
- **한 evaluation phase에서 여러 번 write한 값이 모두 반영될 것으로 기대** — LRM §6.4.4: last write wins. 중간 값은 사라진다.
- **값이 같은 값을 다시 써도 event가 날 것으로 기대** — LRM §6.4.4: `sc_signal`은 값이 **실제로 변한 경우에만** value-changed event를 notify한다. 매 write마다 event가 필요하면 `sc_buffer`(§6.6.4)를 쓴다.
- **`sc_clock`에 `write()` 호출** — LRM §6.7.5: 애플리케이션이 `sc_clock::write`를 호출하면 error. base `sc_signal`의 `write`는 clock에 적용되지 않는다.
- **duty cycle을 0.0이나 1.0으로 설정** — LRM §6.7.3: duty cycle은 0.0과 1.0 **사이(양끝 배제)** 여야 shall. period도 **0보다 커야 shall**.
- **resolved signal에 time zero write를 안 함** — LRM §6.13.4: 첫 write 전 그 process의 기여값은 사실상 `'Z'`지만, resolved signal의 **기본 초기값은 `'X'`**. 각 writer process가 **time zero에 한 번 write하는 것이 strongly recommended**.
- **`sc_in_resolved`/`sc_inout_resolved`를 일반 `sc_signal<sc_logic>`에 bind** — LRM §6.14.3, §6.15.3: `end_of_elaboration`의 error check에서 error. (`sc_in_rv`/`sc_inout_rv`는 §6.18.3, §6.19.3에서 `sc_signal_rv` 요구.)
- **자식 모듈의 `sc_inout`을 부모의 `sc_in`에 bind** — LRM §6.10.7: 불가능. 반대 방향(자식 `sc_in` → 부모 `sc_inout`)만 된다.
- **bind 전에 `sc_inout::write()` 호출** — LRM §6.10.3: elaboration 중 포트가 bind되기 전에 `write`를 호출하면 안 된다 shall not. 초기값 설정에는 `initialize()`를 쓴다 — bind 전 호출이 허용되고 구현이 호출을 연기한다(§6.10.4).
- **fifo에 write한 값을 같은 delta cycle에 읽으려 함** — LRM §6.23.6: 현재 delta cycle에 쓰인 값은 그 cycle에서 읽을 수 없다. 다음 delta cycle에 읽을 수 있다.
- **fifo에서 read해 생긴 빈 슬롯에 같은 delta cycle에 write** — LRM §6.23.7: 읽어서 생긴 빈 슬롯은 다음 delta cycle이 되어야 free해진다.
- **`num_available()`/`num_free()`가 즉시 갱신될 것으로 기대** — LRM §6.23.10: `num_available`은 현재 cycle의 write를 더하지 않고, `num_free`는 현재 cycle의 read로 비워진 슬롯을 더하지 않는다.
- **하나의 fifo에 같은 방향 포트를 둘 이상 bind** — LRM §6.23.5: `sc_fifo_in_if` 포트 둘 이상, 또는 `sc_fifo_out_if` 포트 둘 이상이면 error.
- **`nb_read`/`nb_write` 반환값 무시** — LRM §6.23.6, §6.23.7: 실패 시 fifo 상태를 전혀 바꾸지 않고 `false`를 반환한다. 반환값을 확인하지 않으면 데이터가 조용히 유실된다.
- **blocking `read(T&)`/`write(const T&)`에 임시 객체를 넘김** — LRM §6.21.3, §6.22.3: 호출 완료까지 실인자 수명을 유지해야 shall, 그 사이 값을 수정해서도 안 된다 shall not. 이 함수들은 suspend할 수 있으므로 지역 변수를 쓰고 그동안 건드리지 마라.
- **`sc_fifo`의 T에 `operator<<`를 정의하지 않음** — LRM §6.23.3 a): 정의되어야 shall. 생성자가 하나라도 있으면 **default constructor도 필수** shall (c).
- **`sc_signal`의 T에 `operator==`를 정의하지 않음** — LRM §6.4.3 a): 정의되어야 shall — 구현이 event 발생 판정에 쓴다. 포트를 bind할 거면 `sc_trace(sc_trace_file*, const T&, const std::string&)`도 정의해야 shall (e).
- **다른 process가 lock한 mutex를 unlock** — LRM §6.27.4: `unlock`은 `–1`을 반환하고 mutex는 locked로 남는다. 반환값을 확인하지 않으면 조용히 실패한다.
- **`trylock`/`trywait` 반환값을 확인하지 않음** — LRM §6.27.4, §6.29.4: 실패 시 `–1`을 반환하고 상태를 바꾸지 않는다.
- **여러 process가 경쟁할 때 lock/semaphore 획득 순서를 가정** — LRM §6.27.4, §6.29.4: 어느 process가 획득하는지는 **non-deterministic**.
- **semaphore를 음수 인자로 생성** — LRM §6.29.3: 생성자 `int` 값은 non-negative여야 shall. 또한 §6.29.4: semaphore value는 음수가 되면 안 된다 shall not.
- **`sc_event_queue`를 simulation 중 생성** — LRM §6.30.3: hierarchical channel이므로 **elaboration 중에만** 생성 가능.
- **한 delta cycle에 여러 event queue notification이 함께 발생할 것으로 기대** — LRM §6.30.6: default event는 한 delta cycle에 두 번 이상 notify되지 않는다 shall not; 여러 pending notification은 연속된 delta cycle에 나뉘어 발생한다.
- **`sc_unbound`를 입력 포트에 사용** — LRM §6.31: `sc_port<sc_signal_inout_if<T>>`가 아닌 포트에 bind하면 error이며, 읽히는 값이 implementation-defined이므로 입력 포트에는 쓸 수 없다.
