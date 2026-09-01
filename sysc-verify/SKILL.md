---
name: sysc-verify
description: 기존 SystemC / TLM-2.0 코드를 IEEE 1666-2023 준수 여부로 감사한다. shall 위반, deprecated 기능 사용, undefined·implementation-defined 동작 의존, base protocol phase 위반, elaboration/simulation 단계 위반, sc_signal writer policy 위반을 찾아 절 번호와 규범 강도를 근거로 지적한다. "이 SystemC 코드 표준에 맞나", "LRM 위반 찾아줘", "TLM 모델 리뷰", "base protocol 준수 확인", "deprecated 쓰고 있나", "코드 감사", "conformance check" 같은 요청에 사용한다. 코드를 새로 쓰는 것이 목적이면 sysc-lt / sysc-at / sysc-ca를 쓴다.
---

# SystemC / TLM-2.0 준수 감사

**판정 기준은 `../SKILL.md`의 「검토 체크리스트」와 `../references/`다.** 이 스킬은 그것을 **어떤 순서로, 무엇을 grep 해서, 어떤 형식으로 보고할지**만 정한다. 체크 항목 자체를 여기 복제하지 않는다 — 원본이 갱신되면 이쪽이 낡는다.

## 감사 전에 확정할 것

1. **타깃 SystemC 버전.** 2.3.x(IEEE 1666-2011)면 2023 신규 기능은 전부 위반이고, `SC_HAS_PROCESS`는 위반이 아니라 필수다. 판별:
   ```bash
   grep -r "IEEE_1666_SYSTEMC\|SC_VERSION_MAJOR" "$SYSTEMC_HOME/include/sysc/kernel/sc_ver.h"
   ```
2. **코딩 규칙 적용 대상인가.** `../CODING-RULES.md` 규칙 5 — 레거시 매크로 모듈은 개조 대상이 아니다. 신규 코드에만 적용한다.

## 순서

**단계 1이 나머지의 전제다.** ch04를 모르면 나머지 판정이 전부 틀린다.

### 1. 단계 위반 — elaboration인가 simulation인가

각 코드가 어느 콜백 안인지 먼저 확정한다. 인스턴스화·바인딩·`SC_*` 매크로·`sensitive`·event finder는 대부분 **elaboration 전용**이다. 콜백별 허용/금지 매트릭스는 `../references/ch04-elaboration-simulation.md`.

```bash
grep -rn "before_end_of_elaboration\|end_of_elaboration\|start_of_simulation\|end_of_simulation" --include=*.cpp --include=*.h .
```

### 2. 프로세스 종류 불일치

```bash
grep -rn "SC_METHOD\|SC_THREAD\|SC_CTHREAD" --include=*.cpp --include=*.h .
```

각 프로세스 함수 본문에서:

- method process 안의 `wait()` → **error**
- thread process 안의 `next_trigger()` → **error**
- clocked thread 안의 `wait()`/`wait(int)` 외 오버로드 → **error**

### 3. deprecated 사용 — 컴파일러는 알려주지 않는다

**경고 0건은 아무것도 증명하지 않는다.** SystemC 3.0.2가 `[[deprecated]]`로 표시한 것은 `SC_HAS_PROCESS` 하나뿐이고, Annex C의 나머지 33개 항목은 **조용히 컴파일된다.** 손으로 대조해야 한다:

```bash
grep -rnE "sc_cycle|sc_initialize|sc_simcontext|sc_process_b|sc_get_curr_process_handle|notify_delayed|timed_out|end_module|sc_simulation_time|sc_set_default_time_unit|sc_get_default_time_unit|get_data_ref|get_new_value|add_trace|sc_inout_clk|sc_out_clk|sc_signal_out_if|SC_DEFAULT_STACK_SIZE|SC_MAX_NUM_DELTA_CYCLES|SYSTEMC_VERSION|sc_set_vcd_time_unit|sc_trace_delta_cycles|sc_bit|sc_string|sc_pvector|sc_plist|sc_phash|sc_ppq|DECLARE_EXTENDED_PHASE|SC_HAS_PROCESS" --include=*.cpp --include=*.h .
```

전체 목록과 대체 수단은 `../references/annexC-deprecated.md` (a–ah 34항목). `SC_CTHREAD`는 **두 번째 인자가 event finder면 예외**다.

`SC_ALLOW_DEPRECATED_IEEE_API`가 정의돼 있으면 경고가 죽는다 — 빌드 파일도 확인하라:

```bash
grep -rn "SC_ALLOW_DEPRECATED_IEEE_API" .
```

### 4. 2011 → 2023 이식 문제

2023 신규 기능을 2.3.x 타깃 코드가 쓰고 있으면 컴파일되지 않는다. 대조표는 `../references/annexD-changes-2011-2023.md` (43항목).

```bash
grep -rnE "sc_suspend_all|sc_unsuspend_all|sc_unsuspendable|sc_suspendable|async_attach_suspending|sc_stage_callback_if|sc_register_stage_callback|sc_delta_count_at_current_time|SC_NAMED|sc_hierarchy_scope|sc_register_hierarchical_name|sc_unbound|sc_tie|emplace_back_with_name|tlm_base_socket_if|SC_AS|SC_ZS|SC_YS" --include=*.cpp --include=*.h .
```

### 5. 채널·바인딩

- `sc_signal` writer policy — 기본은 `SC_ONE_WRITER` (Annex D `24)`). 두 프로세스가 같은 신호를 쓰면 위반.
- 포트를 이름과 위치 **양쪽**으로 바인딩 → **shall not**
- export 이중 바인딩 또는 미바인딩 → **error**
- `operator->` / `operator[]`를 `end_of_elaboration` 이전에 호출 → 위반

