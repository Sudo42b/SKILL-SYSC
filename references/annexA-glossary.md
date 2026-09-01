# Annex A — Glossary (용어집)

LRM pp. 591–607.

**이 Annex는 informative(참고)이며 규범(normative)이 아니다.**

이 용어집은 본 표준에서 사용되는 다수의 용어·구절에 대한 짧고 비형식적인 설명을 담는다. 적절한 경우 각 용어의 완전하고 형식적인 정의는 표준 본문에 있다. 각 항목은 본문 내 정의의 절 번호를 담거나, 그 용어가 ISO/IEC 14882:2017에 정의되어 있음을 표시한다(= "C++ 용어").

용어는 원문 영어, 정의는 한국어. 알파벳 순서.

---

## A

- **abstract class** — 순수 가상 함수를 하나 이상 갖거나 상속했으며 그것이 비순수 가상 함수로 오버라이드되지 않은 클래스. (C++ 용어)
- **adapter** — 트랜잭션 레벨 인터페이스를 핀 레벨 인터페이스(일반적 의미의 interface)에 연결하거나, 두 개의 트랜잭션 레벨 인터페이스를(흔히 서로 다른 추상화 레벨의) 서로 연결하는 모듈. adapter는 서로 다른 protocol type으로 특수화된 두 socket 사이를 변환하는 데 쓰일 수 있다. 참조: bridge; transactor. (§14.2.3)
- **application** — 최종 사용자가 작성한, SystemC 또는 TLM-2.0 클래스 라이브러리를 사용하는(클래스를 쓰고, 함수를 호출하고, 매크로를 쓰는 등) C++ 프로그램. 애플리케이션은 C++의 기능과 SystemC의 기능을 적절하다고 보는 만큼 적게 또는 많이 사용해도 된다. (§3.1.2)
- **approximately-timed** — 모델의 외부 관측 가능 상태와 대응하는 상세 reference model의 상태 사이에 일대일 매핑이 존재하며, 그 매핑이 상태 전이의 **순서는 보존하지만 정확한 타이밍은 보존하지 않는** 모델링 스타일. 타이밍 정확도의 정도는 undefined. 참조: cycle-approximate. (§10.3.5)
- **argument** — 함수 호출(또는 매크로·템플릿 인스턴스화)의 괄호로 둘러싸인 쉼표 구분 목록 안의 표현식. actual argument라고도 한다. 참조: parameter. (C++ 용어)
- **attach** — `sc_object`의 멤버 함수 `add_attribute`를 호출하여 어트리뷰트를 객체에 연관시키는 것. (§5.16.8)
- **attribute (of a transaction)** — 트랜잭션의 일부이며 트랜잭션과 함께 운반되고 트랜잭션 객체의 멤버로 구현되는 데이터. 모델링 대상 버스·프로토콜에 내재된 어트리뷰트와, 시뮬레이션 모델의 인공물인 어트리뷰트(예: 타임스탬프)를 포함할 수 있다. (§11.2.3, §14.7)
- **automatic deletion** — automatic deletion으로 표시된 generic payload extension은 트랜잭션 수명이 끝날 때, 즉 트랜잭션 참조 카운트가 0에 도달할 때 삭제된다. (§14.21.4)

## B

- **backward path** — target 또는 interconnect component가 다른 interconnect component 또는 initiator 방향으로 되돌아가며 interface method call을 수행하는 호출 경로. (§10.4)
- **base class sub-object** — 주어진 객체의 기저 클래스 타입을 갖는 sub-object. 참조: sub-object. (C++ 용어)
- **base protocol** — generic payload와 `tlm_phase` 타입으로 구성된 protocol traits class 및 이에 연관된 프로토콜 규칙 집합. 트랜잭션 레벨 모델 간 최대한의 상호운용성을 보장하는 데 함께 기여한다. (§15.2)
- **base-protocol-compliant** — TLM-2.0 base protocol의 모든 규칙을 지키는 것. (§9.2)
- **bidirectional interface** — 한 쌍의 트랜잭션 객체(request와 response)가 서로 반대 방향으로 전달되며 각각이 unidirectional interface의 규칙에 따라 전달되는 TLM-1 트랜잭션 레벨 인터페이스. 각 트랜잭션 객체에 대해, 트랜잭션 어트리뷰트는 첫 timing point와 트랜잭션 수명 종료 사이 기간 동안 엄격히 read-only다. (§17.2.1)
- **binding, bound** — elaboration 중에 한쪽의 port/export와 다른 쪽의 채널(또는 다른 port/export) 사이에 만들어지는 비대칭적 연관. port(또는 export)가 채널에 바인드되면, 프로세스는 그 port를 통해 채널에 정의된 메서드로 interface method call을 할 수 있다. Port는 이름 또는 위치로 바인드될 수 있다. Export는 이름으로만 바인드될 수 있다. 참조: interface method call. (§4.2.4)
- **bit-select** — 다중 비트 데이터 타입 안의 단일 비트를 참조하는 클래스 또는 그런 클래스의 인스턴스. Bit-select는 각 SystemC numeric type과 vector class에 대해 정의된다. 특정 타입의 lvalue에 대응하는 bit-select와 rvalue에 대응하는 bit-select는 서로 다른 클래스다. (§7.2.6)
- **bit vector** — `sc_bv_base`에서 파생된 클래스 또는 그런 클래스의 인스턴스. Bit vector는 각 비트가 심볼 "0" 또는 "1"로 표현되는 다중 비트 데이터 타입을 구현한다. (§7.1)
- **blocking** — 멤버 함수 `wait`를 호출하는 것이 허용됨. Blocking 함수는 시뮬레이션 시간을 소비하거나 컨텍스트 스위치를 수행할 수 있으므로 **method process에서 호출되면 안 된다 shall not**. Blocking interface는 blocking 함수만 정의한다.
- **blocking transport interface** — 단일 멤버 함수 `b_transport`를 담는 TLM-2.0 표준의 blocking interface. TLM-1에 속하는 `transport`라는 이름의 blocking transport 멤버 함수가 여전히 존재함에 유의. (§11.2.2)
- **body** — 함수 또는 생성자의 파라미터 선언과 (있다면) 생성자 초기화 리스트 바로 뒤에 오는, 실행될 문장들을 담는 복합문. (C++ 용어)
- **bridge** — 통신 네트워크의 두 세그먼트를 연결하는 컴포넌트. bus bridge는 유사하거나 상이한 두 메모리 맵드 버스를 연결하는 장치다. 참조: adapter; transaction bridge; transactor. (§10.4, §14.21.3)
- **buffer** — `sc_buffer`의 인스턴스. `sc_signal`에서 파생된 primitive channel. buffer는 값이 쓰일 때마다, 그 쓰기가 값 변화를 일으키는지와 무관하게 이벤트가 발생한다는 점에서 signal과 다르다. signal에는 값이 변할 때만 이벤트가 발생한다. (§6.6.1)

## C

