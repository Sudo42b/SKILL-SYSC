# SystemC 코딩 규칙

이 리포의 모든 예제 코드가 따르는 규칙 — `sysc-lt` / `sysc-at` / `sysc-ca` / `sysc-verify`의 예제와 `examples/lt_demo`.

전제: **IEEE 1666-2023 / SystemC 3.0.x 이상.** 2.3.x 타깃 코드는 각 항목의 `[2.3.x]`를 따른다.

## 규칙

1. **`SC_MODULE` / `SC_CTOR` 사용 금지.**
   `class X : public sc_core::sc_module`로 직접 작성하고 생성자도 수동 작성한다.
   생성자는 `explicit`, 첫 인자는 `sc_core::sc_module_name`.

2. **`SC_HAS_PROCESS` 사용 금지** — 1666-2023에서 deprecated (Annex C `ah)`, Annex D `10)`).
   생성자 안에서 `SC_THREAD` / `SC_METHOD`를 바로 호출한다.
   `[2.3.x]` 2.3.x를 타깃하는 파일에서는 `SC_HAS_PROCESS`가 필요하다.
   **해당 파일 최상단에 타깃 버전을 주석으로 명시할 것.**

3. **포트 / export / 소켓은 `public`.**
   부모 모듈이 바인딩하므로 private이면 컴파일되지 않는다.
   그 외 모든 내부 상태는 `private` + trailing underscore (`count_`).

4. **포트 / 소켓 / 서브모듈 / 채널 이름은 `SC_NAMED()`로 선언한다** (Annex D `11)`).
   변수명과 SystemC 계층 이름의 불일치를 방지한다.
   **`SC_NAMED`로 선언하는 것에는 underscore를 붙이지 않는다** — 변수명이 곧 계층 이름이 되고,
   계층 이름은 트레이스·리포트·디버거에서 객체를 찾는 경로다 (§5.17). underscore만 붙는 순수 C++ 멤버와 구분된다.
   `[2.3.x]` 초기화 리스트에서 문자열로 지정: `socket("socket")`

5. **기존 매크로 기반 레거시 모듈은 개조하지 않는다.** 신규 코드에만 적용.

6. **빌드는 `-Wall -Wextra` 경고 0건이 머지 조건.**
   `SC_ALLOW_DEPRECATED_IEEE_API`는 레거시 디렉터리에서만 허용.

## 왜

**`SC_HAS_PROCESS`** — SystemC 3.0.2가 `[[deprecated]]`로 표시한 **유일한** 항목이다. 규칙 6과 맞물려, 이걸 쓰면 신규 코드는 머지될 수 없다:

```
warning: 'sc_core::sc_has_process_used' is deprecated: SC_HAS_PROCESS(user_module_name)
is obsolete in IEEE 1666-2023, define SC_ALLOW_DEPRECATED_IEEE_API to suppress.
```

거꾸로, **경고 0건이 deprecated 미사용을 뜻하지는 않는다.** Annex C의 34개 항목 중 3.0.2가 경고하는 것은 `SC_HAS_PROCESS` 하나뿐이고 나머지 33개는 조용히 컴파일된다. 규칙 5의 레거시 판별은 컴파일러가 아니라 `references/annexC-deprecated.md`를 손으로 대조해야 한다.

**포트가 `public`이어야 하는 이유** — 부모가 자식의 포트를 바인딩한다. private이면:

```
error: 'sc_core::sc_out<int> Producer::out' is private within this context
```

**`SC_NAMED`** — 수동으로 `out("out")`이라 쓰면 변수명을 바꿀 때 문자열이 조용히 어긋난다. `SC_NAMED`는 어긋날 수 없다. 가변인자이므로 추가 생성자 인자를 받는 서브모듈에도 쓴다: `SC_NAMED(mem, 4096)`.

## 골격

```cpp
#include "systemc"                              // systemc.h 아닌 systemc (§5.1.2)

class Producer : public sc_core::sc_module {
public:
    sc_core::sc_out<int>  SC_NAMED(out);        // 포트는 public, underscore 없음
    sc_core::sc_in<bool>  SC_NAMED(clk);

    explicit Producer(sc_core::sc_module_name name)
        : sc_core::sc_module(name) {
        SC_THREAD(run);                         // SC_HAS_PROCESS 없이
        sensitive << clk.pos();                 // §5.2.14 프로세스 생성 직후에만 유효
    }

private:
    int count_ = 0;                             // 순수 C++ 멤버: private + underscore (§5.2.10)

    void run() {
        for (;;) {                              // §5.2.11 조기 종료 방지 관용구
            wait();
            out.write(++count_);
        }
    }
};

class Top : public sc_core::sc_module {
public:
    explicit Top(sc_core::sc_module_name name)
        : sc_core::sc_module(name) {
        p.clk(clk);                             // §5.12.7 named binding
        p.out(sig);
    }

private:
    sc_core::sc_clock       SC_NAMED(clk);      // 채널·서브모듈도 SC_NAMED, underscore 없음
    sc_core::sc_signal<int> SC_NAMED(sig);
    Producer                SC_NAMED(p);
};

int sc_main(int, char*[]) {                     // §4.4.5.2 전역 네임스페이스, 이 시그니처 shall
    Top top("top");                             // elaboration = 여기부터 첫 sc_start 직전까지
    sc_core::sc_start(100, sc_core::SC_NS);
    sc_core::sc_stop();                         // §4.4.5.3 end_of_simulation 콜백을 위해 should
    return 0;
}
```

## SystemC 2.3.x를 타깃할 때

2.3.x는 IEEE 1666-2011이다. 파일 최상단에 타깃을 명시하고, 두 규칙을 뒤집는다:

```cpp
// 타깃: SystemC 2.3.x (IEEE 1666-2011) — SC_HAS_PROCESS 필요, SC_NAMED 없음
```

- `SC_HAS_PROCESS`가 **필요하다** — 2011에서는 생략할 수 없다.
- `SC_NAMED`가 **없다** — 초기화 리스트에서 수동으로 이름을 준다: `out("out")`.

나머지(클래스 직접 작성, `explicit`, public 포트, private 상태, 경고 0건)는 그대로 유효하다.
