# Annex B (informative) — Introduction to SystemC

LRM pp. 608–611. **informative** — 표준의 규범적 일부가 아니다. 클래스 라이브러리의 구조와 의도를 이해하는 데 도움을 주려는 목적.

SystemC를 처음 접하거나 전체 그림이 필요할 때 여기부터 읽는다.

## SystemC가 제공하는 것

시스템의 기능 모델링을 위해 다음을 표현하는 클래스를 제공한다:

- 시스템을 **모듈**로 계층 분해
- port와 export로 모듈 간 **구조적 연결**
- event와 sensitivity로 동시 프로세스의 **스케줄링·동기화**
- **모의 시간(simulated time)의 흐름**
- **연산(process)과 통신(channel)의 분리**
- interface를 통한 연산과 통신의 **독립적 정제(refinement)**
- 디지털 로직과 고정소수점 산술 모델링을 위한 **하드웨어 지향 데이터 타입**

> 요약: 사용자는 C++ 함수(프로세스) 집합을 작성하고, 스케줄러가 모의 시간의 흐름을 흉내내는 순서로 이들을 실행한다. 프로세스는 시스템의 구조적 관계와 연결을 담는 모듈 계층에 캡슐화된다. 프로세스 간 통신은 **interface method call** 메커니즘을 쓰며, 이것이 시스템 수준 인터페이스의 추상화와 독립적 정제를 가능하게 한다.

## Figure B-1 — SystemC 언어 아키텍처

```
┌───────────────────────────────────────────────────────────────────────────┐
│                              Application                                   │
│                          (최종 사용자가 작성)                                │
├───────────────────────────────────────────────────────────────────────────┤
│              Methodology- and technology-specific libraries                │
│      SystemC verification library, bus models, TLM interfaces  (표준 범위 밖) │
├──────────────┬──────────────────┬───────────────┬─────────────────────────┤
│ Core language│ Predefined       │ Utilities     │ Data types              │
│              │ channels         │               │                         │
│ Modules      │ Signal, clock,   │ Report        │ 4-valued logic type     │
│ Ports        │ FIFO, mutex,     │ handling,     │ 4-valued logic vectors  │
│ Exports      │ semaphore        │ tracing       │ Bit vectors             │
│ Processes    │                  │               │ Finite-precision int    │
│ Interfaces   │                  │               │ Limited-precision int   │
│ Channels     │                  │               │ Fixed-point types       │
│ Events       │                  │               │                         │
├──────────────┴──────────────────┴───────────────┴─────────────────────────┤
│                        Programming language C++                            │
└───────────────────────────────────────────────────────────────────────────┘
```

음영 블록(아래 4개 + C++)이 SystemC 클래스 라이브러리 자체. 바로 위 계층(특정 설계·검증 방법론이나 통신 채널과 연관된 표준/독점 C++ 라이브러리)은 **이 표준의 범위 밖**.

라이브러리 클래스는 네 범주로 나뉜다: **core language / SystemC data types / predefined channels / utilities**. core language와 data types는 서로 독립적으로 쓸 수 있으나 보통 함께 쓴다.

## 시뮬레이션 엔진

- 핵심은 **process scheduler**를 포함한 시뮬레이션 엔진.
- 프로세스는 **event notification에 반응해** 실행된다. event는 모의 시간의 특정 시점에 notify된다.
- **시간 순서가 다른 event에 대해서는 결정적(deterministic)**, **같은 시점의 event에 대해서는 비결정적(non-deterministic)**.
- 스케줄러는 **비선점(non-preemptive)** (→ [ch04](ch04-elaboration-simulation.md) §4.3.2).

## 모듈

기본 구조 빌딩 블록. 시스템은 인스턴스화로 연결된 모듈 집합, 즉 **module hierarchy**로 표현된다.

모듈이 담을 수 있는 것:

| 항목 | 참조 |
|---|---|
| Ports | §5.12 |
| Exports | §5.13 |
| Channels | §5.2, §5.15 |
| Processes | §5.2.10, §5.2.11 |
| Events | §5.10 |
| 다른 모듈의 인스턴스 | §4.2.2 |
| 기타 데이터 멤버 | — |
| 기타 멤버 함수 | — |

