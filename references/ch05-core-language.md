# Clause 5 — Core language class definitions

LRM pp. 61–164. 모듈·프로세스·포트·이벤트·시간. **SystemC 코드의 뼈대 전부.**

각 절은 LRM 형식을 따른다: Description → Class definition → Constraints on usage → Constructors → Member functions.

---

## 5.1 헤더 파일

| 헤더 | 도입되는 이름 |
|---|---|
| `#include "systemc"` | `sc_core`, `sc_dt`, `sc_unnamed` **세 이름만** shall. 표준 C/C++ 라이브러리 이름은 일절 도입 안 함 shall not |
| `#include "systemc.h"` | `sc_core`·`sc_dt`의 **모든** 이름 + `sc_unnamed` + 선별된 std 이름 (`std::cout`, `std::endl`, `std::string` 계열 C 함수 등) |

**`systemc`를 권장**(recommended). `systemc.h`는 하위 호환용이며 향후 deprecated 가능.

```cpp
#include "systemc"
using sc_core::sc_module;
using sc_core::sc_signal;
using sc_core::SC_NS;
using sc_core::sc_start;
using sc_dt::sc_logic;
```

`systemc.h`가 끌어오는 std 이름 (일부): `ios streambuf streampos streamsize iostream istream ostream cin cout cerr endl flush dec hex oct fstream ifstream ofstream size_t memchr memcmp memcpy memmove memset strcat strncat strchr strrchr strcmp strncmp strcpy strncpy strcspn strspn strlen strpbrk strstr strtok`.

---

## 5.2 `sc_module`

모듈은 SystemC의 주된 구조적 빌딩 블록.

### Class definition (요약)

```cpp
namespace sc_core {
class sc_bind_proxy† { implementation-defined };
const sc_bind_proxy† SC_BIND_PROXY_NIL;

class sc_module : public sc_object {
public:
    virtual ~sc_module();
    virtual const char* kind() const;                 // "sc_module"
    void operator() ( const sc_bind_proxy†& p001, ... p064 = SC_BIND_PROXY_NIL );  // positional binding
    virtual const std::vector<sc_object*>& get_child_objects() const;
    virtual const std::vector<sc_event*>&  get_child_events() const;
protected:
    sc_module( const sc_module_name& );
    sc_module();
    void reset_signal_is      ( const sc_in<bool>&|sc_inout<bool>&|sc_out<bool>&|sc_signal_in_if<bool>&, bool );
    void async_reset_signal_is( 같은 4가지 오버로드, bool );
    sc_sensitive† sensitive;
    void dont_initialize();
    void set_stack_size( size_t );
    void next_trigger( ... );    // 12개 오버로드
    void wait( ... );            // 13개 오버로드
    virtual void before_end_of_elaboration();
    virtual void end_of_elaboration();
    virtual void start_of_simulation();
    virtual void end_of_simulation();
private:
    sc_module( const sc_module& );                    // Disabled
    sc_module& operator= ( const sc_module& );        // Disabled
};

// 동일 시그니처의 비멤버 next_trigger / wait 도 제공됨

#define SC_MODULE(name)            struct name : sc_module
#define SC_CTOR(name, ...)         implementation-defined; name(sc_module_name, implementation-defined)
#define SC_NAMED(identifier, ...)  implementation-defined
#define SC_METHOD(name)            implementation-defined
#define SC_THREAD(name)            implementation-defined
#define SC_CTHREAD(name, clk)      implementation-defined

const char* sc_gen_unique_name( const char* );
typedef sc_module sc_behavior;
typedef sc_module sc_channel;
}
```

### 5.2.3 Constraints on usage

- `sc_module` 객체는 **elaboration 중에만** 생성 가능. simulation 중 인스턴스화는 **error**.
- `sc_module` 파생 클래스는 **생성자를 최소 하나** 가져야 shall. 모든 생성자는 `sc_module_name` 타입 파라미터를 **정확히 하나** 가져야 shall (다른 타입 파라미터는 추가 가능). **첫 번째 파라미터일 필요는 없다.**
- 모든 모듈 인스턴스 생성자에 **문자열 인자를 넘겨야** shall. C++ 변수명과 같게 하는 것이 좋은 관행.
- 모듈 간 통신은 **interface method call**로 하는 것이 원칙 (should) — 모듈은 자신의 포트를 통해 환경과 통신. 디버깅·진단 목적의 다른 메커니즘은 허용.
- 생성자가 protected이므로 `sc_module` 자체는 직접 인스턴스화 불가, base class로만 사용.
- 모듈은 `sc_module`에서 **public 파생**되어야 should.
- **모듈을 다른 모듈에서 파생시키는 것은 허용**되고 유용한 관용구.

### 5.2.5 `SC_MODULE`

선택적 매크로. 아래 둘은 동등:

```cpp
SC_MODULE(M) {
    M(sc_core::sc_module_name) {}
};

class M : public sc_core::sc_module {
public:
    M(sc_core::sc_module_name) {}
};
```

### 5.2.7 `SC_CTOR`

C++가 생성자 선언을 허용하는 위치에서만 사용 shall. 첫 인자는 모듈 클래스명(필수). 추가 인자로 생성자 파라미터를 더 선언 가능.

```cpp
SC_MODULE(M1) {
    SC_CTOR(M1) : i(0) {}          // 생성자 정의
    int i;
};

SC_MODULE(M2) {
    SC_CTOR(M2);                   // 생성자 선언
    int i;
};
M2::M2(sc_core::sc_module_name) : i(0) {}

SC_MODULE(M3) {
    SC_CTOR(M3, int a, int b) {}   // 추가 파라미터
};
// 등가 수기 작성:
SC_MODULE(M4) {
    M4(sc_core::sc_module_name nm, int a, int b) : sc_core::sc_module(nm) {}
};
```

- **모듈당 최대 1회** 사용 shall.
- `sc_module_name`을 첫 번째가 아닌 위치에 두려면 생성자를 **명시적으로** 작성해야 shall (`SC_CTOR`은 항상 첫 파라미터로 선언).

### 5.2.8 `SC_METHOD` / `SC_THREAD` / `SC_CTHREAD`

- 인자(첫 인자)는 **멤버 함수 이름** shall. unspawned process instance 생성의 **유일한 방법** shall.
- `SC_CTHREAD`의 두 번째 인자는 **`sc_event_finder` 타입 표현식** shall.
- 매크로 호출은 **세미콜론으로 종료** shall.
- 호출 위치: **생성자 본문**, 모듈의 `before_end_of_elaboration`/`end_of_elaboration` 콜백, 또는 거기서 호출된 멤버 함수 안 shall only. **`SC_CTHREAD`는 `end_of_elaboration`에서 호출 금지** shall not.
- 첫 인자는 **같은 모듈의** 멤버 함수 이름 shall.
- unspawned process에 연관된 멤버 함수는 **반환형 `void`, 인자 없음** shall. (spawned process는 반환형·인자 가능.)
- **한 멤버 함수를 같은 모듈 내 여러 프로세스 인스턴스에 연관시킬 수 있다.** 각각 별도의 `sc_object` 파생 객체이며, 각 매크로는 멤버 함수 이름(따옴표 붙은 문자열)을 `sc_object` 생성자에 넘긴다. 각 인스턴스는 독립적인 static sensitivity를 갖고 독립적으로 트리거·재개된다 shall.
- 멤버 함수를 프로세스에 연관시켜도 그 함수를 애플리케이션이 직접 호출하는 것은 여전히 가능.

```cpp
SC_MODULE(M) {
    sc_core::sc_in<bool> clk;
    SC_CTOR(M) : clk("clk") {
        SC_METHOD(a_method);
        SC_THREAD(a_thread);
        SC_CTHREAD(a_cthread, clk.pos());
    }
    void a_method();
    void a_thread();
    void a_cthread();
};
```

### 5.2.9 `SC_NAMED`

식별자를 문자열로 변환해 (첫) 생성자 파라미터로 넘기는 매크로. 변수명과 인스턴스 문자열명 불일치를 원천 차단.

```cpp
SC_MODULE(M) {
    sc_core::sc_in<bool>     port_i;
    sc_core::sc_out<bool>    SC_NAMED(port_o);     // in-class member initializer
    sc_core::sc_signal<bool> sig;

    SC_CTOR(M)
    : SC_NAMED(port_i)
    , SC_NAMED(sig, true)                          // 파라미터 여러 개 지원
    {}
};
```

### 5.2.10 Method process (spawned/unspawned 모두)

- **triggered** = 커널이 연관 함수를 호출. 함수는 처음부터 끝까지 실행 후 커널에 제어 반환.
- method process는 **process handle의 `kill`로만 종료** 가능.
- static sensitivity 가질 수 있음. **method process만** `next_trigger`로 dynamic sensitivity 생성 가능.
- **자기 자신의 immediate notification으로는 runnable이 되지 않는다** (static/dynamic sensitivity 무관, §4.3.2.3).
- 구현이 별도 SW 스레드로 돌릴 의무 없음 — 커널과 같은 실행 컨텍스트에서 돌아도 됨.
- `kind()` == `"sc_method_process"`.

> 프로세스 내 지역 변수는 반환 시 소멸된다. 지속 상태는 **모듈의 데이터 멤버**로 둘 것 (should).

### 5.2.11 Thread / clocked thread process

- 연관 함수는 커널이 **정확히 한 번** 호출 shall. 예외: 프로세스가 reset되면 다시 호출될 수 있음.
- **thread/clocked thread만** `wait` 호출 가능. `wait`는 호출 프로세스를 **suspend**시킴.
- **resumed** = 커널이 가장 최근 `wait` 호출 **다음 문장**부터 실행 재개. 다음 `wait`까지 실행하고 다시 suspend.
- thread는 static sensitivity 가능 + `wait`로 dynamic sensitivity 생성. **clocked thread는 단 하나의 clock에만 static sensitive.**
- 자기 immediate notification으로 runnable 안 됨.
- 각 thread는 **자기 실행 스택**을 요구 → context switch 오버헤드가 method process보다 큼.
- 함수 본문을 끝까지 실행하거나 `return`하면 연관 함수는 **다시 호출되지 않는다** shall not → **terminated**.
- `kind()`: thread = `"sc_thread_process"`, clocked thread = `"sc_cthread_process"`.

> - 조기 종료를 막기 위해 `wait`를 포함한 무한 루프를 쓰는 것이 일반적 관용구.
> - 재개 시 지역 변수는 suspend 시점 값을 유지한다.
> - `wait` 없는 무한 루프는 절대 suspend하지 않음 → 스케줄러가 비선점이므로 **다른 어떤 프로세스도 실행 불가**.

```cpp
SC_MODULE(synchronous_module) {
    sc_in<bool> SC_NAMED(clock);
    SC_CTOR(synchronous_module) {
        SC_THREAD(thread);
        sensitive << clock.pos();
    }
    void thread() {              // 단 한 번만 호출되는 멤버 함수
        for (;;) {
            wait();              // clock의 posedge에서 재개
            ...
        }
    }
};
```

### 5.2.12 Clocked thread process

- **반드시 static process** shall. **spawn 불가.**
- `SC_CTHREAD` 두 번째 인자의 event finder가 반환하는 event **하나**에만 static sensitive shall.
- **`wait()`와 `wait(int)`만 호출 가능.** 다른 오버로드 호출은 **error** shall.
- synchronous/asynchronous reset signal을 **개수 제한 없이** 가질 수 있음.
- process control 함수 전부 호출 가능하나 clock에 비동기적으로 불릴 수 있어 주의:
  - **`disable`/`enable` 권장, `suspend`/`resume` 비권장.**
  - `resume`을 호출한다면 **clock event notification이 발생한 delta/timed notification phase 바로 다음 evaluation phase**에서 호출해야 shall. 그 외 시점이면 동작은 **구현 정의**.
  - `kill`, `reset`, `throw_it`은 clock에 비동기적으로 호출 가능하며 **효과는 즉시**. asynchronous reset이 active 값에 도달할 때도 즉시.
- **첫 clock event 발생 시**, reset signal의 active 여부와 무관하게 연관 함수가 호출된다 shall.
- **terminated된 clocked thread는 clock event가 무시된다. terminated process는 reset 불가.**

권장 본문 구조 (should): reset 동작 → `wait` 포함 루프 → 매 clock cycle 동작.

```cpp
SC_MODULE(M) {
    sc_core::sc_in<bool> SC_NAMED(clock);
    sc_core::sc_in<bool> SC_NAMED(reset);
    sc_core::sc_in<bool> SC_NAMED(async_reset);

    SC_CTOR(M) {
        SC_CTHREAD(CT1, clock.pos());
        reset_signal_is(reset, true);

        SC_CTHREAD(CT2, clock.pos());
        async_reset_signal_is(async_reset, true);
    }

    void CT1() {
        if (reset.read()) { ... }        // reset 동작
        while (true) {
            wait(true);                  // 1 clock cycle 대기
            ...                          // clocked 동작
        }
    }

    void CT2() {
        ...                              // reset 동작
        while (true) {
            try {
                while (true) {
                    wait();              // 1 clock cycle
                    ...
                }
            } catch (...) {              // 예외 발생
                ...                      // 처리 후 다시 clock 대기로
            }
        }
    }
};
```

### 5.2.13 `reset_signal_is` / `async_reset_signal_is`

thread, clocked thread, **method process 모두**에 대해 synchronous/asynchronous reset signal 지정.

- 호출 위치: **생성자 본문**, 모듈의 `before_end_of_elaboration` 콜백, 또는 거기서 호출된 멤버 함수 shall only. **그리고 같은 생성자·콜백 안에서 프로세스 인스턴스를 만든 뒤에만.**
- **가장 최근 생성된 프로세스 인스턴스**와 연관된다. 프로세스 생성과 이 호출 사이에 **모듈이 인스턴스화되면 효과는 undefined.**
- 첫 인자 = reset로 쓸 signal 인스턴스 (포트로 간접 지정 가능). 둘째 인자 = **active level** — reset signal 값이 이 값과 같을 때만 reset.

동작:
- reset signal이 active 값에 도달 → 프로세스는 **synchronous reset state**에 진입 shall. 이 상태에서는 **재개될 때마다** (event notification이든 time-out이든) reset된다 shall. reset의 효과는 `sc_process_handle::reset` 호출과 동일 (§5.6.6.4).
- suspend 중인 프로세스가 synchronous reset state에 들어가면 동작은 **구현 정의** (§5.6.6.12).
- **`async_reset_signal_is`만**: reset signal이 active 값에 도달할 때마다 **즉시** reset된다 shall. reset signal은 primitive channel이라 update phase에만 값이 바뀌므로, 실제로는 그 **직후 evaluation phase 중 커널이 정한 순서로** reset된다. asynchronous reset은 같은 evaluation phase의 다른 프로세스보다 **우선하지 않는다.**
- 우선순위: asynchronous reset signal의 active 도달 = `reset` 호출과 동일. synchronous reset state에서의 reset = `sync_reset_on`과 동일 (reset signal이 sync든 async든 무관).
- 한 프로세스는 sync/async reset signal을 **몇 개든** 가질 수 있음. **모든** reset signal이 active의 부정 값이 되면 synchronous reset state를 벗어난다 shall — 단 `sync_reset_on`이 유효 중이면 예외.
- **method process reset의 효과**: dynamic sensitivity 취소 + static sensitivity 복원 + 연관 함수 호출.

```cpp
SC_MODULE(M) {
    sc_core::sc_in<bool> SC_NAMED(clock);
    sc_core::sc_in<bool> SC_NAMED(reset);

    SC_CTOR(M) {
        SC_METHOD(rtl_proc);
        sensitive << clock.pos();
        async_reset_signal_is(reset, true);
    }

    void rtl_proc() {
        if (reset.read()) {
            ...   // 비동기 reset 동작. reset이 active인 동안 매번 실행
        } else {
            ...   // 동기 동작. posedge에서만 실행
        }
    }
};
```

### 5.2.14 `sensitive` — unspawned process의 static sensitivity

- `sc_sensitive†` 타입 데이터 멤버. `operator<<`로 static sensitivity 생성 (§5.4). **unspawned process에 대해 이것이 유일한 방법** shall. (spawned는 `sc_spawn_options::set_sensitivity`)
- 호출 위치: 생성자 본문, `before_end_of_elaboration` 또는 `end_of_elaboration` 콜백, 또는 거기서 호출된 멤버 함수 shall only. **같은 생성자·콜백에서 unspawned process를 만든 뒤에만.**
- **simulation 중 unspawned process의 static sensitivity 수정은 error.**
- **가장 최근 생성된** 프로세스 인스턴스와 연관.
- **clocked thread는 clock 외의 static sensitivity를 가질 수 없다.** `sensitive` 사용은 효과 없음.
- `sensitive`는 한 프로세스에 여러 번 사용 가능 — 매 `operator<<`가 sensitivity에 event를 추가.

