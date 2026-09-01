# Clause 3 — Terminology and conventions

LRM pp. 27–33. **정확한 해석에 필수.** 코드 작성 시 "무엇이 module인가/무엇이 channel인가"를 여기 정의대로 판단한다.

## 3.1.2 implementation vs application

- **implementation** = SystemC/TLM-1/TLM-2.0 클래스 라이브러리 구현체 (시뮬레이터 벤더). public interface만 노출하면 됨.
- **application** = 그 라이브러리를 쓰는 최종 사용자 C++ 프로그램. **당신이 쓰는 모델 코드가 application이다.**

→ LRM 문장에서 "The implementation shall ..."은 시뮬레이터의 의무, "An application shall ..."은 당신 코드의 의무. 코드 리뷰 시 후자만 강제 대상.

## 3.1.3 call / called from / derived from

- **call** = 직접 호출 **또는** 간접 호출 (중간 함수 체인 무제한).
- **called from** = 직접/간접 모두.
- **derived from** = 명시적 한정이 없으면 직접/간접 파생 모두.

→ "shall not be called from `end_of_elaboration`"은 그 안에서 호출한 헬퍼 함수 깊숙한 곳도 포함. 위반 탐지 시 콜 그래프 전체를 봐야 함.

## 3.1.4 핵심 용어 정의 (엄밀하게)

| 용어 | 정의 |
|---|---|
| **module** | `sc_module`에서 파생된 클래스 |
| **port** | `sc_port`에서 파생된 클래스 또는 `sc_port` 객체 |
| **export** | `sc_export` 객체 |
| **interface** | `sc_interface`에서 파생된 클래스 |
| **interface proper** | `sc_interface`에서 파생됐지만 `sc_object`에서는 파생되지 않은 **추상** 클래스 |
| **primitive channel** | 하나 이상의 interface + `sc_prim_channel`에서 파생된 **비추상** 클래스 |
| **hierarchical channel** | 하나 이상의 interface + `sc_module`에서 파생된 **비추상** 클래스 |
| **channel** | 하나 이상의 interface에서 파생된 비추상 클래스. primitive 또는 hierarchical. 둘 다 아니면 `sc_object` 파생을 강력 권장 |
| **event** | `sc_event` 객체 |
| **signal** | `sc_signal` 객체 |
| **process instance** | `SC_METHOD`/`SC_THREAD`/`SC_CTHREAD` 또는 `sc_spawn`으로 생성된, `sc_object` 파생 구현정의 클래스의 객체 |
| **process** | process instance 또는 그와 연관된 멤버 함수. 문맥으로 구분 |
| **process handle** | `sc_process_handle` 객체 |

### static vs dynamic, spawned vs unspawned — 직교 개념

| | 생성 시점 기준 | 생성 방법 기준 |
|---|---|---|
| **static process** | 모듈 계층 생성 중 또는 `before_end_of_elaboration`에서 생성 | — |
| **dynamic process** | `end_of_elaboration`에서 또는 simulation 중 생성 | — |
| **unspawned process** | — | `SC_METHOD`/`SC_THREAD`/`SC_CTHREAD` 매크로로 생성 |
| **spawned process** | — | `sc_spawn` 호출로 생성 |

교차 경우가 실제로 존재:
- unspawned인데 dynamic: `end_of_elaboration`에서 `SC_METHOD` 호출
- spawned인데 static: elaboration 종료 전에 `sc_spawn` 호출

### 계층 (hierarchy)

- **module hierarchy** = elaboration 중 생성된 module instance 전체 집합. (넓게는 그 안에 인스턴스화된 모든 객체 포함)
- **object hierarchy** = `sc_object` 전체 집합. module hierarchy는 그 부분집합. object hierarchy에는 동적 process instance도 포함(§5.16).
  - **event는 object hierarchy에 속하지 않는다.** 단 계층적 이름은 가질 수 있음.
- **within module M** = 인스턴스의 생성자가 M의 생성자에서 (명시적/암묵적) 호출되고, M 안의 다른 module instance 안에 있지 않은 경우.
- **child of M** = M 안에 있는 인스턴스. **parent** = 그 반대. **top-level module** = 어떤 모듈 안에도 인스턴스화되지 않은 모듈.

### 시간 표현 관용구 — 정확히 어느 구간인가

