# Clause 4 — Elaboration and simulation semantics

LRM pp. 34–60. **커널의 의미론. Clause 5–8의 모든 클래스 의미는 여기 위에 세워진다.** 스케줄러 동작을 모르면 SystemC 코드를 옳게 못 쓴다.

## 4.1 실행 전체 순서 (shall, 이 순서로)

```
a) Elaboration — 모듈 계층 생성
b) Elaboration — before_end_of_elaboration 콜백
c) Elaboration — end_of_elaboration 콜백
d) Simulation  — start_of_simulation 콜백
e) Simulation  — initialization phase
f) Simulation  — evaluation / update / delta notification / timed notification phase (반복)
g) Simulation  — end_of_simulation 콜백
h) Simulation  — 모듈 계층 파괴
```

## 4.2 Elaboration

### 4.2.2 인스턴스화 규칙

아래 클래스(및 파생 클래스)의 인스턴스는 **elaboration 중에만** 생성 가능하며, 모듈 계층 파괴 전에 삭제되면 안 된다 shall not:

`sc_module`(§5.2), `sc_port`(§5.12), `sc_export`(§5.13), `sc_prim_channel`(§5.15)

위치 제약 (위반 시 **error**):

| 클래스 | 생성 가능 위치 |
|---|---|
| `sc_module`, `sc_prim_channel` | 모듈 안, 또는 `sc_main`(및 그로부터 호출된 함수) 안, 또는 `sc_main`이 없으면 top-level module 형태 |
| `sc_port`, `sc_export` | **모듈 안에서만** |

- 구현은 top-level module 0개 또는 1개를 허용해야 shall하고, 여러 개를 허용해도 된다 may.
- 모듈 인스턴스화는 `sc_module_name`과 `sc_sensitive†` 객체 생성을 함의한다.
- **강력 권장**: module/port/export/primitive channel 인스턴스는 모듈의 데이터 멤버로 두거나, 주소를 데이터 멤버에 저장할 것. 그리고 데이터 멤버 이름과 인스턴스 문자열 이름을 일치시킬 것.

```cpp
#include <systemc>

struct Mod : sc_core::sc_module {
    SC_CTOR(Mod) {}
};

struct S {
    Mod m;                          // 비권장: 모듈이 아닌 struct 안의 모듈 인스턴스
    S(char* name_) : m(name_) {}
};

struct Top : sc_core::sc_module {   // Top 안에 Mod 인스턴스 5개가 존재
    Mod  m1;                        // 권장
    Mod *m2;                        // 권장
    S    s1;

    SC_CTOR(Top)
    : m1("m1"),                     // m1.name() == "top.m1"
      s1("s1")                      // s1.m.name() == "top.s1"
    {
        m2 = new Mod("m2");         // m2->name() == "top.m2"
        f();
        S *s2 = new S("s2");        // s2->m.name() == "top.s2"
    }

    void f() {
        Mod *m3 = new Mod("m3");    // 비권장. m3->name() == "top.m3"
    }
};

int sc_main(int argc, char* argv[]) {
    Top top("top");
    sc_core::sc_start();
    return 0;
}
```

> SystemC는 simulation 중 모듈 계층의 동적 생성·수정을 지원하지 않는다. 단 동적 **process**는 지원한다.

### 4.2.3 Process macros

`SC_METHOD` / `SC_THREAD` / `SC_CTHREAD` → unspawned process 생성. 인자는 `sc_module` 파생 클래스의 **멤버 함수 이름** shall.

- unspawned process: elaboration 중 또는 `end_of_elaboration` 콜백에서 생성 가능
- spawned process (`sc_spawn`): elaboration 중 또는 simulation 중 생성 가능

> 프로세스 매크로는 하위 호환과 하드웨어 합성용 clocked thread를 위해 존재. 같은 목적을 spawned process로도 달성 가능.

### 4.2.4 Port / export binding

| | port | export |
|---|---|---|
| 바인딩 대상 | channel, 다른 port, export | channel, 다른 export (**port 불가**) |
| 바인딩 방법 | 이름(`sc_port` 멤버 함수, §5.12.7) 또는 위치(`sc_module` 멤버 함수, §5.2.19) | **이름만** (§5.13.7) |
| 횟수 | port policy에 따라 0회 이상. multiport는 여러 번 가능 (§5.12.3) | **정확히 1회** shall |
| 완료 시점 | 구현이 지연 가능. `end_of_elaboration` 콜백 전까지 완료 shall | **즉시** shall |

- 한 port 인스턴스를 이름과 위치 **양쪽으로 바인딩하면 안 된다** shall not.
- 같은 port를 같은 channel에 두 번 바인딩하면 **error** (multiport라도).
- 전이 규칙: A→B, B→C 이면 A→C와 동일한 효과 shall. 표준이 "port A가 channel C에 바인딩됐다"고 할 때는 이 전이를 포함한다. export도 동일.
- port가 channel에 바인딩되면 커널이 channel의 `register_port`를 호출해야 shall. **export 바인딩에는 대응 호출이 없다** (§5.14).
- port가 바인딩된 channel은 모듈 계층 파괴 전에 삭제되면 안 된다 shall not.
- 바인딩은 **elaboration 중에만** 가능.

