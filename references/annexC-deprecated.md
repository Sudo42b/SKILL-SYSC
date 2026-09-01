# Annex C — Deprecated features (폐지 예정 기능)

LRM pp. 612–613.

**이 Annex는 informative(참고)이며 규범(normative)이 아니다.**

deprecated feature = OSCI open source proof-of-concept SystemC 구현 **버전 2.0.1에는 있었지만 본 표준에는 포함되지 않은** 기능. deprecated feature는 향후 Accellera Systems Initiative 구현에 남을 수도 있고 남지 않을 수도 있다.

- **구현은 이런 기능을 지원할 의무가 없으므로 사용자가 deprecated feature를 쓰는 것은 강력히 권장되지 않는다(strongly discouraged).**
- 구현은 각 deprecated feature의 첫 출현 시 warning을 발행해도 된다 may. 그럴 의무는 없다.

**쓰면 안 되는 것 체크리스트** — 코드 리뷰 시 아래 이름이 보이면 대체 수단으로 교체하라.

| # | Deprecated 기능 | 대신 쓸 것 (원문에 명시된 경우만) |
|---|---|---|
| a | 함수 `sc_cycle`, `sc_initialize` | `sc_start` |
| b | 클래스 `sc_simcontext` | 함수 `sc_delta_count`, `sc_is_running`, `sc_get_top_level_objects`, `sc_find_object` 및 멤버 함수 `get_child_objects`, `get_parent_object`로 대체됨 |
| c | 타입 `sc_process_b` | 클래스 `sc_process_handle`로 대체됨 |
| d | 함수 `sc_get_curr_process_handle` | 함수 `sc_get_current_process_handle`로 대체됨 |
| e | `sc_event`의 멤버 함수 `notify_delayed` | `notify(SC_ZERO_TIME)` |
| f | 비멤버 함수 `notify` | `sc_event`의 멤버 함수 `notify` |
| g | `sc_module`과 `sc_prim_channel`의 멤버 함수 `timed_out` | (원문에 대체 수단 명시 없음) |
| h | positional port binding용 `sc_module`의 `operator,` 와 `operator<<` | `operator()` |
| i | positional port binding용 `sc_module`의 `operator()` 를 **모듈 인스턴스당 두 번 이상** 호출하는 것 | named port binding |
| j | port 객체 생성 시점에 port를 바인드하는 `sc_port`의 생성자들 | (원문에 대체 수단 명시 없음) |
| k | `sc_sensitive`의 `operator()` | `operator<<` |
| l | 클래스 `sc_sensitive_pos`, `sc_sensitive_neg` 및 `sc_module`의 대응 데이터 멤버 | event finder `pos`와 `neg` |
| m | `sc_module`의 멤버 함수 `end_module` | (원문에 대체 수단 명시 없음) |
| n | **Default time unit** 및 연관된 모든 함수·생성자: <br>1) 함수 `sc_simulation_time` <br>2) 함수 `sc_set_default_time_unit` <br>3) 함수 `sc_get_default_time_unit` <br>4) 함수 `sc_start(double)` <br>5) 생성자 `sc_clock(const char*, double, double, double, bool)` | (원문에 대체 수단 명시 없음) |
| o | `sc_object`, `sc_signal`, `sc_clock`, `sc_fifo`의 멤버 함수 `trace` | `sc_trace` |
| p | `sc_in`, `sc_inout`의 멤버 함수 `add_trace` | `sc_trace` |
| q | `sc_signal`, `sc_clock`의 멤버 함수 `get_data_ref` | 멤버 함수 `read` |
| r | `sc_signal`의 멤버 함수 `get_new_value` | (원문에 대체 수단 명시 없음) |
| s | Typedef `sc_inout_clk`, `sc_out_clk` | `sc_out<bool>` |
| t | Typedef `sc_signal_out_if` | (원문에 대체 수단 명시 없음) |
| u | 상수 `SC_DEFAULT_STACK_SIZE` | (함수 `set_stack_size`는 deprecated 아님) |
| v | 상수 `SC_MAX_NUM_DELTA_CYCLES` | (원문에 대체 수단 명시 없음) |
| w | 상수 `SYSTEMC_VERSION` | (함수 `sc_version`은 deprecated 아님) |
| x | **wif**와 **isdb** trace file 형식 지원 | vcd trace file 형식은 deprecated 아님 |
| y | `vcd_trace_file`의 멤버 함수 `sc_set_vcd_time_unit` | (원문에 대체 수단 명시 없음) |
| z | 함수 `sc_trace_delta_cycles` | (원문에 대체 수단 명시 없음) |
| aa | enumeration literal을 trace file에 쓰는 `sc_trace` 함수 | 다른 `sc_trace` 함수들은 deprecated 아님 |
| ab | 타입 `sc_bit` | 타입 `bool` |
| ac | 매크로 `SC_CTHREAD` — **단, 두 번째 인자가 event finder인 경우는 예외이며 여전히 지원됨** | (그 예외 형태를 쓸 것) |
| ad | clocked thread에 대한 global watching과 local watching | 함수 `reset_signal_is` |
| ae | 정수 id 기반 reporting 메커니즘 및 `sc_report`의 대응 멤버 함수: `register_id`, `get_message`, `is_suppressed`, `suppress_id`, `suppress_infos`, `suppress_warnings`, `make_warnings_errors`, `get_id` | 문자열 message type을 쓰는 reporting 메커니즘으로 대체됨 |
| af | 유틸리티 클래스 `sc_string`, `sc_pvector`, `sc_plist`, `sc_phash`, `sc_ppq` | (원문에 대체 수단 명시 없음) |
| ag | 매크로 `DECLARE_EXTENDED_PHASE` (TLM-2.0.1 유래) | `TLM_DECLARE_EXTENDED_PHASE` |
| ah | 매크로 `SC_HAS_PROCESS` | (원문에 대체 수단 명시 없음. Annex D 10)에 따르면 더 이상 필요하지 않아 deprecate됨) |
