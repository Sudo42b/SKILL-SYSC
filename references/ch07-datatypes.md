# Clause 7 — SystemC data types

LRM pp. 220–411. `sc_dt` 네임스페이스의 하드웨어 워드 길이 데이터 타입 — 정수·비트/로직 벡터·고정소수점. **RTL 수준 값 표현의 전부.**

각 절은 LRM 형식을 따른다: Description → Class definition → Constraints on usage → Constructors → Member functions.
`†` 표시 클래스 = **애플리케이션이 직접 인스턴스화하면 안 되는 proxy/base 클래스**.

---

## 7.1 Introduction

모든 native C++ 타입은 SystemC 애플리케이션에서 사용 가능. SystemC는 추가로 `sc_dt` 네임스페이스에 **애플리케이션별 워드 길이**를 표현하는 데이터 타입 클래스를 제공한다.

### 카테고리

| 카테고리 | 정의 | 정밀도 |
|---|---|---|
| Limited-precision integer | `sc_int_base` / `sc_uint_base` 파생 클래스 또는 인스턴스 | 하부 native C++ 표현 **및** 지정 워드 길이로 제한 shall |
| Finite-precision integer | `sc_signed` / `sc_unsigned` 파생 | 지정 워드 길이로만 제한 shall |
| Finite-precision fixed-point | `sc_fxnum` 파생 | 워드 길이 + integer word length + quantization mode + overflow mode |
| Limited-precision fixed-point | `sc_fxnum_fast` 파생 | 위 + 하부 native C++ 부동소수 표현 |
| Variable-precision fixed-point | `sc_fxval` | 시간에 따라 변할 수 있음. quantization/overflow 대상 아님 |
| Limited variable-precision fixed-point | `sc_fxval_fast` | 하부 C++ 부동소수 표현으로 제한 |
| Single-bit logic | `sc_logic` | 4치 논리: logic 0, logic 1, high-impedance, unknown — 기호 `'0'`, `'1'`, `'X'`, `'Z'` shall. 소문자 `'x'`/`'z'`도 문자 리터럴로 허용 |
| Bit vector | `sc_bv_base` 파생 | 각 비트가 logic 0 / logic 1 (`'0'`/`'1'`) |
| Logic vector | `sc_lv_base` 파생 | 각 비트가 logic 0/1/Z/X. 문자열 리터럴 내 소문자 `'x'`/`'z'` 허용 |

single-bit logic, variable-precision fixed-point, limited variable-precision fixed-point를 제외한 각 카테고리는 base class에 공통 동작을 두는 객체지향 계층으로 구성된다. 구현은 각 base class로부터 **워드 길이를 템플릿 인자로 지정하는 class template**을 파생해야 shall.

### 용어

- **fixed-point type** = finite-precision fixed-point 또는 limited-precision fixed-point. `sc_fxval`/`sc_fxval_fast`는 fixed-point 값을 저장하고 표현식에서 혼용될 수 있다는 제한된 의미에서만 fixed-point이며, quantization/overflow를 모델링하지 않고 **애플리케이션이 직접 쓸 의도가 아니다**.
- **numeric type** = limited-precision integer, finite-precision integer, finite-precision fixed-point, limited-precision fixed-point.
- **vector** = bit vector 또는 logic vector.
- numeric type/vector 객체의 **워드 길이는 초기화 시 설정되고 이후 변경되지 않아야 shall**.
- 각 비트는 인덱스를 가진다 shall. **우측 비트가 index 0** (numeric type에서는 LSB), **좌측 비트 index = 워드 길이 − 1**.

### Table 4 — SystemC data types

| Class template | Base class | Generic base class | Representation | Precision |
|---|---|---|---|---|
| `sc_int` | `sc_int_base` | `sc_value_base` | signed integer | limited |
| `sc_uint` | `sc_uint_base` | `sc_value_base` | unsigned integer | limited |
| `sc_bigint` | `sc_signed` | `sc_value_base` | signed integer | finite |
| `sc_biguint` | `sc_unsigned` | `sc_value_base` | unsigned integer | finite |
| `sc_fixed` | `sc_fix` | `sc_fxnum` | signed fixed-point | finite |
| `sc_ufixed` | `sc_ufix` | `sc_fxnum` | unsigned fixed-point | finite |
| `sc_fixed_fast` | `sc_fix_fast` | `sc_fxnum_fast` | signed fixed-point | limited |
| `sc_ufixed_fast` | `sc_ufix_fast` | `sc_fxnum_fast` | unsigned fixed-point | limited |
| — | — | `sc_fxval` | fixed-point | variable |
| — | — | `sc_fxval_fast` | fixed-point | limited-variable |
| — | `sc_logic` | — | single bit | — |
| `sc_bv` | `sc_bv_base` | — | bit vector | — |
| `sc_lv` | `sc_lv_base` | — | logic vector | — |

**권장(recommended)**: 애플리케이션은 untemplated base class (`sc_int_base` 등)가 아니라 **class template (`sc_int` 등)으로 객체를 생성**하라.

---

## 7.2 Common characteristics

### 7.2.1 Overview

7.2는 공통 연산자·함수에 대한 **구현의 의무**를 규정한다. 구현 방식에는 일부 유연성이 있다.

기본 원리: **native C++ 정수/부동소수 타입, C++ 문자열 타입, SystemC 데이터 타입은 표현식에서 혼용될 수 있다 may.**

- **Equality 연산자와 bitwise 연산자**는 모든 SystemC 데이터 타입에서 사용 가능.
- **Arithmetic 연산자와 relational 연산자**는 **numeric type에서만** 사용 가능.
- equality/bitwise/arithmetic/relational 연산자의 의미론은 **C++와 동일**.
- 구현이 제공하는 user-defined conversion이 SystemC 타입 → C++ native 타입 및 다른 SystemC 타입 변환을 지원한다.

### Proxy class 규칙 (핵심)

Bit-select, part-select, concatenation 연산자는 **proxy class의 인스턴스를 반환**한다. proxy class = 표현식 안에서 SystemC 데이터 타입 객체를 대표하고, 원 객체에 없는 추가 연산자/기능(rvalue/lvalue 구분 등)을 제공하는 클래스.

- proxy class 인스턴스는 **자신을 생성한 표현식 안에서만 사용될 의도**다.
- 애플리케이션은 proxy class 생성자를 호출해 **named object를 만들면 안 된다 should not**.
- proxy class에 대한 **포인터나 레퍼런스를 선언하면 안 된다 should not**.
- proxy class를 **함수의 반환 타입으로 쓰는 것을 피할 것이 강력히 권장(strongly recommended)** — proxy가 참조하는 객체의 수명이 함수 return 문 너머까지 이어지지 않을 수 있다.

> NOTE 1 — single-bit logic type에 대한 bitwise shift left/right 연산은 의미가 없고 **undefined**.
>
> NOTE 2 — 여기서 user-defined conversion은 C++ 표준과 같은 의미(생성자·변환 함수를 통한 암시적/명시적 변환).
>
> NOTE 3 — 암시적 변환을 쓰는 표현식에서 signed/unsigned numeric type을 혼용할 때 주의. 변환된 값의 부호(polarity)가 바뀌어도 **구현이 경고를 낼 의무가 없다**.

### 7.2.2 Initialization and assignment operators

| 대상 | 규칙 |
|---|---|
| 모든 integer class template (limited + finite) | **임의의 SystemC 데이터 타입 객체로 초기화**하는 오버로드 생성자를 제공해야 shall |
| 모든 vector class template (bit + logic) | **임의의 SystemC integer 또는 vector 객체로 초기화**하는 오버로드 생성자를 제공해야 shall |
| 모든 finite/limited-precision fixed-point class template | **임의의 SystemC integer 데이터 타입 객체로 초기화**하는 오버로드 생성자를 제공해야 shall |
| 모든 SystemC 데이터 타입 클래스 | **같은 값·같은 워드 길이**의 사본을 만드는 copy constructor를 정의해야 shall |

- 오버로드된 대입 연산자와 생성자는 타입 간 직접/간접 변환을 수행해야 shall. data type base class는 SystemC 데이터 타입의 **부분집합**만 직접 초기화 허용하는 제한된 생성자/대입 연산자 집합을 정의해도 된다 may.
- 일반 원칙: **class template 생성자는 암시적 호출 가능 may** (워드 길이가 템플릿 인자로 지정되므로). 반면 **base class의 단일 인자 이종 타입 생성자는 explicit 호출만 해야 should** (필요한 워드 길이가 지정되지 않으므로).

#### 잘림 / 확장 규칙 (전 타입 공통)

- 대입 대상의 워드 길이가 **부족**하면 값의 **좌측 비트가 잘린다 shall (truncate)**. 잘림 발생 시 구현이 warning을 낼 수 있으나 **의무는 아니다**. 애플리케이션은 어떤 경우든 그 warning을 비활성화할 수 있다(3.3.6 참조).
- 데이터 타입 객체나 문자열 리터럴을 **더 긴** 워드 길이 대상에 대입하면 좌측에 비트를 추가해 확장해야 shall.
  - **signed numeric type의 확장** → 부호와 크기를 모두 보존 = **sign extension**.
  - **그 외 모든 타입** → logic 0 비트 삽입 = **zero extension**.
- **fixed-point → integer 대입**: 정수부만 사용하고 **소수부는 버린다 shall**.
- **워드 길이 > 1인 값을 single-bit logic type에 대입하면 error shall.**

> NOTE — 정수 리터럴은 마이너스 기호가 앞에 붙지 않는 한 **항상 unsigned로 취급**된다. unsigned 정수 리터럴은 더 큰 워드 길이 객체에 대입될 때 **객체 자신의 signed/unsigned 여부와 무관하게 leading zero로 확장**된다.

### 7.2.3 Precision of arithmetic expressions

| 표현식 구성 | 반환 타입 |
|---|---|
| limited-precision integer만, 또는 limited-precision integer + native C++ 정수 | **implementation-defined C++ 정수 타입, 최대 워드 길이 64비트** shall. 반환값에 요구되는 정밀도가 64비트를 넘으면 구현의 동작은 **undefined**이고 값은 **implementation-dependent** |
| finite-precision integer만, 또는 finite-precision integer + limited-precision/native C++ 정수의 임의 조합 | **정확도 손실 없이 값을 담기에 충분한 워드 길이의 finite-precision integer** shall |
| fixed-point type을 포함하는 임의 표현식 | **variable-precision 또는 limited variable-precision fixed-point type** shall (7.10.5) |

구현이 피연산자 타입과 정확히 일치하는 시그니처의 오버로드 연산자를 제공하지 않는 경우, 여러 타입을 조합하는 표현식에서 **명시적 타입 캐스트를 사용해야 should**.

```cpp
int i = 10;
sc_dt::int64 i64 = 100;                                 // long long int
sc_dt::sc_int<16> sci = 2;
sc_dt::sc_bigint<16> bi = 20;
float f = 2.5;
sc_dt::sc_fixed<16, 8> scf = 2.5;
(i * sci);                                              // Ambiguous
(i * static_cast<sc_dt::int_type>(sci));                // Implementation-defined C++ integer
(i * bi);                                               // 48-bit finite-precision integer (int = 32비트 가정)
(i64 * bi);                                             // 80-bit finite-precision integer
(f * bi);                                               // Ambiguous
(static_cast<int>(f) * bi);                             // 48-bit finite-precision integer (int = 32비트 가정)
(scf * sci);                                            // Variable-precision fixed-point type
```

### 7.2.4 Base class default word length

```cpp
namespace sc_dt {
  enum { SC_NOW, SC_LATER };
}
```

- data type **base class**의 default 생성자가 (암시적/명시적으로) 호출되는 곳에서는 **default word length**가 사용되어야 shall.
- default word length는 **생성 시점에 context에 있는 length parameter**로 설정되어야 shall.
- length parameter는 **length context 객체를 생성**함으로써 context에 들어온다 may.
- length context는 **local scope**를 가지며 기본적으로 **즉시 활성화**되어야 shall. 활성화되면 scope 안에 있는 동안, 또는 다른 length context가 활성화될 때까지 유효하다 shall.
- 두 번째 생성자 인자가 `SC_LATER`이면 활성화가 **지연**되어야 shall (기본값은 `SC_NOW`). 지연된 context는 멤버 함수 `begin` 호출로 활성화할 수 있다.

#### Length context stack

- length context는 **전역 length context stack**으로 관리되어야 shall. 활성화 시 stack top에 놓인다 shall.
- 멤버 함수 `end` 호출로 비활성화하고 stack top에서 제거할 수 있다 may. **`end`는 현재 stack top인 length context에 대해서만 호출해야 shall.**
- scope를 벗어나면 암시적으로 비활성화되고 stack에서 제거된다.
- `begin`으로 활성화한 deferred length context는 `end`를 명시적으로 호출해 비활성화·제거해야 should.
- **현재 context는 항상 stack top의 length context여야 shall.**
- **length context는 한 번만 활성화되어야 shall. 활성화된 length context는 한 번만 비활성화되어야 shall.**

클래스: SystemC integer와 vector용 length parameter/context = **`sc_length_param`** / **`sc_length_context`**.

fixed-point 타입은 워드 길이 외에 default **integer word length**와 **mode** 속성을 가져야 shall. 이들은 생성 시점 context의 fixed-point type parameter로 설정되어야 shall. fixed-point type context의 사용 규칙은 length context와 동일해야 shall. length context stack과 같은 특성의 **fixed-point type context stack이 존재해야 shall**. 클래스: **`sc_fxtype_params`** / **`sc_fxtype_context`**.

```cpp
sc_dt::sc_length_param length10(10);
sc_dt::sc_length_context cntxt10(length10);                   // length10 now in context
sc_dt::sc_int_base int_array[2];                              // 10비트 정수 배열
sc_core::sc_signal<sc_dt::sc_int_base> S1;                    // 10비트 정수 시그널
{
   sc_dt::sc_length_param length12(12);
   sc_dt::sc_length_context cntxt12(length12, sc_dt::SC_LATER);       // cntxt12 deferred
   sc_dt::sc_length_param length14(14);
   sc_dt::sc_length_context cntxt14(length14, sc_dt::SC_LATER);       // cntxt14 deferred
   sc_dt::sc_uint_base var1;                                  // length 10
   cntxt12.begin();                                           // length12를 context로
   sc_dt::sc_uint_base var2;                                  // length 12
   cntxt14.begin();                                           // length14를 context로
   sc_dt::sc_uint_base var3;                                  // length 14
   cntxt14.end();                                             // cntx14 종료, cntx12 복원
   sc_dt::sc_bv_base var4;                                    // length 12
}                                                             // cntxt12 scope 밖, cntx10 복원
sc_dt::sc_bv_base var5; // length 10
```

> NOTE 1 — context stack은 default context를 지역적으로 대체했다가 복원할 수 있게 한다.
>
> NOTE 2 — 활성화된 context는 context 객체의 수명 동안, 또는 명시적으로 비활성화될 때까지 활성 상태다. 따라서 context는 **활성화된 함수 밖에서 생성되는 데이터 타입 객체의 default parameter에도 영향**을 줄 수 있다. 실행 순서가 비결정적인 함수 안에서 생성·활성화된 context가 애플리케이션의 다른 부분에 시간적 순서 의존성을 만들지 않도록 보장해야 should. 이 조건을 못 지키면 동작이 **implementation-dependent**가 될 수 있다.

### 7.2.5 Word length

SystemC integer, vector, part-select, concatenation의 **워드 길이(비트 수를 나타내는 양의 정수)**는 멤버 함수 **`length`**가 반환해야 shall.

### 7.2.6 Bit-select

Bit-select는 연관된 SystemC numeric type/vector 객체 내 지정 위치의 비트를 참조하는 **proxy class 인스턴스**.

- C++ subscript 연산자 **`operator[]`**는 단일 non-negative 정수 인자(비트 위치)로 호출될 때 bit-select를 생성하도록 오버로드되어야 shall.
- **지정한 비트 위치가 numeric type/vector 객체의 범위 밖이면 error shall.**
- user-defined conversion으로 bit-select를 `bool` 피연산자가 기대되는 표현식에 쓸 수 있어야 shall.
- **lvalue의 bit-select는 rvalue 또는 lvalue로 사용 가능 may. rvalue의 bit-select는 rvalue로만 사용해야 shall.**
- lvalue bit-select에는 bit-select 또는 `bool` 값을 대입할 수 있다 may. 대입은 lvalue가 대표하는 연관 객체 내 선택된 비트의 상태를 수정해야 shall.
- **애플리케이션은 rvalue bit-select에 값을 대입하면 안 된다 shall not.**
- integer, bit vector, logic vector 타입의 bit-select는 선택된 비트 상태를 반환하는 **explicit `to_bool` 변환 함수**를 가져야 shall.

```cpp
sc_dt::sc_int<4> I1;                          // 4비트 signed 정수
I1[1] = true;                                 // 선택 비트를 lvalue로 사용
bool b0 = I1[0].to_bool();                    // 선택 비트를 rvalue로 사용
```

> NOTE 1 — 특정 타입의 lvalue용/rvalue용 bit-select는 **서로 다른 두 클래스**의 객체다.
>
> NOTE 2 — bit-select 클래스는 선택 비트 값의 `bool` 암시적·명시적 변환을 모두 담을 수 있다.

### 7.2.7 Part-select

Part-select는 연관된 numeric type/vector 객체 내 **연속된 비트 부분집합**에 접근하는 proxy class 인스턴스.

- numeric type, bit vector, logic vector의 멤버 함수 **`range(int, int)`**가 part-select를 생성해야 shall. 두 non-negative 정수 인자는 **좌측·우측 인덱스 위치**.
- part-select는 좌측 인덱스에서 시작해 우측 인덱스를 **포함**하는 워드에 대한 참조를 제공해야 shall.
- **좌측 또는 우측 인덱스 위치가 객체 범위 밖이면 error shall.**
- **좌측 인덱스가 우측 인덱스보다 작으면 error shall.**
- C++ 함수 호출 연산자 **`operator()`**도 part-select를 생성하도록 오버로드되어야 shall. `range`의 직접 대체로 사용 가능 may.
- user-defined conversion으로 part-select를 연관 numeric/vector 타입 객체가 기대되는 표현식에 쓸 수 있어야 shall (제약은 7.5.7.3, 7.6.8.3, 7.9.8.3).
- **lvalue의 part-select는 rvalue/lvalue로 사용 가능 may. rvalue의 part-select는 rvalue로만 사용해야 shall.**

#### 직접 대입 가능 대상

| part-select 종류 | 직접 대입 가능 대상 |
|---|---|
| Integer part-select | **bit-select를 제외한** 임의의 SystemC 데이터 타입 객체 may |
| Fixed-point part-select | 임의의 SystemC integer 또는 vector, 임의의 part-select, 임의의 concatenation may |
| Vector part-select | **vector, vector part-select, vector concatenation에만** 직접 대입 가능 may (다른 타입 대입은 ambiguous이거나 명시적 변환 필요) |

- part-select 내 비트는 **연관 객체의 부호를 반영하지 않으며**, 수치로 변환될 때 **unsigned 이진수로 취급되어야 shall**.
- part-select를 더 긴 워드 길이 대상에 대입할 때는 **연관 객체의 타입과 무관하게 zero extension** 되어야 shall.

```cpp
sc_dt::sc_int<8> I2 = 2;                                // "0b00000010"
I2.range(3,2) = I2.range(1,0);                          // "0b00001010"
sc_dt::sc_int<8> I3 = I2.range(3,0);                    // "0b00001010"
                                                        // 8비트로 zero-extend
sc_dt::sc_bv<8> b1 = "0b11110000";
```

> NOTE 1 — part-select로 limited-precision integer 타입의 비트 순서를 뒤집을 수 없다.
>
> NOTE 2 — 특정 타입의 lvalue/rvalue part-select는 서로 다른 두 클래스의 객체다.
>
> NOTE 3 — part-select가 객체 레퍼런스 피연산자를 대체할 수 있어야 할 필요는 없다. 구현이 그런 대체 메커니즘을 제공하더라도(예: 적절한 오버로드 멤버 함수 정의) 모든 데이터 타입에 대해 제공할 필요는 없다.

### 7.2.8 Concatenation

Concatenation은 여러 객체의 비트를 하나의 집합 객체처럼 참조하는 proxy class 인스턴스.

- **`concat(arg0, arg1)`** 함수가 concatenation을 생성해야 shall. 인자는 두 개의 SystemC integer, vector, bit-select, part-select, concatenation 객체일 수 있다 may.
- C++ **comma 연산자 `operator,`**도 concatenation을 생성하도록 오버로드되어야 shall. `concat`의 직접 대체로 사용 가능 may.

#### Concatenation base type

- concatenation 인자의 타입은 **concatenation base type이거나 그로부터 파생된 타입이어야 shall**.
- 구현은 **모든 SystemC integer에 대한 공통 concatenation base type**과 **모든 vector에 대한 공통 concatenation base type**을 제공해야 shall.
- bit-select/part-select 인자의 concatenation base type = 연관된 integer/vector 객체의 것과 동일.
- 두 인자는 **같은 concatenation base type을 가진 임의의 조합**일 수 있다 may.
- concatenation 객체는 자신을 만든 함수에 전달된 인자와 **같은 concatenation base type을 가져야 shall**.

주어진 concatenation base type에 대해 허용되는 인자 집합:

- a) base class 또는 concatenation base type이 해당 base type과 일치하는 객체
- b) a)의 bit-select
- c) a)의 part-select
- d) a)/b)/c)의 임의 조합의 concatenation

#### lvalue/rvalue 및 대입

- **두 인자가 모두 lvalue면 concatenation은 lvalue여야 shall.** 인자 중 하나라도 rvalue면 concatenation은 rvalue여야 shall.
- 다른 인자가 SystemC integer/vector/bit-select/part-select/concatenation 객체일 때 **한쪽 인자만 `bool` 값일 수 있다 may**. 결과 concatenation은 **rvalue여야 shall**.
- 표현식 반환값의 base type이 lvalue concatenation의 base type과 같으면 그 표현식을 lvalue concatenation에 대입할 수 있다 may.
- **signed base type**의 concatenation에 대입되는 값의 워드 길이가 더 짧으면 **sign-extend** 되어야 shall. **그 외 모든 numeric type과 vector의 concatenation 대입은 zero-extend** 되어야 shall (필요 시).
- concatenation에 대입하면 그 인자로 지정된 객체들의 값이 갱신되어야 shall.
- concatenation은 base class가 concatenation base type과 같은 객체에 대입될 수 있다 may.
  - 대상 워드 길이가 **더 길면 zero-extend**.
  - 대상 워드 길이가 **더 짧으면 좌측 비트가 잘려야 shall**. 잘림 시 구현이 warning을 낼 수 있으나 의무는 아니며, 애플리케이션은 이를 비활성화할 수 있다(3.3.6).

```cpp
// well-formed
sc_dt::sc_uint<8> U1 = 2;                                 // "0b00000010"
sc_dt::sc_uint<2> U2 = 1;                                 // "0b01"
sc_dt::sc_uint<8> U3 = (true, U1.range(3, 0), U2, U2[0]); // U3 = "0b10010011"
                                                          // base class가 concatenation base type과 동일
(U2[0], U1[0], U1.range(7, 1)) = (U1[7], U1);             // U1[7]을 U2[0]에 복사, U1은 좌회전
concat(U2[0], concat(U1[0], U1.range(7, 1))) = concat(U1[7], U1);
                                                          // 위와 동일, concat 사용

// ill-formed
sc_dt::sc_bv<8> Bv1;
(Bv1, U1) = "0xffff";                          // Bv1과 U1은 공통 base type이 아님

bool C1 = true; bool C2 = false;
U2 = (C1, C1);                                 // bool 2개는 concatenate 불가
(C1, I1) = "0x1ff";                            // bool 인자는 rvalue concatenation을 만듦
```

> NOTE 1 — C++ comma 연산자는 우선순위가 낮으므로 concatenation 인자를 **괄호로 감싸야** 한다.
>
> NOTE 2 — 구현이 concatenation의 bit-select/part-select를 지원할 필요는 없다.
>
> NOTE 3 — 특정 타입의 lvalue/rvalue concatenation은 서로 다른 두 클래스의 객체다.

### 7.2.9 Reduction operators

reduction 연산자는 SystemC integer 또는 vector에 대해 일련의 bitwise 연산을 수행해 **`bool` 결과**를 만들어야 shall.

1. 첫 단계: 객체의 **첫 번째와 두 번째 비트**에 boolean 연산 적용.
2. 이전 결과와 **다음 비트**로 boolean 연산 재적용.
3. 모든 비트를 처리할 때까지 반복. 최종 boolean 연산 결과를 반환해야 shall.

제공되어야 하는 연산자 shall:

| 함수 | 연산 |
|---|---|
| `and_reduce` | 모든 비트 간 bitwise AND |
| `nand_reduce` | 모든 비트 간 bitwise NAND |
| `or_reduce` | 모든 비트 간 bitwise OR |
| `nor_reduce` | 모든 비트 간 bitwise NOR |
| `xor_reduce` | 모든 비트 간 bitwise XOR |
| `xnor_reduce` | 모든 비트 간 bitwise XNOR |

### 7.2.10 Integer conversion

- 모든 SystemC 데이터 타입은 **C++ 정수 값을 받는 대입 연산자를 제공해야 shall**. signed 값은 대상 길이에 맞게 **sign-extend** 되어야 shall.
- C++ 정수 타입으로의 **명시적 변환** 멤버 함수:

| 함수 | 변환 대상 |
|---|---|
| `to_int` | native C++ `int` |
| `to_uint` | native C++ `unsigned` |
| `to_long` | native C++ `long` |
| `to_ulong` | native C++ `unsigned long` |
| `to_uint64()` | 워드 길이 정확히 64비트인 native C++ unsigned 정수 타입 |
| `to_int64()` | 워드 길이 정확히 64비트인 native C++ 정수 타입 |

- 이 함수들은 SystemC integer, fixed-point type, vector 및 그 part-select/concatenation의 비트를 **unsigned 이진값으로 해석해야 shall**. 예외: **signed integer와 signed fixed-point type**.
- C++ 정수로 표현하기 위해 필요한 경우 **잘림(truncation)이 수행되어야 shall**.
- **`'X'` 또는 `'Z'` 값을 포함하는 logic vector를 정수로 변환 시도하면 error shall.**

### 7.2.11 String input and output

```cpp
void scan( std::istream& is = std::cin );
void print( std::ostream& os = std::cout ) const;
```

- 모든 SystemC 데이터 타입은 지정된 C++ 입력 스트림에서 문자열을 읽어 객체 값을 설정하는 **`scan`**을 제공해야 shall. 문자열 내용은 **7.3이 허용하는 임의의 표현**을 쓸 수 있다.
- 모든 SystemC 데이터 타입은 값을 C++ 출력 스트림에 쓰는 **`print`**를 제공해야 shall.
- **SystemC numeric type은 signed 또는 unsigned 10진 값으로 출력되어야 shall. SystemC vector type은 비트 값 문자열로 출력되어야 shall.**
- 모든 SystemC 데이터 타입은 **`operator<<`** (output stream inserter)를 지원해야 shall. 형식은 `print`와 동일해야 shall.
- C++ ostream manipulator **`dec`, `oct`, `hex`**는 limited-precision/finite-precision integer 및 vector 타입에 대해 표준 C++ 정수와 동일한 효과를 가져야 shall — 각각 10/8/16진 출력. 형식은 7.3에 기술된 것이되, **예외: `dec` manipulator가 활성일 때 vector는 비트 패턴 문자열로 출력되어야 shall**.
- 모든 SystemC 데이터 타입은 **`operator>>`** (input stream inserter)를 지원해야 shall. 허용 형식은 `scan`과 동일해야 shall.

```cpp
void dump ( std::ostream& os = std::cout ) const;
```

- **모든 fixed-point 타입은 추가로 `dump`를 제공해야 shall** — 최소한 타입 이름과 값을 인자로 전달된 스트림에 출력해야 shall. 목적은 구현이 진단 정보를 덤프해 사용자 디버깅을 돕는 것.

### 7.2.12 Conversion of application-defined types in integer expressions

- generic base proxy class template **`sc_generic_base`**는 구현이 제공해야 shall. 애플리케이션 정의 클래스의 base class로 사용될 수 있다 may.
- 모든 SystemC **integer, integer part-select, integer concatenation** 클래스는 `sc_generic_base` 파생 객체를 받는 **대입 연산자를 제공해야 shall**.
- 모든 SystemC **integer 클래스**는 추가로 generic base proxy 객체의 const 레퍼런스를 단일 인자로 받는 **오버로드 생성자를 제공해야 shall**.

> NOTE — generic base proxy class는 이 표준에서 말하는 "SystemC data types"에 **포함되지 않는다**.

---

## 7.3 String literals

문자열 리터럴 표현은 SystemC numeric type/vector 객체의 값으로 사용될 수 있다 may. **표준 prefix + 하나 이상의 digit로 표현된 magnitude**로 구성되어야 shall.

| 타입 | magnitude 표현의 기반 |
|---|---|
| SystemC integer type | C++ **정수 리터럴** shall |
| SystemC vector type | C++ **unsigned 정수 리터럴** shall |
| SystemC fixed-point type | C++ **floating literal** (단, optional floating suffix 없음) shall |

**prefix나 magnitude의 알파벳 문자는 대소문자 무관 may. 문자열 리터럴 표현은 case sensitive하지 않아야 shall.**

### Table 5 — String literal representation (`sc_numrep`)

| `sc_numrep` | Prefix (대소문자 무관) | Magnitude format |
|---|---|---|
| `SC_NOBASE` | implementation-defined | implementation-defined |
| `SC_DEC` | `0d` | decimal |
| `SC_BIN` | `0b` | binary |
| `SC_BIN_US` | `0bus` | binary unsigned |
| `SC_BIN_SM` | `0bsm` | binary sign & magnitude |
| `SC_OCT` | `0o` | octal |
| `SC_OCT_US` | `0ous` | octal unsigned |
| `SC_OCT_SM` | `0osm` | octal sign & magnitude |
| `SC_HEX` | `0x` | hexadecimal |
| `SC_HEX_US` | `0xus` | hexadecimal unsigned |
| `SC_HEX_SM` | `0xsm` | hexadecimal sign & magnitude |
| `SC_CSD` | `0csd` | canonical signed digit |

### 규칙

- 구현은 Table 5의 prefix를 가진 문자열로 **임의의 SystemC numeric type/vector 값을 설정하는 오버로드 생성자와 대입 연산자를 제공해야 shall**.
- **decimal 및 "sign & magnitude" 형식**에서는 극성을 나타내기 위해 prefix **앞에** `'+'` 또는 `'-'`를 선택적으로 둘 수 있다 may.
- prefix 뒤에는 unsigned 정수 값이 와야 shall. **예외: binary/octal/hexadecimal 형식** — 이 경우 prefix 뒤에는 **2의 보수 값**이 binary/octal/hexadecimal 정수로 표현되어야 shall.
- 구현은 더 긴 워드 길이 객체의 값을 설정하는 **정수 문자열 리터럴을 sign-extend 해야 shall**.
- **canonical signed digit 표현은 비트 값 −1을 나타내기 위해 문자 `'-'`를 사용해야 shall.**
- **bit-pattern string** (prefix 없이 비트/로직 문자 값만 포함)은 vector에 대입될 수 있다 may. 문자 수가 vector 워드 길이보다 적으면 **좌측을 zero extend** 해야 shall. **그런 문자열을 numeric type에 대입한 결과는 undefined.**

### to_string

```cpp
std::string to_string( sc_numrep numrep , bool with_prefix );
```

- SystemC numeric type, vector, part-select, concatenation의 인스턴스는 `to_string` 호출로 C++ `std::string`으로 변환될 수 있다 may.
- `numrep` 인자는 Table 5의 `sc_numrep` 값 중 하나여야 shall.
- **unsigned integer 또는 vector로 만든 문자열의 magnitude 표현은 단일 0으로 prefix되어야 shall** — `numrep`이 `SC_DEC`인 경우는 제외.
- `with_prefix`가 `true`면 `numrep`에 해당하는 prefix가 결과 문자열 좌측에 붙어야 shall. **`with_prefix`의 기본값은 `true`여야 shall.**
- **logic-vector 객체의 원소 중 값이 `'X'` 또는 `'Z'`인 것이 있으면 `to_string` 호출은 error shall.**

single-bit logic type 인스턴스의 값은 멤버 함수 **`to_char`** 호출로 단일 문자로 변환될 수 있다 may.

```cpp
sc_dt::sc_int<4> I1;                                 // 4비트 signed 정수
I1 = "0b10100";                                      // 5비트 signed 이진 리터럴 → 4비트로 잘림
std::string S1 = I1.to_string(sc_dt::SC_BIN, true);  // S1 == "0b0100"
sc_dt::sc_int<10> I2;                                // 10비트 정수
I2 = "0d478";                                        // "0b0111011110"과 동일
std::string S2 = I2.to_string(sc_dt::SC_CSD, false); // S2 == "1000-000-0"
sc_dt::sc_uint<8> I3;                                // 8비트 unsigned 정수
I3 = "0x7";                                          // 8비트 "0x07"로 zero-extend
std::string S3 = I3.to_string(sc_dt::SC_HEX);        // S3 == "0x007"
sc_dt::sc_lv<16> lv;                                 // 16비트 logic vector
lv = "0xff";                                         // 16비트 "0xffff"로 sign-extend
std::string S4 = lv.to_string(sc_dt::SC_HEX);        // S4 == "0x0ffff"
sc_dt::sc_bv<8> bv;                                  // 8비트 bit vector
bv = "11110000";                                     // bit-pattern string
std::string S5 = bv.to_string(sc_dt::SC_BIN);        // S5 == "0b011110000"
```

> NOTE — SystemC 데이터 타입은 인자 개수가 다른 추가 오버로드 `to_string` 함수를 제공할 수 있다.

---

## 7.4 `sc_value_base`†

### 7.4.1 Description

`sc_value_base`†는 **모든 SystemC limited-precision integer와 finite-precision integer의 공통 base class**를 제공한다. 구현이 concatenation 연산을 수행하기 위해 호출할 수 있는 virtual 멤버 함수 집합을 제공한다.

### 7.4.2 Class definition

```cpp
namespace sc_dt {

class sc_value_base†
{
    friend class sc_concatref†;
    private:
         virtual void concat_clear_data( bool to_ones=false );
         virtual bool concat_get_ctrl( implementation-defined* dst_p , int low_i ) const;
         virtual bool concat_get_data( implementation-defined* dst_p , int low_i ) const;
         virtual uint64 concat_get_uint64() const;
         virtual int concat_length( bool* xz_present_p=0 ) const;
         virtual void concat_set( int64 src , int low_i );
         virtual void concat_set( const sc_signed& src , int low_i );
         virtual void concat_set( const sc_unsigned& src , int low_i );
         virtual void concat_set( uint64 src , int low_i );
};

}         // namespace sc_dt
```

### 7.4.3 Constraints on usage

- 애플리케이션은 `sc_value_base`† 타입 객체를 **생성하면 안 되고 should not**, 파생 클래스가 `sc_value_base`† 부모로부터 상속한 멤버 함수를 **직접 호출하면 안 된다 should not**.
- `sc_generic_base` 파생 애플리케이션 정의 클래스가 `sc_value_base`†로부터도 파생된 경우, 그 객체는 integer concatenation의 인자로 사용될 수 있다 may. 그런 클래스는 해당 타입에 허용되는 concatenation 연산을 제공하기 위해 **`sc_value_base`†의 virtual 멤버 함수를 private 멤버로 override 해야 shall**.
- **파생 클래스에서 override되지 않은 `sc_value_base`† 멤버 함수를 그 파생 클래스 객체에 대해 호출하면 error shall.**

### 7.4.4 Member functions

| 함수 | 규칙 |
|---|---|
| `virtual void concat_clear_data( bool to_ones=false )` | `sc_value_base`† 객체의 **모든 비트**를 인자가 제공하는 상태로 설정해야 shall |
| `virtual bool concat_get_ctrl( impl-def* dst_p, int low_i ) const` | 제어 데이터를 첫 인자의 packed-array에, 두 번째 인자가 주는 비트 위치부터 복사해야 shall. **반환값은 항상 `false`여야 shall**. 첫 인자 타입은 unsigned integral type에 대한 포인터여야 shall |
| `virtual bool concat_get_data( impl-def* dst_p, int low_i ) const` | 데이터를 첫 인자의 packed-array에, 두 번째 인자 비트 위치부터 복사해야 shall. **데이터가 non-zero면 `true`, 아니면 `false`** 반환해야 shall. 첫 인자 타입은 unsigned integral type 포인터여야 shall |
| `virtual uint64 concat_get_uint64() const` | 객체의 값을 **정확히 64비트** 워드 길이의 C++ unsigned 정수로 반환해야 shall |
| `virtual int concat_length( bool* xz_present_p=0 ) const` | 객체의 비트 수를 반환해야 shall. **비트 중 `'X'` 또는 `'Z'`가 있으면** optional 인자와 연관된 객체 값을 `true`로 설정해야 shall |
| `virtual void concat_set( int64\|const sc_signed&\|const sc_unsigned&\|uint64 src , int low_i )` | 객체의 값을 첫 인자 정수의 비트 패턴으로 설정해야 shall. 비트 패턴은 두 번째 인자가 주는 위치부터 시작하는 연속 비트열로 읽어야 shall |

---

## 7.5 Limited-precision integer types

### 7.5.1 Type definitions

```cpp
namespace sc_dt {

typedef implementation-defined int_type;
typedef implementation-defined uint_type;
typedef implementation-defined int64;
typedef implementation-defined uint64;

}          // namespace sc_dt
```

| typedef | 규칙 |
|---|---|
| `int_type` | implementation-dependent native C++ 정수 타입. 구현은 **최소 64비트** 표현 크기를 제공해야 shall |
| `uint_type` | implementation-dependent native C++ unsigned 정수 타입. **최소 64비트** shall |
| `int64` | **정확히 64비트** 워드 길이의 native C++ 정수 타입 |
| `uint64` | **정확히 64비트** 워드 길이의 native C++ unsigned 정수 타입 |

