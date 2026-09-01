# Clause 8 — SystemC utilities

LRM pp. 411–439.

트레이스 파일·리포트 핸들러·벡터·유틸 함수. **디버깅 출력과 반복 구조 기술의 표준 수단.**

각 절은 LRM 형식을 따른다: Description → Class definition → Constraints on usage → Member functions.

---

## 8.1 Trace files

### 8.1.1 Overview

트레이스 파일은 시뮬레이션 중 값 변화를 시간 순서로 기록한다. **VCD 형식은 지원해야 shall.**

| 규칙 | 내용 |
|---|---|
| 생성 | VCD 트레이스 파일은 `sc_create_vcd_trace_file` 호출로**만** 생성·오픈될 수 있다 |
| 오픈 시점 | elaboration 중 또는 simulation 중 **아무 때나** may |
| 값 추적 | 값은 `sc_trace` 호출로**만** 추적될 수 있다 |
| 순서 | 트레이스 파일을 **먼저 열어야** shall 값을 추적할 수 있다 |
| **핵심 제약** | 파일을 연 뒤 **하나 이상의 delta cycle이 경과했으면** 그 파일에 값을 추적하면 안 된다 shall not |
| 닫기 | VCD 트레이스 파일은 `sc_close_vcd_trace_file` 호출로 닫아야 shall |
| 닫는 시점 | 시뮬레이션의 **최종 delta cycle 이전에 닫으면 안 된다** shall not |

- 구현은 `sc_create_vcd_trace_file` / `sc_close_vcd_trace_file`의 대안을 제공해 **다른 트레이스 파일 형식을 지원해도 된다** may.
- 추적 대상 객체의 lifetime이 트레이스 파일이 열려 있는 전 기간에 걸칠 필요는 **없다**.

> NOTE(informative) — 트레이스 파일은 아무 때나 열 수 있지만, 시뮬레이션 종료 전에 트레이싱을 **끄는 메커니즘은 없다**.

### 8.1.2 Class definition and function declarations

```cpp
namespace sc_core {

class sc_trace_file
{
public:
    virtual void set_time_unit( double , sc_time_unit ) = 0;
    implementation-defined
};

sc_trace_file* sc_create_vcd_trace_file( const char* name );
void sc_close_vcd_trace_file( sc_trace_file* tf );
void sc_write_comment( sc_trace_file* tf , const std::string& comment );
void sc_trace ...

}          // namespace sc_core
```

### 8.1.3 `sc_trace_file`

VCD 및 기타 implementation-defined 트레이스 파일 형식의 file handle 클래스가 파생되는 **추상 기반 클래스**.

- 애플리케이션은 `sc_trace_file` 객체를 **생성하면 안 된다** shall not. 다만 이 타입의 포인터·참조를 정의해도 된다 may.
- `set_time_unit`은 파생 클래스에서 override되어야 shall 하며 트레이스 파일의 time unit을 설정한다.
- `double` 인자 값은 **양수여야 shall** 하고 **10의 거듭제곱이어야 shall**.
- `set_time_unit`을 한 번도 호출하지 않으면 기본 트레이스 파일 time unit은 **1 picosecond**.

### 8.1.4 `sc_create_vcd_trace_file`

```cpp
sc_trace_file* sc_create_vcd_trace_file( const char* name );
```

- 새 `sc_trace_file` file handle 객체를 생성하고, 연결된 새 VCD 파일을 열고, handle 포인터를 반환해야 shall.
- 파일 이름은 인자로 넘긴 문자열에 **`".vcd"` 를 덧붙여** 구성되어야 shall.

### 8.1.5 `sc_close_vcd_trace_file`

```cpp
void sc_close_vcd_trace_file( sc_trace_file* tf );
```

VCD 파일을 닫고 인자가 가리키는 **file handle을 delete해야 shall**.

### 8.1.6 `sc_write_comment`

```cpp
void sc_write_comment( sc_trace_file* tf , const std::string& comment );
```

두 번째 인자 문자열을 첫 번째 인자의 트레이스 파일에 **주석으로**, **함수가 호출된 시뮬레이션 시각에** 기록해야 shall.

### 8.1.7 `sc_trace` — 전체 오버로드

```cpp
void sc_trace( sc_trace_file* , const bool& , const std::string& );
void sc_trace( sc_trace_file* , const bool* , const std::string& );
void sc_trace( sc_trace_file* , const float& , const std::string& );
void sc_trace( sc_trace_file* , const float* , const std::string& );
void sc_trace( sc_trace_file* , const double& , const std::string& );
void sc_trace( sc_trace_file* , const double* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_logic& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_logic* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_int_base& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_int_base* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_uint_base& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_uint_base* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_signed& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_signed* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_unsigned& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_unsigned* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_bv_base& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_bv_base* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_lv_base& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_lv_base* , const std::string& );

void sc_trace( sc_trace_file* , const sc_dt::sc_fxval& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_fxval* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_fxval_fast& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_fxval_fast* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_fxnum& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_fxnum* , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_fxnum_fast& , const std::string& );
void sc_trace( sc_trace_file* , const sc_dt::sc_fxnum_fast* , const std::string& );

// 정수/문자 계열 — width 기본 인자 있음 (& 와 * 두 형태 모두 존재)
void sc_trace( sc_trace_file* , const unsigned char&  , const std::string& , int width = 8 * sizeof( unsigned char ) );
void sc_trace( sc_trace_file* , const unsigned char*  , const std::string& , int width = 8 * sizeof( unsigned char ) );
void sc_trace( sc_trace_file* , const unsigned short& , const std::string& , int width = 8 * sizeof( unsigned short ) );
void sc_trace( sc_trace_file* , const unsigned short* , const std::string& , int width = 8 * sizeof( unsigned short ) );
void sc_trace( sc_trace_file* , const unsigned int&   , const std::string& , int width = 8 * sizeof( unsigned int ) );
void sc_trace( sc_trace_file* , const unsigned int*   , const std::string& , int width = 8 * sizeof( unsigned int ) );
void sc_trace( sc_trace_file* , const unsigned long&  , const std::string& , int width = 8 * sizeof( unsigned long ) );
void sc_trace( sc_trace_file* , const unsigned long*  , const std::string& , int width = 8 * sizeof( unsigned long ) );
void sc_trace( sc_trace_file* , const char&  , const std::string& , int width = 8 * sizeof( char ) );
void sc_trace( sc_trace_file* , const char*  , const std::string& , int width = 8 * sizeof( char ) );
void sc_trace( sc_trace_file* , const short& , const std::string& , int width = 8 * sizeof( short ) );
void sc_trace( sc_trace_file* , const short* , const std::string& , int width = 8 * sizeof( short ) );
void sc_trace( sc_trace_file* , const int&   , const std::string& , int width = 8 * sizeof( int ) );
void sc_trace( sc_trace_file* , const int*   , const std::string& , int width = 8 * sizeof( int ) );
void sc_trace( sc_trace_file* , const long&  , const std::string& , int width = 8 * sizeof( long ) );
void sc_trace( sc_trace_file* , const long*  , const std::string& , int width = 8 * sizeof( long ) );
void sc_trace( sc_trace_file* , const sc_dt::int64&  , const std::string& , int width = 8 * sizeof( sc_dt::int64 ) );
void sc_trace( sc_trace_file* , const sc_dt::int64*  , const std::string& , int width = 8 * sizeof( sc_dt::int64 ) );
void sc_trace( sc_trace_file* , const sc_dt::uint64& , const std::string& , int width = 8 * sizeof( sc_dt::uint64 ) );
void sc_trace( sc_trace_file* , const sc_dt::uint64* , const std::string& , int width = 8 * sizeof( sc_dt::uint64 ) );

// 인터페이스
template <class T>
void sc_trace( sc_trace_file* , const sc_signal_in_if<T>& , const std::string& );

void sc_trace( sc_trace_file* , const sc_signal_in_if<char>&  , const std::string& , int width );
void sc_trace( sc_trace_file* , const sc_signal_in_if<short>& , const std::string& , int width );
void sc_trace( sc_trace_file* , const sc_signal_in_if<int>&   , const std::string& , int width );
void sc_trace( sc_trace_file* , const sc_signal_in_if<long>&  , const std::string& , int width );

// event / sc_time
void sc_trace( sc_trace_file* , const event& , const std::string& );
void sc_trace( sc_trace_file* , const sc_time& , const std::string&);
void sc_trace( sc_trace_file* , const event* , const std::string& );
void sc_trace( sc_trace_file* , const sc_time* , const std::string&);
```

- `sc_trace`는 두 번째 인자 값을 첫 번째 인자의 트레이스 파일에 추적해야 shall 하며, 세 번째 인자 문자열로 트레이스 파일 내에서 그 값을 식별한다.
- **호출 시점부터 트레이스 파일이 닫힐 때까지** 발생하는 두 번째 인자 값의 **모든 변화가 기록되어야 shall**.
- `sc_time` 값은 `sc_trace_file::set_time_unit`으로 지정한 단위로 표현되어야 shall.

> NOTE 1(informative) — `sc_trace`는 이 표준의 다른 곳(6.8.4, 6.10.5)에서도 추가 데이터 타입 지원을 위해 오버로드된다.
>
> NOTE 2(informative) — `sc_trace`에 넘긴 값, `sc_trace_file::set_time_unit`에 넘긴 값, `sc_core::sc_set_time_resolution`에 넘긴 값에 따라 추적되는 시간 값에 **정밀도 손실**이 생길 수 있다. 구현은 그런 경우 warning을 낼 수 있다 may.

---

## 8.2 `sc_report`

### 8.2.1 Description

`sc_report`는 `sc_report_handler::report`가 생성한 리포트의 인스턴스를 표현한다.

- `sc_report` 객체는 특정 severity level·message type에 대해 **`SC_CACHE_REPORT` 액션이 설정된 경우** 애플리케이션이 접근 가능하다.
- report handler가 던진 `sc_report` 객체를 애플리케이션이 catch해도 된다 may (8.3 참조).
- `sc_severity`는 리포트의 severity level을 표현한다.