- **call** — "call"이라는 용어는 함수가 직접 호출되거나, 문제의 함수를 호출하는 중간 함수를 호출함으로써 간접적으로 호출되는 것을 모두 뜻한다. (§3.1.3)
- **caller** — 함수 호출에서, 주어진 함수가 호출되는 지점의 문장 시퀀스. 지시 대상은 함수·프로세스·모듈일 수 있다. 트랜잭션의 initiator가 아니라 함수의 호출자를 가리킬 때 initiator 대신 이 용어를 쓴다.
- **callee** — 함수 호출에서, caller가 호출하는 함수 또는 그 함수가 정의된 모듈. 지시 대상은 함수 또는 모듈일 수 있다. 트랜잭션의 target이 아니라 함수 본문을 가리킬 때 target 대신 이 용어를 쓴다.
- **callback** — 모듈 계층 안의 클래스에서 오버라이드되며 elaboration과 simulation 중 특정 고정 지점에서 커널이 호출해 주는 멤버 함수. callback 함수는 `before_end_of_elaboration`, `end_of_elaboration`, `start_of_simulation`, `end_of_simulation`이다. (§4.5)
- **channel** — 하나 이상의 인터페이스를 구현하는 클래스 또는 그런 클래스의 인스턴스. 채널은 hierarchical channel이거나 primitive channel일 수 있으며, 둘 다 아니라면 최소한 `sc_object`에서 파생될 것이 강력히 권장된다. 채널은 통신 메커니즘 또는 프로토콜의 정의를 캡슐화하는 역할을 한다. (§3.1.4)
- **child** — 주어진 모듈 안(within)에 있는 인스턴스. 모듈 A가 모듈 B 안에 있으면 A는 B의 child다. (§3.1.4, §5.16.1)
- **class template** — 정의가 템플릿 파라미터에 의존하는 임의 개수 클래스에 대한 패턴. 컴파일러는 클래스의 모든 멤버 함수를 클래스 템플릿과 같은 파라미터를 갖는 함수 템플릿으로 취급한다. 함수 템플릿 자체는 정의가 템플릿 파라미터에 의존하는 임의 개수 함수에 대한 패턴이다. (C++ 용어)
- **clock** — `sc_clock`의 인스턴스. 주기적 디지털 클록 신호의 동작을 모델링하는 사전 정의 primitive channel. 대안으로 클록은 `sc_signal<bool>`의 인스턴스로 모델링할 수도 있다. (§6.7.1)
- **clocked thread process** — 단일 명시적 클록 에지의 발생 시에만 resume되는 thread process. `SC_CTHREAD` 매크로로 생성한다. **동적 clocked thread는 없다.** (§5.2.8, §5.2.12)
- **combined interfaces** — socket 클래스를 파라미터화하는 데 쓰이는 core interface의 사전 정의 그룹. blocking / non-blocking × forward / backward 의 **네 가지** combined interface가 있다. (§10.6, §13.1)
- **complete object** — 다른 어떤 객체의 sub-object도 아닌 객체. 클래스 타입이면 most derived object라고도 한다. (C++ 용어)
- **component** — SystemC 모듈의 인스턴스. 본 표준은 세 종류의 컴포넌트를 인정한다: initiator, interconnect component, target. (§10.4)
- **concatenation** — 여러 객체 안의 비트들을 단일 집합 객체의 일부인 것처럼 참조하는 객체. (§7.2.8)
- **contain** — 두 모듈 사이의 within의 역관계. 모듈 B가 모듈 A 안에 있으면 A는 B를 contain한다. (§3.1.4)
- **convenience socket** — `tlm_initiator_socket` 또는 `tlm_target_socket`에서 파생되어 추가 기능을 구현하며 편의를 위해 제공되는 socket 클래스. 여러 convenience socket이 utility로 제공된다. (§16.2)
- **conversion function** — `operator type_id` 형태의 멤버 함수로, 클래스 타입에서 `type_id` 타입으로의 변환을 지정한다. 참조: user-defined conversion. (C++ 용어)
- **copy-constructible type** — `T(t)`가 `t`와 동등하고 `&t`가 `t`의 주소를 나타내는 타입 `T`. 기본 타입과 특정 클래스들이 포함된다. (C++ 용어)
- **core interface** — 본 표준이 정의하는 특정 트랜잭션 레벨 인터페이스 중 하나. blocking·non-blocking transport interface, direct memory interface, debug transport interface를 포함한다. 각 core interface는 interface proper다. Core interface는 generic payload API와는 별개다. (Clause 9)
- **custom-protocol-compliant** — `tlm_base_protocol_types`가 아닌 traits class로 특수화된 TLM-2.0 표준 socket(또는 그로부터 파생된 클래스)을 사용하고 TLM-2.0 generic payload를 사용하는 것. (§9.2)
- **cycle-accurate** — 모델 경계에서 임의의 주어진 사이클에서 모델의 상태를 예측할 수 있어, 대응하는 RTL 모델의 외부 관측 가능 상태와 모델 상태 사이에 매 사이클 일대일 대응을 성립시킬 수 있는 모델링 스타일. 단 모델 전체의 상태를 매 사이클 명시적으로 재평가하거나 모든 경계 핀·내부 레지스터 상태를 명시적으로 표현할 필요는 없다. 이 용어는 사이클 개념이 있는 모델에만 적용된다. (§10.3.8)
- **cycle-approximate** — 모델의 외부 관측 가능 상태와 대응하는 cycle-accurate 모델의 상태 사이에 일대일 매핑이 존재하며, 그 매핑이 상태 전이의 순서는 보존하지만 정확한 타이밍은 보존하지 않는 모델. 타이밍 정확도의 정도는 undefined. 이 용어는 사이클 개념이 있는 모델에만 적용된다.
- **cycle count accurate, cycle count accurate at transaction boundaries** — 트랜잭션 경계를 표시하는 timing point에서 샘플링했을 때, 모델의 상태와 대응 RTL 모델의 외부 관측 가능 상태 사이에 일대일 대응을 성립시킬 수 있는 모델링 스타일. cycle count accurate 모델은 매 사이클 cycle-accurate일 필요는 없지만, 다른 모델과 통신하는 트랜잭션의 경계로 정의되는 특정 핵심 timing point에서 기능적 상태와 사이클 수를 모두 정확히 예측해야 한다.

## D

- **data member** — 클래스 정의 안에 선언된 객체. 비정적 데이터 멤버는 클래스의 sub-object다. 정적 데이터 멤버는 클래스의 sub-object가 아니지만 정적 저장 기간을 갖는다. 해당 클래스나 파생 클래스의 생성자·멤버 함수 바깥에서는 데이터 멤버에 `.`과 `->` 연산자로만 접근할 수 있다. (C++ 용어)
- **declaration** — C++ 프로그램에 이름을 도입하고 C++ 컴파일러가 그 이름을 어떻게 해석할지 지정하는 C++ 언어 구성. 모든 declaration이 definition은 아니다. 예를 들어 클래스 선언은 클래스 이름을 지정하지만 클래스 멤버는 지정하지 않고, 함수 선언은 함수 파라미터를 지정하지만 함수 본문은 지정하지 않는다. 참조: definition. (C++ 용어)
- **definition** — 변수·함수·타입·템플릿의 완전한 명세. 예를 들어 클래스 정의는 클래스 이름과 클래스 멤버를 지정하고, 함수 정의는 함수 파라미터와 함수 본문을 지정한다. 참조: declaration. (C++ 용어)
- **delta cycle** — 하나의 evaluation phase 뒤에 하나의 update phase가 오는, 스케줄러 내부의 제어 루프. delta cycle 메커니즘은 계산(evaluation) 단계와 통신(update) 단계를 분리·교대시킴으로써 동시 프로세스의 결정론적 시뮬레이션을 보장하는 역할을 한다. (§4.3.3)
- **delta notification** — 시간 인자를 0으로 하여 `notify` 함수를 호출한 결과로 만들어지는 notification. 이벤트는 `notify` 호출로부터 한 delta cycle 뒤에 notify된다. (§4.3.2, §5.10.6)
- **delta notification phase** — delta notification의 결과로 프로세스들이 runnable해지는 스케줄러 내 제어 단계. (§4.3.2.5)
- **during elaboration, during simulation** — "during elaboration"과 "during simulation"이라는 구절은 어떤 동작이 그 시점에 일어날 수 있는지 여부를 나타내는 데 쓰인다. 그 의미는 elaboration·simulation callback의 정의와 밀접하게 연결된다. 예를 들어 elaboration 중 허용되는 여러 동작이 `end_of_elaboration` callback에서는 명시적으로 금지된다. (§3.1.4, §4.5)
- **dynamic process** — `end_of_elaboration` callback에서 또는 simulation 중에 생성된 프로세스.
- **dynamic sensitivity** — 프로세스가 resume되거나 trigger되게 할 이벤트 또는 time-out의 집합으로, (thread process의 경우) 멤버 함수 `wait` 또는 (method process의 경우) 멤버 함수 `next_trigger`의 가장 최근 호출로 만들어진 것. 참조: sensitivity. (§4.3)

