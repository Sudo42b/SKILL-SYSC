# Clause 13 — Combined TLM-2.0 interfaces and sockets

LRM pp. 482–492.

TLM-2.0 **interoperability layer**의 핵심. Clause 10~12의 core interface들을 forward/backward 두 묶음으로 결합하고,
port+export를 한 객체로 묶은 initiator/target socket을 정의한다. **모든 TLM-2.0 모델의 연결 지점.**

---

## 13.1 Combined interfaces

### 13.1.1 Introduction

- Combined forward/backward transport interface는 initiator·target socket이 쓸 TLM-2.0 core interface들을 그룹화한다.
- 포함: transport, DMI, debug transport interface. **TLM-1 core interface는 포함하지 않는다.**
- forward interface = initiator socket → target socket 방향(forward path)의 메서드 호출.
  backward interface = target socket → initiator socket 방향(backward path).
- **blocking transport interface와 debug transport interface는 backward calling path를 필요로 하지 않는다.**
- 표준 initiator/target socket과 무관한 새 socket class template을 combined interface로 인스턴스화하는 것은 기술적으로 가능하지만,
  **interoperability를 위해 비권장(not recommended)**. 반대로 **표준 socket에서 파생시키는 것은 권장(recommended)**.
- Combined interface template은 **protocol traits class**로 파라미터화된다. traits class는 forward/backward interface가 쓰는
  타입 — payload type과 phase type — 을 정의한다. 하나의 traits class는 특정 protocol에 대응한다.
  기본 protocol type은 `tlm_base_protocol_types` (15.2 참조).

### 13.1.2 Class definition

```cpp
namespace tlm {

// The default protocol traits class:
struct tlm_base_protocol_types
{
    typedef tlm_generic_payload                tlm_payload_type;
    typedef tlm_phase                          tlm_phase_type;
};

// The combined forward interface:
template< typename TYPES = tlm_base_protocol_types >
class tlm_fw_transport_if
    : public virtual tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type,
                                                   typename TYPES::tlm_phase_type>
    , public virtual tlm_blocking_transport_if<    typename TYPES::tlm_payload_type>
    , public virtual tlm_fw_direct_mem_if <        typename TYPES::tlm_payload_type>
    , public virtual tlm_transport_dbg_if<         typename TYPES::tlm_payload_type>
{};

// The combined backward interface:
template < typename TYPES = tlm_base_protocol_types >
class tlm_bw_transport_if
    : public virtual tlm_bw_nonblocking_transport_if< typename TYPES::tlm_payload_type,
                                                      typename TYPES::tlm_phase_type >
    , public virtual tlm_bw_direct_mem_if
{};

} // namespace tlm
```

| interface | 상속하는 core interface |
|---|---|
| `tlm_fw_transport_if<TYPES>` | `tlm_fw_nonblocking_transport_if`, `tlm_blocking_transport_if`, `tlm_fw_direct_mem_if`, `tlm_transport_dbg_if` (모두 virtual public) |
| `tlm_bw_transport_if<TYPES>` | `tlm_bw_nonblocking_transport_if`, `tlm_bw_direct_mem_if` (모두 virtual public) |

> `tlm_bw_direct_mem_if`는 payload로 템플릿화되지 않는다 (`invalidate_direct_mem_ptr`은 주소 범위만 받음).

---

## 13.2 Initiator and target sockets

### 13.2.1 Introduction

- **socket = port + export**.
  - initiator socket: forward path용 **port**, backward path용 **export**.
  - target socket: forward path용 **export**, backward path용 **port**.
- socket은 SystemC의 port binding operator를 오버로드해서, 상대 socket의 export/port에 port와 export를 **한 번에** 바인딩한다.
- **hierarchical binding (parent↔child) 시 바인딩 순서를 신중히 고려해야 한다.**
- initiator/target socket은 C++ 상속 계층으로 구현되며, 애플리케이션은 보통 **가장 파생된
  `tlm_initiator_socket` / `tlm_target_socket`만 직접 사용**한다.
