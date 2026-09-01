# Clause 10 — Introduction to TLM-2.0

LRM pp. 443–452.

TLM-2.0의 **개념 지도**. coding style(loosely-timed / approximately-timed), initiator·target·socket·transaction bridge,
DMI·debug interface, namespace, header file·version macro를 정의한다.
대부분은 informative한 배경 설명이지만 **10.7 (namespace)과 10.8 (header file / version)은 `shall` 규칙**을 포함한다.

---

## 10.1 Background — TLM-1의 세 가지 한계

TLM-1은 트랜잭션을 값(value) 또는 const reference로 전달하는 core interface 집합을 정의했다.
memory-mapped bus 및 on-chip 통신망 모델링 관점에서 세 가지 결함이 있었다:

| # | TLM-1의 결함 | TLM-2.0의 해법 |
|---|---|---|
| a | 표준 transaction class가 없음 → 각 애플리케이션이 비표준 클래스를 만들어 서로 다른 출처의 모델 간 interoperability가 매우 나쁨 | **generic payload** |
| b | timing annotation에 대한 명시적 지원 없음 → 모델 간 타이밍 정보 전달의 표준 방식 부재 | blocking/non-blocking transport interface에 **timing annotation 함수 인자** 추가 |
| c | 모든 transaction 객체·데이터를 value 또는 const reference로 전달 → 일부 use case에서 시뮬레이션 저하 | transaction 객체를 **non-const reference**로 전달 |

---

## 10.2 Transaction-level modeling, use cases, and abstraction

TLM-2.0은 use case마다 abstraction level을 정의하는 대신, **interface(API)와 coding style을 구분**하는 접근을 취한다.

- **interface**: 트랜잭션 레벨 모델 구현을 위한 저수준 프로그래밍 메커니즘. **표준의 normative part**이며 interoperability를 보장한다.
- **coding style**: 여러 use case에 적합하지만 use case에 **고정되지는 않은** 스타일. 각 coding style은 기능·타이밍·통신에 걸친 추상화 범위를 지원한다. 사용자가 **자신만의 coding style을 만들어도 원칙적으로 가능**.

```
Use cases:   Software        Software         Architectural      Hardware
             development     performance      analysis           verification
Coding styles:   ├──── Loosely-timed ────┤
                              ├──── Approximately-timed ────┤
Mechanisms:  Blocking iface · DMI · Quantum · Sockets · Generic payload · Phases · Non-blocking iface
```
(Figure 17)

### 왜 transaction-level인가

- 단일 소프트웨어 스레드로 된 untimed functional model(= algorithmic model)은 C 함수 또는 단일 SystemC process로 작성 가능하지만, **transaction-level이 아니다** — 트랜잭션은 정의상 통신의 추상화인데 단일 스레드 모델에는 프로세스 간 통신이 없다.
- transaction-level model은 동시 실행과 통신을 시뮬레이션하기 위해 **여러 SystemC process를 필요로 한다**.

### 제어 양보(yield)

- SystemC는 **cooperative multitasking** 모델 — 실행 중인 process는 다른 process에 의해 preempt되지 **않는다**.
- process는 thread process면 `wait` 호출로, method process면 커널로 return함으로써 제어를 양보한다.
- `wait` 호출은 보통 API 뒤에 숨겨지며, 그 API는 타이밍 정보에 의존할 수도 아닐 수도 있는 추상/구체 프로토콜을 모델링한다.

### 동기화의 강약

| 동기화 | 의미 | SystemC 구현 |
|---|---|---|
| **strong** | 통신 이벤트의 순서가 사전에 정밀히 결정됨 | FIFO 또는 semaphore로 쉽게 구현. 원칙적으로 **시뮬레이션 시간을 진행시키지 않고도** 실행 가능한 완전 untimed 스타일 허용 |
| **weak** | 통신 이벤트의 순서가 개별 process의 상세 타이밍에 부분적으로 좌우됨 | — |