## E

- **effective local time** — temporally decoupled initiator 내부의 현재 시각. `effective_local_time = sc_time_stamp() + local_time_offset`. (§11.2.4.2)
- **elaboration** — 모듈 계층이 생성되고 port가 바인드되는 실행 단계. C++ 애플리케이션의 실행은 elaboration 뒤에 simulation이 오는 것으로 구성된다. (Clause 4)
- **error** — 구현이 report-handling 메커니즘(`sc_report_handler`의 `report` 함수)을 사용하여 severity `SC_ERROR`로 진단 메시지를 생성할 의무. (§3.2.5)
- **evaluation phase** — 프로세스가 실행되는 스케줄러 내 제어 단계. evaluation phase는 runnable 프로세스 집합이 비면 완료된다. 참조: delta cycle. (§4.3.2.3)
- **event** — `sc_event` 클래스의 객체. event는 프로세스 간 동기화 메커니즘을 제공한다. `sc_event`의 멤버 함수 `notify`는 이벤트가 특정 시점에 notify되게 한다. (이벤트의 notification은 `sc_event` 타입의 객체와 구별된다. 전자는 유일한 시점에서의 동적 발생이고, 후자는 수명 동안 여러 번 notify될 수 있는 객체다.) 참조: notification. (§3.1.4, §5.10)
- **event expression** — `operator&` 또는 `operator|`로 구분된 이벤트 또는 이벤트 리스트의 목록으로, 멤버 함수 `wait` 또는 `next_trigger`에 인자로 전달된다. (§5.9)
- **event finder** — port가 바인드된 채널 인스턴스 안의 이벤트를 반환하는 port 클래스의 멤버 함수. event finder는 **static sensitivity를 만들 때만** 호출될 수 있다. (§5.7)
- **event list** — 멤버 함수 `wait` 또는 `next_trigger`에 인자로 전달될 수 있는 `sc_event_and_list` 또는 `sc_event_or_list` 타입의 객체. (§5.8)
- **exclusion rule** — base protocol의 규칙으로, 어떤 socket을 통해 이미 진행 중인 request(또는 response)가 있으면 그 socket을 통해 request(또는 response)를 보내는 것을 금지한다. base protocol에는 request exclusion rule과 response exclusion rule 두 개가 있으며 서로 독립적으로 작동한다. (§15.2.6)
- **export** — `sc_export`의 인스턴스. export는 모듈이 **제공**하는 인터페이스를 지정한다. 시뮬레이션 중 port는 export가 바인드된 채널로 메서드 호출을 전달한다. export는 메서드 호출을 모듈 인스턴스 **안쪽·아래쪽**으로 전달한다. (§3.1.4, §5.13)
- **extension** — generic payload 트랜잭션 객체에 추가되어 함께 운반되는 사용자 정의 객체, 또는 `tlm_phase` 타입과 대입 호환인 값의 집합을 확장하는 사용자 정의 클래스. ignorable extension은 base protocol과 함께 사용될 수 있지만, non-ignorable 또는 mandatory extension은 새 protocol traits class의 정의를 요구한다. (§14.21)

## F

- **fifo** — `sc_fifo`의 인스턴스. first-in-first-out 버퍼를 모델링하는 primitive channel. 대안으로 fifo는 모듈로 모델링할 수도 있다. (§6.23)
- **finite-precision fixed-point type** — `sc_fxnum`에서 파생된 클래스 또는 그런 클래스의 인스턴스. 지정된 word length, integer word length, quantization mode, overflow mode에 의해서만 정밀도가 제한되는 부호 있는/없는 고정소수점 값을 표현한다. (§7.1)
- **finite-precision integer** — `sc_signed` 또는 `sc_unsigned`에서 파생된 클래스, 또는 그런 클래스의 인스턴스. 지정된 word length에 의해서만 정밀도가 제한되는 부호 있는/없는 정수 값을 표현한다. (§7.1)
- **forward path** — initiator 또는 interconnect component가 다른 interconnect component 또는 target 방향으로 나아가며 interface method call을 수행하는 호출 경로. (§10.4)

## G

- **generic payload** — 메모리 맵드 버스를 통해 통신하는 컴포넌트의 loosely-timed 모델과 approximately-timed 모델 사이에 어느 정도의 상호운용성을 달성하기 위해 사용될 수 있는 트랜잭션 페이로드를 함께 정의하는 특정 트랜잭션 어트리뷰트 집합과 그 의미론. 모든 모델링 스타일에 같은 트랜잭션 클래스가 쓰인다. (Clause 14)
- **global quantum** — 모든 quantum keeper와 temporally decoupled initiator가 사용하는 기본 time quantum. 모든 temporally decoupled initiator가 통상 global quantum의 정수배 시점에서, 또는 필요 시 더 자주 동기화하는 것이 의도다. (Clause 12)

## H

- **hierarchical binding** — child 모듈의 socket을 parent 모듈의 socket에, 또는 parent 모듈의 socket을 child 모듈의 socket에 바인드하여 트랜잭션을 모듈 계층 위아래로 전달하는 것. (§16.2.4)
- **hierarchical channel** — `sc_module`에서 파생되며 하나 이상의 인터페이스를 구현하는 클래스, 또는 더 비형식적으로는 그런 클래스의 인스턴스. 채널이 자체 port·프로세스·모듈 인스턴스를 필요로 할 때 사용한다. 참조: channel. (§3.1.4, §5.2.23)
- **hierarchical name** — 모듈 계층 안 인스턴스의 유일한 이름. top-level 모듈에서 시작해 이름 붙일 인스턴스의 string name으로 끝나는 parent-child 사슬의 string name들로 구성된다. string name들은 점 문자로 구분되어 연결된다. (§5.3.4, §5.16.4)
- **hop** — 하나의 initiator socket이 하나의 target socket에 바인드된 것. initiator에서 target까지 경로는 여러 hop으로 구성될 수 있으며, 각 hop은 인접한 두 컴포넌트를 연결한다. initiator와 target 사이 hop 수는 항상 그 경로상 interconnect component 수보다 1 크다. 예를 들어 initiator가 중간 interconnect component 없이 target에 직접 연결되면 hop 수는 1이다. (§10.4)

## I