- 두 socket은 protocol traits class로 파라미터화된다. **socket끼리는 protocol type이 동일할 때만 바인딩 가능.**
  기본값은 `tlm_base_protocol_types`.
- 새 protocol을 정의하면, generic payload 기반이든 아니든 **새 protocol traits class로 combined interface template을
  인스턴스화해야 권장(should)**.

socket이 주는 이점:

| | 내용 |
|---|---|
| a) | forward/backward 양쪽의 transport·DMI·debug transport interface를 **하나의 객체로 그룹화** |
| b) | forward/backward path의 port와 export를 **한 번의 호출로 바인딩**하는 메서드 제공 |
| c) | 호환되지 않는 protocol type으로 파라미터화된 socket 바인딩 시 **강한 타입 체크** |
| d) | transaction 해석에 쓸 수 있는 **bus width 파라미터** 포함 |

- `tlm_initiator_socket`·`tlm_target_socket`은 TLM-2.0 **interoperability layer**에 속한다.
- 추가로 utilities namespace에 파생 socket 계열이 있으며, 이를 통칭 **convenience socket**이라 한다.

### 13.2.2 Class definition

```cpp
namespace tlm {

enum tlm_socket_category
{
   TLM_UNKNOWN_SOCKET,
   TLM_INITIATOR_SOCKET,
   TLM_TARGET_SOCKET
};

class tlm_base_socket_if
{
public:
    virtual sc_core::sc_port_base &                         get_base_port() = 0;
    virtual sc_core::sc_port_base const &                   get_base_port() const = 0;
    virtual sc_core::sc_export_base &                       get_base_export() = 0;
    virtual sc_core::sc_export_base const &                 get_base_export() const = 0;
    virtual unsigned int                                    get_bus_width() const = 0;
    virtual std::type_index                                 get_protocol_types() const = 0;
    virtual tlm_socket_category                             get_socket_category() const = 0;
    virtual bool                                            is_multi_socket() const = 0;

protected:
    virtual ~tlm_base_socket_if() {}
};

// Abstract base class for initiator sockets
template <
    unsigned int BUSWIDTH = 32,
    typename FW_IF = tlm_fw_transport_if<>,
    typename BW_IF = tlm_bw_transport_if<>
>
class tlm_base_initiator_socket_b : public tlm_base_socket_if
{
public:
    virtual ~tlm_base_initiator_socket_b() {}

    virtual sc_core::sc_port_b<FW_IF> &                         get_base_port() = 0;
    virtual const sc_core::sc_port_b<FW_IF> &                   get_base_port() const = 0;

    virtual BW_IF &                                             get_base_interface() = 0;
    virtual const BW_IF &                                       get_base_interface() const = 0;

    virtual sc_core::sc_export<BW_IF> &                         get_base_export() = 0;
    virtual const sc_core::sc_export<BW_IF> &                   get_base_export() const = 0;
};

// Abstract base class for target sockets
template <
    unsigned int BUSWIDTH = 32,
    typename FW_IF = tlm_fw_transport_if<>,
    typename BW_IF = tlm_bw_transport_if<>
>
class tlm_base_target_socket_b : public tlm_base_socket_if
{
public:
    virtual ~tlm_base_target_socket_b();

    virtual sc_core::sc_port_b<BW_IF> &                         get_base_port() = 0;
    virtual const sc_core::sc_port_b<BW_IF> &                   get_base_port() const = 0;

    virtual sc_core::sc_export<FW_IF> &                         get_base_export() = 0;
    virtual const sc_core::sc_export <FW_IF> &                  get_base_export() const = 0;

    virtual FW_IF &                                             get_base_interface() = 0;
    virtual const FW_IF &                                       get_base_interface() const = 0;
};

// Base class for initiator sockets, providing binding methods
template <
    unsigned int BUSWIDTH = 32,
    typename FW_IF = tlm_fw_transport_if<>,
    typename BW_IF = tlm_bw_transport_if<>,
    int N = 1,
    sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
>
class tlm_base_initiator_socket : public tlm_base_initiator_socket_b<BUSWIDTH, FW_IF, BW_IF>,
                                  public sc_core::sc_port<FW_IF, N, POL>
{
public:
    typedef FW_IF                                               fw_interface_type;
    typedef BW_IF                                               bw_interface_type;
    typedef sc_core::sc_port<fw_interface_type, N, POL>         port_type;
    typedef sc_core::sc_export<bw_interface_type>               export_type;
    typedef tlm_base_target_socket_b<BUSWIDTH, fw_interface_type, bw_interface_type>
                                                                base_initiator_socket_type;
    typedef tlm_base_initiator_socket_b<BUSWIDTH, fw_interface_type, bw_interface_type>
                                                                base_type;

    tlm_base_initiator_socket();
    explicit tlm_base_initiator_socket(const char* name);
    virtual const char* kind() const;

    virtual void bind(base_target_socket_type& s);
    void operator() (base_target_socket_type& s);
    virtual void bind(base_type& s);
    void operator() (base_type& s);
    virtual void bind(bw_interface_type& ifs);
    void operator() (bw_interface_type& s);

    virtual unsigned int         get_bus_width() const { return BUSWIDTH; }
    virtual tlm_socket_category  get_socket_category() const final { return TLM_INITIATOR_SOCKET; }
    virtual bool                 is_multi_socket() const final { return (N != 1); }

    // Implementation of pure virtual functions of base class
    virtual sc_core::sc_port_b<FW_IF> &        get_base_port()             { return *this; }
    virtual const sc_core::sc_port_b<FW_IF> &  get_base_port() const       { return *this; }

    virtual BW_IF &                            get_base_interface()        { return m_export; }
    virtual const BW_IF &                      get_base_interface() const  { return m_export; }

    virtual sc_core::sc_export<BW_IF> &        get_base_export()           { return m_export; }
    virtual const sc_core::sc_export<BW_IF> &  get_base_export() const     { return m_export; }

protected:
    export_type m_export;
};

// Base class for target sockets, providing binding methods
template <
    unsigned int BUSWIDTH = 32,
    typename FW_IF = tlm_fw_transport_if<>,
    typename BW_IF = tlm_bw_transport_if<>,
    int N = 1,
    sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
>
class tlm_base_target_socket : public tlm_base_target_socket_b<BUSWIDTH, FW_IF, BW_IF>,
                               public sc_core::sc_export<FW_IF>
{
public:
    typedef FW_IF                                            fw_interface_type;
    typedef BW_IF                                            bw_interface_type;
    typedef sc_core::sc_port<bw_interface_type, N, POL>      port_type;
    typedef sc_core::sc_export<fw_interface_type>            export_type;
    typedef tlm_base_initiator_socket_b<BUSWIDTH, fw_interface_type, bw_interface_type>
                                                             base_initiator_socket_type;
    typedef tlm_base_target_socket_b<BUSWIDTH, fw_interface_type, bw_interface_type>
                                                             base_type;

    tlm_base_target_socket();
    explicit tlm_base_target_socket(const char* name);
    virtual const char* kind() const;

    virtual void bind(base_initiator_socket_type& s);
    void operator() (base_initiator_socket_type& s);
    virtual void bind(base_type& s);
    void operator() (base_type& s);
    virtual void bind(fw_interface_type& ifs);
    void operator() (fw_interface_type& s);

    int size() const;
    bw_interface_type* operator-> ();
    bw_interface_type* operator[] (int i);

    virtual unsigned int         get_bus_width() const { return BUSWIDTH; }
    virtual tlm_socket_category  get_socket_category() const final { return TLM_TARGET_SOCKET; }
    virtual bool                 is_multi_socket() const final { return (N != 1; }   // 원문 표기 그대로

    // Implementation of pure virtual functions of base class
    virtual sc_core::sc_port_b<BW_IF> &        get_base_port()             { return m_port; }
    virtual const sc_core::sc_port_b<BW_IF> &  get_base_port() const       { return *this; }

    virtual FW_IF &                            get_base_interface()        { return *this; }
    virtual const FW_IF &                      get_base_interface() const  { return *this; }

    virtual sc_core::sc_export<FW_IF> &        get_base_export()           { return *this; }
    virtual const sc_core::sc_export<FW_IF> &  get_base_export() const     { return *this; }

protected:
    port_type m_port;
};

// Principal initiator socket, parameterized with protocol traits class
template <
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm_base_protocol_types,
    int N = 1,
    sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
>
class tlm_initiator_socket : public tlm_base_initiator_socket <
    BUSWIDTH, tlm_fw_transport_if<TYPES>, tlm_bw_transport_if<TYPES>, N, POL>
{
public:
    tlm_initiator_socket();
    explicit tlm_initiator_socket(const char* name);
    virtual const char* kind() const;

    virtual std::type_index get_protocol_types() const final { return std::type_index(typeid(TYPES)); }
};

// Principal target socket, parameterized with protocol traits class
template <
    unsigned int BUSWIDTH = 32,
    typename TYPES = tlm_base_protocol_types,
    int N = 1,
    sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
>
class tlm_target_socket : public tlm_base_target_socket <
    BUSWIDTH, tlm_fw_transport_if<TYPES>, tlm_bw_transport_if<TYPES>, N, POL>
{
public:
    tlm_target_socket();
    explicit tlm_target_socket(const char* name);
    virtual const char* kind() const;

    virtual std::type_index get_protocol_types() const final { return std::type_index(typeid(TYPES)); }
};

} // namespace tlm
```