- 이런 의미의 untimed 모델링은 **TLM-2.0의 범위 밖**이다.
- 여러 임베디드 소프트웨어 스레드를 병렬 실행하는 빠른 virtual platform 모델은 strong·weak 어느 쪽도 쓸 수 있고, 이 표준에서 그 coding style을 **loosely-timed**라 부른다.
- 트랜잭션마다 프로토콜 각 phase의 시작·끝을 표시하는 **다수의 프로토콜 특화 timing point**를 연관시켜야 하는 더 상세한 모델의 스타일을 **approximately-timed**라 부른다. timing point 개수를 적절히 고르면 **매 클럭 사이클마다 컴포넌트 모델을 실행하지 않고도** 높은 타이밍 정확도를 낼 수 있다.

---

## 10.3 Coding styles

### 10.3.1 Overview

- coding style = **잘 어울리는 프로그래밍 언어 관용구의 집합**이지, 특정 추상화 레벨이나 소프트웨어 프로그래밍 인터페이스가 **아니다**.
- 이 표준은 단순·명확성을 위해 **loosely-timed와 approximately-timed 두 가지만** 상술한다.
- coding style은 그 본성상 정밀하게 정의되지 않으며, **TLM-2.0 core interface를 지배하는 규칙은 coding style과 독립적으로 정의**된다.
- TLM-1·TLM-2.0 메커니즘 위에 다른 coding style을 정의하는 것도 원칙적으로 가능.

### 10.3.2 Untimed coding style

- TLM-2.0은 untimed coding style에 대해 **명시적 대비를 하지 않는다**. 현대의 모든 버스 기반 시스템은 임베디드 프로세서 위 소프트웨어를 모델링하려면 어떤 형태로든 시간 개념을 요구하기 때문.
- 다만 **untimed 모델링은 TLM-1 core interfaces가 지원**한다.

> untimed라는 용어가 "정확도가 명시되지 않은 제한적 타이밍 정보를 담은 모델"을 가리키는 데 쓰이기도 하는데, TLM-2.0에서 그런 모델은 **loosely-timed로 분류**된다.

### 10.3.3 Loosely-timed coding style and temporal decoupling

**사용 인터페이스: blocking transport interface**

- blocking transport interface는 트랜잭션당 **timing point 2개만** 허용 — blocking transport 함수의 **호출**과 **반환**.
- base protocol의 경우: 첫 timing point는 **request의 시작**, 둘째는 **response의 시작**. 두 timing point는 **같은 시뮬레이션 시각일 수도, 다른 시각일 수도 있다**.
- 적합 use case: **MPSoC virtual platform 모델을 이용한 소프트웨어 개발**(소프트웨어에 OS가 하나 이상 포함될 수 있음). timer와 interrupt 모델링을 지원하므로 **OS를 부팅하고 타깃 머신에서 임의 코드를 실행하기에 충분**하다.

**Temporal decoupling**

개별 SystemC process가 시뮬레이션 시간을 실제로 진행시키지 않은 채 로컬 "time warp"에서 **앞서 달리도록(run ahead)** 허용하고, 시스템 나머지와 동기화가 필요한 지점에서만 맞추는 기법.

- 데이터·코드 지역성을 높이고 시뮬레이터 스케줄링 오버헤드를 줄여 **매우 빠른 시뮬레이션**을 낼 수 있다.
- 각 process는 정해진 time slice(**quantum**) 동안 달린 뒤 다음 process로 전환하거나, 명시적 동기화 지점에 도달하면 제어를 양보한다.

배경 — 순수 SystemC의 동작:
- 스케줄러는 시뮬레이션 시간을 **다음 이벤트 시각으로 진행**시키고, 그 시각에 실행될 process나 그 이벤트에 sensitive한 process를 실행한다.
- SystemC process는 **현재 시뮬레이션 시각에서만** 실행되며(`sc_time_stamp`), 변수를 읽거나 쓸 때 **현재 시각의 상태**에 접근한다.
- process 실행이 끝나면 반드시 커널에 제어를 반환해야 한다.
- 세밀한 수준으로 작성된 모델에서는 **이벤트 스케줄링과 process context switch 오버헤드가 시뮬레이션 속도의 지배 요인**이 된다.

외부 의존이 발생했을 때 process의 **두 가지 선택**:

| 선택 | 의미 | 대가 |
|---|---|---|
| **force synchronization** | 양보하여 다른 모든 process가 정상 실행되고 시뮬레이션 시간이 따라잡을 때까지 기다림 | 표준 SystemC 시뮬레이션 의미론과 **기능적으로 합치(congruent)** |
| **현재 값을 sample/update하고 계속** | 현재 값에 접근하고 진행 | 모델링 대상의 통신·타이밍에 관한 **가정에 의존** — 값을 너무 이르게/늦게 읽거나 써도 문제없다는 가정. virtual platform 시뮬레이션에서는 대개 유효 (소프트웨어 스택이 저수준 하드웨어 타이밍 세부에 의존하면 안 되므로). 타이밍 정확도 일부 손실 가능 |