- **ignorable extension** — extension을 설정한 컴포넌트 외의 어떤 컴포넌트도 무시해도 되는 generic payload extension. ignorable extension은 존재할 것이 요구되지 않는다. base protocol에서 허용된다. (§14.21.1.2)
- **ignorable phase** — 매크로 `DECLARE_EXTENDED PHASE`로 만들어진 phase로, 그것을 받는 어떤 컴포넌트도 무시해도 되며 어떤 종류의 응답도 요구할 수 없다. base protocol에서 허용된다. (§15.2.5)
- **immediate notification** — 빈 인자 목록으로 함수를 호출한 결과로 만들어지는 notification. 그 이벤트에 sensitive한 모든 프로세스가 즉시 runnable해진다. (§4.3.2, §5.10.6)
- **implementation** — SystemC와 TLM-2.0 클래스 라이브러리 전체의 특정 구체 구현. 그중 public shell만 애플리케이션에 노출되면 된다(예: 일부는 미리 컴파일되어 도구 벤더가 오브젝트 코드로 배포할 수 있다). 참조: kernel. (§3.1.2)
- **implement** — 파생 원본 인터페이스에 선언된 모든 순수 가상 함수에 대해 정의를 제공하는 채널을 만드는 것. (§5.14.1)
- **implicit conversion** — 표준 변환 또는 사용자 정의 변환이 특정 상황에서 암묵적으로 호출되는 C++ 언어 메커니즘. 사용자 정의 변환은 모호하지 않은 경우에만 암묵적으로 적용되며, 주어진 값에 대해 최대 하나의 사용자 정의 변환만 암묵적으로 적용된다. 참조: user-defined conversion. (C++ 용어)
- **initialization phase** — 스케줄러의 첫 단계. 이 단계에서 모든 프로세스가 정지하거나 반환할 때까지 한 번씩 실행된다. (§4.3.2.2)
- **initializer list** — 기저 클래스 sub-object와 데이터 멤버를 초기화하는 데 쓰이는 생성자 정의의 C++ 문법 부분. (C++ 용어 mem-initializer-list와 관련)
- **initiator** — 트랜잭션을 개시할 수 있는 모듈. initiator는 트랜잭션 객체의 상태를 초기화하고, 트랜잭션 수명이 끝날 때 트랜잭션 객체를 삭제하거나 재사용할 책임이 있다. TLM-1 인터페이스의 경우 여기 정의된 initiator라는 용어가 엄밀히 적용되지 않을 수 있으므로, 명확성을 위해 caller와 callee라는 용어를 대신 쓸 수 있다. (§10.4)
- **initiator socket** — forward path의 interface method call을 위한 port와 backward path의 interface method call을 위한 export를 담는 클래스. socket은 SystemC 바인딩 연산자를 오버로드하여 port와 export를 모두 바인드한다. (§13.2)
- **interconnect component** — 트랜잭션 객체에 접근하지만 그 트랜잭션에 대해 initiator나 target 역할을 하지 않는 모듈. interconnect component가 트랜잭션 객체의 어트리뷰트를 수정하는 것이 허용되는지는 페이로드의 규칙에 달렸다. arbiter나 router는 통상 interconnect component로 모델링되며, 대안은 한 트랜잭션의 target이자 별개 트랜잭션의 initiator로 모델링하는 것이다. (§10.4)
- **instance** — 주어진 범주의 특정 사례. 예를 들어 module instance는 `sc_module`에서 파생된 클래스의 객체다. core language 정의 내에서 instance는 통상 `sc_object`에서 파생된 클래스의 객체이며 유일한 hierarchical name을 갖는다. (§3.1.4)
- **instantiation** — 새 객체의 생성. 예를 들어 module instantiation은 `sc_module`에서 파생된 클래스의 새 객체를 만든다. (§4.2.2)
- **integer** — limited-precision integer 또는 finite-precision integer. (§7.2.2)
- **interface** — `sc_interface`에서 파생된 클래스. interface proper는 interface이며, 객체지향적 의미에서 채널도 interface다. 그러나 채널은 interface proper가 아니다. (§3.1.4)
- **interface method call (IMC)** — interface method에 대한 호출. interface method는 인터페이스 안에 선언된 멤버 함수다. IMC 패러다임은 메서드 호출과 채널 내 메서드 구현 사이에 한 단계의 간접을 제공하여, caller에 영향을 주지 않고 한 채널을 다른 채널로 대체할 수 있게 한다. (§4.2.4, §5.12.1)
- **interface proper** — `sc_interface`에서 파생되었지만 `sc_object`에서는 파생되지 않은 추상 클래스. interface proper는 채널 안에서 구현되고 port를 통해 호출될 메서드 집합을 선언한다. interface proper는 순수 가상 함수 선언을 담지만, 통상 함수 정의도 데이터 멤버도 담지 않는다. (§3.1.4, §5.14.1)
- **interoperability** — 다양한 출처의 둘 이상 트랜잭션 레벨 모델이 본 표준이 정의하는 인터페이스로 정보를 교환할 수 있는 능력. programmers view 사용 사례에서 일반적인 메모리 맵드 버스 프로토콜을 구현하는 모델은 명시적 adapter 없이 상호운용 가능해야 한다는 것이 의도다. 나아가 상이한 프로토콜·사용 사례의 모델에 대해 상호운용성을 달성하는 데 필요한 엔지니어링 노력을 줄이는 것이 의도이지만, 일반적으로는 adapter가 필요할 것으로 예상된다. (Clause 9)
- **interoperability layer** — 상호운용성에 필요한 본 표준 클래스들의 부분집합. TLM-2.0 core interface, initiator·target socket, generic payload, `tlm_global_quantum`, `tlm_phase`로 구성된다. base protocol과 밀접히 관련된다. (Clause 9)

## K

- **kernel** — 기반 elaboration 엔진과 simulation 엔진을 포함하는 SystemC 구현의 핵심. 커널은 본 표준이 정의하는 의미론을 지키지만, 본 표준의 범위를 벗어난 구현 특정 기능도 담을 수 있다. 참조: implementation. (Clause 4)

## L

- **lifetime (of an object)** — 객체의 수명은 저장 공간이 할당되고 (있다면) 생성자 호출이 완료될 때 시작한다. 저장 공간이 해제될 때 또는 (있다면) 소멸자가 호출되기 직전에 끝난다. (C++ 용어)
- **lifetime (of a transaction)** — 트랜잭션이 유효해질 때 시작하여 무효해질 때 끝나는 기간. 트랜잭션 객체를 풀링하거나 재사용할 수 있으므로 트랜잭션 객체의 수명이 대응 트랜잭션의 수명보다 길 수 있다. 예를 들어 트랜잭션 객체가 TLM-1 인터페이스의 여러 `put` 호출에 인자로 전달되는 스택 변수일 수 있다.
- **limited-precision fixed-point type** — `sc_fxnum_fast`에서 파생된 클래스 또는 그런 클래스의 인스턴스. 기반 네이티브 C++ 부동소수점 표현과 지정된 word length, integer word length, quantization mode, overflow mode에 의해 정밀도가 제한되는 부호 있는/없는 고정소수점 값을 표현한다. (§7.1)
- **limited-precision integer** — `sc_int_base` 또는 `sc_uint_base`에서 파생된 클래스, 또는 그런 클래스의 인스턴스. 기반 네이티브 C++ 표현과 지정된 word length에 의해 정밀도가 제한되는 부호 있는/없는 정수 값을 표현한다. (§7.1)
- **local quantum** — initiator가 동기화해야 하기까지 남은 시뮬레이션 시간의 양. 통상 local quantum은 global quantum의 다음으로 큰 정수배에서 현재 시뮬레이션 시간을 뺀 값이지만, 주어진 quantum keeper에 대해 이 계산은 오버라이드될 수 있다. (§16.3)
- **local time offset** — temporally decoupled initiator에서 가장 최근 quantum 경계를 기준으로 측정된 시간. `b_transport`와 `nb_transport` 메서드의 timing annotation 인자가 local time offset이다. `effective_local_time = sc_time_stamp() + local_time_offset`. (§16.3)
- **logic vector** — `sc_lv_base`에서 파생된 클래스 또는 그런 클래스의 인스턴스. 각 비트가 4치 논리 심볼 "0", "1", "X", "Z"로 표현되는 다중 비트 데이터 타입을 구현한다. (§7.1)
- **loosely-timed** — 운영체제를 부팅하고 스레드 간 명시적 동기화 없이 다수 스레드를 관리하는 데 필요한 기능만을 지원하기에 충분한 최소한의 타이밍 정보를 표현하는 모델링 스타일. loosely-timed 모델은 타이머 모델과 명목상의 arbitration 간격 또는 실행 슬롯 길이를 포함할 수 있다. 프로토콜 견고성 테스트를 위해 loosely-timed 기술에 임의 지연을 삽입하는 관행을 채택하는 사용자도 있지만, 이 관행이 모델링 스타일의 기본 특성을 바꾸지는 않는다. (§10.3.3)
- **lvalue** — 주소를 취할 수 있는 객체 참조. 내장 대입 연산자의 좌변 피연산자는 비-const lvalue여야 한다. (C++ 용어)

## M