### 템플릿 파라미터 요약 (기본값 포함)

| 클래스 | 템플릿 파라미터 (기본값) |
|---|---|
| `tlm_base_initiator_socket_b` | `BUSWIDTH = 32`, `FW_IF = tlm_fw_transport_if<>`, `BW_IF = tlm_bw_transport_if<>` |
| `tlm_base_target_socket_b` | 동일 |
| `tlm_base_initiator_socket` | `BUSWIDTH = 32`, `FW_IF = tlm_fw_transport_if<>`, `BW_IF = tlm_bw_transport_if<>`, `N = 1`, `POL = sc_core::SC_ONE_OR_MORE_BOUND` |
| `tlm_base_target_socket` | 동일 |
| `tlm_initiator_socket` | `BUSWIDTH = 32`, `TYPES = tlm_base_protocol_types`, `N = 1`, `POL = sc_core::SC_ONE_OR_MORE_BOUND` |
| `tlm_target_socket` | 동일 |
| `tlm_fw_transport_if` / `tlm_bw_transport_if` | `TYPES = tlm_base_protocol_types` |

### 상속 관계

```
tlm_base_socket_if
 ├─ tlm_base_initiator_socket_b<BUSWIDTH,FW_IF,BW_IF>
 │   └─ tlm_base_initiator_socket<...,N,POL>   (+ sc_core::sc_port<FW_IF,N,POL>)
 │       └─ tlm_initiator_socket<BUSWIDTH,TYPES,N,POL>
 └─ tlm_base_target_socket_b<BUSWIDTH,FW_IF,BW_IF>
     └─ tlm_base_target_socket<...,N,POL>      (+ sc_core::sc_export<FW_IF>)
         └─ tlm_target_socket<BUSWIDTH,TYPES,N,POL>
```