### 8.2.2 Class definition

```cpp
namespace sc_core {

enum sc_severity {
   SC_INFO = 0,
   SC_WARNING,
   SC_ERROR,
   SC_FATAL,
   SC_MAX_SEVERITY
};

enum sc_verbosity {
   SC_NONE   = 0,
   SC_LOW    = 100,
   SC_MEDIUM = 200,
   SC_HIGH   = 300,
   SC_FULL   = 400,
   SC_DEBUG  = 500
};

class sc_report
: public std::exception
{
public:

     sc_report( const sc_report& );
     sc_report& operator= ( const sc_report& );
     virtual ~sc_report() throw();

     sc_severity get_severity() const;
     const char* get_msg_type() const;
     const char* get_msg() const;
     int get_verbosity() const;
     const char* get_file_name() const;
     int get_line_number() const;

     const sc_time& get_time() const;
     const char* get_process_name() const;

     virtual const char* what() const throw();
};

}         // namespace sc_core
```

### 8.2.3 Constraints on usage

- `sc_report` 객체는 `sc_report_handler::report` 호출로 생성된다.
- 애플리케이션은 **copy constructor 호출 외의 방법으로 `sc_report` 객체를 직접 생성하면 안 된다** shall not.
- `sc_report` 객체의 개별 속성은 `sc_report_handler::report` 함수로**만** 설정될 수 있다 may only.
- 구현은 액션 `SC_THROW`에 대한 응답으로 `sc_report_handler::default_handler`에서 `sc_report` 객체를 **던져야 shall**.
- 애플리케이션은 자체 report handler 함수에서 `sc_report` 객체를 던져도 된다 may. try-block에서 catch해도 된다 may.

### 8.2.4 `sc_verbosity`

`sc_verbosity` enum은 `sc_report_handler::set_verbosity_level`과 `sc_report_handler::report`에 인자로 넘길 수 있는 **지시적(indicative) verbosity level** 값을 제공한다.

| 값 | 정수 |
|---|---|
| `SC_NONE` | 0 |
| `SC_LOW` | 100 |
| `SC_MEDIUM` | 200 |
| `SC_HIGH` | 300 |
| `SC_FULL` | 400 |
| `SC_DEBUG` | 500 |

### 8.2.5 `sc_severity`

- severity level은 **네 개여야 shall**.
- `SC_MAX_SEVERITY`는 severity level이 **아니다 shall not**. `sc_severity` 타입 인자를 요구하는 함수에 `SC_MAX_SEVERITY` 값을 넘기는 것은 **error**.

**Table 50 — Levels for `sc_severity`** (의도된 의미. 정확한 의미는 `sc_report_handler`가 override 가능)

| Severity level | Description |
|---|---|
| `SC_INFO` | 정보성 메시지 |
| `SC_WARNING` | 잠재적 문제 |
| `SC_ERROR` | 애플리케이션이 복구 가능할 수도 있는 실제 문제 |
| `SC_FATAL` | 애플리케이션이 복구 불가능한 실제 문제 |

### 8.2.6 Copy constructor and assignment

```cpp
sc_report( const sc_report& );
sc_report& operator= ( const sc_report& );
```

각각 인자로 넘긴 `sc_report` 객체의 **deep copy를 생성해야 shall**.

### 8.2.7 Member functions

여기 명시된 여러 멤버 함수는 null-terminated 문자열 포인터를 반환한다. 구현은 반환된 문자열을 **`sc_report` 객체의 lifetime 동안만** 유효하게 유지할 의무가 있다.

| 함수 | 규칙 |
|---|---|
| `get_severity()` / `get_msg_type()` / `get_msg()` / `get_verbosity()` / `get_file_name()` / `get_line_number()` | 대응하는 속성을 반환해야 shall. 속성 자체는 `sc_report_handler::report`에 인자로 넘겨서**만** 설정 가능. **severity level이 `SC_INFO`가 아니면 `get_verbosity` 반환값은 implementation-defined** |
| `get_time()` / `get_process_name()` | 대응 속성 반환 shall. 이 속성들은 리포트가 생성된 **시뮬레이션 시각**과 생성된 **process instance**에 따라 `report`가 설정해야 shall |
| `what()` | severity level, message type, message, file name, line number, process name, time으로 구성된 텍스트 문자열을 반환해야 shall. 구현은 severity level에 따라 내용을 달리해도 된다 may |

### Example

```cpp
try {
    ...
    SC_REPORT_ERROR("msg_type", "msg");
    ...
    } catch ( sc_core::sc_report e ) {
        std::cout << "Caught " << e.what() << std::endl;
    }
```

---

## 8.3 `sc_report_handler`

### 8.3.1 Description

예외적 상황 발생 시 텍스트 리포트를 출력하고, 그 리포트 생성 시 실행될 애플리케이션 고유 동작을 정의하는 기능을 제공한다.

- 멤버 함수 `report`가 리포팅 메커니즘의 **중심**이며, 기본 액션·기본 핸들러만으로 리포트를 생성하기에 그것만으로 충분하다.
- 구현은 예외적 상황을 보고할 필요가 있을 때마다 `report`를 **호출해야 shall**.
- `report`는 IP 벤더·EDA 툴 벤더·최종 사용자가 만든 SystemC 애플리케이션에서 호출해도 된다 may.
- 액션은 **severity level과 message type**으로 분류된다. message type 선택은 최종 사용자가 액션을 충분히 제어할 수 있도록 **신중해야 should**.
- 권장 message type 형식:

  ```
  "/originating_company_or_institution/product_identifier/subcategory/subcategory..."
  ```

- precompiled SystemC 코드를 배포하는 측은 최종 사용자가 액션 설정 목적으로 구분할 필요가 있는 리포트에 **고유 message type을 할당할 책임이 있다**.

### 8.3.2 Class definition

```cpp
namespace sc_core {

typedef unsigned sc_actions;

enum {
   SC_UNSPECIFIED,
   SC_DO_NOTHING,
   SC_THROW,
   SC_LOG,
   SC_DISPLAY,
   SC_CACHE_REPORT,
   SC_INTERRUPT,
   SC_STOP,
   SC_ABORT,

   // default action constants
   SC_DEFAULT_INFO_ACTIONS    = SC_LOG | SC_DISPLAY,
   SC_DEFAULT_WARNING_ACTIONS = SC_LOG | SC_DISPLAY,
   SC_DEFAULT_ERROR_ACTIONS   = SC_LOG | SC_CACHE_REPORT | SC_THROW,
   SC_DEFAULT_FATAL_ACTIONS   = SC_LOG | SC_DISPLAY | SC_CACHE_REPORT | SC_ABORT
};

typedef void ( * sc_report_handler_proc ) ( const sc_report& , const sc_actions& );

class sc_report_handler
{
public:
    static void report( sc_severity , const char* msg_type , const char* msg , const char* file , int line );
    static void report( sc_severity , const char* msg_type , const char* msg , int verbosity,
                                                                           const char* file , int line );

     static sc_actions set_actions( sc_severity , sc_actions = SC_UNSPECIFIED );
     static sc_actions set_actions( const char * msg_type , sc_actions = SC_UNSPECIFIED );
     static sc_actions set_actions( const char * msg_type , sc_severity , sc_actions = SC_UNSPECIFIED );

     static int stop_after( sc_severity , int limit = -1 );
     static int stop_after( const char* msg_type , int limit = -1 );
     static int stop_after( const char* msg_type , sc_severity , int limit = -1 );

     static int get_count( sc_severity );
     static int get_count( const char* msg_type );
     static int get_count( const char* msg_type , sc_severity );

     int set_verbosity_level( int );
     int get_verbosity_level();

     static sc_actions suppress( sc_actions );
     static sc_actions suppress();
     static sc_actions force( sc_actions );
     static sc_actions force();

     static void set_handler( sc_report_handler_proc );
     static sc_report_handler_proc get_handler();
     static void default_handler( const sc_report& , const sc_actions& );
     static sc_actions get_new_action_id();

     static sc_report* get_cached_report();
     static void clear_cached_report();

     static bool set_log_file_name( const char* );
     static const char* get_log_file_name();
};

#define SC_REPORT_INFO_VERB( msg_type , msg, verbosity ) \
             sc_report_handler::report( SC_INFO , msg_type , msg , verbosity, __FILE__ , __LINE__ )

#define SC_REPORT_INFO( msg_type , msg ) \
             sc_report_handler::report( SC_INFO , msg_type , msg , __FILE__ , __LINE__ )

#define SC_REPORT_WARNING( msg_type , msg ) \
             sc_report_handler::report( SC_WARNING , msg_type , msg , __FILE__ , __LINE__ )

#define SC_REPORT_ERROR( msg_type , msg ) \
             sc_report_handler::report( SC_ERROR , msg_type , msg , __FILE__ , __LINE__ )

#define SC_REPORT_FATAL( msg_type , msg ) \
             sc_report_handler::report( SC_FATAL , msg_type , msg , __FILE__ , __LINE__ )

#define sc_assert( expr ) \
              ( ( void ) ( ( expr ) ? 0 : ( ::sc_core::sc_assertion_failed(#expr,__FILE__,__LINE__ ) , 0 ) ) )
   [[noreturn]] void sc_assertion_failed(const char* msg, const char* file, int line);

void sc_interrupt_here( const char* msg_type , sc_severity );
void sc_stop_here( const char* msg_type , sc_severity );

}         // namespace sc_core
```

`sc_actions` enum 값들의 **실제 정수 값은 implementation-defined**이며, 여러 값의 **bitwise 조합이 가능하도록 선택되어야 shall**.

### 8.3.3 Constraints on usage