```cpp
SC_MODULE(Mod) {
    sc_core::sc_signal<bool> SC_NAMED(A), SC_NAMED(B), SC_NAMED(C), SC_NAMED(D), SC_NAMED(E);
    SC_CTOR(Mod) {
        sensitive << A;          // 효과 없음. 비권장 스타일 (앞에 프로세스 없음)

        SC_THREAD(T);
        sensitive << B << C;     // T가 B, C에 sensitive

        SC_METHOD(M);
        f();                     // M이 D에 sensitive
        sensitive << E;          // M이 D에 더해 E에도 sensitive
    }
    void f() { sensitive << D; } // 비권장 스타일
    void T();
    void M();
};
```

### 5.2.15 `dont_initialize`

unspawned process 인스턴스가 **initialization phase에서 runnable이 되지 않게** 함. 즉 event로 트리거·재개될 때까지 연관 함수가 호출되지 않는다 shall.

- 호출 위치 규칙과 "가장 최근 생성 프로세스와 연관" 규칙은 `sensitive`와 동일 (`end_of_elaboration`까지 가능).
- 프로세스 생성과 이 호출 사이에 모듈이 인스턴스화되면 효과 **undefined**.
- **clocked thread에는 효과 없음** — 애초에 initialization phase에서 runnable이 되지 않음. 구현이 warning을 낼 수도 있음.

### 5.2.16 `set_stack_size`

thread/clocked thread 인스턴스의 실행 스택 크기 변경 요청. **효과는 구현 정의.**
- 호출 위치·연관 규칙은 위와 동일.
- **다른 시점에 호출하거나, method process에 대해 호출하면 error** shall.

### 5.2.17 `next_trigger` — method process 전용 dynamic sensitivity

`sc_module`·`sc_prim_channel`의 멤버 함수 및 비멤버 함수 전부에 적용. spawned/unspawned 모두 적용.

- 인자와 함께 호출 시, **바로 다음 트리거 1회에 한해** dynamic sensitivity를 설정 shall.
- 한 실행 중 여러 번 호출되면 **마지막 호출이 우선**하고 앞의 것은 취소된다 shall.
- 한 실행 중 호출하지 않으면 **static sensitivity대로** 다음 트리거.
- 인자 있는 호출은 **static sensitivity를 오버라이드**한다 shall.
- **thread/clocked thread에서 호출하면 error** shall.
- `next_trigger`는 method process를 suspend하지 않는다 — method process는 항상 끝까지 실행 후 반환.

| 형태 | 의미 |
|---|---|
| `next_trigger()` | static sensitivity로 트리거. static sensitivity가 없으면 **현재 simulation 동안 다시 트리거되지 않는다.** 현재 실행 중 앞선 `next_trigger` 호출을 **취소** — 즉 아예 호출 안 한 것과 동등 |
| `next_trigger(const sc_event&)` | 그 event가 notify될 때 |
| `next_trigger(const sc_event_or_list&)` | `\|`로 연결된 리스트. **하나라도** notify되면. 중복 event는 1회로 취급. **빈 리스트는 error** |
| `next_trigger(const sc_event_and_list&)` | `&`로 연결. **전부** notify되어야 함 (시각·순서 제약 없음). **가장 늦은 simulation time**에 notify되는 것이 마지막. 중복은 1회 취급. **빈 리스트는 error** |
| `next_trigger(const sc_time&)` | 그 시간 경과 후 (호출 시점 기준 상대). **time-out**. **음수 금지** |
| `next_trigger(double v, sc_time_unit tu)` | `next_trigger(sc_time(v, tu))`와 동등 |
| `next_trigger(time, event)` / `(time, or_list)` / `(time, and_list)` (+ double 변형) | time-out **또는** event/list 중 **먼저 만족되는 쪽** |

```cpp
SC_MODULE(M) {
    SC_CTOR(M) {
        SC_METHOD(entry);
        sensitive << sig;
    }
    void entry() {                                          // initialization에서 최초 실행
        if (sig == 0)      next_trigger(e1 | e2);           // 다음엔 e1 또는 e2에서
        else if (sig == 1) next_trigger(1, sc_core::SC_NS); // 1 ns 후 time-out
        else               next_trigger();                  // 다음엔 sig에서 (static)
    }
    sc_core::sc_signal<int> SC_NAMED(sig);
    sc_core::sc_event SC_NAMED(e1), SC_NAMED(e2);
};
```

### 5.2.18 `wait` — thread/clocked thread 전용

- 호출 프로세스를 suspend시키고, **바로 다음 재개 1회에 한해** dynamic sensitivity 설정 shall.
- **빈 인자 또는 정수 인자 1개**로 호출하면 **static sensitivity 사용** shall. **clocked thread에서 허용되는 유일한 형태.**
- 비정수 인자 1개 이상이면 **static sensitivity 오버라이드** shall.
- **참조 전달 인자의 수명**: 애플리케이션이 호출 시점부터 호출 완료 시점까지 실인자의 수명을 보장해야 shall. `sc_time` 파라미터의 경우 그 기간 동안 **실인자 값을 수정해서도 안 된다** shall not.
- **method process에서 호출하면 error** shall.

| 형태 | 의미 |
|---|---|
| `wait()` | static sensitivity로 재개. static sensitivity 없으면 현재 simulation 동안 재개되지 않음 |
| `wait(int n)` | `wait()`를 연속 n회 호출한 것과 동등. **n <= 0이면 error.** 구현은 clocked thread에서 이 함수를 최적화하도록 기대됨 |
| `wait(const sc_event&)` | 그 event가 notify될 때 |
| `wait(const sc_event_or_list&)` | 하나라도 notify. 중복 1회 취급. **빈 리스트 error** |
| `wait(const sc_event_and_list&)` | 전부 notify. 가장 늦은 것에서 재개. 중복 1회 취급. **빈 리스트 error** |
| `wait(const sc_time&)` | 시간 경과 후 (상대). **음수 금지** |
| `wait(double v, sc_time_unit tu)` | `wait(sc_time(v, tu))`와 동등 |
| `wait(time, event)` / `(time, or_list)` / `(time, and_list)` (+ double 변형) | 먼저 만족되는 쪽 |

### 5.2.19 Positional port binding — `sc_module::operator()`

```cpp
void operator() ( const sc_bind_proxy†& p001, ... , const sc_bind_proxy†& p064 = SC_BIND_PROXY_NIL );
```

- **포트가 생성된 순서**로 인자와 매칭 shall. 첫 생성 포트 ↔ 첫 인자.
- **인자 수가 포트 수보다 많으면 error** shall.
- **multiport는 위치 바인딩에서 단일 포트로 취급**되고 **한 번만** 바인딩 가능. 단 multiport P를 multiport Q에 위치 바인딩하면 P는 Q를 통해 간접적으로 여러 channel에 바인딩될 수 있음. **한 multiport를 위치와 이름 양쪽으로 바인딩하면 안 된다** shall not.
- **포트만 바인딩. export는 무시** shall.
- 구현은 64개 초과를 허용해도 되나 의무 아님. **한 모듈 인스턴스당 `operator()`를 두 번 이상 호출하면 안 된다** shall not.
- 실인자로 쓸 수 있는 것은 오직: (a) channel (`sc_interface` 파생 객체), (b) port (`sc_port` 파생 객체).
- 채널이 구현하는 interface 또는 포트의 타입이, 바인딩되는 포트의 타입과 **같거나 그로부터 파생**되어야 shall.
- 64개 넘으면 **named binding을 쓸 것** (should).
- **export는 실인자가 될 수 없다** (C++ 암묵 변환 2회가 필요해서). 명시적으로 `sc_export::operator IF&`를 호출하면 가능. 또는 named binding 사용.

```cpp
SC_MODULE(M2) {
    sc_core::sc_inout<int> SC_NAMED(S);
    sc_core::sc_inout<int> *T;              // 포트 포인터 — 비권장
    sc_core::sc_inout<int> SC_NAMED(U);
    SC_CTOR(M2) { T = new sc_inout<int>("T"); }
};

SC_MODULE(Top2) {
    sc_core::sc_inout<int> SC_NAMED(D), SC_NAMED(E);
    sc_core::sc_signal<int> SC_NAMED(F);
    M2 m2;
    SC_CTOR(Top2) : m2("m2") {
        m2(D, E, F);   // S-D, U-E, (*T)-F  ← 바인딩 순서는 포트 생성 순서에 의존!
    }
};
```

### 5.2.21 `get_child_objects` / `get_child_events`

- `get_child_objects`: 이 모듈 **안에** 있는 모든 `sc_object` 포인터 vector. module/port/primitive channel/unspawned·spawned process 인스턴스 + 애플리케이션 정의 `sc_object` 파생 객체 포함.
- `get_child_events`: 이 모듈이 부모인 **hierarchically named event** 전부.
- "within a module"은 **직계 자식만** — 중첩 모듈 안의 인스턴스는 포함 안 됨.
- event는 hierarchical name과 부모를 가질 수 있으나 **object hierarchy의 일부가 아님** → `get_child_objects`로 반환되지 않음.

```cpp
int sc_main (int argc, char* argv[]) {
    Top_level_module top("top");
    std::vector<sc_core::sc_object*> children = top.get_child_objects();
    for (unsigned i = 0; i < children.size(); i++)
        std::cout << children[i]->name() << " " << children[i]->kind() << std::endl;
    sc_core::sc_start();
    return 0;
}
```

### 5.2.22 `sc_gen_unique_name`

```cpp
const char* sc_gen_unique_name( const char* seed );
```

- 호출 **컨텍스트에 의존**하는 유일 문자열 반환 shall. **모듈 인스턴스마다 별도의 유일 이름 공간**, 모듈 밖 호출은 **단일 전역 공간**.
- 각 이름 공간 **안에서만** 유일성 보장 shall. 이 함수가 생성하지 않은 문자열과의 충돌은 보장 안 함.
- 접미사 형태: **밑줄 하나 + 십진 숫자 1개 이상**. 숫자 개수·선택은 구현 정의.
- seed 문자 집합 제한 없음. 빈 문자열도 가능.
- 이름은 **대소문자 구분**, 모든 문자가 유의미. `"a"`, `"A"`, `"a_"`, `"A_"`는 서로 다른 이름.

### 5.2.23 `sc_behavior` / `sc_channel`

```cpp
typedef sc_module sc_behavior;
typedef sc_module sc_channel;
```

의도 표현용 typedef. **behavior와 hierarchical channel 사이에 의도 차이 외의 구분은 없다.** 둘 다 포트와 public 멤버 함수를 가질 수 있음.

```cpp
class bus_interface : virtual public sc_core::sc_interface {
public:
    virtual void write(int addr, int data) = 0;
    virtual void read (int addr, int& data) = 0;
};

class bus_adapter : public bus_interface, public sc_core::sc_channel {
public:
    virtual void write(int addr, int data);      // channel에서 구현되는 interface method
    virtual void read (int addr, int& data);

    sc_core::sc_in<bool>  SC_NAMED(clock);
    sc_core::sc_out<bool> SC_NAMED(wr), SC_NAMED(rd);
    sc_core::sc_out<int>  SC_NAMED(addr_bus), SC_NAMED(data_out);
    sc_core::sc_in<int>   SC_NAMED(data_in);

    SC_CTOR(bus_adapter) { ... }
};
```

---

## 5.3 `sc_module_name`

모듈 문자열 이름의 컨테이너이자, elaboration 중 계층적 이름을 만드는 메커니즘.

```cpp
class sc_module_name {
public:
    sc_module_name( const char* );
    sc_module_name( const sc_module_name& );
    ~sc_module_name();
    operator const char*() const;
private:
    sc_module_name();                                     // Disabled
    sc_module_name& operator= ( const sc_module_name& );  // Disabled
};
```

### 5.3.3 Constraints on usage
- **`sc_module` 파생 클래스 생성자의 파라미터 타입으로만** 사용 shall. 모든 그런 생성자는 정확히 하나의 `sc_module_name` 파라미터를 가져야 shall (첫 번째일 필요 없음).
- 클래스 D가 C에서 직접 파생되고 C가 `sc_module` 파생일 때, **D의 `sc_module_name` 파라미터를 C의 생성자에 그대로 넘겨야** shall.
- 단 C가 `sc_module`에서 **직접** 파생된 경우, C의 생성자가 `sc_module_name`을 `sc_module` 생성자로 넘길 의무는 없다 — `sc_module()` 기본 생성자가 (명시적/암묵적으로) 호출되어도 됨.

### 5.3.4–5.3.5 계층 스택 메커니즘

구현은 `sc_module_name*` **스택**을 유지할 수 있다. `sc_module`/`sc_port`/`sc_export`/`sc_prim_channel`/`sc_event` 생성 시, 또는 프로세스 인스턴스 생성 시, **스택 top의 `sc_module_name`이 가리키는 모듈 안에 존재하는 것으로 간주**된다.

| 함수 | 스택 동작 |
|---|---|
| `sc_module_name(const char*)` | 생성 중인 객체 포인터를 **스택에 push** shall. 인자는 모듈 문자열 이름이 됨 |
| `sc_module_name(const sc_module_name&)` | 인자를 복사만. **스택 미변경** shall |
| `~sc_module_name()` | **`sc_module_name(const char*)`로 생성된 경우에만** 스택 top에서 **pop** shall |
| `operator const char*()` | **계층 이름이 아니라 문자열 이름** 반환 shall |

```cpp
struct A : sc_core::sc_module {
    A(sc_core::sc_module_name) {}          // sc_module() 호출
};
struct B : sc_core::sc_module {
    B(sc_core::sc_module_name n) : sc_core::sc_module(n) {}  // sc_module(sc_module_name&) 호출
};
struct C : B {                              // 모듈에서 파생된 모듈
    C(sc_core::sc_module_name n) : B(n) {}  // sc_module_name 복사 생성자 → B(sc_module_name)
};
struct Top : sc_core::sc_module {
    A a;  C c;
    Top(sc_core::sc_module_name n)
    : sc_core::sc_module(n)
    , a("a")                                // sc_module_name(char*) → A(sc_module_name)
    , c("c") {}
};
```

> 모든 파생 클래스 생성자에 `sc_module_name` 파라미터를 강제하는 이유(사용하지 않더라도): **그 클래스가 그 자체로 모듈로 인스턴스화될 수 있게** 보장하기 위함.

---

## 5.4 `sc_sensitive†`

```cpp
class sc_sensitive† {
public:
    sc_sensitive†& operator<< ( const sc_event& );
    sc_sensitive†& operator<< ( const sc_interface& );
    sc_sensitive†& operator<< ( const sc_port_base& );
    sc_sensitive†& operator<< ( sc_event_finder& );

    template <typename Collection>
    sc_sensitive†& operator<< ( const Collection& );
    // 나머지는 implementation-defined
};
```

**애플리케이션이 `sc_sensitive†` 객체를 명시적으로 만들면 안 된다** shall not. `sc_module`이 `sensitive`라는 이름의 데이터 멤버로 제공한다 shall.

| 인자 | static sensitivity에 추가되는 것 |
|---|---|
| `const sc_event&` | 그 event 자체 |
| `const sc_interface&` (channel) | 그 channel의 `default_event()` 반환 event. 오버라이드 안 했으면 `sc_interface::default_event` 상속 호출 |
| `const sc_port_base&` (port) | 그 포트가 바인딩된 channel의 `default_event()`. **multiport면 바인딩된 모든 channel 각각의 `default_event()`** 전부 |
| `sc_event_finder&` | event finder가 찾은 event (§5.7) |
| `const Collection&` | 컬렉션의 **모든 원소**. `std::begin/end`가 정의되고 `s << e`가 유효할 때만 오버로드 후보. 전형적으로 `sc_vector`(§8.5) 또는 포트/export/channel/event의 C 배열 |

> export는 `sc_export<IF>::operator IF&` 사용자 정의 변환 덕분에 실인자로 넘길 수 있다.
> channel에 여러 event가 있어 선택이 필요하면 `default_event` 메커니즘으로는 부족 → **event finder**가 필요.

---

## 5.5 `sc_spawn_options` / `sc_spawn`