**initiator socket은 `sc_port` 파생, target socket은 `sc_export` 파생** — 이 사실이 hierarchical binding 순서 규칙의 근거다.

---

## 13.2.3 `tlm_base_socket_if`

- a) 추상 base class `tlm_base_socket_if`는 pure virtual 함수를 선언하며, 파생 socket class에서 **오버라이드해야 권장(should)**.
- b) `get_base_port`·`get_base_export`는 파생 socket class와 연관된 port·export를 각각 **비템플릿 base type으로 반환해야 shall**
  (`sc_core::sc_port_base&`, `sc_core::sc_export_base&`).
- c) `get_bus_width`, `get_protocol_types`, `get_socket_category`, `is_multi_socket`은 파생 socket class에서 쓰인
  **템플릿 파라미터의 값 또는 그에 관한 정보를 반환하려는 의도**다.

## 13.2.4 `tlm_base_initiator_socket_b` / `tlm_base_target_socket_b`

- a) 두 추상 base class는 `tlm_base_socket_if` 추상 base class를 **상속해야 shall**.
- b) `get_base_port`·`get_base_export`의 오버라이딩 멤버 함수를 **제공해야 shall**. 반환 타입은
  `tlm_base_socket_if`에서 정의된 반환값의 **covariant** (FW_IF, BW_IF로 템플릿화된 타입).