- `sc_report_handler`의 멤버 함수들은 **elaboration 중이든 simulation 중이든 언제든 호출 가능**.
- 액션은 해당 severity level 또는 message type이 `report`의 인자로 **처음 사용되기 전에도 후에도** 설정 가능.

### 8.3.4 `sc_actions`

- `sc_actions` typedef는 **각 비트가 서로 다른 액션을 나타내는 word**. 둘 이상의 비트가 설정될 수 있으며, 그 경우 **대응하는 모든 액션이 실행되어야 shall**.
- 이 enum은 **default handler가 인식·수행하는 액션 집합**을 정의한다. `set_handler`로 설정한 애플리케이션 고유 핸들러는 이 집합을 수정·확장해도 된다 may.
- `SC_UNSPECIFIED`는 액션이 아니라, `sc_actions` 타입 변수·인자의 **기본값**으로서 "액션이 설정되지 않았음"을 의미한다.
- 반면 `SC_DO_NOTHING`은 **구체적 액션**이며, 8.3.6의 규칙에 따라 **더 낮은 우선순위로 설정된 모든 액션을 억제해야 shall**.
- 각 severity level에는 이름에 어울리는 기본 액션 집합이 연관되며, `set_actions` 호출로 override 가능. 기본 액션은 다음 매크로로 정의되어야 shall:

| severity | 기본 액션 매크로 | 값 |
|---|---|---|
| `SC_INFO` | `SC_DEFAULT_INFO_ACTIONS` | `SC_LOG \| SC_DISPLAY` |
| `SC_WARNING` | `SC_DEFAULT_WARNING_ACTIONS` | `SC_LOG \| SC_DISPLAY` |
| `SC_ERROR` | `SC_DEFAULT_ERROR_ACTIONS` | `SC_LOG \| SC_CACHE_REPORT \| SC_THROW` |
| `SC_FATAL` | `SC_DEFAULT_FATAL_ACTIONS` | `SC_LOG \| SC_DISPLAY \| SC_CACHE_REPORT \| SC_ABORT` |

### 8.3.5 `report`

```cpp
static void report( sc_severity , const char* msg_type , const char* msg , const char* file , int line );
static void report( sc_severity , const char* msg_type , const char* msg , int verbosity, const char* file ,
                                                                                            int line );
```

- 리포트를 생성하고 아래 정의된 적절한 액션이 취해지도록 해야 shall.
- 첫 인자 severity와 둘째 인자 message type을 사용해, 이전의 `set_actions` / `stop_after` / `suppress` / `force` 호출 결과로 **실행할 액션 집합을 결정해야 shall**.
- **다섯 개 인자 값 전부로 초기화한 `sc_report` 객체를 생성해야 shall** 하고, 이를 `set_handler`가 설정한 핸들러에 전달해야 shall.
- 그 `sc_report` 객체는 **`SC_CACHE_REPORT` 액션이 설정된 경우가 아니면 `report` 호출 이후까지 존속하지 않아야 shall**. 설정된 경우 `get_cached_reports` 호출로 회수 가능.
- 구현은 **process instance마다 별도의 `sc_report` 캐시**와, process 밖에서의 `report` 호출을 위한 **단일 global 캐시**를 유지해야 shall. **각 캐시는 가장 최근 리포트 하나만 저장해야 shall**.
- 실행할 액션 집합 **결정**은 `report`의 책임, 그 액션의 **실행**은 `set_handler`가 설정한 핸들러 함수의 책임 shall.
- `report`는 8.3.7에 기술된 대로 생성 리포트 수의 카운트를 유지해야 shall. 이 카운트는 **액션이 실행되든 억제되든 관계없이 증가되어야 shall**. **단, verbosity level 때문에 리포트가 무시된 경우에는 카운트를 증가시키지 않아야 shall**.

**verbosity 인자 처리 규칙**

| 조건 | 동작 |
|---|---|
| verbosity 인자 존재 + severity == `SC_INFO` + verbosity > 최대 verbosity level | **어떤 액션도 실행하지 않고, 어떤 카운트도 증가시키지 않고 반환해야 shall** |
| verbosity 인자 부재 + severity == `SC_INFO` | verbosity 인자가 있고 값이 **`SC_MEDIUM`**인 것처럼 동작해야 shall |
| severity != `SC_INFO` | 구현은 verbosity 인자를 **무시해야 shall** |

**매크로**

`SC_REPORT_INFO_VERB`, `SC_REPORT_INFO`, `SC_REPORT_WARNING`, `SC_REPORT_ERROR`, `SC_REPORT_FATAL`, `sc_assert`는 `report` 호출 편의를 위해 제공되지만, 애플리케이션이 이 매크로를 **사용할 의무는 없다**.

`sc_assert`가 호출하는 `sc_assertion_failed` 함수는 **implementation-defined**이지만, **`[[noreturn]]` C++ attribute를 가져야 하고 FATAL error가 보고되도록 해야 한다**.

> NOTE(informative) — `sc_report`는 이 속성들의 초기화를 위해 `sc_report_handler` 전용 constructor를 제공할 수 있다.

### 8.3.6 `set_actions`

```cpp
static sc_actions set_actions( sc_severity , sc_actions = SC_UNSPECIFIED );
static sc_actions set_actions( const char * msg_type , sc_actions = SC_UNSPECIFIED );
static sc_actions set_actions( const char * msg_type , sc_severity , sc_actions = SC_UNSPECIFIED );
```

- 주어진 severity level, message type, 또는 둘 다로 `report`가 호출될 때 취할 액션을 설정해야 shall.

**우선순위 (낮음 → 높음)**

1. `set_actions( sc_severity , sc_actions )`
2. `set_actions( const char* msg_type , sc_actions )`
3. `set_actions( const char* msg_type , sc_severity , sc_actions )`

즉 **message type이 severity level보다 우선하고, message type + severity 조합이 각각 단독보다 우선한다 shall**. **더 낮은 우선순위 매치의 액션은 억제되어야 shall.**

**반환값**

- 각 호출은 해당 severity / message type / (severity, message type) 쌍에 대해 **이전 호출이 설정한 액션을 대체해야 shall**.
- 반환값은 **동일한 오버로딩**에 대해 그 severity / message type / 쌍에 대한 **직전 호출이 설정한 액션**.
- `set_actions( sc_severity , sc_actions )`의 **첫 호출**은 해당 severity level의 **기본 액션**을 반환해야 shall.
- 나머지 두 함수의 특정 message type에 대한 **첫 호출**은 **`SC_UNSPECIFIED`**를 반환해야 shall.
- 세 오버로드는 이 점에서 **각각 독립적으로 동작한다**. 우선순위는 `report`가 호출될 때만 의미가 있다.

### Example

```cpp
sc_core::sc_report_handler::set_actions(sc_core::SC_WARNING, sc_core::SC_DO_NOTHING);
sc_core::sc_report_handler::set_actions("/Acme_IP", sc_core::SC_DISPLAY);
sc_core::sc_report_handler::set_actions("/Acme_IP", sc_core::SC_INFO, sc_core::SC_DISPLAY |
                                                                      sc_core::SC_CACHE_REPORT);
...
SC_REPORT_WARNING("", "1");                        // 침묵
SC_REPORT_WARNING("/Acme_IP", "2");                // 표준 출력에 기록
SC_REPORT_INFO("/Acme_IP", "3");                   // 표준 출력에 기록 + 캐시
```

### 8.3.7 `stop_after`

```cpp
static int stop_after( sc_severity , int limit = -1 );
static int stop_after( const char* msg_type , int limit = -1 );
static int stop_after( const char* msg_type , sc_severity , int limit = -1 );
```

- `report`는 **각 severity level, 각 message type, 각 (severity, message type) 쌍마다 독립적인 카운트**를 유지해야 shall.
- `stop_after`는 각 경우에 생성될 리포트 수의 **한계를 설정해야 shall**.
- `report`는 주어진 경우에 대해 **정확히 `limit` 개수의 리포트가 생성되었을 때 `sc_stop`을 호출해야 shall**.

**우선순위 (낮음 → 높음)** — `set_actions`와 동일

1. `stop_after( sc_severity , int )`
2. `stop_after( const char* msg_type , int )`
3. `stop_after( const char* msg_type , sc_severity , int )`

`report`가 둘 이상의 limit과 매치되는 (severity, message type) 조합으로 호출되면 **더 높은 우선순위의 limit만 효력을 가진다 shall**.

**카운트 규칙**

- 특정 severity level / message type / 쌍에 대해 `report`가 **처음 호출될 때 카운트는 값 1로 초기화되어야 shall**.
- `stop_after` 호출로 카운트가 수정·리셋되지 않아야 shall.
- `report` 호출마다 **액션 실행 여부와 무관하게 세 카운트 모두 증가되어야 shall**. (severity, message type) 쌍의 카운트가 증가할 때 **해당 severity의 카운트와 해당 message type의 카운트도 함께 증가되어야 shall**.
- `stop_after` 호출 시점에 이미 카운트가 설정하려는 limit에 도달·초과했다면, **`sc_stop`은 즉시 호출되지 않고 해당 카운트가 다음에 증가할 때 호출되어야 shall**.

**limit 값**

| limit | 의미 |
|---|---|
| `-1` (기본값) | stop limit 없음. 이미 설정된 stop limit을 **제거해야 shall** |
| `0` | 해당 경우에 stop limit 없음. 게다가 **명시적 0은 더 낮은 우선순위 경우의 동작을 override해야 shall**. 단, 명시적 0이어도 해당 경우에 설정된 액션(또는 기본 액션)이 `sc_stop`·abort 호출이나 예외 throw를 일으킬 수 있다 may |

- `report`가 severity `SC_FATAL`로 호출되면, `set_actions`·`stop_after` 호출이 전혀 없는 경우의 기본 동작은 **`SC_ABORT`을 포함한** 액션 집합의 실행이다.
- 반환값은 **동일한 오버로딩**에 대해 그 severity / message type / 쌍에 대한 **직전 호출이 설정한 limit**. 그렇지 않으면 기본 limit인 **`-1`**.