```cpp
class sc_spawn_options {
public:
    sc_spawn_options();
    void spawn_method();
    void dont_initialize();
    void set_stack_size( int );
    void set_sensitivity( const sc_event* );
    void set_sensitivity( sc_port_base* );
    void set_sensitivity( sc_export_base* );
    void set_sensitivity( sc_interface* );
    void set_sensitivity( sc_event_finder* );
    void reset_signal_is      ( const sc_in<bool>&|sc_inout<bool>&|sc_out<bool>&|sc_signal_in_if<bool>&, bool );
    void async_reset_signal_is( 같은 4가지, bool );
private:
    sc_spawn_options( const sc_spawn_options& );                    // Disabled
    sc_spawn_options& operator= ( const sc_spawn_options& );        // Disabled
};

template <typename T>
sc_process_handle sc_spawn( T object, const char* name_p = 0, const sc_spawn_options* opt_p = 0 );

template <typename T>
sc_process_handle sc_spawn( typename T::result_type* r_p, T object,
                            const char* name_p = 0, const sc_spawn_options* opt_p = 0 );

#define sc_bind  boost::bind
#define sc_ref(r)  boost::ref(r)
#define sc_cref(r) boost::cref(r)
#define SC_FORK  implementation-defined
#define SC_JOIN  implementation-defined

namespace sc_unnamed {   // sc_bind용 인자 placeholder
    implementation-defined _1, _2, _3, _4, _5, _6, _7, _8, _9;
}
```

### 5.5.3 Constraints
`sc_spawn`은 elaboration 중, 또는 simulation 중 static/dynamic/spawned/unspawned 어느 프로세스에서든 호출 가능. `sc_spawn_options` 객체도 언제든 생성·수정 가능.

### 5.5.5 Member functions

| 함수 | 효과 | 기본값 |
|---|---|---|
| `spawn_method()` | spawn된 프로세스를 **method process**로 | 기본은 thread process |
| `dont_initialize()` | initialization phase 또는 생성 시 runnable이 되지 않게 | 기본은 설정 안 됨 → elaboration 중 spawn이면 initialization phase에서, simulation 중 spawn이면 현재/다음 evaluation phase에서 **static sensitivity와 무관하게** runnable |
| `set_stack_size(int)` | 스택 크기. **효과는 구현 정의**. **method process에 호출하면 error** | — |
| `set_sensitivity(...)` | §5.4.4의 `operator<<`처럼 static sensitivity에 추가. **export 주소면 그 export가 바인딩된 channel에 sensitive**. **multiport 주소면 바인딩된 모든 channel의 `default_event()`**. **호출은 누적**되며 오버로드를 섞어도 됨 | 기본은 빈 sensitivity |
| `reset_signal_is` / `async_reset_signal_is` | §5.2.13처럼 sync/async reset signal 추가. **호출마다 하나씩 추가**. 포트로 간접 지정 가능. 개수 제한 없음 | — |

> - thread process로 spawn하거나 initialization에서 runnable하게 만드는 멤버 함수는 **없다** — 기본값에 의존.
> - **dynamic clocked thread process는 spawn할 수 없다.**

### 5.5.6 `sc_spawn` 규칙

- **elaboration 중 호출**: spawn된 프로세스는 호출한 모듈 인스턴스의 **자식**. `sc_main`에서 호출하면 **top-level object**.
- **simulation 중 호출**: spawn된 프로세스는 **호출한 프로세스의 자식**. method/thread/clocked thread 어디서든 호출 가능.
- 호출한 프로세스 또는 모듈이 **부모** → 동적 프로세스도 계층 관계를 가지며 계층 이름에 반영됨.
- **evaluation phase에서 호출** → 현재 evaluation phase에서 runnable (`dont_initialize` 제외).
  **update phase에서 호출** → **바로 다음** evaluation phase에서 runnable.
- 인자 `T`: **함수 포인터 또는 함수 객체**(멤버 `operator()` 오버로드). **유일한 필수 인자.**
- `T::result_type*` 인자: 반환값을 받을 메모리 위치. `T`는 `result_type` 중첩 타입을 노출하는 함수 객체여야 하고 `operator()`의 반환형이 `result_type`이어야 함. **해당 메모리가 spawn된 함수 반환 시점에도 유효함을 보장할 책임은 애플리케이션에 있다** — 예컨대 감싸는 `sc_module`의 데이터 멤버는 되고, 그때쯤 해제될 **스택 변수는 안 된다**.
- `const char*` 인자: 프로세스 인스턴스 문자열 이름. 없거나 빈 문자열이면 구현이 `sc_gen_unique_name`을 seed `"thread_p"`(thread) / `"method_p"`(method)로 호출해 생성 shall.
- `sc_spawn_options*` 인자: 없으면 기본값. **`sc_spawn` 반환 후 options 객체를 유지할 의무는 없다.**
- **spawn options 인자를 주면 프로세스 이름 인자도 반드시 줘야** shall (빈 문자열이라도).
- 반환값은 **유효한 process handle** shall.
- `sc_bind`/`sc_ref`/`sc_cref`는 Boost 바인딩 편의 매크로. 사용은 의무 아님. `sc_unnamed`의 `_1.._9`는 Boost placeholder 접근용.

```cpp
int f();
struct Functor {
    typedef int result_type;
    result_type operator() ();
};
int h(int a, int& b, const int& c);

struct MyMod : sc_core::sc_module {
    sc_core::sc_signal<int> SC_NAMED(sig);
    void g();
    int ret;
    SC_CTOR(MyMod) { SC_THREAD(T); }

    void T() {
        using namespace sc_core;
        sc_spawn(&f);                              // 인자 없는 함수 spawn, 반환값 버림
        sc_process_handle handle = sc_spawn(&f);   // handle 획득

        Functor fr;
        sc_spawn(&ret, fr);                        // 함수 객체 spawn + 반환값 수신

        sc_spawn_options opt;
        opt.spawn_method();
        opt.set_sensitivity(&sig);
        opt.dont_initialize();
        sc_spawn(f, "f1", &opt);                   // sig에 sensitive한 method process "f1", 미초기화
        sc_spawn(&ret, fr, "f2", &opt);

        sc_spawn(sc_bind(&MyMod::g, this));        // 멤버 함수 spawn

        int A = 0, B, C;
        sc_spawn(&ret, sc_bind(&h, A, sc_ref(B), sc_cref(C)));
    }
};
```

### 5.5.7 `SC_FORK` / `SC_JOIN`

- **쌍으로만** 사용. thread 또는 clocked thread process **안에서만**. **method process에서 사용하면 error.**
- 사이의 각 `sc_spawn` 호출이 별도 프로세스를 생성 shall. 자식들은 **지연 없이** spawn되고 (spawn options에 따라) 모두 현재 evaluation phase에서 runnable이 될 수 있음.
- spawn되는 것은 **thread process** shall. **fork-join 안에서 method process를 spawn하면 error.**
- 모든 spawn된 프로세스가 **terminate되면** 제어가 fork-join을 벗어난다.
- 사이의 텍스트는 **콤마로 구분된 `sc_spawn` 호출들** shall. 반환값을 버리거나, **대입문 우변의 유일한 표현식**으로 쓸 수 있음.
- 마지막 `sc_spawn` 뒤 `SC_JOIN` 직전의 콤마는 **선택**. 그 외 **공백 이외의 문자를 넣으면 효과는 undefined.**

```cpp
{
    using namespace sc_core;
    sc_process_handle h1, h2, h3;
    SC_FORK
        h1 = sc_spawn( arguments ) ,
        h2 = sc_spawn( arguments ) ,
        h3 = sc_spawn( arguments )
    SC_JOIN
}
```

---

## 5.6 `sc_process_handle`

### 5.6.1 valid / invalid / empty

| 상태 | 정의 |
|---|---|
| **valid** | 하나의 프로세스 인스턴스와 연관 (terminated 여부 무관) |
| **invalid** | **empty**이거나, **terminated된** 프로세스 인스턴스와 연관 |
| **empty** | 어떤 프로세스 인스턴스와도 연관되지 않은 invalid handle |

한 프로세스 인스턴스는 0개·1개·여러 개 handle과 연관될 수 있고, 그 수와 정체는 시간에 따라 변할 수 있다.

> 동적 프로세스는 simulation 중 생성·소멸되므로 **raw 포인터로 조작하는 것은 일반적으로 안전하지 않다.** `sc_process_handle`이 안전하고 균일한 메커니즘.
> `operator<`가 strict weak ordering을 제공하므로 `std::map` 등 표준 컨테이너에 저장 가능.

### 5.6.2 Class definition

```cpp
enum sc_curr_proc_kind { SC_NO_PROC_, SC_METHOD_PROC_, SC_THREAD_PROC_, SC_CTHREAD_PROC_ };
enum sc_descendant_inclusion_info { SC_NO_DESCENDANTS, SC_INCLUDE_DESCENDANTS };

class sc_unwind_exception : public std::exception {
public:
    virtual const char* what() const throw();
    virtual bool is_reset() const;
protected:
    sc_unwind_exception();
    sc_unwind_exception( const sc_unwind_exception& );
    virtual ~sc_unwind_exception() throw();
};

class sc_process_handle {
public:
    sc_process_handle();
    sc_process_handle( const sc_process_handle& );
    explicit sc_process_handle( sc_object* );
    ~sc_process_handle();

    bool valid() const;
    sc_process_handle& operator= ( const sc_process_handle& );
    bool operator== ( const sc_process_handle& ) const;
    bool operator!= ( const sc_process_handle& ) const;
    bool operator<  ( const sc_process_handle& ) const;
    void swap( sc_process_handle& );

    const char* name() const;
    const char* basename() const;
    sc_curr_proc_kind proc_kind() const;
    const std::vector<sc_object*>& get_child_objects() const;
    const std::vector<sc_event*>&  get_child_events() const;
    sc_object* get_parent_object() const;
    sc_object* get_process_object() const;
    bool dynamic() const;
    bool terminated() const;
    const sc_event& terminated_event() const;

    void suspend( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
    void resume ( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
    void disable( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
    void enable ( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
    void kill   ( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
    void reset  ( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
    bool is_unwinding() const;
    const sc_event& reset_event() const;
    void sync_reset_on ( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
    void sync_reset_off( sc_descendant_inclusion_info = SC_NO_DESCENDANTS );

    template <typename T>
    void throw_it( const T& user_defined_exception, sc_descendant_inclusion_info = SC_NO_DESCENDANTS );
};

sc_process_handle sc_get_current_process_handle();
bool sc_is_unwinding();
```

**Constraints on usage: 없음.** handle은 elaboration/simulation 어느 때나 생성·복사·삭제 가능.

### 5.6.5 주요 멤버 함수

| 함수 | 규칙 |
|---|---|
| `valid()` | valid일 때만 true |
| `operator==` | **둘 다 valid이고 같은 프로세스 인스턴스**를 공유할 때만 true |
| `operator<` | strict weak ordering. `H1 < H1`은 false. 같은 인스턴스거나 둘 다 empty면 양방향 false. 다르거나 한쪽만 empty면 정확히 한 방향만 true. 추이적 |
| `swap()` | 두 handle의 인스턴스를 교환. 이전 `H1<H2`였다면 이후 `H2<H1` |
| `name()` / `basename()` | 계층 이름 / 문자열 이름. **invalid면 빈 문자열 `""`**. 문자열은 handle이 valid한 동안만 유효 보장 |
| `proc_kind()` | valid면 `SC_METHOD_PROC_`/`SC_THREAD_PROC_`/`SC_CTHREAD_PROC_`, invalid면 `SC_NO_PROC_` |
| `get_child_objects()` | 이 프로세스의 **직계 자식** `sc_object` 전부 (실행 중 spawn되어 아직 삭제 안 된 동적 프로세스 포함). **손자는 제외**. invalid면 빈 vector |
| `get_child_events()` | 이 프로세스가 부모인 hierarchically named event 전부. invalid면 빈 vector |
| `get_parent_object()` | spawn된 출처 모듈/프로세스. invalid면 null. **부모가 terminated된 프로세스여도 그 포인터 반환.** 프로세스 인스턴스는 살아있는 자식이 있는 동안 삭제되지 않는다 |
| `get_process_object()` | valid면 프로세스 인스턴스 포인터, invalid면 null. **역참조 전 null 검사 권장.** 포인터는 **호출 프로세스가 suspend할 때까지만** 유효하다고 가정할 것 |
| `dynamic()` | dynamic process면 true, static이면 false. invalid면 **false** |
| `terminated()` | terminated면 true. thread/cthread는 연관 함수에서 제어가 반환된 후. `kill`로도 종료 가능 — **method process를 종료시키는 유일한 방법**. **empty handle이면 false** |
| `terminated_event()` | 인스턴스 종료 시 notify되는 event 참조. **invalid handle에 호출하면 error** |

**handle 무효화 규칙** (`terminated()` 설명 중):
- 인스턴스가 terminate되고 **살아있는 자식이 없으면** 구현은 연관 handle을 무효화**해도 되지만 의무는 아니다.**
- **자식 객체가 있는 동안에는 무효화하면 안 된다** shall not.
- 무효화한다면 **그 시점의 모든 연관 handle을 함께** 무효화해야 shall — 같은 인스턴스에 valid와 invalid handle이 공존할 수 없다.
- terminate 후에는 handle이 invalid가 되어도 `terminated()`는 계속 true를 반환한다.
- **handle이 valid한 동안 프로세스 인스턴스는 계속 존재한다** shall.
- `operator<`와 `swap`의 순서 관계에서는, 인스턴스가 삭제된 후에도 invalid handle이 그 인스턴스와 연관된 것처럼 취급된다.

```cpp
{
    using namespace sc_core;
    sc_process_handle a, b;                 // empty 2개
    sc_assert( !a.valid() && !b.valid() );
    sc_assert( a != b );                    // empty끼리도 == 는 false
    sc_assert( !(a < b) && !(b < a) );

    a = sc_spawn(...);  b = sc_spawn(...);
    sc_assert( a != b );
    sc_assert( (a < b) || (b < a) );

    sc_process_handle c = b;
    sc_assert( b == c );
    sc_assert( !(b < c) && !(c < b) );

    wait( a.terminated_event() & b.terminated_event() );
    sc_assert( (a < b) || (b < a) );        // valid 여부와 무관하게 순서 동일
    sc_assert( b.valid() == c.valid() );    // 무효화는 일관적
    sc_assert( !(b < c) && !(c < b) );
    sc_assert( c.terminated() );
}
```

### 5.6.6 Process control

**공통 규칙:**
- handle이 **invalid**면 구현은 **warning을 내고 아무 효과 없이 즉시 반환** shall.
- `include_descendants == SC_INCLUDE_DESCENDANTS`면 **bottom-up 순서**로 (가장 깊은 자손부터, 마지막에 자기 자신) 재귀 적용 shall. `SC_NO_DESCENDANTS`면 자기 자신만. suspend-resume, disable-enable, sync_reset_on-off 쌍에서 이 인자를 다르게 써도 제약 없음 (자손 포함 suspend 후 자기만 resume 가능).
- **process control 멤버 함수는 update phase에서 호출하면 안 된다** shall not.
- "during elaboration or before the process has first executed"가 뜻하는 범위: elaboration 전체 / `before_end_of_elaboration`·`end_of_elaboration`·`start_of_simulation` 콜백 / simulation 중이지만 해당 프로세스가 아직 처음 실행되기 전.

#### suspend vs disable — 핵심 차이

| | suspend/resume | disable/enable |
|---|---|---|
| 중지 중 sensitivity | **커널이 추적함** | **무효화됨 (완전히 무시)** |
| 중지 중 발생한 event | `resume` 호출 시 **즉시 runnable** | `enable` 후 **새로 발생하는** event에만 반응 |
| runnable 집합에 있었다면 | **즉시 제거**, 현재 evaluation phase에서 실행 안 됨 | **제거하지 않음** — 현재 evaluation phase에서 실행 허용 |

→ disable된 프로세스는 **disable 시점에 이미 runnable이었을 때에 한해 딱 한 번** 실행될 수 있다.

```cpp
// 시간축 예시 (LRM 5.6.6.1)
// ticker: 10ns마다 ev.notify()
void calling() {
    wait(15.0, SC_NS);   // target은 10ns에 실행됨
    t.suspend();
    wait(10.0, SC_NS);   // 20ns에 target 실행 안 됨 (suspended)
    t.resume();          // 25ns에 target 실행됨 (중지 중 notify 기억)
    wait(10.0, SC_NS);   // 30ns에 notification으로 실행
    t.disable();
    wait(10.0, SC_NS);   // 40ns에 실행 안 됨 (disabled)
    t.enable();          // 45ns에 실행 안 됨 (enable로는 runnable 안 됨)
    wait(10.0, SC_NS);   // 50ns에 notification으로 실행
    sc_stop();
}
```