### 7.5.2 `sc_int_base`

#### 7.5.2.1 Description

`sc_int_base`는 limited word-length 정수를 표현. 워드 길이는 **생성자 인자로, 또는 기본적으로 현재 scope의 `sc_length_context` 객체로** 지정된다. **`sc_int_base` 객체의 워드 길이는 인스턴스화 시 고정되고 이후 변경되지 않아야 shall.**

정수 값은 implementation-dependent native C++ 정수 타입에 담겨야 shall. **최소 표현 크기 64비트가 요구된다.**

`sc_int_base`는 `sc_int` class template의 base class.

#### 7.5.2.2 Class definition

```cpp
namespace sc_dt {

class sc_int_base
: public sc_value_base†
{
    friend class sc_uint_bitref_r†;
    friend class sc_uint_bitref†;
    friend class sc_uint_subref_r†;
    friend class sc_uint_subref†;

    public:
        // Constructors
        explicit sc_int_base( int w = sc_length_param().len() );
        sc_int_base( int_type v , int w );
        sc_int_base( const sc_int_base& a );

        template<typename T>
        explicit sc_int_base( const sc_generic_base<T>& a );
        explicit sc_int_base( const sc_int_subref_r†& a );
        explicit sc_int_base( const sc_signed& a );
        explicit sc_int_base( const sc_unsigned& a );
        explicit sc_int_base( const sc_bv_base& v );
        explicit sc_int_base( const sc_lv_base& v );
        explicit sc_int_base( const sc_uint_subref_r†& v );
        explicit sc_int_base( const sc_signed_subref_r†& v );
        explicit sc_int_base( const sc_unsigned_subref_r†& v );

        // Destructor
        ~sc_int_base();

        // Assignment operators
        sc_int_base& operator= ( int_type v );
        sc_int_base& operator= ( const sc_int_base& a );
        sc_int_base& operator= ( const sc_int_subref_r†& a );
        template<class T>
        sc_int_base& operator= ( const sc_generic_base<T>& a );
        sc_int_base& operator= ( const sc_signed& a );
        sc_int_base& operator= ( const sc_unsigned& a );
        sc_int_base& operator= ( const sc_fxval& a );
        sc_int_base& operator= ( const sc_fxval_fast& a );
        sc_int_base& operator= ( const sc_fxnum& a );
        sc_int_base& operator= ( const sc_fxnum_fast& a );
        sc_int_base& operator= ( const sc_bv_base& a );
        sc_int_base& operator= ( const sc_lv_base& a );
        sc_int_base& operator= ( const char* a );
        sc_int_base& operator= ( unsigned long a );
        sc_int_base& operator= ( long a );
        sc_int_base& operator= ( unsigned int a );
        sc_int_base& operator= ( int a );
        sc_int_base& operator= ( uint64 a );
        sc_int_base& operator= ( double a );

        // Prefix and postfix increment and decrement operators
        sc_int_base& operator++ ();                 // Prefix
        sc_int_base operator++ ( int );             // Postfix
        sc_int_base& operator-- ();                 // Prefix
        sc_int_base operator-- ( int );             // Postfix

        // Bit selection
        sc_int_bitref† operator[] ( int i );
        sc_int_bitref_r† operator[] ( int i ) const;

        // Part selection
        sc_int_subref† operator() ( int left , int right );
        sc_int_subref_r† operator() ( int left , int right ) const;
        sc_int_subref† range( int left , int right );
        sc_int_subref_r† range( int left , int right ) const;

        // Capacity
        int length() const;

        // Reduce member functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

        // Implicit conversion to int_type
        operator int_type() const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        double to_double() const;

        // Explicit conversion to character string
        std::string to_string( sc_numrep numrep = SC_DEC ) const;
        std::string to_string( sc_numrep numrep , bool w_prefix ) const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is = std::cin );
};

}          // namespace sc_dt
```

#### 7.5.2.3 Constraints on usage

- **`sc_int_base` 객체의 워드 길이는 값을 담는 정수 표현의 최대 크기보다 크면 안 된다 shall not.**

#### 7.5.2.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `explicit sc_int_base( int w = sc_length_param().len() )` | 워드 길이 `w`인 객체 생성 shall. `w` 미지정 시 default 생성자이며 값은 **현재 length context**가 설정 shall. **초기값 0** shall |
| `sc_int_base( int_type v , int w )` | 워드 길이 `w`, 초기값 `v`. 지정 워드 길이로 값을 표현할 수 없으면 **MSB 잘림이 발생해야 shall** |
| `template<class T> sc_int_base( const sc_generic_base<T>& a )` | 인자와 **일치하는 워드 길이**로 생성 shall. 초기값은 인자의 **`to_int64`** 반환값 shall |
| 그 외 생성자 | 인자의 크기·값과 일치하는 객체 생성 shall. **인자의 크기가 `sc_int_base`의 최대 워드 길이보다 크면 안 된다 shall not** |

#### 7.5.2.5 Assignment operators

오버로드 대입 연산자는 SystemC 데이터 타입과 native C++ 정수 표현 → `sc_int_base` 변환을 제공해야 shall. **truncation 또는 sign-extension은 7.2.2대로.**

#### 7.5.2.6 Implicit type conversion

```cpp
operator int_type() const;
```
`sc_int_base` → native C++ 정수 표현의 암시적 변환에 사용될 수 있다.

> NOTE 1 — 이 연산자로 표준 C++ bitwise logical/arithmetic 연산자를 `sc_int_base` 객체에 사용할 수 있다.
>
> NOTE 2 — C++ output stream 연산자, 그리고 `sc_int_base`용으로 명시적 오버로드되지 않은 다른 데이터 타입 클래스의 멤버 함수가 이 연산자를 사용한다.

#### 7.5.2.7 Explicit type conversion

```cpp
std::string to_string( sc_numrep numrep = SC_DEC ) const;
std::string to_string( sc_numrep numrep, bool w_prefix ) const;
```
7.2.12대로 `std::string` 변환을 수행해야 shall. **인자 1개 호출 ≡ 두 번째 인자 `true`. 인자 0개 호출 ≡ (`SC_DEC`, `true`).**

#### 7.5.2.8 Arithmetic, bitwise, and comparison operators

`n` = `sc_int_base` 객체, `i` = `int_type` 정수 객체. 비교 연산자의 인자는 **`sc_int_base`에서 파생된 임의 클래스**여도 된다 may.

##### Table 6 — `sc_int_base` arithmetic, bitwise, and comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `n += i` | `sc_int_base&` | assign sum |
| `n -= i` | `sc_int_base&` | assign difference |
| `n *= i` | `sc_int_base&` | assign product |
| `n /= i` | `sc_int_base&` | assign quotient |
| `n %= i` | `sc_int_base&` | assign remainder |
| `n &= i` | `sc_int_base&` | assign bitwise and |
| `n \|= i` | `sc_int_base&` | assign bitwise or |
| `n ^= i` | `sc_int_base&` | assign bitwise exclusive or |
| `n <<= i` | `sc_int_base&` | assign left-shift |
| `n >>= i` | `sc_int_base&` | assign right-shift |
| `n == n` | `bool` | test equal |
| `n != n` | `bool` | test not equal |
| `n < n` | `bool` | test less than |
| `n <= n` | `bool` | test less than or equal |
| `n > n` | `bool` | test greater than |
| `n >= n` | `bool` | test greater than or equal |

C++ 정수 타입에 허용되는 arithmetic/bitwise 연산은 **암시적 타입 변환을 통해 `sc_int_base` 객체에도 허용되어야 shall**. 그 연산들의 **반환 타입은 implementation-dependent C++ 정수 타입**.

> NOTE — 구현은 이 절의 요구를 만족하는 오버로드 연산자를 제공해야 한다. 이들이 `sc_int_base`의 멤버인지, 전역 연산자인지, 다른 방식인지는 **unspecified**.

#### 7.5.2.9 Other member functions

| 함수 | 규칙 |
|---|---|
| `void scan( std::istream& is = std::cin )` | 지정 입력 스트림에서 다음 형식화된 문자열을 읽어 값을 설정해야 shall (7.2.11) |
| `void print( std::ostream& os = std::cout ) const` | 값을 형식화된 문자열로 지정 출력 스트림에 써야 shall (7.2.11) |
| `int length() const` | 워드 길이를 반환해야 shall (7.2.5) |

### 7.5.3 `sc_uint_base`

#### 7.5.3.1 Description

limited word-length **unsigned** 정수. 워드 길이는 생성자 인자 또는 현재 scope의 `sc_length_context`로 지정되어야 shall. 인스턴스화 시 고정되고 이후 변경되지 않아야 shall. 값은 implementation-dependent native C++ unsigned 정수 타입에 담겨야 shall (**최소 64비트 요구**). `sc_uint` class template의 base class.

#### 7.5.3.2 Class definition

```cpp
namespace sc_dt {

class sc_uint_base
: public sc_value_base†
{
    friend class sc_uint_bitref_r†;
    friend class sc_uint_bitref†;
    friend class sc_uint_subref_r†;
    friend class sc_uint_subref†;

    public:
        // Constructors
        explicit sc_uint_base( int w = sc_length_param().len() );
        sc_uint_base( uint_type v , int w );
        sc_uint_base( const sc_uint_base& a );
        explicit sc_uint_base( const sc_uint_subref_r†& a );

        template <class T>
        explicit sc_uint_base( const sc_generic_base<T>& a );
        explicit sc_uint_base( const sc_bv_base& v );
        explicit sc_uint_base( const sc_lv_base& v );
        explicit sc_uint_base( const sc_int_subref_r†& v );
        explicit sc_uint_base( const sc_signed_subref_r†& v );
        explicit sc_uint_base( const sc_unsigned_subref_r†& v );
        explicit sc_uint_base( const sc_signed& a );
        explicit sc_uint_base( const sc_unsigned& a );

        // Destructor
        ~sc_uint_base();

        // Assignment operators
        sc_uint_base& operator= ( uint_type v );
        sc_uint_base& operator= ( const sc_uint_base& a );
        sc_uint_base& operator= ( const sc_uint_subref_r†& a );
        template <class T>
        sc_uint_base& operator= ( const sc_generic_base<T>& a );
        sc_uint_base& operator= ( const sc_signed& a );
        sc_uint_base& operator= ( const sc_unsigned& a );
        sc_uint_base& operator= ( const sc_fxval& a );
        sc_uint_base& operator= ( const sc_fxval_fast& a );
        sc_uint_base& operator= ( const sc_fxnum& a );
        sc_uint_base& operator= ( const sc_fxnum_fast& a );
        sc_uint_base& operator= ( const sc_bv_base& a );
        sc_uint_base& operator= ( const sc_lv_base& a );
        sc_uint_base& operator= ( const char* a );
        sc_uint_base& operator= ( unsigned long a );
        sc_uint_base& operator= ( long a );
        sc_uint_base& operator= ( unsigned int a );
        sc_uint_base& operator= ( int a );
        sc_uint_base& operator= ( int64 a );
        sc_uint_base& operator= ( double a );

        // Prefix and postfix increment and decrement operators
        sc_uint_base& operator++ ();                // Prefix
        sc_uint_base operator++ ( int );            // Postfix
        sc_uint_base& operator-- ();                // Prefix
        sc_uint_base operator-- ( int );            // Postfix

        // Bit selection
        sc_uint_bitref† operator[] ( int i );
        sc_uint_bitref_r† operator[] ( int i ) const;

        // Part selection
        sc_uint_subref† operator() ( int left, int right );
        sc_uint_subref_r† operator() ( int left, int right ) const;
        sc_uint_subref† range( int left, int right );
        sc_uint_subref_r† range( int left, int right ) const;

        // Capacity
        int length() const;

        // Reduce member functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

        // Implicit conversion to uint_type
        operator uint_type() const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        double to_double() const;

        // Explicit conversion to character string
        std::string to_string( sc_numrep numrep = SC_DEC ) const;
        std::string to_string( sc_numrep numrep , bool w_prefix ) const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is = std::cin );
};

}         // namespace sc_dt
```

#### 7.5.3.3 Constraints on usage

- **`sc_uint_base` 객체의 워드 길이는 값을 담는 unsigned 정수 표현의 최대 크기보다 크면 안 된다 shall not.**

#### 7.5.3.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `explicit sc_uint_base( int w = sc_length_param().len() )` | 워드 길이 `w`. 미지정 시 현재 length context가 설정. **초기값 0** shall |
| `sc_uint_base( uint_type v , int w )` | 워드 길이 `w`, 초기값 `v`. 표현 불가 시 **MSB 잘림** shall |
| `template<class T> sc_uint_base( const sc_generic_base<T>& a )` | 인자와 일치하는 워드 길이. 초기값은 인자의 **`to_uint64`** 반환값 shall |
| 그 외 | 인자의 크기·값과 일치. **인자 크기가 `sc_uint_base` 최대 워드 길이보다 크면 안 된다 shall not** |

#### 7.5.3.5~7.5.3.7

- **Assignment**: SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_uint_base` 변환 제공 shall, truncation/sign-extension은 7.2.2대로.
- **Implicit conversion** `operator uint_type() const` — native C++ unsigned 정수 표현으로 암시 변환. (NOTE는 `sc_int_base`와 동일 취지.)
- **`to_string`** — 7.2.12대로. 인자 1개 ≡ 두 번째 `true`; 인자 0개 ≡ (`SC_DEC`, `true`).

#### 7.5.3.8 Arithmetic, bitwise, and comparison operators

`U` = `sc_uint_base` 객체, `u` = `uint_type` 정수 객체. 비교 연산자 인자는 `sc_uint_base` 파생 클래스여도 된다 may.

##### Table 7 — `sc_uint_base` arithmetic, bitwise, and comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `U += u` | `sc_uint_base&` | assign sum |
| `U -= u` | `sc_uint_base&` | assign difference |
| `U *= u` | `sc_uint_base&` | assign product |
| `U /= u` | `sc_uint_base&` | assign quotient |
| `U %= u` | `sc_uint_base&` | assign remainder |
| `U &= u` | `sc_uint_base&` | assign bitwise and |
| `U \|= u` | `sc_uint_base&` | assign bitwise or |
| `U ^= u` | `sc_uint_base&` | assign bitwise exclusive or |
| `U <<= u` | `sc_uint_base&` | assign left-shift |
| `U >>= u` | `sc_uint_base&` | assign right-shift |
| `U == U` | `bool` | test equal |
| `U != U` | `bool` | test not equal |
| `U < U` | `bool` | test less than |
| `U <= U` | `bool` | test less than or equal |
| `U > U` | `bool` | test greater than |
| `U >= U` | `bool` | test greater than or equal |

C++ 정수 타입에 허용되는 arithmetic/bitwise 연산은 암시적 변환을 통해 `sc_uint_base`에도 허용되어야 shall. 반환 타입은 **implementation-dependent C++ 정수 타입**.

#### 7.5.3.9 Other member functions

`scan` / `print` / `length` — `sc_int_base`와 동일 (7.2.11, 7.2.5).

### 7.5.4 `sc_int`

#### 7.5.4.1 Description

limited word-length **signed** 정수 class template. **워드 길이는 템플릿 인자로 지정되어야 shall.**

base class `sc_int_base`의 public 멤버 함수 중 `sc_int`에서 override된 것은 **두 클래스에서 동일한 동작을 가져야 shall**. override되지 않은 것은 `sc_int`가 public 상속해야 shall.

#### 7.5.4.2 Class definition

```cpp
namespace sc_dt {

template <int W>
class sc_int
: public sc_int_base
{
    public:
        // Constructors
        sc_int();
        sc_int( int_type v );
        sc_int( const sc_int<W>& a );
        sc_int( const sc_int_base& a );
        sc_int( const sc_int_subref_r†& a );

        template <class T>
        sc_int( const sc_generic_base<T>& a );
        sc_int( const sc_signed& a );
        sc_int( const sc_unsigned& a );
        explicit sc_int( const sc_fxval& a );
        explicit sc_int( const sc_fxval_fast& a );
        explicit sc_int( const sc_fxnum& a );
        explicit sc_int( const sc_fxnum_fast& a );
        sc_int( const sc_bv_base& a );
        sc_int( const sc_lv_base& a );
        sc_int( const char* a );
        sc_int( unsigned long a );
        sc_int( long a );
        sc_int( unsigned int a );
        sc_int( int a );
        sc_int( uint64 a );
        sc_int( double a );

        // Assignment operators
        sc_int<W>& operator= ( int_type v );
        sc_int<W>& operator= ( const sc_int_base& a );
        sc_int<W>& operator= ( const sc_int_subref_r†& a );
        sc_int<W>& operator= ( const sc_int<W>& a );
        template <class T>
        sc_int<W>& operator= ( const sc_generic_base<T>& a );
        sc_int<W>& operator= ( const sc_signed& a );
        sc_int<W>& operator= ( const sc_unsigned& a );
        sc_int<W>& operator= ( const sc_fxval& a );
        sc_int<W>& operator= ( const sc_fxval_fast& a );
        sc_int<W>& operator= ( const sc_fxnum& a );
        sc_int<W>& operator= ( const sc_fxnum_fast& a );
        sc_int<W>& operator= ( const sc_bv_base& a );
        sc_int<W>& operator= ( const sc_lv_base& a );
        sc_int<W>& operator= ( const char* a );
        sc_int<W>& operator= ( unsigned long a );
        sc_int<W>& operator= ( long a );
        sc_int<W>& operator= ( unsigned int a );
        sc_int<W>& operator= ( int a );
        sc_int<W>& operator= ( uint64 a );
        sc_int<W>& operator= ( double a );

        // Prefix and postfix increment and decrement operators
        sc_int<W>& operator++ ();                   // Prefix
        sc_int<W> operator++ ( int );               // Postfix
        sc_int<W>& operator-- ();                   // Prefix
        sc_int<W> operator-- ( int );               // Postfix
};

}         // namespace sc_dt
```

#### 7.5.4.3 Constraints on usage

- **`sc_int` 객체의 워드 길이는 `sc_int_base`의 최대 워드 길이보다 크면 안 된다 shall not.**

#### 7.5.4.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `sc_int()` | 템플릿 인자 `W` 워드 길이로 생성 shall. **초기값 0** shall |
| `template<class T> sc_int( const sc_generic_base<T>& a )` | 템플릿 인자 워드 길이로 생성 shall. 초기값 = 인자의 **`to_int64`** 반환값 shall |
| 그 외 | 워드 길이 `W`, 값은 인자의 정수 크기에 대응 shall. 초기값 워드 길이가 템플릿 인자와 다르면 **truncation 또는 sign-extension** (7.2.2) shall |

#### 7.5.4.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_int` 변환 제공 shall. truncation/sign-extension은 7.2.2대로.

#### 7.5.4.6 Arithmetic and bitwise operators

`n` = `sc_int` 객체, `i` = `int_type` 정수 객체.

##### Table 8 — `sc_int` arithmetic and bitwise operations

| Expression | Return type | Operation |
|---|---|---|
| `n += i` | `sc_int<W>&` | assign sum |
| `n -= i` | `sc_int<W>&` | assign difference |
| `n *= i` | `sc_int<W>&` | assign product |
| `n /= i` | `sc_int<W>&` | assign quotient |
| `n %= i` | `sc_int<W>&` | assign remainder |
| `n &= i` | `sc_int<W>&` | assign bitwise and |
| `n \|= i` | `sc_int<W>&` | assign bitwise or |
| `n ^= i` | `sc_int<W>&` | assign bitwise exclusive or |
| `n <<= i` | `sc_int<W>&` | assign left-shift |
| `n >>= i` | `sc_int<W>&` | assign right-shift |

C++ 정수 타입에 허용되는 arithmetic/bitwise 연산은 암시적 변환을 통해 `sc_int`에도 허용되어야 shall. 반환 타입은 **implementation-dependent C++ 정수 타입**.

### 7.5.5 `sc_uint`

#### 7.5.5.1 Description

limited word-length **unsigned** 정수 class template. **워드 길이는 템플릿 인자로 지정되어야 shall.** base class `sc_uint_base`의 override 규칙은 `sc_int`와 동일.

#### 7.5.5.2 Class definition

```cpp
namespace sc_dt {

template <int W>
class sc_uint
: public sc_uint_base
{
    public:
        // Constructors
        sc_uint();
        sc_uint( uint_type v );
        sc_uint( const sc_uint<W>& a );
        sc_uint( const sc_uint_base& a );
        sc_uint( const sc_uint_subref_r†& a );
        template <class T>
        sc_uint( const sc_generic_base<T>& a );
        sc_uint( const sc_signed& a );
        sc_uint( const sc_unsigned& a );
        explicit sc_uint( const sc_fxval& a );
        explicit sc_uint( const sc_fxval_fast& a );
        explicit sc_uint( const sc_fxnum& a );
        explicit sc_uint( const sc_fxnum_fast& a );
        sc_uint( const sc_bv_base& a );
        sc_uint( const sc_lv_base& a );
        sc_uint( const char* a );
        sc_uint( unsigned long a );
        sc_uint( long a );
        sc_uint( unsigned int a );
        sc_uint( int a );
        sc_uint( int64 a );
        sc_uint( double a );

        // Assignment operators
        sc_uint<W>& operator= ( uint_type v );
        sc_uint<W>& operator= ( const sc_uint_base& a );
        sc_uint<W>& operator= ( const sc_uint_subref_r†& a );
        sc_uint<W>& operator= ( const sc_uint<W>& a );
        template <class T>
        sc_uint<W>& operator= ( const sc_generic_base<T>& a );
        sc_uint<W>& operator= ( const sc_signed& a );
        sc_uint<W>& operator= ( const sc_unsigned& a );
        sc_uint<W>& operator= ( const sc_fxval& a );
        sc_uint<W>& operator= ( const sc_fxval_fast& a );
        sc_uint<W>& operator= ( const sc_fxnum& a );
        sc_uint<W>& operator= ( const sc_fxnum_fast& a );
        sc_uint<W>& operator= ( const sc_bv_base& a );
        sc_uint<W>& operator= ( const sc_lv_base& a );
        sc_uint<W>& operator= ( const char* a );
        sc_uint<W>& operator= ( unsigned long a );
        sc_uint<W>& operator= ( long a );
        sc_uint<W>& operator= ( unsigned int a );
        sc_uint<W>& operator= ( int a );
        sc_uint<W>& operator= ( int64 a );
        sc_uint<W>& operator= ( double a );

        // Prefix and postfix increment and decrement operators
        sc_uint<W>& operator++ ();                  // Prefix
        sc_uint<W> operator++ ( int );              // Postfix
        sc_uint<W>& operator-- ();                  // Prefix
        sc_uint<W> operator-- ( int );              // Postfix
};

}         // namespace sc_dt
```

#### 7.5.5.3 Constraints on usage

- **`sc_uint` 객체의 워드 길이는 `sc_uint_base`의 최대 워드 길이보다 크면 안 된다 shall not.**

#### 7.5.5.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `sc_uint()` | 템플릿 인자 `W` 워드 길이. **초기값 0** shall |
| `template<class T> sc_uint( const sc_generic_base<T>& a )` | 템플릿 인자 워드 길이. 초기값 = 인자의 **`to_uint64`** 반환값 shall |
| 그 외 | 워드 길이 `W`, 인자의 정수 크기에 대응하는 값. 다르면 truncation 또는 sign-extension (7.2.2) shall |

#### 7.5.5.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_uint` 변환 제공 shall. 데이터 타입/문자열 리터럴 피연산자 크기가 `sc_uint` 워드 길이와 다르면 truncation 또는 sign-extension (7.2.2) shall.

#### 7.5.5.6 Arithmetic and bitwise operators

`U` = `sc_uint` 객체, `u` = `uint_type` 정수 객체.

##### Table 9 — `sc_uint` arithmetic and bitwise operations

| Expression | Return type | Operation |
|---|---|---|
| `U += u` | `sc_uint<W>&` | assign sum |
| `U -= u` | `sc_uint<W>&` | assign difference |
| `U *= u` | `sc_uint<W>&` | assign product |
| `U /= u` | `sc_uint<W>&` | assign quotient |
| `U %= u` | `sc_uint<W>&` | assign remainder |
| `U &= u` | `sc_uint<W>&` | assign bitwise and |
| `U \|= u` | `sc_uint<W>&` | assign bitwise or |
| `U ^= u` | `sc_uint<W>&` | assign bitwise exclusive or |
| `U <<= u` | `sc_uint<W>&` | assign left-shift |
| `U >>= u` | `sc_uint<W>&` | assign right-shift |

C++ 정수 타입에 허용되는 arithmetic/bitwise 연산은 암시적 변환을 통해 `sc_uint`에도 허용되어야 shall. 반환 타입은 implementation-dependent C++ 정수.

### 7.5.6 Bit-selects (limited-precision)

#### 7.5.6.1 Description

| 클래스 | 의미 |
|---|---|
| `sc_int_bitref_r`† | `sc_int_base`에서 선택된 비트, **rvalue**로 사용 |
| `sc_int_bitref`† | `sc_int_base`에서 선택된 비트, **lvalue**로 사용 |
| `sc_uint_bitref_r`† | `sc_uint_base`에서 선택된 비트, **rvalue**로 사용 |
| `sc_uint_bitref`† | `sc_uint_base`에서 선택된 비트, **lvalue**로 사용 |

#### 7.5.6.2 Class definition

```cpp
namespace sc_dt {

class sc_int_bitref_r†
: public sc_value_base†
{
    friend class sc_int_base;

    public:
        // Copy constructor
        sc_int_bitref_r†( const sc_int_bitref_r†& a );

        // Destructor
        virtual ~sc_int_bitref_r†();

        // Capacity
        int length() const;

        // Implicit conversion to uint64
        operator uint64 () const;
        bool operator! () const;
        bool operator~ () const;

        // Explicit conversions
        bool to_bool() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;

    protected:
        sc_int_bitref_r†();

    private:
        // Disabled
        sc_int_bitref_r†& operator= ( const sc_int_bitref_r†& );
};

// -------------------------------------------------------------

class sc_int_bitref†
: public sc_int_bitref_r†
{
    friend class sc_int_base;

    public:
        // Copy constructor
        sc_int_bitref†( const sc_int_bitref†& a );

        // Assignment operators
        sc_int_bitref†& operator= ( const sc_int_bitref_r†& b );
        sc_int_bitref†& operator= ( const sc_int_bitref†& b );
        sc_int_bitref†& operator= ( bool b );
        sc_int_bitref†& operator&= ( bool b );
        sc_int_bitref†& operator|= ( bool b );
        sc_int_bitref†& operator^= ( bool b );

        // Other member functions
        void scan( std::istream& is = std::cin );

    private:
        sc_int_bitref†();
};

// -------------------------------------------------------------

class sc_uint_bitref_r†
: public sc_value_base†
{
    friend class sc_uint_base;
    // ... sc_int_bitref_r†와 동일 패턴 (length / operator uint64 / operator! / operator~
    //     / to_bool / print / protected 기본 생성자 / Disabled operator= )
};

// -------------------------------------------------------------

class sc_uint_bitref†
: public sc_uint_bitref_r†
{
    friend class sc_uint_base;
    // ... sc_int_bitref†와 동일 패턴 (copy ctor / operator= (bitref_r†, bitref†, bool)
    //     / operator&= |= ^= (bool) / scan / private 기본 생성자 )
};

}         // namespace sc_dt
```

#### 7.5.6.3 Constraints on usage

- Bit-select 객체는 **`sc_int_base` 또는 `sc_uint_base` 객체(또는 그 파생 클래스 인스턴스)의 bit-select 연산자로만 생성되어야 shall.**
- 애플리케이션은 **어떤 bit-select 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.**
- 애플리케이션은 bit-select 객체에 대한 **레퍼런스나 포인터를 선언하면 안 된다 should not.**
- bit-select를 **함수 반환 타입으로 쓰는 것을 피할 것이 강력히 권장(strongly recommended)** — 참조 대상의 수명이 return 문 너머까지 이어지지 않을 수 있다.

```cpp
sc_dt::sc_int_bitref get_bit_n(sc_dt::sc_int_base i, int n) {
   return i[n]; // 위험: 반환된 bit-select가 지역 변수를 참조
}
```

#### 7.5.6.4 Assignment operators

- **lvalue** bit-select의 오버로드 대입 연산자는 `bool` 값으로부터의 변환을 제공해야 shall.
- **rvalue** bit-select의 대입 연산자는 애플리케이션이 쓸 수 없도록 **private으로 선언되어야 shall (Disabled).**

#### 7.5.6.5 Implicit type conversion

| 연산자 | 규칙 |
|---|---|
| `operator uint64() const` | bit-select → 정확히 64비트 native C++ unsigned 정수 암시 변환. 선택 비트가 `'1'`(true)이면 **1**, 아니면 **0** 반환해야 shall |
| `bool operator!() const` / `bool operator~() const` | 선택 비트의 **역**인 C++ `bool` 값을 반환해야 shall |

#### 7.5.6.6 Other member functions

| 함수 | 규칙 |
|---|---|
| `scan` | **lvalue** bit-select가 참조하는 비트의 값을 설정해야 shall. 값은 입력 스트림에서 읽은 다음 형식화 문자열로 얻은 C++ `bool` 값에 대응해야 shall (7.2.11) |
| `print` | bit-select가 참조하는 비트 값을 출력 스트림에 출력해야 shall. **형식은 implementation-defined**이나 `to_bool` 반환값 출력과 동등해야 shall |
| `length` | **무조건 워드 길이 1을 반환해야 shall** (7.2.5) |

### 7.5.7 Part-selects (limited-precision)

#### 7.5.7.1 Description

| 클래스 | 의미 |
|---|---|
| `sc_int_subref_r`† | `sc_int_base`의 signed 정수 part-select, **rvalue** |
| `sc_int_subref`† | `sc_int_base`의 signed 정수 part-select, **lvalue** |
| `sc_uint_subref_r`† | `sc_uint_base`의 unsigned 정수 part-select, **rvalue** |
| `sc_uint_subref`† | `sc_uint_base`의 unsigned 정수 part-select, **lvalue** |

#### 7.5.7.2 Class definition

```cpp
namespace sc_dt {

class sc_int_subref_r†
{
    friend class sc_int_base;
    friend class sc_int_subref†;

    public:
        // Copy constructor
        sc_int_subref_r†( const sc_int_subref_r†& a );

        // Destructor
        virtual ~sc_int_subref_r†();

        // Capacity
        int length() const;

        // Reduce member functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

        // Implicit conversion to uint_type
        operator uint_type() const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        double to_double() const;

        // Explicit conversion to character string
        std::string to_string( sc_numrep numrep = SC_DEC ) const;
        std::string to_string( sc_numrep numrep , bool w_prefix ) const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;

    protected:
        sc_int_subref_r†();

    private:
        // Disabled
        sc_int_subref_r†& operator= ( const sc_int_subref_r†& );
};

// -------------------------------------------------------------

class sc_int_subref†
: public sc_int_subref_r†
{
    friend class sc_int_base;

    public:
        // Copy constructor
        sc_int_subref†( const sc_int_subref†& a );

        // Assignment operators
        sc_int_subref†& operator= ( int_type v );
        sc_int_subref†& operator= ( const sc_int_base& a );
        sc_int_subref†& operator= ( const sc_int_subref_r†& a );
        sc_int_subref†& operator= ( const sc_int_subref†& a );
        template< class T >
        sc_int_subref†& operator= ( const sc_generic_base<T>& a );
        sc_int_subref†& operator= ( const char* a );
        sc_int_subref†& operator= ( unsigned long a );
        sc_int_subref†& operator= ( long a );
        sc_int_subref†& operator= ( unsigned int a );
        sc_int_subref†& operator= ( int a );
        sc_int_subref†& operator= ( uint64 a );
        sc_int_subref†& operator= ( double a );
        sc_int_subref†& operator= ( const sc_signed& );
        sc_int_subref†& operator= ( const sc_unsigned& );
        sc_int_subref†& operator= ( const sc_bv_base& );
        sc_int_subref†& operator= ( const sc_lv_base& );

        // Other member functions
        void scan( std::istream& is = std::cin );

    protected:
        sc_int_subref†();
};

// -------------------------------------------------------------

class sc_uint_subref_r†
{
    friend class sc_uint_base;
    friend class sc_uint_subref†;
    // ... sc_int_subref_r†와 동일 패턴:
    //     copy ctor / virtual ~sc_uint_subref_r() / length
    //     / and_reduce nand_reduce or_reduce nor_reduce xor_reduce xnor_reduce
    //     / operator uint_type() / to_int to_uint to_long to_ulong to_int64 to_uint64 to_double
    //     / to_string( sc_numrep = SC_DEC ) , to_string( sc_numrep , bool )
    //     / print / protected 기본 생성자
    private:
        // Disabled
        sc_uint_subref_r& operator= ( const sc_uint_subref_r& );
};

// -------------------------------------------------------------

class sc_uint_subref†
: public sc_uint_subref_r†
{
    friend class sc_uint_base;

    public:
        // Copy constructor
        sc_uint_subref†( const sc_uint_subref†& a );

        // Assignment operators
        sc_uint_subref†& operator= ( uint_type v );
        sc_uint_subref†& operator= ( const sc_uint_base& a );
        sc_uint_subref†& operator= ( const sc_uint_subref_r& a );
        sc_uint_subref†& operator= ( const sc_uint_subref& a );
        template<class T>
        sc_uint_subref†& operator= ( const sc_generic_base<T>& a );
        sc_uint_subref†& operator= ( const char* a );
        sc_uint_subref†& operator= ( unsigned long a );
        sc_uint_subref†& operator= ( long a );
        sc_uint_subref†& operator= ( unsigned int a );
        sc_uint_subref†& operator= ( int a );
        sc_uint_subref†& operator= ( int64 a );
        sc_uint_subref†& operator= ( double a );
        sc_uint_subref†& operator= ( const sc_signed& );
        sc_uint_subref†& operator= ( const sc_unsigned& );
        sc_uint_subref†& operator= ( const sc_bv_base& );
        sc_uint_subref†& operator= ( const sc_lv_base& );

        // Other member functions
        void scan( std::istream& is = std::cin );

    protected:
        sc_uint_subref†();
};

}         // namespace sc_dt
```

#### 7.5.7.3 Constraints on usage

- Integer part-select 객체는 **`sc_int_base`/`sc_uint_base` 객체(또는 파생 클래스 인스턴스)의 part-select 연산자로만 생성되어야 shall** (7.2.7).
- 애플리케이션은 **어떤 integer part-select 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.**
- integer part-select 객체에 대한 **레퍼런스/포인터를 선언하면 안 된다 should not.**
- **limited-precision integer part-select의 좌측 인덱스가 우측 인덱스보다 작으면 error shall.**
- part-select를 함수 반환 타입으로 쓰는 것을 피할 것이 **강력히 권장(strongly recommended)**.

```cpp
sc_dt::sc_int_subref get_byte(sc_dt::sc_int_base ib, int pos) {
   return ib(pos+7, pos); // 위험: 반환된 part-select가 지역 변수를 참조
}
```

#### 7.5.7.4 Assignment operators

- **lvalue** integer part-select에 대해 SystemC 데이터 타입 및 native C++ 정수 표현으로부터의 변환을 제공해야 shall. 크기가 다르면 **truncation, zero-extension, sign-extension** (7.2.2) shall.
- **rvalue** integer part-select의 대입 연산자는 **private으로 선언되어야 shall.**

#### 7.5.7.5 Implicit type conversion

```cpp
sc_int_subref_r†::operator uint_type() const;
sc_uint_subref_r†::operator uint_type() const;
```
integer part-select → native C++ **unsigned** 정수 표현의 암시적 변환에 사용될 수 있다.

> NOTE 1 — 이 연산자들이 표준 C++ bitwise logical/arithmetic 연산자를 integer part-select 객체에 사용 가능하게 한다.
>
> NOTE 2 — C++ output stream 연산자와, integer part-select용으로 명시적 오버로드되지 않은 다른 데이터 타입 클래스의 멤버 함수가 이들을 사용한다.

#### 7.5.7.6 Explicit type conversion

`to_string` — 7.2.12대로. 인자 1개 ≡ 두 번째 `true`; 인자 0개 ≡ (`SC_DEC`, `true`).

#### 7.5.7.7 Other member functions

| 함수 | 규칙 |
|---|---|
| `scan` | **lvalue** part-select가 참조하는 비트들의 값을 입력 스트림의 다음 형식화 문자열로 설정해야 shall |
| `print` | part-select가 참조하는 비트들의 값을 출력 스트림에 출력해야 shall |
| `length` | part-select의 워드 길이를 반환해야 shall (7.2.5) |

---

## 7.6 Finite-precision integer types

### 7.6.1 Type definitions

```cpp
namespace sc_dt{

typedef implementation-defined int64;
typedef implementation-defined uint64;

}          // namespace sc_dt
```

- `int64` = **정확히 64비트** 워드 길이의 native C++ 정수 타입.
- `uint64` = **정확히 64비트** 워드 길이의 native C++ unsigned 정수 타입.

### 7.6.2 Constraints on usage

오버로드된 arithmetic/comparison 연산자는 finite-precision integer 객체를 표준 C++ 정수 타입과 **유사하지만 동일하지는 않은** 규칙으로 표현식에 쓸 수 있게 한다. 표준 C++ 정수 연산자 동작과의 차이:

- a) 한 피연산자가 unsigned이고 다른 하나가 signed이면, **unsigned 피연산자가 signed로 변환되고 반환 타입도 signed여야 shall.**
- b) **뺄셈의 반환 타입은 항상 signed여야 shall.**
- c) arithmetic 연산자 반환 타입의 워드 길이는 **연산의 성질과 피연산자의 워드 길이에만 의존해야 shall.**
- d) **부동소수점 변수나 리터럴을 피연산자로 직접 쓰면 안 된다 shall not.** 먼저 적절한 signed/unsigned 정수 타입으로 변환해야 should.

### 7.6.3 `sc_signed`

#### 7.6.3.1 Description

finite word-length 정수. 워드 길이는 생성자 인자 또는 현재 scope의 length context 객체로 지정되어야 shall. **인스턴스화 시 고정되고 이후 변경되지 않아야 shall.**