### Example 1

```cpp
sc_core::sc_report_handler::stop_after(sc_core::SC_WARNING, 1);
sc_core::sc_report_handler::stop_after("/Acme_IP", 2);
sc_core::sc_report_handler::stop_after("/Acme_IP", sc_core::SC_WARNING, 3);
...
SC_REPORT_WARNING("/Acme_IP", "Overflow");
SC_REPORT_WARNING("/Acme_IP", "Conflict");
SC_REPORT_WARNING("/Acme_IP", "Misuse");                       // sc_core::sc_stop() 호출됨
```

### Example 2

```cpp
sc_core::sc_report_handler::stop_after(sc_core::SC_WARNING, 5);
sc_core::sc_report_handler::stop_after("/Acme_IP", sc_core::SC_WARNING, 1);
...
SC_REPORT_WARNING("/Star_IP", "Unexpected");
SC_REPORT_INFO("/Acme_IP", "Invoked");
SC_REPORT_WARNING("/Acme_IP", "Mistimed");                     // sc_core::sc_stop() 호출됨
```

### 8.3.8 `get_count`

```cpp
static int get_count( sc_severity );
static int get_count( const char* msg_type );
static int get_count( const char* msg_type , sc_severity );
```

`report`가 유지하는, 각 severity level / message type / (severity, message type) 쌍의 리포트 생성 카운트를 반환해야 shall. 해당 경우로 `report`가 호출된 적이 없으면 **값 0을 반환해야 shall**.

### 8.3.9 Verbosity level

- 최대 verbosity level은 **단일 global 값**이며 **severity `SC_INFO` 리포트에만 적용되어야 shall**.
- severity `SC_INFO`이면서 verbosity level이 최대 verbosity level보다 **큰** 개별 리포트는 **무시되어야 shall**. 즉 구현은 그 리포트에 연관된 모든 액션을 사실상 억제해야 shall.

| 함수 | 규칙 |
|---|---|
| `int set_verbosity_level( int );` | 최대 verbosity level을 인자 값으로 설정하고 **이전 값을 반환해야 shall** |
| `int get_verbosity_level();` | 최대 verbosity level 값을 반환해야 shall |

### 8.3.10 `suppress` and `force`

```cpp
static sc_actions suppress( sc_actions );
static sc_actions suppress();
static sc_actions force( sc_actions );
static sc_actions force();
```

**`suppress`**

- 이후의 `report` 호출에 대해 주어진 액션 집합의 실행을 **억제해야 shall**.
- 반환값은 `suppress` 호출 **직전에 억제되고 있던 액션 집합** shall.
- 인자로 넘긴 액션이 **이전에 억제된 액션들을 완전히 대체해야 shall**. 억제 액션 집합은 **단일 global 집합 하나뿐**.
- 기본값: 억제 액션 없음.
- **인자 목록이 비어 있으면** 억제 액션 집합이 clear되어 기본 동작이 복원되어야 shall.
- 특정 액션의 억제가 억제되지 않은 다른 액션의 실행을 방해하지 않아야 shall.

**`force`**

- 이후의 `report` 호출에 대해 주어진 액션 집합의 실행을 **강제해야 shall**.
- 반환값은 `force` 호출 **직전에 강제되고 있던 액션 집합** shall.
- 인자로 넘긴 액션이 **이전에 강제된 액션들을 완전히 대체해야 shall**. 강제 액션 집합도 **단일 global 집합 하나뿐**.
- 기본값: 강제 액션 없음. 인자 목록이 비어 있으면 clear되어 기본 동작 복원 shall.
- 강제 액션은 해당 severity level의 **기본 액션에 더해서**, 그리고 `set_actions`로 설정된 어떤 액션에도 **더해서** 실행되어야 shall.

**충돌 규칙** — 동일한 액션이 suppress되면서 동시에 force되면 **force가 우선한다 shall**.

### 8.3.11 `set_handler` / `get_handler` / `default_handler`

```cpp
typedef void ( * sc_report_handler_proc ) ( const sc_report& , const sc_actions& );
static void set_handler( sc_report_handler_proc );
static sc_report_handler_proc get_handler();
static void default_handler( const sc_report& , const sc_actions& );
```

| 함수 | 규칙 |
|---|---|
| `set_handler` | `report`에서 호출될 핸들러 함수를 설정해야 shall. 애플리케이션 고유 report handler 제공 수단 |
| `get_handler` | 핸들러 함수를 반환해야 shall |
| `default_handler` | 기본 핸들러. 즉 `set_handler` 호출이 전혀 없으면 `report`에서 `default_handler`가 호출되어야 shall. **두 번째 인자 값에 따라** Table 51의 액션을 0개, 1개, 또는 그 이상 수행해야 shall |

**composite message** — severity level, message type, message, file name, line number, process name, `sc_report` 객체의 time으로 구성된 텍스트 문자열이어야 shall. 구현은 severity level에 따라 내용을 달리해도 된다 may.

> NOTE(informative) — 기본 핸들러를 복원하려면 `set_handler( &sc_report_handler::default_handler )`를 호출한다.

**Table 51 — Actions by `default_handler`**

| Action | Description |
|---|---|
| `SC_UNSPECIFIED` | 아무 동작 없음 (단, `report`는 더 낮은 우선순위 액션을 실행함) |
| `SC_DO_NOTHING` | 아무 동작 없음 (단, `report`가 더 낮은 우선순위 액션을 **억제**하게 함) |
| `SC_THROW` | `sc_report` 객체를 throw |
| `SC_LOG` | `set_log_file_name`으로 설정된 로그 파일에 composite message 기록 |
| `SC_DISPLAY` | composite message를 표준 출력에 기록 |
| `SC_CACHE_REPORT` | 아무 동작 없음 (단, `report`가 리포트를 **캐시**하게 함) |
| `SC_INTERRUPT` | `sc_report` 객체의 message type과 severity level을 인자로 `sc_interrupt_here` 호출 |
| `SC_STOP` | `sc_report` 객체의 message type과 severity level을 인자로 `sc_stop_here` 호출, 그 다음 `sc_stop` 호출 |
| `SC_ABORT` | `abort()` 호출 |

### 8.3.12 `get_new_action_id`

```cpp
static sc_actions get_new_action_id();
```

- 사용되지 않은 액션을 표현하는 `sc_actions` 값을 반환해야 shall. 반환값은 **정확히 한 비트만 설정된 word여야 shall**.
- 의도: 애플리케이션 고유 report handler 작성 시 액션 집합을 확장하는 데 사용.
- 사용 가능한 고유 값이 더 없으면 **`SC_UNSPECIFIED`를 반환해야 shall**.
- 애플리케이션은 **elaboration 시작 전에 `get_new_action_id`를 호출하면 안 된다 shall not**.

### 8.3.13 `sc_interrupt_here` and `sc_stop_here`

```cpp
void sc_interrupt_here( const char* msg_type , sc_severity );
void sc_stop_here( const char* msg_type , sc_severity );
```

- 각각 액션 타입 `SC_INTERRUPT`, `SC_STOP`에 대한 응답으로 `default_handler`에서 호출되어야 shall.
- 애플리케이션 고유 report handler에서 호출해도 된다 may.
- 의도: 사용자가 이 함수 안이나 위에 breakpoint를 걸 수 있게 하는 디버깅 보조. 이를 위해 구현은 severity 파라미터에 의존하는 switch 문으로 구현해도 된다 may.

### 8.3.14 `get_cached_report` and `clear_cached_report`

```cpp
static sc_report* get_cached_report();
static void clear_cached_report();
```

- `get_cached_report`는 process 안에서 호출되면 **현재 process instance의 가장 최근 캐시 리포트**를, 그 외에는 **global 캐시**의 것을 가리키는 포인터를 반환해야 shall. **이전 리포트들은 접근 불가능해야 shall.**
- `clear_cached_report`는 process 안에서 호출되면 현재 process instance의 리포트 캐시를, 그 외에는 global 캐시를 비워야 shall. 이후 `get_cached_report`는 해당 캐시에 새 리포트가 캐시될 때까지 **null 포인터를 반환**한다.

### 8.3.15 `set_log_file_name` and `get_log_file_name`

```cpp
static bool set_log_file_name( const char* );
static const char* get_log_file_name();
```

| 상황 | 동작 |
|---|---|
| 기본값 | 로그 파일 이름은 **빈 문자열 포인터** |
| non-empty 문자열 + 기존 이름 없음 | 인자 문자열을 **복제해 설정**하고 **`true` 반환** shall |
| non-empty 문자열 + 기존 이름 있음 | 기존 이름을 **수정하지 않고 `false` 반환** shall |
| empty 문자열 포인터 | 기존 로그 파일 이름을 **삭제**하고 **`false` 반환** shall |

- 로그 파일의 열기·쓰기·닫기는 **report handler의 책임 shall**.
- `default_handler`는 `SC_LOG` 액션에 대한 응답으로 `get_log_file_name`을 호출해야 shall. 애플리케이션 고유 report handler에서 호출해도 된다 may.

### Example

```cpp
sc_core::sc_report_handler::set_log_file_name("foo");                    // true 반환
sc_core::sc_report_handler::get_log_file_name();                         // "foo" 반환
sc_core::sc_report_handler::set_log_file_name("bar");                    // false 반환
sc_core::sc_report_handler::get_log_file_name();                         // "foo" 반환
sc_core::sc_report_handler::set_log_file_name(0);                        // false 반환
sc_core::sc_report_handler::get_log_file_name();                         // 0 반환
```

---

## 8.4 `sc_exception`

### 8.4.1 Description

SystemC report handler가 생성하는 리포트를 표현하는 `sc_report`는 `std::exception`에서 파생된다. typedef `sc_exception`은 이전 버전 SystemC 클래스 라이브러리와의 **하위 호환성**을 위해 존재한다 (8.2 참조).

### 8.4.2 Class definition