#### 5.6.6.2 `suspend` / `resume`

- `suspend`: 대상이 `resume`될 때까지 runnable이 될 수 없게 shall. runnable 집합에 있으면 **즉시 제거**되어 현재 evaluation phase에서 실행 안 됨. suspend 시점 sensitivity에 해당하는 event/time-out이 중지 중 발생하고 이후 `resume`이 호출되면, **`resume`이 호출된 evaluation phase에서 runnable**이 된다 shall. (구현은 사실상 `operator&`로 암묵 event를 sensitivity에 추가하고 `resume` 시 immediate notification을 만든다.)
- 이미 suspend된 것에 `suspend`는 효과 없음 (자손에는 영향 가능). **`resume` 한 번이면 해제 충분.**
- **method process가 자기를 suspend**하면 연관 함수는 **끝까지 실행 후** 커널에 반환. 반환 전에 자기가 `resume`을 호출하면 suspend가 없었던 것처럼 효과 제거.
- **thread process가 자기를 suspend**하면 `suspend`에서 제어가 돌아오지 않고 **즉시 suspend**.
- suspend되지 않은 대상이 **synchronous reset state**에 있을 때 `suspend` 호출 → **구현 정의** (§5.6.6.12).
- terminated 인스턴스에는 효과 없음 (자손에는 영향 가능).
- elaboration 중/첫 실행 전 호출 허용. `dont_initialize`가 유효하면 위의 암묵 event 방식, 아니면 `resume` 시 **즉시 runnable** (초기화를 resume 시점까지 지연).

- `resume`: 이전 `suspend` 효과를 제거하고, **중지 중 sensitivity가 runnable로 만들었을 경우에만** 현재 evaluation phase에서 runnable로 shall. suspend 시점에 이미 runnable이었으면 `resume`이 현재 evaluation phase에서 runnable로 만든다 shall. 아무 일도 없었으면 runnable로 만들지 않는다 shall.
- **`suspend` 시점의 sensitivity를 복원** shall.
- `resume` 전에 `reset`이 있었으면, **reset 이전에 발생한 event/time-out은 무시** shall.
- thread는 `suspend` 다음 실행문부터 재개, method는 연관 함수 호출로 재개.
- suspend되지 않은 것에 `resume`은 효과 없음 → **같은 evaluation phase에서 여러 번 `resume`을 호출하면 대상이 한 번만 실행될 수도, 여러 번 실행될 수도 있다** (실행 순서에 따라).
- suspended이면서 disabled인 대상에 `resume` → **구현 정의**.
- **suspend와 resume을 같은 프로세스가 호출할 의무는 없다.**

#### 5.6.6.3 `disable` / `enable`

- `disable`: `enable`될 때까지 runnable이 될 수 없게 shall. **runnable 집합에서 제거하지 않고** 현재 evaluation phase 실행을 허용 shall. disable 중 sensitive한 event/time-out은 **무시**되어 runnable 집합에 들어가지 않는다 shall.
- 이미 disable된 것에 `disable`은 효과 없음. **`enable` 한 번이면 해제 충분.**
- **자기를 disable**하면 (method든 thread든) `wait`까지 또는 끝까지 계속 실행. 반환 전에 자기가 `enable`하면 효과 제거.
- disable되지 않은 대상이 **time-out을 기다리는 중**(event/list 동반 여부 무관)이면 `disable` 동작은 **구현 정의**.
- terminated에는 효과 없음.
- elaboration 중/첫 실행 전 호출 허용 — 이미 runnable이면 (initialization 중 등) 실행이 허용된다.

- `enable`: `disable` 효과 제거. **`resume`과 달리, disable 중 발생한 event로는 runnable이 되지 않는다** shall not.
- disable 중 time-out이 발생했고 그 프로세스가 time-out 외 다른 event에 sensitive하지 않으면, 그 프로세스는 **(reset되지 않는 한) 다시 실행되지 않으며** 구현이 warning을 낼 수 있다.
- 재개 시 thread는 disable된 `wait` 다음 문장부터, method는 함수 호출로.
- **elaboration 중 `disable`하고 initialization phase 이후에 `enable`하면**, 대상은 initialization phase에서도 runnable이 되지 않고 `enable` 호출 시점에도 runnable이 되지 않는다 shall.

#### 5.6.6.4 Reset 메커니즘 정리

프로세스가 **reset되는 3가지 방법**:
1. `reset` 호출 → **즉시** reset
2. `async_reset_signal_is`로 지정한 signal이 reset 값 도달 → **즉시** reset
3. **synchronous reset state**에 있는 동안 재개됨

**synchronous reset state 진입 3가지**:
1. `sync_reset_on` 호출
2. `reset_signal_is` 지정 signal이 active 값 도달
3. `async_reset_signal_is` 지정 signal이 active 값 도달

**벗어나는 조건 (전부 만족해야)**:
- `sync_reset_off`가 호출되었거나 `sync_reset_on`이 아직 호출된 적 없음
- `reset_signal_is`로 지정한 **모든** signal이 active의 부정 값
- `async_reset_signal_is`로 지정한 **모든** signal이 active의 부정 값

어느 경우든 결과 reset은 `reset` 호출과 동등하며 `reset_event()`가 notify된다 shall.

#### 5.6.6.5 `kill`

- `sc_unwind_exception`이 던져지고, 대상은 **현재 simulation 동안 다시 runnable이 되지 않는다** shall. **terminated된다.**
- `sc_unwind_exception::is_reset()`은 **false** shall.
- **즉시 효과** shall — 반환 전에: runnable 집합에서 제거 → call stack unwind → terminated 상태 → terminated event notify. **부작용이 있을 수 있다.**
- 다른 프로세스를 kill하면 제어가 `kill` 호출자에게 반환. **자기를 kill하면 `kill` 이후 문장은 현재 simulation 동안 실행되지 않고** 제어가 커널로 간다.
- terminated에는 효과 없음.
- 첫 실행 전 호출 허용 → 그 프로세스는 현재 simulation 동안 실행되지 않는다.
- **elaboration 중, initialization phase 전, simulation이 paused/stopped인 동안 호출하면 error** shall.

#### 5.6.6.6 `reset`

- runnable 집합에서 제거 → `sc_unwind_exception` 던짐 → **dynamic sensitivity 제거, static sensitivity 복원** → **terminate되지 않음, handle은 valid 유지** → **연관 함수를 처음부터 다시 호출** shall. 그 결과 `wait` 호출, self-suspend, 또는 return까지 실행된다.
- **즉시 효과** shall — 반환 전에 제거·unwind·reset event notify 완료. **부작용 가능.** 서로 reset하는 프로세스 간 **상호 재귀에 주의** (스택 오버플로).
- `sc_unwind_exception::is_reset()`은 **true** shall.
- `sc_unwind_exception`은 **잡을 수 있으나 즉시 다시 던져야** shall.
- **method process도 reset 가능** — unwind할 call stack은 (자기 자신을 reset하는 경우를 제외하면) 없다. dynamic sensitivity 제거, static 복원, 함수 재호출.
- terminated에는 효과 없음.
- **첫 실행 전 호출은 허용되나 `reset_event()` notify 외에 아무 효과 없다.**
- **elaboration 중, initialization phase 전, paused/stopped 중 호출은 error** shall.

#### 5.6.6.7 `sc_unwind_exception`

`kill`과 `reset`이 던지는 예외 타입.

- `wait`로 suspend된 thread, 또는 자기를 kill/reset하는 thread·method의 경우 **call stack이 unwind되고 지역 객체의 소멸자가 호출된다** shall.
- unwind 중 부작용(소멸자 호출 등)은 **`wait`/`next_trigger` 호출을 포함해서는 안 된다** shall not. 단 **다른 프로세스(자기 자신 포함)에 대한 process control 호출은 포함될 수 있다** → `kill`/`reset`/`throw_it` 호출은 **중첩 가능**.
- report handler의 기본 동작에 `SC_THROW`가 있지만, 구현은 report handler가 다른 예외를 던지기 전에 `sc_unwind_exception`을 **먼저 잡을 의무가 있다.**
- 대상 프로세스는 함수 본문 안에서 잡을 수 있으나 **반드시 다시 던져야** shall. **다시 던지지 않고 잡으면 error** shall.
- catch 블록은 **대상 프로세스의 컨텍스트**에서 실행된다 → 그 안의 `sc_get_current_process_handle()`은 **대상** 프로세스 handle을 반환 shall.
- 생성자·소멸자는 protected — **커널만 던진다** shall.

```cpp
SC_MODULE(m) {
    SC_CTOR(m) { SC_THREAD(run); }
    void run() {
        try {
            ...
        } catch (const sc_core::sc_unwind_exception& ex) {
            // 정리 작업
            if (ex.is_reset()) { ... } else { ... }
            throw ex;                      // 반드시 재던지기
        }
    }
};
```

#### 5.6.6.8 `is_unwinding` / 5.6.8 `sc_is_unwinding`

- 커널이 `sc_unwind_exception`을 던진 시점부터 그것을 잡는 시점까지 **true** shall. → **`kill`/`reset`에 의한 unwind 중에는 true, `throw_it`에는 false.**
- 의도: 프로세스 지역 객체의 **소멸자에서** "프로세스가 kill/reset된 것인지, simulation 종료인지" 구분.
- invalid handle이면 warning + **false** 반환.
- `sc_is_unwinding()`은 현재 실행 중 프로세스에 대한 비멤버 버전.

```cpp
struct wait_on_exit {
    ~wait_on_exit() {
        if (!sc_core::sc_is_unwinding())        // 프로세스가 kill될 수 있으므로 필요
            wait(10.0, sc_core::SC_NS);         // ... 그 경우 이 wait는 불법
    }
};

void some_module::some_process() {
    while (true) {
        try {
            wait_on_exit w;                     // 지역 객체, catch 전에 소멸
            ...
        } catch (const sc_core::sc_unwind_exception&) {
            ...                                 // 기타 정리
            throw;
        }
    }
}
```

#### 5.6.6.9 `reset_event`

대상이 reset될 때마다 notify되는 event 참조 반환 shall — 명시적 `reset` 호출이든, synchronous reset state에서의 재개든, reset signal의 active 도달이든. **reset이 발생한 evaluation phase에서 immediate notification으로** 스케줄된다 shall. **invalid handle에 호출하면 error** shall.

#### 5.6.6.10 `sync_reset_on` / `sync_reset_off`

- `sync_reset_on`: 대상을 synchronous reset state로 진입시킴 shall. 이 상태에서는 **재개될 때마다** (event든 time-out이든) reset된다 shall. **호출 자체가 즉시 reset을 일으키지는 않는다** — 이후 재개될 때마다 reset.
- `sync_reset_off`: synchronous reset state에서 벗어나게 shall — 단 `reset_signal_is`/`async_reset_signal_is`로 지정한 signal이 active면 예외. **reset signal의 효과나 명시적 `reset` 호출의 효과는 수정하지 않는다** shall not.
- 따라서 `sync_reset_on` 후 재개 전에 `sync_reset_off`를 호출하면 그 호출로 인한 reset은 일어나지 않는다 (reset signal이나 `reset` 호출로 인한 것은 별개).
- `SC_INCLUDE_DESCENDANTS`로 method/thread가 섞여 있어도 각각에 맞는 동작이 취해진다 shall.
- 이미 그 상태면 효과 없음. **`sync_reset_off` 한 번이면 해제 충분** (reset signal이 active가 아니라면).
- 자기 자신을 대상으로 호출 가능 — **효과는 다음 재개 때 보인다.**
- synchronous reset state가 아니고 **suspended**인 대상에 `sync_reset_on` → **구현 정의**.
- terminated에는 효과 없음.
- elaboration 중/첫 실행 전 호출 허용. **synchronous reset state는 초기화에 영향 없다** — 첫 evaluation phase에서 재개되어 return 또는 `wait`까지 실행된다. `wait`에서 처음 재개될 때 여전히 그 상태면 그때 reset된다.

#### 5.6.6.11 `throw_it`

- 대상 프로세스의 **컨텍스트에서** 예외를 던진다 shall (호출자 컨텍스트 아님). 자기 자신에게 던지는 경우를 빼면 **두 번의 컨텍스트 전환** 필요.
- 예외는 `std::exception` 파생을 **권장**하나 강제는 아님.
- **dynamic sensitivity 제거, static sensitivity 복원** shall. **`throw_it`만으로는 terminate되지 않는다** — 단 예외를 잡은 뒤 연관 함수에서 제어가 반환되면 thread는 즉시 terminate될 수 있음.
- **즉시 효과** shall — 반환 전에 제어 이동·예외 던지기·잡기가 완료. **부작용 가능.**
- **대상은 연관 함수에서 예외를 반드시 잡아야** shall — **잡지 않으면 error.** 잡은 뒤 return하거나 `wait` 호출 가능. **첫 예외 처리 중 다른 예외를 던지면 error** shall.
- 다른 프로세스에 던지면 제어가 `throw_it` 호출자로 반환. 자기에게 던지면 `throw_it`은 반환하지 않고 자기 catch 블록으로 제어 이동.
- 예외 처리의 부작용은 **`wait`/`next_trigger`를 포함해서는 안 된다** shall not. process control 호출은 가능 → **중첩 가능.**
- **적용 대상은 terminate되지 않은 thread process뿐.** method process나 terminated 프로세스가 대상이면 **허용되지만 효과 없음** (구현이 warning 가능). `SC_INCLUDE_DESCENDANTS`로 섞여 있어도 각각 적절히 처리.
- **대상이 실행 중 suspend된 상태(즉 `wait` 호출했거나 `suspend`/`disable`의 대상이 된 상태)에서만 적용 가능.** 그 외 문맥에서는 효과 없음(warning 가능). 특히 **simulation 중 대상이 첫 실행되기 전에 호출하면 warning만 나고 효과 없음.**
- **elaboration 중, initialization phase 전, paused/stopped 중 호출은 error** shall.

#### 5.6.6.12 상호작용 규칙 (가장 중요)

**Table 2 — 효과 발생 시점**

| 멤버 함수 | 효과 시점 |
|---|---|
| `suspend` | **현재** evaluation phase |
| `resume` | **현재** evaluation phase |
| `disable` | **다음** evaluation phase |
| `enable` | **현재** evaluation phase |
| `kill` | **즉시** |
| `reset` | **즉시** |
| `throw_it` | **즉시** |
| `sync_reset_on` | **현재** evaluation phase |
| `sync_reset_off` | **현재** evaluation phase |

**우선순위 (높음 → 낮음)**:
1. `kill`, `reset`, `throw_it`
2. `disable`, `enable`
3. `suspend`, `resume`
4. `sync_reset_on`, `sync_reset_off`

구현은 각 프로세스마다 **독립된 3개 플래그**를 유지한다 shall — disable/enable, suspend/resume, sync_reset_on/off. 각 플래그는 **나머지 두 플래그 값과 무관하게** 해당 쌍의 호출로 설정·해제된다 shall. 예: `suspend` 없이 `resume`을 연속 두 번 호출하면 중간에 `disable`/`enable`이 있었든 없었든 두 번째 `resume`은 효과 없음.

`kill`/`reset`/`throw_it`은 **대상이 disabled/suspended/synchronous reset state여도 즉시 실행**되며, 즉시 동작 완료 후에도 **그 상태는 유지된다** shall. 예: disabled인 대상에 `throw_it` → 깨어나 예외를 잡고 catch 안에서 `wait`를 호출하고 커널에 제어를 반환하는 동안 **계속 disabled 상태**.

**구현 정의인 상호작용 4가지:**

| # | 조건 |
|---|---|
| a) | **suspended이면서 disabled**인 대상에 `resume` |
| b) | **suspended이면서 synchronous reset state가 아닌** 대상에 `sync_reset_on` 호출 또는 sync/async reset signal의 active 도달 |
| c) | **synchronous reset state이면서 suspended가 아닌** 대상에 `suspend` |
| d) | **disabled가 아니면서 time-out을 기다리는**(event/list 동반 여부 무관) 대상에 `disable` |

**reset이 suspend 중인 대상에 호출되면**: dynamic sensitivity를 제거하여 **reset 이전에 발생한 event/time-out은 이후 `resume` 동작 판정에서 무시**된다 shall. reset이 "슬레이트를 지운다".