- c) `get_interface`는 파생 socket class와 연관된 interface 객체를 반환하려는 의도.

## 13.2.5 `tlm_base_initiator_socket` / `tlm_base_target_socket`

### 상속·이름 규칙

| 항목 | 규칙 |
|---|---|
| a) | `tlm_base_initiator_socket`은 `tlm_base_initiator_socket_b`에서 파생되어야 shall |
| b) | `tlm_base_target_socket`은 `tlm_base_target_socket_b`에서 파생되어야 shall |
| e) | `kind()`는 클래스명을 C string으로 반환해야 shall — `"tlm_base_initiator_socket"` / `"tlm_base_target_socket"` |

**c) initiator socket 이름 생성 (shall)**
- name 인자 있는 생성자: 문자열을 base class `sc_port` 생성자에 전달해 module 계층 내 인스턴스 이름을 설정하고,
  **같은 문자열에 접미사 `"_export"`를 붙이고 `sc_gen_unique_name`을 호출**해 backward path의 `sc_export` 이름을 설정한다.
  예: `tlm_initiator_socket("foo")` → port 이름 `"foo"`, export 이름 `"foo_export"`.
- default 생성자: port는 `sc_gen_unique_name("tlm_base_initiator_socket")`,
  export는 `sc_gen_unique_name("tlm_base_initiator_socket_export")`로 이름 생성.

**d) target socket 이름 생성 (shall)**
- name 인자 있는 생성자: 문자열을 base class `sc_export` 생성자에 전달하고,
  같은 문자열에 접미사 `"_port"` + `sc_gen_unique_name`으로 backward path의 `sc_port` 이름 설정.
  예: `tlm_target_socket("foo")` → export 이름 `"foo"`, port 이름 `"foo_port"`.
- default 생성자: export는 `sc_gen_unique_name("tlm_base_target_socket")`,
  port는 `sc_gen_unique_name("tlm_base_target_socket_port")`.

### BUSWIDTH

- f) `get_bus_width`는 `BUSWIDTH` 템플릿 인자 값을 **반환해야 shall**.
- g) `BUSWIDTH`는 socket을 통해 전송되는 **개별 data word의 word length를 bit 수로** 결정해야 shall.
  burst transfer에서는 **burst의 각 beat의 bit 수**를 결정해야 shall.
  정확한 해석은 **transaction type에 의존한다 shall**. generic payload에서의 의미는 14.12 참조.
- h) socket-to-socket 바인딩 시 두 socket은 **동일한 BUSWIDTH 값을 가져야 shall**.
  initiator나 target의 실행 코드가 BUSWIDTH를 읽고 그에 따라 동작해도 된다 may.

### bind / operator() 오버로드 전부

**`tlm_base_initiator_socket`**

| 오버로드 | 효과 |
|---|---|
| `virtual void bind(base_target_socket_type& s)` | initiator socket ↔ target socket 바인딩 |
| `void operator() (base_target_socket_type& s)` | 위와 동일 (내부적으로 `bind` 호출) |
| `virtual void bind(base_type& s)` | initiator socket ↔ initiator socket (hierarchical) |
| `void operator() (base_type& s)` | 위와 동일 |
| `virtual void bind(bw_interface_type& ifs)` | socket의 **export를 channel(interface 구현 객체)에** 바인딩 |
| `void operator() (bw_interface_type& s)` | 위와 동일 |