정수 값은 지정 워드 길이가 결정하는 **finite precision**으로 저장되어야 shall. **정밀도는 표준 C++ 정수 타입의 제한된 해상도에 의존하지 않아야 shall.**

`sc_signed`는 `sc_bigint` class template의 base class.

#### 7.6.3.2 Class definition

```cpp
namespace sc_dt {

class sc_signed
: public sc_value_base†
{
    friend class sc_concatref†;
    friend class sc_signed_bitref_r†;
    friend class sc_signed_bitref†;
    friend class sc_signed_subref_r†;
    friend class sc_signed_subref†;
    friend class sc_unsigned;
    friend class sc_unsigned_subref;

    public:
        // Constructors
        explicit sc_signed( int nb = sc_length_param().len() );
        sc_signed( const sc_signed& v );
        sc_signed( const sc_unsigned& v );
        template<class T>
        explicit sc_signed( const sc_generic_base<T>& v );
        explicit sc_signed( const sc_bv_base& v );
        explicit sc_signed( const sc_lv_base& v );
        explicit sc_signed( const sc_int_subref_r& v );
        explicit sc_signed( const sc_uint_subref_r& v );
        explicit sc_signed( const sc_signed_subref_r& v );
        explicit sc_signed( const sc_unsigned_subref_r& v );

        // Assignment operators
        sc_signed& operator= ( const sc_signed& v );
        sc_signed& operator= ( const sc_signed_subref_r†& a );
        template< class T >
        sc_signed& operator= ( const sc_generic_base<T>& a );
        sc_signed& operator= ( const sc_unsigned& v );
        sc_signed& operator= ( const sc_unsigned_subref_r†& a );
        sc_signed& operator= ( const char* v );
        sc_signed& operator= ( int64 v );
        sc_signed& operator= ( uint64 v );
        sc_signed& operator= ( long v );
        sc_signed& operator= ( unsigned long v );
        sc_signed& operator= ( int v );
        sc_signed& operator= ( unsigned int v );
        sc_signed& operator= ( double v );
        sc_signed& operator= ( const sc_int_base& v );
        sc_signed& operator= ( const sc_uint_base& v );
        sc_signed& operator= ( const sc_bv_base& );
        sc_signed& operator= ( const sc_lv_base& );
        sc_signed& operator= ( const sc_fxval& );
        sc_signed& operator= ( const sc_fxval_fast& );
        sc_signed& operator= ( const sc_fxnum& );
        sc_signed& operator= ( const sc_fxnum_fast& );

        // Destructor
        ~sc_signed();

        // Increment operators.
        sc_signed& operator++ ();
        sc_signed operator++ ( int );

        // Decrement operators.
        sc_signed& operator-- ();
        sc_signed operator-- ( int );

        // Bit selection
        sc_signed_bitref† operator[] ( int i );
        sc_signed_bitref_r† operator[] ( int i ) const;

        // Part selection
        sc_signed_subref† range( int i , int j );
        sc_signed_subref_r† range( int i , int j ) const;
        sc_signed_subref† operator() ( int i , int j );
        sc_signed_subref_r† operator() ( int i , int j ) const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        double to_double() const;

        // Explicit conversion to character string
        std::string to_string( sc_numrep numrep = SC_DEC ) const;
        std::string to_string( sc_numrep numrep, bool w_prefix ) const;

        // Print functions
        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is = std::cin );

        // Capacity
        int length() const;

        // Reduce member functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

    // Overloaded operators

};

}          // namespace sc_dt
```

#### 7.6.3.3 Constraints on usage

- **`sc_signed` 객체는 C++ 정수 타입의 직접 대체로 사용되면 안 된다 shall not** — 암시적 타입 변환 멤버 함수가 제공되지 않기 때문. C++ 정수 값 인자를 기대하는 함수에 `sc_signed` 값을 전달하려면 **명시적 타입 변환이 필요**하다.

#### 7.6.3.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `explicit sc_signed( int nb = sc_length_param().len() )` | 워드 길이 `nb`. 미지정 시 현재 length context가 설정. **초기값 0** shall |
| `template<class T> sc_signed( const sc_generic_base<T>& a )` | 인자와 **일치하는 워드 길이**. 초기값 = 인자의 **`to_sc_signed`** 반환값 shall |
| 그 외 | 생성자 인자와 **동일한 워드 길이·값** |

#### 7.6.3.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_signed` 변환 제공 shall. truncation/sign-extension은 7.2.2대로.

#### 7.6.3.6 Explicit type conversion

`to_string` — 7.2.12대로. 인자 1개 ≡ 두 번째 `true`; 인자 0개 ≡ (`SC_DEC`, `true`).

#### 7.6.3.7 Arithmetic, bitwise, and comparison operators

- `S` = `sc_signed` 객체
- `U` = `sc_unsigned` 객체
- `i` = `int`, `long`, `unsigned int`, `unsigned long`, `sc_signed`, `sc_unsigned`, `sc_int_base`, `sc_uint_base` 중 하나의 정수 객체
- `s` = `int`, `long`, `sc_signed`, `sc_int_base` 중 하나의 **signed** 정수 객체

피연산자는 위 타입들로부터 파생된 **임의의 다른 클래스여도 된다 may**.

##### Table 10 — `sc_signed` arithmetic operations

| Expression | Return type | Operation |
|---|---|---|
| `S + i` | `sc_signed` | sc_signed addition |
| `i + S` | `sc_signed` | sc_signed addition |
| `U + s` | `sc_signed` | addition of sc_unsigned and signed |
| `s + U` | `sc_signed` | addition of signed and sc_unsigned |
| `S += i` | `sc_signed&` | sc_signed assign sum |
| `S - i` | `sc_signed` | sc_signed subtraction |
| `i - S` | `sc_signed` | sc_signed subtraction |
| `U - i` | `sc_signed` | sc_unsigned subtraction |
| `i - U` | `sc_signed` | sc_unsigned subtraction |
| `S -= i` | `sc_signed&` | sc_signed assign difference |
| `S * i` | `sc_signed` | sc_signed multiplication |
| `i * S` | `sc_signed` | sc_signed multiplication |
| `U * s` | `sc_signed` | multiplication of sc_unsigned by signed |
| `s * U` | `sc_signed` | multiplication of signed by sc_unsigned |
| `S *= i` | `sc_signed&` | sc_signed assign product |
| `S / i` | `sc_signed` | sc_signed division |
| `i / S` | `sc_signed` | sc_signed division |
| `U / s` | `sc_signed` | division of sc_unsigned by signed |
| `s / U` | `sc_signed` | division of signed by sc_unsigned |
| `S /= i` | `sc_signed&` | sc_signed assign quotient |
| `S % i` | `sc_signed` | sc_signed remainder |
| `i % S` | `sc_signed` | sc_signed remainder |
| `U % s` | `sc_signed` | remainder of sc_unsigned with signed |
| `s % U` | `sc_signed` | remainder of signed with sc_unsigned |
| `S %= i` | `sc_signed&` | sc_signed assign remainder |
| `+S` | `sc_signed` | sc_signed unary plus |
| `-S` | `sc_signed` | sc_signed unary minus |
| `-U` | `sc_signed` | sc_unsigned unary minus |

##### Table 11 — `sc_signed` bitwise operations

| Expression | Return type | Operation |
|---|---|---|
| `S & i` | `sc_signed` | sc_signed bitwise and |
| `i & S` | `sc_signed` | sc_signed bitwise and |
| `U & s` | `sc_signed` | sc_unsigned bitwise and signed |
| `s & U` | `sc_signed` | signed bitwise and sc_unsigned |
| `S &= i` | `sc_signed&` | sc_signed assign bitwise and |
| `S \| i` | `sc_signed` | sc_signed bitwise or |
| `i \| S` | `sc_signed` | sc_signed bitwise or |
| `U \| s` | `sc_signed` | sc_unsigned bitwise or signed |
| `s \| U` | `sc_signed` | signed bitwise or sc_unsigned |
| `S \|= i` | `sc_signed&` | sc_signed assign bitwise or |
| `S ^ i` | `sc_signed` | sc_signed bitwise exclusive or |
| `i ^ S` | `sc_signed` | sc_signed bitwise exclusive or |
| `U ^ s` | `sc_signed` | sc_unsigned bitwise exclusive or signed |
| `s ^ U` | `sc_signed` | sc_unsigned bitwise exclusive or signed |
| `S ^= i` | `sc_signed&` | sc_signed assign bitwise exclusive or |
| `S << i` | `sc_signed` | sc_signed left-shift |
| `U << S` | `sc_unsigned` | sc_unsigned left-shift |
| `S <<= i` | `sc_signed&` | sc_signed assign left-shift |
| `S >> i` | `sc_signed` | sc_signed right-shift |
| `U >> S` | `sc_unsigned` | sc_unsigned right-shift |
| `S >>= i` | `sc_signed&` | sc_signed assign right-shift |
| `~S` | `sc_signed` | sc_signed bitwise complement |

##### Table 12 — `sc_signed` comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `S == i` / `i == S` | `bool` | test equal |
| `S != i` / `i != S` | `bool` | test not equal |
| `S < i` / `i < S` | `bool` | test less than |
| `S <= i` / `i <= S` | `bool` | test less than or equal |
| `S > i` / `i > S` | `bool` | test greater than |
| `S >= i` / `i >= S` | `bool` | test greater than or equal |

#### 결과 워드 길이 규칙 (finite-precision signed)

**arithmetic 연산 결과가 0이면 반환값의 워드 길이는 scope 내 `sc_length_context`가 설정해야 shall.** 그 외:

| 연산 | 결과 워드 길이 |
|---|---|
| Addition | **가장 긴 피연산자의 워드 길이 + 1** shall |
| Multiplication | **두 피연산자 워드 길이의 합** shall |
| Remainder | **가장 짧은 피연산자의 워드 길이** shall |
| 그 외 모든 arithmetic 연산자 | **가장 긴 피연산자의 워드 길이** shall |
| Binary bitwise 연산자 | **가장 긴 피연산자의 워드 길이** shall |
| Left shift | **`sc_signed` 피연산자 워드 길이 + 우측(정수) 피연산자** shall. 우측에 추가되는 비트는 **0** shall |
| Right shift | **`sc_signed` 피연산자의 워드 길이** shall. 좌측에 추가되는 비트는 **`sc_signed` 피연산자의 좌측 비트와 같은 값** — 즉 **right-shift는 부호를 보존** shall |

- **우측 피연산자가 음수이면 shift 연산자의 동작은 undefined.**

> NOTE — 구현은 이 절의 요구를 만족하는 오버로드 연산자를 `sc_signed` 객체에 제공해야 한다. 멤버인지 전역인지 등은 **unspecified**.

#### 7.6.3.8 Other member functions

`scan` / `print` / `length` — 7.2.11, 7.2.5대로.

### 7.6.4 `sc_unsigned`

#### 7.6.4.1 Description

finite word-length **unsigned** 정수. 워드 길이는 생성자 인자 또는 현재 scope의 length context로 지정되어야 shall. 인스턴스화 시 고정되고 이후 변경되지 않아야 shall. 값은 지정 워드 길이가 결정하는 finite precision으로 저장되어야 shall — 표준 C++ 정수 타입의 해상도에 의존하지 않아야 shall. `sc_biguint` class template의 base class.

#### 7.6.4.2 Class definition

```cpp
namespace sc_dt {

class sc_unsigned
: public sc_value_base†
{
    friend class sc_concatref†;
    friend class sc_unsigned_bitref_r†;
    friend class sc_unsigned_bitref†;
    friend class sc_unsigned_subref_r†;
    friend class sc_unsigned_subref†;
    friend class sc_signed;
    friend class sc_signed_subref†;

    public:
        // Constructors
        explicit sc_unsigned( int nb = sc_length_param().len() );
        sc_unsigned( const sc_unsigned& v );
        sc_unsigned( const sc_signed& v );
        template<class T>
        explicit sc_unsigned( const sc_generic_base<T>& v );
        explicit sc_unsigned( const sc_bv_base& v );
        explicit sc_unsigned( const sc_lv_base& v );
        explicit sc_unsigned( const sc_int_subref_r& v );
        explicit sc_unsigned( const sc_uint_subref_r& v );
        explicit sc_unsigned( const sc_signed_subref_r& v );
        explicit sc_unsigned( const sc_unsigned_subref_r& v );

        // Assignment operators
        sc_unsigned& operator= ( const sc_unsigned& v);
        sc_unsigned& operator= ( const sc_unsigned_subref_r†& a );
        template<class T>
        sc_unsigned& operator= ( const sc_generic_base<T>& a );
        sc_unsigned& operator= ( const sc_signed& v );
        sc_unsigned& operator= ( const sc_signed_subref_r†& a );
        sc_unsigned& operator= ( const char* v);
        sc_unsigned& operator= ( int64 v );
        sc_unsigned& operator= ( uint64 v );
        sc_unsigned& operator= ( long v );
        sc_unsigned& operator= ( unsigned long v );
        sc_unsigned& operator= ( int v );
        sc_unsigned& operator= ( unsigned int v );
        sc_unsigned& operator= ( double v );
        sc_unsigned& operator= ( const sc_int_base& v );
        sc_unsigned& operator= ( const sc_uint_base& v );
        sc_unsigned& operator= ( const sc_bv_base& );
        sc_unsigned& operator= ( const sc_lv_base& );
        sc_unsigned& operator= ( const sc_fxval& );
        sc_unsigned& operator= ( const sc_fxval_fast& );
        sc_unsigned& operator= ( const sc_fxnum& );
        sc_unsigned& operator= ( const sc_fxnum_fast& );

        // Destructor
        ~sc_unsigned();

        // Increment operators
        sc_unsigned& operator++ ();
        sc_unsigned operator++ ( int );

        // Decrement operators
        sc_unsigned& operator-- ();
        sc_unsigned operator-- ( int) ;

        // Bit selection
        sc_unsigned_bitref† operator[] ( int i );
        sc_unsigned_bitref_r† operator[] ( int i ) const;

        // Part selection
        sc_unsigned_subref† range ( int i , int j );
        sc_unsigned_subref_r† range( int i , int j ) const;
        sc_unsigned_subref† operator() ( int i , int j );
        sc_unsigned_subref_r† operator() ( int i , int j ) const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        double to_double() const;

        // Explicit conversion to character string
        std::string to_string( sc_numrep numrep = SC_DEC ) const;
        std::string to_string( sc_numrep numrep, bool w_prefix ) const;

        // Print functions
        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is = std::cin );

        // Capacity
        int length() const;                                            // Bit width

        // Reduce member functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

        // Overloaded operators

};

}         // namespace sc_dt
```

#### 7.6.4.3 Constraints on usage

- **`sc_unsigned` 객체는 C++ 정수 타입의 직접 대체로 사용될 수 없다** — 암시적 타입 변환 멤버 함수가 없기 때문. C++ 정수 값 인자를 기대하는 함수에 전달하려면 **명시적 타입 변환이 필요**하다.

#### 7.6.4.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `explicit sc_unsigned( int nb = sc_length_param().len() )` | 워드 길이 `nb`. 미지정 시 현재 length context. **초기값 0** shall |
| `template<class T> sc_unsigned( const sc_generic_base<T>& a )` | 인자와 일치하는 워드 길이. 초기값 = 인자의 **`to_sc_unsigned`** 반환값 shall |
| 그 외 | 생성자 인자와 동일한 워드 길이·값 |

#### 7.6.4.5 / 7.6.4.6

- **Assignment**: SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_unsigned` 변환 제공 shall, truncation/sign-extension은 7.2.2대로.
- **`to_string`** — 7.2.12대로. 인자 1개 ≡ 두 번째 `true`; 인자 0개 ≡ (`SC_DEC`, `true`).

#### 7.6.4.7 Arithmetic, bitwise, and comparison operators

- `S` = `sc_signed`, `U` = `sc_unsigned`
- `i` = `int`, `long`, `unsigned int`, `unsigned long`, `sc_signed`, `sc_unsigned`, `sc_int_base`, `sc_uint_base`
- `s` = **signed** 정수 타입 `int`, `long`, `sc_signed`, `sc_int_base`
- `u` = **unsigned** 정수 타입 `unsigned int`, `unsigned long`, `sc_unsigned`, `sc_uint_base`

피연산자는 위 타입들로부터 파생된 임의의 다른 클래스여도 된다 may.

##### Table 13 — `sc_unsigned` arithmetic operations

| Expression | Return type | Operation |
|---|---|---|
| `U + u` / `u + U` | `sc_unsigned` | sc_unsigned addition |
| `U + s` / `s + U` | `sc_signed` | addition of sc_unsigned and signed |
| `U += i` | `sc_unsigned&` | sc_unsigned assign sum |
| `U - i` / `i - U` | `sc_signed` | sc_unsigned subtraction |
| `U -= i` | `sc_unsigned&` | sc_unsigned assign difference |
| `U * u` / `u * U` | `sc_unsigned` | sc_unsigned multiplication |
| `U * s` / `s * U` | `sc_signed` | multiplication with signed |
| `U *= i` | `sc_unsigned&` | sc_unsigned assign product |
| `U / u` / `u / U` | `sc_unsigned` | sc_unsigned division |
| `U / s` / `s / U` | `sc_signed` | division with signed |
| `U /= i` | `sc_unsigned&` | sc_unsigned assign quotient |
| `U % u` / `u % U` | `sc_unsigned` | sc_unsigned remainder |
| `U % s` / `s % U` | `sc_signed` | remainder with signed |
| `U %= i` | `sc_unsigned&` | sc_unsigned assign remainder |
| `+U` | `sc_unsigned` | sc_unsigned unary plus |
| `-U` | `sc_signed` | sc_unsigned unary minus |

##### Table 14 — `sc_unsigned` bitwise operations

| Expression | Return type | Operation |
|---|---|---|
| `U & u` / `u & U` | `sc_unsigned` | sc_unsigned bitwise and |
| `U & s` / `s & U` | `sc_signed` | bitwise and with signed |
| `U &= i` | `sc_unsigned&` | assign bitwise and |
| `U \| u` / `u \| U` | `sc_unsigned` | sc_unsigned bitwise or |
| `U \| s` / `s \| U` | `sc_signed` | bitwise or with signed |
| `U \|= i` | `sc_unsigned&` | assign bitwise or |
| `U ^ u` / `u ^ U` | `sc_unsigned` | sc_unsigned bitwise exclusive or |
| `U ^ s` / `s ^ U` | `sc_signed` | bitwise exclusive or with signed |
| `U ^= i` | `sc_unsigned&` | assign bitwise exclusive or |
| `U << i` | `sc_unsigned` | sc_unsigned left-shift |
| `S << U` | `sc_signed` | sc_signed left-shift |
| `U <<= i` | `sc_unsigned&` | assign left-shift |
| `U >> i` | `sc_unsigned` | sc_unsigned right-shift |
| `S >> U` | `sc_signed` | sc_signed right-shift |
| `U >>= i` | `sc_unsigned&` | assign right-shift |
| `~U` | `sc_unsigned` | sc_unsigned bitwise complement |

##### Table 15 — `sc_unsigned` comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `U == i` / `i == U` | `bool` | test equal |
| `U != i` / `i != U` | `bool` | test not equal |
| `U < i` / `i < U` | `bool` | test less than |
| `U <= i` / `i <= U` | `bool` | test less than or equal |
| `U > i` / `i > U` | `bool` | test greater than |
| `U >= i` / `i >= U` | `bool` | test greater than or equal |

#### 결과 워드 길이 규칙 (finite-precision unsigned)

**arithmetic 연산 결과가 0이면 반환값의 워드 길이는 scope 내 `sc_length_context`가 설정해야 shall.** 그 외:

| 연산 | 결과 워드 길이 |
|---|---|
| Addition | 가장 긴 피연산자 워드 길이 **+ 1** shall |
| Multiplication | 두 피연산자 워드 길이의 **합** shall |
| Remainder | **가장 짧은** 피연산자의 워드 길이 shall |
| 그 외 arithmetic | **가장 긴** 피연산자의 워드 길이 shall |
| Binary bitwise | **가장 긴** 피연산자의 워드 길이 shall |
| Left shift | `sc_unsigned` 피연산자 워드 길이 **+ 우측(정수) 피연산자** shall. 우측 추가 비트는 **0** shall |
| Right shift | `sc_unsigned` 피연산자의 워드 길이 shall. **좌측 추가 비트는 0** shall |

#### 7.6.4.8 Other member functions

`scan` / `print` / `length` — 7.2.11, 7.2.5대로.

### 7.6.5 `sc_bigint`

#### 7.6.5.1 Description

finite word-length **signed** 정수 class template. **워드 길이는 템플릿 인자로 지정되어야 shall.** 값은 지정 워드 길이가 결정하는 finite precision으로 저장되어야 shall — 표준 C++ 정수 타입 해상도에 의존하지 않아야 shall.

base class `sc_signed`의 public 멤버 함수 중 override된 것은 두 클래스에서 **동일 동작을 가져야 shall**. override되지 않은 것은 public 상속되어야 shall. **7.6.3.7의 연산이 `sc_bigint` 객체에 허용된다.**

#### 7.6.5.2 Class definition

```cpp
namespace sc_dt {

template< int W >
class sc_bigint
: public sc_signed
{
    public:
        // Constructors
        sc_bigint();
        sc_bigint( const sc_bigint<W>& v );
        sc_bigint( const sc_signed& v );
        sc_bigint( const sc_signed_subref†& v );
        template< class T >
        sc_bigint( const sc_generic_base<T>& a );
        sc_bigint( const sc_unsigned& v );
        sc_bigint( const sc_unsigned_subref†& v );
        sc_bigint( const char* v );
        sc_bigint( int64 v );
        sc_bigint( uint64 v );
        sc_bigint( long v );
        sc_bigint( unsigned long v );
        sc_bigint( int v );
        sc_bigint( unsigned int v );
        sc_bigint( double v );
        sc_bigint( const sc_bv_base& v );
        sc_bigint( const sc_lv_base& v );
        explicit sc_bigint( const sc_fxval& v );
        explicit sc_bigint( const sc_fxval_fast& v );
        explicit sc_bigint( const sc_fxnum& v );
        explicit sc_bigint( const sc_fxnum_fast& v );

        // Destructor
        ~sc_bigint();

        // Assignment operators
        sc_bigint<W>& operator= ( const sc_bigint<W>& v );
        sc_bigint<W>& operator= ( const sc_signed& v );
        sc_bigint<W>& operator= (const sc_signed_subref†& v );
        template< class T >
        sc_bigint<W>& operator= ( const sc_generic_base<T>& a );
        sc_bigint<W>& operator= ( const sc_unsigned& v );
        sc_bigint<W>& operator= ( const sc_unsigned_subref†& v );
        sc_bigint<W>& operator= ( const char* v );
        sc_bigint<W>& operator= ( int64 v );
        sc_bigint<W>& operator= ( uint64 v );
        sc_bigint<W>& operator= ( long v );
        sc_bigint<W>& operator= ( unsigned long v );
        sc_bigint<W>& operator= ( int v );
        sc_bigint<W>& operator= ( unsigned int v );
        sc_bigint<W>& operator= ( double v );
        sc_bigint<W>& operator= ( const sc_bv_base& v );
        sc_bigint<W>& operator= ( const sc_lv_base& v );
        sc_bigint<W>& operator= ( const sc_int_base& v );
        sc_bigint<W>& operator= ( const sc_uint_base& v );
        sc_bigint<W>& operator= ( const sc_fxval& v );
        sc_bigint<W>& operator= ( const sc_fxval_fast& v );
        sc_bigint<W>& operator= ( const sc_fxnum& v );
        sc_bigint<W>& operator= ( const sc_fxnum_fast& v );
};

}         // namespace sc_dt
```

#### 7.6.5.3 Constraints on usage

- **`sc_bigint` 객체는 C++ 정수 타입의 직접 대체로 사용될 수 없다** — 암시적 타입 변환 멤버 함수 없음. **명시적 타입 변환 필요.**

#### 7.6.5.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `sc_bigint()` | 템플릿 인자 `W` 워드 길이. **초기값 0** shall |
| `template<class T> sc_bigint( const sc_generic_base<T>& a )` | 템플릿 인자 워드 길이. 초기값 = 인자의 **`to_sc_signed`** 반환값 shall |
| 그 외 | 워드 길이 `W`, 인자의 정수 크기에 대응하는 값. 다르면 truncation 또는 sign-extension (7.2.2) shall |

> NOTE — 대부분의 생성자는 fundamental type이나 SystemC 데이터 타입 → `sc_bigint`의 **암시적 변환**으로 쓸 수 있다. 따라서 `sc_bigint` 파라미터를 가진 함수에 부동소수 인자를 넘기면 암시적으로 변환된다. **예외는 fixed-point 타입 → `sc_bigint` 변환으로, 이는 명시적으로 호출해야 한다.**

#### 7.6.5.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_bigint` 변환 제공 shall. truncation/sign-extension은 7.2.2대로.

### 7.6.6 `sc_biguint`

#### 7.6.6.1 Description

finite word-length **unsigned** 정수 class template. 워드 길이는 템플릿 인자로 지정되어야 shall. base class `sc_unsigned`의 override 규칙은 `sc_bigint`와 동일. **7.6.4.7의 연산이 `sc_biguint` 객체에 허용된다.**

#### 7.6.6.2 Class definition

```cpp
namespace sc_dt {

template< int W >
class sc_biguint
: public sc_unsigned
{
    public:
        // Constructors
        sc_biguint();
        sc_biguint( const sc_biguint<W>& v );
        sc_biguint( const sc_unsigned& v );
        sc_biguint( const sc_unsigned_subref†& v );
        template< class T >
        sc_biguint( const sc_generic_base<T>& a );
        sc_biguint( const sc_signed& v );
        sc_biguint( const sc_signed_subref†& v );
        sc_biguint( const char* v );
        sc_biguint( int64 v );
        sc_biguint( uint64 v );
        sc_biguint( long v );
        sc_biguint( unsigned long v );
        sc_biguint( int v );
        sc_biguint( unsigned int v );
        sc_biguint( double v );
        sc_biguint( const sc_bv_base& v );
        sc_biguint( const sc_lv_base& v );
        explicit sc_biguint( const sc_fxval& v );
        explicit sc_biguint( const sc_fxval_fast& v );
        explicit sc_biguint( const sc_fxnum& v );
        explicit sc_biguint( const sc_fxnum_fast& v );

        // Destructor
        ~sc_biguint();

        // Assignment operators
        sc_biguint<W>& operator= ( const sc_biguint<W>& v );
        sc_biguint<W>& operator= ( const sc_unsigned& v );
        sc_biguint<W>& operator= ( const sc_unsigned_subref†& v );
        template< class T >
        sc_biguint<W>& operator= ( const sc_generic_base<T>& a );
        sc_biguint<W>& operator= ( const sc_signed& v );
        sc_biguint<W>& operator= ( const sc_signed_subref†& v );
        sc_biguint<W>& operator= ( const char* v );
        sc_biguint<W>& operator= ( int64 v );
        sc_biguint<W>& operator= ( uint64 v );
        sc_biguint<W>& operator= ( long v );
        sc_biguint<W>& operator= ( unsigned long v );
        sc_biguint<W>& operator= ( int v );
        sc_biguint<W>& operator= ( unsigned int v );
        sc_biguint<W>& operator= ( double v );
        sc_biguint<W>& operator= ( const sc_bv_base& v );
        sc_biguint<W>& operator= ( const sc_lv_base& v );
        sc_biguint<W>& operator= ( const sc_int_base& v );
        sc_biguint<W>& operator= ( const sc_uint_base& v );
        sc_biguint<W>& operator= ( const sc_fxval& v );
        sc_biguint<W>& operator= ( const sc_fxval_fast& v );
        sc_biguint<W>& operator= ( const sc_fxnum& v );
        sc_biguint<W>& operator= ( const sc_fxnum_fast& v );
};

}          // namespace sc_dt
```

#### 7.6.6.3 Constraints on usage

- **`sc_biguint` 객체는 C++ 정수 타입의 직접 대체로 사용될 수 없다** — 명시적 타입 변환 필요.

#### 7.6.6.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `sc_biguint()` | 템플릿 인자 `W` 워드 길이. **초기값 0** shall |
| `template<class T> sc_biguint( const sc_generic_base<T>& a )` | 템플릿 인자 워드 길이. 초기값 = 인자의 **`to_sc_unsigned`** 반환값 shall |
| 그 외 | 워드 길이 `W`, 인자의 정수 크기에 대응. 다르면 truncation 또는 sign-extension (7.2.2) shall |

> NOTE — 대부분의 생성자는 암시적 변환으로 쓸 수 있다. **예외는 fixed-point 타입 → `sc_biguint` 변환으로, 명시적 호출이 필요하다.**

#### 7.6.6.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_biguint` 변환 제공 shall. truncation/sign-extension은 7.2.2대로.

### 7.6.7 Bit-selects (finite-precision)

#### 7.6.7.1 Description

| 클래스 | 의미 |
|---|---|
| `sc_signed_bitref_r`† | `sc_signed`에서 선택된 비트, **rvalue** |
| `sc_signed_bitref`† | `sc_signed`에서 선택된 비트, **lvalue** |
| `sc_unsigned_bitref_r`† | `sc_unsigned`에서 선택된 비트, **rvalue** |
| `sc_unsigned_bitref`† | `sc_unsigned`에서 선택된 비트, **lvalue** |

#### 7.6.7.2 Class definition

```cpp
namespace sc_dt {

class sc_signed_bitref_r†
: public sc_value_base†
{
    friend class sc_signed;
    friend class sc_signed_bitref†;

    public:
        // Copy constructor
        sc_signed_bitref_r†( const sc_signed_bitref_r†& a );

        // Destructor
        virtual ~sc_signed_bitref_r†();

        // Capacity
        int length() const;

        // Implicit conversion to uint64
        operator uint64 () const;
        bool operator! () const;
        bool operator~ () const;

        // Explicit conversions
        bool to_bool() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;

    protected:
        sc_signed_bitref_r†();

    private:
        // Disabled
        sc_signed_bitref_r†& operator= ( const sc_signed_bitref_r†& );
};

// -----------------------------------------------------------------

class sc_signed_bitref†
: public sc_signed_bitref_r†
{
    friend class sc_signed;

    public:
        // Copy constructor
        sc_signed_bitref†( const sc_signed_bitref†& a );

        // Assignment operators
        sc_signed_bitref†& operator= ( const sc_signed_bitref_r†& );
        sc_signed_bitref†& operator= ( const sc_signed_bitref†& );
        sc_signed_bitref†& operator= ( bool );

        sc_signed_bitref†& operator&= ( bool );
        sc_signed_bitref†& operator|= ( bool );
        sc_signed_bitref†& operator^= ( bool );

        // Other member functions
        void scan( std::istream& is = std::cin );

    protected:
        sc_signed_bitref†();
};

// -----------------------------------------------------------------

class sc_unsigned_bitref_r†
: public sc_value_base†
{
    friend class sc_unsigned;
    // ... sc_signed_bitref_r†와 동일 패턴 (copy ctor / virtual dtor / length
    //     / operator uint64 / operator! / operator~ / to_bool / print
    //     / protected 기본 생성자 / Disabled operator= )
};

// -----------------------------------------------------------------

class sc_unsigned_bitref†
: public sc_unsigned_bitref_r†
{
    friend class sc_unsigned;
    // ... sc_signed_bitref†와 동일 패턴 (copy ctor / operator= (bitref_r†, bitref†, bool)
    //     / operator&= |= ^= (bool) / scan / protected 기본 생성자 )
};

}         // namespace sc_dt
```

#### 7.6.7.3 Constraints on usage

- Bit-select 객체는 **`sc_signed`/`sc_unsigned` 객체(또는 파생 클래스 인스턴스)의 bit-select 연산자로만 생성되어야 shall.**
- 애플리케이션은 **어떤 bit-select 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.**
- bit-select 객체에 대한 **레퍼런스/포인터를 선언하면 안 된다 should not.**
- bit-select를 함수 반환 타입으로 쓰는 것을 피할 것이 **강력히 권장(strongly recommended)**.

```cpp
sc_dt::sc_signed_bitref get_bit_n(sc_dt::sc_signed iv, int n) {
   return iv[n]; // 위험: 반환된 bit-select가 지역 변수를 참조
}
```

#### 7.6.7.4 Assignment operators

- **lvalue** bit-select의 오버로드 대입 연산자는 `bool` 값 변환을 제공해야 shall.
- **rvalue** bit-select의 대입 연산자는 애플리케이션이 쓸 수 없도록 **private으로 선언되어야 shall.**

#### 7.6.7.5 Implicit type conversion

| 연산자 | 규칙 |
|---|---|
| `operator uint64() const` | 정확히 64비트 native C++ unsigned 정수로 암시 변환. 선택 비트가 `'1'`(true)이면 **1**, 아니면 **0** shall |
| `bool operator!()` / `bool operator~()` | 선택 비트의 **역**인 C++ `bool` 반환 shall |

#### 7.6.7.6 Other member functions

| 함수 | 규칙 |
|---|---|
| `scan` | **lvalue** bit-select가 참조하는 비트 값 설정 shall. 입력 스트림의 다음 형식화 문자열이 주는 C++ `bool` 값에 대응 shall |
| `print` | bit-select가 참조하는 비트 값 출력 shall. **형식은 implementation-defined**이나 `to_bool` 반환값 출력과 동등해야 shall |
| `length` | **무조건 워드 길이 1 반환** shall |

### 7.6.8 Part-selects (finite-precision)

#### 7.6.8.1 Description

| 클래스 | 의미 |
|---|---|
| `sc_signed_subref_r`† | `sc_signed`의 signed 정수 part-select, **rvalue** |
| `sc_signed_subref`† | `sc_signed`의 signed 정수 part-select, **lvalue** |
| `sc_unsigned_subref_r`† | `sc_unsigned`의 unsigned 정수 part-select, **rvalue** |
| `sc_unsigned_subref`† | `sc_unsigned`의 unsigned 정수 part-select, **lvalue** |

#### 7.6.8.2 Class definition

```cpp
namespace sc_dt {

class sc_signed_subref_r†
: public sc_value_base†
{
    friend class sc_signed;
    friend class sc_unsigned;

    public:
        // Copy constructor
        sc_signed_subref_r†( const sc_signed_subref_r†& a );

        // Destructor
        virtual ~sc_unsigned_subref_r†();

        // Capacity
        int length() const;

        // Implicit conversion to sc_unsigned
        operator sc_unsigned () const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        double to_double() const;

        // Explicit conversion to character string
        std::string to_string( sc_numrep numrep = SC_DEC ) const;
        std::string to_string( sc_numrep numrep, bool w_prefix ) const;

        // Reduce member functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;

    protected:
        sc_signed_subref_r†();

    private:
        // Disabled
        sc_signed_subref_r†& operator= ( const sc_signed_subref_r†& );
};

// --------------------------------------------------------------

class sc_signed_subref†
: public sc_signed_subref_r†
{
    friend class sc_signed;

    public:
        // Copy constructor
        sc_signed_subref†( const sc_signed_subref†& a );

        // Assignment operators
        sc_signed_subref†& operator= ( const sc_signed_subref_r†& a );
        sc_signed_subref†& operator= ( const sc_signed_subref†& a );
        sc_signed_subref†& operator= ( const sc_signed& a );
        template< class T >
        sc_signed_subref†& operator= ( const sc_generic_base<T>& a );
        sc_signed_subref†& operator= ( const sc_unsigned_subref_r†& a );
        sc_signed_subref†& operator= ( const sc_unsigned& a );
        sc_signed_subref†& operator= ( const char* a );
        sc_signed_subref†& operator= ( unsigned long a );
        sc_signed_subref†& operator= ( long a );
        sc_signed_subref†& operator= ( unsigned int a );
        sc_signed_subref†& operator= ( int a );
        sc_signed_subref†& operator= ( uint64 a );
        sc_signed_subref†& operator= ( int64 a );
        sc_signed_subref†& operator= ( double a );
        sc_signed_subref†& operator= ( const sc_int_base& a );
        sc_signed_subref†& operator= ( const sc_uint_base& a );

        // Other member functions
        void scan( std::istream& is = std::cin );

    private:
        // Disabled
        sc_signed_subref†();
};

// --------------------------------------------------------------

class sc_unsigned_subref_r†
: public sc_value_base†
{
    friend class sc_signed;
    friend class sc_unsigned;
    // ... sc_signed_subref_r†와 동일 패턴:
    //     copy ctor / virtual dtor / length / operator sc_unsigned()
    //     / to_int to_uint to_long to_ulong to_int64 to_uint64 to_double
    //     / to_string( sc_numrep = SC_DEC ) , to_string( sc_numrep , bool )
    //     / and_reduce nand_reduce or_reduce nor_reduce xor_reduce xnor_reduce
    //     / print / protected 기본 생성자
    private:
        // Disabled
        sc_unsigned_subref_r& operator= ( const sc_unsigned_subref_r†& );
};

// --------------------------------------------------------------

class sc_unsigned_subref†
: public sc_unsigned_subref_r†
{
    friend class sc_unsigned;

    public:
        // Copy constructor
        sc_unsigned_subref†( const sc_unsigned_subref†& a );

        // Assignment operators
        sc_unsigned_subref†& operator= ( const sc_unsigned_subref_r†& a );
        sc_unsigned_subref†& operator= ( const sc_unsigned_subref†& a );
        sc_unsigned_subref†& operator= ( const sc_unsigned& a );
        template<class T>
        sc_unsigned_subref†& operator= ( const sc_generic_base<T>& a );
        sc_unsigned_subref†& operator= ( const sc_signed_subref_r& a );
        sc_unsigned_subref†& operator= ( const sc_signed& a );
        sc_unsigned_subref†& operator= ( const char* a );
        sc_unsigned_subref†& operator= ( unsigned long a );
        sc_unsigned_subref†& operator= ( long a );
        sc_unsigned_subref†& operator= ( unsigned int a );
        sc_unsigned_subref†& operator= ( int a );
        sc_unsigned_subref†& operator= ( uint64 a );
        sc_unsigned_subref†& operator= ( int64 a );
        sc_unsigned_subref†& operator= ( double a );
        sc_unsigned_subref†& operator= ( const sc_int_base& a );
        sc_unsigned_subref†& operator= ( const sc_uint_base& a );

        // Other member functions
        void scan( std::istream& is = std::cin );

    protected:
        sc_unsigned_subref†();
};

}         // namespace sc_dt
```