- **mandatory extension** — 주어진 사용자 정의 프로토콜 타입의 socket을 통해 전송되는 **모든** 트랜잭션에 존재할 것이 요구되는 generic payload extension. (§14.21.1.3)
- **member function** — friend 함수를 제외하고, 클래스 정의 안에 선언된 함수. 해당 클래스나 파생 클래스의 생성자·멤버 함수 바깥에서는 비정적 멤버 함수에 `.`과 `->` 연산자로만 접근할 수 있다. 참조: method. (C++ 용어)
- **memory manager** — generic payload 트랜잭션 객체에 대해 메모리 관리를 수행하는 사용자 정의 클래스. memory manager는 트랜잭션의 참조 카운트가 0에 도달할 때 호출되는 `free` 메서드를 제공해야 한다. (§14.5)
- **method** — 클래스의 동작을 구현하는 함수. C++ 용어 member function과 동의어. SystemC에서 method라는 용어는 interface method call의 맥락에서 쓰인다. 본 표준 전반에서 C++ 표준 준수를 위해 클래스를 정의할 때는 member function을, 더 비형식적인 맥락과 interface method call을 논할 때는 method를 쓴다.
- **method process** — 스케줄러의 스레드에서 실행되며 스케줄러가 그 sensitivity로 결정되는 시점에 호출(trigger)하는 프로세스. unspawned method process는 `SC_METHOD` 매크로로, spawned method process는 `sc_spawn` 함수 호출로 생성한다. (§5.2.8, §5.2.10)
- **module** — `sc_module`에서 파생된 클래스 또는 더 비형식적으로는 그런 클래스의 인스턴스. SystemC 애플리케이션은 모듈로 구성되며, 각 module instance는 계층 경계를 나타낸다. 모듈은 port·프로세스·primitive channel·다른 모듈의 인스턴스를 담을 수 있다. (§3.1.4, §5.2)
- **module hierarchy** — elaboration 중에 생성되어 module instantiation, port instantiation, primitive channel instantiation, process instantiation, port binding 메커니즘으로 서로 연결된 모든 인스턴스의 집합. module hierarchy는 object hierarchy의 부분집합이다. (§3.1.4, Clause 4)
- **multiport** — 하나 이상의 채널 또는 port 인스턴스에 바인드될 수 있는 port. 애플리케이션이 port를 주소 지정 가능한 채널 집합에 바인드하고자 하고 채널 수가 elaboration 전에는 알려지지 않은 경우에 사용한다. (§4.2.4, §5.12.3)
- **multi-socket** — 다른 컴포넌트에 속한 여러 socket에 바인드될 수 있는 convenience socket 계열의 하나. initiator multi-socket은 하나 이상의 target socket에 바인드될 수 있고, 하나 이상의 initiator socket이 하나의 target multi-socket에 바인드될 수 있다. multi-socket을 통해 interface method를 호출할 때 목적지는 첨자 연산자로 구별된다. (§16.2.4)
- **mutex** — `sc_mutex`의 인스턴스. 상호 배제 통신 메커니즘을 모델링하는 사전 정의 채널. (§6.27.1)

## N

- **nb_transport** — 멤버 함수 `nb_transport_fw`와 `nb_transport_bw`. 본 문서에서 이탤릭체 용어 *nb_transport*는 둘을 구별할 필요가 없는 상황에서 두 메서드를 함께 가리키는 데 쓰인다. (§11.2.3.4)
- **non-abstract class** — abstract class가 아닌 클래스. (C++ 용어)
- **non-blocking** — 멤버 함수 `wait`를 호출하는 것이 허용되지 않음. non-blocking 함수는 시뮬레이션 시간을 소비하거나 컨텍스트 스위치를 수행하지 않고 반환할 것으로 기대되며, 따라서 thread process에서든 method process에서든 호출될 수 있다. non-blocking interface는 non-blocking 함수만 정의한다.
- **non-blocking transport interface** — TLM-2.0 표준의 non-blocking interface. `nb_transport_fw`와 `nb_transport_bw`라는 멤버 함수를 담는 **두 개**의 인터페이스가 있다. (§10.3.9)
- **non-ignorable extension** — 존재한다면 트랜잭션을 받는 모든 컴포넌트가 반드시 그에 따라 동작해야 하는 generic payload extension. (§14.21.1.3)
- **notification** — `sc_event`의 멤버 함수 `notify`가 수행하는, 이벤트 발생을 스케줄링하는 행위. immediate notification, delta notification, timed notification 세 종류가 있다. 참조: event. (§4.3.2, §5.10.6)
- **notified** — 이벤트가 pending 이벤트 집합에서 제거되고 현재 그 이벤트에 sensitive한 프로세스들이 runnable해지는 스케줄러 제어 단계에서 그 이벤트가 notify되었다고 한다. 비형식적으로는, 이벤트는 notify되는 바로 그 시점에 발생한다. (§4.3)
- **numeric type** — finite-precision integer, limited-precision integer, finite-precision fixed-point type, limited-precision fixed-point type 중 하나. (§7.1)

## O

- **object** — 저장 공간의 영역. 모든 객체는 타입과 수명을 갖는다. 정의로 만들어진 객체는 이름을 갖고, new 표현식으로 만들어진 객체는 익명이다. (C++ 용어)
- **object hierarchy** — `sc_object` 클래스의 모든 객체의 집합. 각 객체는 유일한 hierarchical name을 갖는다. module hierarchy에 속하지 않는 객체는 시뮬레이션 중 동적으로 생성·파괴될 수 있다. (§3.1.4, §5.16.1)
- **occurrence** — 이벤트의 notification. immediate notification의 경우를 제외하면 `sc_event`의 멤버 함수 `notify` 호출은 이벤트가 이후의 delta cycle 또는 이후의 시뮬레이션 시점에 발생하게 한다. time-out의 경우: 지정된 시간 간격이 경과했을 때 time-out이 발생한다. (§5.10.1)
- **opposite path** — 주어진 경로와 반대 방향의 경로. forward path에 대해 opposite path는 forward return path 또는 backward path다. backward path에 대해 opposite path는 forward path 또는 backward return path다. (§10.4)
- **overload** — 같은 스코프에 선언되고 파라미터의 개수나 타입이 다른, 같은 이름의 함수를 둘 이상 만드는 것. (C++ 용어)
- **override** — 기저 클래스의 멤버 함수와 같은 이름과 파라미터 목록을 갖는 멤버 함수를 파생 클래스에 만드는 것. (C++ 용어)

## P