권장 배치 (should):
- port는 그 port를 가진 모듈 인스턴스의 **부모** 안에서 바인딩
- export는 그 export를 가진 **모듈 자신** 안에서 바인딩
- port는 같은 모듈 내 channel/port, 또는 자식 모듈의 export에 바인딩
- 자식 모듈의 port를 그 **같은 자식 모듈의 export**에 바인딩하는 것은 허용됨

의미: port는 interface가 정의한 서비스를 **요구**(requires), export는 **제공**(provides)한다.

### 4.2.5 시간 해상도

`sc_set_time_resolution`(§5.11.5)은 **elaboration 중에만** 호출 가능. **전역 설정만 존재** — local time resolution 개념 없음.

---

## 4.3 Simulation — 스케줄러

### 4.3.1 기본

- 스케줄러는 **event-driven**. event는 `sc_event` 객체로만 표현된다 (§5.10).
- simulation time은 **정수량**. 0에서 시작해 **단조 증가**. 물리적 의미는 time resolution이 결정.
- 프로세스가 실행되는 원인은 **다음 5가지뿐**:
  1. initialization phase에서 runnable이 됨 (§4.3.2.2)
  2. simulation 중 `sc_spawn` 호출
  3. 프로세스가 sensitive한 event의 발생
  4. time-out 발생
  5. `sc_process_handle`의 process control 멤버 함수 호출

- **sensitivity** = 프로세스를 재개·트리거할 수 있는 event와 time-out의 집합.
  - static sensitivity: unspawned는 elaboration 중 고정, spawned는 `sc_spawn` 호출 시 고정
  - dynamic sensitivity: 프로세스 자신이 시간에 따라 변경

### 4.3.2 스케줄링 알고리즘

네 개의 집합으로 기술된다:
- runnable processes
- update requests
- delta notifications and time-outs
- timed notifications and time-outs

> 한 process instance는 runnable 집합에 **두 번 나타날 수 없다** shall not. 이미 runnable인 것을 추가하려는 시도는 무시된다 shall.

발생 원인 (배타적, "and only from"):

| 항목 | 유일한 발생 원인 |
|---|---|
| update request | `sc_prim_channel::request_update` 또는 `async_request_update` (§5.15.6) |
| immediate notification | `sc_event::notify()` — **인자 없음** (§5.10.6) |
| delta notification | `sc_event::notify(t)` — **t == 0** |
| timed notification | `sc_event::notify(t)` — **t != 0** (t는 호출 시점 기준 상대 시간) |
| time-out | `wait` / `next_trigger` 특정 호출. 0이면 delta 집합, 0이 아니면 timed 집합 (§5.2.17, §5.2.18) |

#### 4.3.2.2 Initialization phase (이 순서로)

```
a) update phase 실행 (단, delta notification phase로 넘어가지 않음)
b) object hierarchy의 모든 method/thread process instance를 runnable에 추가
   - 단 dont_initialize()가 호출된 것은 제외
   - 단 clocked thread process는 제외
c) delta notification phase 실행 → 끝나면 evaluation phase로
```

> update/delta notification phase가 필요한 이유: elaboration 중에 update request가 만들어질 수 있음 (예: `sc_inout::initialize`).

#### 4.3.2.3 Evaluation phase

runnable 집합에서 프로세스 하나를 **고르고, 집합에서 제거한 뒤에야** 실행을 트리거·재개한다. 실행은 **중단 없이(without interruption)** 진행되어 반환하거나, thread/clocked thread의 경우 `wait` 호출 또는 자기 자신의 handle에 대한 `suspend` 호출 지점까지 간다.

- **co-routine semantics (협력적 멀티태스킹)**: 동시에 오직 한 프로세스만 실행. 프로세스는 다른 프로세스를 선점·중단하지 못한다 shall not.
- 선택 순서는 **구현 정의**. 단 같은 구현·같은 버전·같은 애플리케이션·같은 입력이면 **실행 순서는 run 간 변하지 않아야** shall.
- 스케줄러는 **비선점**. method process는 전체가 중단 없이, thread process는 두 `wait` 사이가 중단 없이 실행된다고 가정 가능.
- **공유 저장소 접근은 명시적으로 동기화할 것** (should) — 실행 순서가 애플리케이션 제어 밖이므로 비결정성 발생.

evaluation phase 중 프로세스가 할 수 있는 일:
- **immediate notification**: 그 event에 **현재** sensitive한 모든 프로세스가 runnable에 추가되고 **현재 evaluation phase에서 실행**된다 shall. 단 **자기 자신은 추가되지 않는다** shall not.
  - "현재 sensitive"만 대상. 같은 evaluation phase 안에서 **나중에** 그 event에 dynamic sensitive해지는 프로세스는 제외 (static sensitive면 포함).