| 표현 | 포함 범위 |
|---|---|
| **during elaboration** | 모듈 계층 생성 + `before_end_of_elaboration` 콜백 |
| **during simulation** | initialization / evaluation / update phase + 시뮬레이션이 paused인 기간 |
| **during elaboration or simulation** | 모듈 계층 생성부터 마지막 delta cycle까지 전부. 마지막 delta cycle 이후는 포함도 배제도 아님 |
| **after elaboration** | 모든 `end_of_elaboration` 콜백이 실행 완료된 시점 |

> **주의**: "during elaboration"에 허용된 동작이라도 `end_of_elaboration` 콜백에서는 명시적으로 금지된 것이 다수 있다. 콜백별 허용/금지는 §4.5 (→ [ch04](ch04-elaboration-simulation.md)) 표를 볼 것. "during elaboration"만 보고 판단하면 안 됨.

## 3.2 문법 규약 (LRM 읽는 법)

| 표기 | 의미 |
|---|---|
| *implementation-defined* (이탤릭) | C++ 정의 일부가 표준에서 생략됨. 구현이 의미론을 지키는 정의를 제공 |
| *disabled* (이탤릭) | 애플리케이션이 호출할 수 없도록 구현이 비활성화해야 하는 멤버 함수 그룹. 보통 기본 생성자·복사 생성자·대입 연산자 |
| `...` (점 셋) | 무관하거나 반복되는 코드 부분 생략 |
| *클래스명*† (단검) | **애플리케이션이 명시적으로 사용하면 안 되는 클래스명.** 그 클래스의 객체를 생성해서도 안 됨 (shall not). 구현이 다른 이름으로 대체 가능 |
| **볼드체** | 가독성용. SystemC 의미 없음. 매크로/상수/enum 리터럴 대문자 이름에는 안 씀 |

### † 클래스 목록 (직접 쓰지 말 것)

```
sc_bind_proxy†        sc_concatref†         sc_concref†           sc_concref_r†
sc_context_begin†     sc_event_and_expr†    sc_event_or_expr†     sc_bitref†
sc_bitref_r†          sc_subref†            sc_subref_r†          sc_switch†
sc_sensitive†         sc_value_base†        sc_vector_iter†
sc_fxnum_bitref†      sc_fxnum_bitref_r     sc_fxnum_subref†      sc_fxnum_subref_r
sc_fxnum_fast_bitref† sc_fxnum_fast_bitref_r sc_fxnum_fast_subref† sc_fxnum_fast_subref_r
sc_int_bitref†        sc_int_bitref_r†      sc_int_subref†        sc_int_subref_r†
sc_uint_bitref†       sc_uint_bitref_r†     sc_uint_subref†       sc_uint_subref_r†
sc_signed_bitref†     sc_signed_bitref_r†   sc_signed_subref†     sc_signed_subref_r†
sc_unsigned_bitref†   sc_unsigned_bitref_r† sc_unsigned_subref†   sc_unsigned_subref_r†
```

→ 실무 함의: 비트/부분 선택 결과(`x[3]`, `x.range(7,0)`)를 **변수에 담지 말 것**. 이들은 † proxy 타입이다. 즉시 소비하거나 값 타입으로 변환하라.

```cpp
sc_uint<8> x = 0xAB;
auto r = x.range(7,4);        // 위험: sc_uint_subref† 를 잡음 (수명·별칭 문제)
sc_uint<4> r2 = x.range(7,4); // OK: 값으로 변환
```

## 3.3 의미론 규약

### 3.3.1 클래스 정의와 상속 계층
구현은 base class·멤버·friend를 추가할 수 있고, 표준이 정의한 멤버를 표준 외 base class로 옮길 수 있다.
→ **정확한 상속 계층에 의존하는 코드를 쓰지 말 것.** 표준에 명시된 관계만 가정.

### 3.3.2 함수와 부작용
"함수 F는 동작 A를 수행한다 shall"라고 하면 F는 A 외의 동작을 하지 않는다 shall. 단 자원 관리·성능 최적화·구현 부가 기능은 예외.

### 3.3.3 `const char*` 파라미터
**애플리케이션은 null 포인터를 넘기면 안 된다 shall not.** null 종료 문자열(빈 문자열 가능)을 넘겨야 shall. 구현은 null 포인터가 오면 에러를 던져야 shall.

### 3.3.4 참조/포인터 반환 함수의 수명