- **각 process가 스스로** 모델 기능을 깨지 않고 앞서 달릴 수 있는지 판단할 책임이 있다.
- **temporal decoupling은 loosely-timed coding style의 특징**이다.

**Global quantum**

- 제한 없이 앞서 달리면 SystemC 스케줄러가 동작할 수 없고 다른 process가 실행 기회를 얻지 못한다 → **global quantum**이 앞서 달릴 수 있는 시간의 상한을 부과한다.
- quantum은 **애플리케이션이 설정**하며, 그 값은 **시뮬레이션 속도와 정확도의 트레이드오프**다.
  - 너무 작은 quantum: 너무 잦은 yield·동기화 → 시뮬레이션 저하.
  - 너무 큰 quantum: 시스템 전반의 타이밍 불일치 → 심하면 시스템이 동작을 멈출 수 있음.

> 예: 프로세서 + 메모리 + timer + 느린 외부 peripheral 시스템. 소프트웨어는 대부분 시간을 시스템 메모리에서 명령을 fetch/execute하는 데 쓰고, 1 ms마다 timer interrupt가 걸릴 때만 나머지와 상호작용한다. ISS는 quantum 1 ms까지 앞서 달리며 메모리 모델에 직접 접근하고 timer interrupt 주기로만 peripheral과 동기화하면 된다. 모델 상세에 따라 temporal decoupling만으로 약 **10X**, **DMI와 결합 시 100X** 속도 향상이 가능하다.

- 일부 process만 temporally decoupled이고 나머지는 아닌 것, process마다 다른 quantum 값을 쓰는 것 모두 **가능**하다. 다만 temporally decoupled가 아닌 process는 **시뮬레이션 속도 병목이 되기 쉽다**.
- TLM-2.0에서 temporal decoupling은 **`tlm_global_quantum` 클래스**와 blocking/non-blocking transport interface의 **timing annotation**으로 지원된다. utility class **`tlm_quantumkeeper`**가 global quantum에 편리하게 접근하는 수단을 제공한다.

### 10.3.4 Synchronization in loosely-timed models

- untimed 모델은 실행 중 정해진 지점에서 initiator 간 제어를 넘기기 위해 **명시적 동기화 지점**(`wait` 호출 또는 blocking method call)의 존재에 의존한다.
- loosely-timed 모델도 결정론적 실행을 보장하는 데 명시적 동기화의 도움을 받을 수 있으나, **명시적 동기화 지점이 없어도 진행할 수 있다** — 각 initiator가 time quantum 끝까지만 앞서 달린 뒤 제어를 양보하기 때문.
- **synchronization-on-demand**: time quantum 끝에 도달하기 전에 스케줄러에 제어를 양보하여 타이밍 정확도를 높이는 기법.
- **time quantum 메커니즘은 올바른 시스템 동기화를 보장하기 위한 것이 아니다.** 스케줄러 기반 시뮬레이션 환경에서 여러 system initiator가 진행하게 해주는 **시뮬레이션 메커니즘**일 뿐이며, **시스템 레벨의 명시적 동기화 설계를 대체하지 않는다**.

### 10.3.5 Approximately-timed coding style

**사용 인터페이스: non-blocking transport interface**