**`tlm_base_target_socket`**

| 오버로드 | 효과 |
|---|---|
| `virtual void bind(base_initiator_socket_type& s)` | target socket ↔ initiator socket 바인딩 |
| `void operator() (base_initiator_socket_type& s)` | 위와 동일 |
| `virtual void bind(base_type& s)` | target socket ↔ target socket (hierarchical) |
| `void operator() (base_type& s)` | 위와 동일 |
| `virtual void bind(fw_interface_type& ifs)` | socket의 **export를 channel에** 바인딩 |
| `void operator() (fw_interface_type& s)` | 위와 동일 |

규칙:
- i) socket을 인자로 받는 `bind`/`operator()`는 **자신이 속한 socket 인스턴스를 인자로 넘어온 socket 인스턴스에 바인딩해야 shall**.
- j) interface를 인자로 받는 `bind`/`operator()`는 **자신이 속한 socket의 export를 인자로 넘어온 channel 인스턴스에
  바인딩해야 shall**. (channel = interface를 구현하는 class를 가리키는 SystemC 용어.)
- k) 각 경우 `operator()`의 구현은 **대응하는 virtual 멤버 함수 `bind`를 호출해서** 효과를 달성해야 shall.

### 소켓 바인딩 규칙 (핵심)

| 조합 | 규칙 |
|---|---|
| **initiator → target** (l, m) | `bind`/`operator()`는 initiator socket의 **port를 target socket의 export**에, target socket의 **port를 initiator socket의 export**에 각각 바인딩해야 shall. **계층상 같은 레벨**에서 socket-to-socket 바인딩할 때 사용. 어느 쪽 socket의 `bind`/`operator()`를 호출해도 **효과는 정확히 동일**하며, 두 경우 모두 forward path는 initiator socket → target socket 방향이다. |
| **initiator → initiator, target → target** (n) | `bind`/`operator()`는 **한 socket의 port를 다른 socket의 port에**, **한 socket의 export를 다른 socket의 export에** 바인딩해야 shall. **hierarchical binding** 용도 — child module의 socket을 parent module의 socket에, 또는 parent의 socket을 child의 socket에 바인딩해 transaction을 계층 위/아래로 전달. |

**o) hierarchical binding 순서 (must)**
- **initiator socket ↔ initiator socket**: **child의 socket을 parent의 socket에** 바인딩해야 한다.
  → `child->init_socket.bind(parent_init_socket)`
- **target socket ↔ target socket**: **parent의 socket을 child의 socket에** 바인딩해야 한다.
  → `parent_targ_socket.bind(child->targ_socket)`
- 근거: `tlm_base_initiator_socket`은 `sc_port` 파생, `tlm_base_target_socket`은 `sc_export` 파생.
  **계층을 올라갈 때는 port-to-port, 최상단 가로지를 때는 port-to-export, 계층을 내려갈 때는 export-to-export.**

**p) `tlm_base_initiator_socket`과 `tlm_base_target_socket` 두 socket이 바인딩되려면
동일한 forward/backward interface type과 동일한 bus width를 공유해야 한다 (must).**

### target socket의 port 위임 함수

| 함수 | 규칙 |
|---|---|
| `size()` | target socket 내부 port(backward path)의 `size`를 호출하고 그 반환값을 반환해야 shall |
| `operator->()` | 내부 port의 `operator->`를 호출하고 그 반환값을 반환해야 shall |
| `operator[](int)` | 내부 port의 `operator[]`를 **같은 인자로** 호출하고 그 반환값을 반환해야 shall |

### multi-socket (t~v)

- `tlm_base_initiator_socket`·`tlm_base_target_socket`은 **multi-socket으로 동작해도 된다 may**:
  하나의 initiator socket이 여러 target socket에, 하나의 target socket이 여러 initiator socket에 바인딩될 수 있다.