모듈·포트·export·채널·인터페이스·이벤트·시간은 모두 **C++ 클래스로 구현**된다.

## 실행 구조

**elaboration**(모듈 계층 생성) → **simulation**(스케줄러 실행). 양쪽 모두 애플리케이션 코드와 커널 코드를 실행한다. **kernel** = elaboration과 스케줄러의 핵심 기능을 제공하는 구현 부분.

- port/export/channel/module 인스턴스는 **elaboration 중에만** 생성 가능. 한 번 생성되면 그 계층 구조는 이후 elaboration·simulation 내내 **고정**된다 (→ Clause 4).
- 프로세스 인스턴스는 elaboration 중 정적으로(§5.2.8) 또는 simulation 중 동적으로(§5.5) 생성 가능.
- module/channel/port/export/process는 공통 base class **`sc_object`**에서 파생되며, 이것이 계층 순회 메서드를 제공한다. `sc_object` 인스턴스에 임의의 **attribute(이름-값 쌍)**를 붙일 수 있다 (§5.16).
- port/export/channel/module 인스턴스는 **모듈 안에서만** 생성 가능. **유일한 예외가 top-level module.**

## 프로세스와 sensitivity

- 프로세스는 연산을 수행해 시스템 기능을 모델링한다. **개념상 동시적이지만 실제로는 순차 실행되도록 스케줄**된다.
- 프로세스는 elaboration 중(static) 또는 simulation 중(dynamic) 커널에 등록된 C++ 함수이며, simulation 중 커널이 호출한다.
- **sensitivity** = 그 event들이 notify되면 스케줄러가 그 프로세스를 실행하게 되는 event 집합.
  - **static sensitivity**: 프로세스 인스턴스 생성 시점에 생성
  - **dynamic sensitivity**: simulation 중 연관 함수 실행 중에 생성
- 프로세스는 이름 있는 event에 sensitive할 수도, **채널 안에 묻혀 있거나 포트 뒤에 있는 event에 event finder로 찾아** sensitive할 수도 있다.
- dynamic sensitivity는 **time-out**과 함께 생성 가능 — 주어진 시간 간격 경과 후 실행 (§4.3.2, §5.2.14~§5.2.18).

## Channel과 Interface

- **channel**은 프로세스가 통신하는 메커니즘을 캡슐화 → 시스템의 통신 측면·프로토콜을 모델링. 모듈 간 통신에도, 모듈 내 프로세스 간 통신에도 쓸 수 있다.
- **interface**는 채널에 접근하는 수단. **interface proper**는 pure virtual 함수(interface method) 집합을 선언하는 추상 클래스. 채널이 그 안의 모든 메서드를 정의하면 그 interface를 **구현한다(implements)**고 한다.
- 목적: **C++ 객체지향 타입 시스템을 이용해, 채널을 그것을 사용하는 모듈과 독립적으로 정제**하기 위함. 구체적으로, **특정 interface를 구현한 어떤 채널이든, 그 interface 타입을 명시한 문맥에서 다른 채널로 교체할 수 있다.**
- 채널의 메서드는 보통 interface를 통해 호출된다. **한 채널이 여러 interface를 구현**할 수 있고, **한 interface를 여러 채널이 구현**할 수 있다.
- 채널에 구현된 interface method는 **그 채널 안의 event에 대한 dynamic sensitivity를 만들 수 있다.** 이것이 전형적 코딩 관용구이며, 그 결과 메서드를 호출한 프로세스가 해당 event 발생까지 suspend되는 **blocking method**가 된다. 그런 메서드는 **thread process에서만** 호출 가능 (§5.2.10, §5.2.11).

## Port와 Export — 계층 경계 넘기

프로세스와 채널이 모듈 안에 캡슐화되므로, (채널을 통한) 프로세스 간 통신은 모듈 계층의 경계를 넘을 수 있다. 이를 중재하는 것이 port와 export — 모듈 안의 프로세스에서 온 메서드 호출을 **바인딩된 채널로 전달**한다.

| | 의미 | 방향 |
|---|---|---|
| **port** | 모듈이 특정 interface를 **요구(requires)** | 메서드 호출을 모듈 **위/밖으로만** 전달 |
| **export** | 모듈이 특정 interface를 **제공(provides)** | 메서드 호출을 모듈 **아래/안으로만** 전달 |