#### 7.6.8.3 Constraints on usage

- Integer part-select 객체는 **`sc_signed`/`sc_unsigned` 객체(또는 파생 클래스 인스턴스)의 part-select 연산자로만 생성되어야 shall** (7.2.7).
- 애플리케이션은 **어떤 integer part-select 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.**
- integer part-select 객체에 대한 **레퍼런스/포인터를 선언하면 안 된다 should not.**
- part-select를 함수 반환 타입으로 쓰는 것을 피할 것이 **강력히 권장(strongly recommended)**.
- **finite-precision integer part-select의 좌측 인덱스는 우측 인덱스보다 작으면 안 된다 shall not. part-select에서 비트 순서를 뒤집을 수 없다.**

```cpp
sc_dt::sc_signed_subref get_byte(sc_dt::sc_signed s, int pos) {
   return s(pos+7, pos); // 위험: 반환된 part-select가 지역 변수를 참조
}
```

#### 7.6.8.4 Assignment operators

- **lvalue** integer part-select에 대해 SystemC 데이터 타입 및 native C++ 정수 표현으로부터의 변환 제공 shall. 크기가 다르면 **truncation, zero-extension, sign-extension** (7.2.2) shall.
- **rvalue** integer part-select의 대입 연산자는 **private으로 선언되어야 shall.**

#### 7.6.8.5 Implicit type conversion

```cpp
sc_signed_subref_r†:: operator sc_unsigned () const;
sc_unsigned_subref_r†:: operator sc_unsigned () const;
```
integer part-select → **`sc_unsigned`**의 암시적 변환에 사용될 수 있다.

> NOTE — output stream 연산자와, finite-precision integer part-select용으로 명시적 오버로드되지 않은 다른 데이터 타입 클래스의 멤버 함수가 이들을 사용한다.

#### 7.6.8.6 Explicit type conversion

`to_string` — 7.2.12대로. 인자 1개 ≡ 두 번째 `true`; 인자 0개 ≡ (`SC_DEC`, `true`).

#### 7.6.8.7 Other member functions

`scan` (lvalue part-select 비트 값 설정) / `print` / `length` (part-select 워드 길이) — 7.2.11, 7.2.5대로.

---

## 7.7 Integer concatenations

### 7.7.1 Description

`sc_concatref`†는 **concatenation base type이 SystemC integer인** 하나 이상의 객체의 비트를 concatenate한 것을 표현한다.

### 7.7.2 Class definition

```cpp
namespace sc_dt {

class sc_concatref†
: public sc_generic_base<sc_concatref†>, public sc_value_base†
{
    public:
        // Destructor
        virtual ~sc_concatref†();

        // Capacity
        unsigned int length() const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        double to_double() const;
        void to_sc_signed( sc_signed& target ) const;
        void to_sc_unsigned( sc_unsigned& target ) const;

        // Implicit conversions
        operator uint64() const;
        operator const sc_unsigned&() const;

        // Unary operators
        sc_unsigned operator+ () const;
        sc_unsigned operator- () const;
        sc_unsigned operator~ () const;

        // Explicit conversion to character string
        std::string to_string( sc_numrep numrep = SC_DEC ) const;
        std::string to_string( sc_numrep numrep , bool w_prefix ) const;

        // Assignment operators
        const sc_concatref†& operator= ( int v );
        const sc_concatref†& operator= ( unsigned int v );
        const sc_concatref†& operator= ( long v );
        const sc_concatref†& operator= ( unsigned long v );
        const sc_concatref†& operator= ( int64 v );
        const sc_concatref†& operator= ( uint64 v );
        const sc_concatref†& operator= ( const sc_concatref†& v );
        const sc_concatref†& operator= ( const sc_signed& v );
        const sc_concatref†& operator= ( const sc_unsigned& v );
        const sc_concatref†& operator= ( const char* v_p );
        const sc_concatref†& operator= ( const sc_bv_base& v );
        const sc_concatref†& operator= ( const sc_lv_base& v );

        // Reduce member functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is );

    private:
        sc_concatref†( const sc_concatref†& );
        ~sc_concatref†();
};

sc_concatref†& concat( sc_value_base†& a , sc_value_base†& b );
const sc_concatref†& concat( const sc_value_base†& a , const sc_value_base†& b );
const sc_concatref†& concat( const sc_value_base†& a, bool b );
const sc_concatref†& concat( bool a , const sc_value_base†& b );
sc_concatref†& operator, ( sc_value_base†& a , sc_value_base†& b );
const sc_concatref†& operator, ( const sc_value_base†& a , const sc_value_base†& b );
const sc_concatref†& operator, ( const sc_value_base†& a , bool b );
const sc_concatref†& operator, ( bool a , const sc_value_base†& b );

}         // namespace sc_dt
```

> **Disabled 멤버**: copy constructor와 destructor가 **private**.

### 7.7.3 Constraints on usage

- Integer concatenation 객체는 **7.2.8의 규칙에 따라 `concat` 함수(또는 `operator,`)로만 생성되어야 shall.**
- **적어도 하나의 concatenation 인자는 SystemC integer concatenation base type을 가진 객체, 즉 `sc_value_base`†로부터 직접/간접 파생된 클래스의 인스턴스여야 shall.**
- 한쪽 인자는 다음 중 하나여도 된다 may:
  - `bool` 값
  - `sc_core::sc_signal<bool,WRITER_POLICY>` 채널에 대한 레퍼런스
  - `sc_core::sc_in<bool>`, `sc_core::sc_inout<bool>`, `sc_core::sc_out<bool>` 포트에 대한 레퍼런스
- 애플리케이션은 **어떤 integer concatenation 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.** 또한 **함수 인자나 함수 반환값으로 사용해 암시적으로 생성해도 안 된다 shall not.**
- integer concatenation 객체에 대한 **레퍼런스/포인터를 선언하면 안 된다 should not.**

### 7.7.4 Assignment operators

- **lvalue** integer concatenation에 대해 SystemC 데이터 타입 및 native C++ 정수 표현으로부터의 변환 제공 shall. 크기가 다르면 **truncation, zero-extension, sign-extension** (7.2.2) shall.
- **rvalue** integer concatenation의 대입 연산자는 **애플리케이션이 호출하면 안 된다 shall not.**

### 7.7.5 Implicit type conversion

```cpp
operator uint64 () const;
operator const sc_unsigned& () const;
```
integer concatenation → **정확히 64비트 native C++ unsigned 정수**, 또는 **concatenation이 참조하는 객체들의 총 비트 수와 같은 길이의 `sc_unsigned` 객체**로 **unsigned** 암시적 타입 변환을 제공해야 shall.

> NOTE — 표준 C++ 및 SystemC bitwise logical/arithmetic 연산자를 integer concatenation 객체에 사용할 수 있게 한다.

### 7.7.6 Explicit type conversion

`to_string` — 7.2.12대로. 인자 1개 ≡ 두 번째 `true`; 인자 0개 ≡ (`SC_DEC`, `true`).

### 7.7.7 Other member functions

| 함수 | 규칙 |
|---|---|
| `scan` | **lvalue** concatenation이 참조하는 비트들의 값을 입력 스트림의 다음 형식화 문자열로 설정 shall |
| `print` | concatenation이 참조하는 비트들의 값 출력 shall |
| `length` | concatenation의 워드 길이 반환 shall (7.2.5) |

---

## 7.8 Generic base proxy class

### 7.8.1 Description

class template `sc_generic_base`는 **SystemC integer로 변환되어야 하는 애플리케이션 정의 데이터 타입**을 위한 공통 proxy base class를 제공한다.

### 7.8.2 Class definition

```cpp
namespace sc_dt {

template< class T >
class sc_generic_base
{
    public:
        inline const T* operator-> () const;
        inline T* operator-> ();
};

}         // namespace sc_dt
```

### 7.8.3 Constraints on usage

- 애플리케이션은 **`sc_generic_base`의 인스턴스를 명시적으로 생성하면 안 된다 shall not.**
- `sc_generic_base`에서 파생된 애플리케이션 정의 타입은 다음 **public const 멤버 함수를 제공해야 shall**:

| 함수 | 규칙 |
|---|---|
| `int length() const` | 정수 값을 담는 데 필요한 **비트 수**를 반환해야 shall |
| `uint64 to_uint64() const` | 값을 **정확히 64비트** native C++ unsigned 정수로 반환해야 shall |
| `int64 to_int64() const` | 값을 **정확히 64비트** native C++ signed 정수로 반환해야 shall |
| `void to_sc_unsigned( sc_unsigned& ) const` | 레퍼런스로 전달된 `sc_unsigned` 인자를 통해 값을 unsigned 정수로 반환해야 shall |
| `void to_sc_signed( sc_signed& ) const` | 레퍼런스로 전달된 `sc_signed` 인자를 통해 값을 signed 정수로 반환해야 shall |

---

## 7.9 Logic and vector types

### 7.9.1 Type definitions

logic/vector 타입 클래스가 사용하는 열거형. 리터럴 값은 (수치 순서로) 네 가지 가능한 logic 상태 — logic 0, logic 1, high-impedance, unknown — 을 표현한다.

**이 타입은 애플리케이션이 직접 쓸 의도가 아니다.** 애플리케이션은 대신 문자 리터럴 `'0'`, `'1'`, `'Z'`, `'X'`를 써야 should. 문자 리터럴이 ambiguous한 context에서는 상수 `SC_LOGIC_0`, `SC_LOGIC_1`, `SC_LOGIC_Z`, `SC_LOGIC_X`를 써도 된다 may.

```cpp
namespace sc_dt {

enum sc_logic_value_t
{
   Log_0 = 0,
   Log_1,
   Log_Z,
   Log_X
};

}         // namespace sc_dt
```

### 7.9.2 `sc_logic`

#### 7.9.2.1 Description

`sc_logic`은 네 logic 상태 중 하나에 해당하는 값을 갖는 **단일 비트**를 표현한다.

- 애플리케이션은 상태 logic 0 / logic 1 / high-impedance / unknown을 각각 문자 리터럴 `'0'`, `'1'`, `'Z'`, `'X'`로 표현해야 should.
- 소문자 `'z'`, `'x'`는 각각 `'Z'`, `'X'`의 허용 가능한 대안.
- **그 외 임의의 문자를 `sc_logic` 리터럴로 쓰면 unknown 상태로 해석되어야 shall.**
- C++ `bool` 값 `false`/`true`는 `sc_logic` 생성자와 연산자의 인자로 쓸 수 있다 may. 각각 **logic 0 / logic 1로 해석되어야 shall**.

logic 연산은 Table 16~19의 진리표를 따라 허용되어야 shall.

##### Table 16 — `sc_logic` AND truth table

| AND | `'0'` | `'1'` | `'Z'` | `'X'` |
|---|---|---|---|---|
| **`'0'`** | `'0'` | `'0'` | `'0'` | `'0'` |
| **`'1'`** | `'0'` | `'1'` | `'X'` | `'X'` |
| **`'Z'`** | `'0'` | `'X'` | `'X'` | `'X'` |
| **`'X'`** | `'0'` | `'X'` | `'X'` | `'X'` |

##### Table 17 — `sc_logic` OR truth table

| OR | `'0'` | `'1'` | `'Z'` | `'X'` |
|---|---|---|---|---|
| **`'0'`** | `'0'` | `'1'` | `'X'` | `'X'` |
| **`'1'`** | `'1'` | `'1'` | `'1'` | `'1'` |
| **`'Z'`** | `'X'` | `'1'` | `'X'` | `'X'` |
| **`'X'`** | `'X'` | `'1'` | `'X'` | `'X'` |

##### Table 18 — `sc_logic` exclusive or truth table

| XOR | `'0'` | `'1'` | `'Z'` | `'X'` |
|---|---|---|---|---|
| **`'0'`** | `'0'` | `'1'` | `'X'` | `'X'` |
| **`'1'`** | `'1'` | `'0'` | `'X'` | `'X'` |
| **`'Z'`** | `'X'` | `'X'` | `'X'` | `'X'` |
| **`'X'`** | `'X'` | `'X'` | `'X'` | `'X'` |

##### Table 19 — `sc_logic` complement truth table

| 입력 | `'0'` | `'1'` | `'Z'` | `'X'` |
|---|---|---|---|---|
| **`~`** | `'1'` | `'0'` | `'X'` | `'X'` |

#### 7.9.2.2 Class definition

```cpp
namespace sc_dt {

class sc_logic
{
    public:
        // Constructors
        sc_logic();
        sc_logic( const sc_logic& a );
        sc_logic( sc_logic_value_t v );
        explicit sc_logic( bool a );
        explicit sc_logic( char a );
        explicit sc_logic( int a );

        // Destructor
        ~sc_logic();

        // Assignment operators
        sc_logic& operator= ( const sc_logic& a );
        sc_logic& operator= ( sc_logic_value_t v );
        sc_logic& operator= ( bool a );
        sc_logic& operator= ( char a );
        sc_logic& operator= ( int a );

        // Explicit conversions
        sc_logic_value_t value() const;
        char to_char() const;
        bool to_bool() const;
        bool is_01() const;

        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is = std::cin );

    private:
        // Disabled
        explicit sc_logic( const char* );
        sc_logic& operator= ( const char* );
};

}         // namespace sc_dt
```

> **Disabled 멤버**: `const char*` 생성자와 `const char*` 대입 연산자가 **private**.

#### 7.9.2.3 Constraints on usage

- `sc_logic` 생성자/연산자의 **정수 인자는 대응하는 `sc_logic_value_t` 열거 값과 동등해야 shall.**
- **그런 정수 인자가 0~3 범위 밖이면 error shall.**
- `sc_logic` 객체에 대입하거나 초기화하는 리터럴 값은 **문자 리터럴이어도 되지만 문자열 리터럴은 안 된다 may / but not**.

#### 7.9.2.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `sc_logic()` | **초기값 unknown**인 객체 생성 shall |
| `sc_logic( const sc_logic& )`, `sc_logic( sc_logic_value_t )`, `explicit sc_logic( bool )`, `explicit sc_logic( char )`, `explicit sc_logic( int )` | 인자가 지정하는 값의 객체 생성 shall |

#### 7.9.2.5 Explicit type conversion

| 함수 | 규칙 |
|---|---|
| `sc_logic_value_t value() const` | `sc_logic` 값을 `sc_logic_value_t` 등가로 변환해야 shall |
| `char to_char() const` | `sc_logic` 값을 `char` 등가로 변환해야 shall |
| `bool to_bool() const` | `false`/`true`로 변환해야 shall. **`sc_logic` 값이 logic 0 또는 logic 1이 아니면 이 함수 호출은 error shall** |
| `bool is_01() const` | 값이 logic 0 또는 logic 1이면 `true`, 아니면 `false` 반환 shall |

#### 7.9.2.6 Bitwise and comparison operators

`L` = `sc_logic` 객체. `n` = `int`, `sc_logic`, `sc_logic_value_t`, `bool`, `char`, `int` 타입 객체.

##### Table 20 — `sc_logic` bitwise and comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `~L` | `const sc_logic` | sc_logic bitwise complement |
| `L & n` / `n & L` | `const sc_logic` | sc_logic bitwise and |
| `L &= n` | `sc_logic&` | sc_logic assign bitwise and |
| `L \| n` / `n \| L` | `const sc_logic` | sc_logic bitwise or |
| `L \|= n` | `sc_logic&` | sc_logic assign bitwise or |
| `L ^ n` / `n ^ L` | `const sc_logic` | sc_logic bitwise exclusive or |
| `L ^= n` | `sc_logic&` | sc_logic assign bitwise exclusive or |
| `L == n` / `n == L` | `bool` | test equal |
| `L != n` / `n != L` | `bool` | test not equal |

> NOTE — 구현은 이 절의 요구를 만족하는 오버로드 연산자를 제공해야 한다. 멤버인지 전역인지 등은 **unspecified**.
>
> (7.2.1 NOTE 1) — single-bit logic type에 대한 **shift 연산은 의미가 없고 undefined**.

#### 7.9.2.7 Other member functions

| 함수 | 규칙 |
|---|---|
| `scan` | 지정 입력 스트림에서 **다음 non-white-space 문자**를 읽어 값을 설정해야 shall (7.2.11) |
| `print` | 값을 문자 리터럴 `'0'`, `'1'`, `'X'`, `'Z'`로 지정 출력 스트림에 써야 shall (7.2.11) |

#### 7.9.2.8 `sc_logic` constant definitions

네 가지 `sc_logic_value_t` 상태 각각에 대해 `sc_logic` 타입 상수가 정의되어야 shall. 애플리케이션은 다른 `sc_logic` 객체에 값을 대입하거나 비교할 때, 특히 **C++ `char` 값으로부터의 암시적 변환이 ambiguous한 경우** 이 상수들을 써야 should.

```cpp
namespace sc_dt {

const sc_logic SC_LOGIC_0( Log_0 );
const sc_logic SC_LOGIC_1( Log_1 );
const sc_logic SC_LOGIC_Z( Log_Z );
const sc_logic SC_LOGIC_X( Log_X );

}         // namespace sc_dt
```

```cpp
sc_core::sc_signal<sc_dt::sc_logic> A;
A = sc_dt::SC_LOGIC_0;                                  // logic 0의 권장 표현
A = static_cast<sc_dt::sc_logic>('0');                  // 올바르지만 비권장
A = '0';                                                // Error: ambiguous conversion
```

### 7.9.3 `sc_bv_base`

#### 7.9.3.1 Description

finite word-length **bit vector**. `bool` 배열 또는 `sc_logic_value_t` 배열로 취급할 수 있다 (**단, logic 0과 logic 1 상태만 legal**). 워드 길이는 생성자 인자 또는 현재 scope의 length context 객체로 지정되어야 shall. **인스턴스화 시 고정되고 이후 변경되지 않아야 shall.** `sc_bv` class template의 base class.

#### 7.9.3.2 Class definition

```cpp
namespace sc_dt {

class sc_bv_base
{
    friend class sc_lv_base;

    public:
        // Constructors
        explicit sc_bv_base( int nb = sc_length_param().len() );
        explicit sc_bv_base( bool a, int nb = sc_length_param().len() );
        sc_bv_base( const char* a );
        sc_bv_base( const char* a , int nb );
        template <class X>
        sc_bv_base( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_bv_base( const sc_concref_r†<T1,T2>& a );
        sc_bv_base( const sc_lv_base& a );
        sc_bv_base( const sc_bv_base& a );

        // Destructor
        virtual ~sc_bv_base();

        // Assignment operators
        template <class X>
        sc_bv_base& operator= ( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_bv_base& operator= ( const sc_concref_r†<T1,T2>& a );
        sc_bv_base& operator= ( const sc_bv_base& a );
        sc_bv_base& operator= ( const sc_lv_base& a );
        sc_bv_base& operator= ( const char* a );
        sc_bv_base& operator= ( const bool* a );
        sc_bv_base& operator= ( const sc_logic* a );
        sc_bv_base& operator= ( const sc_unsigned& a );
        sc_bv_base& operator= ( const sc_signed& a );
        sc_bv_base& operator= ( const sc_uint_base& a );
        sc_bv_base& operator= ( const sc_int_base& a );
        sc_bv_base& operator= ( unsigned long a );
        sc_bv_base& operator= ( long a );
        sc_bv_base& operator= ( unsigned int a );
        sc_bv_base& operator= ( int a );
        sc_bv_base& operator= ( uint64 a );
        sc_bv_base& operator= ( int64 a );

        // Bitwise rotations
        sc_bv_base& lrotate( int n );
        sc_bv_base& rrotate( int n );

        // Bitwise reverse
        sc_bv_base& reverse();

        // Bit selection
        sc_bitref†<sc_bv_base> operator[] ( int i );
        sc_bitref_r†<sc_bv_base> operator[] ( int i ) const;

        // Part selection
        sc_subref†<sc_bv_base> operator() ( int hi , int lo );
        sc_subref_r†<sc_bv_base> operator() ( int hi , int lo ) const;
        sc_subref†<sc_bv_base> range( int hi , int lo );
        sc_subref_r†<sc_bv_base> range( int hi , int lo ) const;

        // Reduce functions
        sc_logic_value_t and_reduce() const;
        sc_logic_value_t nand_reduce() const;
        sc_logic_value_t or_reduce() const;
        sc_logic_value_t nor_reduce() const;
        sc_logic_value_t xor_reduce() const;
        sc_logic_value_t xnor_reduce() const;

        // Common member functions
        int length() const;

        // Explicit conversions to character string
        std::string to_string() const;
        std::string to_string( sc_numrep ) const;
        std::string to_string( sc_numrep , bool ) const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        bool is_01() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is = std::cin );
};

}         // namespace sc_dt
```

> **주의**: vector의 reduce 함수 반환 타입은 `bool`이 아니라 **`sc_logic_value_t`** (정수 타입의 `bool`과 다름).

#### 7.9.3.3 Constraints on usage

- **`sc_bv_base` 객체의 어떤 원소에든 `sc_logic_value_t` 값 high-impedance 또는 unknown을 대입 시도하면 error shall.**
- `bool` 배열 또는 `sc_logic` 배열을, **배열 원소 수보다 워드 길이가 더 긴** `sc_bv_base` 객체에 대입한 결과는 **undefined**.

#### 7.9.3.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `explicit sc_bv_base( int nb = sc_length_param().len() )` | 워드 길이 `nb`, **모든 원소 초기값 logic 0** shall. `nb` 미지정 시 현재 length context가 설정 |
| `explicit sc_bv_base( bool a , int nb = sc_length_param().len() )` | 워드 길이 `nb` (미지정 시 현재 length context). **모든 원소 초기값 = `a`** shall |
| `sc_bv_base( const char* a )` | 문자열 `a`로 초기값 설정. **워드 길이 = 문자열의 문자 수** shall |
| `sc_bv_base( const char* a , int nb )` | 문자열로 초기값, 워드 길이 `nb`. 문자 수가 `nb`와 다르면 **truncate 또는 zero extend** shall |
| `sc_subref_r†<X>` / `sc_concref_r†<T1,T2>` / `sc_lv_base` / `sc_bv_base` 인자 생성자 | **`a`와 같은 워드 길이·값**의 객체 생성 shall |

> NOTE — 구현은 `sc_subref_r†<T>`, `sc_concref_r†<T1,T2>`, `sc_lv_base` 객체로부터 `sc_bv_base`를 만드는 **다른 생성자 집합**을 제공해도 된다(예: 이들 모두의 공통 base class가 되는 class template 제공).

#### 7.9.3.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_bv_base` 변환 제공 shall. **truncation 또는 zero-extension** (7.2.2) 사용 shall.

#### 7.9.3.6 Explicit type conversion

```cpp
std::string to_string() const;
std::string to_string( sc_numrep ) const;
std::string to_string( sc_numrep , bool ) const;
```

- 7.2.12대로 `std::string` 변환 shall. **인자 1개 호출 ≡ 두 번째 인자 `true`.**
- **인자 없이 호출하면** 각 비트에 대응하는 단일 `'1'`/`'0'` 이진 문자열을 만들어야 shall. **이 문자열은 `"0b"`나 leading zero로 prefix되지 않아야 shall.**

```cpp
sc_dt::sc_bv_base B(4);                                          // 4비트 vector
B = "0xf";                                                       // 모든 비트 logic 1
std::string S1 = B.to_string(sc_dt::SC_BIN, false);              // S1 == "01111"
std::string S2 = B.to_string(sc_dt::SC_BIN);                     // S2 == "0b01111"
std::string S3 = B.to_string();                                  // S3 == "1111"
```

| 함수 | 규칙 |
|---|---|
| `bool is_01() const` | `sc_bv_base` 객체는 logic 0/1 원소만 가질 수 있으므로 **항상 `true` 반환** shall |
| 정수 변환 함수들 | 비트 표현의 정수 등가를 반환하는 멤버 함수가 **7.2.10 요구를 만족하도록 제공되어야 shall** |

#### 7.9.3.7 Bitwise and comparison operators

- `B` = `sc_bv_base` 객체
- `Vi` = logic vector 타입 `sc_bv_base`, `sc_lv_base`, `sc_subref_r†<T>`, `sc_concref_r†<T1,T2>` 또는 정수 타입 `int`, `long`, `unsigned int`, `unsigned long`, `sc_signed`, `sc_unsigned`, `sc_int_base`, `sc_uint_base`
- `i` = `int` 타입 정수 객체
- `A` = 원소 타입이 `char`, `bool`, `sc_logic`인 배열 객체

피연산자는 위 타입들로부터 파생된 임의의 다른 클래스여도 된다 may.

##### Table 21 — `sc_bv_base` bitwise operations

| Expression | Return type | Operation |
|---|---|---|
| `B & Vi` / `Vi & B` | `const sc_lv_base` | bitwise and |
| `B & A` / `A & B` | `const sc_lv_base` | bitwise and |
| `B &= Vi` / `B &= A` | `sc_bv_base&` | assign bitwise and |
| `B \| Vi` / `Vi \| B` | `const sc_lv_base` | bitwise or |
| `B \| A` / `A \| B` | `const sc_lv_base` | bitwise or |
| `B \|= Vi` / `B \|= A` | `sc_bv_base&` | assign bitwise or |
| `B ^ Vi` / `Vi ^ B` | `const sc_lv_base` | bitwise exclusive or |
| `B ^ A` / `A ^ B` | `const sc_lv_base` | bitwise exclusive or |
| `B ^= Vi` / `B ^= A` | `sc_bv_base&` | assign bitwise exclusive or |
| `B << i` | `const sc_lv_base` | left-shift |
| `B <<= i` | `sc_bv_base&` | assign left-shift |
| `B >> i` | `const sc_lv_base` | right-shift |
| `B >>= i` | `sc_bv_base&` | assign right-shift |
| `~B` | `const sc_lv_base` | bitwise complement |

##### Table 22 — `sc_bv_base` comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `B == Vi` / `Vi == B` | `bool` | test equal |
| `B == A` / `A == B` | `bool` | test equal |

#### 결과 워드 길이 및 shift 규칙

- Binary bitwise 연산자는 **가장 긴 피연산자의 워드 길이**로 결과를 반환해야 shall.
- Left shift: **`sc_bv_base` 피연산자 워드 길이 + 우측(정수) 피연산자** shall. 우측 추가 비트는 **0** shall.
- Right shift: **`sc_bv_base` 피연산자의 워드 길이** shall. **좌측 추가 비트는 0** shall.
- **shift 연산자의 우측 피연산자가 음수이면 error.**

| 함수 | 규칙 |
|---|---|
| `sc_bv_base& lrotate( int n )` | 객체를 **`n`칸 좌회전**해야 shall |
| `sc_bv_base& rrotate( int n )` | 객체를 **`n`칸 우회전**해야 shall |
| `sc_bv_base& reverse()` | 객체의 **비트 순서를 뒤집어야** shall |

#### 7.9.3.8 Other member functions

`scan` / `print` / `length` — 7.2.11, 7.2.5대로.

### 7.9.4 `sc_lv_base`

#### 7.9.4.1 Description

finite word-length bit vector. **`sc_logic_value_t` 값의 배열로 취급할 수 있다.** 워드 길이는 생성자 인자 또는 현재 scope의 length context 객체로 지정되어야 shall. 인스턴스화 시 고정되고 이후 변경되지 않아야 shall. `sc_lv` class template의 base class.

#### 7.9.4.2 Class definition

```cpp
namespace sc_dt {

class sc_lv_base
{
    friend class sc_bv_base;

    public:
        // Constructors
        explicit sc_lv_base( int length_ = sc_length_param().len() );
        explicit sc_lv_base( const sc_logic& a, int length_ = sc_length_param().len() );
        sc_lv_base( const char* a );
        sc_lv_base( const char* a , int length_ );
        template <class X>
        sc_lv_base( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_lv_base( const sc_concref_r†<T1,T2>& a );
        sc_lv_base( const sc_bv_base& a );
        sc_lv_base( const sc_lv_base& a );

        // Destructor
        virtual ~sc_lv_base();

        // Assignment operators
        template <class X>
        sc_lv_base& operator= ( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_lv_base& operator= ( const sc_concref_r†<T1,T2>& a );
        sc_lv_base& operator= ( const sc_bv_base& a );
        sc_lv_base& operator= ( const sc_lv_base& a );
        sc_lv_base& operator= ( const char* a );
        sc_lv_base& operator= ( const bool* a );
        sc_lv_base& operator= ( const sc_logic* a );
        sc_lv_base& operator= ( const sc_unsigned& a );
        sc_lv_base& operator= ( const sc_signed& a );
        sc_lv_base& operator= ( const sc_uint_base& a );
        sc_lv_base& operator= ( const sc_int_base& a );
        sc_lv_base& operator= ( unsigned long a );
        sc_lv_base& operator= ( long a );
        sc_lv_base& operator= ( unsigned int a );
        sc_lv_base& operator= ( int a );
        sc_lv_base& operator= ( uint64 a );
        sc_lv_base& operator= ( int64 a );

        // Bitwise rotations
        sc_lv_base& lrotate( int n );
        sc_lv_base& rrotate( int n );

        // Bitwise reverse
        sc_lv_base& reverse();

        // Bit selection
        sc_bitref†<sc_bv_base> operator[] ( int i );
        sc_bitref_r†<sc_bv_base> operator[] ( int i ) const;

        // Part selection
        sc_subref†<sc_lv_base> operator() ( int hi , int lo );
        sc_subref_r†<sc_lv_base> operator() ( int hi , int lo ) const;
        sc_subref†<sc_lv_base> range( int hi , int lo );
        sc_subref_r†<sc_lv_base> range( int hi , int lo ) const;

        // Reduce functions
        sc_logic_value_t and_reduce() const;
        sc_logic_value_t nand_reduce() const;
        sc_logic_value_t or_reduce() const;
        sc_logic_value_t nor_reduce() const;
        sc_logic_value_t xor_reduce() const;
        sc_logic_value_t xnor_reduce() const;

        // Common member functions
        int length() const;

        // Explicit conversions to character string
        std::string to_string() const;
        std::string to_string( sc_numrep ) const;
        std::string to_string( sc_numrep , bool ) const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        bool is_01() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;
        void scan( std::istream& is = std::cin );
};

}         // namespace sc_dt
```

#### 7.9.4.3 Constraints on usage

- `bool` 배열 또는 `sc_logic` 배열을, **배열 원소 수보다 워드 길이가 더 긴** `sc_lv_base` 객체에 대입한 결과는 **undefined**.

#### 7.9.4.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `explicit sc_lv_base( int nb = sc_length_param().len() )` | 워드 길이 `nb`, **모든 원소 초기값 logic 0** shall. 미지정 시 현재 length context가 설정 shall |
| `explicit sc_lv_base( bool a, int nb = sc_length_param().len() )` | 워드 길이 `nb`, **모든 원소 초기값 = `a`** shall. 미지정 시 현재 length context |
| `sc_lv_base( const char* a )` | 문자열 리터럴 `a`로 초기값. **워드 길이 = 문자열 리터럴의 문자 수** shall |
| `sc_lv_base( const char* a , int nb )` | 문자열 리터럴로 초기값, 워드 길이 `nb`. 문자 수가 `nb`와 다르면 **truncate 또는 zero extend** shall |
| `sc_lv_base( const sc_bv_base& a )` / `sc_lv_base( const sc_lv_base& a )` | **`a`와 같은 워드 길이·값** shall |

> NOTE — 구현은 `sc_subref_r†<T>`, `sc_concref_r†`, `sc_bv_base` 객체로부터 `sc_lv_base`를 만드는 다른 생성자 집합을 제공해도 된다.

#### 7.9.4.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_lv_base` 변환 제공 shall. **truncation 또는 zero-extension** (7.2.2) 사용 shall.

#### 7.9.4.6 Explicit type conversion

```cpp
std::string to_string() const;
std::string to_string( sc_numrep ) const;
std::string to_string( sc_numrep , bool ) const;
```

- 7.2.12대로. **인자 1개 호출 ≡ 두 번째 인자 `true`.**
- **원소 중 하나 이상이 high-impedance 또는 unknown 상태인 `sc_lv_base` 객체에 대해 1-인자 또는 2-인자 `to_string`을 호출하면 error shall.**
- **인자 없이 호출**하면 각 비트에 대응하는 단일 `'1'`/`'0'`/`'Z'`/`'X'` logic 값 문자열을 만들어야 shall. **`"0b"`나 leading zero로 prefix되지 않아야 shall.**

```cpp
sc_dt::sc_lv_base L(4);                            // 4비트 vector
L = "0xf";                                         // 모든 비트 logic 1
std::string S1 = L.to_string(sc_dt::SC_BIN, false); // S1 == "01111"
std::string S2 = L.to_string(sc_dt::SC_BIN);       // S2 == "0b01111"
std::string S3 = L.to_string();                    // S3 == "1111"
```

| 함수 | 규칙 |
|---|---|
| `bool is_01() const` | **모든 원소가 logic 0 또는 logic 1일 때만 `true`** 반환 shall. 원소 중 high-impedance/unknown이 있으면 `false` shall |
| 정수 변환 함수들 | 7.2.10 요구를 만족하도록 제공되어야 shall. **비트 중 하나 이상이 high-impedance 또는 unknown 상태인 객체에 대해 정수 변환 함수를 호출하면 error shall** |

#### 7.9.4.7 Bitwise and comparison operators

- `L` = `sc_lv_base` 객체
- `Vi` = logic vector 타입 `sc_bv_base`, `sc_lv_base`, `sc_subref_r†<T>`, `sc_concref_r†<T1,T2>` 또는 정수 타입 `int`, `long`, `unsigned int`, `unsigned long`, `sc_signed`, `sc_unsigned`, `sc_int_base`, `sc_uint_base`
- `i` = `int` 타입 정수 객체
- `A` = 원소 타입이 `char`, `bool`, `sc_logic`인 배열 객체

##### Table 23 — `sc_lv_base` bitwise operations

| Expression | Return type | Operation |
|---|---|---|
| `L & Vi` / `Vi & L` | `const sc_lv_base` | bitwise and |
| `L & A` / `A & L` | `const sc_lv_base` | bitwise and |
| `L &= Vi` / `L &= A` | `sc_lv_base&` | assign bitwise and |
| `L \| Vi` / `Vi \| L` | `const sc_lv_base` | bitwise or |
| `L \| A` / `A \| L` | `const sc_lv_base` | bitwise or |
| `L \|= Vi` / `L \|= A` | `sc_lv_base&` | assign bitwise or |
| `L ^ Vi` / `Vi ^ L` | `const sc_lv_base` | bitwise exclusive or |
| `L ^ A` / `A ^ L` | `const sc_lv_base` | bitwise exclusive or |
| `L ^= Vi` / `L ^= A` | `sc_lv_base&` | assign bitwise exclusive or |
| `L << i` | `const sc_lv_base` | left-shift |
| `L <<= i` | `sc_lv_base&` | assign left-shift |
| `L >> i` | `const sc_lv_base` | right-shift |
| `L >>= i` | `sc_lv_base&` | assign right-shift |
| `~L` | `const sc_lv_base` | bitwise complement |

##### Table 24 — `sc_lv_base` comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `L == Vi` / `Vi == L` | `bool` | test equal |
| `L == A` / `A == L` | `bool` | test equal |

- Binary bitwise 연산자: **가장 긴 피연산자의 워드 길이** shall.
- Left shift: `sc_lv_base` 피연산자 워드 길이 **+ 우측(정수) 피연산자** shall. 우측 추가 비트는 **0** shall.
- Right shift: `sc_lv_base` 피연산자의 워드 길이 shall. **좌측 추가 비트는 0** shall.
- **shift 연산자의 우측 피연산자가 음수이면 error.**

| 함수 | 규칙 |
|---|---|
| `lrotate( int n )` | `n`칸 좌회전 shall |
| `rrotate( int n )` | `n`칸 우회전 shall |
| `reverse()` | 비트 순서 반전 shall |

#### 7.9.4.8 Other member functions

`scan` / `print` / `length` — 7.2.11, 7.2.5대로.

### 7.9.5 `sc_bv`

#### 7.9.5.1 Description

finite word-length bit vector class template. `bool` 배열 또는 `sc_logic_value_t` 배열로 취급 가능 (**logic 0/1 상태만 legal**). **워드 길이는 템플릿 인자로 지정되어야 shall.**

base class `sc_bv_base`의 public 멤버 함수 중 override된 것은 두 클래스에서 동일 동작 shall. override되지 않은 것은 public 상속 shall.

#### 7.9.5.2 Class definition

```cpp
namespace sc_dt {

template <int W>
class sc_bv
: public sc_bv_base
{
    public:
        // Constructors
        sc_bv();
        explicit sc_bv( bool init_value );
        explicit sc_bv( char init_value );
        sc_bv( const char* a );
        sc_bv( const bool* a );
        sc_bv( const sc_logic* a );
        sc_bv( const sc_unsigned& a );
        sc_bv( const sc_signed& a );
        sc_bv( const sc_uint_base& a );
        sc_bv( const sc_int_base& a );
        sc_bv( unsigned long a );
        sc_bv( long a );
        sc_bv( unsigned int a );
        sc_bv( int a );
        sc_bv( uint64 a );
        sc_bv( int64 a );
        template <class X>
        sc_bv( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_bv( const sc_concref_r†<T1,T2>& a );
        sc_bv( const sc_bv_base& a );
        sc_bv( const sc_lv_base& a );
        sc_bv( const sc_bv<W>& a );

        // Assignment operators
        template <class X>
        sc_bv<W>& operator= ( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_bv<W>& operator= ( const sc_concref_r†<T1,T2>& a );
        sc_bv<W>& operator= ( const sc_bv_base& a );
        sc_bv<W>& operator= ( const sc_lv_base& a );
        sc_bv<W>& operator= ( const sc_bv<W>& a );
        sc_bv<W>& operator= ( const char* a );
        sc_bv<W>& operator= ( const bool* a );
        sc_bv<W>& operator= ( const sc_logic* a );
        sc_bv<W>& operator= ( const sc_unsigned& a );
        sc_bv<W>& operator= ( const sc_signed& a );
        sc_bv<W>& operator= ( const sc_uint_base& a );
        sc_bv<W>& operator= ( const sc_int_base& a );
        sc_bv<W>& operator= ( unsigned long a );
        sc_bv<W>& operator= ( long a );
        sc_bv<W>& operator= ( unsigned int a );
        sc_bv<W>& operator= ( int a );
        sc_bv<W>& operator= ( uint64 a );
        sc_bv<W>& operator= ( int64 a );
};

}         // namespace sc_dt
```