**즉시 의미론 3함수(`kill`/`reset`/`throw_it`)를 elaboration 중, initialization phase 전, paused/stopped 중 호출하면 error** shall. 나머지 함수는 그 상태에서도 호출 가능.

```cpp
sc_core::sc_process_handle t;
t.sync_reset_on();  // synchronous reset state 진입
A();                // 블로킹 호출
t.suspend();        // suspend
B();
t.disable();        // disable
C();
t.reset();          // 즉시 reset. 여전히 disabled. static sensitivity 복원
D();
t.enable();         // enable. 여전히 suspended
E();
t.disable();
F();
t.enable();         // enable. 여전히 suspended
G();
t.resume();         // resume. 여전히 synchronous reset state
                    // E 또는 G가 notify한 event에 sensitive하면 runnable
                    // A, B, C, D, F가 notify한 것에는 반응 안 함 (reset이 슬레이트를 지웠으므로)
```

### 5.6.7 `sc_get_current_process_handle`

| 호출 문맥 | 반환값 |
|---|---|
| elaboration 중 모듈 생성자 본문 (또는 거기서 호출된 함수) | 그 모듈 안에서 **가장 최근 생성된** 프로세스 인스턴스 handle (있으면) |
| `before_end_of_elaboration` / `end_of_elaboration` 콜백 | **그 특정 콜백 함수 안에서** 가장 최근 생성된 프로세스 handle (있으면) |
| 위 두 경우에서 가장 최근 프로세스가 현재 모듈/콜백 소속이 아니면 | 구현이 그 handle을 주거나 invalid handle을 줄 수 있음 |
| elaboration 중 `sc_main` 또는 `start_of_simulation` 콜백 | 가장 최근 생성 프로세스 handle **또는** invalid handle (구현 선택) |
| simulation 중 | **현재 실행 중인** 프로세스 handle. 없으면 invalid |
| simulation 중 `sc_main` | **invalid handle** shall |

---

## 5.7 `sc_event_finder` / `sc_event_finder_t`

**event finder** = 반환형이 `sc_event_finder&`인 **포트 클래스의 멤버 함수**. 여러 event를 가진 channel에 포트가 바인딩될 때, 포트를 통해 특정 event를 골라 static sensitivity에 추가하게 해준다.

port binding이 지연될 수 있으므로 프로세스 생성 시점에 event를 못 가져올 수 있다 → event finder를 쓰면 **구현이 port binding 완료 후로 sensitivity 추가를 지연**시킨다 shall. 이 지연 동작은 **`end_of_elaboration` 콜백 전에 완료**되어야 shall.

**multiport에 event finder를 호출하면 바인딩된 모든 channel의 event가 sensitivity에 추가된다** shall.

```cpp
class sc_event_finder implementation-defined;

template <class IF>
class sc_event_finder_t : public sc_event_finder {
public:
    sc_event_finder_t( const sc_port_base& port_, const sc_event& (IF::*event_method_)() const );
    // 나머지 implementation-defined
};
```

### 5.7.3 Constraints on usage

- 애플리케이션은 `sc_event_finder`를 **포트 클래스 멤버 함수의 반환형(참조)** 으로만, 또는 애플리케이션 정의 event finder 클래스 템플릿의 base class로만 사용 shall.
- `sc_event_finder_t<interface>`는 **event finder가 반환하는 객체를 생성할 때만** 사용 shall.
- event finder의 반환형은 `sc_event_finder&` shall이며 `sc_event_finder_t<interface>` 또는 애플리케이션 정의 event finder 템플릿 객체를 반환 shall. 여기서
  a) *interface*는 그 포트가 바인딩될 수 있는 interface 이름 shall
  b) 생성자 첫 인자는 **포트 객체 자신** shall
  c) 둘째 인자는 그 interface의 **멤버 함수 주소** shall — 그 함수가 반환하는 event가 찾아지는 event
- event finder 멤버 함수는 **`operator<<`, `set_sensitivity`, `SC_CTHREAD`로 static sensitivity를 만들 때만** 호출 가능.
- **elaboration 중, 생성자 또는 `before_end_of_elaboration`에서만** 호출 shall only. **`end_of_elaboration` 콜백이나 simulation 중 호출 금지** shall not — 대신 event에 직접 sensitive하게 만들 것.
- **multiport에서는 인덱스로 개별 channel의 event를 찾을 수 없다.** 우회: port binding 완료 후 `end_of_elaboration` 콜백에서 개별 channel의 event를 가져온다.

```cpp
#include <systemc>

class if_class : virtual public sc_core::sc_interface {
public:
    virtual const sc_core::sc_event& ev_func() const = 0;
};

class chan_class : public if_class, public sc_core::sc_prim_channel {
public:
    virtual const sc_core::sc_event& ev_func() const { return an_event; }
private:
    sc_core::sc_event an_event;
};

template<int N = 1>
class port_class : public sc_core::sc_port<if_class, N> {
public:
    sc_core::sc_event_finder& event_finder() const {
        return *new sc_core::sc_event_finder_t<if_class>( *this, &if_class::ev_func );
    }
};

SC_MODULE(mod_class) {
    port_class<1> port_var;
    port_class<0> multiport;

    SC_CTOR(mod_class) {
        SC_METHOD(method);
        sensitive << port_var.event_finder();          // chan_class::an_event에 sensitive
    }
    void method();

    void end_of_elaboration() {                        // multiport 우회
        SC_METHOD(method2);
        for (int i = 0; i < multiport.size(); i++)
            sensitive << multiport[i]->ev_func();
    }
    void method2();
};
```

> 실제 event finder 예: `sc_in<bool>::pos()`, `sc_in<bool>::neg()` (§6.9).

---

## 5.8 / 5.9 Event lists and expressions

### 5.8 `sc_event_and_list` / `sc_event_or_list` — 명시적 event list 객체

```cpp
class sc_event_and_list {
public:
    sc_event_and_list();
    sc_event_and_list( const sc_event_and_list& );
    sc_event_and_list( const sc_event& );
    sc_event_and_list& operator= ( const sc_event_and_list& );
    ~sc_event_and_list();

    int size() const;
    void swap( sc_event_and_list& );

    sc_event_and_list& operator&= ( const sc_event& );
    sc_event_and_list& operator&= ( const sc_event_and_list& );
    sc_event_and_expr† operator&  ( const sc_event& ) const;
    sc_event_and_expr† operator&  ( const sc_event_and_list& ) const;
};
// sc_event_or_list 는 & → |, and → or 로 동일 구조
```

- event list는 **0개 이상의 event 포인터/참조를 저장**하며 같은 event를 여러 번 담을 수 있다. **순서는 dynamic sensitivity 동작에 영향 없다** shall.
- **수명 관리 책임은 애플리케이션에 있다** (event *expression*과 달리). list의 event notify로 프로세스가 재개·트리거되는 시점에 list 객체가 **아직 유효함을 보장해야** shall. 이미 파괴되었으면 동작은 **undefined** shall.
- 기본 생성자는 **빈 list** 생성. **빈 list를 `wait`/`next_trigger`에 넘기면 error** shall.
- `size()`는 event 개수를 반환하되 **중복은 세지 않는다** shall.
- `&=`/`|=`는 `*this`를 수정하고 자기 참조 반환. `&`/`|`는 `*this`를 **수정하지 않고** 늘어난 expression 반환.

```cpp
{
    sc_event ev;
    sc_event_or_list list = ev | ev;
    sc_assert( list.size() == 1 );      // 중복은 1개로
}
```

```cpp
struct M : sc_core::sc_module {
    sc_core::sc_port<sc_core::sc_signal_in_if<int>, 0> p;   // multiport

    M(sc_core::sc_module_name) { SC_THREAD(T); }

    sc_core::sc_event_or_list all_events() const {
        sc_core::sc_event_or_list or_list;
        for (int i = 0; i < p.size(); i++)
            or_list |= p[i]->default_event();
        return or_list;
    }

    sc_core::sc_event event1, event2;

    void T() {
        for (;;) {
            wait(all_events());

            sc_core::sc_event_and_list list;
            sc_assert(list.size() == 0);
            list = list & event1;   sc_assert(list.size() == 1);
            list &= event2;         sc_assert(list.size() == 2);
            wait(list);
            sc_assert(list.size() == 2);
        }
    }
};
```

### 5.9 `sc_event_and_expr†` / `sc_event_or_expr†` — 임시 expression

```cpp
class sc_event_and_expr† {
public:
    operator const sc_event_and_list &() const;
    // 나머지 implementation-defined
};
sc_event_and_expr† operator& ( sc_event_and_expr†, sc_event const& );
sc_event_and_expr† operator& ( sc_event_and_expr†, sc_event_and_list const& );
// or 버전 동일
```

- **애플리케이션이 명시적으로 생성하면 안 된다** shall not. 명시적 list가 필요하면 `sc_event_and_list`/`sc_event_or_list`를 쓸 것 (should).
- `sc_event`, `sc_event_and_list`, `sc_event_or_list`의 `operator&`/`operator|` **반환형**이다.
- 타입 변환 연산자 덕분에 `wait(ev1 & ev2)`처럼 바로 넘길 수 있다.
- **수명 관리는 구현 책임** — 프로세스가 그 expression 안 event의 notify로 재개·트리거되면 구현이 list 객체를 삭제한다 shall.

---

## 5.10 `sc_event`

프로세스 동기화용 객체. **event는 `sc_event` 객체로만 표현되며 이 클래스 외에는 없다.**

### 5.10.1 계층적 이름

| 종류 | 특징 |
|---|---|
| **hierarchically named event** | object hierarchy에 부모가 있거나(→ 부모의 `get_child_events`로 반환됨) top-level event(→ `sc_get_top_level_events`로 반환됨). 부모는 module instance 또는 process instance |
| **이름 없는 event** | 구현 정의 이름을 가짐. **부모 없음** shall not |
| **kernel event** | 구현이 만든 event (predefined primitive channel 내부 등). **hierarchically named가 아니어야** shall하고 구현 정의 이름 shall — elaboration/simulation 어느 때 생성되든 |

**kernel event를 제외하고, elaboration 중 또는 initialization phase 전에 생성된 모든 event는 hierarchically named여야** shall.

> event는 부모가 `sc_object`일 수 있지만 **event 자체는 `sc_object`가 아니다** → `get_child_objects`로 반환되지 않는다. `get_child_events`를 쓸 것.

### 5.10.2 Class definition

```cpp
class sc_event {
public:
    sc_event();
    explicit sc_event( const char* );
    ~sc_event();

    const char* name() const;
    const char* basename() const;
    bool in_hierarchy() const;
    sc_object* get_parent_object() const;
    bool triggered() const;

    void notify();
    void notify( const sc_time& );
    void notify( double, sc_time_unit );
    void cancel();

    sc_event_and_expr† operator& ( const sc_event& ) const;
    sc_event_and_expr† operator& ( const sc_event_and_list& ) const;
    sc_event_or_expr†  operator| ( const sc_event& ) const;
    sc_event_or_expr†  operator| ( const sc_event_or_list& ) const;

    static const sc_event none;
private:
    sc_event( const sc_event& );                // Disabled
    sc_event& operator= ( const sc_event& );    // Disabled
};

const std::vector<sc_event*>& sc_get_top_level_events();
sc_event* sc_find_event( const char* );
```

### 5.10.3 Constraints on usage

`sc_event` 객체는 elaboration 또는 simulation 중 생성 가능. notify도 양쪽에서 가능. 단 **immediate notification을 elaboration 중 또는 `before_end_of_elaboration`/`end_of_elaboration`/`start_of_simulation` 콜백에서 만들면 error.**

### 5.10.4 생성자와 이름

- `sc_event(const char*)`에 **빈 문자열**을 주면 기본 생성자와 동일한 효과 shall (호출 시점 무관).
- **elaboration 중 또는 initialization phase 전** 호출: 두 생성자 모두 **hierarchically named event 생성** shall.
- **initialization phase 이후 기본 생성자** 호출: hierarchically named 여부는 **인스턴스별로 구현 정의**.
- **initialization phase 이후 `sc_event(const char*)`에 비어있지 않은 문자열**: 구현은 **hierarchically named event를 생성해야** shall.
- hierarchically named event에서 비어있지 않은 문자열을 주면 그것이 문자열 이름. 아니면 `"event"`.
- 이름 충돌로 새 문자열 이름을 대체하면 생성자가 **warning 하나** 생성 shall.
- 이름 없는 event의 구현 정의 이름은 **비어있지 않아야** shall하고, 계층 이름 형태를 취하되 **권장 문자 집합 밖의 문자를 포함할 수 있다** — 구현이 애플리케이션 정의 이름과 구별하려는 의도.
- 소멸자는 event를 object hierarchy에서 제거해 top-level event/child event가 아니게 만든다 shall.

### 5.10.5 이름·계층 순회

| 함수 | 규칙 |
|---|---|
| `name()` | hierarchically named면 계층 이름, 아니면 구현 정의 이름. **항상 비어있지 않다** shall |
| `basename()` | hierarchically named면 문자열 이름, 아니면 구현 정의 이름 |
| `in_hierarchy()` | hierarchically named일 때만 true |
| `triggered()` | **직전 delta notification phase에서 트리거되었거나, 현재 evaluation phase에서 immediate notification으로 트리거되었을 때만** true. `sc_event_or_list` 중 어느 것이 트리거했는지, time-out인지 판정에 유용 |
| `get_parent_object()` | hierarchically named면 부모 `sc_object` 포인터, 아니면 **null**. **top-level event도 null** shall |
| `sc_get_top_level_events()` | hierarchically named이면서 부모 없는 event 전부 |
| `sc_find_event(const char*)` | 계층 이름이 **정확히 일치**하는 event 포인터. 없으면 null. **구현 정의 이름을 가진 event는 반환하지 않는다** shall not |

이 세 순회 함수는 elaboration/simulation 어느 단계에서든 호출 가능하며, elaboration 완료 전이면 **그 시점까지 생성된 부분 계층** 정보를 반환한다 shall.

### 5.10.6 `notify` / `cancel`

| 호출 | 종류 |
|---|---|
| `notify()` | **immediate notification.** sensitive한 모든 프로세스가 `notify` 반환 전에 runnable이 된다 shall. **단 현재 실행 중인 프로세스는 static/dynamic sensitivity와 무관하게 runnable이 되지 않는다** |
| `notify(t)` where `t == 0` | **delta notification** |
| `notify(t)` where `t != 0` | **timed notification.** 시각은 `notify` 호출 시점 기준 상대. **음수 금지** shall not |
| `cancel()` | 이 event의 **pending notification을 삭제** shall |

> - immediate notification으로 sensitive해진 프로세스들은 `notify`를 호출한 프로세스가 suspend하거나 반환할 때까지 실행되지 않는다.
> - 그들은 현재 evaluation phase에서 **구현 정의 순서**로 실행된다 → **immediate notification은 비결정성을 도입한다.**
> - **`sc_prim_channel::update`는 immediate notification을 만들기 위해 `notify()`를 호출하면 안 된다** shall not.
> - 한 event에 대해 **pending notification은 최대 1개.**
> - **immediate notification은 취소할 수 없다.**

### 5.10.8 None event

```cpp
static const sc_event none;
```
사용되지 않는 event 참조가 필요한 문맥용. **절대 notify되지 않으며 const**임이 보장된다. 애플리케이션 코드에서 실수로 명시적 notify를 하는 것을 막는다.

> **구현 불일치 주의** (표준 원문이 아닌 관찰 사항). Accellera SystemC 3.0.2는 이것을 데이터 멤버가 아니라 **정적 멤버 함수** `static const sc_event& none()`으로 제공한다. 따라서 그 구현에서는 `sc_event::none`이 아니라 **`sc_event::none()`**으로 써야 컴파일된다. LRM Introduction은 표준과 참조 시뮬레이터의 동작이 어긋나면 **표준이 확정적(definitive)**이라고 규정하므로, 이는 구현 쪽 이탈이다. 이식성이 필요하면 이 이름에 의존하지 말 것.

### 5.10.9 다중 notification — 가장 이른 것만 살아남는다

한 event는 pending notification을 **최대 1개**만 갖는다 shall. 이미 pending인 상태에서 `notify`가 불리면 **가장 이른 시각에 발생할 것만 살아남고** 늦은 것은 취소된다 (또는 애초에 스케줄되지 않는다) shall.

**이른 순서: immediate < delta < timed.** `notify` **호출 순서와 무관하다.**