- `sc_spawn`: 새 process instance가 runnable에 추가되고 **바로 이 evaluation phase에서** 실행됨 (`sc_spawn_options::dont_initialize` 제외).
- `request_update`/`async_request_update`: 해당 channel의 `update`가 **바로 다음 update phase**에 콜백됨.
- process control 함수: 현재 evaluation phase에서 runnable로 만들거나 runnable에서 제거 가능.

runnable 집합이 빌 때까지 반복 → update phase로.

#### 4.3.2.4 Update phase

직전 evaluation phase(또는 initialization phase의 경우 elaboration 중)의 `request_update` 및 `async_request_update`로 인한 모든 `update` 호출을 실행. **각 primitive channel 인스턴스당 update phase마다 `update`는 최대 1회** 호출된다 shall.

남은 update 호출이 없으면 delta notification phase로 (initialization phase에서 실행된 경우는 제외).

#### 4.3.2.5 Delta notification phase

1. 하나 이상의 프로세스가 suspension을 요청했고, 어떤 프로세스도 unsuspendable을 선언하지 않았다면 → 스케줄러 실행 중단, simulation suspend (§4.3.3).
2. 그렇지 않고 pending delta notification/time-out이 있으면 (직전 evaluation 또는 update phase의 `notify`/`wait`에서만 발생 가능):
   - a) 그 event/time-out에 sensitive한 process instance를 판정
   - b) 전부 runnable에 추가
   - c) 해당 notification/time-out을 delta 집합에서 제거

끝에서 runnable 집합이 비어있지 않으면 → evaluation phase로 되돌아감.

#### 4.3.2.6 Timed notification phase

pending timed notification/time-out이 있으면:
- a) **simulation time을 가장 이른 pending timed notification/time-out 시각으로 전진**
- b) 정확히 그 시각에 notify되는 event / lapse되는 time-out에 sensitive한 프로세스 판정
- c) 전부 runnable에 추가
- d) 해당 notification/time-out을 timed 집합에서 제거

pending이 없으면:
- `async_attach_suspending`으로 attach한 primitive channel이 하나라도 있으면 → 스케줄러 중단, simulation suspend (§4.3.3)
- 아니면 → **simulation 종료**, 스케줄러 exit

끝에서 runnable이 비어있지 않으면 → evaluation phase로.

### 4.3.3 delta cycle, pause, suspend

**delta cycle** = evaluation phase → update phase → delta notification phase (이 순서). **initialization phase는 delta cycle을 포함하지 않는다.**

- elaboration 중/initialization phase 전에 만들어진 update request는 initialization phase의 update phase에서 실행되도록 스케줄된다 shall.
- 같은 시점에 만들어진 delta notification은 initialization phase의 delta notification phase에서 발생하도록 스케줄된다 shall.
- 스케줄러는 **온전한 delta cycle 단위로** 실행하고, delta notification phase와 evaluation phase의 **경계에서** 실행을 멈출 수 있다. 이 경계 외에서 멈출 수 있는 경우는 오직: `sc_stop` 호출 후, 예외가 던져질 때, report handler가 simulation을 중단/abort할 때(§8.3).
- `sc_pause` 호출 시: delta notification phase 끝에서 중단, 재개 시 다음 evaluation phase 시작에서 재개 shall.

**paused 상태에서 만든 것의 처리 (shall)**:

| 만든 것 | 재개 후 처리 |
|---|---|
| update request | 재개 후 **첫 update phase**에서 실행 |
| immediate notification | 재개 후 **첫 evaluation phase**에서 sensitive 프로세스를 runnable로 |
| delta notification | 재개 후 **두 번째 evaluation phase**에서 runnable로 |

- **simulation이 suspend된 경우** (`sc_suspend_all` 또는 channel이 attach한 상태에서 timed notification 고갈): 재개 방법은 **외부 OS 스레드로부터의 `async_request_update` 호출뿐이다**.
- initialization phase 전 또는 paused 중에 만든 update request는 **어떤 process instance와도 연관되지 않는다** shall not — 예컨대 `sc_signal`의 writer policy 판정에서 제외.

**세 개의 인과 루프** (NOTE, informative):
- immediate notification 루프: 단일 evaluation phase 안에 갇힘. **비결정적** (프로세스 실행과 notification이 인터리브되고 순서 미정의)
- delta notification 루프: eval → update → delta notify → eval. delta cycle 1 진행. **결정적**
- timed notification 루프: eval → update → delta notify → timed notify → eval. 시간 전진. **결정적**

> 프로세스 간 통신을 결정적 primitive channel로만 한정하면, evaluation phase 내 실행 순서와 무관하게 애플리케이션 동작이 결정적이 된다.

---

## 4.4 Elaboration/simulation 구동

구현은 다음 둘 중 **하나 이상**을 제공해야 shall:
- 애플리케이션 제어: `sc_main` + `sc_start`
- 커널 제어 (§4.4.6, 메커니즘은 구현 정의)