#### 7.9.5.3 Constraints on usage

- **`sc_bv` 객체의 어떤 원소에든 `sc_logic_value_t` 값 high-impedance 또는 unknown을 대입 시도하면 error shall.**
- `bool` 배열/`sc_logic` 배열을 **배열 원소 수보다 워드 길이가 긴** `sc_bv` 객체에 대입한 결과는 **undefined**.

#### 7.9.5.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `sc_bv()` | 템플릿 인자 `W` 워드 길이, **모든 원소 초기값 logic 0** shall |
| 그 외 | 워드 길이 `W`, 생성자 인자에 대응하는 값 shall. 데이터 타입/문자열 리터럴 인자의 워드 길이가 다르면 **truncation 또는 zero-extension** (7.2.2) shall. **`bool` 배열/`sc_logic` 배열 인자의 원소 수가 워드 길이보다 적으면 모든 원소의 초기값은 undefined** |

> NOTE — 구현은 `sc_subref_r†<T>`, `sc_concref_r†<T1,T2>`, `sc_bv_base`, `sc_lv_base` 객체로부터 `sc_bv`를 만드는 다른 생성자 집합을 제공해도 된다.

#### 7.9.5.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_bv` 변환 제공 shall. **truncation 또는 zero-extension** (7.2.2). **예외는 `bool` 배열 / `sc_logic` 배열의 `sc_bv` 대입** (7.9.5.4 참조 — undefined).

### 7.9.6 `sc_lv`

#### 7.9.6.1 Description

finite word-length bit vector class template. **`sc_logic_value_t` 값의 배열로 취급 가능.** 워드 길이는 템플릿 인자로 지정되어야 shall. base class `sc_lv_base`의 override 규칙은 `sc_bv`와 동일.

#### 7.9.6.2 Class definition

```cpp
namespace sc_dt {

template <int W>
class sc_lv
: public sc_lv_base
{
    public:
        // Constructors
        sc_lv();
        explicit sc_lv( const sc_logic& init_value );
        explicit sc_lv( bool init_value );
        explicit sc_lv( char init_value );
        sc_lv( const char* a );
        sc_lv( const bool* a );
        sc_lv( const sc_logic* a );
        sc_lv( const sc_unsigned& a );
        sc_lv( const sc_signed& a );
        sc_lv( const sc_uint_base& a );
        sc_lv( const sc_int_base& a );
        sc_lv( unsigned long a );
        sc_lv( long a );
        sc_lv( unsigned int a );
        sc_lv( int a );
        sc_lv( uint64 a );
        sc_lv( int64 a );
        template <class X>
        sc_lv( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_lv( const sc_concref_r†<T1,T2>& a );
        sc_lv( const sc_bv_base& a );
        sc_lv( const sc_lv_base& a );
        sc_lv( const sc_lv<W>& a );

        // Assignment operators
        template <class X>
        sc_lv<W>& operator= ( const sc_subref_r†<X>& a );
        template <class T1, class T2>
        sc_lv<W>& operator= ( const sc_concref_r†<T1,T2>& a );
        sc_lv<W>& operator= ( const sc_bv_base& a );
        sc_lv<W>& operator= ( const sc_lv_base& a );
        sc_lv<W>& operator= ( const sc_lv<W>& a );
        sc_lv<W>& operator= ( const char* a );
        sc_lv<W>& operator= ( const bool* a );
        sc_lv<W>& operator= ( const sc_logic* a );
        sc_lv<W>& operator= ( const sc_unsigned& a );
        sc_lv<W>& operator= ( const sc_signed& a );
        sc_lv<W>& operator= ( const sc_uint_base& a );
        sc_lv<W>& operator= ( const sc_int_base& a );
        sc_lv<W>& operator= ( unsigned long a );
        sc_lv<W>& operator= ( long a );
        sc_lv<W>& operator= ( unsigned int a );
        sc_lv<W>& operator= ( int a );
        sc_lv<W>& operator= ( uint64 a );
        sc_lv<W>& operator= ( int64 a );
};

}          // namespace sc_dt
```

#### 7.9.6.3 Constraints on usage

- `bool` 배열/`sc_logic` 배열을 **배열 원소 수보다 워드 길이가 긴** `sc_lv` 객체에 대입한 결과는 **undefined**.

#### 7.9.6.4 Constructors

| 생성자 | 규칙 |
|---|---|
| `sc_lv()` | 템플릿 인자 `W` 워드 길이, **모든 원소 초기값 unknown** shall (※ `sc_bv()`는 logic 0) |
| 그 외 | 워드 길이 `W`, 생성자 인자에 대응하는 값 shall. 워드 길이가 다르면 **truncation 또는 zero-extension** (7.2.2) shall. **`bool`/`sc_logic` 배열 인자의 원소 수가 워드 길이보다 적으면 모든 원소 초기값은 undefined** |

> NOTE — 구현은 `sc_subref_r†<T>`, `sc_concref_r†<T1,T2>`, `sc_bv_base`, `sc_lv_base`로부터 `sc_lv`를 만드는 다른 생성자 집합을 제공해도 된다.

#### 7.9.6.5 Assignment operators

SystemC 데이터 타입 및 native C++ 정수 표현 → `sc_lv` 변환 제공 shall. truncation 또는 zero-extension (7.2.2). **예외는 `bool` 배열 / `sc_logic` 배열로부터의 대입** (7.9.6.4 — undefined).

### 7.9.7 Bit-selects (vector)

#### 7.9.7.1 Description

| 클래스 | 의미 |
|---|---|
| `sc_bitref_r†<T>` | vector에서 선택된 비트, **rvalue** |
| `sc_bitref†<T>` | vector에서 선택된 비트, **lvalue** |

여기서 "vector"는 bit vector와 logic vector의 **part-select와 concatenation을 포함**한다. 템플릿 파라미터는 bit-select가 접근하는 클래스의 이름.

#### 7.9.7.2 Class definition

```cpp
namespace sc_dt {

template <class T>
class sc_bitref_r†
{
    friend class sc_bv_base;
    friend class sc_lv_base;

    public:
        // Copy constructor
        sc_bitref_r†( const sc_bitref_r†<T>& a );

        // Bitwise complement
        sc_logic operator~ () const;

        // Implicit conversion to sc_logic
        operator sc_logic() const;

        // Explicit conversions
        bool is_01() const;
        bool to_bool() const;
        char to_char() const;
        explicit operator bool() const;
        bool operator!() const;

        // Common member functions
        int length() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;

    private:
        // Disabled
        sc_bitref_r†();
        sc_bitref_r†<T>& operator= ( const sc_bitref_r†<T>& );
};

// -------------------------------------------------------------

template <class T>
class sc_bitref†
: public sc_bitref_r†<T>
{
    friend class sc_bv_base;
    friend class sc_lv_base;

    public:
        // Copy constructor
        sc_bitref†( const sc_bitref†<T>& a );

        // Assignment operators
        sc_bitref†<T>& operator= ( const sc_bitref_r†<T>& a );
        sc_bitref†<T>& operator= ( const sc_bitref†<T>& a );
        sc_bitref†<T>& operator= ( const sc_logic& a );
        sc_bitref†<T>& operator= ( sc_logic_value_t v );
        sc_bitref†<T>& operator= ( bool a );
        sc_bitref†<T>& operator= ( char a );
        sc_bitref†<T>& operator= ( int a );

        // Bitwise assignment operators
        sc_bitref†<T>& operator&= ( const sc_bitref_r†<T>& a );
        sc_bitref†<T>& operator&= ( const sc_logic& a );
        sc_bitref†<T>& operator&= ( sc_logic_value_t v );
        sc_bitref†<T>& operator&= ( bool a );
        sc_bitref†<T>& operator&= ( char a );
        sc_bitref†<T>& operator&= ( int a );

        sc_bitref†<T>& operator|= ( ... );   // 동일 6종 오버로드
        sc_bitref†<T>& operator^= ( ... );   // 동일 6종 오버로드

        // Other member functions
        void scan( std::istream& is = std::cin );

    private:
        // Disabled
        sc_bitref();
};

}         // namespace sc_dt
```

> **Disabled 멤버**: `sc_bitref_r†`의 **기본 생성자와 대입 연산자**가 private. `sc_bitref†`의 **기본 생성자**가 private.

#### 7.9.7.3 Constraints on usage

- Bit-select 객체는 **`sc_bv_base`/`sc_lv_base` 객체(또는 파생 클래스 인스턴스), 또는 그것의 part-select/concatenation의 bit-select 연산자로만 생성되어야 shall** (7.2.7).
- 애플리케이션은 **어떤 bit-select 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.**
- bit-select 객체에 대한 **레퍼런스/포인터를 선언하면 안 된다 should not.**
- bit-select를 함수 반환 타입으로 쓰는 것을 피할 것이 **강력히 권장(strongly recommended)**.

```cpp
sc_dt::sc_bitref<sc_dt::sc_bv_base> get_bit_n(sc_dt::sc_bv_base bv, int n) {
   return bv[n]; // 위험: 반환된 bit-select가 지역 변수를 참조
}
```

#### 7.9.7.4 Assignment operators

- **lvalue** bit-select의 오버로드 대입 연산자는 `sc_logic_value_t` 값으로의 변환을 제공해야 shall.
- **rvalue** bit-select의 대입 연산자는 애플리케이션이 쓸 수 없도록 **private으로 선언되어야 shall.**

#### 7.9.7.5 Implicit type conversion

```cpp
operator sc_logic() const;
```
bit-select와 **같은 값의 `sc_logic` 객체를 생성해야 shall**.

#### 7.9.7.6 Explicit type conversion

| 함수 | 규칙 |
|---|---|
| `char to_char() const` | bit-select 값을 `char` 등가로 변환해야 shall |
| `bool to_bool() const` | `false`/`true`로 변환해야 shall. **`sc_logic` 값이 logic 0 또는 logic 1이 아니면 호출은 error shall** |
| `bool is_01() const` | 값이 logic 0 또는 logic 1이면 `true`, 아니면 `false` shall |
| `explicit operator bool() const` | `bool` 값으로의 **명시적** 변환 제공 shall |
| `bool operator!() const` | `bool` 값으로의 **명시적 부정 변환** 제공 shall |

#### 7.9.7.7 Bitwise and comparison operators

`B` = `sc_bitref_r†<T>` (또는 임의의 파생 클래스) 객체.

##### Table 25 — `sc_bitref_r†<T>` bitwise and comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `B & B` | `const sc_logic` | bitwise and |
| `B \| B` | `const sc_logic` | bitwise or |
| `B ^ B` | `const sc_logic` | bitwise exclusive or |
| `B == B` | `bool` | test equal |
| `B != B` | `bool` | test not equal |

> NOTE — 구현은 이 절의 요구를 만족하는 오버로드 연산자를 제공해야 한다. 멤버인지 전역인지 등은 **unspecified**.

#### 7.9.7.8 Other member functions

| 함수 | 규칙 |
|---|---|
| `scan` | **lvalue** bit-select가 참조하는 비트의 값 설정 shall. 입력 스트림의 다음 형식화 문자열이 주는 C++ `bool` 값에 대응 shall |
| `print` | bit-select가 참조하는 비트 값 출력 shall. **형식은 implementation-defined**이나 `to_bool` 반환값 출력과 동등해야 shall |
| `length` | **무조건 워드 길이 1 반환** shall |

### 7.9.8 Part-selects (vector)

#### 7.9.8.1 Description

| 클래스 | 의미 |
|---|---|
| `sc_subref_r†<T>` | vector의 part-select, **rvalue** |
| `sc_subref†<T>` | vector의 part-select, **lvalue** |

"vector"는 bit vector/logic vector의 part-select와 concatenation을 포함한다. 템플릿 파라미터는 part-select가 접근하는 클래스의 이름.

**part-select에 수행 가능한 연산 집합은 연관 vector의 것과 동일해야 shall** (rvalue 객체에 적용되는 제약 하에서).

#### 7.9.8.2 Class definition

```cpp
namespace sc_dt {

template <class T>
class sc_subref_r†
{
    public:
        // Copy constructor
        sc_subref_r†( const sc_subref_r†<T>& a );

        // Bit selection
        sc_bitref_r†<sc_subref_r†<T>> operator[] ( int i ) const;

        // Part selection
        sc_subref_r†<sc_subref_r†<T>> operator() ( int hi , int lo ) const;
        sc_subref_r†<sc_subref_r†<T>> range( int hi , int lo ) const;

        // Reduce functions
        sc_logic_value_t and_reduce() const;
        sc_logic_value_t nand_reduce() const;
        sc_logic_value_t or_reduce() const;
        sc_logic_value_t nor_reduce() const;
        sc_logic_value_t xor_reduce() const;
        sc_logic_value_t xnor_reduce() const;

        // Common member functions
        int length() const;

        // Explicit conversions to character string
        std::string to_string() const;
        std::string to_string( sc_numrep ) const;
        std::string to_string( sc_numrep , bool ) const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        bool is_01() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;
        bool reversed() const;

    private:
        // Disabled
        sc_subref_r†();
        sc_subref_r†<T>& operator= ( const sc_subref_r†<T>& );
};

// -------------------------------------------------------------

template <class T>
class sc_subref†
: public sc_subref_r†<T>
{
    public:
        // Copy constructor
        sc_subref†( const sc_subref†<T>& a );

        // Assignment operators
        template <class T>
        sc_subref†<T>& operator= ( const sc_subref_r†<T>& a );
        template <class T1, class T2>
        sc_subref†<T>& operator= ( const sc_concref_r†<T1,T2>& a );
        sc_subref†<T>& operator= ( const sc_bv_base& a );
        sc_subref†<T>& operator= ( const sc_lv_base& a );
        sc_subref†<T>& operator= ( const sc_subref_r†<T>& a );
        sc_subref†<T>& operator= ( const sc_subref†<T>& a );
        sc_subref†<T>& operator= ( const char* a );
        sc_subref†<T>& operator= ( const bool* a );
        sc_subref†<T>& operator= ( const sc_logic* a );
        sc_subref†<T>& operator= ( const sc_unsigned& a );
        sc_subref†<T>& operator= ( const sc_signed& a );
        sc_subref†<T>& operator= ( const sc_uint_base& a );
        sc_subref†<T>& operator= ( const sc_int_base& a );
        sc_subref†<T>& operator= ( unsigned long a );
        sc_subref†<T>& operator= ( long a );
        sc_subref†<T>& operator= ( unsigned int a );
        sc_subref†<T>& operator= ( int a );
        sc_subref†<T>& operator= ( uint64 a );
        sc_subref†<T>& operator= ( int64 a );

        // Bitwise rotations
        sc_subref†<T>& lrotate( int n );
        sc_subref†<T>& rrotate( int n );

        // Bitwise reverse
        sc_subref†<T>& reverse();

        // Bit selection
        sc_bitref†<sc_subref†<T>> operator[] ( int i );

        // Part selection
        sc_subref†<sc_subref†<T>> operator() ( int hi , int lo );
        sc_subref†<sc_subref†<T>> range( int hi , int lo );

        // Other member functions
        void scan( std::istream& = std::cin );

    private:
        // Disabled
        sc_subref†();
};

}         // namespace sc_dt
```

> **Disabled 멤버**: `sc_subref_r†`의 **기본 생성자와 대입 연산자**가 private. `sc_subref†`의 **기본 생성자**가 private.

#### 7.9.8.3 Constraints on usage

- Part-select 객체는 **`sc_bv_base`/`sc_lv_base` 객체(또는 파생 클래스 인스턴스), 또는 그것의 part-select/concatenation의 part-select 연산자로만 생성되어야 shall** (7.2.7).
- 애플리케이션은 **어떤 part-select 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.**
- part-select 객체에 대한 **레퍼런스/포인터를 선언하면 안 된다 should not.**
- **rvalue part-select는 연관된 vector를 수정하는 데 사용되면 안 된다 shall not.**
- part-select를 함수 반환 타입으로 쓰는 것을 피할 것이 **강력히 권장(strongly recommended)**.
- **vector 타입 part-select의 좌측 인덱스는 우측 인덱스보다 작으면 안 된다 shall not. part-select에서 비트 순서를 뒤집을 수 없다.**

```cpp
sc_dt::sc_subref<sc_dt::sc_bv_base> get_byte(sc_dt::sc_bv_base bv, int pos) {
   return bv(pos+7, pos);        // 위험: 반환된 part-select가 지역 변수를 참조
}
```

#### 7.9.8.4 Assignment operators

- **lvalue** part-select에 대해 SystemC 데이터 타입 및 native C++ 정수 표현으로부터의 변환 제공 shall. 크기가 다르면 **truncation 또는 zero-extension** (7.2.2) shall.
- **`bool` 배열 또는 `sc_logic` 배열을 part-select에 대입할 때 원소 수가 part-select 워드 길이보다 적으면 part-select의 값은 undefined.**
- **rvalue** part-select의 default 대입 연산자는 애플리케이션이 쓸 수 없도록 **private**.

#### 7.9.8.5 Explicit type conversion

```cpp
std::string to_string() const;
std::string to_string( sc_numrep ) const;
std::string to_string( sc_numrep , bool ) const;
```

- 7.2.12대로. **인자 1개 ≡ 두 번째 인자 `true`.**
- **원소 중 하나 이상이 high-impedance 또는 unknown 상태인 part-select에 대해 1-인자/2-인자 `to_string`을 호출하면 error shall.**
- **인자 없이 호출**하면 각 비트에 대응하는 `'1'`/`'0'`/`'Z'`/`'X'` logic 값 문자열 생성 shall. **`"0b"`나 leading zero prefix 없음** shall.

| 함수 | 규칙 |
|---|---|
| `bool is_01() const` | **모든 원소가 logic 0/1일 때만 `true`** shall. high-impedance/unknown 원소가 있으면 `false` shall |
| 정수 변환 함수들 | 7.2.10 요구를 만족하도록 제공되어야 shall. **비트 중 하나 이상이 high-impedance/unknown인 객체에 대해 정수 변환 함수를 호출하면 error shall** |

#### 7.9.8.6 Bitwise and comparison operators

- `P` = **lvalue 또는 rvalue** vector part-select
- `L` = **lvalue** vector part-select
- `Vi` = logic vector 타입 `sc_bv_base`, `sc_lv_base`, `sc_subref_r†<T>`, `sc_concref_r†<T1,T2>` 또는 정수 타입 `int`, `long`, `unsigned int`, `unsigned long`, `sc_signed`, `sc_unsigned`, `sc_int_base`, `sc_uint_base`
- `i` = `int` 타입 정수 객체
- `A` = 원소 타입이 `char`, `bool`, `sc_logic`인 배열 객체

Table 26/28은 **모든** vector part-select에, **Table 27은 lvalue vector part-select에만** 허용된다.

##### Table 26 — `sc_subref_r†<T>` bitwise operations (모든 part-select)

| Expression | Return type | Operation |
|---|---|---|
| `P & Vi` / `Vi & P` | `const sc_lv_base` | bitwise and |
| `P & A` / `A & P` | `const sc_lv_base` | bitwise and |
| `P \| Vi` / `Vi \| P` | `const sc_lv_base` | bitwise or |
| `P \| A` / `A \| P` | `const sc_lv_base` | bitwise or |
| `P ^ Vi` / `Vi ^ P` | `const sc_lv_base` | bitwise exclusive or |
| `P ^ A` / `A ^ P` | `const sc_lv_base` | bitwise exclusive or |
| `P << i` | `const sc_lv_base` | left-shift |
| `P >> i` | `const sc_lv_base` | right-shift |
| `~P` | `const sc_lv_base` | bitwise complement |

##### Table 27 — `sc_subref†<T>` bitwise operations (**lvalue 전용**)

| Expression | Return type | Operation |
|---|---|---|
| `L &= Vi` / `L &= A` | `sc_subref_r†<T>&` | assign bitwise and |
| `L \|= Vi` / `L \|= A` | `sc_subref_r†<T>&` | assign bitwise or |
| `L ^= Vi` / `L ^= A` | `sc_subref_r†<T>&` | assign bitwise exclusive or |
| `L <<= i` | `sc_subref_r†<T>&` | assign left-shift |
| `L >>= i` | `sc_subref_r†<T>&` | assign right-shift |

##### Table 28 — `sc_subref_r†<T>` comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `P == Vi` / `Vi == P` | `bool` | test equal |
| `P == A` / `A == P` | `bool` | test equal |

- Binary bitwise 연산자: **가장 긴 피연산자의 워드 길이** shall.
- Left shift: **part-select 피연산자 워드 길이 + 우측(정수) 피연산자** shall. 우측 추가 비트 **0** shall.
- Right shift: **part-select 피연산자의 워드 길이** shall. **좌측 추가 비트 0** shall.
- **shift 연산자의 우측 피연산자가 음수이면 error.**

| 함수 | 규칙 |
|---|---|
| `sc_subref†<T>& lrotate( int n )` | **lvalue** part-select를 `n`칸 좌회전 shall |
| `sc_subref†<T>& rrotate( int n )` | **lvalue** part-select를 `n`칸 우회전 shall |
| `sc_subref†<T>& reverse()` | **lvalue** part-select의 비트 순서 반전 shall |

#### 7.9.8.7 Other member functions

| 함수 | 규칙 |
|---|---|
| `scan` | **lvalue** part-select가 참조하는 비트들의 값을 입력 스트림의 다음 형식화 문자열로 설정 shall |
| `print` | part-select가 참조하는 비트들의 값 출력 shall |
| `length` | part-select의 워드 길이 반환 shall |
| `bool reversed() const` | part-select의 원소가 **연관 vector와 역순**이면 (즉, part-select 생성에 쓰인 좌측 인덱스가 우측 인덱스보다 작으면) `true`, 아니면 `false` 반환 shall |

### 7.9.9 Concatenations (vector)

#### 7.9.9.1 Description

| 클래스 | 의미 |
|---|---|
| `sc_concref_r†<T1,T2>` | 하나 이상의 vector 비트를 concatenate한 것, **rvalue** |
| `sc_concref†<T1,T2>` | 하나 이상의 vector 비트를 concatenate한 것, **lvalue** |

"vector"는 bit vector/logic vector의 part-select와 concatenation을 포함. 템플릿 파라미터는 concatenation을 만드는 데 쓰인 **두 vector의 클래스 이름**.

**concatenation에 수행 가능한 연산 집합은 연관 vector들의 것과 동일해야 shall** (rvalue 객체 제약 하에서).

#### 7.9.9.2 Class definition

```cpp
namespace sc_dt {

template <class T1, class T2>
class sc_concref_r†
{
    public:
        // Copy constructor
        sc_concref_r†( const sc_concref_r†<T1,T2>& a );

        // Destructor
        virtual ~sc_concref_r†();

        // Bit selection
        sc_bitref_r†<sc_concref_r†<T1,T2>> operator[] ( int i ) const;

        // Part selection
        sc_subref_r†<sc_concref_r†<T1,T2>> operator() ( int hi , int lo ) const;
        sc_subref_r†<sc_concref_r†<T1,T2>> range( int hi , int lo ) const;

        // Reduce functions
        sc_logic_value_t and_reduce() const;
        sc_logic_value_t nand_reduce() const;
        sc_logic_value_t or_reduce() const;
        sc_logic_value_t nor_reduce() const;
        sc_logic_value_t xor_reduce() const;
        sc_logic_value_t xnor_reduce() const;

        // Common member functions
        int length() const;

        // Explicit conversions to character string
        std::string to_string() const;
        std::string to_string( sc_numrep ) const;
        std::string to_string( sc_numrep , bool ) const;

        // Explicit conversions
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        bool is_01() const;

        // Other member functions
        void print( std::ostream& os = std::cout ) const;

    private:
        // Disabled
        sc_concref†();
        sc_concref_r†<T1,T2>& operator= ( const sc_concref_r†<T1,T2>& );
};

// -------------------------------------------------------------

template <class T1, class T2>
class sc_concref†
: public sc_concref_r†<T1,T2>
{
    public:
        // Copy constructor
        sc_concref†( const sc_concref†<T1,T2>& a );

        // Assignment operators
        template <class T>
        sc_concref†<T1,T2>& operator= ( const sc_subref_r†<T>& a );
        template <class T1, class T2>
        sc_concref†<T1,T2>& operator= ( const sc_concref_r†<T1,T2>& a );
        sc_concref†<T1,T2>& operator= ( const sc_bv_base& a );
        sc_concref†<T1,T2>& operator= ( const sc_lv_base& a );
        sc_concref†<T1,T2>& operator= ( const sc_concref†<T1,T2>& a );
        sc_concref†<T1,T2>& operator= ( const char* a );
        sc_concref†<T1,T2>& operator= ( const bool* a );
        sc_concref†<T1,T2>& operator= ( const sc_logic* a );
        sc_concref†<T1,T2>& operator= ( const sc_unsigned& a );
        sc_concref†<T1,T2>& operator= ( const sc_signed& a );
        sc_concref†<T1,T2>& operator= ( const sc_uint_base& a );
        sc_concref†<T1,T2>& operator= ( const sc_int_base& a );
        sc_concref†<T1,T2>& operator= ( unsigned long a );
        sc_concref†<T1,T2>& operator= ( long a );
        sc_concref†<T1,T2>& operator= ( unsigned int a );
        sc_concref†<T1,T2>& operator= ( int a );
        sc_concref†<T1,T2>& operator= ( uint64 a );
        sc_concref†<T1,T2>& operator= ( int64 a );

        // Bitwise rotations
        sc_concref†<T1,T2>& lrotate( int n );
        sc_concref†<T1,T2>& rrotate( int n );

        // Bitwise reverse
        sc_concref†<T1,T2>& reverse();

        // Bit selection
        sc_bitref†<sc_concref†<T1,T2>> operator[] ( int i );

        // Part selection
        sc_subref†<sc_concref†<T1,T2>> operator() ( int hi , int lo );
        sc_subref†<sc_concref†<T1,T2>> range( int hi , int lo );

        // Other member functions
        void scan( std::istream& = std::cin );

    private:
        // Disabled
        sc_concref†();
};

// r-value concatenation operators and functions

template <typename C1, typename C2>
sc_concref_r†<C1,C2> operator, ( C1 , C2 );

template <typename C1, typename C2>
sc_concref_r†<C1,C2> concat( C1 , C2 );

// l-value concatenation operators and functions

template <typename C1, typename C2>
sc_concref†<C1,C2> operator, ( C1 , C2 );

template <typename C1, typename C2>
sc_concref†<C1,C2> concat( C1 , C2 );

}         // namespace sc_dt
```

#### 7.9.9.3 Constraints on usage

- Concatenation 객체는 **7.2.8 규칙에 따라 `concat` 함수(또는 `operator,`)로만 생성되어야 shall.**
- concatenation 인자는 **`sc_bv_base` 또는 `sc_lv_base`(또는 파생 클래스 인스턴스)를 공통 concatenation base type으로 갖는 객체, 또는 그것의 part-select/concatenation이어야 shall.**
- 애플리케이션은 **어떤 concatenation 클래스의 인스턴스도 명시적으로 생성하면 안 된다 shall not.**
- concatenation 객체에 대한 **레퍼런스/포인터를 선언하면 안 된다 should not.**
- **`concat`(또는 `operator,`)의 인자 중 하나라도 rvalue이면 rvalue concatenation이 생성되어야 shall. rvalue concatenation은 연관된 어떤 vector도 수정하는 데 사용되면 안 된다 shall not.**
- concatenation을 함수 반환 타입으로 쓰는 것을 피할 것이 **강력히 권장(strongly recommended)**.

```cpp
sc_dt::sc_concref_r<sc_dt::sc_bv_base, sc_dt::sc_bv_base> pad(sc_dt::sc_bv_base& bv, char pchar) {
   const sc_dt::sc_bv<4> padword(pchar);                 // 위험: 반환된 concatenation이
                                                         // non-static 지역 변수(padword)를 참조
   return concat(bv,padword);
}
```

#### 7.9.9.4 Assignment operators

- **lvalue** concatenation에 대해 SystemC 데이터 타입 및 native C++ 정수 표현으로부터의 변환 제공 shall. 크기가 다르면 **truncation 또는 zero-extension** (7.2.2) shall.
- **`bool` 배열 또는 `sc_logic` 배열을 concatenation에 대입할 때 원소 수가 concatenation 워드 길이보다 적으면 concatenation의 값은 undefined.**
- **rvalue** concatenation의 default 대입 연산자는 애플리케이션이 쓸 수 없도록 **private으로 선언되어야 shall.**

#### 7.9.9.5 Explicit type conversion

```cpp
std::string to_string() const;
std::string to_string( sc_numrep ) const;
std::string to_string( sc_numrep , bool ) const;
```

- 7.2.12대로. **인자 1개 ≡ 두 번째 인자 `true`.**
- **원소 중 하나 이상이 high-impedance 또는 unknown 상태인 concatenation에 대해 1-인자/2-인자 `to_string`을 호출하면 error shall.**
- **인자 없이 호출**하면 각 비트에 대응하는 `'1'`/`'0'`/`'Z'`/`'X'` logic 값 문자열 생성 shall. **`"0b"`나 leading zero prefix 없음** shall.

| 함수 | 규칙 |
|---|---|
| `bool is_01() const` | **모든 원소가 logic 0/1일 때만 `true`** shall |
| 정수 변환 함수들 | 7.2.10 요구를 만족하도록 제공 shall. **비트 중 하나 이상이 high-impedance/unknown인 객체에 대해 정수 변환 함수를 호출하면 error shall** |

#### 7.9.9.6 Bitwise and comparison operators

- `C` = **lvalue 또는 rvalue** vector concatenation
- `L` = **lvalue** vector concatenation
- `Vi` / `i` / `A` — 7.9.8.6과 동일 정의

Table 29/31은 **모든** vector concatenation에, **Table 30은 lvalue vector concatenation에만** 허용된다.

##### Table 29 — `sc_concref_r†<T1,T2>` bitwise operations (모든 concatenation)

| Expression | Return type | Operation |
|---|---|---|
| `C & Vi` / `Vi & C` | `const sc_lv_base` | bitwise and |
| `C & A` / `A & C` | `const sc_lv_base` | bitwise and |
| `C \| Vi` / `Vi \| C` | `const sc_lv_base` | bitwise or |
| `C \| A` / `A \| C` | `const sc_lv_base` | bitwise or |
| `C ^ Vi` / `Vi ^ C` | `const sc_lv_base` | bitwise exclusive or |
| `C ^ A` / `A ^ C` | `const sc_lv_base` | bitwise exclusive or |
| `C << i` | `const sc_lv_base` | left-shift |
| `C >> i` | `const sc_lv_base` | right-shift |
| `~C` | `const sc_lv_base` | bitwise complement |

##### Table 30 — `sc_concref†<T1,T2>` bitwise operations (**lvalue 전용**)

| Expression | Return type | Operation |
|---|---|---|
| `L &= Vi` / `L &= A` | `sc_concref†<T1,T2>&` | assign bitwise and |
| `L \|= Vi` / `L \|= A` | `sc_concref†<T1,T2>&` | assign bitwise or |
| `L ^= Vi` / `L ^= A` | `sc_concref†<T1,T2>&` | assign bitwise exclusive or |
| `L <<= i` | `sc_concref†<T1,T2>&` | assign left-shift |
| `L >>= i` | `sc_concref†<T1,T2>&` | assign right-shift |

##### Table 31 — `sc_concref_r†<T1,T2>` comparison operations

| Expression | Return type | Operation |
|---|---|---|
| `C == Vi` / `Vi == C` | `bool` | test equal |
| `C == A` / `A == C` | `bool` | test equal |

- Binary bitwise 연산자: **가장 긴 피연산자의 워드 길이** shall.
- Left shift: **concatenation 피연산자 워드 길이 + 우측(정수) 피연산자** shall. 우측 추가 비트 **0** shall.
- Right shift: **concatenation 피연산자의 워드 길이** shall. **좌측 추가 비트 0** shall.

| 함수 | 규칙 |
|---|---|
| `sc_concref†<T1,T2>& lrotate( int n )` | lvalue part-select를 `n`칸 좌회전 shall |
| `sc_concref†<T1,T2>& rrotate( int n )` | lvalue part-select를 `n`칸 우회전 shall |
| `sc_concref†<T1,T2>& reverse()` | lvalue part-select의 비트 순서 반전 shall |

#### 7.9.9.7 Other member functions

`scan` (lvalue concatenation 비트 값 설정) / `print` / `length` (concatenation 워드 길이) — 7.2.11, 7.2.5대로.

#### 7.9.9.8 `concat` and `operator,`

```cpp
template <typename C1, typename C2>
sc_concref_r†<C1,C2> operator, ( C1 , C2 );

template <typename C1, typename C2>
sc_concref_r†<C1,C2> concat( C1 , C2 );

template <typename C1, typename C2>
sc_concref†<C1,C2> operator, ( C1 , C2 );

template <typename C1, typename C2>
sc_concref†<C1,C2> concat( C1 , C2 );
```

- **허용되는 모든 concatenation에 대해 `concat`과 `operator,`의 명시적 template specialization이 제공되어야 shall.**
- **`concat`/`operator,`의 명시적 template specialization이 정의되어 있지 않은 두 객체를 concatenate 시도하면 error shall.**

##### rvalue concatenation specialization

인자 타입 C1, C2의 **모든 조합**에 대해 제공되어야 shall.

C1의 타입:

```
sc_bitref_r†<T>
sc_subref_r†<T>
sc_concref_r†<T1,T2>
const sc_bv_base&
const sc_lv_base&
```

C2는 위 목록 중 하나 **또는** 다음 중 하나:

```
sc_bitref†<T>
sc_subref†<T>
sc_concref†<T1,T2>
sc_bv_base&
sc_lv_base&
```

추가로, **한쪽 인자가 `bool`, `const char*`, `const sc_logic&`인 경우**에 대해서도 rvalue concatenation template specialization이 제공되어야 shall. **이 인자는 등가의 single-bit `const sc_lv_base` 객체로 암시적 변환되어야 shall.**

##### lvalue concatenation specialization

각 인자가 다음 목록의 타입인 C1, C2의 **모든 조합**에 대해 제공되어야 shall:

```
sc_bitref†<T>
sc_subref†<T>
sc_concref†<T1,T2>
sc_bv_base&
sc_lv_base&
```

---

## 7.10 Fixed-point types

### 7.10.1 Overview

7.10은 fixed-point 타입과 그 타입들이 강제하는 연산·규약을 정의한다.

### 7.10.2 Fixed-point representation

SystemC 이진 fixed-point 표현에서 수는 **binary point 위치가 지정된 비트열**로 표현되어야 shall. binary point 왼쪽 비트는 정수부, 오른쪽 비트는 소수부를 나타내야 shall.

SystemC fixed-point 타입은 다음으로 특징지어져야 shall:

| 요소 | 의미 |
|---|---|
| **wl** (word length) | 수 표현의 **전체 비트 수** |
| **iwl** (integer word length) | 정수부 비트 수 = **최좌측 비트 기준 binary point의 위치** |
| bit encoding | signed(2의 보수) 또는 unsigned |

- 최우측 비트가 LSB, 최좌측 비트가 MSB여야 shall.
- binary point는 **데이터 비트 바깥에 위치해도 된다 may** — LSB보다 오른쪽이거나 MSB보다 왼쪽일 수 있다.

세 가지 해석 케이스:

| 조건 | 해석 |
|---|---|
| `wl < iwl` | LSB와 binary point 사이에 `(iwl-wl)`개의 0이 있다 |
| `0 <= iwl <= wl` | binary point가 비트 표현 내부에 있다 |
| `iwl < 0` | binary point와 MSB 사이에 `(-iwl)`개의 부호확장 비트가 있다. unsigned면 0, signed면 MSB 반복 |

**Table 32 — fixed-point 형식 예** (x = 임의 이진 숫자, s = 부호확장 숫자)

| Index | wl | iwl | 표현 | Range signed | Range unsigned |
|---|---|---|---|---|---|
| 1 | 5 | 7 | `xxxxx00.` | [–64, 60] | [0, 124] |
| 2 | 5 | 5 | `xxxxx.` | [–16, 15] | [0, 31] |
| 3 | 5 | 3 | `xxx.xx` | [–4, 3.75] | [0, 7.75] |
| 4 | 5 | 1 | `x.xxxx` | [–1, 0.9375] | [0, 1.9375] |
| 5 | 5 | 0 | `.xxxxx` | [–0.5, 0.46875] | [0, 0.96875] |
| 6 | 5 | –2 | `.ssxxxxx` | [0.125, 0.1171875] | [0, 0.2421875] |
| 7 | 1 | –1 | `.sx` | [–0.25, 0] | [0, 0.25] |

- signed 타입의 MSB는 **부호 비트**여야 shall. 부호 비트가 binary point 뒤에 있어도 된다 may.
- signed 값 범위: `[ –2^(iwl–1) , 2^(iwl–1) – 2^(–(wl–iwl)) ]`
- unsigned 값 범위: `[ 0 , 2^iwl – 2^(–(wl–iwl)) ]`

### 7.10.3 Fixed-point type conversion

fixed-point 변수에 값이 **대입(초기화 포함)될 때마다** 타입 변환이 수행되어야 shall.

값의 크기가 표현 범위를 벗어나거나 정밀도가 표현 가능 범위를 초과하면, 표현 가능한 값으로 매핑되어야 shall. 이 변환은 **2단계**로 수행되어야 shall:

1. 범위 안이지만 정밀도가 초과되면(표현 가능한 두 값 사이) → **quantization** 수행
2. 크기가 범위를 벗어나면 → **overflow handling** 수행