```cpp
{
    sc_event e;
    e.notify(SC_ZERO_TIME);   // delta notification
    e.notify(1.0, SC_NS);     // 무시됨 (pending delta가 더 이름)
    e.notify();               // immediate가 pending delta를 취소. e가 notify됨

    e.notify(2.0, SC_NS);     // timed
    e.notify(3.0, SC_NS);     // 무시됨 (더 이른 timed가 pending)
    e.notify(1.0, SC_NS);     // pending timed를 취소하고 대체
    e.notify(SC_ZERO_TIME);   // delta가 pending timed를 취소. 다음 delta cycle에 notify
}
```

---

## 5.11 `sc_time`

- simulation time과 시간 간격(지연, time-out) 표현.
- 내부적으로 **최소 64비트 unsigned 정수** (`sc_time::value_type`) shall.
- `sc_time_unit`은 국제단위계를 따른다 — `SC_FS`는 femtosecond = 1.0E-15초.

```cpp
enum sc_time_unit { SC_SEC, SC_MS, SC_US, SC_NS, SC_PS, SC_FS, SC_AS, SC_ZS, SC_YS };

class sc_time {
public:
    typedef implementation-defined value_type;

    sc_time();
    sc_time( double, sc_time_unit );
    sc_time( const sc_time& );
    explicit sc_time( std::string_view );
    static sc_time from_value( value_type );
    static sc_time from_seconds( double );
    static sc_time from_string( std::string_view );

    sc_time& operator= ( const sc_time& );
    value_type value() const;
    double to_double() const;
    double to_seconds() const;

    bool operator== / != / < / <= / > / >= ( const sc_time& ) const;
    sc_time& operator+= / -= ( const sc_time& );
    sc_time& operator*= / /= ( double );
    sc_time& operator%= ( const sc_time& );

    std::string to_string() const;
    void print( std::ostream& = std::cout ) const;
};

sc_time operator+ / - ( const sc_time&, const sc_time& );
sc_time operator* ( const sc_time&, double );
sc_time operator* ( double, const sc_time& );
sc_time operator/ ( const sc_time&, double );
double  operator/ ( const sc_time&, const sc_time& );   // 비율 (double 반환)
sc_time operator% ( const sc_time&, const sc_time& );
std::ostream& operator<< ( std::ostream&, const sc_time& );

const sc_time SC_ZERO_TIME;

void    sc_set_time_resolution( double, sc_time_unit );
sc_time sc_get_time_resolution();
const sc_time& sc_max_time();
```

### 5.11.3 `sc_time(double, sc_time_unit)` 규칙

- 값은 **time resolution의 가장 가까운 배수로 스케일·반올림** shall. 올림/내림은 **구현 정의**. 반올림 시 warning을 낼 수 있으나 의무 아님.
- **음수** → **warning + `SC_ZERO_TIME` 사용** shall
- **양수이지만 time resolution보다 작음** → **warning + `SC_ZERO_TIME` 사용** shall
- **`sc_max_time`보다 큼** → **warning + `sc_max_time` 사용** shall

### 5.11.3 `sc_time(std::string_view)`

문자열은 최소한 십진 또는 과학 표기법 값을 포함해야 shall. 국제단위계의 metric prefix와 시간 단위 포함 가능. **시간 단위가 없으면 초로 해석.** 변환 불가하면 **error** shall.

```cpp
sc_time("0");        // SC_ZERO_TIME과 동등
sc_time("1 ms");     // sc_time(1.0, SC_MS)
sc_time("1.0E-3");   // sc_time(1.0e-3, SC_SEC)
sc_time("2.0m");     // sc_time(2.0, SC_MS)
sc_time("3 US");     // 오류 — 국제단위계 표기 아님. error를 던져야 함
sc_time("3E us");    // 오류 — 지수 표기 불완전
sc_time("ps");       // 오류 — 값 없음
sc_time("1 fs!");    // 오류 — 잘못된 문자
```

### 5.11.4 연산

모든 산술·관계·동등·대입 연산자는 내부 정수 표현에 대한 **자연스러운 정수 연산** 의미 shall. **정수 underflow와 0 나눗셈 결과는 구현 정의.**
`to_string`/`print`/`operator<<`의 **형식은 구현 정의.**

### 5.11.5 Time resolution

- 시간은 내부적으로 time resolution의 **정수 배수**로 표현된다 shall. **기본값 = 1 picosecond.** 모든 `sc_time` 객체가 **단일 전역 time resolution을 공유**한다 shall.
- `sc_set_time_resolution`은:
  - **elaboration 중에만** 호출 shall only
  - **한 번 초과 호출 금지** shall not
  - **0이 아닌 시간값의 `sc_time` 객체를 만든 후 호출 금지** shall not
  - double 인자는 **양수이고 10의 거듭제곱** shall
  - 이 규칙 위반은 **error**

### 5.11.6 `sc_max_time`

time resolution을 반영한 `sc_time` 최댓값 반환 shall. 한 simulation run 동안 모든 호출이 같은 값 반환 shall. 실제 값은 구현 정의.
- **`sc_max_time` 호출만으로는 time resolution이 고정되지 않는다** — 이후에도 변경 가능.
- 반면 `value`, `to_double`, `to_seconds`, `to_string`, `print`, `operator<<`를 **어떤 `sc_time` 객체에든** 호출하면 **time resolution이 고정된다.**

### 5.11.7 `SC_ZERO_TIME`

시간값 0. **delta notification이나 delta time-out을 만들 때 이 상수를 쓰는 것이 좋은 관행.**

```cpp
sc_core::sc_event e;
e.notify(sc_core::SC_ZERO_TIME);    // delta notification
wait(sc_core::SC_ZERO_TIME);        // delta time-out
```

---

## 5.12 `sc_port`

포트는 모듈이 인스턴스화되는 문맥과 독립적으로 작성될 수 있게 한다. 포트는 바인딩된 channel로 **interface method call을 전달**한다. 포트는 그 모듈이 **요구하는(requires)** 서비스 집합을 정의한다.

**코딩 스타일 규칙:**
- 모듈 **밖**의 channel 멤버 함수 호출 → **포트를 통한 interface method call로** 할 것 (should). 아니면 나쁜 스타일.
- 현재 모듈 **안에** 인스턴스화된 channel의 멤버 함수는 **직접 호출 가능** — 이를 **portless channel access**라 한다.
- **자식 모듈 안의** channel 멤버 함수 호출 → **자식 모듈의 export를 통해** 할 것 (should, §5.13).

### 5.12.2 Class definition

```cpp
enum sc_port_policy {
    SC_ONE_OR_MORE_BOUND,     // Default
    SC_ZERO_OR_MORE_BOUND,
    SC_ALL_BOUND
};

class sc_port_base : public sc_object {
public:
    virtual sc_interface* get_interface() = 0;
    virtual const sc_interface* get_interface() const = 0;
    virtual std::type_index get_interface_type() const = 0;
protected:
    explicit sc_port_base( int, sc_port_policy );
    sc_port_base( const char*, int, sc_port_policy );
    virtual ~sc_port_base();
};

template <class IF>
class sc_port_b : public sc_port_base {
public:
    void operator() ( IF& );
    void operator() ( sc_port_b<IF>& );
    virtual void bind( IF& );
    virtual void bind( sc_port_b<IF>& );

    int size() const;
    IF* operator-> ();
    const IF* operator-> () const;
    IF* operator[] ( int );
    const IF* operator[] ( int ) const;

    virtual sc_interface* get_interface();
    virtual const sc_interface* get_interface() const;
    virtual std::type_index get_interface_type() const;
protected:
    virtual void before_end_of_elaboration();
    virtual void end_of_elaboration();
    virtual void start_of_simulation();
    virtual void end_of_simulation();
    explicit sc_port_b( int, sc_port_policy );
    sc_port_b( const char*, int, sc_port_policy );
    virtual ~sc_port_b();
private:
    sc_port_b();                                        // Disabled
    sc_port_b( const sc_port_b<IF>& );                  // Disabled
    sc_port_b<IF>& operator= ( const sc_port_b<IF>& );  // Disabled
};

template <class IF, int N = 1, sc_port_policy P = SC_ONE_OR_MORE_BOUND>
class sc_port : public sc_port_b<IF> {
public:
    sc_port();
    explicit sc_port( const char* );
    virtual ~sc_port();
    virtual const char* kind() const;                   // "sc_port"
private:
    sc_port( const sc_port<IF,N,P>& );                        // Disabled
    sc_port<IF,N,P>& operator= ( const sc_port<IF,N,P>& );    // Disabled
};
```

### 5.12.3 Template parameters

| 인자 | 의미 |
|---|---|
| `IF` | **interface proper**의 이름 shall. 이것이 **포트의 타입**. 포트는 이 타입에서 파생된 channel, 또는 이 타입에서 파생된 타입의 다른 port/export에만 바인딩 가능 |
| `N` (기본 1) | **한 포트 인스턴스가 바인딩될 수 있는 channel 인스턴스 최대 수.** **0이면 무제한.** 초과 바인딩은 **error**. **N != 1이면 multiport** |
| `P` (기본 `SC_ONE_OR_MORE_BOUND`) | port policy — multiport 바인딩 규칙과 미바인딩 규칙 |

| policy | 의미 |
|---|---|
| `SC_ONE_OR_MORE_BOUND` | 1개 이상 N개 이하. **elaboration 끝에 미바인딩이면 error** |
| `SC_ZERO_OR_MORE_BOUND` | 0개 이상 N개 이하. **미바인딩 허용** |
| `SC_ALL_BOUND` | **정확히 N개** (N > 0일 때). N == 0이면 `SC_ONE_OR_MORE_BOUND`와 동일. 미바인딩 또는 N개 미만이면 **error** |

- **같은 포트를 같은 channel에 두 번 바인딩하면 error** shall (직접이든 다른 포트를 통해서든).
- **port policy는 각 포트 인스턴스에 독립적으로 적용**된다 — 포트끼리 바인딩되어도 마찬가지. 예: 자식의 `sc_port<IF>`를 부모의 `sc_port<IF,2,SC_ALL_BOUND>`에 바인딩하면 두 정책이 모순되어 elaboration 끝에 반드시 error가 난다.
- port policy는 **`end_of_elaboration` 콜백 직전 바인딩 완료 시점에** 성립하면 되고, 그 이전에는 성립할 필요 없다. 예: `sc_port<IF,2,SC_ALL_BOUND>`를 생성자에서 한 번, `before_end_of_elaboration`에서 한 번 바인딩해도 됨.

```cpp
sc_port<IF>                              p0;  // 정확히 1개
sc_port<IF, 0>                           p1;  // 1개 이상, 상한 없음
sc_port<IF, 3>                           p2;  // 1, 2, 3개
sc_port<IF, 0, SC_ZERO_OR_MORE_BOUND>    p3;  // 0개 이상, 상한 없음
sc_port<IF, 1, SC_ZERO_OR_MORE_BOUND>    p4;  // 0 또는 1개
sc_port<IF, 3, SC_ZERO_OR_MORE_BOUND>    p5;  // 0~3개
sc_port<IF, 3, SC_ALL_BOUND>             p6;  // 정확히 3개
```

### 5.12.4 Constraints on usage

- 구현은 `sc_port_base`를 `sc_object`에서 파생시켜야 shall.
- 포트는 **elaboration 중, 모듈 안에서만** 인스턴스화 shall only. 그 외 위치나 simulation 중은 **error**.
- **`size`와 `get_interface`는 elaboration/simulation 어느 때나** 호출 가능. **`operator->`와 `operator[]`는 `end_of_elaboration`부터 또는 simulation 중에만** 호출 should.
- **강력 권장**: 포트는 그 모듈이 인스턴스화되는 지점(즉 인스턴스화하는 쪽 생성자)에서 바인딩할 것. 그리고 그 모듈 인스턴스를 담고 있는 모듈 안에 인스턴스화된 channel/port 또는 자식 모듈의 export에 바인딩할 것. (예외적으로 포트 자신의 `before_end_of_elaboration` 콜백에서 미바인딩 포트를 바인딩하는 것은 편리하다.)
- **강력 권장**: 포트 인스턴스는 가능하면 **모듈의 데이터 멤버**로 둘 것 — 아니면 named binding 문법이 난해해진다.
- 원격 바인딩·모듈 계층을 벗어난 바인딩은 문법적으로 가능하지만 **모듈 계층 규율을 무너뜨리므로 강력히 권장하지 않음.**

### 5.12.5 Constructors

`sc_port` 기본 생성자는 `sc_gen_unique_name("port")`로 유일 이름을 생성해 `sc_object` 생성자에 넘긴다 shall. → **포트에 명시적 문자열 이름을 안 줘도 된다.**

### 5.12.7 Named port binding

```cpp
void operator() ( IF& );          virtual void bind( IF& );           // channel(또는 export)에
void operator() ( sc_port_b<IF>& ); virtual void bind( sc_port_b<IF>& ); // 다른 port에
```

`operator()`는 **virtual `bind`를 호출해서** 효과를 낸다 shall. 실인자가 export면 C++ 컴파일러가 `sc_export<IF>::operator IF&` 암묵 변환을 호출한다.

```cpp
SC_MODULE(M) {
    sc_core::sc_inout<int> SC_NAMED(P), SC_NAMED(Q), SC_NAMED(R), SC_NAMED(S);
    sc_core::sc_inout<int> *T;                 // 포트 포인터 — 비권장
    SC_CTOR(M) { T = new sc_core::sc_inout<int>("T"); }
};

SC_MODULE(Top) {
    sc_core::sc_inout<int>  SC_NAMED(A), SC_NAMED(B);
    sc_core::sc_signal<int> SC_NAMED(C), SC_NAMED(D), SC_NAMED(E);
    M m;
    SC_CTOR(Top) : m("m") {
        m.P(A);          // P-A
        m.Q.bind(B);     // Q-B
        m.R(C);          // R-C
        m.S.bind(D);     // S-D
        m.T->bind(E);    // T-E
    }
};
```

### 5.12.8 바인딩된 포트 조회

**순서집합 S 결정 규칙** (§5.12.8.1):
- a) port/export가 channel 인스턴스에 바인딩되면 그 channel을 **S의 끝에 추가**
- b) port/export가 export에 바인딩되면 그 export에 대해 a), b)를 **재귀 적용**
- c) port가 다른 port에 바인딩되면 그 port에 대해 a), b), c)를 **재귀 적용**

구현이 바인딩 완료를 지연할 수 있으므로 **elaboration 중에는 개수와 순서가 변할 수 있고 최종 순서는 구현 정의**. 단 **`end_of_elaboration` 콜백 중과 simulation 중에는 변하지 않는다** shall.

> 그 결과 같은 channel이 계층상 위치가 다른 포트에서 볼 때 순서집합의 다른 위치에 있을 수 있다 — 부모 포트의 첫 channel이 자식 포트의 세 번째 channel일 수 있다.

| 함수 | 규칙 |
|---|---|
| `size()` | 바인딩된 channel 인스턴스 수. **`end_of_elaboration` 이전에 호출하면 반환값 구현 정의** |
| `operator->()` | **첫 번째** 바인딩 channel 포인터. **미바인딩 포트에 호출하면 error** shall. `end_of_elaboration` 이전 호출은 **구현 정의** |
| `operator[](int)` | 인덱스 channel 포인터. 인덱스는 0부터, **바인딩 완료 순서** (구현 정의). 범위 `0..N-1` 밖이면 **error** shall. `end_of_elaboration` 이전 호출은 구현 정의. **multiport가 아닌 포트에도 호출 가능** (인자는 0이어야 should) |
| `get_interface()` | 첫 번째 바인딩 channel 포인터. **미바인딩이면 null.** 바인딩 여부 테스트에 사용 가능. 생성자나 `before_end_of_elaboration`에서 호출 시 null인지 여부는 **구현 정의**. 특수 포트 클래스 구현용 — 일반적으로 `operator->`를 쓸 것. 단 **포트 interface 타입의 멤버가 아닌 channel 클래스 멤버 함수를 호출할 수 있게 해준다** |
| `get_interface_type()` | 포트의 interface proper 타입 반환 shall |

