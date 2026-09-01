# Annex D — IEEE Std 1666-2011 → IEEE Std 1666-2023 변경점

LRM pp. 614–616.

**이 Annex는 informative(참고)이며 규범(normative)이 아니다.**

본 SystemC 표준의 2011년판과 2023년판 사이의 **더 유의미한** 변경점 목록. 괄호 안 번호는 원문 항목 번호.

---

## 1. 언어 기반 / 전역 요구사항

| # | 내용 | 분류 |
|---|---|---|
| 1 | 본 표준을 준수하는 구현과 도구는 **최소 C++17**(ISO/IEC 14882:2017)을 사용할 것을 요구 | 변경 |
| 43 | 언어·표현을 더 포용적으로 만드는 전반적 갱신 | 변경 |

## 2. 제약 추가 (Restriction)

| # | 내용 | 분류 |
|---|---|---|
| 2 | 애플리케이션은 `const char*`를 파라미터로 쓰는 함수에 **null pointer를 전달하면 안 된다 shall not** | 제약 추가 |
| 3 | `sc_core::sc_start(float)`는 더 이상 **음수 값으로 호출될 수 없다** | 제약 추가 |
| 29 | part-select의 **비트 순서를 뒤집을 수 없다**는 제약 | 제약 추가 |

## 3. 신규 추가 — 커널 / 시뮬레이션 제어

| # | 내용 |
|---|---|
| 5 | 시뮬레이션 커널을 suspend/unsuspend 하는 신규 함수: `sc_core::sc_suspend_all`, `sc_core::sc_unsuspend_all`, `sc_core::sc_unsuspendable`, `sc_core::sc_suspendable`, `sc_core::sc_prim_channel::async_attach_suspending`, `sc_core::sc_prim_channel::async_detach_suspending` |
| 6 | 신규 클래스 `sc_core::sc_stage_callback_if` — 애플리케이션이 달리 접근할 수 없는 elaboration/simulation 단계에서 사용자 정의 callback을 가능하게 하는 인터페이스. 신규 함수: `sc_core::sc_stage_callback_if::stage_callback`, `sc_core::sc_register_stage_callback`, `sc_core::sc_unregister_stage_callback` |
| 7 | 신규 함수 `sc_core::sc_delta_count_at_current_time` — 현재 시각에 대한 delta cycle 수를 반환 |

## 4. 변경 — 커널 / 상태 질의

| # | 내용 |
|---|---|
| 4 | `sc_core::sc_argc`, `sc_core::sc_argv`의 정의가 C++ `main` 함수의 `argc`/`argv` 사용과 일관되도록 정제됨 |
| 8 | `sc_core::sc_get_status`는 멀티스레드 구현·애플리케이션 생성을 가능하게 하기 위해 **thread-safe일 것이 요구됨**. 이 함수는 시뮬레이션 커널이 suspend된 경우 이제 `sc_core::SC_SUSPENDED`를 반환한다 |

## 5. 모듈 / 매크로

| # | 내용 | 분류 |
|---|---|---|
| 9 | 생성자 매크로 `SC_CTOR`가 이제 **추가 생성자 파라미터를 지원** | 변경 |
| 10 | 매크로 `SC_HAS_PROCESS`가 더 이상 필요하지 않으며 **deprecated됨** (Annex C ah 참조) | 제거(deprecate) |
| 11 | 신규 매크로 `SC_NAMED` — 이름이 일치하는 변수를 선언하는 편리한 방법 제공 | 신규 |

## 6. 프로세스 / sensitivity / 이벤트 / 시간