```cpp
namespace sc_core {
    int sc_elab_and_sim( int argc, char* argv[] );
    int sc_argc();
    const char* const* sc_argv();

    enum sc_starvation_policy { SC_RUN_TO_TIME, SC_EXIT_ON_STARVATION };

    void sc_start();
    void sc_start( const sc_time&, sc_starvation_policy p = SC_RUN_TO_TIME );
    void sc_start( double, sc_time_unit, sc_starvation_policy p = SC_RUN_TO_TIME );
}
```

### 4.4.3 `sc_elab_and_sim`
`main`을 애플리케이션이 제공하면 `main`은 `sc_elab_and_sim`을 호출해야 shall (SystemC 구현으로의 진입점). 반환값 0 = 정상 완료.

### 4.4.4 `sc_argc` / `sc_argv`
`sc_argv`는 `main`/`sc_elab_and_sim`에 전달된 인자의 **복사본**을 반환 shall (수정 가능). 마지막 포인터는 null 포인터.

### 4.4.5 `sc_main` / `sc_start`

```cpp
int sc_main( int argc, char* argv[] );   // 전역 네임스페이스에 애플리케이션이 제공 shall
```

- **elaboration = `sc_main` 시작부터 첫 `sc_start` 호출 직전까지.**
- `sc_main`은 커널로부터 **한 번** 호출되며 애플리케이션으로의 유일한 진입점.
- 반환 0 = 정상 완료.
- **`sc_start`는 `sc_main`에서만 호출 가능** shall (간접 호출 포함).

`sc_start` 동작:

| 호출 형태 | 동작 |
|---|---|
| 최초 호출 | initialization phase 실행 후 첫 evaluation phase |
| 2회차 이후 | 직전 `sc_start` 종료 시각에서 재개. 새 delta cycle의 evaluation phase부터 |
| 시간 인자 있음 | 종료 시각(현재 시각 + 인자) 이하의 마지막 timed notification phase까지 실행 |
| 인자 없음 | 남은 활동이 없을 때까지 (네 집합이 모두 비고 suspension 요청 없음). 반환 시 시간 = 가장 최근 notification/time-out 시각 |
| 시간 인자 == 0 | **delta cycle 1회** (eval→update→delta notify). starvation policy 무시, 시간 전진 없음. 최초 호출이면 앞에 initialization phase |

starvation policy (반환 시 simulation time):
- `SC_RUN_TO_TIME` (기본): **end time으로 설정** shall — 마지막 notification 시각과 무관하게
- `SC_EXIT_ON_STARVATION`: **가장 최근 notification/time-out 시각으로 설정** shall (end time보다 작을 수 있음)

- 0 인자 호출 시 `sc_pending_activity_at_current_time() == false`이면 구현은 **warning**을 내고 `sc_delta_count`를 증가시키지 않는다 shall.
- 첫 `sc_start` 전: update request / timed notification / delta notification은 생성 가능. **immediate notification은 생성 불가** shall not. `sc_start` 호출 사이·이후에는 immediate 포함 전부 가능.
- `sc_pause`가 호출됐으면 `sc_start` 재호출 가능. **`sc_stop` 후에는 `sc_start` 재호출 불가** shall not (error, §4.6.4).
- 둘 다 호출되지 않았으면 구현이 반환 전에 **암묵적 `sc_pause`**를 삽입하고, `sc_start` 재호출 가능.
- **권장(should)**: `sc_main` 반환 전에 `sc_stop`을 호출해 `end_of_simulation` 콜백이 불리게 할 것.
- 음수 부동소수 시간 인자로 `sc_start` 호출은 **error**. 이 경우 시간은 `SC_ZERO_TIME`으로 설정된다.

```cpp
int sc_main( int argc, char* argv[] ) {
    using namespace sc_core;
    Top top("top");                 // 모듈 계층 인스턴스화

    sc_start(100.0, SC_NS);         // 정확히 100 ns 실행
    sc_start();                     // 활동이 없을 때까지 실행

    if (sc_get_status() == SC_PAUSED) {
        SC_REPORT_INFO("", "sc_stop called to terminate a paused simulation");
        sc_stop();
    }
    return 0;
}
```

---

## 4.5 Elaboration / simulation 콜백

```cpp
virtual void before_end_of_elaboration();
virtual void end_of_elaboration();
virtual void start_of_simulation();
virtual void end_of_simulation();
```

구현은 이 넷을 `sc_module`, `sc_port`, `sc_export`, `sc_prim_channel`의 멤버 함수로 **빈 본문**으로 정의해야 shall. 애플리케이션은 이들 파생 클래스에서 오버라이드 가능 may.

- 각 카테고리 내에서 객체 간 콜백 순서는 **구현 정의**.
- 미리 정의된 클래스의 콜백을 오버라이드할 때 base class 함수를 호출하는지 여부에 따라 동작이 달라진다 (예: `sc_in::end_of_elaboration`).