- 적합 use case: **architectural exploration, performance analysis**.
- non-blocking transport interface는 timing annotation과, 트랜잭션 생애 동안의 **다중 phase 및 timing point**를 제공한다.
- 트랜잭션이 **여러 phase로 분해**되고, phase 간 전이를 명시적 timing point가 표시한다.
- **base protocol에서는 정확히 4개의 timing point** — request의 시작·끝, response의 시작·끝.
- 특정 프로토콜은 timing point를 더 추가해야 할 수 있으며, 그 경우 **generic payload와의 직접 호환성을 잃을 수 있다**.
- non-blocking transport interface를 phase 2개만으로 써서 트랜잭션의 시작·끝만 표시하는 것도 가능하지만, **loosely-timed 모델링에는 blocking transport interface가 일반적으로 선호된다**.
- approximately-timed는 타이밍 정확도 요구 때문에 **일반적으로 temporal decoupling을 활용할 수 없다**. 대신 각 process가 SystemC 스케줄러와 **lock step으로 실행**되고, process 상호작용에 구체적 delay가 annotate된다.
- approximately-timed 모델을 만들려면 일반적으로 **write/read 명령의 data transfer time과 target의 latency**를 annotate하는 것으로 충분하다. base protocol에서 data transfer time은 사실상 연속된 두 request 또는 두 response 사이의 **minimum initiation interval(accept delay)**과 같다.
- annotate된 delay는 SystemC 스케줄러 호출, 즉 **`wait(delay)` 또는 `notify(delay)`**로 구현된다.

### 10.3.6 Coding style 특성 비교

| | timing point | 시뮬레이션 시간 | 진행/양보 방식 |
|---|---|---|---|
| **Loosely-timed** | 트랜잭션당 **2개** (시작·끝) | 사용하되 process가 시뮬레이션 시간과 **temporally decoupled 가능** | 각 process가 시뮬레이션 시간보다 얼마나 앞섰는지 tally를 유지. **명시적 동기화 지점 도달** 또는 **time quantum 소진** 시 yield |
| **Approximately-timed** | 트랜잭션당 **다수** | process가 일반적으로 SystemC 시뮬레이션 시간과 **lock-step** 필요 | process 상호작용에 annotate된 delay를 **time-out(`wait`) 또는 timed event notification**으로 구현 |
| **Untimed** | — | 시뮬레이션 시간 개념이 **불필요** | **명시적·사전 결정된 동기화 지점**에서 yield |

### 10.3.7 Switching between loosely-timed and approximately-timed

- 모델은 시뮬레이션 **도중에** loosely-timed와 approximately-timed coding style 사이를 **전환할 수 있다**.
- 의도: reset·boot 시퀀스는 loosely-timed로 빠르게 통과시키고, 흥미로운 단계에 도달하면 approximately-timed로 전환해 상세 분석.

### 10.3.8 Cycle-accurate modeling

- cycle-accurate 모델링은 **현재 TLM-2.0의 범위 밖**이다.
- SystemC와 TLM-1만으로 cycle-accurate 모델을 만드는 것은 **가능**하지만, cycle-accurate coding style의 표준화 요구는 미해결 과제로 남아 있으며 향후 Accellera Systems Initiative 표준이 다룰 수 있다.
- **원칙상으로만**: 적절한 phase 집합과 규칙을 정의하면 approximately-timed coding style을 cycle-accurate까지 확장할 수 있다. TLM-2.0 릴리스에 그럴 만한 기계 장치는 충분히 들어 있으나 **세부는 정해지지 않았다**.

### 10.3.9 Blocking versus non-blocking transport interfaces — 선택 기준

blocking과 non-blocking transport interface는 **서로 다른 타이밍 상세 수준을 지원하기 위해 존재하는 별개의 인터페이스**다.

| | blocking transport | non-blocking transport |
|---|---|---|
| 모델링 가능한 것 | 트랜잭션의 **시작과 끝만** | 트랜잭션을 **다중 timing point**로 분해 |
| 함수 호출 | **단일 함수 호출** 내에서 트랜잭션 완료 | 트랜잭션 하나에 **복수 함수 호출**이 일반적 |
| 강점 | 단일 호출로 트랜잭션을 완료할 수 있는 모델에 **단순한 coding style** 제공 | 트랜잭션 하나에 **다수 timing point 연관** 지원 |
| temporal decoupling | 원칙적으로 지원 | 원칙적으로 지원하나, approximately-timed의 다중 timing point 때문에 **속도 이득이 무효화되기 쉽다** |

**interoperability 규칙**