| # | 내용 | 분류 |
|---|---|---|
| 12 | daggered 클래스 `sc_core::sc_sensitive`의 `operator<<`가 이제 프로세스 인스턴스의 static sensitivity에 추가할 **요소 컬렉션 전달을 지원**. 전형적 컬렉션은 `sc_core::sc_vector`의 요소들 또는 port·export·채널·이벤트의 C 배열 | 변경 |
| 13 | `sc_core::sc_process_handle`이 이제 멤버 함수 `basename`을 정의 — 기반 process instance의 string name을 반환 | 신규 |
| 14 | `sc_core::sc_event`가 멤버 함수 `triggered`를 도입 — 이벤트가 최근에 trigger되었는지 판정. 추가로 **정적 멤버 `none`** 제공 — 결코 notify되지 않는 이벤트에 대한 참조가 필요한 경우 사용 | 신규 |
| 15 | 열거 타입 `sc_core::sc_time_unit`이 이제 attosecond(`sc_core::SC_AS`), zeptosecond(`sc_core::SC_ZS`), yoctosecond(`sc_core::SC_YS`)를 지원 | 신규 |
| 16 | `sc_core::sc_time`이 시간을 **문자열 인자로 전달하는 생성자**를 지원. **모듈로 `operator%`** 추가. `sc_core::sc_time` 값이 `sc_core::sc_get_time_resolution` 미만인 경우와 `sc_core::sc_max_time` 초과인 경우에 대한 정의 개선. 기타 신규 함수: `sc_core::sc_time::from_value`, `from_seconds`, `from_string`, `to_string` | 신규 + 변경 |
| 17 | `sc_core::sc_max_time` 호출 **후에도 time resolution을 변경할 수 있다**는 정의 추가. 단 `sc_core::sc_time::to_double`, `to_seconds`, `to_string`, `print`, 또는 임의의 `sc_core::sc_time` 객체(`sc_max_time`이 만든 객체 포함)에 대한 `operator<<` 호출 후에는 `sc_core::sc_max_time`을 변경할 수 없다 | 변경 |

## 7. Port / export / 계층 / 객체

| # | 내용 | 분류 |
|---|---|---|
| 18 | `sc_core::sc_port_base`, `sc_core::sc_export_base`에 인터페이스 또는 인터페이스 타입에 접근하는 신규 함수: `sc_core::sc_port_base::get_interface`, `get_interface_type`, `sc_core::sc_export_base::get_interface`, `get_interface_type` | 신규 |
| 19 | 함수가 **값으로 반환**하는 경우 반환 타입에서 `const` 한정자 **제거** | 제거 |
| 20 | `sc_object`가 이제 **virtual 소멸자**를 가짐 | 변경 |
| 21 | `sc_core::sc_object`가 계층 컨텍스트에 접근하는 신규 멤버 함수 정의: `sc_core::sc_object::get_current_sc_object`, `sc_core::sc_object::get_hierarchy_scope` | 신규 |
| 22 | 신규 클래스 `sc_core::sc_hierarchy_scope` — 애플리케이션이 `sc_core::sc_object` 타입 객체를 현재 계층 스코프 **바깥의** 객체 계층에 배치할 수 있게 하는 인터페이스. 신규 함수: `sc_core::sc_hierarchy_scope::get_root` | 신규 |
| 23 | hierarchical name을 등록·해제하는 함수: `sc_core::sc_register_hierarchical_name`, `sc_core::sc_unregister_hierarchical_name` | 신규 |
| 27 | 신규 정적 객체 `sc_core::sc_unbound` — 애플리케이션에서 port에 **바인드되지 않은(open) 연결**을 만드는 데 쓸 수 있다. 추가로 `sc_core::sc_tie::value(const &T)` 함수로 port를 타입 `T`의 지정된 값에 tie할 수 있다 | 신규 |

## 8. 채널 (signal / buffer)

| # | 내용 | 분류 |
|---|---|---|
| 24 | 상수 `SC_DEFAULT_WRITER_POLICY`의 정의. 기본값으로 `SC_DEFAULT_WRITER_POLICY`는 이전 표준 버전과의 하위 호환을 위해 `SC_ONE_WRITER`와 같아야 shall. `sc_core::sc_signal`, `sc_core::sc_signal_resolved`, `sc_core::sc_buffer`의 기본 writer policy가 `SC_DEFAULT_WRITER_POLICY`로 설정됨. **구현은 `SC_DEFAULT_WRITER_POLICY`에 다른 값을 정의해도 된다 may** | 신규 |
| 25 | `sc_core::sc_signal`, `sc_core::sc_signal_resolved`, `sc_core::sc_buffer`에 채널의 **초기값을 설정하는 신규 생성자** — 이벤트와 그에 따른 delta cycle을 트리거하지 않음 | 신규 |
| 26 | `sc_signal`의 **인터페이스 클래스 상속 수정** | 수정 |