```cpp
namespace sc_core {
    bool sc_start_of_simulation_invoked();   // 모든 start_of_simulation 콜백 완료 후에만 true
    bool sc_end_of_simulation_invoked();     // 모든 end_of_simulation 콜백 완료 후에만 true
}
```

### 콜백별 허용/금지 매트릭스

`•` = 허용(may), `✗` = 금지(shall not), `–` = 언급 없음/해당 없음

| 동작 | before_eoe | end_of_elab | start_of_sim | end_of_sim | stage_callback |
|---|:--:|:--:|:--:|:--:|:--:|
| `sc_module`/`sc_port`/`sc_export`/`sc_prim_channel` 인스턴스화 | • | ✗ | ✗ | (미보장) | ✗ |
| 기타 `sc_object` 파생 인스턴스화 | • | • | • | (미보장) | – |
| port binding / export binding | • | ✗ | ✗ | – | ✗ |
| `SC_METHOD` | • | • | ✗ | – | ✗ |
| `SC_THREAD` | • | • | ✗ | – | ✗ |
| `SC_CTHREAD` | • | ✗ | ✗ | – | ✗ |
| `SC_CTOR` | (중첩 모듈 내부에서만) | ✗ | ✗ | – | ✗ |
| `sensitive`, `dont_initialize`, `set_stack_size` | • | • | ✗ | – | – |
| `reset_signal_is`, `async_reset_signal_is` (`sc_module`) | • | ✗ | ✗ | – | ✗ |
| event finder 함수 호출 | • | ✗ | ✗ | – | ✗ |
| `sc_spawn` | • (static, spawned) | • (dynamic) | • (dynamic) | (미보장) | ✗ |
| `sc_spawn_options::set_sensitivity` (event/interface/event finder) | • (port는 불가) | – | – | – | – |
| `sc_spawn_options::reset_signal_is` / `async_reset_signal_is` | • | – | – | – | – |
| `request_update` / `async_request_update` | • | • | • | (미보장) | ✗ |
| `notify(const sc_time&)`, `notify(double, sc_time_unit)` | • | • | • | (미보장) | ✗ |
| `notify()` — **immediate** | ✗ | ✗ | ✗ | (미보장) | ✗ |
| process control: `suspend` `resume` `disable` `enable` `sync_reset_on` `sync_reset_off` | • | • | • | – | ✗ |
| process control: `kill` `reset` `throw_it` | ✗ | ✗ | ✗ | – | ✗ |
| `sc_port::operator->` / `operator[]` (IMC) | **should not** (바인딩 미완료 가능 → null) | • (단 event notify 등 금지 동작 제외) | • (동일) | – | – |

### 4.5.2 `before_end_of_elaboration`
모듈 계층 생성 완료 **후**에 호출 shall. 이 콜백에서 **모듈 계층을 더 확장할 수 있다** — 계층 깊숙한 event를 감시할 top-level 모듈 인스턴스화 등.

- `operator->`/`operator[]`는 **호출하지 말 것 should not** — 바인딩이 미완료라 null 포인터 반환 가능. `size()`도 최종값보다 작을 수 있음.
- 이 콜백에서 만든 `sc_object`는 **콜백이 속한 모듈의 생성자에서 만든 것처럼** 계층에 배치된다 shall. 콜백이 port/export/primitive channel의 것이면 부모 모듈에.
- 여기서 만든 객체가 다시 네 콜백을 오버라이드할 수 있고, 구현은 그 **중첩 콜백을 전부 수행해야** shall.

### 4.5.3 `end_of_elaboration`
elaboration의 맨 끝, 모든 `before_end_of_elaboration`과 그로 인한 인스턴스화·바인딩 완료 후, simulation 시작 전 호출 shall.
목적: **계층을 수정하지 않는** 정리 작업 — design rule checking, port 바인딩 횟수 의존 동작, 진단 메시지.

### 4.5.4 `start_of_simulation`
`sc_start` **최초 호출 즉시** (또는 커널 제어 시 simulation 맨 앞) 호출 shall. `sc_start`를 여러 번 불러도 **첫 호출에서만** 콜백 shall. `end_of_elaboration` 뒤, initialization phase 앞.
목적: 자극/응답 파일 열기, 진단 출력.

### 4.5.5 `end_of_simulation`
`sc_stop`으로 스케줄러가 멈춘 시점(§4.6.4), 또는 커널 제어 시 simulation 맨 끝에 호출 shall. **`sc_stop`을 여러 번 불러도 콜백은 한 번만** shall.
모듈 계층 파괴 **전에** 호출된다 shall. 소멸자 안에서 `sc_end_of_simulation_invoked()`로 콜백 여부 확인 가능.

> 이 콜백과 모듈 계층 객체의 **소멸자** 안에서는 `sc_object` 인스턴스화, `sc_spawn`, `request_update`/`async_request_update`, `sc_event::notify`가 **지원 보장되지 않는다**. 에러 발생 여부는 구현 정의.

---

## 4.6 스케줄러 관련 기타 함수