- 두 인터페이스는 interoperability를 위해 **하나의 인터페이스로 결합**되어 있다.
- 트랜잭션을 개시하는 모델은 coding style에 따라 blocking 또는 non-blocking (또는 **둘 다**)을 사용해도 된다.
- **TLM-2.0 transport interface를 제공하는 모든 모델은 최대 interoperability를 위해 blocking과 non-blocking 형태를 모두 제공할 의무가 있다.** 다만 **내부적으로 둘 다 구현할 의무는 없다.**
- TLM-2.0은 들어오는 blocking/non-blocking transport call을 각각 non-blocking/blocking call로 자동 변환하는 메커니즘(**convenience socket**)을 제공한다.
  - 변환에는 비용이 따르며, 특히 **들어오는 non-blocking call을 blocking 구현으로 변환**하는 쪽이 비싸다.
  - 다만 approximately-timed 모델의 존재 자체가 이미 시뮬레이션 속도에 부정적이므로 오버헤드는 상쇄되는 편.
- C++ **static typing 규칙이 두 인터페이스의 구현을 강제**하지만, initiator는 blocking과 non-blocking 중 어느 메서드를 호출할지 **동적으로 선택**할 수 있다. 서로 다른 initiator가 다른 메서드를 호출하거나, 하나의 initiator가 실행 중 blocking↔non-blocking을 전환하는 것도 가능하다.
- 같은 target에 대한 blocking·non-blocking transport call의 **혼용 및 순서를 지배하는 규칙이 이 표준에 포함**되어 있다.

### 10.3.10 Use case와 coding style 매핑 (Table 52)

| Use case | Coding style |
|---|---|
| Software application development | Loosely-timed |
| Software performance analysis | Loosely-timed |
| Hardware architectural analysis | Loosely-timed **또는** approximately-timed |
| Hardware performance verification | Approximately-timed **또는** cycle-accurate |
| Hardware functional verification | Untimed (verification environment), loosely-timed 또는 approximately-timed |

---

## 10.4 Initiators, targets, sockets, and transaction bridges

### 역할 정의

| 역할 | 정의 |
|---|---|
| **initiator** | 트랜잭션을 개시할 수 있는 module — 새 transaction 객체를 생성하고 core interface 중 하나의 메서드를 호출해 넘긴다 |
| **target** | 트랜잭션의 **최종 목적지** 역할을 하는 module |
| **interconnect component** | 트랜잭션에 접근하되 그 트랜잭션의 initiator도 target도 아닌 module. 전형적 예: **arbiter, router** |

- write 트랜잭션: initiator(예: 프로세서)가 target(예: 메모리)에 데이터를 쓴다. read는 그 반대.
- **initiator / interconnect / target 역할은 동적으로 바뀔 수 있다.** 한 컴포넌트가 어떤 트랜잭션에는 interconnect로, 다른 트랜잭션에는 target으로 동작할 수 있다.

### 경로 (forward / backward / return path)

전형적 transaction 객체의 생애:

1. initiator가 transaction 객체를 생성하고 transport interface(blocking 또는 non-blocking) 메서드의 인자로 전달.
2. arbiter 같은 interconnect component가 그 메서드를 구현 — 객체의 attribute를 읽은 뒤 다음 transport call로 넘길 수 있다.
3. router 같은 두 번째 interconnect component가 세 번째 transport call로 target(예: memory)에 전달. **interconnect component의 실제 개수는 트랜잭션마다 다르며, 하나도 없을 수도 있다.**
4. 이 메서드 호출 시퀀스를 **forward path**라 한다.
5. target에서 트랜잭션이 실행되고, transaction 객체는 두 방식 중 하나로 initiator에 되돌아온다:
   - **return path**: transport 메서드 호출이 풀리면서(unwind) 반환과 함께 실려 옴.
   - **backward path**: target→initiator 방향의 반대 경로로 **명시적 transport 메서드 호출**을 하여 전달.
6. 어느 쪽인지는 **non-blocking transport 메서드의 반환값이 결정**한다.

> 엄밀히는 forward path와 backward path에 각각 대응하는 return path가 둘 있으나, 문맥상 의미는 대개 명확하다.

- **forward path** = initiator 또는 interconnect component가 다른 interconnect component 또는 target 방향으로 interface method call을 하는 호출 경로.
- **backward path** = target 또는 interconnect component가 다른 interconnect component 또는 initiator 방향으로 되돌려 interface method call을 하는 호출 경로.
- initiator와 target 사이 전체 경로는 여러 **hop**으로 구성된다. 한 hop = **initiator socket 하나가 target socket 하나에 bound**된 것, 즉 인접한 두 컴포넌트를 잇는다.
- **hop 개수 = 그 경로 위의 interconnect component 개수 + 1.**
- generic payload를 사용할 때 forward path와 backward path는 **동일한 컴포넌트·소켓 집합을 반대 방향으로 통과해야 한다(should)**.