```cpp
// operator-> : interface method call의 핵심
struct iface : virtual sc_core::sc_interface {
    virtual int read() const = 0;
};
struct chan : iface, sc_core::sc_prim_channel {
    virtual int read() const;
};

SC_MODULE(modu) {
    sc_port<iface> SC_NAMED(P);
    SC_CTOR(modu) { SC_THREAD(thread); }
    void thread() { int i = P->read(); }        // interface method call
};

// operator[] + size() : multiport 순회
class bus_channel : public bus_interface, public sc_core::sc_module {
public:
    sc_core::sc_port<peripheral_interface, 0> peripheral_port;   // multiport
    SC_CTOR(bus_channel) { SC_THREAD(action); }
private:
    void action() {
        for (int i = 0; i < peripheral_port.size(); i++)
            peripheral_port[i]->peripheral_write(0, 0);
    }
};

SC_MODULE(top_level) {
    bus_channel bus;
    memory ram0, ram1, ram2, ram3;
    SC_CTOR(top_level) : bus("bus"), ram0("ram0"), ram1("ram1"), ram2("ram2"), ram3("ram3") {
        bus.peripheral_port(ram0);
        bus.peripheral_port(ram1);
        bus.peripheral_port(ram2);
        bus.peripheral_port(ram3);        // 하나의 multiport를 4개 channel에
    }
};

// get_interface : 포트가 바인딩된 객체의 비-interface 멤버 함수 호출
SC_MODULE(Top) {
    sc_core::sc_in<bool> clock;
    void before_end_of_elaboration() {
        sc_interface* i_f = clock.get_interface();
        sc_clock* clk = dynamic_cast<sc_clock*>(i_f);
        sc_time t = clk->period();
    }
};
```

> `get_interface`는 multiport의 첫 channel 이후를 반환할 수 없다 — `operator[]`를 쓸 것.

---

## 5.13 `sc_export`

export는 모듈이 **부모 모듈에게 interface를 제공**하게 한다. export는 바인딩된 channel로 interface method call을 전달한다. export는 그 모듈이 **제공하는(provides)** 서비스 집합을 정의한다.

> 모듈이 단순히 interface를 직접 구현하는 것의 대안. 명시적 export를 쓰면 **한 모듈 인스턴스가 여러 interface를 구조적으로 제공**할 수 있다.

```cpp
class sc_export_base : public sc_object {
public:
    virtual sc_interface* get_interface() = 0;
    virtual const sc_interface* get_interface() const = 0;
    virtual std::type_index get_interface_type() const = 0;
protected:
    sc_export_base();
    sc_export_base( const char* );
    virtual ~sc_export_base();
};

template<class IF>
class sc_export : public sc_export_base {
public:
    sc_export();
    explicit sc_export( const char* );
    virtual ~sc_export();
    virtual const char* kind() const;      // "sc_export"

    void operator() ( IF& );
    virtual void bind( IF& );
    operator IF& ();
    operator const IF& () const;

    IF* operator-> ();
    const IF* operator-> () const;

    virtual sc_interface* get_interface();
    virtual const sc_interface* get_interface() const;
    virtual std::type_index get_interface_type() const;
protected:
    virtual void before_end_of_elaboration();
    virtual void end_of_elaboration();
    virtual void start_of_simulation();
    virtual void end_of_simulation();
private:
    sc_export( const sc_export<IF>& );                    // Disabled
    sc_export<IF>& operator= ( const sc_export<IF>& );    // Disabled
};
```

### 5.13.3–5.13.4 Constraints

- 템플릿 인자는 **interface proper** 이름 shall = **export의 타입**. export는 그 타입에서 파생된 channel, 또는 그 타입에서 파생된 타입의 다른 export에만 바인딩 가능.
- export는 **elaboration 중, 모듈 안에서만** 인스턴스화 shall only. 그 외는 **error**.
- **모든 모듈 인스턴스의 모든 export는 elaboration 중 정확히 한 번 바인딩되어야** shall. elaboration 끝에 미바인딩이면 **error**. 두 개 이상 channel에 바인딩해도 **error**.
- `get_interface`는 elaboration/simulation 어느 때나, **`operator->`는 simulation 중에만** 호출 should.
- **강력 권장**: export는 **같은 모듈 안에서** 바인딩. 그리고 그 모듈 안에 인스턴스화된 channel, 그 모듈이 구현한 channel, 또는 자식 모듈의 export에 바인딩.

### 5.13.5 Constructors
기본 생성자는 `sc_gen_unique_name("export")`로 유일 이름 생성 shall.

### 5.13.7 Export binding

**위치 바인딩 개념은 export에 없다.** 두 함수 모두 **즉시 바인딩**한다 (포트와 달리 지연 없음).

```cpp
struct i_f : virtual sc_core::sc_interface {
    virtual void print() = 0;
};

struct Chan : sc_core::sc_channel, i_f {
    SC_CTOR(Chan) {}
    void print() { std::cout << "I'm Chan, name=" << name() << std::endl; }
};

struct Caller : sc_core::sc_module {
    sc_core::sc_port<i_f> p;
    SC_CTOR(Caller) { ... }
};

struct Bottom : sc_core::sc_module {
    sc_core::sc_export<i_f> xp;
    Chan ch;
    SC_CTOR(Bottom) : xp("xp"), ch("ch") {
        xp.bind(ch);                 // export를 channel에
    }
};

struct Middle : sc_core::sc_module {
    sc_core::sc_export<i_f> xp;
    Bottom *b;
    SC_CTOR(Middle) : xp("xp") {
        b = new Bottom("b");
        xp.bind(b->xp);              // export를 export에
        b->xp->print();              // 자식 모듈의 export를 통한 호출
    }
};

struct Top : sc_core::sc_module {
    Caller *c;  Middle *m;
    SC_CTOR(Top) {
        c = new Caller("c");
        m = new Middle("m");
        c->p(m->xp);                 // port를 export에
    }
};
```

### 5.13.8 조회

바인딩된 channel 결정: (a) channel에 바인딩되면 그것, (b) 다른 export에 바인딩되면 재귀 적용.

| 함수 | 규칙 |
|---|---|
| `operator->()`, `operator IF&()` | 바인딩된 channel 포인터/참조 반환 shall. **미바인딩 상태에서 호출하면 error** shall |
| `get_interface()` | 바인딩된 channel 포인터. **미바인딩이면 null.** 바인딩 여부 테스트에 사용 가능 |
| `get_interface_type()` | export의 interface proper 타입 |

> - `operator->`는 **simulation 중, export를 가진 모듈의 부모에서 interface method call**을 하기 위한 것.
> - `operator IF&`는 **elaboration 중 암묵 변환용** — 포트를 export에 바인딩하거나 export를 프로세스의 static sensitivity에 추가할 때.
> - **`sc_export`에는 `operator[]`가 없고 multi-export 개념도 없다.** 각 export는 단 하나의 channel에만 바인딩된다.

---

## 5.14 `sc_interface`

모든 interface의 추상 base class.

```cpp
class sc_interface {
public:
    virtual void register_port( sc_port_base&, const char* );
    virtual const sc_event& default_event() const;
    virtual ~sc_interface();
protected:
    sc_interface();
private:
    sc_interface( const sc_interface& );                  // Disabled
    sc_interface& operator= ( const sc_interface& );      // Disabled
};
```

### 5.14.3 interface proper 규칙

`sc_interface`를 interface proper 외의 클래스의 **직접 base class로 쓰면 안 된다** (should not).

**interface proper가 지켜야 할 규칙 (shall):**
- `sc_interface`에서 **직접 또는 간접적으로 public 파생**
- `sc_interface`에서 **직접** 파생되면 **`virtual` 지정자 사용**
- **`sc_object`에서 직접·간접적으로 파생되면 안 됨**

**전형적으로 지켜야 할 것 (should):**
- pure virtual 함수를 하나 이상 포함
- interface proper가 아닌 다른 클래스에서 파생되지 않음
- pure virtual 함수 외의 함수 선언·정의를 포함하지 않음
- 데이터 멤버를 포함하지 않음

> interface proper는 다른 interface proper 하나 또는 여럿에서 파생 가능 (다중 상속 계층 생성 가능). channel 클래스는 몇 개의 interface proper에서든 파생 가능.

### 5.14.4 `register_port`

```cpp
virtual void register_port( sc_port_base&, const char* );
```

`sc_interface`의 정의는 **아무것도 하지 않는다.** 애플리케이션이 channel에서 오버라이드할 수 있다.
목적: **port binding에 의존하는 동작을 elaboration 중에 수행** — 연결 오류 검사 등.

- 포트가 channel 인스턴스에 바인딩될 때마다 구현이 호출한다 shall.
- 첫 인자 = **바인딩되는 포트 인스턴스 참조**. 둘째 인자 = **`typeid(IF).name()`** 값 (IF는 포트의 interface 타입).
- **export가 channel에 바인딩될 때는 호출되지 않는다** shall not.
- 포트 P가 포트 Q에, Q가 channel에 바인딩되면 **첫 인자는 P** (자식 쪽 포트) shall — 계층 아래쪽으로 재귀적으로.
- **여러 포트가 같은 channel/port 인스턴스에 바인딩되면 각 포트마다 한 번씩** 호출된다 shall.

```cpp
void register_port( sc_core::sc_port_base& port_, const char* if_typename_ ) {
    std::string nm( if_typename_ );
    if ( nm == typeid( my_interface ).name() )
        std::cout << " channel " << name() << " bound to port " << port_.name() << std::endl;
}
```

### 5.14.5 `default_event`

포트나 channel 인스턴스가 `sc_sensitive†::operator<<`에 **직접 인자로 넘겨져** static sensitivity를 정의할 때 **모든 경우에** 구현이 호출한다 shall. 애플리케이션은 그 channel에서 이 함수를 오버라이드해 프로세스가 sensitive해질 event 참조를 반환해야 shall.

**구현이 호출했는데 애플리케이션이 오버라이드하지 않았으면 구현이 warning을 낼 수 있다.**

```cpp
struct my_if : virtual sc_core::sc_interface {
    virtual int read() = 0;
};

class my_ch : public my_if, public sc_core::sc_module {
public:
    virtual int read() { return m_val; }
    virtual const sc_core::sc_event& default_event() const { return m_ev; }
private:
    int m_val;
    sc_core::sc_event m_ev;
};
```

---

## 5.15 `sc_prim_channel`

모든 primitive channel의 base class. **스케줄러의 update phase에 대한 독점 접근**을 제공한다.

```cpp
class sc_prim_channel : public sc_object {
public:
    virtual const char* kind() const;                 // "sc_prim_channel"
protected:
    sc_prim_channel();
    explicit sc_prim_channel( const char* );
    virtual ~sc_prim_channel();

    void request_update();
    void async_request_update();
    void async_attach_suspending();
    void async_detach_suspending();

    virtual void update();

    void next_trigger( ... );    // sc_module과 동일한 12개 오버로드
    void wait( ... );            // sc_module과 동일한 13개 오버로드

    virtual void before_end_of_elaboration();
    virtual void end_of_elaboration();
    virtual void start_of_simulation();
    virtual void end_of_simulation();
private:
    sc_prim_channel( const sc_prim_channel& );                  // Disabled
    sc_prim_channel& operator= ( const sc_prim_channel& );      // Disabled
};
```

### 5.15.3 Constraints on usage

- **elaboration 중에만** 생성 가능. simulation 중 인스턴스화는 **error**.
- primitive channel은 `sc_prim_channel`에서 **public 파생**되어야 should.
- primitive channel은 **하나 이상의 interface를 구현해야** shall.
- `request_update`/`async_request_update`는 elaboration 또는 simulation 중 호출 가능. **elaboration 중 호출하면 initialization phase에서 실행**된다 shall.
- 생성자가 protected이므로 직접 인스턴스화 불가.

### 5.15.6 `request_update` / `async_request_update` / `update`

- **한 primitive channel 인스턴스에 대해 `request_update`와 `async_request_update`를 둘 다 호출하면 안 된다** (should not). 둘 다 호출하면 동작은 **undefined**. (서로 다른 channel 인스턴스에 각각 쓰는 것은 무방.)

**`request_update`**: 현재 channel에 대한 update request를 큐잉 shall. **한 update phase에 한 인스턴스당 update request는 최대 1개** — 같은 evaluation phase에서 여러 번 호출해도 request는 하나 shall.

**`async_request_update`**: 호스트 OS에 대해 **thread-safe하게** update request를 큐잉 shall. **SystemC 커널·thread process가 아닌 다른 OS 스레드에서 안정적으로 호출 가능**하도록 의도됨.
- elaboration/simulation 중 언제든, 커널에 대해 비동기적으로 호출 가능.
- 구현은 각 호출이 **evaluation phase에서 실행된 것처럼** 동작함을 보장해야 shall.
- 한 update phase당 인스턴스당 최대 1개 — 두 update phase 사이에 받은 여러 호출은 하나로 병합된다 shall.
- **어느 phase에서 받고 처리되는지는 undefined** → 좁은 시간창의 두 호출이 update request 1개가 될 수도, 연속 delta cycle의 2개가 될 수도, 비연속 2개가 될 수도 있다.
- **공유 메모리 접근 동기화 책임은 애플리케이션에 있다** — `async_request_update`를 호출하는 SW 스레드가 보통 `update`가 읽는 변수에 값을 쓴다. 구현은 그 무결성을 보장하지 않는다.
- **SystemC 커널 컨텍스트(thread/method process 등)에서 호출하는 것은 권장하지 않는다** — 별도 OS 스레드에서만. `request_update`보다 성능이 나쁠 수 있다.

**`update`**: `request_update`/`async_request_update`에 대응해 스케줄러가 update phase에 콜백 shall. `sc_prim_channel`의 정의는 아무것도 하지 않음.

파생 클래스에서 오버라이드할 때 **다음을 수행하면 안 된다** (위반 시 동작 **undefined**):
- a) `sc_prim_channel`의 **어떤 멤버 함수도 호출** — 단 base class에 오버라이드된 `update` 자신은 예외
- b) `sc_event::notify()` (인자 없음) 로 **immediate notification 생성**
- c) `sc_process_handle`의 **process control 멤버 함수** 호출 (`suspend`, `kill` 등)

**추가로 should not (위반 시 비결정적 동작 가능):**
- 현재 객체의 데이터 멤버 외의 **저장소 상태 변경**
- 현재 객체 외의 **primitive channel 인스턴스 상태 읽기**
- **다른 channel 인스턴스의 interface method 호출** — 특히 **signal에 write 금지**

**허용**: `sc_spawn`으로 동적 프로세스 생성 가능 — 단 그 프로세스는 **다음 evaluation phase까지 runnable이 되지 않는다** shall.

> `request_update`/`update`의 목적: evaluation phase에서 발생한 **동시 요청을 update phase에서 해소·중재**. 중재 방식은 애플리케이션 책임 — 결정적이든 무작위든.
> `update`는 전형적으로 현재 객체의 데이터 멤버만 읽고 수정하며 delta notification을 만든다.

### 5.15.7 `async_attach_suspending` / `async_detach_suspending`

- `async_attach_suspending`: channel이 **suspension 요청에 자신을 attach** shall. **하나 이상의 primitive channel이 attach되어 있으면**, 내부 event가 고갈되어도 **simulation을 끝내지 않고 suspend**하고 외부의 `async_request_update` 호출을 기다린다.
- `async_detach_suspending`: 이전 attach를 해제 shall. **attach된 channel이 하나도 없으면** 내부 event 고갈 시 simulation이 종료된다.

> 유스케이스: 시뮬레이터 외부 자극을 기다리는 primitive channel이, 내부 event 고갈로 simulation이 끝나버리는 것을 막는다.

### 5.15.8 `next_trigger` / `wait`

`sc_module`의 동명·동일 시그니처 멤버 함수와 **동작이 동일** shall. 호출 문맥 제약도 동일 — 예컨대 `next_trigger`는 **method process에서만** 호출 가능.

### primitive channel 작성 템플릿

```cpp
struct my_if : virtual sc_core::sc_interface {          // interface proper
    virtual int  read()  = 0;
    virtual void write(int) = 0;
};

struct my_prim : sc_core::sc_prim_channel, my_if {      // primitive channel
    my_prim()
    : sc_core::sc_prim_channel(sc_core::sc_gen_unique_name("my_prim")),
      m_req(false), m_written(false), m_cur_val(0) {}

    virtual void write(int val) {
        if (!m_req) {                    // 한 delta에 처음 쓴 값만 보존
            m_new_val = val;
            request_update();            // update request 스케줄
            m_req = true;
        }
    }

    virtual void update() {              // update phase에 스케줄러가 콜백
        m_cur_val = m_new_val;
        m_req = false;
        m_written = true;
        m_write_event.notify(SC_ZERO_TIME);   // delta notification
    }

    virtual int read() {
        if (!m_written)
            wait(m_write_event);         // update()가 불릴 때까지 블록
        m_written = false;
        return m_cur_val;
    }

    bool m_req, m_written;
    sc_core::sc_event m_write_event;
    int m_new_val, m_cur_val;
};
```

---