- 타깃 표현이 더 높은 정밀도를 가지면 추가 LSB는 **zero extension**되어야 shall.
- 타깃 표현이 더 넓은 범위를 가지면 signed는 **sign extension**, unsigned는 **zero extension**되어야 shall.

복수의 quantization mode(7.10.10.2)와 overflow mode(7.10.10.10)가 정의된다.

### 7.10.4 Fixed-point data types

#### 7.10.4.2 Finite-precision fixed-point types

다음 finite-/variable-precision 타입이 제공되어야 shall:

```cpp
sc_fixed<wl,iwl,q_mode,o_mode,n_bits>
sc_ufixed<wl,iwl,q_mode,o_mode,n_bits>
sc_fix
sc_ufix
sc_fxval
```

- 이 타입들은 표현(wl, iwl)과 변환 모드(q_mode, o_mode, n_bits)로 파라미터화되어야 shall. 변수 선언 시 이 값들을 지정해야 shall.
- 변수의 **타입 파라미터 값은 선언 이후 수정되면 안 된다 shall not**.
- 변수에 대입되는 모든 값은 지정된 표현으로 변환되고, 필요하면 지정된 q_mode/o_mode/n_bits가 적용되어야 shall.
- finite-precision 타입들의 **공통 base class는 `sc_fxnum`**. 애플리케이션도 구현도 `sc_fxnum` 타입 객체를 직접 생성하면 **안 된다 shall not**. `sc_fxnum`의 참조/포인터로 파생 타입 객체에 접근해도 된다 may.
- `sc_fxval`은 variable-precision 타입. 임의 폭·임의 binary point 위치의 값을 저장해도 된다 may. 대입된 값은 **정밀도·크기 손실 없이** 저장되어야 shall (quantization·overflow로 값이 변경되면 안 된다 shall not).
- `sc_fixed`, `sc_fix`, `sc_fxval`은 **signed(2의 보수)** 표현이어야 shall. `sc_ufixed`, `sc_ufix`는 unsigned 표현.
- 초기값 없이 선언된 fixed-point 변수는 **uninitialized**여야 shall. uninitialized 변수는 initialized 변수가 허용되는 곳 어디서나 사용해도 된다 may. **uninitialized 변수에 대한 연산 결과는 undefined**.

#### 7.10.4.3 Limited-precision fixed-point types

```cpp
sc_fixed_fast<wl,iwl,q_mode,o_mode,n_bits>
sc_ufixed_fast<wl,iwl,q_mode,o_mode,n_bits>
sc_fix_fast
sc_ufix_fast
sc_fxval_fast
```

- limited-precision 타입은 finite-precision 타입과 **동일한 semantics**를 사용해야 shall. 표현식에서 둘을 자유롭게 섞어도 된다 may.
- limited-precision 변수는 대응하는 finite-precision 변수가 기대되는 어떤 표현식에서도 **합법적 대체물**이어야 shall.
- limited-precision 값은 **implementation-dependent한 네이티브 C++ 부동소수점 타입**에 보관되어야 shall. 구현은 mantissa 표현에 **최소 53비트**를 제공해야 shall.

> NOTE(informative) — limited-precision 타입에서 bit-true 동작을 얻으려면 연산·표현식 결과의 word length가 **53비트를 초과하지 않아야 한다(should not)**.

### 7.10.5 Fixed-point expressions and operations

fixed-point 연산은 **variable-precision 값으로** 수행되어야 shall. 즉 연산자 평가는(아래 특수 연산자 제외):

- 피연산자들이 variable-precision fixed-point 값으로 변환(승격)되어야 shall.
- 연산이 수행되어 variable-precision 결과를 계산해야 shall. 결과는 **정밀도·크기 손실이 없도록**(정확히 표현할 만큼 충분한 비트로) 계산되어야 shall.

- fixed-point 대입의 우변은 variable-precision 값으로 평가된 뒤 좌변이 지정한 표현으로 변환되어야 shall.
- 모든 피연산자가 limited-precision 타입이면 **limited-precision 연산**이 수행되어야 shall. 이 연산은 `sc_fxval_fast`를 사용하며 결과도 limited variable-precision 값이어야 shall.
- shift 연산의 **우변(shift 양)은 `int` 타입이어야 shall**. fixed-point 값이 우변으로 주어지면 소수부는 **truncate**되어야 shall (quantization 없음).
- equality·relational 연산자 결과는 `bool` 타입이어야 shall.
- **bitwise 연산자의 fixed-point 피연산자는 finite- 또는 limited-precision 타입이어야 shall** (variable-precision이면 안 된다 shall not). 또한 이항 bitwise 연산자의 두 피연산자는 **같은 부호 표현**(둘 다 signed 또는 둘 다 unsigned)이어야 shall. 결과는 피연산자 부호 표현에 따라 `sc_fix`/`sc_ufix`(또는 `sc_fix_fast`/`sc_ufix_fast`)여야 shall. 이항 연산자에서 두 피연산자는 binary point 기준으로 정렬되어야 shall. 필요하면 임시로 확장해 iwl·fwl을 같게 만들고, 결과는 그 확장된 피연산자와 같은 iwl·fwl을 가져야 shall.
- **remainder 연산자 `%`는 fixed-point 타입에서 지원되지 않는다.**

**Table 33 — 허용되는 산술·bitwise 연산** (A = fixed-point 객체, B/C = 적절한 수치 값·객체, s* = signed finite/limited 객체, u* = unsigned finite/limited 객체)

| 표현식 | 연산 |
|---|---|
| `A = B + C;` | 덧셈 후 대입 |
| `A = B - C;` | 뺄셈 후 대입 |
| `A = B * C;` | 곱셈 후 대입 |
| `A = B / C;` | 나눗셈 후 대입 |
| `A = B << i;` | 좌시프트 후 대입 |
| `A = B >> i;` | 우시프트 후 대입 |
| `s1 = s2 & s3;` | signed bitwise and 후 대입 |
| `s1 = s2 \| s3;` | signed bitwise or 후 대입 |
| `s1 = s2 ^ s3;` | signed bitwise xor 후 대입 |
| `u1 = u2 & u3;` | unsigned bitwise and 후 대입 |
| `u1 = u2 \| u3;` | unsigned bitwise or 후 대입 |
| `u1 = u2 ^ u3;` | unsigned bitwise xor 후 대입 |

**피연산자 조합** — 아래 4개 표의 형태가 허용된다. 뺄셈·곱셈·나눗셈도 **동일한 조합**으로 허용된다.

| 표 | 대상 | 허용 표현식 | `n`으로 올 수 있는 타입 |
|---|---|---|---|
| Table 34 | finite-precision (`F` = `sc_fxnum` 파생) | `F = F1 + F2;` `F1 += F2;` `F1 = F2 + n;` `F1 = n + F2;` `F += n;` | `int, long, unsigned int, unsigned long, float, double, sc_signed, sc_unsigned, sc_int_base, sc_uint_base, sc_fxval, sc_fxval_fast`, `sc_fxnum_fast` 파생 객체, 수치 문자열 리터럴 |
| Table 35 | variable-precision (`V` = `sc_fxval`) | `V = V1 + V2;` `V1 += V2;` `V1 = V2 + n;` `V1 = n + V2;` `V += n;` | 위와 같되 `sc_fxval` 제외, `sc_fxval_fast` 및 `sc_fxnum_fast` 파생 포함 |
| Table 36 | limited-precision (`F` = `sc_fxnum_fast` 파생) | 동일 5가지 | `int, long, unsigned int, unsigned long, float, double, sc_signed, sc_unsigned, sc_int_base, sc_uint_base, sc_fxval_fast`, 수치 문자열 리터럴 |
| Table 37 | limited variable-precision (`V` = `sc_fxval_fast`) | 동일 5가지 | `int, long, unsigned int, unsigned long, float, double, sc_signed, sc_unsigned, sc_int_base, sc_uint_base`, 수치 문자열 리터럴 |

피연산자는 위 타입들에서 **파생된 다른 클래스**여도 된다 may.

### 7.10.6 Bit and part selection

- fixed-point 타입에 대해 bit·part selection이 지원되어야 shall (7.2.6, 7.2.7 참조).
- **variable-precision 타입 `sc_fxval`·`sc_fxval_fast`에는 지원되지 않는다.**
- part-select의 좌측 인덱스가 우측 인덱스보다 작으면 part-select의 **비트 순서가 역전되어야 shall**.
- part-select는 **범위 미지정**으로 생성해도 된다(`range` 또는 `operator()`를 인자 없이 호출). 이 경우 part-select는 연관된 fixed-point 객체와 **같은 word length, 같은 값**을 가져야 shall.

### 7.10.7 Variable-precision fixed-point value limits

- 나눗셈 같은 경우 variable precision이 무한 word length로 이어질 수 있다. 구현은 variable-precision 값의 **최대 허용 word length를 정의하고 그 한계 도달을 검출하는 적절한 메커니즘을 제공하는 것이 권장(should)**된다.
- variable-precision 값이 최대 word length에 도달했을 때 구현이 취하는 **동작은 undefined**.
- variable-precision 값이 최대 word length에 도달하게 만든 연산의 결과는 **이상적(무한 정밀도) 결과에 가장 가까운 implementation-dependent 표현 가능 값**이어야 shall.

### 7.10.8 Fixed-point word length and mode

```cpp
namespace sc_dt {
   enum { SC_ON, SC_OFF };
}
```

fixed-point 타입의 기본 word length·quantization mode·saturation mode는 **생성 시점에 context에 있는 fixed-point type parameter(`sc_fxtype_params`)로 설정되어야 shall** (7.2.4 참조). 이 파라미터는 (wl, iwl)과 (q_mode, o_mode, n_bits)에 대응하는 필드를 가져야 shall.

**Table 38 — 내장 기본값**

| Parameter | Value |
|---|---|
| wl | 32 |
| iwl | 32 |
| q_mode | `SC_TRN` |
| o_mode | `SC_WRAP` |
| n_bits | 0 |

**floating-point cast switch**:

- 산술 연산에서 fixed-point 객체가 부동소수점 변수처럼 동작하도록, **생성 시점에 context에 있는 floating-point cast switch**로 설정해도 된다 may.
- floating-point cast switch는 **floating-point cast context 객체를 생성**해 context에 들어와야 shall. `sc_fxcast_switch`(7.11.6)와 `sc_fxcast_context`(7.11.7)가 각각 switch와 context를 만든다.
- **전역 floating-point cast context stack**이 7.2.4의 length context stack과 **동일한 semantics**로 cast context들을 관리해야 shall.
- cast switch는 `SC_ON` 또는 `SC_OFF`로 초기화해도 된다 may. 각각 산술 동작을 **fixed-point** / **floating-point**로 만들어야 shall.
- **기본 floating-point context는 값 `SC_ON`으로 정의되어야 shall.**

```cpp
sc_dt::sc_fxtype_params fxt(32, 16);
sc_dt::sc_fxtype_context fcxt(fxt);
sc_dt::sc_fix A, B, res;                    // wl = 32, iwl = 16
A = 10.0;
B = 0.1;
res = A * B;                                // res = .999908447265625

sc_dt::sc_fxcast_switch fxs(sc_dt::SC_OFF);
sc_dt::sc_fxcast_context fccxt(fxs);
sc_dt::sc_fix C, D;                         // Floating-point behavior
C = 10.0;
D = 0.1;
res = C * D;                                // res = 1
```

#### 7.10.8.2 Reading parameter settings

모든 finite-precision·limited-precision fixed-point 객체에 대해 정의되며 **런타임에** 현재 파라미터 설정을 반환해야 shall.

| 함수 | 반환 |
|---|---|
| `const sc_fxcast_switch& cast_switch() const;` | cast switch 파라미터 |
| `int iwl() const;` | integer word-length 파라미터 |
| `int n_bits() const;` | saturated bit 수 파라미터 |
| `sc_o_mode o_mode() const;` | overflow mode 파라미터 |
| `sc_q_mode q_mode() const;` | quantization mode 파라미터 |
| `const sc_fxtype_params& type_params() const;` | 타입 파라미터 |
| `int wl() const;` | 전체 word-length 파라미터 |

```cpp
enum sc_o_mode
{
   SC_SAT,          // Saturation
   SC_SAT_ZERO,     // Saturation to zero
   SC_SAT_SYM,      // Symmetrical saturation
   SC_WRAP,         // Wrap-around (*)
   SC_WRAP_SM       // Sign magnitude wrap-around (*)
};

enum sc_q_mode
{
   SC_RND,          // Rounding to plus infinity
   SC_RND_ZERO,     // Rounding to zero
   SC_RND_MIN_INF,  // Rounding to minus infinity
   SC_RND_INF,      // Rounding to infinity
   SC_RND_CONV,     // Convergent rounding
   SC_TRN,          // Truncation
   SC_TRN_ZERO      // Truncation to zero
};
```

#### 7.10.8.3 Value attributes

모든 fixed-point 객체에 대해 정의되며 현재 값 속성을 반환해야 shall.

| 함수 | 규칙 |
|---|---|
| `bool is_neg() const;` | 음수 값을 담고 있으면 true, 아니면 false |
| `bool is_zero() const;` | 0 값을 담고 있으면 true, 아니면 false |
| `bool overflow_flag() const;` | 이 객체에 대한 **마지막 write 동작**이 overflow를 일으켰으면 true |
| `bool quantization_flag() const;` | 이 객체에 대한 **마지막 write 동작**이 quantization을 일으켰으면 true |

현재 값 반환:

- finite-precision 객체: `sc_fxval value() const;`
- limited-precision 객체: `sc_fxval_fast value() const;`

### 7.10.9 Conversions to character string

#### 7.10.9.1 Overview

- fixed-point 타입의 문자열 변환은 멤버 함수 `to_string`으로 지원되어야 shall (7.3 참조).
- `to_string`은 문자열 형식을 지정하는 **추가 인자**를 받아도 된다 may. 이 인자는 `sc_fmt` 열거형이어야 하며 **항상 인자 리스트의 맨 오른쪽**이어야 shall.

```cpp
enum sc_fmt { SC_F, SC_E };
```

- fmt 기본값: finite-·limited-precision 타입은 **`SC_F`**, `sc_fxval`·`sc_fxval_fast`는 **`SC_E`**여야 shall.
- 선택한 형식은 **binary point가 wl 비트 안에 있지 않을 때에만** 다른 문자열을 만들어야 shall. 그 경우 부호확장(MSB 측) 또는 0확장(LSB 측)을 하거나(`SC_F`), 지수를 사용해야 shall(`SC_E`).
- `SC_DEC` 표현으로 변환하거나 variable-precision 변수에서 변환할 때는 **값을 유일하게 표현하는 데 필요한 문자만** 생성되어야 shall.
- finite-·limited-precision 변수를 binary/octal/hex로 변환할 때 문자 수는 변수의 **(iwl, fwl)로 결정되어야 shall** (필요 시 부호·0 확장).

```cpp
sc_dt::sc_fixed<7,4> a = –1.5;
a.to_string(sc_dt::SC_DEC);                        // –1.5
a.to_string(sc_dt::SC_BIN);                        // 0b1110.100
sc_dt::sc_fxval b(–1.5);
b.to_string(sc_dt::SC_BIN);                        // 0b10.1
sc_dt::sc_fixed<4,6> c = 20;
c.to_string(sc_dt::SC_BIN, false, sc_dt::SC_F);    // 010100
c.to_string(sc_dt::SC_BIN, false, sc_dt::SC_E);    // 0101e+2
```

#### 7.10.9.2 String shortcut member functions

자주 쓰이는 인자 조합용 단축 멤버 함수 4개가 제공되어야 shall. 단축 함수는 **기본 문자열 형식**을 사용해야 shall.

**Table 39**

| 단축 멤버 함수 | 수 표현 |
|---|---|
| `to_dec()` | `SC_DEC` |
| `to_bin()` | `SC_BIN` |
| `to_oct()` | `SC_OCT` |
| `to_hex()` | `SC_HEX` |

```cpp
sc_dt::sc_fixed<4,2> a = –1;
a.to_dec();      // std::string "–1" 반환
a.to_bin();      // std::string "0b11.00" 반환
```

#### 7.10.9.3 Bit-pattern string conversion

- bit-pattern 문자열을 **fixed-point part-select에 대입해도 된다 may**.
- **part-select를 거치지 않고** fixed-point 객체에 bit-pattern 문자열을 대입한 결과는 **undefined**.
- 문자열의 문자 수가 part-select word length보다 적으면, **좌측을 0확장**해 part-select word length에 맞춰야 shall.

### 7.10.10 Finite word-length effects

#### 7.10.10.2 Overflow modes

- 값의 크기가 fixed-point 표현을 초과해 limited-precision 변수에 대입될 때 overflow가 발생해야 shall.
- 아래 **상호 배타적** overflow mode들이 제공되어야 shall. **기본 overflow mode는 `SC_WRAP`**.
- wrap-around 계열 모드를 쓸 때 saturated bit 수(`n_bits`)는 **기본적으로 0**으로 설정되어야 shall(변경 가능).

**Table 40 — Overflow modes**

| Overflow mode | Name |
|---|---|
| Saturation | `SC_SAT` |
| Saturation to zero | `SC_SAT_ZERO` |
| Symmetrical saturation | `SC_SAT_SYM` |
| Wrap-around ᵃ | `SC_WRAP` |
| Sign magnitude wrap-around ᵃ | `SC_WRAP_SM` |

ᵃ saturated bit 0개 또는 n_bits개(n_bits > 0). n_bits 기본값은 0.

MIN/MAX 정의:

- signed: `–2^(n–1) (= MIN) <= x <= (2^(n–1) – 1) (= MAX)`, n = 비트 수
- unsigned: `MIN = 0`, `MAX = 2^n – 1`

#### 7.10.10.3 Overflow for signed fixed-point numbers

```
Before:  x   x   x   x   x    x     x      x  x  x  x  x  x  x  x  x  x
After:                        x     x      x  x  x  x  x  x  x  x  x  x
Flags:   sD  D   D   D   lD   sR  R(N)  R(IN) R  R  R  R  R  R  R  R  lR
```

플래그: `x` 이진 숫자 / `sD` overflow 처리 전 부호 비트 / `D` 삭제되는 비트 / `lD` 최하위 삭제 비트 / `sR` 결과 수의 MSB 위치 비트(`SC_WRAP_SM,0`·`SC_WRAP_SM,1`에서는 원래 값 `sRo`와 새 값 `sRn`을 구분) / `N` saturated bit, 개수는 `n_bits - 1`, 항상 결과의 부호 비트 다음에 온다. **`n_bits`는 `SC_WRAP`과 `SC_WRAP_SM`에서만 고려된다** / `lN` 최하위 saturated bit — `SC_WRAP`·`SC_WRAP_SM`에서만 유효하고 다른 모드에서는 R 비트로 취급. `SC_WRAP_SM, n_bits>1`에서 `lNo`는 이 비트의 원래 값 / `R` 나머지 비트 / `lR` 최하위 나머지 비트.

**Table 41 — signed fixed-point overflow 처리** (`!` = bitwise 부정, `^` = bitwise XOR)

| Overflow mode | Sign bit (sR) | Saturated bits (N, lN) | Remaining bits (R, lR) | 설명 |
|---|---|---|---|---|
| `SC_SAT` | `sD` | — | `!sD` | 결과는 원래 수의 부호 비트를 갖고, 나머지 비트는 부호 비트의 역값을 가져야 shall |
| `SC_SAT_ZERO` | `0` | — | `0` | 모든 비트가 0으로 설정되어야 shall |
| `SC_SAT_SYM` | `sD` | — | `!sD`, | 결과는 원래 부호 비트를 갖고, 나머지 비트는 부호 비트의 역값을 갖되 **최하위 나머지 비트는 1**로 설정되어야 shall |
| `SC_WRAP`, n_bits=0 | `sR` | — | `x` | 삭제 비트를 제외한 모든 비트를 결과로 복사해야 shall |
| `SC_WRAP`, n_bits=1 | `sD` | — | `x` | 결과는 원래 부호 비트를 갖고, 나머지 비트는 원래 수에서 복사되어야 shall |
| `SC_WRAP`, n_bits>1 | `sD` | `!sD` | `x` | 결과는 원래 부호 비트, saturated bit는 원래 부호 비트의 역값, 나머지 비트는 원래 수에서 복사 |
| `SC_WRAP_SM`, n_bits=0 | `lD` | — | `x ^ sRo ^ sRn` | 결과 부호 비트는 **최하위 삭제 비트 값**을 가져야 shall. 나머지 비트는 결과 부호 비트의 원래 값·새 값과 XOR |
| `SC_WRAP_SM`, n_bits=1 | `sD` | — | `x ^ sRo ^ sRn` | 결과는 원래 부호 비트. 나머지 비트는 결과 부호 비트의 원래 값·새 값과 XOR |
| `SC_WRAP_SM`, n_bits>1 | `sD` | `!sD` | `x ^ lNo ^ !sD` | 결과는 원래 부호 비트, saturated bit는 원래 부호 비트의 역값, 나머지 비트는 최하위 saturated bit의 원래 값 및 원래 부호 비트의 역값과 XOR |

**overflow 발생 조건** — 삭제 비트(`sD`, `D`, `lD`) 중 적어도 하나의 값이 결과 MSB 위치 비트의 원래 값(`sRo`)과 다를 때 overflow가 발생해야 shall.

#### 7.10.10.4 Overflow for unsigned fixed-point numbers

```
Before:  x  x  x  x  x     x     x     x   x  x  x  x  x  x  x  x  x
After:                     x     x     x   x  x  x  x  x  x  x  x  x
Flags:   D  D  D  D  lD  R(N)  R(N) R(IN)  R  R  R  R  R  R  R  R  R
```

플래그: `D` 삭제 비트 / `lD` 최하위 삭제 비트 / `N` saturated bit, 개수는 **`n_bits`와 같음**, `SC_WRAP`·`SC_WRAP_SM`에서만 고려 / `R` 나머지 비트.

**Table 42 — unsigned fixed-point overflow 처리**

| Overflow mode | Saturated bits (N) | Remaining bits (R) | 설명 |
|---|---|---|---|
| `SC_SAT` | — | `1`(overflow) / `0`(underflow) | 나머지 비트를 1(overflow) 또는 0(underflow)으로 설정해야 shall |
| `SC_SAT_ZERO` | — | `0` | 나머지 비트를 0으로 설정해야 shall |
| `SC_SAT_SYM` | — | `1`(overflow) / `0`(underflow) | 나머지 비트를 1(overflow) 또는 0(underflow)으로 설정해야 shall |
| `SC_WRAP`, n_bits=0 | — | `x` | 삭제 비트를 제외한 모든 비트를 결과로 복사해야 shall |
| `SC_WRAP`, n_bits>0 | `1` | `x` | saturated bit를 1로 설정하고 나머지 비트는 결과로 복사해야 shall |
| `SC_WRAP_SM` | **unsigned에 대해 정의되지 않음 (Not defined)** | | |

- **signed → unsigned** 변환 시에는 **sign extension이 overflow 처리보다 먼저** 일어나야 shall.
- **unsigned → signed** 변환 시에는 **zero extension이 먼저** 일어나야 shall.

#### 7.10.10.5 SC_SAT

overflow 시 출력이 **MAX로 포화**, 음의 overflow 시 **MIN으로 포화**됨을 나타내는 데 쓰여야 shall.

> 예 (signed, 3비트): `0110 (6)` → `011 (3)`; `1011 (–5)` → `100 (–4)`
> 예 (unsigned, 3비트): `01110 (14)` → `111 (7)`. `SC_SAT` 모드는 saturate할 비트 수가 유지 비트 수와 같은 `SC_WRAP`·`SC_WRAP_SM` 모드에 대응한다.

#### 7.10.10.6 SC_SAT_ZERO

overflow 시(MAX 또는 MIN 초과 시) 출력이 **0으로 강제**됨을 나타내는 데 쓰여야 shall.

> 예 (signed, 3비트): `0110 (6)` → `000 (0)`; `1011 (–5)` → `000 (0)`
> 예 (unsigned, 3비트): `01110 (14)` → `000 (0)`

#### 7.10.10.7 SC_SAT_SYM

overflow 시 **MAX로 포화**, 음의 overflow 시 **–MAX(signed) 또는 MIN(unsigned)로 포화**됨을 나타내는 데 쓰여야 shall.

> 예 (signed, 3비트): `0110 (6)` → `011 (3)`; `1011 (–5)` → `101 (–3)`
> 예 (unsigned, 3비트): `01110 (14)` → `111 (7)`

#### 7.10.10.8 SC_WRAP

overflow 시 출력이 **wrap-around** 됨을 나타내는 데 쓰여야 shall. 두 경우가 있다: `n_bits = 0`, `n_bits > 0`.

**`SC_WRAP, 0`** — 이것이 **기본 overflow mode**여야 shall. 삭제 비트를 제외한 모든 비트를 결과로 복사해야 shall.

> 예 (signed, 3비트): `0100 (4)` → `100 (–4)`; `1011 (–5)` → `011 (3)`
> 예 (unsigned, 3비트): `11011 (27)` → `011 (3)`

**`SC_WRAP, n_bits > 0`** — `n_bits`가 0보다 크면 결과 MSB 측 지정 비트 수만큼이 **원래 부호를 보존한 채 포화**되어야 shall. 나머지 비트는 원본에서 복사. **양수는 양수로, 음수는 음수로 유지되어야 shall.**

> 예 (signed, 3비트, n_bits=1): `0101 (5)` → `001 (1)`; `1011 (–5)` → `111 (–1)`
> 예 (unsigned, 5비트, n_bits=3): `0110010 (50)` → `11110 (30)` — MSB 측 3비트를 1로 설정, 나머지 복사

#### 7.10.10.9 SC_WRAP_SM

overflow 시 출력이 **sign-magnitude wrap-around** 됨을 나타내는 데 쓰여야 shall. `n_bits`는 원래 부호를 보존한 채 포화되는 MSB 측 비트 수를 나타내야 shall.

**`SC_WRAP_SM, 0`** — 필요 word length 바깥의 MSB들이 삭제되어야 shall. 결과의 부호 비트는 **삭제 비트 중 최하위 비트의 값**을 가져야 shall. 유지되는 비트 중 최상위 비트의 **원래 값과 새 값이 다르면** 나머지 비트들은 반전되어야 shall. 다르지 않으면 원본에서 결과로 복사되어야 shall.

> 예: `0100 (4)` → 3비트로 truncate → `100 (–4)` → 새 부호 비트는 최하위 삭제 비트 값인 0 → 원래 값과 새 값이 다르므로 나머지 비트 반전 → `011 (3)`

**Table 43 — `SC_WRAP_SM, n_bits=0`, 3비트**

| Decimal | Binary |
|---|---|
| 8 | 111 |
| 7 | 000 |
| 6 | 001 |
| 5 | 010 |
| 4 | 011 |
| 3 | 011 |
| 2 | 010 |
| 1 | 001 |
| 0 | 000 |
| –1 | 111 |
| –2 | 110 |
| –3 | 101 |
| –4 | 100 |
| –5 | 100 |
| –6 | 101 |
| –7 | 110 |

**`SC_WRAP_SM, n_bits > 0`** — 결과 MSB 측 첫 `n_bits` 비트는:

- 양수면 **MAX로 포화**
- 음수면 **MIN으로 포화**

모든 수가 **부호를 유지해야 shall**.

- `n_bits == 1`이면 나머지 비트는 복사된 뒤 결과 부호 비트의 **원래 값·새 값과 XOR**되어야 shall.
- `n_bits > 1`이면 나머지 비트는 **최하위 saturated bit의 원래 값** 및 **원래 부호 비트의 역값**과 XOR되어야 shall.

> 예 `SC_WRAP_SM, 3`: 234를 5비트로 캐스트. `011101010 (234)` → 5비트로 truncate → 원래 부호 비트를 새 MSB(bit 4)로 복사 → bit 2·3·4를 부호 비트를 바꾸지 않는 3비트 최대값으로 포화 → bit 2의 원래 값 0과 원래 부호 비트의 역값 1을 LSB 측 나머지 비트(10)와 XOR → `01101 (13)`
> 예 `SC_WRAP_SM, 1`: 12를 3비트로. `01100 (12)` → 3비트로 truncate → 원래 부호 비트를 새 MSB(bit 2)로 복사 → LSB 측 2비트를 새 부호 비트의 원래 값(1)·새 값(0)과 XOR

**Table 44 — `SC_WRAP_SM, n_bits=1`, 3비트**

| Decimal | Binary |
|---|---|
| 9 | 001 |
| 8 | 000 |
| 7 | 000 |
| 6 | 001 |
| 5 | 010 |
| 4 | 011 |
| 3 | 011 |
| 2 | 010 |
| 1 | 001 |
| 0 | 000 |
| –1 | 111 |
| –2 | 110 |
| –3 | 101 |
| –4 | 100 |
| –5 | 100 |
| –6 | 101 |
| –7 | 110 |
| –8 | 111 |
| –9 | 111 |

#### 7.10.10.10 Quantization modes

fixed-point 변수에 대입되는 값의 정밀도가 변수 정밀도를 초과할 때 quantization이 적용되어야 shall. 아래 **상호 배타적** 모드가 제공되어야 shall. **기본 quantization mode는 `SC_TRN`.**

**Table 45 — Quantization modes**

| Quantization mode | Name |
|---|---|
| Rounding to plus infinity | `SC_RND` |
| Rounding to zero | `SC_RND_ZERO` |
| Rounding to minus infinity | `SC_RND_MIN_INF` |
| Rounding to infinity | `SC_RND_INF` |
| Convergent rounding | `SC_RND_CONV` |
| Truncation | `SC_TRN` |
| Truncation to zero | `SC_TRN_ZERO` |

핵심 규칙:

- 값이 정확히 표현 가능하면 quantization은 **값을 바꾸면 안 된다 shall not**.
- 모든 rounding 모드는 값을 **가장 가까운 표현 가능한 값**으로 매핑해야 shall. 가장 가까운 값이 둘(정확히 중간)일 때 각 rounding 모드가 서로 다른 선택 기준을 제공해야 shall.
- 두 truncate 모드는 **양수**를 그 값보다 **작은 가장 가까운 표현 가능 값**으로 매핑해야 shall.
- `SC_TRN`은 **음수**를 그 값보다 **작은** 가장 가까운 값으로, `SC_TRN_ZERO`는 음수를 그 값보다 **큰** 가장 가까운 값으로 매핑해야 shall.

#### 7.10.10.11 Quantization for signed fixed-point numbers

```
Before:  x   x  x  x  x  x  x  x   x   x   x  x  x  x  x
After:   x   x  x  x  x  x  x  x   x
Flags:   sR  R  R  R  R  R  R  R  lR  mD   D  D  D  D  D
```

플래그: `sR` 부호 비트 / `R` 나머지 비트 / `lR` 최하위 나머지 비트 / `mD` 최상위 삭제 비트 / `D` 삭제 비트 / `r` = **`mD`를 제외한 삭제 비트들의 논리 OR**. 나머지가 없으면 `r`은 false — 즉 **가장 가까운 두 수가 등거리일 때 `r`은 false**.

가장 가까운 두 표현 가능 수가 **등거리가 아니면** 결과는 가장 가까운 표현 가능 수여야 shall. 이는 `SC_RND` 모드를 적용해(최상위 삭제 비트를 나머지 비트에 더해) 구해야 shall.

**Table 46 — signed quantization 처리** (2열 = 나머지 비트에 더할 식, 1 또는 0으로 평가. `!` 부정, `|` OR, `&` AND)

| Quantization mode | 더할 식 | 설명 |
|---|---|---|
| `SC_RND` | `mD` | 최상위 삭제 비트를 나머지 비트에 더한다 |
| `SC_RND_ZERO` | `mD & (sR \| r)` | 최상위 삭제 비트가 1이고, 부호 비트나 다른 삭제 비트 중 하나가 1이면 나머지 비트에 1을 더한다 |
| `SC_RND_MIN_INF` | `mD & r` | 최상위 삭제 비트가 1이고 다른 삭제 비트 중 하나가 1이면 1을 더한다 |
| `SC_RND_INF` | `mD & (!sR \| r)` | 최상위 삭제 비트가 1이고, 부호 비트의 역값이나 다른 삭제 비트 중 하나가 1이면 1을 더한다 |
| `SC_RND_CONV` | `mD & (lR \| r)` | 최상위 삭제 비트가 1이고, 최하위 나머지 비트나 다른 삭제 비트 중 하나가 1이면 1을 더한다 |
| `SC_TRN` | `0` | 나머지 비트를 그대로 복사 |
| `SC_TRN_ZERO` | `sR & (mD \| r)` | 부호 비트가 1이고, 최상위 삭제 비트나 다른 삭제 비트 중 하나가 1이면 1을 더한다 |

#### 7.10.10.12 Quantization for unsigned fixed-point numbers

```
Before:  x  x  x  x  x  x  x  x   x   x   x  x  x  x  x
After:   x  x  x  x  x  x  x  x   x
Flags:   R  R  R  R  R  R  R  R  lR  mD   D  D  D  D  D
```

플래그 의미는 7.10.10.11과 동일(부호 비트 없음).

**Table 47 — unsigned quantization 처리**

| Quantization mode | 더할 식 | 설명 |
|---|---|---|
| `SC_RND` | `mD` | 최상위 삭제 비트를 남은 비트에 더한다 |
| `SC_RND_ZERO` | `0` | 나머지 비트를 그대로 복사 |
| `SC_RND_MIN_INF` | `0` | 나머지 비트를 그대로 복사 |
| `SC_RND_INF` | `mD` | 최상위 삭제 비트를 남은 비트에 더한다 |
| `SC_RND_CONV` | `mD & (lR \| r)` | 최상위 삭제 비트가 1이고, 최하위 나머지 비트나 다른 삭제 비트 중 하나가 1이면 1을 더한다 |
| `SC_TRN` | `0` | 나머지 비트를 그대로 복사 |
| `SC_TRN_ZERO` | `0` | 나머지 비트를 그대로 복사 |

등거리가 아닐 때 모든 rounding 모드는 `SC_RND`를 적용해 가장 가까운 수를 구해야 shall.

> NOTE(informative) — 모든 rounding 모드에서 overflow가 발생할 수 있다. 결과를 완전한 정밀도로 표현하려면 MSB 측에 **1비트가 추가로** 필요하다.

#### 7.10.10.13 SC_RND

최상위 삭제 LSB를 나머지 비트에 더해 **가장 가까운 표현 가능 수로 반올림**되어야 shall. 이 규칙은 등거리가 아닌 모든 rounding 모드에 사용되어야 shall. 등거리일 때 이 규칙은 **plus infinity 방향 반올림**을 의미한다. (그림의 `q`는 quantization step = 데이터 타입의 해상도)

> 예 (signed): `sc_fixed<4,2>` → `sc_fixed<3,2,SC_RND>`
> `(1.25)` → `01.1 (1.5)`; `10.11 (–1.25)` → `11.0 (–1)`
> 예 (unsigned): `sc_ufixed<16,8>` → `sc_ufixed<12,8,SC_RND>`
> `00100110.01001111 (38.30859375)` → `00100110.0101 (38.3125)`

#### 7.10.10.14 SC_RND_ZERO

등거리가 아니면 `SC_RND_ZERO` 모드가 적용되어야 shall. 등거리이면 출력이 **0 방향으로** 반올림되어야 shall. 양수는 LSB 측 잉여 비트를 삭제하고, 음수는 최상위 삭제 LSB를 나머지 비트에 더해야 shall.

> 예 (signed): `sc_fixed<4,2>` → `sc_fixed<3,2,SC_RND_ZERO>`
> `(1.25)` → `01.0 (1)`; `10.11 (–1.25)` → `11.0 (–1)`
> 예 (unsigned): `sc_ufixed<16,8>` → `sc_ufixed<12,8,SC_RND_ZERO>`
> `000100110.01001 (38.28125)` → `000100110.0100 (38.25)`

#### 7.10.10.15 SC_RND_MIN_INF

등거리가 아니면 `SC_RND_MIN_INF` 모드가 적용되어야 shall. 등거리이면 LSB 측 잉여 비트를 생략해 **minus infinity 방향으로** 반올림되어야 shall.

> 예 (signed): `sc_fixed<4,2>` → `sc_fixed<3,2,SC_RND_MIN_INF>`
> `01.01 (1.25)` → `01.0 (1)`; `10.11 (–1.25)` → `10.1 (–1.5)`
> 예 (unsigned): `sc_ufixed<16,8>` → `sc_ufixed<12,8,SC_RND_MIN_INF>`
> `000100110.01001 (38.28125)` → `000100110.0100 (38.25)`

#### 7.10.10.16 SC_RND_INF

가장 가까운 두 표현 가능 수가 **등거리일 때** 반올림이 수행되어야 shall.

- **양수**: 나머지 비트의 LSB가 1이면 plus infinity 방향, 0이면 minus infinity 방향으로 반올림되어야 shall.
- **음수**: 나머지 비트의 LSB가 1이면 minus infinity 방향, 0이면 plus infinity 방향으로 반올림되어야 shall.

> 예 (signed): `sc_fixed<4,2>` → `sc_fixed<3,2,SC_RND_INF>`
> `01.01 (1.25)` → `01.1 (1.5)`; `10.11 (–1.25)` → `10.1 (–1.5)`
> 예 (unsigned): `sc_ufixed<16,8>` → `sc_ufixed<12,8,SC_RND_INF>`
> `000100110.01001 (38.28125)` → `000100110.0101 (38.3125)`

#### 7.10.10.17 SC_RND_CONV

등거리가 아니면 `SC_RND_CONV` 모드가 적용되어야 shall. 등거리이면 나머지 비트의 LSB가 **1이면 plus infinity 방향**, **0이면 minus infinity 방향**으로 반올림되어야 shall.

> 예 (signed): `sc_fixed<4,2>` → `sc_fixed<3,2,SC_RND_CONV>`
> `00.11 (0.75)` → `01.0 (1)`; `10.11 (–1.25)` → `11.0 (–1)`
> 예 (unsigned): `sc_ufixed<16,8>` → `sc_ufixed<12,8,SC_RND_CONV>`
> `000100110.01001 (38.28125)` → `000100110.0100 (38.25)`
> `000100110.01011 (38.34375)` → `000100110.0110 (38.375)`