근거는 `../references/ch05-core-language.md`, `../references/ch06-predefined-channels.md`.

### 6. TLM — 코딩 스타일부터 판정한다

`b_transport`만 쓰면 LT, `nb_transport`가 있으면 AT다. 판정 후 해당 스킬의 「자주 틀리는 것」을 그대로 적용하라:

| 발견한 것 | 적용할 기준 |
|---|---|
| `b_transport`, `tlm_quantumkeeper`, DMI | `sysc-lt` |
| `nb_transport_fw` / `nb_transport_bw`, phase 인자 | `sysc-at` |
| `sc_clock` + `SC_CTHREAD` 신호 수준 | `sysc-ca` |

AT 코드에서 특히 볼 것 — 근거는 `../references/ch15-tlm2-base-protocol.md`:

```bash
grep -rn "TLM_ACCEPTED\|TLM_UPDATED\|TLM_COMPLETED\|BEGIN_REQ\|END_REQ\|BEGIN_RESP\|END_RESP" --include=*.cpp --include=*.h .
```

- `TLM_ACCEPTED`를 반환하면서 트랜잭션·phase·time을 수정했는가 (§11.2.3.7 e — **shall not**)
- `TLM_COMPLETED` 반환 후 phase 값을 읽는가 (§11.2.3.7 g — **undefined**)
- 같은 소켓에 request 또는 response가 둘 outstanding인가 (§15.2.12 — **금지**)
- `END_REQ`/`BEGIN_RESP`를 target socket이 아닌 곳으로 보내는가 (§15.2.3 c — **shall**)
- `nb_transport`에 넘기는 트랜잭션에 memory manager가 있는가 (§15.2.3 p — **obliged**)
- write에 `TLM_OK_RESPONSE`를 target 확인 없이 설정하는가 (§15.2.11 c — **shall**)

### 7. undefined / implementation-defined 의존

- evaluation phase의 프로세스 실행 순서에 의존 → **구현 정의** (§4.3.2.3)
- immediate notification(`notify()`)으로 프로세스 간 순서를 만들려는 시도 → **비결정적**
- `x.range(7,4)`를 `auto`로 받기 → † proxy 타입을 잡는다 (§3.2.4)

### 8. 코딩 규칙 준수 (신규 코드만)

`../CODING-RULES.md` 6항목. 규칙 6은 기계적으로 확인 가능하다:

```bash
g++ -std=c++17 -Wall -Wextra -I"$SYSTEMC_HOME/include" -c <file>
```

## 보고 형식

발견 하나당 한 줄. **절 번호와 규범 강도를 반드시 붙인다.**

```
<file>:<line>  <위반 내용> — LRM §<절> (<shall|should|may|error|undefined|implementation-defined>)
                 고치는 법: <구체적 수정>
```

예:

```
router.cpp:88  get_direct_mem_ptr 구현에서 wait 호출 — LRM §11.3.3 o (shall not)
                 고치는 법: DMI 조회를 즉시 반환하도록 바꾸고, 지연은 transport 경로에서 부과할 것
mem.cpp:41     TLM_ACCEPTED 반환 전에 trans.set_response_status 호출 — LRM §11.2.3.7 e (shall not)
                 고치는 법: 갱신했다면 TLM_UPDATED를 반환하거나, 갱신을 backward path로 미룰 것
```

**규범 강도를 바꾸지 마라.** `shall` 위반은 비적합 코드이고 반드시 고쳐야 한다. `should` 이탈은 근거를 남기면 허용된다. `may`는 규칙이 아니다. NOTE와 예제는 **informative**이므로 위반 근거로 쓸 수 없다 — 인용할 때 그 사실을 밝혀라.

## 판정할 수 없는 것

- **원문에 없는 규칙을 지어내지 마라.** 해당 Clause 파일을 더 읽거나, 표준에 규정이 없다고 말한다.
- **구현 동작과 표준이 다르면 표준이 정의적이다** (LRM Introduction). 예: `sc_event::none`은 3.0.2에서 함수, LRM에서는 데이터 멤버다 — 코드는 `sc_event::none()`을 써야 돌아가지만, 규범 판정은 표준을 따르고 이탈은 따로 표시한다.
- **성능·스타일 지적은 준수 판정과 섞지 마라.** 이 감사는 conformance 감사다.

## 이 스킬 저장소 자체를 검증할 때

`references/`가 LRM과 어긋나지 않았는지 확인하려면 (본인의 정식 라이선스 PDF 필요):

```bash
python3 ../tools/verify_references.py --pdf /path/to/1666-2023.pdf   # references/ 가 LRM과 맞는가
python3 ../tools/verify_citations.py --pdf /path/to/1666-2023.pdf    # 스킬의 § 인용이 맞는 절을 가리키는가
python3 ../tools/verify_references.py --selftest                     # PDF 없이 스크립트 자체 검사
python3 ../tools/verify_citations.py --selftest
```

인용 검증기는 세 가지를 본다: 인용 번호가 LRM에 실존하는가, LRM이 식별자로 이름 붙인 절을 인용해 놓고 문장은 형제 절의 식별자를 말하고 있지 않은가, `shall`/`should`/`error`를 뒷받침하는 인용이 규칙을 담은 절 대신 그 부모 절에서 멈추지 않았는가. 스킬 파일의 절 기호 표기와 예제 소스의 `LRM` 접두 표기를 모두 센다.