- 두 class template은 바인딩 개수(`N`)와 port binding policy(`POL`) 템플릿 파라미터를 갖고,
  이들은 내부 `sc_port` 템플릿 인스턴스화를 파라미터화하는 데 쓰인다.
- 여러 번 바인딩된 경우 `operator[]`로 대응 객체를 지정할 수 있다. **index는 `bind`/`operator()`가 호출된 순서**로 결정.
- 단, 그런 socket이 **수신하는 incoming interface method call은 anonymous** — 호출자를 식별할 메커니즘이 없다.
  호출자 식별 메커니즘은 **convenience socket이 제공**한다 (16.2.4).
- 예: 두 target에 바인딩된 socket에서 `socket[0]->nb_transport_fw(...)`와 `socket[1]->nb_transport_fw()`는
  두 target을 각각 지정하지만, 그 둘 중 하나에서 들어온 `nb_transport_bw()`의 호출자를 식별할 방법은 없다.

### get_base_port / get_base_export / get_base_interface 구현 (w)

| 함수 | 반환해야 하는 것 (shall) |
|---|---|
| `get_base_port` | socket의 **port 객체** |
| `get_base_export` | socket의 **export 객체** |
| `get_base_interface` | initiator port의 경우 **export 객체**, target socket의 경우 **socket 객체 자신** |

### 카테고리 질의 (x, y)

- `get_socket_category`는 `tlm_base_initiator_socket`에 대해 `TLM_INITIATOR_SOCKET`,
  `tlm_base_target_socket`에 대해 `TLM_TARGET_SOCKET`을 **반환해야 shall**.
- `is_multi_socket`은 템플릿 파라미터 `N`이 1과 다르면(= multi-socket이면) **true를 반환해야 shall**.

---

## 13.2.6 `tlm_initiator_socket` / `tlm_target_socket`

- a) 이 두 socket은 **protocol traits class를 템플릿 파라미터로** 받는다.
  애플리케이션은 base socket보다 **이 socket들(또는 여기서 파생된 convenience socket)을 쓰는 것이 일반적으로 권장(should)**.
- b) `tlm_initiator_socket`은 `tlm_base_initiator_socket`에서, `tlm_target_socket`은 `tlm_base_target_socket`에서
  **파생되어야 shall**.
- c) 두 class의 생성자는 각자의 base class 생성자를 **호출해야 shall**. `char*` 인자가 있으면 그대로 전달.
- d) 두 socket이 바인딩되려면 **같은 protocol traits class(기본 `tlm_base_protocol_types`)와 같은 bus width를
  공유해야 한다 (must)**. 서로 다른 protocol마다 별도의 protocol traits class를 정의하면
  (generic payload 기반 여부와 무관하게) socket 간 강한 타입 체크를 얻을 수 있다.
- e) `kind()`는 `"tlm_initiator_socket"` / `"tlm_target_socket"`을 **반환해야 shall**.
- f) `get_protocol_types`는 `TYPES` 템플릿 파라미터에 대응하는 **`std::type_index` 객체를 반환해야 shall**.

### Example