```cpp
namespace sc_core {
    void sc_pause();

    enum sc_stop_mode { SC_STOP_FINISH_DELTA, SC_STOP_IMMEDIATE };
    extern void sc_set_stop_mode( sc_stop_mode mode );
    extern sc_stop_mode sc_get_stop_mode();
    void sc_stop();

    const sc_time& sc_time_stamp();
    sc_dt::uint64 sc_delta_count();
    sc_dt::uint64 sc_delta_count_at_current_time();
    bool sc_is_running();
    bool sc_pending_activity_at_current_time();
    bool sc_pending_activity_at_future_time();
    bool sc_pending_activity();
    sc_time sc_time_to_pending_activity();

    enum sc_status {
        SC_ELABORATION, SC_BEFORE_END_OF_ELABORATION, SC_END_OF_ELABORATION,
        SC_START_OF_SIMULATION, SC_RUNNING, SC_PAUSED, SC_SUSPENDED,
        SC_STOPPED, SC_END_OF_SIMULATION
    };
    sc_status sc_get_status();

    enum sc_stage {
        SC_POST_BEFORE_END_OF_ELABORATION, SC_POST_END_OF_ELABORATION,
        SC_POST_START_OF_SIMULATION, SC_POST_UPDATE, SC_PRE_TIMESTEP,
        SC_PRE_PAUSE, SC_PRE_SUSPEND, SC_POST_SUSPEND, SC_PRE_STOP,
        SC_POST_END_OF_SIMULATION
    };
    void sc_register_stage_callback( const sc_stage_callback_if&, int );
    void sc_unregister_stage_callback( const sc_stage_callback_if&, int );
}
```

`sc_status`의 실제 정수값은 구현 정의이며, **비트 조합이 가능하도록** 선택되어야 shall.

### 4.6.2 `sc_pause`
현재 delta cycle 끝에서 스케줄러 실행 중단 shall — 나중에 재개 가능하도록. `sc_start`로 진입했다면 `sc_start`에서 제어를 반환.
**비블로킹** shall — 호출한 함수는 yield하거나 return할 때까지 계속 실행됨.
evaluation/update phase에서 호출되면 현재 delta cycle(eval+update+delta notify)을 **완료한 뒤** 중단 shall.
elaboration 중, `sc_main`에서, 또는 네 콜백에서 호출하면 **효과 없이 무시**된다 shall. `sc_stop`이 이미 호출됐으면 효과 없음.

paused 중 **허용**:
- simulation 중 인스턴스화가 허용되는 타입의 `sc_object` 인스턴스화
- `sc_spawn`
- `request_update` / `async_request_update`
- `sc_event::notify` — immediate/delta/timed 전부
- process control: `suspend` `resume` `disable` `enable` `sync_reset_on` `sync_reset_off`
- `sc_port::operator->` / `operator[]` (금지 동작을 수행하지 않는 한)
- `sc_stop`

paused 중 **error**:
- `sc_module`/`sc_port`/`sc_export`/`sc_prim_channel` 인스턴스화
- port/export 바인딩
- `SC_METHOD` / `SC_THREAD` / `SC_CTHREAD`
- `sensitive`로 static sensitivity 생성
- `dont_initialize` `set_stack_size` `reset_signal_is` `async_reset_signal_is`
- event finder 함수 호출
- process control: `kill` `reset` `throw_it`
- `wait` / `next_trigger` (멤버/비멤버 전부)

### 4.6.3 `sc_suspend_all` / `sc_unsuspend_all` / `sc_unsuspendable` / `sc_suspendable`

```cpp
void sc_suspend_all();
void sc_unsuspend_all();
void sc_unsuspendable();
void sc_suspendable();
```

- `sc_suspend_all`: 모든 프로세스 suspend 요청 (thread `suspend`와 같은 의미론). **원자적** — 커널은 전부를 함께만 suspend한다. 전부 suspend되면 시뮬레이터 자체가 suspended.
- 프로세스는 다음 delta notification phase 전에 `sc_unsuspend_all`로 요청 취소 가능. **하나 이상이 요청했을 때만** suspend 발생.
- `sc_unsuspendable`로 opt-out, `sc_suspendable`로 opt-out 취소. **하나라도 opt-out했으면 `sc_suspend_all`은 효과 없음.**
- 연속 중복 호출은 무시 (`sc_suspend_all` 연속, `sc_unsuspendable` 연속).
- 유스케이스: 외부 시뮬레이션과 시간 동기화. 커널은 외부의 `async_request_update` 호출로 재개.

### 4.6.4 `sc_stop` / stop mode

호출 가능 위치: elaboration/simulation 콜백, 프로세스, `sc_prim_channel::update`, `sc_main`. 구현은 `sc_report_handler::report`에서 호출 가능.

- `sc_set_stop_mode`는 elaboration 중 또는 `before_end_of_elaboration`/`end_of_elaboration`/`start_of_simulation`에서 호출 가능. 여러 번이면 **가장 최근 것이 우선**. **initialization phase 이후 호출은 error.**
- 기본값 = `SC_STOP_FINISH_DELTA`.