### Socket

- forward·backward 양방향을 지원하려면 컴포넌트 간 각 연결에 **port와 export가 모두 필요**하고 둘 다 bind되어야 한다. initiator socket과 target socket이 이를 편하게 해준다.
- **initiator socket** = forward path용 interface method call을 위한 **port** + backward path용 **export**. **target socket**은 그 반대.
  - 더 구체적으로: initiator socket은 **`sc_port`에서 파생되고 `sc_export`를 가진다**. target socket은 그 반대.
- initiator/target socket 클래스는 SystemC **port binding 연산자를 오버로드**하여 forward·backward path를 **암묵적으로 함께 bind**한다.
- socket은 transport interface뿐 아니라 **DMI와 debug transport interface도 캡슐화**한다.
- socket 사용 시: initiator 컴포넌트는 최소 하나의 initiator socket, target 컴포넌트는 최소 하나의 target socket, interconnect 컴포넌트는 **각각 최소 하나씩**을 가진다.
- 한 컴포넌트가 **서로 다른 transaction type을 나르는 여러 socket**을 가질 수 있고, 그 경우 단일 컴포넌트가 복수의 독립 트랜잭션에 대해 initiator 또는 target으로 동작할 수 있다.
- **최대 interoperability·편의성·일관된 coding style을 위해 TLM-2.0 socket 사용을 권장(recommended)**. SystemC port와 export를 TLM-2.0 core interface와 직접 쓰는 것도 가능하지만 **권장하지 않는다(not recommended)**.

### Transaction bridge — bus bridge 모델링의 두 선택지

| 방식 | 동작 | 트레이드오프 |
|---|---|---|
| **interconnect component**로 모델링 | **단일 transaction 객체의 포인터를 그대로 넘김** | **시뮬레이션 속도에 최선** |
| **transaction bridge**로 모델링 (두 개의 분리된 TLM-2.0 트랜잭션 사이) | transaction 객체를 **복사해야 함** | 두 트랜잭션이 **서로 다른 attribute를 가질 수 있어** 훨씬 유연 |

---

## 10.5 DMI and debug transport interfaces

DMI와 debug transport interface는 transport interface와는 **구별되는 특수 인터페이스**로, target이 소유한 메모리 영역에 대한 **직접 접근**과 **디버그 접근**을 제공한다.

| | 목적 | 방향 |
|---|---|---|
| **DMI (direct memory interface)** | DMI request가 승인되면 initiator가 transport interface가 쓰던 **interconnect component 경유 경로를 우회**. **loosely-timed 시뮬레이션에서 통상적 메모리 트랜잭션을 가속**하는 것이 목적 | **forward (initiator→target)와 backward (target→initiator) 인터페이스 모두** 보유 (11.3) |
| **debug transport interface** | 통상 트랜잭션에 수반되는 **delay나 side-effect 없는** 디버그 접근 | **forward 인터페이스만** (11.4) |

---

## 10.6 Combined interfaces and sockets

- blocking·non-blocking transport interface가 DMI·debug transport interface와 함께 **표준 initiator/target socket 안에서 결합**된다.
- 네 인터페이스 모두(두 transport + DMI + debug)를 **주어진 target에 대해 병렬로 사용할 수 있다** (이 표준의 규칙을 따르는 한).
- 이 combined interface는 **TLM-2.0 컴포넌트 간 interoperability의 핵심 중 하나**이며, 다른 하나는 **generic payload** (13.1).
- 표준 target socket은 네 인터페이스를 전부 제공하므로, 각 target 컴포넌트는 사실상 **네 인터페이스의 메서드를 모두 구현해야** 한다.
  - 다만 blocking/non-blocking transport interface의 설계와 convenience socket의 변환 기능 덕분에, 주어진 target은 모델의 속도·정확도 요구에 따라 **blocking 또는 non-blocking transport 메서드 중 하나만 구현하면 된다** — 둘 다는 아니어도 된다.
- initiator는 속도·정확도 요구에 따라 **core interface 중 임의의 것 또는 전부**를 호출하도록 선택할 수 있다. coding style이 적절한 인터페이스 기능 집합의 선택을 안내한다.