```cpp
#include <systemc>
#include <tlm>
using namespace sc_core;
using namespace std;

struct Initiator : sc_module, tlm::tlm_bw_transport_if<> { // Initiator가 bw interface를 구현

    tlm::tlm_initiator_socket<32> init_socket;             // protocol types는 base protocol로 기본값
    SC_CTOR(Initiator)
            : init_socket("init_socket") {
        SC_THREAD(thread);
        init_socket.bind(*this);                           // initiator socket을 initiator 자신에게 바인딩
    }

    void thread() {                                        // 프로세스가 placeholder transaction 하나 생성
        tlm::tlm_generic_payload trans;
        sc_time delay = SC_ZERO_TIME;
        init_socket->b_transport(trans, delay);
    }

    virtual tlm::tlm_sync_enum nb_transport_bw(
             tlm::tlm_generic_payload &trans,
             tlm::tlm_phase &phase,
             sc_core::sc_time &t) {
         return tlm::TLM_COMPLETED;                        // placeholder 구현
    }

    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range)
    {}                                                     // placeholder 구현
};

struct Target : sc_module, tlm::tlm_fw_transport_if<> {    // Target이 fw interface를 구현

    tlm::tlm_target_socket<32> targ_socket;                // protocol types는 base protocol로 기본값

    SC_CTOR(Target) : targ_socket("targ_socket") {
       targ_socket.bind(*this);                            // target socket을 target 자신에게 바인딩
    }

    virtual tlm::tlm_sync_enum nb_transport_fw(
             tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &t) {
         return tlm::TLM_COMPLETED;                        // placeholder 구현
    }

    virtual void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
    }                                                      // placeholder 구현

    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi_data) {
         return false;                                     // placeholder 구현
    }

    virtual unsigned int transport_dbg(tlm::tlm_generic_payload &trans) {
         return 0;                                         // placeholder 구현
    }
};

SC_MODULE(Top1) {                          // initiator-target의 단순 비계층 바인딩
   Initiator *init;
   Target *targ;

    SC_CTOR(Top1) {
       init = new Initiator("init");
       targ = new Target("targ");
       init->init_socket.bind(targ->targ_socket);          // initiator socket을 target socket에 바인딩
    }
};

struct Parent_of_initiator : sc_module {                   // 계층적 socket 바인딩
    tlm::tlm_initiator_socket<32> init_socket;
    Initiator *initiator;

    SC_CTOR(Parent_of_initiator) : init_socket("init_socket") {
       initiator = new Initiator("initiator");
       initiator->init_socket.bind(init_socket);   // child의 initiator socket을 parent initiator socket에
    }
};

struct Parent_of_target : sc_module {
    tlm::tlm_target_socket<32> targ_socket;
    Target *target;

    SC_CTOR(Parent_of_target) : targ_socket("targ_socket") {
       target = new Target("target");
       targ_socket.bind(target->targ_socket);      // parent target socket을 child target socket에
    }
};

SC_MODULE(Top2) {
   Parent_of_initiator *init;
   Parent_of_target *targ;

    SC_CTOR(Top2) {
       init = new Parent_of_initiator("init");
       targ = new Parent_of_target("targ");
       init->init_socket.bind(targ->targ_socket);          // 최상위에서 initiator socket을 target socket에
    }
};
```

---

## 흔한 위반 (pitfalls)

- **hierarchical binding 방향을 반대로 씀** — LRM §13.2.5 o): initiator socket끼리는 **child를 parent에**,
  target socket끼리는 **parent를 child에** 바인딩해야 한다. `init_socket.bind(child->init_socket)`처럼
  parent를 child에 바인딩하면 port-to-port 규칙(계층 올라갈 때 port-to-port, 내려갈 때 export-to-export)에 위배된다.
- **서로 다른 BUSWIDTH의 socket을 바인딩** — LRM §13.2.5 h): socket-to-socket 바인딩 시 BUSWIDTH가 동일해야 shall.
- **서로 다른 protocol traits class의 socket을 바인딩** — LRM §13.2.6 d): 같은 traits class와 같은 bus width를 공유해야 한다.
  `tlm_base_protocol_types`와 사용자 정의 traits class는 서로 바인딩되지 않는다(컴파일 타임 타입 체크).
- **combined interface로 비표준 socket template을 직접 만들어 씀** — LRM §13.1.1: 기술적으로 가능하나
  interoperability 관점에서 **비권장(not recommended)**. 표준 socket에서 파생시키는 편이 권장.
- **multi-socket에서 incoming call의 호출자를 식별하려 시도** — LRM §13.2.5 u), v): base socket의 incoming IMC는
  anonymous이며 호출자 식별 메커니즘이 없다. 필요하면 convenience socket(16.2.4)을 쓸 것.
- **TLM-1 interface를 combined interface에서 기대** — LRM §13.1.1: combined interface에는 TLM-1 core interface가
  포함되지 않는다.