```cpp
namespace sc_core {

     typedef std::exception sc_exception;
}
```

---

## 8.5 `sc_vector`

### 8.5.1 Description

`sc_vector`는 모듈·채널·포트·export, 그 밖에 **`sc_object`에서 파생된 임의 타입**의 벡터를 만드는 데 쓰인다. 반복적·파라미터화된 구조 기술의 편의 수단.

- 벡터에서 원소를 골라내는 멤버 함수와 **port binding**(vector-of-ports를 vector-of-objects에 bind하는 것 포함) 멤버 함수를 제공한다.
- `sc_assemble_vector`(및 proxy 클래스 `sc_vector_assembly`)는 **vector-of-modules에 분산된 개별 객체들로부터 객체 벡터를 조립**하는 메커니즘을 제공한다. vector assembly는 port binding 등 여러 문맥에서 벡터 대신 쓸 수 있다.
- `sc_vector`는 vector-of-modules 생성 시 **사용자 정의 모듈 constructor 인자를 전달**하는 메커니즘을 제공한다.
- 이 clause에서 "vector"는 어떤 적절한 `T`에 대한 `sc_vector<T>` 객체를 뜻한다.

### 8.5.2 Class definition

```cpp
namespace sc_core {

enum sc_vector_init_policy
{
   SC_VECTOR_LOCK_AFTER_INIT,
   SC_VECTOR_LOCK_AFTER_ELABORATION
};

class sc_vector_base : public sc_object
{
    public:
        typedef implementation-defined size_type;

        virtual const char* kind() const;
        size_type size() const;
        const std::vector<sc_object*>& get_elements() const;
};

template< typename T >
class sc_vector_iter† : public std::iterator< std::random_access_iterator_tag, T >
{
    // Random Access Iterator category를 만족.
    // ISO/IEC 14882:2017 [lib.iterator.requirements] 참조
    implementation-defined
};

template< typename T >
class sc_vector : public sc_vector_base
{
    public:
        using sc_vector_base::size_type;
        typedef sc_vector_iter†<T> iterator;
        typedef sc_vector_iter†<const T> const_iterator;

        sc_vector();
        explicit sc_vector( const char* prefix );
        sc_vector( const char* prefix, size_type n,
                   sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );
        template< typename Creator >
        sc_vector( const char* prefix, size_type n,
                   Creator creator, sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );
        virtual ~sc_vector();

        void init( size_type n,
                   sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );
        static T* create_element( const char* prefix, size_type index );

        template< typename Creator >
        void init( size_type n,
                   Creator creator, sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );

        template< typename... Args >
        void emplace_back( Args&&... args );

        template< typename... Args >
        void emplace_back_with_name( Args &&... args );

        T& operator[] ( size_type );
        const T& operator[] ( size_type ) const;

        T& at( size_type );
        const T& at( size_type ) const;

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

        const_iterator cbegin() const;
        const_iterator cend() const;

        template< typename ContainerType, typename ArgumentType >
        iterator bind( sc_vector_assembly<ContainerType,ArgumentType> );

        template< typename BindableContainer >
        iterator bind( BindableContainer& );

        template< typename BindableIterator >
        iterator bind( BindableIterator , BindableIterator );

        template< typename BindableIterator >
        iterator bind( BindableIterator , BindableIterator , iterator );

        template< typename ContainerType, typename ArgumentType >
        iterator operator() ( sc_vector_assembly<ContainerType,ArgumentType> c );

        template< typename ArgumentContainer >
        iterator operator() ( ArgumentContainer& );

        template< typename ArgumentIterator >
        iterator operator() ( ArgumentIterator , ArgumentIterator );

        template< typename ArgumentIterator >
        iterator operator() ( ArgumentIterator , ArgumentIterator , iterator );

    private:
        // Disabled
        sc_vector( const sc_vector& );
        sc_vector& operator= ( const sc_vector& );
};

template< typename T, typename MT >
class sc_vector_assembly
{
    public:
        typedef implementation-defined size_type;
        typedef implementation-defined iterator;
        typedef implementation-defined const_iterator;
        typedef MT (T::*member_type);

        sc_vector_assembly( const sc_vector_assembly& );

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

        const_iterator cbegin() const;
        const_iterator cend() const;

        size_type size() const;
        std::vector< sc_object* > get_elements() const;

        iterator::reference operator[] ( size_type );
        const_iterator::reference operator[] ( size_type ) const;

        iterator::reference at( size_type );
        const_iterator::reference at( size_type ) const;

        template< typename ContainerType, typename ArgumentType >
        iterator bind( sc_vector_assembly<ContainerType, ArgumentType> );

        template< typename BindableContainer >
        iterator bind( BindableContainer& );

        template< typename BindableIterator >
        iterator bind( BindableIterator , BindableIterator );

        template< typename BindableIterator >
        iterator bind( BindableIterator , BindableIterator , iterator );

        template< typename BindableIterator >
        iterator bind( BindableIterator , BindableIterator , sc_vector<T>::iterator );

        template< typename ContainerType, typename ArgumentType >
        iterator operator() ( sc_vector_assembly<ContainerType, ArgumentType> );

        template< typename ArgumentContainer >
        iterator operator() ( ArgumentContainer& );

        template< typename ArgumentIterator >
        iterator operator() ( ArgumentIterator , ArgumentIterator );

        template< typename ArgumentIterator >
        iterator operator() ( ArgumentIterator , ArgumentIterator , iterator );

        template< typename ArgumentIterator >
        iterator operator()( ArgumentIterator , ArgumentIterator , sc_vector<T>::iterator );

    private:
        // Disabled
        sc_vector_assembly& operator=( const sc_vector_assembly& );
};

template< typename T, typename MT >
sc_vector_assembly<T,MT> sc_assemble_vector( sc_vector<T> & , MT (T::*member_ptr ) );

} // namespace sc_core
```

**Disabled 멤버**: `sc_vector`의 copy constructor와 copy assignment, `sc_vector_assembly`의 copy assignment (private, Disabled). `sc_vector_assembly`의 **copy constructor는 public** — 클래스는 copyable해야 shall.

`sc_vector_iter†` — † 표시 클래스.

### 8.5.3 Constraints on usage

- 애플리케이션은 `sc_vector<T>`를 **`sc_object`에서 파생된 타입 `T`로만 인스턴스화해야 shall**.
- custom creator를 쓰는 경우를 제외하고, 타입 `T`는 **`const char*`로부터 변환 가능한 타입의 인자 하나를 받는 constructor를 제공해야 shall**. 원소 constructor에 추가 인자를 넘겨야 하면 creator 함수/함수 객체를 쓸 수 있다 (8.5.5).
- `sc_vector<T>` 객체를 **언제 생성할 수 있는가는 템플릿 인자 `T`의 선택에 달려 있다**. `T` 자체에 적용되는 제약만 지킨다면, 벡터는 **simulation 중 동적으로 생성해도 된다** may. 예: **vector-of-modules는 elaboration 중에만 생성 가능**.
- 벡터의 크기는 **단 한 번만 설정될 수 있다** — 생성 시점이나 `init` 호출로. **벡터는 동적으로 resize할 수 없다.**

### 8.5.4 Constructors and destructor

```cpp
sc_vector();
explicit sc_vector( const char* );
sc_vector( const char* , size_type n,
            sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );
template< typename Creator >
sc_vector( const char* , size_type n,
           Creator creator, sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );
virtual ~sc_vector();
```

| 형태 | 규칙 |
|---|---|
| 모든 constructor | 문자열 인자(있으면)를 기반 클래스 `sc_object` constructor로 전달해 **모듈 계층 내 벡터 인스턴스의 string name을 설정해야 shall** |
| 기본 constructor | **`sc_gen_unique_name("vector")`**를 호출해 고유 string name을 생성하고 `sc_object` constructor로 전달해야 shall |
| 두 번째 인자 존재 | `init`을 호출하고 두 번째 인자 값을 `init`의 첫 인자로 전달해야 shall |
| 두 번째 인자 부재 | **빈 벡터**를 생성해야 shall. 이후 `init`을 명시적으로 호출해 초기화할 수 있다 |
| 두 번째·세 번째 인자 존재 | `init`을 호출하고 두·세 번째 인자를 `init`의 첫·둘째 인자로 전달해야 shall |
| destructor | **벡터의 모든 원소를 delete해야 shall** |

### Example

```cpp
SC_MODULE(my_module) {
  sc_core::sc_vector<sc_core::sc_port<i_f> > ports;
  sc_core::sc_vector<sc_core::sc_signal<bool> > signals;
  ...

    SC_CTOR(my_module)
    : ports ("ports", 4)                     // 원소 4개짜리 vector-of-ports
    , signals("signals") {                   // 초기화되지 않은 vector-of-signals
         signals.init(8);                    // 원소 8개(각각 signal)로 초기화
         ...
    }
};
```

### 8.5.5 `init` and `create_element`

```cpp
void init( size_type n, sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );
static T* create_element( const char* prefix, size_type index );
```

**`create_element`**

- 첫 인자의 string name을 갖는 **새 `T` 객체를 할당하고 그 포인터를 반환해야 shall**.
- **새로 만들어진 객체는 벡터와 동일한 parent를 가져야 shall.** 즉 **벡터의 원소들은 SystemC object hierarchy에서 벡터 객체 자신의 sibling이다.**
- 존재 이유: 애플리케이션이 대체 함수를 제공해 각 원소에 사용자 정의 constructor 인자를 넘길 수 있게 하기 위함.

**`init`**

- 첫 인자 값만큼의 `T` 객체를 할당하고 벡터를 그 객체들로 채워야 shall.
- 각 객체는 **`create_element` 호출로 할당되어야 shall**. 첫 인자는 원소의 string name, 둘째 인자는 **0부터 세는 벡터 내 원소 번호**.
- **각 원소의 string name은 `sc_gen_unique_name(this->basename())` 호출로 결정되어야 shall.**
- 둘째 인자(`init_pol`)가 **벡터가 언제 lock되는지**를 결정한다:

| `sc_vector_init_policy` | 의미 |
|---|---|
| `SC_VECTOR_LOCK_AFTER_INIT` (기본값) | **생성 시점에** 벡터를 lock |
| `SC_VECTOR_LOCK_AFTER_ELABORATION` | **elaboration 이후에** 벡터를 lock |

- `init`을 인자 값 **0**으로 호출하는 것은 **허용되며 아무 효과가 없어야 shall**.
- **`init`을 어떤 벡터에 대해 0보다 큰 인자 값으로 두 번 이상 호출하는 것은 error.** 크기를 지정하는 두 번째 인자를 가진 constructor는 `init`을 호출하므로, **크기를 지정해 벡터를 생성한 뒤 다시 `init`을 호출하는 것도 (양쪽 크기가 nonzero라면) error**.

**Creator 버전**

```cpp
template< typename Creator >
void init( size_type n, Creator creator,
           sc_vector_init_policy init_pol = SC_VECTOR_LOCK_AFTER_INIT );
```

- `create_element` 대신 **두 번째 인자(함수 또는 함수 객체)를 사용해** 각 원소를 할당해야 shall.
- 두 번째 인자 `c`는 다음이 **well-formed statement가 되도록 하는 값이어야 한다 must**:

  ```cpp
  T* placeholder1 = c( (const char*)placeholder2, (size_type)placeholder3 );
  ```

  즉 실제 인자는 `create_element` 자리에 **호출 가능(callable)해야 한다**. 이를 통해 creator가 각 원소에 string name 외의 추가 constructor 인자를 넘길 수 있다.
- 모든 벡터 `V`에 대해 **`V.init(N, sc_vector<T>::create_element)`와 `V.init(N)`은 동등해야 shall**.
- 세 번째 인자가 lock 시점을 결정한다 (위 표와 동일).

### Example

```cpp
struct my_module : sc_core::sc_module {
    my_module(sc_core::sc_module_name n, std::string extra_arg);
    ...
};

struct Top : sc_core::sc_module {
    sc_core::sc_vector<my_module> vector1; // Vector-of-modules
    sc_core::sc_vector<my_module> vector2;

    // Case 1: creator가 함수 객체
    struct my_module_creator {
        my_module_creator(std::string arg) : extra_arg(arg) {}

        my_module *operator()(const char *name, size_t) {
           return new my_module(name, extra_arg);
        }
        std::string extra_arg;
    };

    // Case 2: creator가 멤버 함수
    my_module *my_module_creator_func(const char *name, size_t i) {
        return new my_module(name, "value_of_extra_arg");
    }

    Top(sc_core::sc_module_name _name, int N) {
      // my_module로 constructor 인자를 전달하며 벡터 초기화
      // Case 1: 함수 객체를 만들어 전달
      vector1.init(N, my_module_creator("value_of_extra_arg"));

      // Case 2: Boost bind를 사용해 멤버 함수를 전달
      vector2.init(N,
                  sc_bind(&Top::my_module_creator_func, this, sc_unnamed::_1, sc_unnamed::_2));
    }
};
```

### 8.5.6 Incremental additions to `sc_vector` during elaboration phase

```cpp
template< typename... Args > void emplace_back( Args&&... args );
template< typename... Args > void emplace_back_with_name( Args &&... args );
```

| 함수 | 규칙 |
|---|---|
| `emplace_back` | **lock되지 않았으면** 벡터에 원소를 추가한다. 원소 constructor의 첫 인자로 **자동 생성된 이름**을 전달. `args...`는 constructor로 forward |
| `emplace_back_with_name` | **lock되지 않았으면** 벡터에 원소를 추가한다. 원소 constructor의 첫 인자로 **사용자 정의 이름**을 전달. `args...`는 constructor로 forward |

### 8.5.7 `kind`, `size`, `get_elements`

| 함수 | 규칙 |
|---|---|
| `virtual const char* kind() const;` | 문자열 **`"sc_vector"`**를 반환해야 shall |
| `size_type size() const;` | 벡터의 원소 수를 반환해야 shall. **초기화되지 않은 벡터의 size는 0**. 일단 nonzero 값으로 설정되면 **수정 불가** |
| `const std::vector<sc_object*>& get_elements() const;` | `sc_vector` 원소들의 포인터를 **원소당 하나씩, 동일한 순서로** 담은 `std::vector`에 대한 const 참조를 반환해야 shall. 그 `std::vector`는 `sc_vector`와 **같은 size여야 shall**. **참조는 `sc_vector` 객체의 lifetime 동안 유효해야 shall** |

### 8.5.8 `operator[]` and `at`

```cpp
T& operator[] ( size_type );
const T& operator[] ( size_type ) const;
T& at( size_type );
const T& at( size_type ) const;
```

- 인자로 주어진 index 위치에 저장된 객체에 대한 참조 또는 const 한정 참조를 반환해야 shall. **참조는 벡터의 lifetime 동안 유효해야 shall**.
- **인자 값이 벡터의 size보다 크면 `operator[]`의 동작은 undefined**. 반면 **`at`은 error를 검출하고 보고해야 shall**.
- 모든 벡터 `v`와 모든 index `i`, `j`에 대해 **`&V[i] + j == &V[i + j]` 관계의 값은 undefined**. (원소가 메모리상 연속이라고 가정하면 안 됨)

### 8.5.9 Iterators

```cpp
iterator begin();
iterator end();
const_iterator begin() const;
const_iterator end() const;
const_iterator cbegin() const;
const_iterator cend() const;
```

- `sc_vector`는 ISO/IEC 14882:2017 [lib.iterator.requirements]에 정의된 **Random Access Iterator 요구사항을 만족하는** iterator 인터페이스를 제공해야 shall.
- `begin`은 첫 원소를 가리키는 iterator를 반환해야 shall. `end`는 **마지막 원소 다음의 가상 원소**를 가리키는 iterator를 반환해야 shall. 벡터가 비어 있으면 `begin() == end()`.
- `iterator`는 `const_iterator`로 **암시적 변환 가능해야 shall**. 변환은 iterator가 가리키는 원소의 identity를 보존해야 shall.
- `sc_vector_assembly<T,MT>::iterator`는 **`sc_vector<T>::iterator`와 `sc_vector<T>::const_iterator` 양쪽으로** 암시적 변환 가능해야 shall (identity 보존).
- `sc_vector_assembly<T,MT>::const_iterator`는 `sc_vector<T>::const_iterator`로 암시적 변환 가능해야 shall (identity 보존).

### 8.5.10 `bind`

```cpp
template< typename ContainerType, typename ArgumentType >
iterator bind( sc_vector_assembly<ContainerType,ArgumentType> );

template< typename BindableContainer >
iterator bind( BindableContainer& );

template< typename BindableIterator >
iterator bind( BindableIterator , BindableIterator );

template< typename BindableIterator >
iterator bind( BindableIterator , BindableIterator , iterator );

template< typename ContainerType, typename ArgumentType >
iterator operator() ( sc_vector_assembly<ContainerType,ArgumentType> c );

template< typename ArgumentContainer >
iterator operator() ( ArgumentContainer& );

template< typename ArgumentIterator >
iterator operator() ( ArgumentIterator , ArgumentIterator );

template< typename ArgumentIterator >
iterator operator() ( ArgumentIterator , ArgumentIterator , iterator );
```

- 각 `bind`/`operator()`는 현재 벡터 `*this`의 원소를 인자로 결정되는 벡터의 원소에 **원소 단위(element-by-element)로 bind해야 shall**. 구현은 현재 벡터의 각 개별 원소의 `bind`/`operator()`를 호출해 bind해야 shall.

**부분 binding 규칙**

| 형태 | 시작 위치 (현재 벡터 측) | 대상 측 |
|---|---|---|
| 1-인자 | `*this`의 **첫 원소**부터 shall | 인자로 넘긴 컨테이너(vector 또는 vector assembly)의 **첫 원소**부터. 그 컨테이너의 모든 원소에 bind해도 된다 may |
| 2-인자 | `*this`의 **첫 원소**부터 shall | 첫 인자 iterator가 가리키는 원소부터 시작하고, **둘째 인자 iterator가 가리키는 원소와 그 이후는 bind하지 않아야 shall** |
| 3-인자 | **셋째 인자 iterator가 가리키는 원소**부터 shall. **셋째 인자가 현재 객체의 원소를 가리키지 않으면 동작은 undefined** | 2-인자와 동일 |

- 각 경우 `bind`/`operator()`는 binding 수행 후 **`*this` 내 첫 unbound 원소를 가리키는 iterator를 반환해야 shall**.
- 개별 원소의 binding policy만 허용한다면 **한 벡터를 여러 번 bind해도 된다**. 즉 `bind`를 여러 번 호출해 매번 벡터의 다른 원소들을 bind하는 것이 가능.

### Example

```cpp
typedef sc_core::sc_vector<sc_core::sc_inout<int> > port_type;
typedef sc_core::sc_vector<sc_core::sc_signal<int> > signal_type;

struct M : sc_core::sc_module {
    port_type ports;                                             // Vector-of-ports
    M(sc_core::sc_module_name _name, int N)
    : ports("ports", N) {
    ...
    }
};

struct Top : sc_core::sc_module {
    signal_type sigs;                                            // Vector-of-signals
    signal_type hi_sigs;
    signal_type lo_sigs;
    M *m1, *m2;

    Top(sc_core::sc_module_name _name)
    : sigs("sigs", 4), hi_sigs("hi_sigs", 2), lo_sigs("lo_sigs", 2) {
         m1 = new M("m1", 4);
         m2 = new M("m2", 4);

         port_type::iterator it;

         // ports 벡터의 4개 원소 전부를 sigs 벡터의 4개 원소에 bind
         it = m1->ports.bind(sigs);
         sc_assert((it - m1->ports.begin()) == 4);

         // ports 벡터의 앞 2개 원소를 hi_sigs의 2개 원소에 bind
         it = m2->ports.bind(hi_sigs.begin(), hi_sigs.end());
         sc_assert((it - m2->ports.begin()) == 2);

         // 위 벡터 bind와 동등한 명시적 루프
         // port_type::iterator from;
         // signal_type::iterator to;
         //
         // for ( from = m2->ports.begin(), to = hi_sigs.begin();
         // (from != m2->ports.end()) && (to != hi_sigs.end());
         // from++, to++ )
         // (*from).bind( *to );

         // ports 벡터의 뒤 2개 원소를 lo_sigs의 2개 원소에 bind
         it = m2->ports.bind(lo_sigs.begin(), lo_sigs.end(), it);
         sc_assert((it - m2->ports.begin()) == 4);
    }
    ...
};
```