| 전형적 initiator | 호출하는 인터페이스 |
|---|---|
| loosely-timed initiator | **blocking transport + DMI + debug** |
| approximately-timed initiator | **non-blocking transport + debug** |

---

## 10.7 Namespaces

### Rules

- TLM-2.0 클래스는 **두 개의 top-level C++ namespace `tlm`과 `tlm_utils`에 선언되어야 shall**.
- 특정 구현이 이 두 namespace 안에 **중첩 namespace를 두어도 된다(may)**. 그러나 그런 중첩 namespace를 **애플리케이션에서 사용하면 안 된다 shall not**.

| namespace | 내용 |
|---|---|
| `tlm` | memory-mapped bus 모델링을 위한 **interoperability interface를 구성하는 클래스** |
| `tlm_utils` | memory-mapped bus 모델 간 인터페이스에서 interoperability에 엄밀히 필요하지는 않지만 **TLM-2.0 표준의 정당한 일부**인 utility 클래스 |

---

## 10.8 Header files and version numbers

### 10.8.1 Overview

| 헤더 | 규칙 |
|---|---|
| `tlm` | 애플리케이션은 이 헤더를 `#include` 할 것을 **권장(should)**. **TLM-2.0 interoperability layer의 모든 public 선언을 포함해야 shall** |
| `tlm.h` | `tlm`과 **정확히 동일한 선언·매크로 집합을 포함해야 shall** — 두 헤더는 **상호 교환 가능(may)**. 이전 버전 TLM-2.0과의 **하위 호환용**이며 향후 **deprecated 될 수 있다** |

- **TLM-2.0 utilities는 헤더 파일 `tlm` 또는 `tlm.h`에 있으면 안 된다 shall not.**
- 애플리케이션은 사용하려는 TLM-2.0 utility의 헤더 파일도 **명시적으로 `#include` 할 것을 권장(should)**. 이 헤더들은 **`tlm_utils`라는 이름의 디렉터리에 놓여야 shall**.
- 각 utility 헤더의 **정확한 파일 이름은 해당 클래스의 절에서 정의**된다.

### 10.8.2 Software version information

- 헤더 `tlm`과 `tlm.h`는 TLM-2.0 소프트웨어 배포판의 **버전 번호 정보를 제공하는 매크로·상수·함수 집합을 포함해야 shall**.
- 애플리케이션은 이 매크로와 상수를 **사용해도 된다(may)**.
- 이 절의 매크로·상수 값은 **SystemC의 대응 정의(8.6.5)와 독립적일 수 있다(may)**.

### 10.8.3 Definitions

```cpp
namespace tlm
{

#define TLM_VERSION_MAJOR          implementation-defined_number   // For example, 2
#define TLM_VERSION_MINOR          implementation-defined_number   // 0
#define TLM_VERSION_PATCH          implementation-defined_number   // 1
#define TLM_VERSION_ORIGINATOR     implementation-defined_string   // "OSCI"
#define TLM_VERSION_RELEASE_DATE   implementation-defined_date     // "20090329"
#define TLM_VERSION_PRERELEASE     implementation-defined_string   // "beta"
#define TLM_IS_PRERELEASE          implementation-defined_bool     // 1
#define TLM_VERSION                implementation-defined_string   // "2.0.1_beta-OSCI"
#define TLM_COPYRIGHT              implementation-defined_string

const unsigned int  tlm_version_major;
const unsigned int  tlm_version_minor;
const unsigned int  tlm_version_patch;
const std::string   tlm_version_originator;
const std::string   tlm_version_release_date;
const std::string   tlm_version_prerelease;
const bool          tlm_is_prerelease;
const std::string   tlm_version_string;
const std::string   tlm_copyright_string;

inline const char*  tlm_release();
inline const char*  tlm_version();
inline const char*  tlm_copyright();

} // namespace tlm
```

### 10.8.4 Rules