| 호출 위치 | 중단 시점 |
|---|---|
| 네 콜백 중 하나 | 현재 콜백 phase 완료 후 (해당 종류의 모든 콜백 완료 후) |
| evaluation/update phase, `SC_STOP_FINISH_DELTA` | 현재 evaluation phase와 update phase를 모두 완료 후 (delta notification phase 전) |
| evaluation phase, `SC_STOP_IMMEDIATE` | 현재 프로세스 실행만 완료, 이후 프로세스 실행 없이·update phase 없이 중단 |
| update phase, `SC_STOP_IMMEDIATE` | update phase 완료 후 중단 |

어떤 stop mode든 **현재 실행 중인 프로세스가 커널에 제어를 넘길 때까지는** 중단되지 않는다 shall.

- 구현은 `sc_stop`에서 중단 메시지를 표준 출력에 인쇄해야 shall. `end_of_simulation` 콜백 수행 shall.
- **`sc_stop` 후 `sc_start` 호출은 error** shall.
- `sc_stop` 두 번째 호출 시 구현은 **warning**을 낸다 shall. `sc_report_handler::stop_after`로 warning 시 `sc_stop`이 불리게 설정돼 있어도 구현은 이를 무시해 무한 재귀를 피한다 shall.
- **`sc_stop`은 `sc_pause`보다 우선한다** shall.
- `sc_start`를 제공하지 않는 구현이라도 `sc_stop`은 제공해야 shall.

### 4.6.5 `sc_time_stamp`
현재 simulation time 반환 shall. **elaboration과 initialization 중에는 0** 반환 shall. simulation time은 스케줄러만 수정 가능.

### 4.6.6 `sc_delta_count` / `sc_delta_count_at_current_time`
- `sc_delta_count`: **프로세스가 하나 이상 트리거·재개된 delta cycle마다 정확히 1회 증가**. runnable 프로세스가 없던 delta cycle은 세지 않음. elaboration/initialization/첫 evaluation phase 동안 0. 첫 온전한 delta cycle의 evaluation phase 이후 처음 증가. 각 delta cycle의 evaluation phase와 update phase **사이**에 증가. `uint64` 최대치 도달 시 0으로 순환.
- runnable 프로세스 없는 delta cycle: `sc_start(SC_ZERO_TIME)` 호출 시, `sc_pause` 후 재개 시.
- `sc_pause` 후 재개 시에도 delta count는 `sc_pause`가 없었던 것처럼 계속 증가 shall.
- `sc_delta_count_at_current_time`: 동일하나 **시간이 전진할 때마다 리셋**됨.

```cpp
// 두 연속 delta cycle에서 실행됐는지 검사 (LRM NOTE)
if (sc_delta_count() == stored_delta_count + 1) { /* consecutive */ }
stored_delta_count = sc_delta_count();
```

### 4.6.7 `sc_is_running`
스케줄러가 running / paused / suspended (initialization phase 포함)일 때 `true` shall.
elaboration 중, `start_of_simulation`·`end_of_simulation` 콜백 중, `sc_stop` 후 `sc_start` 반환 시 `false` shall.

```cpp
sc_assert( sc_is_running() == (sc_get_status() & (SC_RUNNING | SC_PAUSED | SC_SUSPENDED)) );
```

### 4.6.8 Pending activity 검출

| 함수 | true 조건 |
|---|---|
| `sc_pending_activity_at_current_time()` | runnable 집합 ≠ ∅, **또는** update request 집합 ≠ ∅, **또는** delta notification/time-out 집합 ≠ ∅ |
| `sc_pending_activity_at_future_time()` | timed notification/time-out 집합 ≠ ∅ |
| `sc_pending_activity()` | 위 둘 중 하나라도 true |

`sc_time_to_pending_activity()` 반환값:
- `at_current_time()==true` → `SC_ZERO_TIME`
- 아니고 `at_future_time()==true` → `T - sc_time_stamp()` (T = 가장 이른 timed notification/time-out 시각)
- 아니면 → `sc_max_time() - sc_time_stamp()`

넷 다 elaboration/simulation 중 또는 이후 언제든 호출 가능.

> **주의**: `sc_pending_activity`는 elaboration 끝에서 `false`를 반환할 수 있다. 이 함수들로 `sc_start` 루프 조건을 만들 때 주의할 것 (should).

```cpp
int sc_main( int argc, char* argv[] ) {
    using namespace sc_core;
    // top-level 모듈 인스턴스화 ...

    sc_start( SC_ZERO_TIME );                        // initialization phase를 돌려 pending activity 생성

    while( sc_pending_activity() ) {
        sc_start( sc_time_to_pending_activity() );   // 다음 활동까지 실행
    }
    return 0;
}
```

### 4.6.9 `sc_get_status`