### 8.5.11 `sc_assemble_vector`

```cpp
template< typename T, typename MT >
sc_vector_assembly<T,MT> sc_assemble_vector( sc_vector<T> & , MT (T::*member_ptr ) );
```

- `sc_vector_assembly` 객체를 반환해야 shall. 이 객체는 `sc_vector`의 **proxy** 역할을 하며 `begin`, `end`, `cbegin`, `cend`, `size`, `get_elements`, `operator[]`, `at`, `bind`, `operator()`를 제공해야 shall. 각 멤버 함수는 이 proxy가 표현하는 벡터에 대해 `sc_vector`의 대응 멤버 함수와 **동일한 동작을 해야 shall**.
- **첫 번째 인자는 `sc_vector<T>` 객체여야 shall**, 여기서 `T`는 **`sc_module`에서 파생된 타입**. 즉 첫 인자는 **vector-of-modules여야 shall**.
- **두 번째 인자는** 첫 인자 벡터의 원소 타입인 사용자 정의 모듈 클래스의 **member sub-object의 주소여야 shall**. 즉 첫 인자 모듈의 멤버여야 shall.
- proxy 객체가 표현하는 벡터는 vector-of-modules의 **모든 원소의 (둘째 인자로 지정된) member sub-object에 대한 참조들**로 구성된 원소를 담아야 shall.
- `sc_assemble_vector`는 **`sc_object`에서 파생된 임의 객체 타입의 벡터**에 대한 proxy 생성에 사용해도 된다 may.
- `sc_vector_assembly` 객체의 멤버 함수 호출은 proxy가 표현하는 벡터의 원소, 즉 vector-of-modules 전반에 분산된 member sub-object들에 작용해야 shall. 이 sub-object들은 각 멤버 함수의 동작에 관해 **단일 벡터의 멤버인 것처럼 보여야 shall**. 그 결과 다음 관계가 성립해야 shall:

```cpp
sc_vector_assembly<T, MT> assembly = sc_assemble_vector(vector, &module_type::member);

sc_assert( &*(assembly.begin()) == &(*vector.begin()).member );
sc_assert( &*(assembly.end()) == &(*vector.end()).member );
sc_assert( assembly.size()  == vector.size() );

for (unsigned int i = 0; i < assembly.size(); i++)
{
    sc_assert( &assembly[i] == &vector[i].member );
    sc_assert( &assembly.at(i) == &vector[i].member );
}
```

- `sc_vector_assembly::get_elements`는 `std::vector<sc_object*>`를 **값으로** 반환해야 shall. 각 원소는 vector assembly 내 member pointer를 **`sc_object*`로 static cast**해서 설정되어야 shall. (`sc_vector::get_elements`가 const **참조**를 반환하는 것과 대비)
- 애플리케이션은 **`sc_assemble_vector` 호출 외의 방법으로 `sc_vector_assembly` 객체를 생성하면 안 된다 shall not**.
- `sc_vector_assembly`는 **copyable해야 shall**.

### Example

```cpp
struct i_f: virtual sc_core::sc_interface { ... };

struct Init : sc_core::sc_module {
    sc_core::sc_port<i_f> port;
    ...
    Init(sc_core::sc_module_name _name)
    : port("port") { ... }
};

struct Targ : public sc_core::sc_module, private i_f {
    sc_core::sc_export<i_f> xp;
    ...
    Targ(sc_core::sc_module_name _name)
    : xp("xp") { ... }
};

struct Top : sc_core::sc_module {
    sc_core::sc_vector<Init> init_vec;
    sc_core::sc_vector<Targ> targ_vec;
    ...
    Top(sc_core::sc_module_name _name, int N)
    : init_vec("init_vec", N), targ_vec("targ_vec", N) {

       // vector-of-ports에서 vector-of-exports로 vector-to-vector bind
       sc_core::sc_assemble_vector(init_vec, &Init::port).bind(
           sc_core::sc_assemble_vector(targ_vec, &Targ::xp));
       ...
    }
    ...
};
```

---

## 8.6 Utility functions

### 8.6.1 Function declarations

```cpp
namespace sc_dt {

     template <class T>
     T sc_abs( const T& );

     template <class T>
     T sc_max( const T& a , const T& b ) { return (( a >= b ) ? a : b ); }

     template <class T>
     T sc_min( const T& a , const T& b ) { return (( a <= b ) ? a : b ); }
}

namespace sc_core {

     #define IEEE_1666_SYSTEMC 201101L

     #define SC_VERSION_MAJOR          implementation-defined_number
     #define SC_VERSION_MINOR          implementation-defined_number
     #define SC_VERSION_PATCH          implementation-defined_number
     #define SC_VERSION_ORIGINATOR     implementation-defined_string
     #define SC_VERSION_RELEASE_DATE   implementation-defined_date
     #define SC_VERSION_PRERELEASE     implementation-defined_string
     #define SC_IS_PRERELEASE          implementation-defined_bool
     #define SC_VERSION                implementation-defined_string
     #define SC_COPYRIGHT              implementation-defined_string

     extern const unsigned int   sc_version_major;
     extern const unsigned int   sc_version_minor;
     extern const unsigned int   sc_version_patch;
     extern const std::string    sc_version_originator;
     extern const std::string    sc_version_release_date;
     extern const std::string    sc_version_prerelease;
     extern const bool           sc_is_prerelease;
     extern const std::string    sc_version_string;
     extern const std::string    sc_copyright_string;

     const char* sc_copyright();
     const char* sc_version();
     const char* sc_release();
}
```

`sc_abs` / `sc_max` / `sc_min`은 **`sc_dt` 네임스페이스**, 버전·저작권 관련은 **`sc_core` 네임스페이스**임에 주의.

### 8.6.2 `sc_abs`

```cpp
template <class T>
T sc_abs( const T& );
```

- 인자의 **절댓값을 반환해야 shall**.
- **`bool T::operator>=( const T& )`와 `T T::operator-()`를 호출해 구현되어야 shall**. 따라서 템플릿 인자는 **임의의 SystemC numeric type 또는 임의의 fundamental C++ type**이 될 수 있다.

### 8.6.3 `sc_max`

```cpp
template <class T>
T sc_max( const T& a , const T& b ) { return (( a >= b ) ? a : b ); }
```

두 인자 중 **큰 값을 위 정의대로 반환해야 shall**.

> NOTE(informative) — 템플릿 인자는 `operator>=`가 정의된 타입이거나 그런 타입으로의 사용자 정의 변환이 정의된 타입이어야 한다(should). 예: 임의의 SystemC numeric type, 임의의 fundamental C++ type.

### 8.6.4 `sc_min`

```cpp
template <class T>
T sc_min( const T& a , const T& b ) { return (( a <= b ) ? a : b ); }
```

두 인자 중 **작은 값을 위 정의대로 반환해야 shall**.

> NOTE(informative) — 템플릿 인자는 `operator<=`가 정의된 타입이거나 그런 타입으로의 사용자 정의 변환이 정의된 타입이어야 한다(should).

### 8.6.5 Version and copyright

**`IEEE_1666_SYSTEMC`**

```cpp
#define IEEE_1666_SYSTEMC 201101L
```

구현은 매크로 `IEEE_1666_SYSTEMC`를 **정확히 위 값으로 정의해야 shall**. 이 표준의 향후 버전은 이 매크로 값을 수치적으로 더 큰 값으로 대체하려는 의도이다.

**버전 매크로 / 상수**

```cpp
#define SC_VERSION_MAJOR        implementation-defined_number     // 예: 2
#define SC_VERSION_MINOR        implementation-defined_number     // 3
#define SC_VERSION_PATCH        implementation-defined_number     // 4
#define SC_VERSION_ORIGINATOR   implementation-defined_string     // "OSCI"
#define SC_VERSION_RELEASE_DATE implementation-defined_date       // "20110411"
#define SC_VERSION_PRERELEASE   implementation-defined_string     // "beta"
#define SC_IS_PRERELEASE        implementation-defined_bool       // 1
#define SC_VERSION              implementation-defined_string     // "2.3.4_beta-OSCI"
#define SC_COPYRIGHT            implementation-defined_string
```

- 각 구현은 위의 매크로와 상수를 **정의해야 shall**.
- 각 구현은 추가로 애플리케이션이 어느 구현이 실행 중인지 판별할 수 있도록 **하나 이상의 구현 고유 매크로를 정의하는 것이 권장된다 should**.
- 이 clause의 매크로·상수 값은 **10.8.3의 TLM-2.0 대응 정의 값과 독립적일 수 있다 may**.

**형식 규칙**