| # | 규칙 |
|---|---|
| a | 각 `implementation-defined_number`는 문자 집합 **[0–9]의 십진 숫자 시퀀스**로 구성되어야 shall. **따옴표로 감싸지 않는다** |
| b | originator와 pre-release 문자열은 각각 문자 집합 **[A–Z][a–z][0–9]_**의 시퀀스로 구성되고 **따옴표로 감싸야 shall** |
| c | version release date는 **ISO 8601 basic format**의 `YYYYMMDD` 형태여야 shall. 8자 각각이 십진 숫자이며 **따옴표로 감싼다** |
| d | `TLM_IS_PRERELEASE` 플래그는 **0 또는 1**이어야 shall. **따옴표로 감싸지 않는다** |
| e | version string은 `"major.minor.patch_prerelease-originator"` 또는 `"major.minor.patch-originator"` 값으로 설정되어야 shall. 각 요소는 대응 문자열 값(감싸는 따옴표 제외)이며, **prerelease 문자열의 유무는 `TLM_IS_PRERELEASE` 플래그 값에 따라야 shall** |
| f | copyright string은 저작권 고지로 설정할 것을 **권장(should)** |
| g | 각 상수는 **같은 이름의 매크로가 정의한 값을 적절한 데이터 타입으로 변환한 값으로 초기화되어야 shall** |
| h | 함수 `tlm_release`와 `tlm_version`은 각각 **version string을 C 문자열로 변환한 값을 반환해야 shall** |
| i | 함수 `tlm_copyright`는 **copyright string을 C 문자열로 변환한 값을 반환해야 shall** |

---

## 흔한 위반 (pitfalls)

- **구현의 중첩 namespace를 애플리케이션에서 직접 참조** — LRM §10.7: 구현은 `tlm`/`tlm_utils` 안에 중첩 namespace를 둘 수 있으나 애플리케이션이 그것을 사용하면 안 된다 shall not. `tlm::` / `tlm_utils::`만 쓸 것.
- **`#include "tlm"` 하나로 utility까지 쓸 수 있다고 가정** — LRM §10.8.1: TLM-2.0 utilities는 `tlm`/`tlm.h`에 **있으면 안 된다 shall not**. 사용할 utility의 헤더를 `tlm_utils` 디렉터리에서 명시적으로 include할 것(should).
- **새 코드에서 `tlm.h`를 사용** — LRM §10.8.1: `tlm.h`는 하위 호환용이며 향후 deprecated 될 수 있다. `tlm`을 쓸 것(should).
- **transport interface를 제공하면서 blocking 또는 non-blocking 중 한쪽만 노출** — LRM §10.3.9: TLM-2.0 transport interface를 제공하는 모델은 **양쪽 형태를 모두 제공할 의무**가 있다. 내부 구현은 한쪽만 해도 되며, 나머지는 convenience socket 변환에 맡긴다.
- **SystemC `sc_port`/`sc_export`를 TLM-2.0 core interface에 직접 물림** — LRM §10.4: 가능하지만 **not recommended**. socket이 forward/backward path 바인딩과 DMI·debug 캡슐화를 함께 처리한다.
- **generic payload를 쓰면서 forward path와 backward path를 다른 컴포넌트 집합으로 구성** — LRM §10.4: 두 경로는 동일 컴포넌트·소켓 집합을 반대 방향으로 통과해야 한다(should).
- **approximately-timed 모델에 temporal decoupling으로 속도를 기대** — LRM §10.3.5 / §10.3.9: 타이밍 정확도 요구 때문에 일반적으로 활용 불가하며, 다중 timing point가 속도 이득을 무효화한다.
- **time quantum을 시스템 동기화 수단으로 사용** — LRM §10.3.4: quantum은 시뮬레이션 메커니즘일 뿐 **시스템 레벨 명시적 동기화 설계의 대안이 아니다**.
- **quantum을 무작정 크게 설정** — LRM §10.3.3: 너무 크면 시스템 전반에 타이밍 불일치가 생겨 시스템이 동작을 멈출 수도 있다.
- **untimed 모델링을 TLM-2.0 인터페이스로 시도** — LRM §10.3.2: TLM-2.0은 untimed coding style에 명시적 대비를 하지 않는다. untimed 모델링은 **TLM-1 core interfaces**가 지원한다.
- **base protocol에 timing point를 추가하고 generic payload 호환을 기대** — LRM §10.3.5: base protocol의 timing point는 정확히 4개이며, 추가 timing point는 generic payload와의 직접 호환성 상실을 초래할 수 있다.
- **TLM 버전 매크로를 SystemC 버전 매크로와 동일하다고 가정** — LRM §10.8.2: 두 값 집합은 서로 **독립적일 수 있다(may)**.