- **parameter** — 함수 선언·정의(또는 매크로 정의·템플릿 파라미터)의 일부로 선언된 객체. formal parameter라고도 한다. 참조: argument. (C++ 용어)
- **parent** — child의 역관계. 모듈 B가 모듈 A의 child이면 A는 B의 parent다. (§3.1.4, §5.16.1)
- **part-select** — 다중 비트 데이터 타입 안의 연속된 비트 부분집합을 참조하는 클래스 또는 그런 클래스의 인스턴스. Part-select는 각 SystemC numeric·vector 클래스에 대해 정의된다. 특정 타입의 lvalue에 대응하는 part-select와 rvalue에 대응하는 part-select는 서로 다른 클래스다. (§7.2.8)
- **paused** — `sc_pause` 호출에 이어 스케줄러가 종료된 뒤의 시뮬레이션 상태.
- **payload event queue (PEQ)** — SystemC 이벤트 notification의 큐를 유지하는 클래스로, 각 notification이 연관된 트랜잭션 객체를 운반한다. 트랜잭션은 지연과 함께 큐에 put되고, 각 트랜잭션은 put된 시각에 주어진 지연을 더한 시각에 큐 뒤로 튀어나온다. non-blocking 인터페이스를 approximately-timed 코딩 스타일과 결합할 때 유용하다. (§16.4)
- **pending** — notification이 게시된 이벤트의 상태. 즉 멤버 함수 `notify`는 호출되었지만 이벤트가 아직 notify되지 않은 상태.
- **phase** — 트랜잭션 수명 중의 한 기간. phase는 non-blocking transport 메서드에 인자로 전달된다. 각 phase 전이는 timing point와 연관된다. timing point는 `nb_transport`의 시간 인자로 주어지는 양만큼 지연될 수 있다. (§11.2.3.6)
- **phase transition** — 한 phase에서 다른 phase로의 전이. phase는 non-blocking transport 메서드의 phase 인자 값으로 표현된다. `nb_transport`의 각 호출과, 반환값 `TLM_UPDATED`로의 각 반환이 phase 전이를 표시한다. base protocol은 phase 인자 값이 직전 상태와 같은 `nb_transport` 호출을 허용하지 않는다. (§15.2.4)
- **port** — `sc_port`에서 파생된 클래스 또는 더 비형식적으로는 그런 클래스의 인스턴스. port는 모듈 경계를 넘는 통신을 가능하게 하는 주된 메커니즘이다. port는 모듈이 **요구**하는 인터페이스를 지정한다. 시뮬레이션 중 port는 모듈 안 프로세스가 하는 메서드 호출을, 모듈 인스턴스화 시 그 port가 바인드된 채널로 전달한다. port는 메서드 호출을 모듈 인스턴스 **위쪽·바깥쪽**으로 전달한다. (§3.1.4, §5.12)
- **portless channel access** — port나 export를 통하지 않고 채널의 멤버 함수를 직접 호출하는 것. (§5.12.1)
- **primitive channel** — `sc_prim_channel`에서 파생되어 하나 이상의 인터페이스를 구현하는 클래스 또는 더 비형식적으로는 그런 클래스의 인스턴스. primitive channel은 스케줄러의 update phase에 접근할 수 있지만 port·프로세스·모듈 인스턴스를 담을 수 없다. (§3.1.4, §5.15)
- **process** — process instance는 `sc_object`에서 파생된 implementation-defined 클래스에 속한다. 각 process instance는 프로세스의 동작을 나타내는 연관 함수를 갖는다. 프로세스는 static process, dynamic process, spawned process, unspawned process일 수 있다. 프로세스는 계산을 기술하는 주된 수단이다. 참조: dynamic process; spawned process; static process; unspawned process. (§3.1.4)
- **process handle** — 기반 spawned 또는 unspawned process instance에 대한 안전한 접근을 제공하는 `sc_process_handle` 클래스의 객체. process handle은 valid이거나 invalid일 수 있다. process handle은 연관 process instance가 파괴된 뒤에도 invalid 상태로 계속 존재한다. (§3.1.4, §5.6)
- **programmers view (PV)** — 운영체제를 부팅하고 애플리케이션 소프트웨어를 실행하기 위해 하드웨어 플랫폼의 기능적으로 정확한 loosely-timed 모델을 필요로 하는 소프트웨어 프로그래머의 사용 사례.
- **protocol traits class** — 트랜잭션 객체의 타입과 phase 타입에 대한 typedef를 담는 클래스로, combined interface를 파라미터화하는 데 쓰이며 사실상 프로토콜에 대한 유일한 타입을 정의한다. (§14.2.3, §14.2.4)
- **proxy class** — C++ 의미론상 제한될 특정 문장들의 가독성을 확장하는 것만이 유일한 목적인 클래스. 예를 들어 `sc_int` 변수를 `bool`의 C++ 배열인 것처럼 쓸 수 있게 하는 것. proxy class는 함수가 반환하는 임시(이름 없는) 값에만 쓰이도록 의도되었다. **proxy class 생성자는 이름 있는 객체를 만들기 위해 애플리케이션이 명시적으로 호출하면 안 된다 shall not.** (§7.2.6)

## Q

- **quantum** — temporal decoupling에서, 프로세스가 현재 시뮬레이션 시간보다 앞서 달릴 수 있도록 허용된 양. (§10.3.3, §11.2.2.7, Clause 12)
- **quantum keeper** — 현재 시뮬레이션 시간으로부터의 local time offset을 저장하고 이를 local quantum과 대조해 검사하는 유틸리티 클래스. (§16.3)

## R

- **request** — base protocol에서, 트랜잭션 수명 중 정보가 initiator에서 target으로 전달되는 단계. 사실상 request는 command, address, 그리고 write command의 경우 data 배열을 포함한 generic payload 어트리뷰트를 initiator에서 target으로 운반한다. (트랜잭션은 실제로는 참조로, data 배열은 포인터로 전달된다.)
- **resolved signal** — `sc_signal_resolved` 또는 `sc_signal_rv`의 인스턴스. 둘 이상의 프로세스가 쓸 수 있으며 충돌하는 값이 채널 내에서 해소되는 signal 채널. (§6.13.1)
- **response** — base protocol에서, 트랜잭션 수명 중 정보가 target에서 initiator로 되돌아 전달되는 단계. 사실상 response는 response status, 그리고 read command의 경우 data 배열을 포함한 generic payload 어트리뷰트를 target에서 initiator로 운반한다. (트랜잭션은 실제로는 참조로, data 배열은 포인터로 전달된다.)
- **resume** — thread 또는 clocked thread process가, 정지했던 지점의 멤버 함수 `wait` 바로 다음 실행 문장부터 실행을 계속하게 하는 것. 프로세스의 sensitivity에 의존한다. (§5.2.11) 또한 앞선 `suspend` 호출의 효과를 취소하는 `sc_process_handle`의 멤버 함수. (§5.6.6.2)
- **return path** — 일련의 interface method call의 호출 스택이 forward path 또는 backward path를 따라 되감기는 제어 경로. forward path에 대한 return path는 target에서 initiator로 정보를 운반할 수 있고, backward path에 대한 return path는 initiator에서 target으로 정보를 운반할 수 있다. (§10.4)
- **rvalue** — 반드시 저장 공간이나 주소를 갖지는 않는 값. 기본 타입의 rvalue는 대입의 우변에만 나타날 수 있다. (C++ 용어)

## S

