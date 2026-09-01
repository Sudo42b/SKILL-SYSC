# Clause 9 — Overview of TLM-2.0 and compliance with TLM-2.0 standard

LRM pp. 440–441.

Clause 10~17이 **SystemC Transaction-Level Modeling Standard, version 2.0 (TLM-2.0)**를 정의한다.
이 절은 **무엇을 써야 "TLM-2.0 준수"라고 주장할 수 있는가**를 규정한다. 준수 주장을 할 코드라면 여기서부터 시작.

---

## 9.1 Overview

### TLM-2.0의 구성

TLM-2.0은 다음으로 구성된다:

- core interfaces
- global quantum
- initiator socket / target socket
- generic payload & base protocol
- utilities

TLM-1의 core interfaces, analysis interface, analysis ports도 **포함**되지만, TLM-2.0 표준의 본체와는 **분리**되어 있다.

**TLM-2.0 core interfaces** = 다음 네 가지:

| core interface | 용도 |
|---|---|
| blocking transport interface | 트랜잭션 시작/종료 2개 timing point |
| non-blocking transport interface | 다중 phase / timing point |
| direct memory interface (DMI) | target 메모리 직접 접근 |
| debug transport interface | 지연·부작용 없는 디버그 접근 |

generic payload는 memory-mapped bus의 추상 모델링을 지원하며, **extension mechanism**으로 특정 버스 프로토콜을 모델링하면서도 interoperability를 최대화한다.

### 클래스 계층 (Figure 16)

TLM-2.0 클래스는 SystemC 클래스 라이브러리 **위에 layering** 된다.

```
TLM 2.0 classes
                       ┌ Interoperability layer ───────────────────┐
                       │  Generic payload & base protocol          │
                       │  Initiator & target sockets               │
                       │  Global quantum                           │
                       └───────────────────────────────────────────┘
 TLM-1:                  TLM-2 core interfaces:        Utilities:
 TLM-1 core interfaces   Blocking transport            Convenience sockets
 tlm_fifo                Non-blocking transport        Payload event queues
 Analysis interface      Direct memory interface       Quantum keeper
 Analysis ports          Debug transport interface     Instance-specific extensions
                            SystemC
```

### Interoperability layer

**interoperability layer** = TLM-2.0 core interfaces + sockets + generic payload + base protocol 을 **총칭하는 이름**.

- 최대 interoperability를 위해, 특히 memory-mapped bus 모델링에서는 이 네 가지를 **함께 사용할 것을 권장(recommended)**.
- generic payload가 부적절한 경우: core interfaces + initiator/target sockets 만, 또는 **core interfaces 단독**으로 대체 transaction type과 함께 쓰는 것이 **가능**하다.
- generic payload를 initiator/target socket 없이 core interfaces와 직접 쓰는 것도 기술적으로 가능하지만 **권장하지 않는다(not recommended)**.

### Utilities는 필수가 아니다

- bus model 간 interoperability를 달성하는 데 utilities 사용이 **엄밀히 필요하지는 않다**.
- 그럼에도 스타일 일관성을 위해 **가능한 곳에서는 사용할 것(should)**. utilities도 TLM-2.0 표준의 일부로 문서화·유지된다.

### Generic payload 확장의 대가

- generic payload는 주로 memory-mapped bus 모델링용이지만, 유사한 속성을 가진 **non-bus 프로토콜 모델링에도 쓸 수 있다(may)**.
- generic payload의 attribute와 phase는 특정 프로토콜을 위해 **확장 가능**하나, 표준 non-extended generic payload에서 벗어나는 정도에 비례해 **interoperability가 감소할 수 있다**.

### 코딩 스타일 제안 (비규범)

> 아래는 coding style 제안이며 **TLM-2.0 표준의 normative part가 아니다**.
> - 빠른 loosely-timed 모델: blocking transport interface + DMI + temporal decoupling
> - 더 정확한 approximately-timed 모델: non-blocking transport interface + payload event queues

---

## 9.2 Compliance with the TLM-2.0 standard

세 가지 준수 개념이 정의된다. **(a)는 구현(implementation)의 준수**, **(b)·(c)는 애플리케이션(application)의 준수**.

### a) TLM-2.0-compliant implementation

- 이 표준이 기술하는 **모든 TLM-2.0 클래스**를, 이 표준이 기술하는 **의미론(semantics)대로** 제공하는 구현.
- 여기에는 **TLM-2.0 interoperability layer와 TLM-2.0 utilities가 모두 포함**된다. → utilities는 애플리케이션에게는 선택이지만, **구현에게는 필수**.
- TLM-2.0-compliance만으로 표준 전체에 대한 full compliance가 되지는 **않는다**. 다만 TLM-2.0 클래스가 사용하는 **SystemC 부분집합에 대한 준수는 함의**한다 (1.4 참조).