#### 7.10.10.18 SC_TRN

**`SC_TRN`이 기본 quantization mode여야 shall.** 결과는 **minus infinity 방향으로** 반올림되어야 shall 즉 LSB 측 불필요 비트가 삭제되어야 shall. quantize된 수는 요구 비트 범위 안에서 **원래 값보다 작은 첫 표현 가능 수**로 근사되어야 shall.

> NOTE(informative) — 학술 문헌에서 보통 "value truncation"으로 불린다.

> 예 (signed): `sc_fixed<4,2>` → `sc_fixed<3,2,SC_TRN>`
> `01.01 (1.25)` → `01.0 (1)`; `10.11 (–1.25)` → `10.1 (–1.5)`
> 예 (unsigned): `sc_ufixed<16,8>` → `sc_ufixed<12,8,SC_TRN>`
> `00100110.01001111 (38.30859375)` → `00100110.0100 (38.25)`

#### 7.10.10.19 SC_TRN_ZERO

- **양수**에 대해서는 `SC_TRN`과 동일해야 shall.
- **음수**에 대해서는 결과가 0 방향으로 반올림되어야 shall(`SC_RND_ZERO`) 즉 우측 잉여 비트를 삭제하고, **삭제된 비트 중 최소 하나가 0이 아니면** 좌측 LSB들에 부호 비트를 더해야 shall.
- quantize된 수는 **절댓값이 더 작은 첫 표현 가능 수**로 근사되어야 shall.

> NOTE(informative) — 학술 문헌에서 보통 "magnitude truncation"으로 불린다.

> 예 (signed): `sc_fixed<4,2>` → `sc_fixed<3,2,SC_TRN_ZERO>`
> `10.11 (–1.25)` → `11.0 (–1)`
> 예 (unsigned): `sc_ufixed<16,8>` → `sc_ufixed<12,8,SC_TRN_ZERO>`
> `00100110.01001111 (38.30859375)` → `00100110.0100 (38.25)`

### 7.10.11 sc_fxnum

#### Description

`sc_fxnum`은 **finite-precision fixed-point 타입의 base class**. 파생 클래스 모두에 동작하는 함수·오버로드 연산자를 정의하기 위해 제공되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

class sc_fxnum
{
    friend class sc_fxval;
    friend class sc_fxnum_bitref_r;
    friend class sc_fxnum_bitref†;
    friend class sc_fxnum_subref_r;
    friend class sc_fxnum_subref†;
    friend class sc_fxnum_fast_bitref_r;
    friend class sc_fxnum_fast_bitref†;
    friend class sc_fxnum_fast_subref_r;
    friend class sc_fxnum_fast_subref†;

    public:
        // Unary operators
        const sc_fxval operator- () const;
        const sc_fxval operator+ () const;

        // Binary operators — * + - / 에 대해 friend sc_fxval operator op(...)
        // 상대 타입: sc_fxnum, int, unsigned int, long, unsigned long, float, double,
        //            const char*, sc_fxval, sc_fxval_fast, sc_fxnum_fast,
        //            int64, uint64, sc_int_base, sc_uint_base, sc_signed, sc_unsigned
        // ... 동일 패턴 (DECL_BIN_OP)

        friend sc_fxval operator<< ( const sc_fxnum& , int );
        friend sc_fxval operator>> ( const sc_fxnum& , int );

        // Relational (including equality): < <= > >= == !=  → friend bool operator op(...)
        // ... 동일 패턴 (DECL_REL_OP), 상대 타입은 위와 동일

        // Assignment: = *= /= += -= 및 <<= >>= (int)
        // sc_fxnum& operator op( tp ); ... 동일 패턴 (DECL_ASN_OP)

        // Auto-increment and auto-decrement
        sc_fxval operator++ ( int );
        sc_fxval operator-- ( int );
        sc_fxnum& operator++ ();
        sc_fxnum& operator-- ();

        // Bit selection
        sc_fxnum_bitref_r operator[] ( int ) const;
        sc_fxnum_bitref† operator[] ( int );

        // Part selection
        sc_fxnum_subref_r operator() ( int , int ) const;
        sc_fxnum_subref† operator() ( int , int );
        sc_fxnum_subref_r range( int , int ) const;
        sc_fxnum_subref† range( int , int );
        sc_fxnum_subref_r operator() () const;
        sc_fxnum_subref† operator() ();
        sc_fxnum_subref_r range() const;
        sc_fxnum_subref† range();

        // Implicit conversion
        operator double() const;

        // Explicit conversion to primitive types
        short to_short() const;
        unsigned short to_ushort() const;
        int to_int() const;
        unsigned int to_uint() const;
        long to_long() const;
        unsigned long to_ulong() const;
        int64 to_int64() const;
        uint64 to_uint64() const;
        float to_float() const;
        double to_double() const;

        // Explicit conversion to character string
        std::string to_string() const;
        std::string to_string( sc_numrep ) const;
        std::string to_string( sc_numrep , bool ) const;
        std::string to_string( sc_fmt ) const;
        std::string to_string( sc_numrep , sc_fmt ) const;
        std::string to_string( sc_numrep , bool , sc_fmt ) const;
        std::string to_dec() const;
        std::string to_bin() const;
        std::string to_oct() const;
        std::string to_hex() const;

        // Query value
        bool is_neg() const;
        bool is_zero() const;
        bool quantization_flag() const;
        bool overflow_flag() const;
        sc_fxval value() const;

        // Query parameters
        int wl() const;
        int iwl() const;
        sc_q_mode q_mode() const;
        sc_o_mode o_mode() const;
        int n_bits() const;
        const sc_fxtype_params& type_params() const;
        const sc_fxcast_switch& cast_switch() const;

        // Print or dump content
        void print( std::ostream& = std::cout ) const;
        void scan( std::istream& = std::cin );
        void dump( std::ostream& = std::cout ) const;

    private:
        // Disabled
        sc_fxnum();
        sc_fxnum( const sc_fxnum& );
};

}         // namespace sc_dt
```

#### Constraints on usage

- 애플리케이션은 `sc_fxnum` 타입 인스턴스를 **직접 생성하면 안 된다 shall not**.
- `sc_fxnum` 포인터·참조로 파생 클래스 객체를 가리켜도 된다 may.
- **Disabled 멤버**: 기본 생성자 `sc_fxnum()`, 복사 생성자 `sc_fxnum(const sc_fxnum&)` — private.

#### Assignment / conversion

- 오버로드 대입 연산자는 SystemC 데이터 타입과 네이티브 C++ 수치 표현으로부터 `sc_fxnum`으로의 변환을 **truncation 또는 sign-extension을 사용해**(7.10.5) 제공해야 shall.
- `operator double()`은 `sc_fxnum` → `double` 암시적 변환을 제공해야 shall.
- `to_*()` 계열은 C++ 수치 타입으로 변환해야 shall. `to_string`/`to_dec`/`to_bin`/`to_oct`/`to_hex`는 7.2.12, 7.10.9.1, 7.10.9.2에 따라 문자열 변환을 수행해야 shall.

### 7.10.12 sc_fxnum_fast

#### Description

`sc_fxnum_fast`는 **limited-precision fixed-point 타입의 base class**. 파생 클래스 모두에 동작하는 함수·오버로드 연산자 정의를 위해 제공되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

class sc_fxnum_fast
{
    friend class sc_fxval_fast;
    friend class sc_fxnum_bitref_r;
    friend class sc_fxnum_bitref†;
    friend class sc_fxnum_subref_r;
    friend class sc_fxnum_subref†;
    friend class sc_fxnum_fast_bitref_r;
    friend class sc_fxnum_fast_bitref†;
    friend class sc_fxnum_fast_subref_r;
    friend class sc_fxnum_fast_subref†;

    public:
        // Unary operators
        sc_fxval_fast operator- () const;
        sc_fxval_fast operator+ () const;

        // Binary * + - / : friend sc_fxval_fast operator op ( const sc_fxnum_fast& , tp );
        // tp: sc_fxnum_fast, int, unsigned int, long, unsigned long, float, double,
        //     const char*, sc_fxval_fast, int64, uint64, sc_int_base, sc_uint_base,
        //     sc_signed, sc_unsigned                      // ... 동일 패턴

        friend sc_fxval operator<< ( const sc_fxnum_fast& , int );
        friend sc_fxval operator>> ( const sc_fxnum_fast& , int );

        // Relational: < <= > >= == !=  → friend bool operator op(...)   // ... 동일 패턴
        // Assignment: = *= /= += -= , <<= >>= (int)                     // ... 동일 패턴

        // Auto-increment and auto-decrement
        sc_fxval_fast operator++ ( int );
        sc_fxval_fast operator-- ( int );
        sc_fxnum_fast& operator++ ();
        sc_fxnum_fast& operator-- ();

        // Bit selection
        sc_fxnum_fast_bitref_r operator[] ( int ) const;
        sc_fxnum_bitref† operator[] ( int );

        // Part selection
        sc_fxnum_fast_subref_r operator() ( int , int ) const;
        sc_fxnum_fast_subref† operator() ( int , int );
        sc_fxnum_fast_subref_r range( int , int ) const;
        sc_fxnum_fast_subref† range( int , int );
        sc_fxnum_fast_subref_r operator() () const;
        sc_fxnum_fast_subref† operator() ();
        sc_fxnum_fast_subref_r range() const;
        sc_fxnum_fast_subref† range();

        // Implicit conversion
        operator double() const;

        // Explicit conversion to primitive types / character string
        //   sc_fxnum 과 동일한 to_short/.../to_double, to_string/.../to_hex

        // Query value
        bool is_neg() const;
        bool is_zero() const;
        bool quantization_flag() const;
        bool overflow_flag() const;
        sc_fxval_fast value() const;

        // Query parameters
        int wl() const;
        int iwl() const;
        sc_q_mode q_mode() const;
        sc_o_mode o_mode() const;
        int n_bits() const;
        const sc_fxtype_params& type_params() const;
        const sc_fxcast_switch& cast_switch() const;

        // Print or dump content
        void print( std::ostream& = std::cout ) const;
        void scan( std::istream& = std::cin );
        void dump( std::ostream& = std::cout ) const;

    private:
        // Disabled
        sc_fxnum_fast();
        sc_fxnum_fast( const sc_fxnum_fast& );
};

}         // namespace sc_dt
```

#### Constraints on usage

- 애플리케이션은 `sc_fxnum_fast` 인스턴스를 **직접 생성하면 안 된다 shall not**. 포인터·참조로 파생 객체를 가리켜도 된다 may.
- **Disabled 멤버**: 기본 생성자·복사 생성자 (private).
- 대입 연산자·암시적/명시적 변환 규칙은 `sc_fxnum`과 동일 (7.10.5).

### 7.10.13 sc_fxval

#### Description

`sc_fxval`은 **variable-precision fixed-point 타입**. 어떤 fixed-point 타입의 값이든 담을 수 있고 variable-precision 산술을 수행해도 된다 may. **타입 캐스팅은 fixed-point 타입들 자신이 수행해야 shall.** `sc_fxval_fast`와 `sc_fxval`은 자유롭게 섞어도 된다 may.

#### Class definition

```cpp
namespace sc_dt {

class sc_fxval
{
    public:
        // Constructors and destructor
        sc_fxval();
        explicit sc_fxval( int );
        explicit sc_fxval( unsigned int );
        explicit sc_fxval( long );
        explicit sc_fxval( unsigned long );
        explicit sc_fxval( float );
        explicit sc_fxval( double );
        explicit sc_fxval( const char* );
        sc_fxval( const sc_fxval& );
        sc_fxval( const sc_fxval_fast& );
        sc_fxval( const sc_fxnum& );
        sc_fxval( const sc_fxnum_fast& );
        explicit sc_fxval( int64 );
        explicit sc_fxval( uint64 );
        explicit sc_fxval( const sc_int_base& );
        explicit sc_fxval( const sc_uint_base& );
        explicit sc_fxval( const sc_signed& );
        explicit sc_fxval( const sc_unsigned& );
        ~sc_fxval();

        // Unary operators
        sc_fxval operator- () const;
        const sc_fxval& operator+ () const;
        friend void neg( sc_fxval& , const sc_fxval& );

        // Binary * + - / : friend sc_fxval operator op ( const sc_fxval& , tp );
        // tp: sc_fxval, int, unsigned int, long, unsigned long, float, double,
        //     const char*, sc_fxval_fast, sc_fxnum_fast, int64, uint64,
        //     sc_int_base, sc_uint_base, sc_signed, sc_unsigned    // ... 동일 패턴

        friend sc_fxval operator<< ( const sc_fxval& , int );
        friend sc_fxval operator>> ( const sc_fxval& , int );

        // Relational: < <= > >= == !=                              // ... 동일 패턴
        // Assignment: = *= /= += -= , <<= >>= (int)                // ... 동일 패턴

        // Auto-increment and auto-decrement
        sc_fxval operator++ ( int );
        sc_fxval operator-- ( int );
        sc_fxval& operator++ ();
        sc_fxval& operator-- ();

        // Implicit conversion
        operator double() const;

        // Explicit conversion to primitive types
        short to_short() const;  unsigned short to_ushort() const;
        int to_int() const;      unsigned int to_uint() const;
        long to_long() const;    unsigned long to_ulong() const;
        int64 to_int64() const;  uint64 to_uint64() const;
        float to_float() const;  double to_double() const;

        // Explicit conversion to character string
        std::string to_string() const;
        std::string to_string( sc_numrep ) const;
        std::string to_string( sc_numrep , bool ) const;
        std::string to_string( sc_fmt ) const;
        std::string to_string( sc_numrep , sc_fmt ) const;
        std::string to_string( sc_numrep , bool , sc_fmt ) const;
        std::string to_dec() const;
        std::string to_bin() const;
        std::string to_oct() const;
        std::string to_hex() const;

        // Member functions
        bool is_neg() const;
        bool is_zero() const;
        void print( std::ostream& = std::cout ) const;
        void scan( std::istream& = std::cin );
        void dump( std::ostream& = std::cout ) const;
};

}         // namespace sc_dt
```

#### Constraints on usage

- 초기값 없이 선언된 `sc_fxval` 객체는 **uninitialized**여야 shall (단 **static으로 선언되면 0으로 초기화**되어야 shall).
- uninitialized 객체는 initialized 객체가 허용되는 곳에 사용해도 된다 may. **uninitialized 객체에 대한 연산 결과는 undefined.**

#### Public constructors

- 생성자 인자는 `sc_fxval` 객체의 **초기값**으로 취해져야 shall.
- **기본 생성자는 값을 초기화하지 않아야 shall.**

#### Operators (Table 48)

| Operator class | Operators in class |
|---|---|
| Arithmetic | `* / + - << >> ++ --` |
| Equality | `== !=` |
| Relational | `<<= >>=` |
| Assignment | `= *= /= += -= <<= >>=` |

`operator<<`·`operator>>`는 **sign extension을 수행하는 산술 시프트**를 정의한다. 피연산자 타입은 7.10.5에 정의된 대로여야 shall.

### 7.10.14 sc_fxval_fast

#### Description

`sc_fxval_fast`는 **limited variable-precision fixed-point 타입**이며 **mantissa 53비트로 제한되어야 shall**. 어떤 fixed-point 타입의 값이든 담을 수 있고, limited variable-precision 산술 연산 수행에 사용되어야 shall. `sc_fxval`과 자유롭게 섞어도 된다 may.

#### Class definition

```cpp
namespace sc_dt {

class sc_fxval_fast
{
    public:
        sc_fxval_fast();
        explicit sc_fxval_fast( int );
        explicit sc_fxval_fast( unsigned int );
        explicit sc_fxval_fast( long );
        explicit sc_fxval_fast( unsigned long );
        explicit sc_fxval_fast( float );
        explicit sc_fxval_fast( double );
        explicit sc_fxval_fast( const char* );
        sc_fxval_fast( const sc_fxval& );
        sc_fxval_fast( const sc_fxval_fast& );
        sc_fxval_fast( const sc_fxnum& );
        sc_fxval_fast( const sc_fxnum_fast& );
        explicit sc_fxval_fast( int64 );
        explicit sc_fxval_fast( uint64 );
        explicit sc_fxval_fast( const sc_int_base& );
        explicit sc_fxval_fast( const sc_uint_base& );
        explicit sc_fxval_fast( const sc_signed& );
        explicit sc_fxval_fast( const sc_unsigned& );
        ~sc_fxval_fast();

        // Unary operators
        sc_fxval_fast operator- () const;
        const sc_fxval_fast& operator+ () const;

        // Binary * + - / : friend sc_fxval_fast operator op ( const sc_fxval_fast& , tp );
        // tp: sc_fxval_fast, int, unsigned int, long, unsigned long, float, double,
        //     const char*, int64, uint64, sc_int_base, sc_uint_base,
        //     sc_signed, sc_unsigned                       // ... 동일 패턴

        friend sc_fxval_fast operator<< ( const sc_fxval_fast& , int );
        friend sc_fxval_fast operator>> ( const sc_fxval_fast& , int );

        // Relational: < <= > >= == !=                      // ... 동일 패턴
        // Assignment: = *= /= += -= , <<= >>= (int)        // ... 동일 패턴

        // Auto-increment and auto-decrement
        sc_fxval_fast operator++ ( int );
        sc_fxval_fast operator-- ( int );
        sc_fxval_fast& operator++ ();
        sc_fxval_fast& operator-- ();

        // Implicit conversion
        operator double() const;

        // Explicit conversion to primitive types / character string
        //   sc_fxval 과 동일한 to_short/.../to_double, to_string/.../to_hex

        // Other member functions
        bool is_neg() const;
        bool is_zero() const;
        void print( std::ostream& = std::cout ) const;
        void scan( std::istream& = std::cin );
        void dump( std::ostream& = std::cout ) const;
};

}         // namespace sc_dt
```

#### Constraints on usage

- 초기값 없이 선언된 `sc_fxval_fast` 객체는 **uninitialized**여야 shall (**static이면 0으로 초기화**되어야 shall).
- uninitialized 객체 사용은 허용되나 **연산 결과는 undefined**.

#### Public constructors

- 생성자 인자는 초기값으로 취해져야 shall. **기본 생성자는 값을 초기화하지 않아야 shall.**

#### Operators (Table 49)

| Operator class | Operators in class |
|---|---|
| Arithmetic | `* / + - << >> ++ --` |
| Equality | `== !=` |
| Relational | `<<= >>=` |
| Assignment | `= *= /= += -= <<= >>=` |

> NOTE(informative) — `operator<<`·`operator>>`는 bitwise 시프트가 아니라 **산술 시프트**를 정의한다. 비트 손실이 없고 적절한 sign extension이 수행되므로 `sc_fxval_fast` 같은 signed 타입에서도 잘 정의된다.

### 7.10.15 sc_fix

#### Description

`sc_fix`는 **signed(2의 보수) finite-precision** fixed-point 값을 표현해야 shall. 타입 파라미터 wl, iwl, q_mode, o_mode, n_bits를 **생성자 인자로** 지정해도 된다 may.

#### Class definition

```cpp
namespace sc_dt {

class sc_fix
: public sc_fxnum
{
    public:
        // Constructors and destructor
        sc_fix();
        sc_fix( int , int );
        sc_fix( sc_q_mode , sc_o_mode );
        sc_fix( sc_q_mode , sc_o_mode, int );
        sc_fix( int , int , sc_q_mode , sc_o_mode );
        sc_fix( int , int , sc_q_mode, sc_o_mode, int );
        sc_fix( const sc_fxcast_switch& );
        sc_fix( int , int , const sc_fxcast_switch& );
        sc_fix( sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_fix( sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        sc_fix( int , int , sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_fix( int , int , sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        sc_fix( const sc_fxtype_params& );
        sc_fix( const sc_fxtype_params& , const sc_fxcast_switch& );

        // 초기값 tp 를 첫 인자로 갖는 위 모든 조합 (DECL_CTORS_T)
        //   tp = int, unsigned int, long, unsigned long, float, double, const char*,
        //        const sc_fxval&, const sc_fxval_fast&, const sc_fxnum&, const sc_fxnum_fast&
        //        → sc_fix( tp ); 도 제공 (DECL_CTORS_T_A)
        //   tp = int64, uint64, const sc_int_base&, const sc_uint_base&,
        //        const sc_signed&, const sc_unsigned&
        //        → explicit sc_fix( tp ); (DECL_CTORS_T_B)
        sc_fix( const sc_fix& );

        // Unary bitwise operators
        sc_fix operator~ () const;

        // Binary bitwise operators
        friend sc_fix operator& ( const sc_fix& , const sc_fix& );
        friend sc_fix operator& ( const sc_fix& , const sc_fix_fast& );
        friend sc_fix operator& ( const sc_fix_fast& , const sc_fix& );
        friend sc_fix operator| ( const sc_fix& , const sc_fix& );
        friend sc_fix operator| ( const sc_fix& , const sc_fix_fast& );
        friend sc_fix operator| ( const sc_fix_fast& , const sc_fix& );
        friend sc_fix operator^ ( const sc_fix& , const sc_fix& );
        friend sc_fix operator^ ( const sc_fix& , const sc_fix_fast& );
        friend sc_fix operator^ ( const sc_fix_fast& , const sc_fix& );

        sc_fix& operator= ( const sc_fix& );

        // Assignment: = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_fix& / const sc_fix_fast& )    // ... 동일 패턴

        sc_fxval operator++ ( int );
        sc_fxval operator-- ( int );
        sc_fix& operator++ ();
        sc_fix& operator-- ();
};

}         // namespace sc_dt
```

#### Constraints on usage

- **word length는 0보다 커야 shall.**
- **saturated bit 수(지정된 경우)는 0보다 작으면 안 된다 shall not.**

#### Public constructors

- 생성자 인자로 타입 파라미터를 지정해도 된다 may (7.10.2).
- **기본 생성자는 생성 시점에 scope에 있는 fixed-point context에 따라 타입 파라미터를 설정해야 shall.**
- 초기값을 C++·SystemC 수치 객체 또는 문자열 리터럴로 추가 지정해도 된다 may.
- fixed-point cast switch를 생성자 인자로 넘겨 casting을 설정해도 된다 may (7.10.8).

#### Assignment / bitwise operators

- 오버로드 대입 연산자는 SystemC 데이터 타입·네이티브 C++ 수치 표현에서 `sc_fix`로의 변환을 truncation 또는 sign-extension으로 제공해야 shall.
- `sc_fix`와 `sc_fix_fast`의 **모든 조합**에 대해 bitwise 연산자가 정의되어야 shall (7.10.5).

### 7.10.16 sc_ufix

#### Description

`sc_ufix`는 **unsigned finite-precision** fixed-point 값을 표현해야 shall. wl, iwl, q_mode, o_mode, n_bits를 생성자 인자로 지정해도 된다 may.

#### Class definition

```cpp
namespace sc_dt {

class sc_ufix
: public sc_fxnum
{
    public:
        // Constructors
        explicit sc_ufix();
        sc_ufix( int , int );
        sc_ufix( sc_q_mode , sc_o_mode );
        sc_ufix( sc_q_mode , sc_o_mode , int );
        sc_ufix( int , int , sc_q_mode , sc_o_mode );
        sc_ufix( int , int , sc_q_mode , sc_o_mode, int );
        explicit sc_ufix( const sc_fxcast_switch& );
        sc_ufix( int , int , const sc_fxcast_switch& );
        sc_ufix( sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_ufix( sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        sc_ufix( int , int , sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_ufix( int , int , sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        explicit sc_ufix( const sc_fxtype_params& );
        sc_ufix( const sc_fxtype_params& , const sc_fxcast_switch& );

        // 초기값 tp 를 첫 인자로 갖는 위 모든 조합 (DECL_CTORS_T)
        //   A그룹(암시적): int, unsigned int, long, unsigned long, float, double, const char*,
        //                  sc_fxval&, sc_fxval_fast&, sc_fxnum&, sc_fxnum_fast&
        //   B그룹(explicit): int64, uint64, sc_int_base&, sc_uint_base&, sc_signed&, sc_unsigned&

        // Copy constructor
        sc_ufix( const sc_ufix& );

        // Unary bitwise operators
        sc_ufix operator~ () const;

        // Binary bitwise operators
        friend sc_ufix operator& ( const sc_ufix& , const sc_ufix& );
        friend sc_ufix operator& ( const sc_ufix& , const sc_ufix_fast& );
        friend sc_ufix operator& ( const sc_ufix_fast& , const sc_ufix& );
        friend sc_ufix operator| ( const sc_ufix& , const sc_ufix& );
        friend sc_ufix operator| ( const sc_ufix& , const sc_ufix_fast& );
        friend sc_ufix operator| ( const sc_ufix_fast& , const sc_ufix& );
        friend sc_ufix operator^ ( const sc_ufix& , const sc_ufix& );
        friend sc_ufix operator^ ( const sc_ufix& , const sc_ufix_fast& );
        friend sc_ufix operator^ ( const sc_ufix_fast& , const sc_ufix& );

        // Assignment operators
        sc_ufix& operator= ( const sc_ufix& );
        // = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_ufix& / const sc_ufix_fast& )   // ... 동일 패턴

        // Auto-increment and auto-decrement
        sc_fxval operator++ ( int );
        sc_fxval operator-- ( int );
        sc_ufix& operator++ ();
        sc_ufix& operator-- ();
};

}         // namespace sc_dt
```

#### Constraints on usage

- word length는 0보다 커야 shall. saturated bit 수(지정 시)는 0보다 작으면 안 된다 shall not.

#### Public constructors / assignment / bitwise

- `sc_fix`와 동일: **기본 생성자는 생성 시점 scope의 fixed-point context에 따라 타입 파라미터를 설정해야 shall.** 초기값과 cast switch를 인자로 줄 수 있다 may.
- 대입 연산자는 truncation/sign-extension 변환을 제공해야 shall.
- `sc_ufix`와 `sc_ufix_fast`의 **모든 조합**에 대해 bitwise 연산자가 정의되어야 shall.

### 7.10.17 sc_fix_fast

#### Description

`sc_fix_fast`는 **signed(2의 보수) limited-precision** fixed-point 값을 표현해야 shall. wl, iwl, q_mode, o_mode, n_bits를 생성자 인자로 지정해도 된다 may.

#### Class definition

```cpp
namespace sc_dt {

class sc_fix_fast
: public sc_fxnum_fast
{
    public:
        // Constructors — sc_fix 와 동일한 14가지 조합
        sc_fix_fast();
        sc_fix_fast( int , int );
        sc_fix_fast( sc_q_mode , sc_o_mode );
        sc_fix_fast( sc_q_mode , sc_o_mode , int );
        sc_fix_fast( int , int , sc_q_mode , sc_o_mode );
        sc_fix_fast( int , int , sc_q_mode , sc_o_mode , int );
        sc_fix_fast( const sc_fxcast_switch& );
        sc_fix_fast( int , int , const sc_fxcast_switch& );
        sc_fix_fast( sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_fix_fast( sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        sc_fix_fast( int , int , sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_fix_fast( int , int , sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        sc_fix_fast( const sc_fxtype_params& );
        sc_fix_fast( const sc_fxtype_params& , const sc_fxcast_switch& );
        // + 초기값 tp 를 첫 인자로 갖는 위 모든 조합 (A그룹 암시적 / B그룹 explicit)

        // Copy constructor
        sc_fix_fast( const sc_fix_fast& );

        // Operators
        sc_fix_fast operator~ () const;
        friend sc_fix_fast operator& ( const sc_fix_fast& , const sc_fix_fast& );
        friend sc_fix_fast operator^ ( const sc_fix_fast& , const sc_fix_fast& );
        friend sc_fix_fast operator| ( const sc_fix_fast& , const sc_fix_fast& );
        sc_fix_fast& operator= ( const sc_fix_fast& );

        // = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_fix& / const sc_fix_fast& )    // ... 동일 패턴

        sc_fxval_fast operator++ ( int );
        sc_fxval_fast operator-- ( int );
        sc_fix_fast& operator++ ();
        sc_fix_fast& operator-- ();
};

}         // namespace sc_dt
```

#### Constraints on usage

- word length는 0보다 커야 shall. saturated bit 수(지정 시)는 0보다 작으면 안 된다 shall not.
- **`sc_fix_fast`는 double-precision(부동소수점) 값을 사용해야 shall. double 값의 mantissa는 53비트로 제한되므로 limited-precision 타입에서는 bit-true 동작이 보장될 수 없다.**

#### Public constructors / assignment / bitwise

- 기본 생성자는 생성 시점 scope의 fixed-point context에 따라 타입 파라미터를 설정해야 shall. 초기값·cast switch를 인자로 줄 수 있다 may.
- 대입 연산자는 truncation/sign-extension 변환을 제공해야 shall.
- `sc_fix_fast` 피연산자에 대한 bitwise 연산자가 정의되어야 shall (7.10.5).

### 7.10.18 sc_ufix_fast

#### Description

`sc_ufix_fast`는 **unsigned limited-precision** fixed-point 값을 표현해야 shall. wl, iwl, q_mode, o_mode, n_bits를 생성자 인자로 지정해도 된다 may.

#### Class definition

```cpp
namespace sc_dt {

class sc_ufix_fast
: public sc_fxnum_fast
{
    public:
        // Constructors — sc_ufix 와 동일한 조합 (기본·fxcast_switch·fxtype_params 는 explicit)
        explicit sc_ufix_fast();
        sc_ufix_fast( int , int );
        sc_ufix_fast( sc_q_mode , sc_o_mode );
        sc_ufix_fast( sc_q_mode , sc_o_mode , int );
        sc_ufix_fast( int , int , sc_q_mode , sc_o_mode );
        sc_ufix_fast( int , int , sc_q_mode , sc_o_mode , int );
        explicit sc_ufix_fast( const sc_fxcast_switch& );
        sc_ufix_fast( int , int , const sc_fxcast_switch& );
        sc_ufix_fast( sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_ufix_fast( sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        sc_ufix_fast( int , int , sc_q_mode , sc_o_mode , const sc_fxcast_switch& );
        sc_ufix_fast( int , int , sc_q_mode , sc_o_mode , int , const sc_fxcast_switch& );
        explicit sc_ufix_fast( const sc_fxtype_params& );
        sc_ufix_fast( const sc_fxtype_params& , const sc_fxcast_switch& );
        // + 초기값 tp 를 첫 인자로 갖는 위 모든 조합 (A그룹 암시적 / B그룹 explicit)

        // Copy constructor
        sc_ufix_fast( const sc_ufix_fast& );

        // Unary bitwise operators
        sc_ufix_fast operator~ () const;

        // Binary bitwise operators
        friend sc_ufix_fast operator& ( const sc_ufix_fast& , const sc_ufix_fast& );
        friend sc_ufix_fast operator^ ( const sc_ufix_fast& , const sc_ufix_fast& );
        friend sc_ufix_fast operator| ( const sc_ufix_fast& , const sc_ufix_fast& );

        // Assignment operators
        sc_ufix_fast& operator= ( const sc_ufix_fast& );
        // = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_ufix& / const sc_ufix_fast& )   // ... 동일 패턴

        // Auto-increment and auto-decrement
        sc_fxval_fast operator++ ( int );
        sc_fxval_fast operator-- ( int );
        sc_ufix_fast& operator++ ();
        sc_ufix_fast& operator-- ();
};

}         // namespace sc_dt
```

#### Constraints on usage

- word length는 0보다 커야 shall. saturated bit 수(지정 시)는 0보다 작으면 안 된다 shall not.
- **`sc_ufix_fast`는 double-precision 값을 사용해야 shall. mantissa 53비트 제한으로 bit-true 동작이 보장될 수 없다.**

#### Public constructors / assignment / bitwise

- 기본 생성자는 생성 시점 scope의 fixed-point context에 따라 타입 파라미터를 설정해야 shall.
- 대입 연산자는 truncation/sign-extension 변환을 제공해야 shall.
- `sc_ufix_fast` 피연산자에 대한 bitwise 연산자가 정의되어야 shall.

### 7.10.19 sc_fixed

#### Description

클래스 템플릿 `sc_fixed`는 **signed(2의 보수) finite-precision** fixed-point 값을 표현해야 shall. 타입 파라미터 wl, iwl, q_mode, o_mode, n_bits는 **템플릿 인자로 지정되어야 shall**.

base class `sc_fix`의 public 멤버 함수 중 `sc_fixed`에서 오버라이드된 것은 두 클래스에서 **같은 동작**을 가져야 shall. 오버라이드되지 않은 것은 `sc_fixed`가 **public 상속**해야 shall.

#### Class definition

```cpp
namespace sc_dt {

template <int W, int I,
    sc_q_mode Q = SC_DEFAULT_Q_MODE_,
    sc_o_mode O = SC_DEFAULT_O_MODE_, int N = SC_DEFAULT_N_BITS_>
class sc_fixed
: public sc_fix
{
    public:
        // Constructors
        sc_fixed();
        sc_fixed( const sc_fxcast_switch& );

        // 각 tp 에 대해:  sc_fixed( tp );  sc_fixed( tp , const sc_fxcast_switch& );
        //   tp = int, unsigned int, long, unsigned long, float, double, const char*,
        //        const sc_fxval&, const sc_fxval_fast&, const sc_fxnum&, const sc_fxnum_fast&,
        //        int64, uint64, const sc_int_base&, const sc_uint_base&,
        //        const sc_signed&, const sc_unsigned&
        sc_fixed( const sc_fixed<W,I,Q,O,N>& );

        // Operators
        sc_fixed& operator= ( const sc_fixed<W,I,Q,O,N>& );
        // = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_fix& / const sc_fix_fast& )   // ... 동일 패턴

        sc_fxval operator++ ( int );
        sc_fxval operator-- ( int );
        sc_fixed& operator++ ();
        sc_fixed& operator-- ();
};

}         // namespace sc_dt
```

**템플릿 기본 인자**: `Q = SC_DEFAULT_Q_MODE_`, `O = SC_DEFAULT_O_MODE_`, `N = SC_DEFAULT_N_BITS_`. `W`(wl)와 `I`(iwl)는 **기본값 없음 — 반드시 지정**.

#### Constraints on usage

- word length는 0보다 커야 shall. saturated bit 수(지정 시)는 0보다 작으면 안 된다 shall not.

#### Public constructors / assignment

- 초기값을 C++·SystemC 수치 객체 또는 문자열 리터럴로 생성자 인자에 지정해도 된다 may. cast switch도 생성자 인자로 넘겨도 된다 may (7.10.8).
- 대입 연산자는 truncation/sign-extension 변환을 제공해야 shall.

### 7.10.20 sc_ufixed

#### Description

클래스 템플릿 `sc_ufixed`는 **unsigned finite-precision** fixed-point 값을 표현한다. wl, iwl, q_mode, o_mode, n_bits는 **템플릿 인자로 지정되어야 shall**.

base class `sc_ufix`의 public 멤버 함수 중 오버라이드된 것은 두 클래스에서 같은 동작을 가져야 shall. 나머지는 public 상속되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

template <int W, int I,
     sc_q_mode Q = SC_DEFAULT_Q_MODE_,
     sc_o_mode O = SC_DEFAULT_O_MODE_, int N = SC_DEFAULT_N_BITS_>
class sc_ufixed
: public sc_ufix
{
    public:
        // Constructors
        explicit sc_ufixed();
        explicit sc_ufixed( const sc_fxcast_switch& );

        // A그룹(암시적): int, unsigned int, long, unsigned long, float, double, const char*,
        //                sc_fxval&, sc_fxval_fast&, sc_fxnum&, sc_fxnum_fast&
        //     sc_ufixed( tp );  sc_ufixed( tp , const sc_fxcast_switch& );
        // B그룹(explicit): int64, uint64, sc_int_base&, sc_uint_base&, sc_signed&, sc_unsigned&
        //     explicit sc_ufixed( tp );  sc_ufixed( tp , const sc_fxcast_switch& );

        // Copy constructor
        sc_ufixed( const sc_ufixed<W,I,Q,O,N>& );

        // Assignment operators
        sc_ufixed& operator= ( const sc_ufixed<W,I,Q,O,N>& );
        // = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_ufix& / const sc_ufix_fast& )   // ... 동일 패턴

        // Auto-increment and auto-decrement
        sc_fxval operator++ ( int );
        sc_fxval operator-- ( int );
        sc_ufixed& operator++ ();
        sc_ufixed& operator-- ();
};

}         // namespace sc_dt
```

#### Constraints on usage

- word length는 0보다 커야 shall. saturated bit 수(지정 시)는 0보다 작으면 안 된다 shall not.

#### Public constructors / assignment

- 초기값을 C++·SystemC 수치 객체 또는 문자열 리터럴로 지정해도 된다 may. cast switch도 인자로 넘겨도 된다 may.
- 대입 연산자는 truncation/sign-extension 변환을 제공해야 shall.

### 7.10.21 sc_fixed_fast

#### Description

클래스 템플릿 `sc_fixed_fast`는 **signed(2의 보수) limited-precision** fixed-point 타입을 표현해야 shall. wl, iwl, q_mode, o_mode, n_bits는 **템플릿 인자로 지정되어야 shall**.

base class `sc_fix_fast`의 public 멤버 함수 중 오버라이드된 것은 두 클래스에서 같은 동작을 가져야 shall. 나머지는 public 상속되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

template <int W, int I,
    sc_q_mode Q = SC_DEFAULT_Q_MODE_,
    sc_o_mode O = SC_DEFAULT_O_MODE_, int N = SC_DEFAULT_N_BITS_>
class sc_fixed_fast
: public sc_fix_fast
{
    public:
        // Constructors
        sc_fixed_fast();
        sc_fixed_fast( const sc_fxcast_switch& );

        // 각 tp 에 대해: sc_fixed_fast( tp );  sc_fixed_fast( tp , const sc_fxcast_switch& );
        //   tp = int, unsigned int, long, unsigned long, float, double, const char*,
        //        sc_fxval&, sc_fxval_fast&, sc_fxnum&, sc_fxnum_fast&,
        //        int64, uint64, sc_int_base&, sc_uint_base&, sc_signed&, sc_unsigned&
        sc_fixed_fast( const sc_fixed_fast<W,I,Q,O,N>& );

        // Operators
        sc_fixed_fast& operator= ( const sc_fixed_fast<W,I,Q,O,N>& );
        // = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_fix& / const sc_fix_fast& )   // ... 동일 패턴

        sc_fxval_fast operator++ ( int );
        sc_fxval_fast operator-- ( int );
        sc_fixed_fast& operator++ ();
        sc_fixed_fast& operator-- ();
};

}         // namespace sc_dt
```

