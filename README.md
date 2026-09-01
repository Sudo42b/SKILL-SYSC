# SKILL-SYSC

Claude Code skill for **SystemC / TLM-2.0**, built from **IEEE Std 1666-2023** (Language Reference Manual, 618 p).

LRM의 17개 Clause와 Annex를 1:1로 정리한 reference 집합이다. Claude가 SystemC 모델 코드를 작성·검토할 때 표준 규칙을 따르고, 그 근거를 절 번호로 인용하게 한다.

## 하는 일

- **작성** — LRM 준수 SystemC / TLM-2.0 코드 생성
- **조회** — "LRM이 X에 대해 뭐라고 하냐"에 절 번호를 인용해 답변
- **검토** — 기존 코드의 `shall` 위반, deprecated 기능 사용, `undefined` 동작 의존 탐지

규범 강도(`shall` / `should` / `may` / `error` / `undefined` / `implementation-defined`)를 원문 그대로 보존한다.

## 설치

### 사용자 전역

```bash
git clone https://github.com/Sudo42b/SKILL-SYSC.git ~/.claude/skills/systemc
```

### 프로젝트 한정

```bash
git clone https://github.com/Sudo42b/SKILL-SYSC.git .claude/skills/systemc
```

Claude Code 재시작 후 `/systemc` 로 호출하거나, SystemC 관련 질문 시 자동 활성화된다.

## 구조

```
SKILL-SYSC/
├── SKILL.md                                  진입점 — 어느 reference를 읽을지 라우팅
└── references/
    ├── ch01-overview.md                      Scope, shall/should/may, C++17 baseline
    ├── ch02-normative-references.md          ISO/IEC 14882:2017
    ├── ch03-terminology.md                   용어 정의, † 클래스, 수명 규칙, 네임스페이스
    ├── ch04-elaboration-simulation.md        스케줄러, delta cycle, 콜백 허용/금지 매트릭스
    ├── ch05-core-language.md                 sc_module, 프로세스, sc_port/sc_export, sc_event, sc_time
    ├── ch06-predefined-channels.md           sc_signal, sc_fifo, sc_clock, sc_mutex, sc_semaphore
    ├── ch07-datatypes.md                     sc_int/sc_uint/sc_bigint/sc_logic/sc_bv/sc_lv/sc_fixed
    ├── ch08-utilities.md                     sc_trace, sc_report, sc_vector, sc_assert
    ├── ch09-tlm2-overview.md                 TLM-2.0 준수 조건
    ├── ch10-tlm2-introduction.md             coding style (UT/LT/AT/CA), socket 개념
    ├── ch11-tlm2-core-interfaces.md          b_transport, nb_transport, DMI, transport_dbg
    ├── ch12-tlm2-global-quantum.md           tlm_global_quantum
    ├── ch13-tlm2-sockets.md                  initiator/target socket
    ├── ch14-tlm2-generic-payload.md          속성, 메모리 관리, extension
    ├── ch15-tlm2-base-protocol.md            phase 전이표, 트랜잭션 순서 규칙
    ├── ch16-tlm2-utilities.md                convenience socket, quantum keeper, PEQ
    ├── ch17-tlm1-analysis-ports.md           put/get/peek, tlm_fifo, analysis port
    ├── annexA-glossary.md                    용어집 (informative)
    ├── annexB-introduction.md                SystemC 입문 (informative)
    ├── annexC-deprecated.md                  쓰면 안 되는 기능 목록 (informative)
    └── annexD-changes-2011-2023.md           2011 → 2023 변경점 (informative)
```

`SKILL.md`만 항상 로드되고, `references/*`는 필요한 것만 읽힌다 (progressive disclosure).

## 사용 예

```
> sc_signal에 두 프로세스가 write하면 어떻게 되나?
> 이 TLM initiator가 base protocol을 지키는지 검토해줘
> LT coding style로 temporal decoupling 붙인 initiator 모듈 만들어줘
> end_of_elaboration 콜백 안에서 SC_CTHREAD 써도 되나?
> 이 SystemC 코드에서 deprecated 기능 쓰는 곳 찾아줘
```

## Citation

이 스킬의 reference 내용은 전적으로 아래 표준에서 파생되었다. 이 스킬을 사용한 결과물을 인용할 때는 스킬이 아니라 **표준 원문을 인용하라.**

> IEEE Std 1666™-2023 (Revision of IEEE Std 1666-2011), *IEEE Standard for Standard SystemC® Language Reference Manual*. Design Automation Standards Committee, IEEE Computer Society. Approved 5 June 2023 by the IEEE SA Standards Board. Published 8 September 2023. New York, NY, USA: The Institute of Electrical and Electronics Engineers, Inc.
> Print ISBN 978-1-5044-9867-8 (STD26278) · PDF ISBN 978-1-5044-9868-5 (STDPD26278).

### BibTeX

```bibtex
@standard{ieee1666-2023,
  title        = {{IEEE Standard for Standard SystemC\textsuperscript{\textregistered}
                   Language Reference Manual}},
  organization = {IEEE Computer Society, Design Automation Standards Committee},
  publisher    = {Institute of Electrical and Electronics Engineers},
  address      = {New York, NY, USA},
  number       = {IEEE Std 1666-2023},
  note         = {Revision of IEEE Std 1666-2011.
                  Approved 5 June 2023 by the IEEE SA Standards Board},
  year         = {2023},
  month        = sep,
  isbn         = {978-1-5044-9868-5},
  url          = {https://standards.ieee.org/ieee/1666/7071/}
}
```

### IEEE style

```
IEEE Standard for Standard SystemC Language Reference Manual,
IEEE Std 1666-2023 (Revision of IEEE Std 1666-2011), Sep. 2023.
```

### 절 인용

이 스킬은 규칙의 근거를 LRM 절 번호로 인용한다. 스킬 밖으로 옮길 때는 표준을 함께 밝혀라.

```
IEEE Std 1666-2023, §5.2.8 — SC_CTHREAD shall not be invoked
from the end_of_elaboration callback.
```

## 저작권과 라이선스

- **표준 원문**: Copyright © 2023 by the Institute of Electrical and Electronics Engineers, Inc. **All rights reserved.** 표준 문서 자체는 IEEE의 사전 서면 허락 없이 어떤 형태로도 복제할 수 없다. 표준이 필요하면 [IEEE Xplore](https://standards.ieee.org/ieee/1666/7071/)에서 직접 입수하라.
- **이 리포지토리**: 표준의 규범적 내용을 **요약·재서술한 2차 저작물**이며 IEEE가 승인하거나 검토하지 않았다. 표준 원문을 대체하지 않는다. 정확한 문언이 필요하면 반드시 원문을 확인하라.
- 리포지토리 자체 저작물(SKILL.md 구조, 라우팅·체크리스트, 한국어 서술)의 이용 조건은 [`LICENSE`](LICENSE)를 따른다.
- SystemC®는 Accellera Systems Initiative의 미국 특허상표청 등록 상표다. IEEE는 Institute of Electrical and Electronics Engineers, Inc.의 등록 상표다.

### 면책

표준 문서는 IEEE에 의해 "AS IS", "WITH ALL FAULTS"로 제공되며 IEEE는 상품성·특정 목적 적합성·비침해에 대한 보증을 명시적으로 부인한다. 이 리포지토리 역시 어떠한 보증도 제공하지 않는다.