| 항목 | 규칙 |
|---|---|
| `implementation-defined_number` | `[0–9]` 십진 숫자열, **따옴표로 감싸지 않아야 shall** |
| originator / prerelease 문자열 | `[A–Z][a–z][0–9]_` 문자열, **따옴표로 감싸야 shall** |
| release date | **ISO 8601 basic format** 달력 날짜 `YYYYMMDD`, 8자 모두 십진 숫자, 따옴표로 감싸야 shall |
| `SC_IS_PRERELEASE` | **0 또는 1**, 따옴표로 감싸지 않아야 shall |
| version string | `"major.minor.patch_prerelease-originator"` 또는 `"major.minor.patch-originator"`. prerelease 문자열의 유무는 **`SC_IS_PRERELEASE` 값에 따름 shall** |
| copyright string | 저작권 고지로 설정하는 것이 권장 should. 의도: 애플리케이션이 콘솔이나 로그 파일에 출력할 수 있는 법적 저작권 고지 |

각 상수는 **같은 이름의 매크로가 정의한 값을 적절한 데이터 타입으로 변환해 초기화되어야 shall**.

**함수**

| 함수 | 규칙 |
|---|---|
| `const char* sc_release();` | **`sc_version_string`의 값을 C 문자열로 변환해 반환해야 shall** |
| `const char* sc_version();` | **implementation-defined 문자열**을 반환해야 shall. 의도: SystemC 클래스 라이브러리 구현 버전 정보 |
| `const char* sc_copyright();` | **copyright string 값을 C 문자열로 변환해 반환해야 shall** |

---

## 흔한 위반 (pitfalls)

- **트레이스 파일을 연 뒤 시뮬레이션 도중에 `sc_trace`를 추가 호출** — LRM §8.1.1: 파일을 연 뒤 **하나 이상의 delta cycle이 경과했으면** 그 파일에 값을 추적하면 안 된다 shall not. 모든 `sc_trace` 호출을 파일을 연 직후(같은 delta cycle 내, 보통 elaboration 중)에 몰아서 하라.
- **`sc_close_vcd_trace_file`을 `sc_stop` 직후 등 최종 delta cycle 이전에 호출** — LRM §8.1.1: 트레이스 파일은 시뮬레이션의 최종 delta cycle 이전에 닫으면 안 된다 shall not. `sc_start` 반환 후 `sc_main` 끝에서 닫아라.
- **`set_time_unit`에 10의 거듭제곱이 아니거나 음수인 값을 전달** — LRM §8.1.3: double 인자는 양수이고 10의 거듭제곱이어야 shall.
- **`sc_trace_file` 객체를 직접 생성** — LRM §8.1.3: 애플리케이션은 `sc_trace_file` 객체를 생성하면 안 된다 shall not. 포인터·참조만 정의하고 `sc_create_vcd_trace_file`로 얻어라.
- **`SC_MAX_SEVERITY`를 severity 인자로 전달** — LRM §8.2.5: error. severity level은 `SC_INFO`/`SC_WARNING`/`SC_ERROR`/`SC_FATAL` 네 개뿐이다.
- **`sc_report` 객체를 직접 new/생성** — LRM §8.2.3: copy constructor 외의 방법으로 직접 생성하면 안 된다 shall not. `sc_report_handler::report`(또는 `SC_REPORT_*` 매크로)로 생성하라.
- **`get_cached_report()` 포인터를 나중에 다시 쓰려고 보관** — LRM §8.3.5·§8.3.14: 각 캐시는 **가장 최근 리포트 하나만** 저장하며 이전 리포트는 접근 불가능 shall. 또한 캐시는 process instance마다 분리되어 있으므로 다른 프로세스에서 조회하면 다른 캐시를 본다.
- **`SC_CACHE_REPORT` 없이 `report`가 만든 `sc_report`를 참조로 붙잡아 둠** — LRM §8.3.5: `SC_CACHE_REPORT`가 설정되지 않으면 그 객체는 `report` 호출 이후까지 존속하지 않아야 shall.
- **severity가 `SC_INFO`가 아닌 리포트에서 `get_verbosity()` 값을 신뢰** — LRM §8.2.7: `SC_INFO`가 아니면 반환값은 implementation-defined.
- **verbosity로 걸러진 INFO 리포트가 `get_count`에 잡힐 것으로 기대** — LRM §8.3.5: verbosity level 때문에 무시된 리포트는 **카운트를 증가시키지 않는다 shall**.
- **verbosity를 `SC_WARNING`/`SC_ERROR` 리포트 억제에 사용** — LRM §8.3.9: 최대 verbosity level은 `SC_INFO` 리포트에만 적용된다 shall. 다른 severity는 `set_actions`/`suppress`로 다뤄라.
- **`SC_REPORT_INFO`가 verbosity 필터를 통과할 것으로 가정** — LRM §8.3.5: verbosity 인자가 없는 `SC_INFO`는 verbosity가 **`SC_MEDIUM`(200)**인 것처럼 동작한다. 최대 verbosity level이 `SC_LOW`(100)면 걸러진다.
- **severity 단위 `set_actions`가 message type 설정을 이길 것으로 기대** — LRM §8.3.6: 우선순위는 severity < msg_type < (msg_type, severity) 순이며 **낮은 우선순위 매치의 액션은 억제된다 shall**.
- **여러 `stop_after` limit이 누적 적용될 것으로 기대** — LRM §8.3.7: 둘 이상 매치되면 **더 높은 우선순위의 limit만** 효력을 가진다 shall.
- **`stop_after` limit을 낮춰 즉시 정지시키려 함** — LRM §8.3.7: 호출 시점에 이미 limit에 도달·초과했다면 `sc_stop`은 즉시가 아니라 **해당 카운트가 다음에 증가할 때** 호출된다 shall.
- **`stop_after(..., 0)`이 정지를 완전히 막을 것으로 기대** — LRM §8.3.7: limit 0은 stop limit이 없다는 뜻이지만, 그 경우에 설정된 액션(또는 기본 액션)이 여전히 `sc_stop`·abort·예외를 일으킬 수 있다 may. `SC_FATAL`의 기본 동작에는 `SC_ABORT`이 포함된다.
- **`suppress(SC_ABORT)`로 강제된 액션까지 막으려 함** — LRM §8.3.10: 같은 액션이 suppress되고 동시에 force되면 **force가 우선한다 shall**.
- **`suppress`/`force`를 여러 번 호출해 누적시키려 함** — LRM §8.3.10: 인자가 이전 집합을 **완전히 대체하며**, 억제/강제 집합은 각각 **단일 global 집합 하나뿐**이다.
- **커스텀 핸들러를 설정한 뒤 원복 방법을 몰라 헤맴** — LRM §8.3.11 NOTE: `set_handler( &sc_report_handler::default_handler )`.
- **로그 파일 이름을 두 번째로 설정하려 함** — LRM §8.3.15: 이미 이름이 있으면 `set_log_file_name`은 기존 이름을 수정하지 않고 `false`를 반환한다 shall. 먼저 빈 문자열 포인터로 호출해 지워야 한다.
- **`get_new_action_id()`를 static 초기화 시점에 호출** — LRM §8.3.12: 애플리케이션은 **elaboration 시작 전에 호출하면 안 된다 shall not**.
- **`sc_vector<T>`를 `sc_object` 파생이 아닌 타입(예: `int`, `std::string`)으로 인스턴스화** — LRM §8.5.3: `T`는 `sc_object`에서 파생되어야 shall.
- **크기를 지정해 벡터를 생성한 뒤 다시 `init(n)` 호출** — LRM §8.5.5: constructor가 이미 `init`을 호출하므로 (양쪽 크기가 nonzero면) **error**. `init`을 0보다 큰 인자로 두 번 이상 호출하는 것 자체가 error.
- **벡터를 resize하려 함** — LRM §8.5.3: 크기는 **한 번만** 설정 가능하며 동적 resize는 불가.
- **vector-of-modules를 simulation 중에 생성** — LRM §8.5.3: 벡터 생성 시점 제약은 `T`의 제약을 따른다. vector-of-modules는 **elaboration 중에만** 생성 가능.
- **벡터 원소가 벡터의 자식일 것으로 기대하고 계층 이름을 계산** — LRM §8.5.5: 원소는 벡터와 **같은 parent**를 가지며 벡터 객체의 **sibling**이다. 원소 이름은 `sc_gen_unique_name(this->basename())`으로 결정된다.
- **`V[i]`에 범위 밖 index 전달** — LRM §8.5.8: `operator[]`의 동작은 **undefined**. 검사를 원하면 `at`을 써라(error를 검출·보고한다 shall).
- **`&V[0] + i`로 원소 주소를 계산** — LRM §8.5.8: `&V[i] + j == &V[i + j]`의 값은 **undefined**. 원소가 연속 메모리에 있다고 가정하지 마라.
- **3-인자 `bind`의 셋째 iterator로 다른 벡터의 iterator를 전달** — LRM §8.5.10: 현재 객체의 원소를 가리키지 않으면 동작은 **undefined**.
- **`sc_vector_assembly` 객체를 직접 생성** — LRM §8.5.11: `sc_assemble_vector` 호출 외의 방법으로 생성하면 안 된다 shall not.
- **`sc_assemble_vector`의 첫 인자로 모듈이 아닌 벡터를 전달** — LRM §8.5.11: 첫 인자는 `sc_module` 파생 타입의 `sc_vector<T>`, 즉 vector-of-modules여야 shall.
- **`sc_vector_assembly::get_elements()` 반환값을 참조로 받아 보관** — LRM §8.5.11: 이 함수는 `std::vector<sc_object*>`를 **값으로** 반환한다 shall (`sc_vector::get_elements`는 const 참조).
- **`sc_vector`를 복사하려 함** — LRM §8.5.2: copy constructor와 copy assignment는 **Disabled**(private).
- **`sc_abs`/`sc_max`/`sc_min`을 `sc_core`에서 찾음** — LRM §8.6.1: 이 세 함수는 **`sc_dt`** 네임스페이스에 있다.
- **`sc_version()` 문자열을 파싱해 버전을 추출** — LRM §8.6.5: `sc_version`의 반환값은 **implementation-defined**. 정형화된 버전 문자열이 필요하면 `sc_release()`(= `sc_version_string`) 또는 `SC_VERSION_MAJOR`/`MINOR`/`PATCH` 상수를 써라.