| 값 | 반환되는 시점 |
|---|---|
| `SC_ELABORATION` | 모듈 계층 생성 중, 또는 `sc_start` 사용 시 첫 `sc_start` 호출 전 |
| `SC_BEFORE_END_OF_ELABORATION` | `before_end_of_elaboration` 콜백에서 |
| `SC_END_OF_ELABORATION` | `end_of_elaboration` 콜백에서 |
| `SC_START_OF_SIMULATION` | `start_of_simulation` 콜백에서 |
| `SC_RUNNING` | initialization / evaluation / update phase에서 |
| `SC_PAUSED` | 스케줄러 미실행 + `sc_pause` 호출됨 |
| `SC_SUSPENDED` | 스케줄러 미실행 + suspend됨 (명시적 요청, 또는 pending notification/time-out 고갈 + suspension attach된 primitive channel 존재) |
| `SC_STOPPED` | 스케줄러 미실행 + `sc_stop` 호출됨 |
| `SC_END_OF_SIMULATION` | `end_of_simulation` 콜백에서 |

- **thread-safe** shall — SystemC 커널/thread process가 아닌 다른 OS 스레드에서도 안전하게 호출 가능.
- `before_end_of_elaboration`에서 (간접적으로 호출된 생성자 안이라도) `SC_BEFORE_END_OF_ELABORATION` 반환 shall → §4.4의 계층 생성과 §4.5.2의 확장 생성을 구분 가능.
- `sc_pause`와 `sc_stop`이 **둘 다** 호출됐으면 `SC_STOPPED` 반환 shall.
- `end_of_simulation`에서는 `sc_pause`/`sc_stop` 여부와 무관하게 `SC_END_OF_SIMULATION` shall.

상태 전이 (Figure 1):
```
SC_ELABORATION
  └─ sc_start ─▶ SC_BEFORE_END_OF_ELABORATION ─▶ SC_END_OF_ELABORATION ─▶ SC_START_OF_SIMULATION
                                                                                │
        sc_pause ◀───────────────── SC_RUNNING ─────────────────▶ sc_suspend_all
        Starvation(채널 attach 없음)      │      Starvation(채널 attach 있음)
   SC_PAUSED ──sc_start──▶ SC_RUNNING ◀──async_request_update── SC_SUSPENDED
                            │ sc_stop
                            ▼
                  SC_END_OF_SIMULATION ─▶ SC_STOPPED
```

### 4.6.10 `sc_stage_callback_if`

애플리케이션이 접근할 수 없던 시뮬레이션 단계에서 사용자 콜백을 받기 위한 인터페이스.

```cpp
namespace sc_core {
    class sc_stage_callback_if {
    public:
        virtual ~sc_stage_callback_if() {}
        virtual void stage_callback(const sc_stage & stage) = 0;
    };
}
```

`stage_callback` 안에서 **금지**(shall not, 직접/간접):
- `sc_module`/`sc_port`/`sc_export`/`sc_prim_channel` 인스턴스화
- port/export 바인딩
- `SC_CTOR` `SC_METHOD` `SC_THREAD` `SC_CTHREAD`
- `reset_signal_is` / `async_reset_signal_is`
- event finder 함수 호출
- `sc_event::notify` — **인자 유무 무관, immediate/delta/timed 전부**
- `sc_spawn`
- `request_update` / `async_request_update`
- process control 전부: `kill` `reset` `throw_it` `suspend` `resume` `disable` `enable` `sync_reset_on` `sync_reset_off`

`sc_get_status` 호출은 가능하나 **반환값은 구현 정의**.

### 4.6.11–4.6.13 등록과 실행 시점

```cpp
void sc_register_stage_callback( const sc_stage_callback_if&, int );
void sc_unregister_stage_callback( const sc_stage_callback_if&, int );
```

두 번째 인자 `sc_stage`는 **비트 연산 조합**을 지원해야 shall. 등록 없이 unregister 시 구현이 warning을 낼 수 있으나 의무는 아님.

| `sc_stage` | 실행 시점 |
|---|---|
| `SC_POST_BEFORE_END_OF_ELABORATION` | 모든 `before_end_of_elaboration` 콜백 실행 후 |
| `SC_POST_END_OF_ELABORATION` | 모든 `end_of_elaboration` 콜백 실행 후 |
| `SC_POST_START_OF_SIMULATION` | 모든 `start_of_simulation` 콜백 실행 후 |
| `SC_POST_UPDATE` | update phase 끝 ~ delta notification phase 시작 사이 |
| `SC_PRE_TIMESTEP` | timed notification phase 중, 시뮬레이터가 시간을 전진시키기 **직전** |
| `SC_PRE_PAUSE` | `sc_pause` 때문에 `sc_start`에서 제어를 반환하기 직전 |
| `SC_PRE_SUSPEND` | suspended 상태로 진입하기 직전 |
| `SC_POST_SUSPEND` | suspended 상태에서 빠져나가기 직전 |
| `SC_PRE_STOP` | `sc_stop` 때문에 `sc_start`에서 제어를 반환하기 직전 |
| `SC_POST_END_OF_SIMULATION` | 모든 `end_of_simulation` 콜백 실행 후 |