- **scheduled** — 멤버 함수 `notify`, `next_trigger`, `wait` 호출의 결과로 갖게 되는 이벤트 또는 프로세스의 상태. 이벤트는 발생하도록 스케줄될 수 있고, 프로세스는 이후 delta cycle 또는 이후 시뮬레이션 시점에 trigger되거나 resume되도록 스케줄될 수 있다.
- **scheduler** — 시뮬레이션을 제어하는 커널의 일부. 시간 진행, 이벤트가 notify될 때 프로세스를 runnable하게 만들기, 프로세스 실행, primitive channel 갱신을 담당한다. (Clause 4)
- **sensitivity** — 프로세스가 resume되거나 trigger되게 할 이벤트 또는 time-out의 집합. sensitivity는 static sensitivity 또는 dynamic sensitivity의 형태를 취할 수 있다. (§4.3)
- **signal** — `sc_signal`의 인스턴스. 디지털 하드웨어 시뮬레이션에 적절한 수준에서 단순한 와이어 동작의 관련 측면을 모델링하기 위한 primitive channel. (§3.1.4, §6.4)
- **signature** — 오버로드 해소에 관련된 함수 정보, 예를 들어 파라미터의 타입과 한정자들. (C++ 용어)
- **simple socket** — 사용하기 간단한 convenience socket 계열의 하나. socket이 요구 인터페이스를 구현하는 다른 객체에 바인드되어야 하는 대신, callback 메서드를 socket 객체에 직접 등록할 수 있게 하기 때문이다. simple target socket은 blocking과 non-blocking 사이의 자동 변환을 제공함으로써 target이 두 transport interface를 모두 구현할 필요를 없애 준다. (§16.2.2)
- **simulation** — 스케줄러의 실행과 그 제어 하의 사용자 정의 프로세스 실행으로 구성되는 실행 단계. SystemC 애플리케이션의 실행은 elaboration 뒤에 simulation이 오는 것으로 구성된다. (Clause 4)
- **socket** — 참조: initiator socket; target socket.
- **spawned process** — `sc_spawn` 함수 호출로 생성된 process instance. 참조: process. (§3.1.4, §5.5.6)
- **specialized port** — 템플릿 클래스 `sc_port`에서 파생되어 특정 타입을 `sc_port` 템플릿의 첫 인자로 전달하며, 그 특정 타입의 port에 접근하기 위한 편의 함수를 제공하는 클래스. (§6.8)
- **standard error response** — 트랜잭션을 성공적으로 실행할 수 없는 generic payload target에 대해 본 표준이 규정하는 동작. target은 (1) 트랜잭션을 성공적으로 실행하거나, (2) response status 어트리뷰트를 error response로 설정하거나, (3) SystemC report handler를 호출해야 권장(should). (§14.17.2)
- **statement** — `if` 문, `switch` 문, `for` 문, `return` 문처럼 순서대로 실행되는 특정 범주의 C++ 언어 구성. 세미콜론이 뒤따르는 C++ 표현식도 statement다. (C++ 용어)
- **static process** — 모듈 계층 구축 중에 또는 `before_end_of_elaboration` callback에서 생성된 프로세스.
- **static sensitivity** — 프로세스가 resume되거나 trigger되게 할 이벤트 또는 time-out의 집합으로, (unspawned process의 경우) `sc_module`의 데이터 멤버 `sensitive`를 사용해, 또는 (spawned process의 경우) `sc_spawn_options` 클래스를 사용해 만들어진 것. 참조: sensitivity; spawned process; unspawned process. (§4.3)
- **sticky extension** — 트랜잭션 객체의 수명이 끝날 때 (자동으로든 명시적으로든) 삭제되지 않아 트랜잭션 객체가 풀링될 때 그와 함께 남는 generic payload extension 객체. sticky extension은 memory manager가 삭제하지 않는다. (§14.5)
- **string name** — 모듈 계층 내에서 객체의 정체성을 제공하기 위해 인스턴스의 생성자에 인자로 전달되는 이름. 공통 parent 모듈을 갖는 인스턴스들의 string name은 그 모듈 내에서만, 그리고 그 모듈 내에서는 유일하다. 참조: hierarchical name. (§5.3, §5.16.4)
- **sub-object** — 다른 객체 안에 담긴 객체. 클래스의 sub-object는 그 클래스의 데이터 멤버이거나 기저 클래스 sub-object일 수 있다. (C++ 용어)
- **suspend** — 연관 함수가 `wait`를 호출하게 하여 프로세스가 실행을 중단하게 하는 것. 또한 프로세스가 정지 상태로 남아 있게 하며 그 효과를 `resume` 호출로 취소할 수 있는 `sc_process_handle`의 멤버 함수. 또한 프로세스 자신과 연관된 process handle의 멤버 함수 `suspend`를 호출하여 프로세스가 실행을 중단하게 하는 것. (§5.6.6.2)
- **synchronize** — 다른 프로세스가 실행될 수 있도록 양보(yield)하는 것. temporal decoupling을 쓰는 경우에는 양보하고 현재 time quantum이 끝날 때까지 대기하는 것. (§10.3.3)
- **synchronization-on-demand** — temporally decoupled 프로세스가, 각 quantum이 끝날 때 통상적으로 일어나는 동기화 지점에 더하여, 시뮬레이션 시간이 진행하고 다른 프로세스가 실행되도록 SystemC 스케줄러에 제어를 되돌려주는 행위. (§10.3.4)
- **synchronous reset state** — process instance의 reset signal이 active해지거나 `sync_reset_on`이 호출될 때 그 process instance가 진입하는 상태. synchronous reset state에 있을 때 process instance는 resume될 때마다 reset된다. (§5.6.6.4)

## T

- **tagged socket** — 트랜잭션이 도착한 socket(또는 multi-socket의 요소)을 식별하기 위해 들어오는 모든 interface method call에 `int id` 태그를 추가하는 convenience socket 계열의 하나. (§16.2.3.4)
- **target** — 트랜잭션의 최종 목적지를 나타내는 모듈. initiator가 생성한 트랜잭션에 응답할 수 있지만 스스로 새 트랜잭션을 개시할 수는 없다. write 연산에서는 데이터가 initiator에서 하나 이상의 target으로 복사된다. read 연산에서는 데이터가 하나의 target에서 initiator로 복사된다. target은 트랜잭션 객체의 상태를 읽거나 수정할 수 있다. TLM-1 인터페이스의 경우 여기 정의된 target이라는 용어가 엄밀히 적용되지 않을 수 있으므로, 명확성을 위해 caller와 callee라는 용어를 대신 쓸 수 있다. (§10.4)
- **target socket** — backward path의 interface method call을 위한 port와 forward path의 interface method call을 위한 export를 담는 클래스. socket은 SystemC 바인딩 연산자도 오버로드하여 port와 export를 모두 바인드한다. (§10.4)
- **temporal decoupling** — 컨텍스트 스위칭을 줄이고 그럼으로써 시뮬레이션 속도를 높이기 위해 하나 이상의 initiator가 현재 시뮬레이션 시간보다 앞서 달릴 수 있게 하는 능력. (§10.3.3)
- **terminated** — thread 또는 clocked thread process의 상태로, 연관 함수가 끝까지 실행되거나 `return` 문을 실행하여 제어가 커널로 돌아갔을 때, 또는 프로세스가 kill된 뒤의 상태. `wait` 함수를 호출해도 thread process가 terminate되지는 않는다. 참조: clocked thread process; method process; thread process. (§5.2.11, §5.6.5)
- **thread process** — 자신의 스레드에서 실행되며 initialization 중에 스케줄러가 단 한 번만 호출하는 프로세스. thread process는 멤버 함수 `wait`의 실행으로 정지될 수 있고, 그 경우 스케줄러의 제어 하에 resume된다. unspawned thread process는 `SC_THREAD` 매크로로, spawned thread process는 `sc_spawn` 함수 호출로 생성한다. 참조: spawned process; unspawned process. (§5.2.8, §5.2.11)
- **time-out** — 시간 값 인자를 갖는 멤버 함수 `wait` 또는 `next_trigger` 호출의 결과로 프로세스를 resume하거나 trigger하게 하는 것. 그 멤버 함수를 호출한 프로세스는 지정된 시간이 경과한 뒤 resume 또는 trigger된다. 단 이벤트가 notify되어 이미 resume/trigger된 경우는 예외다. (§4.3, §4.3.2)
- **timed notification** — 0이 아닌 시간 인자로 `notify` 함수를 호출한 결과로 만들어지는 notification. (§4.3.2, §5.10.6)
- **timed notification phase** — timed notification의 결과로 프로세스들이 runnable해지는 스케줄러 내 제어 단계. (§4.3.2.6)
- **timing annotation** — `b_transport`와 `nb_transport` 메서드의 `sc_time` 인자. timing annotation은 local time offset이다. 트랜잭션의 recipient는 `effective_local_time = sc_time_stamp() + local_time_offset` 시점에 트랜잭션을 받은 것처럼 동작할 것이 요구된다. (§11.2.3.12, §11.2.4)
- **timing point** — 트랜잭션 수명 중의 유의미한 시점. loosely-timed 트랜잭션은 `b_transport`의 호출과 반환에 대응하는 두 개의 timing point를 갖는다. approximately-timed base protocol 트랜잭션은 각각 phase 전이에 대응하는 네 개의 timing point를 갖는다.
- **TLM-1** — OSCI Transaction Level Modeling 표준의 첫 주요 버전. TLM-1은 2005년에 릴리스되었다. (Clause 9)
- **TLM-2.0** — OSCI Transaction Level Modeling 표준의 두 번째 주요 버전. TLM-2.0은 2008년에 처음 릴리스되었고 OSCI TLM-2.0 LRM은 2009년에 릴리스되었다. (Clause 9)
- **TLM-2.0-compliant implementation** — TLM-2.0 interoperability layer와 TLM-2.0 utilities를 모두 포함하여, 본 표준에 기술된 모든 TLM-2.0 클래스를 본 표준에 기술된 의미론으로 제공하는 구현. (§9.2)
- **top-level module, top-level object** — 다른 어떤 모듈이나 프로세스 안에도 인스턴스화되지 않은 모듈 또는 객체. top-level 모듈은 `sc_main` 안에서 인스턴스화되거나, `sc_main`이 없는 경우 구현 특정 메커니즘으로 식별된다. (§3.1.4, §5.16.1)
- **traits class** — C++ 프로그래밍에서, 주 클래스의 동작을 특수화하는 데 쓰이는 typedef 같은 정의를 담는 클래스. 통상 traits class를 주 클래스에 템플릿 인자로 전달하는 방식으로 쓴다. 기본 템플릿 파라미터가 주 클래스의 기본 traits를 제공한다. (§14.2.3, §14.2.4)
- **transaction** — 둘 이상의 동시 프로세스 사이 상호작용 또는 통신에 대한 추상화. 트랜잭션은 어트리뷰트 집합을 운반하며 시간상 한정된다. 즉 어트리뷰트는 특정 시간 창 안에서만 유효하다. 트랜잭션에 연관된 타이밍은 트랜잭션 타입에 따라 특정 timing point 집합으로 제한된다. 프로토콜에 따라 프로세스가 트랜잭션의 어트리뷰트를 읽거나 수정하는 것이 허용될 수 있다.
- **transaction bridge** — 들어오는 트랜잭션에 대해서는 target으로, 나가는 트랜잭션에 대해서는 initiator로 동작하는 컴포넌트. 보통 bus bridge를 모델링하기 위한 것이다. 참조: bridge. (§10.4)
- **transaction instance** — 트랜잭션의 유일한 인스턴스. 하나의 transaction instance는 하나의 트랜잭션 객체로 표현되지만, 같은 트랜잭션 객체가 여러 transaction instance에 재사용될 수 있다.
- **transaction level (TL)** — 동시 프로세스 사이의 통신이 핀 위글링에서 트랜잭션으로 추상화되는 추상화 레벨. 이 용어는 시간·구조·동작의 추상화에 관해 어떤 특정 세분성도 함의하지 않는다. (§10.2)
- **transaction object** — 트랜잭션과 연관된 어트리뷰트를 저장하는 객체. 트랜잭션 객체의 타입은 core interface에 템플릿 인자로 전달된다.
- **transaction level model, transaction level modeling (TLM)** — 각각 트랜잭션 레벨의 모델과 그런 모델을 만드는 행위. 트랜잭션 레벨 모델은 RTL 모델이 쓰는 개별 핀·네트에 이벤트를 설정하는 방식과 대조적으로, 통상 함수 호출로 통신한다. (§10.2)
- **transactor** — 트랜잭션 레벨 인터페이스를 핀 레벨 인터페이스(일반적 의미의 interface)에 연결하거나, 둘 이상의 트랜잭션 레벨 인터페이스를(흔히 서로 다른 추상화 레벨의) 서로 연결하는 모듈. 전형적으로 첫 트랜잭션 레벨 인터페이스는 메모리 맵드 버스나 다른 프로토콜을 나타내고, 두 번째 인터페이스는 그 프로토콜의 더 낮은 추상화 레벨 구현을 나타낸다. 단 하나의 transactor가 여러 트랜잭션 레벨 또는 핀 레벨 인터페이스를 가질 수도 있다. 참조: adapter; bridge.
- **transparent component** — 들어오는 모든 interface method call이 지연 없이, 그리고 인자나 트랜잭션 객체에 대한 수정 없이(extension은 예외) 컴포넌트를 즉시 통과해 전파되는 성질을 갖는 interconnect component. transparent component의 의도는 checker와 monitor가 ignorable phase를 통과시킬 수 있게 하는 것이다. (§15.2.5)
- **transport interface** — TLM-1의 유일한 양방향 core interface. transport interface는 request 트랜잭션 객체를 caller에서 callee로 전달하고, response 트랜잭션 객체를 callee에서 caller로 반환한다. TLM-2.0은 별도의 blocking·non-blocking transport interface를 추가한다. (§10.3.9)
- **trigger** — method process instance에 연관된 멤버 함수가 그 sensitivity에 따라 스케줄러에 의해 호출되게 하는 것. 참조: method process; sensitivity. (§5.2.10)