반환 객체가 **valid**하다 = 삭제되지 않았고 값·동작에 접근 가능. valid 종료 후 참조하면 **동작 미정의**.

| 반환 형태 | 구현이 보장하는 유효 기간 |
|---|---|
| `*this` 또는 실인자(`ostream&` 등) | 추가 의무 없음 |
| `const char*` | **프로그램 종료까지.** 예외: `sc_process_handle::name`, `sc_report` 멤버 함수 → 해당 handle/report 객체가 valid한 동안만 |
| `sc_interface*`, `sc_event&`, `sc_event_finder&`, `sc_time&` (계층 내 객체) | 명시적 삭제 또는 모듈 계층 파괴 중 **먼저 오는 것**까지 |
| `sc_object*`, `sc_event*`, `sc_attr_base*`, `std::string&` (transient) | 해당 객체 자체가 삭제될 때까지만 |
| `std::vector<sc_object*>&`, `std::vector<sc_event*>&`, `sc_attr_cltn*` (컬렉션) | 컬렉션 내용을 바꿀 객체가 추가/삭제되기 전까지만 |
| `sc_time_stamp()` | **다음 timed notification phase 시작까지** |
| `sc_signal::read()` | **현재 evaluation phase 끝까지** |

transient 반환 함수 목록:
```cpp
sc_object* sc_process_handle::get_parent_object() const;
sc_object* sc_process_handle::get_process_object() const;
sc_object* sc_object::get_parent_object() const;
sc_object* sc_event::get_parent_object() const;
sc_object* sc_find_object( const char* );
sc_event*  sc_find_event( const char* );
sc_attr_base* sc_object::get_attribute( const std::string& );
const sc_attr_base* sc_object::get_attribute( const std::string& ) const;
sc_attr_base* sc_object::remove_attribute( const std::string& );
const std::string& sc_attr_base::name() const;
```

컬렉션 반환 함수 목록:
```cpp
virtual const std::vector<sc_object*>& sc_module::get_child_objects() const;
virtual const std::vector<sc_event*>&  sc_module::get_child_events() const;
const std::vector<sc_object*>& sc_process_handle::get_child_objects() const;
const std::vector<sc_event*>&  sc_process_handle::get_child_events() const;
virtual const std::vector<sc_object*>& sc_object::get_child_objects() const;
virtual const std::vector<sc_event*>&  sc_object::get_child_events() const;
const std::vector<sc_object*>& sc_get_top_level_objects();
const std::vector<sc_event*>&  sc_get_top_level_events();
sc_attr_cltn& sc_object::attr_cltn();
const sc_attr_cltn& sc_object::attr_cltn() const;
```

> `sc_time_stamp`와 `sc_signal::read`는 참조 반환이든 값 반환이든 **동작이 동일하도록** 코드를 작성할 것을 강력 권장 (§3.3.4.6).

### 3.3.5 네임스페이스

`sc_main` 단 하나를 제외한 모든 선언은 아래 다섯 네임스페이스 중 하나에 있어야 shall:

| 네임스페이스 | 내용 |
|---|---|
| `sc_core` | core language + predefined channels |
| `sc_dt` | SystemC data types proper |
| `sc_unnamed` | `sc_bind`용 인자 placeholder `_1` ~ `_9` (§5.5.6). **이것이 유일한 용도** |
| `tlm` | TLM-1 / TLM-2.0 interoperability layer |
| `tlm_utils` | TLM-2.0 utilities |

SystemC utilities(Clause 8)는 `sc_core`와 `sc_dt`에 나뉘어 있다.

### 3.3.6 비적합 애플리케이션과 에러

- 의무 위반 시 **일반적으로 동작 미정의** shall.
- 표준이 명시적으로 "error"라 하면 구현은 `sc_report_handler::report`를 **severity `SC_ERROR`**로 호출해야 shall.
- "warning"이면 **`SC_WARNING`**으로 호출해야 shall.
- 구현/애플리케이션이 효율 때문에 런타임 검사를 억제할 수 있으나(`set_actions`), 그렇다고 위반이 사라지는 것은 아님.
- "undefined"라고 명시된 곳은 구현에 아무 의무 없음 — 에러가 날 수도, 안 날 수도 있음.

## 3.4 Notes and examples

`NOTE`로 시작하는 문단과 예제 코드는 **informative** — 표준의 공식 일부가 아님. 규칙 근거로 인용할 때 이 점을 밝힐 것.