- port는 모듈 안의 interface method call이 **인스턴스화되는 문맥과 독립**이 되게 한다 — 모듈은 자기 포트가 어떤 채널에 바인딩되는지 알 필요가 없다.
- export는 **한 모듈이 같은 interface의 인스턴스를 여러 개 제공**할 수 있게 한다.
- 포트는 elaboration 중 채널 인스턴스에 바인딩된다. **port binding policy**로 포트가 반드시 바인딩되어야 하는지 제어할 수 있으나, **이후 바인딩을 바꿀 수는 없다.**
- export는 그 export를 담은 모듈 **안 또는 아래**에 있는 채널 인스턴스에 바인딩된다.
- 따라서 port/export를 통한 각 interface method call은 elaborate된 모듈 계층의 **특정 채널 인스턴스**로 향한다.
- 그런 메서드 호출은 **항상 모듈 안의 프로세스에서 시작**되어 모듈 계층 다른 곳에 인스턴스화된 채널로 향한다.
- port와 export는 **interface 타입으로 파라미터화된 템플릿 클래스**의 인스턴스. 그 interface 또는 그로부터 파생된 것을 구현한 채널에만 바인딩 가능 (§5.12~§5.14).

## 채널의 두 범주

| | 정의 | 담을 수 있는 것 |
|---|---|---|
| **hierarchical channel** | **모듈이다** | 프로세스, 모듈·포트·다른 채널의 인스턴스 |
| **primitive channel** | `sc_prim_channel`에서 파생. **모듈이 아니다** | 위의 것들 **전부 불가** |

둘 중 어느 base class에서도 파생되지 않은 채널을 정의하는 것도 가능하다. 단 **모든 채널은 하나 이상의 interface를 구현한다.**

**primitive channel은 스케줄러의 update phase에 대한 독점 접근을 제공** → 특정 통신 방식을 매우 효율적으로 구현할 수 있다.

## 미리 정의된 채널 (Clause 6)

| 채널 | 참조 | 용도 |
|---|---|---|
| `sc_signal` | §6.4 | **RTL 또는 pin-accurate 디지털 하드웨어 모델**의 의미론 |
| `sc_buffer` | §6.6 | |
| `sc_clock` | §6.7 | |
| `sc_signal_resolved` | §6.13 | |
| `sc_signal_rv` | §6.17 | |
| `sc_fifo` | §6.23 | **point-to-point FIFO 기반 통신** — 통신 프로세스 네트워크 모델에 적합 |
| `sc_mutex` | §6.27 | **소프트웨어 모델링**에 적합한 통신 프리미티브 |
| `sc_semaphore` | §6.29 | 동상 |
| `sc_event_queue` | §6.29 | |

## 데이터 타입 (Clause 7)

| 타입 | 참조 | 설명 |
|---|---|---|
| `sc_int<>` | §7.5.4 | **부호 있는 limited-precision 정수** — 워드 길이가 C++ 구현에 의해 제한됨 |
| `sc_uint<>` | §7.5.5 | 부호 없는 limited-precision 정수 |
| `sc_bigint<>` | §7.6.5 | **부호 있는 finite-precision 정수** |
| `sc_biguint<>` | §7.6.6 | 부호 없는 finite-precision 정수 |
| `sc_logic` | §7.9.2 | **4치 논리** |
| `sc_bv<>` | §7.9.6 | **2치 논리 벡터** |
| `sc_lv<>` | §7.9.6 | **4치 논리 벡터** |
| `sc_fixed<>` | §7.10.19 | **부호 있는 고정소수점 산술** |
| `sc_ufixed<>` | §7.10.20 | 부호 없는 고정소수점 산술 |

## 오류 처리 (Clause 8)

`sc_report`와 `sc_report_handler`가 일반적 오류 처리 메커니즘을 제공한다. SystemC 클래스 라이브러리 자신이 사용하며 사용자도 쓸 수 있다. report는 **severity**와 **message type**으로 분류되고, 각 범주마다 맞춤 동작(메시지 출력, 예외 던지기, 프로그램 abort 등)을 설정할 수 있다 (§8.2, §8.3).