## 5.16 `sc_object`

`sc_module`/`sc_port`/`sc_export`/`sc_prim_channel` 및 프로세스 인스턴스 구현 클래스의 **공통 base class**.

- object hierarchy: 각 `sc_object`는 **부모가 최대 1개**, 형제·자식은 여럿 가능. **module 객체와 process 객체만 자식을 가질 수 있다.**
- 모듈 인스턴스의 자식 = 그 모듈 안에 있는 객체 (§3.1.4). 프로세스 인스턴스의 자식 = **그 프로세스 연관 함수 실행 중 생성된** 객체.
- **top-level object** = 부모 없는 객체. 모듈 인스턴스, spawned process 인스턴스, 애플리케이션 정의 `sc_object` 파생 객체가 될 수 있다.
- `sc_spawn` 호출마다 생성되는 spawned process는 **호출자의 자식이거나 top-level object** shall. 부모는 spawned/unspawned process 또는 module instance일 수 있다.
- 각 `sc_object`는 **유일한 계층 이름**을 가져야 shall.
- 명시적으로 금지된 경우(`sc_module`/`sc_port`/`sc_export`/`sc_prim_channel`)를 제외하면 **언제든 삭제 가능**. 삭제되면 자식 관계가 끊긴다. **프로세스 인스턴스 객체는 살아있는 자식이 있는 동안 삭제되지 않는다** shall not.

```cpp
class sc_object {
public:
    virtual ~sc_object();

    const char* name() const;
    const char* basename() const;
    virtual const char* kind() const;                  // "sc_object"

    virtual void print( std::ostream& = std::cout ) const;
    virtual void dump ( std::ostream& = std::cout ) const;

    virtual const std::vector<sc_object*>& get_child_objects() const;
    virtual const std::vector<sc_event*>&  get_child_events() const;
    sc_object* get_parent_object() const;

    bool add_attribute( sc_attr_base& );
    sc_attr_base* get_attribute( const std::string& );
    const sc_attr_base* get_attribute( const std::string& ) const;
    sc_attr_base* remove_attribute( const std::string& );
    void remove_all_attributes();
    int  num_attributes() const;
    sc_attr_cltn& attr_cltn();
    const sc_attr_cltn& attr_cltn() const;
protected:
    sc_object();
    sc_object( const char* );
    sc_object( const sc_object& );
    sc_object& operator= ( const sc_object& );
    virtual sc_hierarchy_scope get_hierarchy_scope();
};

sc_object* get_current_sc_object();
const std::vector<sc_object*>& sc_get_top_level_objects();
sc_object* sc_find_object( const char* );
```

### 5.16.3 Constraints on usage

- 애플리케이션은 `sc_object`를 다른 클래스의 base class로 쓸 수 있다. 계층 이름 접근이나 속성 부착 목적.
- **`sc_object` base class sub-object를 둘 이상 갖는 클래스를 정의하면 안 된다** shall not.
- `sc_object`는 elaboration 또는 simulation 중 인스턴스화 가능. 단 module/port/export/primitive channel은 **elaboration 중에만**.
- **hierarchical channel도 primitive channel도 아니지만 `sc_object`에서 파생된 channel**을 만들어 elaboration 또는 simulation 중 인스턴스화하는 것이 허용된다. 어떤 channel이든 **portless channel access는 허용**되지만, **simulation 중 인스턴스화된 channel에는 port나 export를 바인딩할 수 없다.**
- `sc_object`에 직접 파생된 클래스들(`sc_module` 등)은 `sc_object`를 **비가상 base class**로 가지므로 **`sc_object` 파생 클래스로부터의 다중 상속은 불가.**

### 5.16.4 생성자·소멸자

| 함수 | 규칙 |
|---|---|
| `sc_object()` / `sc_object(const char*)` | object hierarchy에 등록하고 문자열 이름으로 계층 이름을 만든다 shall. **빈 문자열은 기본 생성자와 동일** → 이름은 `"object"`. 이름 충돌로 대체하면 **warning 하나** 생성 shall |
| `sc_object(const sc_object& arg)` | `sc_object(arg.basename())`로 만든 것처럼 **새 객체 생성** shall. 즉 모듈 생성자에서 만들면 그 모듈의 자식이 됨. 기존 객체의 문자열 이름이 **seed**로 쓰임. **속성과 자식은 복사되지 않는다** shall not |
| `operator=` | **계층 이름과 부모를 수정하지 않는다** shall not — 대상 객체는 현재 위치를 유지. 속성과 자식도 수정 안 됨. `*this` 반환 shall |
| `~sc_object()` | 객체 삭제, 부착된 속성 컬렉션 삭제, object hierarchy에서 제거 shall |

### 5.16.5–5.16.6 name / basename / kind / print / dump

- `name()` = 계층 이름, `basename()` = 문자열 이름, `kind()` = 종류 문자열. **구현의 모든 `sc_object` 파생 클래스는 `kind`를 오버라이드해야** shall.
- `print()`: `name()` 문자열만 출력 shall. **추가 문자를 출력하면 안 된다** shall not.
- `dump()`: **최소한 name과 kind**를 출력 shall. 형식은 구현 정의. 디버깅용.

```cpp
SC_MODULE(Mod) {
    sc_core::sc_port<sc_core::sc_signal_in_if<int> > p;
    SC_CTOR(Mod) : p("p") {}    // p.name()=="top.mod.p", p.basename()=="p", p.kind()=="sc_port"
};
SC_MODULE(Top) {
    Mod *mod;                            // mod->name() == "top.mod"
    sc_core::sc_signal<int> sig;         // sig.name()  == "top.sig"
    SC_CTOR(Top) : sig("sig") {
        mod = new Mod("mod");
        mod->p(sig);
    }
};
int sc_main(int, char*[]) {
    Top top("top");                      // top.name() == "top"
    sc_start();
    return 0;
}
```

### 5.16.7 계층 순회

| 함수 | 규칙 |
|---|---|
| `get_child_objects()` | 현재 객체의 자식 `sc_object` 전부. `sc_object`의 정의는 **빈 vector**를 반환하되, **자식을 가질 수 있는 클래스(`sc_module`, 프로세스 클래스)에서 구현이 오버라이드**해야 shall |
| `get_child_events()` | 현재 객체가 부모인 hierarchically named event 전부. 위와 동일한 오버라이드 규칙 |
| `get_parent_object()` | 부모 포인터. **top-level object면 null** shall. 부모가 terminated된 프로세스여도 그 포인터 반환 |
| `get_current_sc_object()` | **현재 hierarchy 컨텍스트**: elaboration 중이면 현재 부모 모듈, simulation 중이면 현재 활성 프로세스, 시뮬레이터 컨텍스트 밖이면 **null** shall |
| `sc_get_top_level_objects()` | 모든 top-level `sc_object` |
| `sc_find_object(const char*)` | 계층 이름이 **정확히 일치**하는 `sc_object` 포인터. 없으면 null |

호출 시점 규칙은 §5.10.5의 event 순회 함수와 동일 — 부분 구축된 계층을 반환한다.

```cpp
void scan_hierarchy(sc_core::sc_object *obj) {
    std::vector<sc_core::sc_object *> children = obj->get_child_objects();
    for (unsigned i = 0; i < children.size(); i++)
        if (children[i]) scan_hierarchy(children[i]);
}

std::vector<sc_core::sc_object *> tops = sc_core::sc_get_top_level_objects();
for (unsigned i = 0; i < tops.size(); i++)
    if (tops[i]) scan_hierarchy(tops[i]);

sc_core::sc_object *obj = sc_core::sc_find_object("foo.foobar");
sc_core::sc_module *m;
if ( (m = dynamic_cast<sc_core::sc_module *>(obj)) ) { /* 모듈이다 */ }

sc_core::sc_object *parent = obj->get_parent_object();
if (parent) std::cout << parent->name() << " " << parent->kind();
```

### 5.16.8 속성

| 함수 | 규칙 |
|---|---|
| `add_attribute(sc_attr_base&)` | 같은 이름의 속성이 이미 붙어 있으면 **붙이지 않고 false** 반환 shall. 아니면 붙이고 **true** shall. 인자는 `sc_attr_base`가 아니라 **`sc_attribute` 객체여야** should |
| `get_attribute(name)` | 있으면 포인터, 없으면 null |
| `remove_attribute(name)` | 있으면 제거하고 포인터 반환, 없으면 null |
| `remove_all_attributes()` | 전부 제거 |
| `num_attributes()` | 개수 |
| `attr_cltn()` | 속성 컬렉션 반환 (§5.20) |

**속성의 수명은 모든 객체에서 완전히 제거될 때까지 유지되어야** shall. 아직 붙어 있는 속성을 애플리케이션이 삭제하면 동작은 **undefined** shall.

`get_attribute` 반환 포인터는 `sc_attribute<T>*`로 캐스트해야 `value` 데이터 멤버에 접근 가능.

```cpp
sc_signal<int> sig;
sc_attribute<int> a("number", 1);
sig.add_attribute(a);

sc_attribute<int>* ap = (sc_attribute<int>*)sig.get_attribute("number");
++ap->value;
```

### 5.16.9 `get_hierarchy_scope`

객체의 계층 스코프를 담은 `sc_hierarchy_scope` 객체 반환 shall (§5.21).

```cpp
SC_MODULE(SignalStub) {
    SC_CTOR(SignalStub) {}
    template<typename T>
    void stub( sc_core::sc_in<T>& port ) {
        // 현재 SystemC 계층을 이 모듈로 강제
        sc_core::sc_hierarchy_scope scope = get_hierarchy_scope();
        auto* sig = new sc_core::sc_signal<T>( sc_core::sc_gen_unique_name(port.basename()) );
        port.bind( *sig );
    }
};
```

---

## 5.17 계층 이름 규칙

- 계층 이름 = **마침표 `.`로 구분된 문자열 이름들** — top-level `sc_object`의 문자열 이름에서 시작해, 계층을 따라 내려가며 각 module/process 인스턴스의 문자열 이름을 포함하고, 마지막에 그 `sc_object`/`sc_event` 자신의 문자열 이름으로 끝난다.
- **대소문자 구분.**
- **문자열 이름에 마침표(`.`)나 공백 문자가 들어가면 error** shall.
- **강력 권장 문자 집합**: `a-z`, `A-Z`, `0-9`, `_`. 벗어나면 구현이 warning을 낼 수 있으나 의무 아님.
- **계층 이름은 단일 전역 네임스페이스**. 각 `sc_object`와 각 hierarchically named `sc_event`는 **유일하고 비어있지 않은** 계층 이름을 가져야 shall. **구현은 애플리케이션이 명시적으로 만든 객체·event의 계층 이름 외에 다른 이름을 이 네임스페이스에 추가하면 안 된다** shall not.
- **이름 생성 알고리즘**: 문자열 이름(또는 기본값 `"object"`/`"event"`)으로 계층 이름을 만들고 유일성 검사 → 유일하면 채택 → 아니면 `sc_gen_unique_name`에 문자열 이름을 seed로 넘겨 반환값을 새 문자열 이름으로 삼아 **유일해질 때까지 반복** shall.
- 한 객체 생성 중 `sc_gen_unique_name`이 두 번 이상 불리면 두 번째 이후의 seed 선택은 구현 정의이나, **첫 호출의 seed이거나 이전 반환값 중 하나여야** shall → **최종 이름은 원래 문자열 이름을 prefix로 갖는다.**

```cpp
namespace sc_core {
    bool sc_hierarchical_name_exists( const char* );

    bool sc_register_hierarchical_name(std::string_view name);
    bool sc_register_hierarchical_name(const sc_object* parent, std::string_view name);
    bool sc_unregister_hierarchical_name(std::string_view name);
    bool sc_unregister_hierarchical_name(const sc_object* parent, std::string_view name);
}
```

- `sc_hierarchical_name_exists`: 인자가 `sc_object` 또는 hierarchically named event의 계층 이름과 **정확히 일치할 때만** true shall. **구현 정의 event 이름과만 일치하면 false** shall (계층 이름과도 일치하지 않는 한).
- `sc_register_hierarchical_name`: 계층 이름을 **예약**해 새 객체·event 생성 시 쓰이지 않게 shall. 즉 `sc_gen_unique_name`과 `sc_object` 생성자가 이 이름을 고려한다. **SystemC 객체·event 계층 밖의 이름과 충돌을 피하려는 용도.**
- `sc_unregister_hierarchical_name`: 예약 해제 shall. **이전에 예약된 이름에만 효과** 있음.
- 2-파라미터 변형은 부모 + 인스턴스 이름 지정 (= 부모 계층 이름 + 둘째 파라미터).

---

## 5.18–5.20 속성 클래스

### `sc_attr_base` — 속성 base class (이름만 저장)

```cpp
class sc_attr_base {
public:
    sc_attr_base( const std::string& );
    sc_attr_base( const sc_attr_base& );
    virtual ~sc_attr_base();
    const std::string& name() const;
private:
    sc_attr_base();                                      // Disabled
    sc_attr_base& operator= ( const sc_attr_base& );     // Disabled
};
```

이름은 객체에서 속성을 꺼낼 때 **키**로 쓰인다. **한 객체에 붙은 속성들은 유일한 이름을 가져야** shall. 서로 다른 객체에 같은 이름의 속성을 붙이는 것은 가능.

### `sc_attribute<T>` — 값 저장

```cpp
template <class T>
class sc_attribute : public sc_attr_base {
public:
    sc_attribute( const std::string& );
    sc_attribute( const std::string&, const T& );
    sc_attribute( const sc_attribute<T>& );
    virtual ~sc_attribute();
    T value;                                             // public 데이터 멤버
private:
    sc_attribute();                                          // Disabled
    sc_attribute<T>& operator= ( const sc_attribute<T>& );   // Disabled
};
```

`T`는 **copy-constructible 타입** shall. 값을 안 주면 `T`의 기본 생성자로 구성.

### `sc_attr_cltn` — 속성 컨테이너

```cpp
class sc_attr_cltn {
public:
    typedef sc_attr_base* elem_type;
    typedef elem_type* iterator;
    typedef const elem_type* const_iterator;
    iterator begin();          const_iterator begin() const;
    iterator end();            const_iterator end() const;
    // 나머지 implementation-defined
private:
    sc_attr_cltn( const sc_attr_cltn& );                 // Disabled
    sc_attr_cltn& operator= ( const sc_attr_cltn& );     // Disabled
};
```

- **애플리케이션이 명시적으로 생성하면 안 된다** shall not. `sc_object::attr_cltn()`이 반환하는 컬렉션을 iterator로 순회할 수 있다.
- 구현은 **새 속성이 붙거나 기존 속성이 제거되기 전까지만** 컬렉션 유효성을 보장한다 → **`attr_cltn()` 반환 즉시 순회할 것** (should).
- 컬렉션의 각 원소는 **속성 포인터**.

```cpp
sc_attr_cltn& c = sig.attr_cltn();
for (sc_attr_cltn::iterator i = c.begin(); i < c.end(); i++) {
    sc_attribute<int>* ap = dynamic_cast<sc_attribute<int>*>(*i);
    if (ap) std::cout << ap->name() << "=" << ap->value << std::endl;
}
```

---

## 5.21 `sc_hierarchy_scope`

**현재 계층 스코프 밖에** `sc_object`를 배치할 수 있게 하는 인터페이스.

```cpp
class sc_hierarchy_scope {
public:
    sc_hierarchy_scope( sc_hierarchy_scope&& );
    ~sc_hierarchy_scope() noexcept(false);
    static sc_hierarchy_scope get_root();
private:
    sc_hierarchy_scope( const sc_hierarchy_scope& ) = delete;
    sc_hierarchy_scope& operator=( const sc_hierarchy_scope& ) = delete;
    void* operator new  ( std::size_t ) = delete;
    void* operator new[]( std::size_t ) = delete;
};
```

복사 생성자·대입 연산자·`new` 연산자는 **삭제되어야** shall. 이동 생성자가 인자로 받은 계층 스코프를 설정한다 shall. `get_root()`는 object hierarchy의 **루트**를 반환 shall.

```cpp
SC_MODULE(TopModule) { SC_CTOR(TopModule) {} };

SC_MODULE(SomeModule) {
    sc_in<bool> SC_NAMED(in);
    SC_CTOR(SomeModule) {
        {
            sc_core::sc_hierarchy_scope scope( sc_core::sc_hierarchy_scope::get_root() );
            TopModule* mod = new TopModule("TopModule");   // top-level object가 됨
        }   // scope 소멸 시 원래 계층으로 복귀
    }
};
```