#### Constraints on usage

- word length는 0보다 커야 shall. saturated bit 수(지정 시)는 0보다 작으면 안 된다 shall not.
- **`sc_fixed_fast`는 mantissa가 53비트로 제한된 double-precision(부동소수점) 값을 사용해야 shall.**

#### Public constructors / assignment

- 초기값을 C++·SystemC 수치 객체 또는 문자열 리터럴로 지정해도 된다 may. cast switch도 인자로 넘겨도 된다 may.
- 대입 연산자는 truncation/sign-extension 변환을 제공해야 shall.

### 7.10.22 sc_ufixed_fast

#### Description

클래스 템플릿 `sc_ufixed_fast`는 **unsigned limited-precision** fixed-point 타입을 표현해야 shall. wl, iwl, q_mode, o_mode, n_bits는 **템플릿 인자로 지정되어야 shall**.

base class `sc_ufix_fast`의 public 멤버 함수 중 오버라이드된 것은 두 클래스에서 같은 동작을 가져야 shall. 나머지는 public 상속되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

template <int W, int I,
    sc_q_mode Q = SC_DEFAULT_Q_MODE_,
    sc_o_mode O = SC_DEFAULT_O_MODE_, int N = SC_DEFAULT_N_BITS_>
class sc_ufixed_fast
: public sc_ufix_fast
{
    public:
        // Constructors
        explicit sc_ufixed_fast();
        explicit sc_ufixed_fast( const sc_fxcast_switch& );

        // A그룹(암시적): int, unsigned int, long, unsigned long, float, double, const char*,
        //                sc_fxval&, sc_fxval_fast&, sc_fxnum&, sc_fxnum_fast&
        //     sc_ufixed_fast( tp );  sc_ufixed_fast( tp , const sc_fxcast_switch& );
        // B그룹(explicit): int64, uint64, sc_int_base&, sc_uint_base&, sc_signed&, sc_unsigned&
        //     explicit sc_ufixed_fast( tp );  sc_ufixed_fast( tp , const sc_fxcast_switch& );

        // Copy constructor
        sc_ufixed_fast( const sc_ufixed_fast<W,I,Q,O,N>& );

        // Assignment operators
        sc_ufixed_fast& operator= ( const sc_ufixed_fast<W,I,Q,O,N>& );
        // = *= /= += -= (모든 수치 타입), <<= >>= (int)
        // &= |= ^= ( const sc_ufix& / const sc_ufix_fast& )   // ... 동일 패턴

        // Auto-increment and auto-decrement
        sc_fxval_fast operator++ ( int );
        sc_fxval_fast operator-- ( int );
        sc_ufixed_fast& operator++ ();
        sc_ufixed_fast& operator-- ();
};

}         // namespace sc_dt
```

#### Constraints on usage

- word length는 0보다 커야 shall. saturated bit 수(지정 시)는 0보다 작으면 안 된다 shall not.
- **`sc_ufixed_fast`는 mantissa가 53비트로 제한된 double-precision 값을 사용해야 shall.**

#### Public constructors / assignment

- 초기값을 C++·SystemC 수치 객체 또는 문자열 리터럴로 지정해도 된다 may. cast switch도 인자로 넘겨도 된다 may.
- 대입 연산자는 truncation/sign-extension 변환을 제공해야 shall.

### 7.10.23 Bit-selects

#### Description

| 클래스 | 역할 |
|---|---|
| `sc_fxnum_bitref_r`† | `sc_fxnum`에서 선택된 **read-only** 비트 |
| `sc_fxnum_bitref`† | `sc_fxnum`에서 선택된 **read-write** 비트 |
| `sc_fxnum_fast_bitref_r`† | `sc_fxnum_fast`에서 선택된 **read-only** 비트 |
| `sc_fxnum_fast_bitref`† | `sc_fxnum_fast`에서 선택된 **read-write** 비트 |

**bit-select가 lvalue로 쓰이든 rvalue로 쓰이든 구별하지 않아야 shall.**

#### Class definition

```cpp
class sc_fxnum_fast_bitref_r {
    friend class sc_fxnum;
    friend class sc_fxnum_fast_bitref;

    protected:
        bool get() const;
        sc_fxnum_fast_bitref_r(sc_fxnum&, int );

    public:
        sc_fxnum_fast_bitref_r ( const sc_fxnum_fast_bitref_r& );

        operator bool() const;

        // print or dump content
        void print( std::ostream& = std::cout ) const;
        void dump( std::ostream& = std::cout ) const;

    protected:
        sc_fxnum& m_num;
        int    m_idx;
};

class sc_fxnum_bitref : public sc_fxnum_bitref_r
{
    friend class sc_fxnum;

    void set( bool );
    sc_fxnum_bitref( sc_fxnum&, int );          // constructor

    public:
        sc_fxnum_bitref( const sc_fxnum_bitref& );   // copy constructor

        // assignment operators:  =  &=  |=  ^=
        //   각각 tp = const sc_fxnum_bitref&, const sc_fxnum_fast_bitref&,
        //             const sc_bit&, bool
        //   sc_fxnum_bitref& operator op ( tp );

        void scan( std::istream& = std::cin );
};

class sc_fxnum_fast_bitref : public sc_fxnum_fast_bitref_r
{
    friend class sc_fxnum_fast;

    void set( bool );
    sc_fxnum_fast_bitref( sc_fxnum_fast&, int );

    public:
        sc_fxnum_fast_bitref( const sc_fxnum_fast_bitref& );

        // assignment operators: = &= |= ^=  (tp 동일)
        //   sc_fxnum_fast_bitref& operator op ( tp );

        void scan( std::istream& = std::cin );
};

class sc_fxnum_fast_bitref_r
{
    friend class sc_fxnum_fast;
    friend class sc_fxnum_bitref;

    protected:
        bool get() const;
        sc_fxnum_fast_bitref_r( sc_fxnum_fast&, int );

    public:
        sc_fxnum_fast_bitref_r( const sc_fxnum_fast_bitref_r& );   // copy constructor

        operator bool() const;

        void print( std::ostream& = std::cout ) const;
        void dump( std::ostream& = std::cout ) const;

    protected:
        sc_fxnum_fast& m_num;
        int     m_idx;
};
```

#### Constraints on usage

- bit-select 객체는 `sc_fxnum` 또는 `sc_fxnum_fast` 파생 클래스 인스턴스의 **bit-select 연산자로만 생성되어야 shall**.
- 애플리케이션은 어떤 bit-select 클래스의 인스턴스도 **명시적으로 생성하면 안 된다 shall not**.
- 애플리케이션은 bit-select 객체에 대한 **참조나 포인터를 선언하지 않는 것이 권장(should not)**된다.

#### Assignment / conversion

- 오버로드 대입 연산자는 `bool` 값으로부터의 변환을 제공해야 shall.
- `operator bool() const;` — bit-select에서 네이티브 C++ `bool` 표현으로의 암시적 변환에 쓸 수 있다.

### 7.10.24 Part-selects

#### Description

| 클래스 | 역할 |
|---|---|
| `sc_fxnum_subref_r`† | `sc_fxnum`에서 선택된 **read-only** part |
| `sc_fxnum_subref`† | `sc_fxnum`에서 선택된 **read-write** part |
| `sc_fxnum_fast_subref_r`† | `sc_fxnum_fast`에서 선택된 **read-only** part |
| `sc_fxnum_fast_subref`† | `sc_fxnum_fast`에서 선택된 **read-write** part |

**part-select가 lvalue로 쓰이든 rvalue로 쓰이든 구별하지 않아야 shall.**

#### Class definition

```cpp
class sc_fxnum_subref_r
{
    friend class sc_fxnum;

    protected:
        bool get() const;
        sc_fxnum_subref_r( sc_fxnum&, int, int );

    public:
        sc_fxnum_subref_r( const sc_fxnum_subref_r& );   // copy constructor
        ~sc_fxnum_subref_r();                            // destructor

        // relational operators: == !=
        //   friend bool operator op ( const sc_fxnum_subref_r&, const sc_fxnum_subref_r& );
        //   friend bool operator op ( const sc_fxnum_subref_r&, const sc_fxnum_fast_subref_r& );
        //   그리고 tp = const sc_bv_base&, const sc_lv_base&, const char*, const bool*,
        //              const sc_signed&, const sc_unsigned&, int, unsigned int,
        //              long, unsigned long        // ... 동일 패턴 (양방향)

        // reduce functions
        bool and_reduce() const;
        bool nand_reduce() const;
        bool or_reduce() const;
        bool nor_reduce() const;
        bool xor_reduce() const;
        bool xnor_reduce() const;

        // query parameter
        int length() const;

        // explicit conversions
        int                 to_int() const;
        unsigned int        to_uint() const;
        long                to_long() const;
        unsigned long       to_ulong() const;
        int64               to_int64() const;
        uint64              to_uint64() const;

        #ifdef SC_DT_DEPRECATED
        int            to_signed() const;
        unsigned int   to_unsigned() const;
        #endif

        std::string to_string() const;
        std::string to_string( sc_numrep ) const;
        std::string to_string( sc_numrep, bool ) const;

        // implicit conversion
        operator sc_bv_base() const;

        // print or dump content
        void print( std::ostream& = std::cout ) const;
        void dump( std::ostream& = std::cout ) const;
};

class sc_fxnum_subref : public sc_fxnum_subref_r
{
    friend class sc_fxnum;
    friend class sc_fxnum_fast_subref;

    bool set();
    sc_fxnum_subref( sc_fxnum&, int, int );      // constructor

    public:
        sc_fxnum_subref( const sc_fxnum_subref& );   // copy constructor

        // assignment operators — sc_fxnum_subref& operator = ( tp );
        //   tp = const sc_fxnum_subref&, const sc_fxnum_fast_subref&,
        //        const sc_bv_base&, const sc_lv_base&, const char*, const bool*,
        //        const sc_signed&, const sc_unsigned&,
        //        const sc_int_base&, const sc_uint_base&,
        //        int64, uint64, int, unsigned int, long, unsigned long, char

        // sc_fxnum_subref& operator &= / |= / ^= ( tp );
        //   tp = const sc_fxnum_subref&, const sc_fxnum_fast_subref&,
        //        const sc_bv_base&, const sc_lv_base&

        void scan( std::istream& = std::cin );
};

class sc_fxnum_fast_subref_r
{
    friend class sc_fxnum_fast;

    protected:
        bool get() const;
        sc_fxnum_fast_subref_r( sc_fxnum_fast&, int, int );

    public:
        sc_fxnum_fast_subref_r( const sc_fxnum_fast_subref_r& );
        ~sc_fxnum_fast_subref_r();

        // relational == != , reduce, length, to_int/.../to_uint64,
        // to_string(3종), operator sc_bv_base(), print, dump
        //   — sc_fxnum_subref_r 과 동일 패턴
};

class sc_fxnum_fast_subref : public sc_fxnum_fast_subref_r
{
    friend class sc_fxnum_fast;
    friend class sc_fxnum_subref;

    bool set();
    sc_fxnum_fast_subref( sc_fxnum_fast&, int, int );

    public:
        sc_fxnum_fast_subref( const sc_fxnum_fast_subref& );

        // assignment = 및 &= |= ^= — sc_fxnum_subref 과 동일 패턴

        void scan( std::istream& = std::cin );
};
```

#### Constraints on usage

- fixed-point part-select 객체는 `sc_fxnum` 또는 `sc_fxnum_fast` 파생 클래스 인스턴스의 **part-select 연산자로만 생성되어야 shall**.
- 애플리케이션은 어떤 fixed-point part-select 클래스의 인스턴스도 **명시적으로 생성하면 안 된다 shall not**.
- 애플리케이션은 fixed-point part-select 객체에 대한 **참조나 포인터를 선언하지 않는 것이 권장(should not)**된다.
- **fixed-point part-select에는 산술 연산자가 제공되지 않는다.**

#### Assignment / bitwise / conversion

- 오버로드 대입 연산자는 SystemC 데이터 타입과 네이티브 C++ 정수 표현에서 fixed-point part-select로의 변환을 제공해야 shall. 데이터 타입·문자열 리터럴 피연산자의 크기가 part-select word length와 다르면 **truncation, zero-extension, 또는 sign-extension**이 사용되어야 shall (7.2.2).
- fixed-point part-select, bit-vector, logic-vector 피연산자에 대해 오버로드 bitwise 연산자가 제공되어야 shall.
- `sc_fxnum_subref_r†::operator sc_bv_base() const;` / `sc_fxnum_fast_subref_r†::operator sc_bv_base() const;` — part-select에서 SystemC bit-vector 표현으로의 암시적 변환에 쓸 수 있다.
- `to_int/to_uint/to_long/to_ulong/to_int64/to_uint64`는 C++ 정수 타입으로 변환해야 shall.
- `to_string` (3종)은 7.2.12, 7.10.9.1, 7.10.9.2에 따라 문자열 변환을 수행해야 shall.

---

## 7.11 Contexts

### 7.11.1 Overview

7.11은 데이터 타입의 **context를 설정하는 클래스들**을 정의한다.

### 7.11.2 sc_length_param

#### Description

`sc_length_param`은 **length parameter를 표현해야 shall**이며 length context를 만드는 데 사용되어야 shall (7.2.4).

#### Class definition

```cpp
namespace sc_dt {

class sc_length_param
{
    public:
        sc_length_param();
        sc_length_param( int );
        sc_length_param( const sc_length_param& );

        sc_length_param& operator= ( const sc_length_param& );
        friend bool operator== ( const sc_length_param& , const sc_length_param& );
        friend bool operator!= ( const sc_length_param& , const sc_length_param& );

        int len() const;
        void len( int );
        std::string to_string() const;
        void print( std::ostream& = std::cout ) const;
        void dump( std::ostream& = std::cout ) const;
};

}         // namespace sc_dt
```

#### Constraints on usage

- 길이는(지정된 경우) **0보다 커야 shall**.

#### Public constructors

| 생성자 | 규칙 |
|---|---|
| `sc_length_param();` | **기본 word length 32**로 객체를 생성해야 shall |
| `sc_length_param( int n );` | `n`(> 0)을 word length로 하는 객체를 생성해야 shall |
| `sc_length_param( const sc_length_param& );` | 인자로 준 객체의 복사본을 생성해야 shall |

#### Member functions

| 함수 | 규칙 |
|---|---|
| `int len() const;` | 저장된 word length를 반환해야 shall |
| `void len( int n );` | word length를 `n`(> 0)으로 설정해야 shall |
| `std::string to_string() const;` | 문자열 표현으로 변환해야 shall |
| `void print( std::ostream& = std::cout ) const;` | 내용을 스트림에 출력해야 shall |

#### Operators

| 연산자 | 규칙 |
|---|---|
| `operator=` | 우변 `a`의 word-length 값을 좌변 인스턴스에 대입해야 shall |
| `operator==` | `a`와 `b`의 저장 길이가 같으면 true를 반환해야 shall |
| `operator!=` | `a`와 `b`의 저장 길이가 다르면 true를 반환해야 shall |

### 7.11.3 sc_length_context

#### Description

`sc_length_context`는 **SystemC 정수·벡터 객체를 위한 length context**를 만드는 데 사용되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

class sc_length_context
{
    public:
        explicit sc_length_context( const sc_length_param& , sc_context_begin† = SC_NOW );
        ~sc_length_context();

        void begin();
        void end();
        static const sc_length_param& default_value();
        const sc_length_param& value() const;
};

}         // namespace sc_dt
```

#### Public constructor

- 첫 인자는 사용할 length parameter여야 shall. 둘째 인자를 주면 **`SC_NOW` 또는 `SC_LATER`** 값이어야 shall.

#### Member functions

| 함수 | 규칙 |
|---|---|
| `void begin();` | 현재 length context를 설정해야 shall (7.2.4) |
| `static const sc_length_param& default_value();` | 현재 context에 있는 length parameter를 반환해야 shall |
| `void end();` | length context를 비활성화하고 **length context stack 최상단에서 제거**해야 shall (7.2.4) |
| `const sc_length_param& value() const;` | length parameter를 반환해야 shall |

### 7.11.4 sc_fxtype_params

#### Description

`sc_fxtype_params`는 length parameter를 표현해야 shall이며 **fixed-point 객체를 위한 length context**를 만드는 데 사용되어야 shall (7.2.4).

#### Class definition

```cpp
namespace sc_dt {

class sc_fxtype_params
{
    public:
        // Constructors and destructor
        sc_fxtype_params();
        sc_fxtype_params( int , int );
        sc_fxtype_params( sc_q_mode , sc_o_mode, int = 0 );
        sc_fxtype_params( int , int , sc_q_mode , sc_o_mode , int = 0 );
        sc_fxtype_params( const sc_fxtype_params& );
        sc_fxtype_params( const sc_fxtype_params& , int , int );
        sc_fxtype_params( const sc_fxtype_params& , sc_q_mode , sc_o_mode , int = 0 );

        // Operators
        sc_fxtype_params& operator= ( const sc_fxtype_params& );
        friend bool operator== ( const sc_fxtype_params& , const sc_fxtype_params& );
        friend bool operator!= ( const sc_fxtype_params& , const sc_fxtype_params& );

        // Member functions
        int wl() const;
        void wl( int );
        int iwl() const;
        void iwl( int );
        sc_q_mode q_mode() const;
        void q_mode( sc_q_mode );
        sc_o_mode o_mode() const;
        void o_mode( sc_o_mode );
        int n_bits() const;
        void n_bits( int );
        std::string to_string() const;
        void print( std::ostream& = std::cout ) const;
        void dump( std::ostream& = std::cout ) const;
};

}         // namespace sc_dt
```

#### Constraints on usage

- 길이는(지정된 경우) **0보다 커야 shall**.

#### Public constructors — 파라미터 규칙

| 파라미터 | 규칙 | 기본값 |
|---|---|---|
| `wl` | fixed-point 형식의 **전체 비트 수**. **0보다 커야 shall** | 현재 scope의 fixed-point context에서 얻어야 shall |
| `iwl` | fixed-point 형식의 **정수 비트 수**. **양수여도 음수여도 된다 may** | 현재 scope의 fixed-point context에서 얻어야 shall |
| `q_mode` | 사용할 quantization mode. 유효 값은 7.10.10.10 | 현재 scope의 fixed-point context에서 얻어야 shall |
| `o_mode` | 사용할 overflow mode. 유효 값은 7.10.10.2 | 현재 scope의 fixed-point context에서 얻어야 shall |
| `n_bits` | 선택된 overflow mode의 saturated bit 수. **0 이상이어야 shall** | **overflow mode가 지정되면 기본값은 0**. overflow mode가 지정되지 않으면 현재 scope의 fixed-point context에서 얻어야 shall |

- **현재 scope에 fixed-point context가 없으면 wl, iwl, q_mode, o_mode, n_bits의 기본값은 Table 38(7.10.8)의 값이어야 shall** — 즉 32, 32, `SC_TRN`, `SC_WRAP`, 0.

#### Member functions

| 함수 | 규칙 |
|---|---|
| `int iwl() const;` / `void iwl( int val );` | iwl 값을 반환 / `val`로 설정해야 shall |
| `int n_bits() const;` / `void n_bits( int );` | n_bits 값을 반환 / `val`로 설정해야 shall |
| `sc_o_mode o_mode() const;` / `void o_mode( sc_o_mode mode );` | o_mode를 반환 / `mode`로 설정해야 shall |
| `sc_q_mode q_mode() const;` / `void q_mode( sc_q_mode mode );` | q_mode를 반환 / `mode`로 설정해야 shall |
| `int wl() const;` / `void wl( int val );` | wl 값을 반환 / `val`로 설정해야 shall |

#### Operators

| 연산자 | 규칙 |
|---|---|
| `operator=` | 우변 `param_`의 wl, iwl, q_mode, o_mode, n_bits를 좌변에 대입해야 shall |
| `operator==` | `param_a`의 5개 값이 `param_b`의 대응 값과 모두 같으면 true, 아니면 false를 반환해야 shall |
| `operator!=` | 5개 값이 대응 값과 같지 않으면 true, 아니면 false를 반환해야 shall |

### 7.11.5 sc_fxtype_context

#### Description

`sc_fxtype_context`는 **fixed-point 객체를 위한 length context**를 만드는 데 사용되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

class sc_fxtype_context
{
    public:
        explicit sc_fxtype_context( const sc_fxtype_params& , sc_context_begin† = SC_NOW );
        ~sc_fxtype_context();

        void begin();
        void end();
        static const sc_fxtype_params& default_value();
        const sc_fxtype_params& value() const;
};

}         // namespace sc_dt
```

#### Public constructor

- 첫 인자는 사용할 fixed-point length parameter여야 shall. 둘째 인자를 주면 **`SC_NOW` 또는 `SC_LATER`** 값이어야 shall.

#### Member functions

| 함수 | 규칙 |
|---|---|
| `void begin();` | 현재 length context를 설정해야 shall (7.2.4) |
| `static const sc_fxtype_params& default_value();` | 현재 context에 있는 length parameter를 반환해야 shall |
| `void end();` | length context를 비활성화하고 **length context stack 최상단에서 제거**해야 shall (7.2.4) |
| `const sc_fxtype_params& value() const;` | length parameter를 반환해야 shall |

### 7.11.6 sc_fxcast_switch

#### Description

`sc_fxcast_switch`는 **floating-point cast context를 설정**하는 데 사용되어야 shall (7.10.8).

#### Class definition

```cpp
namespace sc_dt {

class sc_fxcast_switch
{
    public:
        // Constructors
        sc_fxcast_switch();
        sc_fxcast_switch( sc_switch† );
        sc_fxcast_switch( const sc_fxcast_switch& );

        // Operators
        sc_fxcast_switch& operator= ( const sc_fxcast_switch& );
        friend bool operator== ( const sc_fxcast_switch& , const sc_fxcast_switch& );
        friend bool operator!= ( const sc_fxcast_switch& , const sc_fxcast_switch& );

        // Member functions
        std::string to_string() const;
        void print( std::ostream& = std::cout ) const;
        void dump( std::ostream& = std::cout ) const;
};

}         // namespace sc_dt
```

#### Public constructors

- 인자를 주면 **`SC_OFF` 또는 `SC_ON`** 값이어야 shall (7.10.8). `SC_ON` = fixed-point 산술 동작, `SC_OFF` = floating-point 산술 동작.
- **기본 생성자는 현재 scope의 floating-point cast context를 사용해야 shall.**

#### Member functions / operators

| 항목 | 규칙 |
|---|---|
| `void print( std::ostream& = std::cout ) const;` | 인스턴스 값을 출력 스트림에 출력해야 shall |
| `std::string to_string() const;` | switch 상태를 문자열 **`"SC_OFF"` 또는 `"SC_ON"`**으로 반환해야 shall |
| `operator=` | `cast_switch`를 좌변 `sc_fxcast_switch`에 대입해야 shall |
| `operator==` | `switch_a == switch_b`이면 true, 아니면 false를 반환해야 shall |
| `operator!=` | `switch_a != switch_b`이면 true, 아니면 false를 반환해야 shall |
| `std::ostream& operator<< ( std::ostream& os , const sc_fxcast_switch& a );` | `a`의 인스턴스 값을 `os`에 출력해야 shall |

### 7.11.7 sc_fxcast_context

#### Description

`sc_fxcast_context`는 **fixed-point 객체를 위한 floating-point cast context**를 만드는 데 사용되어야 shall.

#### Class definition

```cpp
namespace sc_dt {

class sc_fxcast_context
{
    public:
        explicit sc_fxcast_context( const sc_fxcast_switch& , sc_context_begin† = SC_NOW );
        sc_fxcast_context();

        void begin();
        void end();
        static const sc_fxcast_switch& default_value();
        const sc_fxcast_switch& value() const;
};

}         // namespace sc_dt
```

#### Public constructor

- 첫 인자는 사용할 floating-point cast switch여야 shall. 둘째 인자를 주면 **`SC_NOW` 또는 `SC_LATER`** 값이어야 shall.

#### Member functions

| 함수 | 규칙 |
|---|---|
| `void begin();` | 현재 floating-point cast context를 설정해야 shall (7.10.8) |
| `static const sc_fxcast_switch& default_value();` | 현재 context에 있는 cast switch를 반환해야 shall |
| `void end();` | floating-point cast context를 비활성화하고 **floating-point cast context stack 최상단에서 제거**해야 shall |
| `const sc_fxcast_switch& value() const;` | cast switch를 반환해야 shall |

---

## 7.12 Control of string representation

### 7.12.1 Description

`sc_numrep` 타입은 데이터 타입 객체의 **`to_string` 멤버 함수 인자로 전달되어** 수 표현의 문자열 형식을 제어하는 데 사용된다.

### 7.12.2 Class definition

```cpp
namespace sc_dt {

    enum sc_numrep
    {
       SC_NOBASE = 0,
       SC_BIN = 2,
       SC_OCT = 8,
       SC_DEC = 10,
       SC_HEX = 16,
       SC_BIN_US,
       SC_BIN_SM,
       SC_OCT_US,
       SC_OCT_SM,
       SC_HEX_US,
       SC_HEX_SM,
       SC_CSD
    };

    std::string to_string( sc_numrep );

};        // namespace sc_dt
```

### 7.12.3 Functions

`std::string to_string( sc_numrep );`
— 함수 `to_string`은 `sc_numrep` 열거형의 **해당 상수 이름과 같은 문자열**을 반환해야 shall.

```cpp
to_string(SC_HEX) == "SC_HEX" // is true
```

---


## 흔한 위반 (pitfalls) — 7.1~7.9 정수·논리 타입

- **proxy 객체를 변수에 담거나 함수에서 반환** — LRM §7.2.1, §7.5.6.3, §7.5.7.3, §7.6.7.3, §7.6.8.3, §7.7.3, §7.9.7.3, §7.9.8.3, §7.9.9.3: `sc_int_bitref†`, `sc_int_subref†`, `sc_signed_subref†`, `sc_concatref†`, `sc_bitref†<T>`, `sc_subref†<T>`, `sc_concref†<T1,T2>` 등의 인스턴스를 애플리케이션이 명시적으로 생성하면 안 된다 shall not이며, 포인터/레퍼런스 선언도 should not. 함수 반환 타입 사용은 참조 대상 수명 문제로 강력히 비권장. 표현식 안에서만 쓰고, 필요하면 `sc_int<W>`/`sc_bv<W>` 같은 값 타입으로 즉시 복사하라.
- **`I1.range(0, 3)`처럼 좌측 인덱스를 우측보다 작게 지정** — LRM §7.2.7, §7.5.7.3, §7.6.8.3, §7.9.8.3: 좌측 인덱스가 우측보다 작으면 error. 비트 순서 역전은 part-select로 불가능하다. `reverse()`를 쓰라.
- **범위 밖 비트/파트 선택** — LRM §7.2.6, §7.2.7: `operator[]`의 비트 위치나 `range()`의 인덱스가 객체 범위를 벗어나면 error.
- **`'X'`/`'Z'`를 담은 `sc_lv`를 정수로 변환** — LRM §7.2.10, §7.9.4.6, §7.9.8.5, §7.9.9.5: `to_int`/`to_uint`/`to_int64` 등의 호출은 error. 먼저 `is_01()`로 확인하라.
- **`'X'`/`'Z'`를 담은 logic vector에 `to_string(numrep, ...)` 호출** — LRM §7.3, §7.9.4.6: error. 인자 없는 `to_string()`은 `'X'`/`'Z'`를 그대로 문자열로 만들 수 있다.
- **`sc_bv`/`sc_bv_base` 원소에 high-impedance나 unknown 대입** — LRM §7.9.3.3, §7.9.5.3: error. 4치 값이 필요하면 `sc_lv`를 쓰라.
- **`A = '0';`로 `sc_logic` 시그널에 대입** — LRM §7.9.2.8 예제: ambiguous conversion으로 error. `SC_LOGIC_0` 상수를 쓰라.
- **`sc_logic( 4 )` 등 0~3 범위 밖 정수로 `sc_logic` 생성** — LRM §7.9.2.3: error.
- **`sc_logic`에 문자열 리터럴 대입** — LRM §7.9.2.2 / §7.9.2.3: `const char*` 생성자·대입 연산자가 Disabled(private). 문자 리터럴만 가능.
- **워드 길이 > 1인 값을 single-bit logic type에 대입** — LRM §7.2.2: error.
- **limited-precision 정수 표현식이 64비트를 넘을 것으로 기대** — LRM §7.2.3: `sc_int`/`sc_uint`만의 산술 반환 타입은 최대 64비트 implementation-defined C++ 정수이고, 초과 시 동작은 undefined. 정확도가 필요하면 `sc_bigint`/`sc_biguint`를 쓰라.
- **`sc_int_base`/`sc_uint_base`의 워드 길이를 하부 표현 크기보다 크게 지정** — LRM §7.5.2.3, §7.5.3.3, §7.5.4.3, §7.5.5.3: 최대 크기를 넘으면 안 된다 shall not.
- **`sc_signed`/`sc_unsigned`/`sc_bigint`/`sc_biguint`를 C++ 정수 자리에 그대로 전달** — LRM §7.6.3.3, §7.6.4.3, §7.6.5.3, §7.6.6.3: 암시적 변환 멤버 함수가 없다. `to_int()`/`to_int64()` 등 명시적 변환이 필요.
- **finite-precision 정수 표현식에 부동소수 피연산자 직접 사용** — LRM §7.6.2 d): 직접 쓰면 안 된다 shall not. 먼저 정수 타입으로 변환해야 should.
- **`sc_bigint`/`sc_biguint`에 fixed-point 값을 암시적으로 넘김** — LRM §7.6.5.4 NOTE, §7.6.6.4 NOTE: fixed-point → big integer 변환은 **명시적**으로만 호출된다.
- **부호 있는 값을 뺄셈해 unsigned 결과를 기대** — LRM §7.6.2 b), Table 13: finite-precision 정수의 뺄셈 반환 타입은 **항상 signed**. `U - i`도 `sc_signed`.
- **shift 연산의 우측 피연산자가 음수** — LRM §7.6.3.7, §7.6.4.7: finite-precision 정수에서는 **undefined**. §7.9.3.7, §7.9.4.7, §7.9.8.6: vector/part-select에서는 **error**.
- **part-select를 signed 값으로 취급** — LRM §7.2.7: part-select 비트는 연관 객체의 부호를 반영하지 않고 **unsigned 이진수**로 취급되며, 더 긴 대상에 대입 시 항상 zero-extend 된다.
- **다른 base type끼리 concatenate** — LRM §7.2.8, §7.7.3, §7.9.9.3: `(sc_bv, sc_uint)`처럼 공통 concatenation base type이 없는 조합은 ill-formed. `bool` 두 개의 concatenate도 불가.
- **`bool` 인자를 포함한 concatenation을 lvalue로 사용** — LRM §7.2.8: `bool` 인자가 있으면 결과는 항상 rvalue shall.
- **`concat`/`operator,` 인자를 괄호 없이 사용** — LRM §7.2.8 NOTE 1: comma 연산자 우선순위가 낮아 괄호가 필요하다.
- **`sc_bv`/`sc_lv`에 원소 수가 부족한 `bool`/`sc_logic` 배열 대입** — LRM §7.9.3.3, §7.9.4.3, §7.9.5.3, §7.9.5.4, §7.9.6.3, §7.9.6.4, §7.9.8.4, §7.9.9.4: 결과는 **undefined**.
- **`sc_lv<W>`의 기본 초기값을 0으로 가정** — LRM §7.9.6.4: `sc_lv()`의 모든 원소 초기값은 **unknown**이다(`sc_bv()`는 logic 0). `sc_logic()`의 기본값도 unknown(§7.9.2.4).
- **`length_context`의 `end()`를 top이 아닌 context에 호출** — LRM §7.2.4: `end`는 stack top인 length context에 대해서만 호출해야 shall. context는 한 번만 활성화/비활성화되어야 shall.
- **비결정적 실행 순서 함수 안에서 length context 활성화** — LRM §7.2.4 NOTE 2: 다른 부분의 default parameter에 영향을 주어 **implementation-dependent** 동작을 낳을 수 있다.
- **잘림 시 경고를 기대** — LRM §7.2.2, §7.2.8: 구현이 warning을 낼 의무는 없다. 워드 길이를 스스로 검증하라.
- **`sc_value_base†` 멤버를 직접 호출** — LRM §7.4.3: 애플리케이션은 호출하면 안 되며 should not, 파생 클래스에서 override되지 않은 멤버 함수를 호출하면 error shall.
- **`sc_concatref†`를 함수 인자/반환값으로 사용** — LRM §7.7.3: 명시적 생성뿐 아니라 **함수 인자/반환값을 통한 암시적 생성도 shall not**.
- **`sc_generic_base` 파생 클래스에 필수 멤버 누락** — LRM §7.8.3: `length`, `to_uint64`, `to_int64`, `to_sc_unsigned`, `to_sc_signed`를 public const로 제공해야 shall.
- **vector의 reduce 반환 타입을 `bool`로 가정** — LRM §7.9.3.2, §7.9.4.2, §7.9.8.2, §7.9.9.2: vector/part-select/concatenation의 `and_reduce` 등은 **`sc_logic_value_t`**를 반환한다. 정수 타입(§7.2.9, §7.5.2.2 등)만 `bool`.
---


## 흔한 위반 (pitfalls) — 7.10~7.12 고정소수점·context

- **`sc_fxnum` / `sc_fxnum_fast` 객체를 직접 생성** — LRM §7.10.11.3, §7.10.12.3: 애플리케이션(그리고 구현)은 `sc_fxnum`·`sc_fxnum_fast` 인스턴스를 직접 만들면 안 된다 shall not. 기본·복사 생성자는 private(Disabled). `sc_fix`/`sc_ufix`/`sc_fixed`… 파생 타입을 쓰고, 다형적 접근은 `sc_fxnum&`·`sc_fxnum*`로 하라.
- **`wl <= 0` 또는 `n_bits < 0` 지정** — LRM §7.10.15.3 등 모든 fixed-point 타입: word length는 0보다 커야 shall, saturated bit 수는 0보다 작으면 안 된다 shall not. `sc_length_param`·`sc_fxtype_params`도 길이가 0보다 커야 shall (§7.11.2.3, §7.11.4.3).
- **초기화하지 않은 fixed-point 변수를 연산에 사용** — LRM §7.10.4.2, §7.10.13.3, §7.10.14.3: uninitialized 변수 사용 자체는 허용되지만 **연산 결과는 undefined**. `sc_fxval`/`sc_fxval_fast`는 static일 때만 0으로 초기화된다. 선언 시 초기값을 주라.
- **`sc_fixed_fast` 등 fast 타입으로 bit-true 결과를 기대** — LRM §7.10.4.3 NOTE, §7.10.17.3, §7.10.21.3: fast 타입은 double 기반이고 mantissa가 53비트로 제한되므로 bit-true 동작이 보장되지 않는다. 연산 결과 word length가 53비트를 넘지 않아야 한다(should not).
- **`sc_fxval`/`sc_fxval_fast`에 bit/part select 시도** — LRM §7.10.6: variable-precision 타입에는 지원되지 않는다.
- **bitwise 연산에서 signed와 unsigned를 섞음** — LRM §7.10.5: 이항 bitwise 연산자의 두 피연산자는 **같은 부호 표현**이어야 shall. 또한 피연산자는 variable-precision이면 안 된다 shall not.
- **fixed-point에 `%` 사용** — LRM §7.10.5: remainder 연산자는 fixed-point 타입에서 지원되지 않는다.
- **part-select 없이 bit-pattern 문자열을 fixed-point 객체에 대입** — LRM §7.10.9.3: 결과가 **undefined**. part-select를 거쳐 대입하라.
- **unsigned 타입에 `SC_WRAP_SM` 지정** — LRM Table 42(§7.10.10.4): unsigned 수에 대해 **정의되지 않았다**.
- **`n_bits`가 wrap 이외 모드에서도 동작한다고 가정** — LRM §7.10.10.3, §7.10.10.4: `n_bits`는 **`SC_WRAP`과 `SC_WRAP_SM`에서만** 고려된다. 다른 모드에서 saturated bit는 R 비트로 취급된다.
- **context 없이 `sc_fix`/`sc_ufix` 기본 생성자를 쓰고 wl/iwl을 가정** — LRM §7.10.15.4, §7.11.4.4: 기본 생성자는 **생성 시점 scope의 fixed-point context**를 따르며, context가 없으면 Table 38의 wl=32, iwl=32, `SC_TRN`, `SC_WRAP`, n_bits=0이 된다.
- **선언 후 타입 파라미터 변경 시도** — LRM §7.10.4.2: 변수의 타입 파라미터 값은 선언 이후 수정되면 안 된다 shall not.
- **bit-select/part-select 객체에 참조·포인터를 저장** — LRM §7.10.23.3, §7.10.24.3: 명시적 인스턴스 생성은 shall not, 참조·포인터 선언은 권장되지 않는다(should not). part-select에는 산술 연산자도 제공되지 않는다.
- **variable-precision 나눗셈이 무한 정밀도라고 가정** — LRM §7.10.7: 최대 word length 도달 시 구현 동작은 **undefined**이고, 결과는 이상적 결과에 가장 가까운 **implementation-dependent** 값이다.
- **`SC_OFF` cast context가 fixed-point 양자화를 유지한다고 가정** — LRM §7.10.8: `sc_fxcast_switch(SC_OFF)`는 산술 동작을 **floating-point**로 만든다(예제에서 `10.0 * 0.1`이 `.999908447265625`가 아니라 `1`이 된다). 기본 floating-point context는 `SC_ON`이다.