## U

- **undefined** — 구현에 대한 어떤 의무도 없음. 본 표준이 어떤 동작이나 결과가 undefined라고 서술하는 경우, 구현은 error나 warning을 생성할 수도 있고 안 할 수도 있다. (§3.3.6)
- **unidirectional interface** — 트랜잭션 객체의 어트리뷰트가 첫 timing point와 트랜잭션 수명 종료 사이 기간 동안 엄격히 read-only인 TLM-1 트랜잭션 레벨 인터페이스. 사실상 트랜잭션 객체가 표현하는 정보는 caller→callee 또는 callee→caller 중 한 방향으로만 엄격히 전달된다. `void put(const T& t)`의 경우 첫 timing point는 함수 호출로 표시된다. `void get(T& t)`의 경우 첫 timing point는 함수로부터의 반환으로 표시된다. `T get()`의 경우 엄밀히는 두 개의 별개 트랜잭션 객체가 있으며, 함수로부터의 반환이 첫 객체의 축약된 수명 종료와 두 번째 객체의 첫 timing point를 표시한다. (§17.2.1)
- **unspawned process** — elaboration 중에 `SC_METHOD`, `SC_THREAD`, `SC_CTHREAD` 세 매크로 중 하나를 호출하여 생성된 프로세스. 참조: process. (§3.1.4, §4.2.3)
- **untimed** — 시간이나 사이클에 대한 명시적 언급이 없지만 동시성과 연산의 순서화를 포함하는 모델링 스타일. 시간 개념이 명시적으로 없으므로, 다수의 동시 스레드에 걸친 연산 순서화는 이벤트·mutex·blocking FIFO 같은 동기화 프리미티브로 달성되어야 한다. 프로토콜 견고성 테스트를 위해 untimed 기술에 임의 지연을 삽입하는 관행을 채택하는 사용자도 있지만, 이 관행이 모델링 스타일의 기본 특성을 바꾸지는 않는다. (§10.3.2)
- **update phase** — primitive channel의 값이 갱신되는 스케줄러 내 제어 단계. update phase는 직전 evaluation phase 중에 멤버 함수 `request_update`를 호출한 모든 primitive channel에 대해 멤버 함수 `update`를 실행하는 것으로 구성된다. (§4.3.2.4)
- **user** — 애플리케이션의 작성자. 구현을 만드는 implementor와 구별된다. user는 사람일 수도 있고 컴퓨터 프로그램 같은 자동화 프로세스일 수도 있다. (§3.1.2)
- **user-defined conversion** — conversion function 또는 정확히 하나의 파라미터를 갖는 non-explicit 생성자. 참조: conversion function; implicit conversion. (C++ 용어)
- **utilities** — 편의를 위해서만 제공되며 트랜잭션 레벨 모델 간 상호운용성 달성에 엄격히 필요하지는 않은 TLM-2.0 표준의 클래스 집합. (Clause 16)

## V

- **valid** — process handle의 상태, 또는 함수에 포인터/참조로 전달되거나 반환된 객체의 상태로, 그 핸들이나 객체가 삭제되지 않았고 그 값이나 동작이 애플리케이션에 접근 가능한 채로 남아 있는 기간 동안의 상태. process handle은 process instance와 연관되어 있을 때 valid하다. (§3.3.4, §5.6.1)
- **variable-precision fixed-point type** — 클래스 `sc_fxval`과 `sc_fxval_fast`. 오버플로와 양자화 효과를 모델링하지 않지만 많은 고정소수점 연산의 피연산자 타입과 반환 타입으로 쓰이는 가변 정밀도 고정소수점 값을 표현한다. 이 타입들은 통상 애플리케이션이 직접 쓰지 않는다. (§7.1)
- **vector** — 참조: bit vector; logic vector. (§7.1, §8.5)

## W

- **warning** — 구현이 report-handling 메커니즘(`sc_report_handler`의 `report` 함수)을 사용하여 severity `SC_WARNING`으로 진단 메시지를 생성할 의무. (§3.3.6)
- **within** — 인스턴스의 생성자가 모듈의 생성자에서 호출되고, 또한 그 인스턴스가 중첩된 모듈 안에 있지 않은 경우, 그 인스턴스와 모듈 사이에 성립하는 관계. (§3.1.4)

## Y

- **yield** — SystemC 스케줄러에 제어를 되돌리는 것. thread process에서 yield는 `wait`를 호출하는 것이다. method process에서 yield는 함수에서 반환하는 것이다.