### b) TLM-2.0 base-protocol-compliant model

애플리케이션의 일부로서, 이 표준이 기술하는 **TLM-2.0 base protocol의 모든 규칙을 지키는** 모델. 필연적으로 다음을 만족한다:

- 하나 이상의 SystemC module로 구성되고,
- protocol traits class **`tlm_base_protocol`로 특수화된 standard socket**을 가지며,
- **15.2에 정의된 모든 규칙을 정확히 준수**한다.

참조: base protocol 규칙 소개는 15.2.1, base protocol에서의 extension 사용은 14.2.2.

### c) TLM-2.0 custom-protocol-compliant model

애플리케이션의 일부로서:

- **사용자 정의 protocol traits class**(명시적으로 `tlm_base_protocol`이 **아닌** 것)로 특수화된 standard socket을 갖고,
- **generic payload를 사용**한다 — 적절한 경우 generic payload의 **extension 및 memory management 메커니즘 포함**.
- base protocol의 규칙을 지킬 **의무는 없다**. 그러나 두 모델을 접속하는 데 드는 엔지니어링 노력을 최소화하기 위해 **가능한 한 base protocol 규칙에 가깝게 따를 것을 권장(recommended)**. TLM-2.0으로 모델링되는 프로토콜 종류에 선험적 제한이 없으므로 이 권고는 필연적으로 비공식(informal)이지만, TLM-2.0 표준의 이득을 얻는 **핵심**이다.

참조: custom protocol 규칙과 base protocol의 관계는 14.2.3.

### standard socket의 정의

b)와 c)에서 **standard socket**이란 다음 타입의 객체를 말한다:

- `tlm_initiator_socket`
- `tlm_target_socket`
- 위 두 클래스 중 하나에서 **파생된 임의의 클래스**

### 준수 개념의 경계

TLM-2.0 class library의 **고립된 기능만 사용하는 모델**은 이 표준에 compliant일 수는 있으나,
**base-protocol-compliant도 custom-protocol-compliant도 아니다**.

---

## 준수 판정 체크리스트

| 주장하려는 것 | 필요 조건 |
|---|---|
| TLM-2.0-compliant **implementation** | interoperability layer + utilities 전부를 표준 의미론대로 제공 |
| TLM-2.0 **base-protocol-compliant** model | standard socket + `tlm_base_protocol` traits + 15.2 규칙 전부 준수 |
| TLM-2.0 **custom-protocol-compliant** model | standard socket + 사용자 정의 traits class (≠ `tlm_base_protocol`) + generic payload 사용 |
| 위 어느 것도 아님 | TLM-2.0 기능을 산발적으로만 사용 |

---

## 흔한 위반 (pitfalls)

- **socket 없이 generic payload를 core interface에 직접 물림** — LRM §9.1: 기술적으로 가능하나 not recommended. §9.2 b)/c) 모두 **standard socket을 요구**하므로 이 형태로는 protocol-compliant 주장이 불가능하다. `tlm_initiator_socket`/`tlm_target_socket`(또는 파생 클래스)을 사용하라.
- **SystemC port/export를 직접 쓰고 "TLM-2.0 준수"라고 주장** — LRM §9.2: standard socket이 아니면 base-/custom-protocol-compliant가 아니다.
- **base protocol에서 벗어난 phase/attribute를 추가하고 base-protocol-compliant 주장** — LRM §9.2 b): 15.2의 규칙을 **precisely** 모두 지켜야 한다. 벗어났다면 custom-protocol-compliant 쪽으로 분류하고 traits class를 `tlm_base_protocol`이 아닌 것으로 특수화하라.
- **custom protocol에서 generic payload를 버리고 자체 transaction type 사용** — LRM §9.2 c): custom-protocol-compliant는 **generic payload 사용을 요구**한다. 자체 transaction type을 쓰면 §9.1의 "core interfaces 단독 사용" 경로일 뿐 custom-protocol-compliant가 아니다.
- **utilities를 빼고 "TLM-2.0-compliant 구현"이라 표기** — LRM §9.2 a): 구현은 interoperability layer와 **utilities를 모두** 제공해야 한다.
- **generic payload를 마구 확장하고 interoperability를 기대** — LRM §9.1: 표준 non-extended generic payload에서 벗어난 정도만큼 interoperability가 줄어든다.