## 9. 데이터 타입

| # | 내용 | 분류 |
|---|---|---|
| 28 | `sc_dt::sc_bitref_r`에 Boolean 변환 지원을 개선하는 신규 함수·연산자: `sc_dt::sc_bitref_r::operator bool`, `sc_dt::sc_bitref_r::operator!` | 신규 |
| 30 | 신규 daggered 클래스 `sc_dt::sc_fxnum_bitref_r`, `sc_dt::sc_fxnum_subref_r`, `sc_dt::sc_fxnum_fast_bitref_r`, `sc_dt::sc_fxnum_fast_subref_r` — 각각 `sc_dt::sc_fxnum` / `sc_dt::sc_fxnum_fast`에서 선택된 **read-only 비트**를 나타냄 | 신규 |

## 10. Tracing / reporting

| # | 내용 | 분류 |
|---|---|---|
| 31 | `sc_core::sc_trace`로 `sc_core::sc_event`와 `sc_core::sc_time` 타입을 tracing하는 것을 지원하는 신규 정의 | 신규 |
| 32 | 신규 함수 `get_handler` — 현재 report handler를 반환 | 신규 |
| 33 | `sc_report_handler::get_log_file_name()`이 더 이상 **null pointer를 반환하지 않음** | 변경 |
| 34 | `SC_DEFAULT_INFO_ACTIONS`, `SC_DEFAULT_WARNING_ACTIONS`, `SC_DEFAULT_ERROR_ACTIONS`, `SC_DEFAULT_FATAL_ACTIONS`가 이제 네임스페이스 `sc_core` 안의 **enum 값**으로 정의됨 | 변경 |
| 35 | `sc_assert`가 이제 명시적으로 implementation-defined 함수 `sc_core::sc_assertion_failed`를 호출하며, 이 함수는 `[[noreturn]]` 속성을 가짐 | 변경 |

## 11. `sc_vector`

| # | 내용 | 분류 |
|---|---|---|
| 36 | 신규 함수 `sc_core::sc_vector::emplace_back`, `sc_core::sc_vector::emplace_back_with_name` — `sc_core::sc_vector`에 점진적 추가를 지원. 추가는 **`sc_vector`가 lock되지 않은 경우에만 허용**. lock 정책은 vector 초기화 시 `sc_core::sc_vector_init_policy`를 지정하여 정의하며, 두 모드를 지원: 초기화 후 lock(`SC_VECTOR_LOCK_AFTER_INIT`) 또는 elaboration 후 lock(`SC_VECTOR_LOCK_AFTER_ELABORATION`). **한 번 lock된 `sc_vector`는 unlock될 수 없다** | 신규 |

## 12. TLM

| # | 내용 | 분류 |
|---|---|---|
| 37 | 신규 **template-free** 기저 클래스 `tlm::tlm_base_socket_if` — 파생 socket 클래스에서 오버라이드되어야 권장(should)되는 순수 가상 함수를 선언 | 신규 |
| 38 | `tlm::tlm_generic_payload::set_extension`과 `set_auto_extension`에 제약 추가 — 같은 generic payload에 같은 타입의 extension을 여러 개 붙이는 것 금지 | 제약 추가 |
| 39 | TLM-2 multi-socket에 대한 **request/response exclusion rule 명확화** | 명확화 |
| 40 | `nb_transport` 호출 시퀀스 다이어그램(Figure 29)의 누락된 화살표 수정 | 수정 |
| 41 | TLM-1 `tlm::tlm_fifo_get_if`의 **C++ 모호성 해소(disambiguation)** | 수정 |

## 13. 기타

| # | 내용 | 분류 |
|---|---|---|
| 42 | 모든 enum 정의에 대한 **정확한 수치 값이 제거되었고 이제 implementation-defined**다 | 변경 |
